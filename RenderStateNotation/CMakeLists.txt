cmake_minimum_required (VERSION 3.6)

project(Diligent-RenderStateNotation CXX)

set(REFLECTED
    ${Diligent-GraphicsEngine_SOURCE_DIR}/interface/BlendState.h
    ${Diligent-GraphicsEngine_SOURCE_DIR}/interface/DepthStencilState.h
    ${Diligent-GraphicsEngine_SOURCE_DIR}/interface/GraphicsTypes.h
    ${Diligent-GraphicsEngine_SOURCE_DIR}/interface/InputLayout.h
    ${Diligent-GraphicsEngine_SOURCE_DIR}/interface/PipelineResourceSignature.h
    ${Diligent-GraphicsEngine_SOURCE_DIR}/interface/PipelineState.h
    ${Diligent-GraphicsEngine_SOURCE_DIR}/interface/RasterizerState.h
    ${Diligent-GraphicsEngine_SOURCE_DIR}/interface/RenderPass.h
    ${Diligent-GraphicsEngine_SOURCE_DIR}/interface/Shader.h
    ${Diligent-GraphicsEngine_SOURCE_DIR}/interface/ShaderResourceVariable.h
    ${Diligent-GraphicsEngine_SOURCE_DIR}/interface/Sampler.h
    ${Diligent-GraphicsEngine_SOURCE_DIR}/../Archiver/interface/ArchiverFactory.h
)

set(RSN_PARSER_HEADERS_DIR ${CMAKE_CURRENT_BINARY_DIR}/parser_headers CACHE INTERNAL "Render State Notation Parser generated headers directory")
set(RSN_PARSER_GENERATED_HEADERS_DIR ${RSN_PARSER_HEADERS_DIR}/generated)
file(MAKE_DIRECTORY "${RSN_PARSER_GENERATED_HEADERS_DIR}")

find_package(Python3 REQUIRED)

execute_process(COMMAND ${Python3_EXECUTABLE} -m pip install libclang==15.0.6.1 # Fails with 16.0.0
                RESULT_VARIABLE PYTHON_PIP_LIBCLANG_RESULT)
if(NOT PYTHON_PIP_LIBCLANG_RESULT EQUAL "0")
    message(FATAL_ERROR "${Python3_EXECUTABLE} -m pip install libclang failed with ${PYTHON_PIP_LIBCLANG_RESULT}")
endif()

execute_process(COMMAND ${Python3_EXECUTABLE} -m pip install jinja2
                RESULT_VARIABLE PYTHON_PIP_JINJIA_RESULT)
if(NOT PYTHON_PIP_JINJIA_RESULT EQUAL "0")
    message(FATAL_ERROR "python -m pip install jinja2 failed with ${PYTHON_PIP_JINJIA_RESULT}")
endif()

file(GLOB INCLUDE    include/*)
file(GLOB INTERFACE  interface/*)
file(GLOB SOURCE     src/*)
file(GLOB SCRIPTS    scripts/*.py)

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
source_group("scripts"   FILES ${SCRIPTS})

if (EXISTS "${CLANG_FORMAT_EXECUTABLE}")
    set(FORMATTING_COMMAND "${CLANG_FORMAT_EXECUTABLE}" -i *.hpp --verbose)
else()
    set(FORMATTING_COMMAND "${CMAKE_COMMAND}" -E echo "clang-format executable is not found: RSN parser headers will not be formatted")
endif()

add_custom_command(OUTPUT ${GENERATED}
                   COMMAND ${Python3_EXECUTABLE} "${CMAKE_CURRENT_SOURCE_DIR}/scripts/cxx_generator.py" --dir "." --files ${REFLECTED}
                   COMMAND ${FORMATTING_COMMAND}
                   WORKING_DIRECTORY "${RSN_PARSER_GENERATED_HEADERS_DIR}"
                   DEPENDS ${REFLECTED}
                   COMMENT "Generating RSN Parser Headers...")

add_library(Diligent-RenderStateNotation STATIC
    ${INCLUDE}
    ${INTERFACE}
    ${SOURCE}
    ${GENERATED}
    ${SCRIPTS}
)

target_include_directories(Diligent-RenderStateNotation
PUBLIC
    interface
PRIVATE
    include
    ../../DiligentCore/Graphics/Archiver/interface
    ${RSN_PARSER_HEADERS_DIR}
)

target_link_libraries(Diligent-RenderStateNotation
PRIVATE
    Diligent-BuildSettings 
    Diligent-Common 
    Diligent-PlatformInterface 
    Diligent-GraphicsEngineInterface 
    Diligent-GraphicsAccessories
    Diligent-GraphicsTools
    Diligent-JSON
)

set_target_properties(Diligent-RenderStateNotation PROPERTIES
    FOLDER DiligentTools
)

set_common_target_properties(Diligent-RenderStateNotation)

if(DILIGENT_INSTALL_TOOLS)
    install_tools_lib(Diligent-RenderStateNotation)
endif()
