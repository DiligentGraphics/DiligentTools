cmake_minimum_required (VERSION 3.6)

project(Diligent-RenderStateNotation CXX)

set(REFLECTED
    ${CMAKE_CURRENT_SOURCE_DIR}/../../DiligentCore/Graphics/GraphicsEngine/interface/BlendState.h
    ${CMAKE_CURRENT_SOURCE_DIR}/../../DiligentCore/Graphics/GraphicsEngine/interface/DepthStencilState.h
    ${CMAKE_CURRENT_SOURCE_DIR}/../../DiligentCore/Graphics/GraphicsEngine/interface/GraphicsTypes.h
    ${CMAKE_CURRENT_SOURCE_DIR}/../../DiligentCore/Graphics/GraphicsEngine/interface/InputLayout.h
    ${CMAKE_CURRENT_SOURCE_DIR}/../../DiligentCore/Graphics/GraphicsEngine/interface/PipelineResourceSignature.h
    ${CMAKE_CURRENT_SOURCE_DIR}/../../DiligentCore/Graphics/GraphicsEngine/interface/PipelineState.h
    ${CMAKE_CURRENT_SOURCE_DIR}/../../DiligentCore/Graphics/GraphicsEngine/interface/RasterizerState.h
    ${CMAKE_CURRENT_SOURCE_DIR}/../../DiligentCore/Graphics/GraphicsEngine/interface/RenderPass.h
    ${CMAKE_CURRENT_SOURCE_DIR}/../../DiligentCore/Graphics/GraphicsEngine/interface/Shader.h
    ${CMAKE_CURRENT_SOURCE_DIR}/../../DiligentCore/Graphics/GraphicsEngine/interface/ShaderResourceVariable.h
    ${CMAKE_CURRENT_SOURCE_DIR}/../../DiligentCore/Graphics/GraphicsEngine/interface/Sampler.h
    ${CMAKE_CURRENT_SOURCE_DIR}/../../DiligentCore/Graphics/Archiver/interface/SerializationDevice.h
)

set(RSN_PARSER_HEADERS_DIR ${CMAKE_CURRENT_BINARY_DIR}/parser_headers CACHE INTERNAL "Render State Notation Parser generated headers directory")
set(RSN_PARSER_GENERATED_HEADERS_DIR ${RSN_PARSER_HEADERS_DIR}/generated)
file(MAKE_DIRECTORY "${RSN_PARSER_GENERATED_HEADERS_DIR}")

find_package(PythonInterp 3 REQUIRED)

execute_process(COMMAND ${PYTHON_EXECUTABLE} -m pip install libclang==12.0.0
                RESULT_VARIABLE PYTHON_PIP_LIBCLANG_RESULT)
if(NOT PYTHON_PIP_LIBCLANG_RESULT EQUAL "0")
    message(FATAL_ERROR "python -m pip install libclang failed with ${PYTHON_PIP_LIBCLANG_RESULT}")
endif()

execute_process(COMMAND ${PYTHON_EXECUTABLE} -m pip install jinja2
                RESULT_VARIABLE PYTHON_PIP_JINJIA_RESULT)
if(NOT PYTHON_PIP_JINJIA_RESULT EQUAL "0")
    message(FATAL_ERROR "python -m pip install jinja2 failed with ${PYTHON_PIP_JINJIA_RESULT}")
endif()

file(GLOB INCLUDE    include/*)
file(GLOB INTERFACE  interface/*)
file(GLOB SOURCE     src/*)

foreach(REFLECTED_FILE_PATH ${REFLECTED})
    get_filename_component(REFLECTED_FILE_NAME ${REFLECTED_FILE_PATH} NAME_WE)
    set(GENERATED_FILE_PATH "${RSN_PARSER_GENERATED_HEADERS_DIR}/${REFLECTED_FILE_NAME}Parser.hpp")
    list(APPEND GENERATED ${GENERATED_FILE_PATH})
endforeach(REFLECTED_FILE_PATH)
list(APPEND GENERATED "${RSN_PARSER_GENERATED_HEADERS_DIR}/CommonParser.hpp")
set_source_files_properties(${GENERATED} PROPERTIES GENERATED TRUE)

source_group("include"   FILES ${INCLUDE})
source_group("interface" FILES ${INTERFACE})
source_group("generated" FILES ${GENERATED})
source_group("source"    FILES ${SOURCE})

if(CMAKE_HOST_SYSTEM_NAME STREQUAL "Windows")
    set(CLANG_FORMAT_VERSION "clang-format_10.0.0.exe")
elseif(CMAKE_HOST_SYSTEM_NAME STREQUAL "Linux")
    set(CLANG_FORMAT_VERSION "clang-format_linux_10.0.0")
elseif(CMAKE_HOST_SYSTEM_NAME STREQUAL "Darwin")
    set(CLANG_FORMAT_VERSION "clang-format_mac_10.0.0")
endif()

add_custom_command(OUTPUT ${GENERATED}
                   COMMAND ${PYTHON_EXECUTABLE} "${CMAKE_CURRENT_SOURCE_DIR}/scripts/cxx_generator.py" --dir "." --files ${REFLECTED}
                   COMMAND "${CMAKE_CURRENT_SOURCE_DIR}/../../DiligentCore/BuildTools/FormatValidation/${CLANG_FORMAT_VERSION}" -i *.hpp --verbose
                   WORKING_DIRECTORY "${RSN_PARSER_GENERATED_HEADERS_DIR}"
                   DEPENDS ${REFLECTED}
                   COMMENT "RSN Parser Header Generation")

add_library(Diligent-RenderStateNotation STATIC
    ${INCLUDE}
    ${INTERFACE}
    ${SOURCE}
    ${GENERATED}
)

target_include_directories(Diligent-RenderStateNotation
PUBLIC
    interface
PRIVATE
    include
    ../ThirdParty/json
    ${RSN_PARSER_HEADERS_DIR}
)

target_link_libraries(Diligent-RenderStateNotation PRIVATE
    Diligent-BuildSettings 
    Diligent-Common 
    Diligent-PlatformInterface 
    Diligent-GraphicsEngineInterface 
    Diligent-GraphicsAccessories
    Diligent-GraphicsTools
)

target_compile_definitions(Diligent-RenderStateNotation PUBLIC JSON_DIAGNOSTICS=1)

set_target_properties(Diligent-RenderStateNotation PROPERTIES
    FOLDER DiligentTools
)

set_common_target_properties(Diligent-RenderStateNotation)
