
# Preamble
DEPENDENCY_PATH := $(call my-dir)
LOCAL_PATH := $(abspath $(DEPENDENCY_PATH))
include $(CLEAR_VARS)

# Project configuration
LOCAL_MODULE := LibPng
# Force .c files to compile as c++
LOCAL_CFLAGS := -DPLATFORM_ANDROID 
LOCAL_CPP_FEATURES := 
LOCAL_STATIC_LIBRARIES += 

# Include paths
PROJECT_ROOT := $(LOCAL_PATH)/../../../
LOCAL_C_INCLUDES := $(PROJECT_ROOT) $(PROJECT_ROOT)/../zlib-1.2.8

# Source files
#VisualGDBAndroid: AutoUpdateSourcesInNextLine
LOCAL_SRC_FILES := ../../../png.c ../../../pngerror.c ../../../pngget.c ../../../pngmem.c ../../../pngpread.c ../../../pngread.c ../../../pngrio.c ../../../pngrtran.c ../../../pngrutil.c ../../../pngset.c ../../../pngtrans.c ../../../pngwio.c ../../../pngwrite.c ../../../pngwtran.c ../../../pngwutil.c

include $(BUILD_STATIC_LIBRARY)
