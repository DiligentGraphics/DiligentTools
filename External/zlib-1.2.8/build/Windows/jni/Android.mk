
# Preamble
DEPENDENCY_PATH := $(call my-dir)
LOCAL_PATH := $(abspath $(DEPENDENCY_PATH))
include $(CLEAR_VARS)

# Project configuration
LOCAL_MODULE := zlib
# Force .c files to compile as c++
LOCAL_CFLAGS := -DPLATFORM_ANDROID 
LOCAL_CPP_FEATURES := 
LOCAL_STATIC_LIBRARIES += 

# Include paths
PROJECT_ROOT := $(LOCAL_PATH)/../../../
LOCAL_C_INCLUDES := $(PROJECT_ROOT)

# Source files
#VisualGDBAndroid: AutoUpdateSourcesInNextLine
LOCAL_SRC_FILES := ../../../adler32.c ../../../compress.c ../../../crc32.c ../../../deflate.c ../../../gzclose.c ../../../gzlib.c ../../../gzread.c ../../../gzwrite.c ../../../infback.c ../../../inffast.c ../../../inflate.c ../../../inftrees.c ../../../trees.c ../../../uncompr.c ../../../zutil.c

include $(BUILD_STATIC_LIBRARY)
