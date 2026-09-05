/*
 *			GPAC - Multimedia Framework C SDK
 *
 *  This file is part of GPAC / MNG decoder filter, based on libmng
 *  (http://www.libmng.com/) - Multiple-image Network Graphics, the animated
 *  companion of PNG, plus its single-frame JNG variant which carries JPEG
 *  data.
 *
 *  libmng drives the decode through callbacks rather than returning frames, so
 *  the filter gives it a canvas to draw into and sends a packet each time it
 *  asks for a refresh. Animation timing goes through a virtual clock: libmng
 *  asks for a timer, and instead of waiting the filter simply advances its own
 *  millisecond counter and resumes - the frames come out with the right
 *  timestamps, as fast as they can be decoded.
 */

#include <gpac/filters.h>
#include <gpac/constants.h>
#include <string.h>
#include <stdlib.h>

#include <libmng.h>

typedef struct
{
	GF_FilterPid *ipid, *opid;
	Bool is_playing;

	/* input, walked by the readdata callback */
	const u8 *data;
	u32 size, pos;

	/* canvas libmng draws into, RGB8 */
	u8 *canvas;
	u32 width, height;

	/* virtual clock, in milliseconds */
	mng_uint32 now;
	mng_uint32 wait;

	u32 nb_frames;
	Bool props_set;
	Bool failed;
} GF_MNGDecCtx;

static mng_ptr mngdec_alloc(mng_size_t len)
{
	void *p = gf_malloc((size_t)len);
	if (p)
		memset(p, 0, (size_t)len);
	return p;
}

static void mngdec_free(mng_ptr p, mng_size_t len)
{
	(void)len;
	gf_free(p);
}

static mng_bool mngdec_openstream(mng_handle h)
{
	(void)h;
	return MNG_TRUE;
}

static mng_bool mngdec_closestream(mng_handle h)
{
	(void)h;
	return MNG_TRUE;
}

static mng_bool mngdec_readdata(mng_handle h, mng_ptr buf, mng_uint32 size, mng_uint32 *read)
{
	GF_MNGDecCtx *ctx = (GF_MNGDecCtx *)mng_get_userdata(h);
	u32 avail = ctx->size - ctx->pos;
	if (size > avail)
		size = avail;
	if (size)
		memcpy(buf, ctx->data + ctx->pos, size);
	ctx->pos += size;
	*read = size;
	return MNG_TRUE;
}

/* Called once the header is known: this is where the canvas is sized and the
 * output pid described. */
static mng_bool mngdec_processheader(mng_handle h, mng_uint32 width, mng_uint32 height)
{
	GF_MNGDecCtx *ctx = (GF_MNGDecCtx *)mng_get_userdata(h);

	if (!width || !height)
		return MNG_FALSE;
	ctx->width = width;
	ctx->height = height;
	ctx->canvas = (u8 *)gf_malloc((size_t)width * height * 3);
	if (!ctx->canvas)
		return MNG_FALSE;
	memset(ctx->canvas, 0, (size_t)width * height * 3);

	if (mng_set_canvasstyle(h, MNG_CANVAS_RGB8) != MNG_NOERROR)
		return MNG_FALSE;

	gf_filter_pid_set_property(ctx->opid, GF_PROP_PID_WIDTH, &PROP_UINT(width));
	gf_filter_pid_set_property(ctx->opid, GF_PROP_PID_HEIGHT, &PROP_UINT(height));
	gf_filter_pid_set_property(ctx->opid, GF_PROP_PID_STRIDE, &PROP_UINT(width * 3));
	gf_filter_pid_set_property(ctx->opid, GF_PROP_PID_PIXFMT, &PROP_UINT(GF_PIXEL_RGB));
	ctx->props_set = GF_TRUE;
	return MNG_TRUE;
}

static mng_ptr mngdec_getcanvasline(mng_handle h, mng_uint32 line)
{
	GF_MNGDecCtx *ctx = (GF_MNGDecCtx *)mng_get_userdata(h);
	if (!ctx->canvas || (line >= ctx->height))
		return NULL;
	return ctx->canvas + (size_t)line * ctx->width * 3;
}

/* One completed frame: sent as a packet timestamped with the virtual clock. */
static mng_bool mngdec_refresh(mng_handle h, mng_uint32 x, mng_uint32 y, mng_uint32 w, mng_uint32 height)
{
	GF_MNGDecCtx *ctx = (GF_MNGDecCtx *)mng_get_userdata(h);
	GF_FilterPacket *dst_pck;
	u8 *output;
	u32 out_size;
	(void)x; (void)y; (void)w; (void)height;

	if (!ctx->canvas || !ctx->width || !ctx->height)
		return MNG_FALSE;

	out_size = ctx->width * ctx->height * 3;
	dst_pck = gf_filter_pck_new_alloc(ctx->opid, out_size, &output);
	if (!dst_pck)
	{
		ctx->failed = GF_TRUE;
		return MNG_FALSE;
	}
	memcpy(output, ctx->canvas, out_size);
	gf_filter_pck_set_cts(dst_pck, ctx->now);
	gf_filter_pck_set_sap(dst_pck, GF_FILTER_SAP_1);
	gf_filter_pck_send(dst_pck);
	ctx->nb_frames++;
	return MNG_TRUE;
}

static mng_uint32 mngdec_gettickcount(mng_handle h)
{
	GF_MNGDecCtx *ctx = (GF_MNGDecCtx *)mng_get_userdata(h);
	return ctx->now;
}

/* libmng asks to be resumed in msecs; the filter records the delay instead of
 * sleeping, and jumps the clock forward when it resumes. */
static mng_bool mngdec_settimer(mng_handle h, mng_uint32 msecs)
{
	GF_MNGDecCtx *ctx = (GF_MNGDecCtx *)mng_get_userdata(h);
	ctx->wait = msecs;
	return MNG_TRUE;
}

static GF_Err mngdec_configure_pid(GF_Filter *filter, GF_FilterPid *pid, Bool is_remove)
{
	GF_MNGDecCtx *ctx = (GF_MNGDecCtx *)gf_filter_get_udta(filter);

	if (is_remove)
	{
		if (ctx->opid)
		{
			gf_filter_pid_remove(ctx->opid);
			ctx->opid = NULL;
		}
		ctx->ipid = NULL;
		return GF_OK;
	}
	if (!gf_filter_pid_check_caps(pid))
		return GF_NOT_SUPPORTED;

	ctx->ipid = pid;
	gf_filter_pid_set_framing_mode(pid, GF_TRUE);

	if (!ctx->opid)
		ctx->opid = gf_filter_pid_new(filter);

	gf_filter_pid_set_property(ctx->opid, GF_PROP_PID_STREAM_TYPE, &PROP_UINT(GF_STREAM_VISUAL));
	gf_filter_pid_set_property(ctx->opid, GF_PROP_PID_CODECID, &PROP_UINT(GF_CODECID_RAW));
	gf_filter_pid_set_property(ctx->opid, GF_PROP_PID_PIXFMT, &PROP_UINT(GF_PIXEL_RGB));
	/* MNG timing is in milliseconds. */
	gf_filter_pid_set_property(ctx->opid, GF_PROP_PID_TIMESCALE, &PROP_UINT(1000));

	return GF_OK;
}

static Bool mngdec_process_event(GF_Filter *filter, const GF_FilterEvent *evt)
{
	GF_MNGDecCtx *ctx = (GF_MNGDecCtx *)gf_filter_get_udta(filter);
	switch (evt->base.type)
	{
	case GF_FEVT_PLAY:
		ctx->is_playing = GF_TRUE;
		return GF_FALSE;
	case GF_FEVT_STOP:
		ctx->is_playing = GF_FALSE;
		return GF_FALSE;
	default:
		return GF_FALSE;
	}
}

static GF_Err mngdec_process(GF_Filter *filter)
{
	GF_FilterPacket *pck;
	u8 *data;
	u32 size, guard;
	mng_handle h;
	mng_retcode rc;
	GF_MNGDecCtx *ctx = (GF_MNGDecCtx *)gf_filter_get_udta(filter);

	pck = gf_filter_pid_get_packet(ctx->ipid);
	if (!pck)
	{
		if (gf_filter_pid_is_eos(ctx->ipid))
		{
			gf_filter_pid_set_eos(ctx->opid);
			return GF_EOS;
		}
		return GF_OK;
	}
	data = (u8 *)gf_filter_pck_get_data(pck, &size);
	if (!data)
	{
		gf_filter_pid_drop_packet(ctx->ipid);
		return GF_IO_ERR;
	}

	ctx->data = data;
	ctx->size = size;
	ctx->pos = 0;
	ctx->canvas = NULL;
	ctx->width = ctx->height = 0;
	ctx->now = 0;
	ctx->wait = 0;
	ctx->nb_frames = 0;
	ctx->failed = GF_FALSE;

	h = mng_initialize((mng_ptr)ctx, mngdec_alloc, mngdec_free, MNG_NULL);
	if (!h)
	{
		gf_filter_pid_drop_packet(ctx->ipid);
		return GF_OUT_OF_MEM;
	}

	mng_setcb_openstream(h, mngdec_openstream);
	mng_setcb_closestream(h, mngdec_closestream);
	mng_setcb_readdata(h, mngdec_readdata);
	mng_setcb_processheader(h, mngdec_processheader);
	mng_setcb_getcanvasline(h, mngdec_getcanvasline);
	mng_setcb_refresh(h, mngdec_refresh);
	mng_setcb_gettickcount(h, mngdec_gettickcount);
	mng_setcb_settimer(h, mngdec_settimer);

	rc = mng_readdisplay(h);

	/* MNG_NEEDTIMERWAIT means "call me back when the delay has elapsed": the
	 * clock is moved forward and the decode resumed, until the animation ends.
	 * The guard stops a file that loops for ever. */
	guard = 0;
	while ((rc == MNG_NEEDTIMERWAIT) && (guard++ < 10000) && !ctx->failed)
	{
		ctx->now += ctx->wait;
		ctx->wait = 0;
		rc = mng_display_resume(h);
	}

	mng_cleanup(&h);
	gf_filter_pid_drop_packet(ctx->ipid);

	if (ctx->canvas)
	{
		gf_free(ctx->canvas);
		ctx->canvas = NULL;
	}

	if (!ctx->nb_frames)
	{
		GF_LOG(GF_LOG_ERROR, GF_LOG_CODEC, ("[MNGDec] No frame decoded (libmng code %d)\n", (int)rc));
		return GF_NON_COMPLIANT_BITSTREAM;
	}

	gf_filter_pid_set_property(ctx->opid, GF_PROP_PID_NB_FRAMES, &PROP_UINT(ctx->nb_frames));
	gf_filter_pid_set_eos(ctx->opid);
	return GF_EOS;
}

static void mngdec_finalize(GF_Filter *filter)
{
	GF_MNGDecCtx *ctx = (GF_MNGDecCtx *)gf_filter_get_udta(filter);
	if (ctx->canvas)
		gf_free(ctx->canvas);
}

static const GF_FilterCapability MNGDecCaps[] =
	{
		CAP_UINT(GF_CAPS_INPUT, GF_PROP_PID_STREAM_TYPE, GF_STREAM_FILE),
		CAP_STRING(GF_CAPS_INPUT, GF_PROP_PID_FILE_EXT, "mng|jng"),
		CAP_STRING(GF_CAPS_INPUT, GF_PROP_PID_MIME, "video/x-mng|image/x-mng|image/x-jng"),
		CAP_UINT(GF_CAPS_OUTPUT, GF_PROP_PID_STREAM_TYPE, GF_STREAM_VISUAL),
		CAP_UINT(GF_CAPS_OUTPUT, GF_PROP_PID_CODECID, GF_CODECID_RAW),
};

GF_FilterRegister MNGDecoderRegister = {
	.name = "mngdec",
	GF_FS_SET_DESCRIPTION("MNG and JNG decoder")
		GF_FS_SET_HELP("This filter decodes MNG animations and JNG images using libmng, sending one frame per packet with the timing carried by the file.")
			.private_size = sizeof(GF_MNGDecCtx),
	SETCAPS(MNGDecCaps),
	.configure_pid = mngdec_configure_pid,
	.process = mngdec_process,
	.process_event = mngdec_process_event,
	.finalize = mngdec_finalize,
};

const GF_FilterRegister *EMSCRIPTEN_KEEPALIVE mngdec_register(GF_FilterSession *session)
{
	return &MNGDecoderRegister;
}

#include "filter_register.h"
__attribute__((constructor))
void register_mngdec(void) {
    gf_filter_auto_register("mngdec", mngdec_register);
}
