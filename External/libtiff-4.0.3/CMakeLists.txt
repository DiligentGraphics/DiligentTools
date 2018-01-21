cmake_minimum_required (VERSION 3.6)

project(LibTiff C)

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

if(PLATFORM_WIN32)
	list(APPEND SOURCE libtiff/tif_win32.c)
elseif(PLATFORM_ANDROID OR PLATFORM_UNIVERSAL_WINDOWS OR PLATFORM_LINUX OR PLATFORM_MACOS)
	list(APPEND SOURCE libtiff/tif_unix.c)
else()
	message(FATAL_ERROR "Unknown platform")
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
	libtiff/tiffconf.linux.h
	libtiff/tiffconf.h
	libtiff/tiffconf.vc.h
	libtiff/tiffconf.wince.h
	libtiff/tiffio.h
	libtiff/tiffio.hxx
	libtiff/tiffiop.h
	libtiff/tiffvers.h
	libtiff/uvcode.h
)

add_library(LibTiff STATIC ${SOURCE} ${INCLUDE})
set_common_target_properties(LibTiff)

if(MSVC)
	# Disable MSVC-specific warnings
	target_compile_definitions(LibTiff PRIVATE -D_CRT_SECURE_NO_WARNINGS)
	target_compile_options(LibTiff PRIVATE /wd4244 /wd4456 /wd4457 /wd4706)

	if(PLATFORM_UNIVERSAL_WINDOWS)
		target_compile_definitions(LibTiff PRIVATE -D_CRT_NONSTDC_NO_WARNINGS)
		target_compile_options(LibTiff PRIVATE /wd4311 /wd4312)
	endif()
endif(MSVC)

if (CMAKE_CXX_COMPILER_ID MATCHES "GNU" OR
    CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    # Suppress the following warnings in tif_unix.c
	#   cast from pointer to integer of different size [-Wpointer-to-int-cast]
	# and
	#   cast to pointer from integer of different size [-Wint-to-pointer-cast]
	set_source_files_properties(
		libtiff/tif_unix.c
		PROPERTIES 
		COMPILE_FLAGS "-Wno-pointer-to-int-cast -Wno-int-to-pointer-cast"
	)
endif()

target_link_libraries(LibTiff PRIVATE BuildSettings)

target_include_directories(LibTiff 
	PUBLIC libtiff
)

source_group("src" FILES ${SOURCE})
source_group("include" FILES ${INCLUDE})

set_target_properties(LibTiff PROPERTIES
	FOLDER Tools/External
)
