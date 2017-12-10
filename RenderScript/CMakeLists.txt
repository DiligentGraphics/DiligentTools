cmake_minimum_required (VERSION 3.3)

project(RenderScript)

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

target_include_directories(RenderScript 
PUBLIC
	include
)


source_group("source" FILES ${SOURCE})
source_group("include" FILES ${INCLUDE})

target_link_libraries(RenderScript PRIVATE BuildSettings GraphicsEngine Lua Common PlatformInterface GraphicsTools)

if(MSVC)
	# Enable link-time code generation for release builds (I was not able to 
	# find any way to set these settings through interface library BuildSettings)
	set_target_properties(RenderScript PROPERTIES
		STATIC_LIBRARY_FLAGS_RELEASE /LTCG
		STATIC_LIBRARY_FLAGS_MINSIZEREL /LTCG
		STATIC_LIBRARY_FLAGS_RELWITHDEBINFO /LTCG
	)
endif()

set_target_properties(RenderScript PROPERTIES
	FOLDER Tools
)