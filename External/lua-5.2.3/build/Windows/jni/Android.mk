
# Preamble
DEPENDENCY_PATH := $(call my-dir)
LOCAL_PATH := $(abspath $(DEPENDENCY_PATH))
include $(CLEAR_VARS)

# Project configuration
LOCAL_MODULE := Lua
# Force .c files to compile as c++
LOCAL_CFLAGS := -x c++
LOCAL_CPP_FEATURES := 
LOCAL_STATIC_LIBRARIES += 

# Include paths
LOCAL_C_INCLUDES := $(LOCAL_PATH)/../../../src

# Source files
#VisualGDBAndroid: AutoUpdateSourcesInNextLine
LOCAL_SRC_FILES := ../../../src/lapi.c ../../../src/lauxlib.c ../../../src/lbaselib.c ../../../src/lbitlib.c ../../../src/lcode.c ../../../src/lcorolib.c ../../../src/lctype.c ../../../src/ldblib.c ../../../src/ldebug.c ../../../src/ldo.c ../../../src/ldump.c ../../../src/lfunc.c ../../../src/lgc.c ../../../src/linit.c ../../../src/liolib.c ../../../src/llex.c ../../../src/lmathlib.c ../../../src/lmem.c ../../../src/loadlib.c ../../../src/lobject.c ../../../src/lopcodes.c ../../../src/loslib.c ../../../src/lparser.c ../../../src/lstate.c ../../../src/lstring.c ../../../src/lstrlib.c ../../../src/ltable.c ../../../src/ltablib.c ../../../src/ltm.c ../../../src/lundump.c ../../../src/lvm.c ../../../src/lzio.c

include $(BUILD_STATIC_LIBRARY)
