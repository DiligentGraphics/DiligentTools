cmake_minimum_required (VERSION 3.6)

project(DiligentToolsGPUTest)

file(GLOB_RECURSE INCLUDE include/*.*)
file(GLOB_RECURSE SOURCE src/*.*)
set(MAIN_CPP ../../../DiligentCore/Tests/DiligentCoreAPITest/src/main.cpp)
file(GLOB_RECURSE SHADERS assets/Shaders/*.*)
file(GLOB_RECURSE RENDER_STATES assets/RenderStates/*.*)

set_source_files_properties(${RENDER_STATES} PROPERTIES VS_TOOL_OVERRIDE "None")
set_source_files_properties(${SHADERS}       PROPERTIES VS_TOOL_OVERRIDE "None")

add_executable(DiligentToolsGPUTest ${SOURCE} ${MAIN_CPP} ${INCLUDE} ${SHADERS} ${RENDER_STATES})
set_common_target_properties(DiligentToolsGPUTest)

target_link_libraries(DiligentToolsGPUTest 
PRIVATE 
    Diligent-BuildSettings
    Diligent-TargetPlatform
    Diligent-Common
    Diligent-GraphicsEngine
    Diligent-RenderStateNotation
    Diligent-GraphicsTools
    Diligent-GPUTestFramework
)

if (TARGET Diligent-RenderStatePackagerLib)
    target_link_libraries(DiligentToolsGPUTest
	PRIVATE
        Diligent-RenderStatePackagerLib
    )
endif()

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCE} ${INCLUDE} ${SHADERS} ${RENDER_STATES})
source_group("DiligentCoreAPITest" FILES ${MAIN_CPP})

set_target_properties(DiligentToolsGPUTest
    PROPERTIES
        VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/assets"
        XCODE_SCHEME_WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/assets"
)

set_target_properties(DiligentToolsGPUTest PROPERTIES
    FOLDER "DiligentTools/Tests"
)

if(PLATFORM_WIN32)
    copy_required_dlls(DiligentToolsGPUTest)
endif()
