cmake_minimum_required (VERSION 3.6)

project(RenderScript CXX)

set(INCLUDE 
    include/BlendStateDescParser.h
    include/BufferParser.h
    include/BufferViewParser.h
    include/ClassMethodBinding.h
    include/ConvenienceFunctions.h
    include/Debug.h
    include/DepthStencilStateDescParser.h
    include/DeviceContextFuncBindings.h
    include/DrawAttribsParser.h
    include/EngineObjectParserBase.h
    include/EngineObjectParserCommon.h
    include/EnumMappings.h
    include/InputLayoutDescParser.h
    include/LuaBindings.h
    include/LuaFunctionBinding.h
    include/LuaWrappers.h
    include/ParsingErrors.h
    include/pch.h
    include/PSODescParser.h
    include/RasterizerStateDescParser.h
    include/ResourceMappingParser.h
    include/SamplerParser.h
    include/ScissorRectParser.h
    include/ScriptParser.h
    include/ShaderParser.h
    include/ShaderResourceBindingParser.h
    include/ShaderVariableParser.h
    include/TextureParser.h
    include/TextureViewParser.h
    include/ViewportParser.h
)

set(SOURCE 
    src/BlendStateDescParser.cpp
    src/BufferParser.cpp
    src/BufferViewParser.cpp
    src/DepthStencilStateDescParser.cpp
    src/DeviceContextFuncBindings.cpp
    src/DrawAttribsParser.cpp
    src/EngineObjectParserBase.cpp
    src/EnumMappings.cpp
    src/InputLayoutDescParser.cpp
    src/LuaBindings.cpp
    src/LuaFunctionBinding.cpp
    src/LuaWrappers.cpp
    src/ParsingErrors.cpp
    src/PSODescParser.cpp
    src/RasterizerStateDescParser.cpp
    src/ResourceMappingParser.cpp
    src/SamplerParser.cpp
    src/ScissorRectParser.cpp
    src/ScriptParser.cpp
    src/ShaderParser.cpp
    src/ShaderResourceBindingParser.cpp
    src/ShaderVariableParser.cpp
    src/TextureParser.cpp
    src/TextureViewParser.cpp
    src/ViewportParser.cpp
)

add_library(RenderScript STATIC ${SOURCE} ${INCLUDE})
set_common_target_properties(RenderScript)

target_include_directories(RenderScript 
PUBLIC
    include
)

if (CMAKE_CXX_COMPILER_ID MATCHES "Clang" OR 
    CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    # Suppress warnings from applying the ‘offsetof’ macro to a non-POD type
    #	warning: offset of on non-standard-layout type [-Winvalid-offsetof]
    # This is OK because the warning is generated for simple structs (such as TextureDesc) 
    # that fail to be a POD type only by virtue of having a constructor and comparison operator
    target_compile_options(RenderScript PRIVATE -Wno-invalid-offsetof)
endif()

source_group("source" FILES ${SOURCE})
source_group("include" FILES ${INCLUDE})

target_link_libraries(RenderScript 
PUBLIC
    Lua 
PRIVATE 
    BuildSettings 
    GraphicsEngineInterface
    GraphicsAccessories
    Common 
    PlatformInterface 
    GraphicsTools
)

set_target_properties(RenderScript PROPERTIES
    FOLDER Tools
)