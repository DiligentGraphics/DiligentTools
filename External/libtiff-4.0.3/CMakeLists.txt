cmake_minimum_required (VERSION 3.3)

project(LibTiff C)

if(WIN32)
	add_definitions(-DPLATFORM_WINDOWS -D_CRT_SECURE_NO_WARNINGS)
elseif(ANDROID)
	add_definitions(-DPLATFORM_ANDROID)
endif()


set(SOURCE 
#	libtiff/mkg3states.c
	libtiff/tif_aux.c
	libtiff/tif_close.c
	libtiff/tif_codec.c
	libtiff/tif_color.c
	libtiff/tif_compress.c
	libtiff/tif_dir.c
	libtiff/tif_dirinfo.c
	libtiff/tif_dirread.c
	libtiff/tif_dirwrite.c
	libtiff/tif_dumpmode.c
	libtiff/tif_error.c
	libtiff/tif_extension.c
	libtiff/tif_fax3.c
	libtiff/tif_fax3sm.c
	libtiff/tif_flush.c
	libtiff/tif_getimage.c
	libtiff/tif_jbig.c
	libtiff/tif_jpeg.c
	libtiff/tif_jpeg_12.c
	libtiff/tif_luv.c
	libtiff/tif_lzma.c
	libtiff/tif_lzw.c
	libtiff/tif_next.c
	libtiff/tif_ojpeg.c
	libtiff/tif_open.c
	libtiff/tif_packbits.c
	libtiff/tif_pixarlog.c
	libtiff/tif_predict.c
	libtiff/tif_print.c
	libtiff/tif_read.c
	libtiff/tif_stream.cxx
	libtiff/tif_strip.c
	libtiff/tif_swab.c
	libtiff/tif_thunder.c
	libtiff/tif_tile.c
	libtiff/tif_version.c
	libtiff/tif_warning.c
	libtiff/tif_write.c
	libtiff/tif_zip.c
)

if(WIN32)
	list(APPEND SOURCE libtiff/tif_win32.c)
elseif(ANDROID)
	list(APPEND SOURCE libtiff/tif_unix.c)
endif()

set(INCLUDE 
	libtiff/t4.h
	libtiff/tif_config.android.h
	libtiff/tif_config.h
	libtiff/tif_config.h-vms
	libtiff/tif_config.vc.h
	libtiff/tif_config.wince.h
	libtiff/tif_dir.h
	libtiff/tif_fax3.h
	libtiff/tif_predict.h
	libtiff/tiff.h
	libtiff/tiffconf.android.h
	libtiff/tiffconf.h
	libtiff/tiffconf.vc.h
	libtiff/tiffconf.wince.h
	libtiff/tiffio.h
	libtiff/tiffio.hxx
	libtiff/tiffiop.h
	libtiff/tiffvers.h
	libtiff/uvcode.h
)

include_directories(
	libtiff
)

add_library(
	LibTiff ${SOURCE} ${INCLUDE}
)

source_group("source" FILES ${SOURCE})
source_group("include" FILES ${INCLUDE})
