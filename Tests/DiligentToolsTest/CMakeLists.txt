cmake_minimum_required (VERSION 3.6)

project(DiligentToolsTest)

file(GLOB_RECURSE INCLUDE include/*.*)
file(GLOB_RECURSE SOURCE src/*.*)
file(GLOB_RECURSE RENDER_STATE_NOTATIONS assets/RenderStates/*.*)

set_source_files_properties(${RENDER_STATE_NOTATIONS} PROPERTIES VS_TOOL_OVERRIDE "None")

add_executable(DiligentToolsTest ${SOURCE} ${INCLUDE} ${RENDER_STATE_NOTATIONS})
set_common_target_properties(DiligentToolsTest)

target_link_libraries(DiligentToolsTest 
PRIVATE 
    gtest_main
    Diligent-BuildSettings 
    Diligent-TargetPlatform
    Diligent-TextureLoader
    Diligent-Common
    Diligent-GraphicsEngine
    LibPng
)

target_include_directories(DiligentToolsTest
	PRIVATE
        include
        ../../RenderStateNotationParser/generated
        ../../ThirdParty/json
)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCE} ${INCLUDE} ${RENDER_STATE_NOTATIONS})

set_target_properties(DiligentToolsTest
    PROPERTIES
        VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/assets"
        XCODE_SCHEME_WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/assets"
)

set_target_properties(DiligentToolsTest PROPERTIES
    FOLDER "DiligentTools/Tests"
)
