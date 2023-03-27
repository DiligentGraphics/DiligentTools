cmake_minimum_required (VERSION 3.6)

project(DiligentToolsTest)

file(GLOB_RECURSE INCLUDE include/*.*)
file(GLOB_RECURSE SOURCE src/*.*)
set(MAIN_CPP ../../../DiligentCore/Tests/DiligentCoreTest/src/main.cpp)
file(GLOB_RECURSE ASSETS assets/*.*)

if (NOT ARCHIVER_SUPPORTED)
    list(REMOVE_ITEM SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/src/RenderStatePackager/RenderStatePackagerTest.cpp)
endif()

set_property(SOURCE src/PNGCodecTest.cpp
APPEND PROPERTY INCLUDE_DIRECTORIES
    "${CMAKE_CURRENT_SOURCE_DIR}/../../ThirdParty/libpng" # png_static target does not define any public include directories
    "${CMAKE_CURRENT_BINARY_DIR}/../../ThirdParty/libpng" # pnglibconf.h is generated in the binary directory
)

set_source_files_properties(${ASSETS} PROPERTIES VS_TOOL_OVERRIDE "None")

add_executable(DiligentToolsTest ${SOURCE} ${MAIN_CPP} ${INCLUDE} ${ASSETS})
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
    PNG::PNG
    Diligent-JSON
)

target_include_directories(DiligentToolsTest
    PRIVATE
        include
        ${RSN_PARSER_HEADERS_DIR}
)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCE} ${INCLUDE} ${ASSETS})
source_group("DiligentCoreTest" FILES ${MAIN_CPP})

set_target_properties(DiligentToolsTest
    PROPERTIES
        VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/assets"
        XCODE_SCHEME_WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/assets"
)

set_target_properties(DiligentToolsTest PROPERTIES
    FOLDER "DiligentTools/Tests"
)

if (TARGET Diligent-RenderStatePackagerLib)
    target_link_libraries(DiligentToolsTest
    PRIVATE
        Diligent-RenderStatePackagerLib
    )
endif()
