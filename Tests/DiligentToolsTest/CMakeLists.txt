cmake_minimum_required (VERSION 3.6)

project(DiligentToolsTest)

file(GLOB_RECURSE INCLUDE include/*.*)
file(GLOB_RECURSE SOURCE src/*.*)
set(MAIN_CPP ../../../DiligentCore/Tests/DiligentCoreTest/src/main.cpp)
file(GLOB_RECURSE SHADERS assets/Shaders/*.*)
file(GLOB_RECURSE RENDER_STATES assets/RenderStates/*.*)
file(GLOB_RECURSE PACKAGER_CONFIG assets/RenderStatePackagerConfig.json)

if (NOT ARCHIVER_SUPPORTED)
    list(REMOVE_ITEM SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/src/RenderStatePackager/RenderStatePackagerTest.cpp)
endif()

set_source_files_properties(${RENDER_STATES}   PROPERTIES VS_TOOL_OVERRIDE "None")
set_source_files_properties(${SHADERS}         PROPERTIES VS_TOOL_OVERRIDE "None")
set_source_files_properties(${PACKAGER_CONFIG} PROPERTIES VS_TOOL_OVERRIDE "None")

set_source_files_properties(${RENDER_STATES} PROPERTIES VS_TOOL_OVERRIDE "None")
set_source_files_properties(${SHADERS}       PROPERTIES VS_TOOL_OVERRIDE "None")

add_executable(DiligentToolsTest ${SOURCE} ${MAIN_CPP} ${INCLUDE} ${SHADERS} ${RENDER_STATES} ${PACKAGER_CONFIG})
set_common_target_properties(DiligentToolsTest)

target_link_libraries(DiligentToolsTest 
PRIVATE 
    Diligent-BuildSettings 
    Diligent-TargetPlatform
    Diligent-TextureLoader
    Diligent-Common
    Diligent-GraphicsEngine
    Diligent-RenderStateNotation
    Diligent-TestFramework
    LibPng
)

target_include_directories(DiligentToolsTest
	PRIVATE
        include
        ${RSN_PARSER_HEADERS_DIR}
        ../../ThirdParty/json
)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCE} ${INCLUDE} ${SHADERS} ${RENDER_STATES} ${PACKAGER_CONFIG})
source_group("DiligentCoreTest" FILES ${MAIN_CPP})

set_target_properties(DiligentToolsTest
    PROPERTIES
        VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/assets"
        XCODE_SCHEME_WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/assets"
)

set_target_properties(DiligentToolsTest PROPERTIES
    FOLDER "DiligentTools/Tests"
)

if (ARCHIVER_SUPPORTED)
    target_include_directories(DiligentToolsTest
	    PRIVATE
            ../../RenderStatePackager/include
    )

    target_sources(DiligentToolsTest 
        PRIVATE 
            ../../RenderStatePackager/src/ParsingEnvironment.cpp  
            ../../RenderStatePackager/src/RenderStatePackager.cpp
    )

    target_link_libraries(DiligentToolsTest 
        PRIVATE 
            Diligent-Archiver-static
    )
endif()
