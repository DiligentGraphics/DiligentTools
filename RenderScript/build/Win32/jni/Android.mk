# Preamble
DEPENDENCY_PATH := $(call my-dir)
LOCAL_PATH := $(abspath $(DEPENDENCY_PATH))
include $(CLEAR_VARS)


# Project configuration
LOCAL_MODULE := RenderScript
LOCAL_CFLAGS := -std=c++11 -Wno-invalid-offsetof
LOCAL_CPP_FEATURES := exceptions rtti
LOCAL_STATIC_LIBRARIES += 

# Include paths
PROJECT_ROOT = $(LOCAL_PATH)/../../..
REPO_ROOT := $(PROJECT_ROOT)/..
ENGINE_ROOT := $(REPO_ROOT)/..
CORE_ROOT := $(ENGINE_ROOT)/diligentcore
GRAPHICS_ROOT := $(CORE_ROOT)/graphics
LOCAL_C_INCLUDES += $(PROJECT_ROOT)/include 
LOCAL_C_INCLUDES += $(GRAPHICS_ROOT)/GraphicsEngine/interface 
LOCAL_C_INCLUDES += $(CORE_ROOT)/Common/include 
LOCAL_C_INCLUDES += $(CORE_ROOT)/Common/interface 
LOCAL_C_INCLUDES += $(REPO_ROOT)/External/lua-5.2.3/src 
LOCAL_C_INCLUDES += $(GRAPHICS_ROOT)/GraphicsTools/include 
LOCAL_C_INCLUDES += $(CORE_ROOT)/Platforms/interface

# Source files
#VisualGDBAndroid: AutoUpdateSourcesInNextLine
LOCAL_SRC_FILES := ../../../src/BlendStateDescParser.cpp ../../../src/BufferParser.cpp ../../../src/BufferViewParser.cpp ../../../src/DepthStencilStateDescParser.cpp ../../../src/DeviceContextFuncBindings.cpp ../../../src/DrawAttribsParser.cpp ../../../src/EngineObjectParserBase.cpp ../../../src/EnumMappings.cpp ../../../src/InputLayoutDescParser.cpp ../../../src/LuaBindings.cpp ../../../src/LuaFunctionBinding.cpp ../../../src/LuaWrappers.cpp ../../../src/ParsingErrors.cpp ../../../src/PSODescParser.cpp ../../../src/RasterizerStateDescParser.cpp ../../../src/ResourceMappingParser.cpp ../../../src/SamplerParser.cpp ../../../src/ScissorRectParser.cpp ../../../src/ScriptParser.cpp ../../../src/ShaderParser.cpp ../../../src/ShaderResourceBindingParser.cpp ../../../src/ShaderVariableParser.cpp ../../../src/TextureParser.cpp ../../../src/TextureViewParser.cpp ../../../src/ViewportParser.cpp

include $(BUILD_STATIC_LIBRARY)
