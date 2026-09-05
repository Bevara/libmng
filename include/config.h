/* config.h.in.  Generated from configure.in by autoheader.  */

/* Define if building universal (internal helper macro) */
/* #undef AC_APPLE_UNIVERSAL_BUILD */

/* Define to 1 if you have the <dlfcn.h> header file. */
#define HAVE_DLFCN_H 1

/* Define to 1 if you have the <inttypes.h> header file. */
#define HAVE_INTTYPES_H 1

/* define if you want JPEG support */
#define HAVE_LIBJPEG 1

/* define if you want lcms v1 support */
/* #undef HAVE_LIBLCMS1 */

/* define if you want lcms v2 support */
/* #undef HAVE_LIBLCMS2 */

/* Define to 1 if you have the `z' library (-lz). */
#define HAVE_LIBZ 1

/* Define to 1 if you have the <memory.h> header file. */
#define HAVE_MEMORY_H 1

/* Define to 1 if you have the <stdint.h> header file. */
#define HAVE_STDINT_H 1

/* Define to 1 if you have the <stdlib.h> header file. */
#define HAVE_STDLIB_H 1

/* Define to 1 if you have the <strings.h> header file. */
#define HAVE_STRINGS_H 1

/* Define to 1 if you have the <string.h> header file. */
#define HAVE_STRING_H 1

/* Define to 1 if you have the <sys/stat.h> header file. */
#define HAVE_SYS_STAT_H 1

/* Define to 1 if you have the <sys/types.h> header file. */
#define HAVE_SYS_TYPES_H 1

/* Define to 1 if you have the <unistd.h> header file. */
#define HAVE_UNISTD_H 1

/* Define to the sub-directory in which libtool stores uninstalled libraries.
   */
/* #undef LT_OBJDIR */

/* define if you want chunk access support */
#define MNG_ACCESS_CHUNKS 1

/* enable building standard shared object */
#define MNG_BUILD_SO 1

/* enable verbose error text */
#define MNG_ERROR_TELLTALE 1

/* define if you want full lcms support */
/* #undef MNG_FULL_CMS */

/* enable support for accessing chunks */
#define MNG_STORE_CHUNKS 1

/* define if you want display support */
#define MNG_SUPPORT_DISPLAY 1

/* define if you want dynamic support */
#define MNG_SUPPORT_DYNAMICMNG 1

/* define if you want full mng support */
#define MNG_SUPPORT_FULL 1

/* define if you want read support */
#define MNG_SUPPORT_READ 1

/* enable support for debug tracing */
/* #undef MNG_SUPPORT_TRACE */

/* define if you want write support */
#define MNG_SUPPORT_WRITE 1

/* enable support for debug messages */
/* #undef MNG_TRACE_TELLTALE */

/* MAJOR number of version */
#define MNG_VERSION_MAJOR 2

/* MINOR number of version */
#define MNG_VERSION_MINOR 0

/* PATCH number of version */
#define MNG_VERSION_RELEASE 2

/* but: libmng.dll (!) */
#define MNG_VERSION_DLL 2

/* eg. libmng.so.1 */
#define MNG_VERSION_SO 2

/* Name of package */
/* #undef PACKAGE */

/* Define to the address where bug reports for this package should be sent. */
/* #undef PACKAGE_BUGREPORT */

/* Define to the full name of this package. */
/* #undef PACKAGE_NAME */

/* Define to the full name and version of this package. */
/* #undef PACKAGE_STRING */

/* Define to the one symbol short name of this package. */
/* #undef PACKAGE_TARNAME */

/* Define to the home page for this package. */
/* #undef PACKAGE_URL */

/* Define to the version of this package. */
#define PACKAGE_VERSION "2.0.2"

/* Version number of package */
#define VERSION "2.0.2"

/* Define to 1 if the C compiler supports function prototypes. */
/* #undef PROTOTYPES */

/* Define to 1 if you have the ANSI C header files. */
/* #undef STDC_HEADERS */

/* Number of bits in a file offset, on hosts where this is settable. */
/* #undef _FILE_OFFSET_BITS */

/* Define for large files, on AIX-style hosts. */
/* #undef _LARGE_FILES */

/* Define like PROTOTYPES; this can be used by system headers. */
/* #undef __PROTOTYPES */

/* Define to empty if `const' does not conform to ANSI C. */
/* #undef const */

/*--------------------------------------------------------*/
/* #undef WORDS_BIGENDIAN */

/* Define WORDS_BIGENDIAN to 1 if your processor stores words with the most
   significant byte first (like Motorola and SPARC, unlike Intel). */
#if defined AC_APPLE_UNIVERSAL_BUILD
#if defined __BIG_ENDIAN__
#define WORDS_BIGENDIAN 1
#endif
#else
#ifndef WORDS_BIGENDIAN
/* #undef WORDS_BIGENDIAN */
#endif
#endif

/* Define to `__inline__' or `__inline' if that's what the C compiler
   calls it, or to nothing if 'inline' is not supported under any name.  */
#ifndef __cplusplus
#ifndef inline
#define inline __inline
#endif
#endif

