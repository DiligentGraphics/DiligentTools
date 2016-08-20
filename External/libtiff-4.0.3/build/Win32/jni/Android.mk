
# Preamble
DEPENDENCY_PATH := $(call my-dir)
LOCAL_PATH := $(abspath $(DEPENDENCY_PATH))
include $(CLEAR_VARS)

# Project configuration
LOCAL_MODULE := LibTiff
# Force .c files to compile as c++
LOCAL_CFLAGS := -DPLATFORM_ANDROID 
LOCAL_CPP_FEATURES := 
LOCAL_STATIC_LIBRARIES += 

# Include paths
PROJECT_ROOT := $(LOCAL_PATH)/../../../
LOCAL_C_INCLUDES := $(PROJECT_ROOT)/libtiff

# Source files
#VisualGDBAndroid: AutoUpdateSourcesInNextLine
LOCAL_SRC_FILES := ../../../libtiff/tif_aux.c ../../../libtiff/tif_close.c ../../../libtiff/tif_codec.c ../../../libtiff/tif_color.c ../../../libtiff/tif_compress.c ../../../libtiff/tif_dir.c ../../../libtiff/tif_dirinfo.c ../../../libtiff/tif_dirread.c ../../../libtiff/tif_dirwrite.c ../../../libtiff/tif_dumpmode.c ../../../libtiff/tif_error.c ../../../libtiff/tif_extension.c ../../../libtiff/tif_fax3.c ../../../libtiff/tif_fax3sm.c ../../../libtiff/tif_flush.c ../../../libtiff/tif_getimage.c ../../../libtiff/tif_jbig.c ../../../libtiff/tif_jpeg.c ../../../libtiff/tif_jpeg_12.c ../../../libtiff/tif_luv.c ../../../libtiff/tif_lzma.c ../../../libtiff/tif_lzw.c ../../../libtiff/tif_next.c ../../../libtiff/tif_ojpeg.c ../../../libtiff/tif_open.c ../../../libtiff/tif_packbits.c ../../../libtiff/tif_pixarlog.c ../../../libtiff/tif_predict.c ../../../libtiff/tif_print.c ../../../libtiff/tif_read.c ../../../libtiff/tif_stream.cxx ../../../libtiff/tif_strip.c ../../../libtiff/tif_swab.c ../../../libtiff/tif_thunder.c ../../../libtiff/tif_tile.c ../../../libtiff/tif_unix.c ../../../libtiff/tif_version.c ../../../libtiff/tif_warning.c ../../../libtiff/tif_win32.c ../../../libtiff/tif_write.c ../../../libtiff/tif_zip.c ../../../port/lfind.c

#VisualGDBAndroid: VSExcludeListLocation
VISUALGDB_VS_EXCLUDED_FILES_Release :=../../../libtiff/tif_win32.c
VISUALGDB_VS_EXCLUDED_FILES_Debug := ../../../libtiff/tif_win32.c
LOCAL_SRC_FILES := $(filter-out $(VISUALGDB_VS_EXCLUDED_FILES_$(VGDB_VSCONFIG)),$(LOCAL_SRC_FILES))

include $(BUILD_STATIC_LIBRARY)
