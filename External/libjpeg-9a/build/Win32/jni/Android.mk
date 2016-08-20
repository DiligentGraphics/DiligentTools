
# Preamble
DEPENDENCY_PATH := $(call my-dir)
LOCAL_PATH := $(abspath $(DEPENDENCY_PATH))
include $(CLEAR_VARS)

# Project configuration
LOCAL_MODULE := LibJpeg
# Force .c files to compile as c++
LOCAL_CFLAGS := -DPLATFORM_ANDROID 
LOCAL_CPP_FEATURES := 
LOCAL_STATIC_LIBRARIES += 

# Include paths
PROJECT_ROOT := $(LOCAL_PATH)/../../../
LOCAL_C_INCLUDES := $(PROJECT_ROOT)

# Source files
#VisualGDBAndroid: AutoUpdateSourcesInNextLine
LOCAL_SRC_FILES := ../../../jaricom.c ../../../jcapimin.c ../../../jcapistd.c ../../../jcarith.c ../../../jccoefct.c ../../../jccolor.c ../../../jcdctmgr.c ../../../jchuff.c ../../../jcinit.c ../../../jcmainct.c ../../../jcmarker.c ../../../jcmaster.c ../../../jcomapi.c ../../../jcparam.c ../../../jcprepct.c ../../../jcsample.c ../../../jctrans.c ../../../jdapimin.c ../../../jdapistd.c ../../../jdarith.c ../../../jdatadst.c ../../../jdatasrc.c ../../../jdcoefct.c ../../../jdcolor.c ../../../jddctmgr.c ../../../jdhuff.c ../../../jdinput.c ../../../jdmainct.c ../../../jdmarker.c ../../../jdmaster.c ../../../jdmerge.c ../../../jdpostct.c ../../../jdsample.c ../../../jdtrans.c ../../../jerror.c ../../../jfdctflt.c ../../../jfdctfst.c ../../../jfdctint.c ../../../jidctflt.c ../../../jidctfst.c ../../../jidctint.c ../../../jmemmgr.c ../../../jmemnobs.c ../../../jquant1.c ../../../jquant2.c ../../../jutils.c

include $(BUILD_STATIC_LIBRARY)
