cmake_minimum_required(VERSION 3.7)
project(Filters)

include(filters.cmake)

add_definitions(-fpic)

SET(LIBMNG_SRC
        ${CMAKE_CURRENT_SOURCE_DIR}/dec_mng.c
)

# libjpeg is here for JNG, whose frames are JPEG-compressed; zlib comes from
# emscripten's own port, like it does for the libpng filter.
SET(LIBMNG_LIBS
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/libmng.a
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/libjpeg.a
)

SET(LIBMNG_INC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

add_filter(libmng
        "${LIBMNG_SRC}"
        "${LIBMNG_LIBS}"
        []
        ""
        "${LIBMNG_INC}"
        "-s USE_ZLIB=1 -lz"
        "1")
