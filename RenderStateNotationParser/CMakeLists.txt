cmake_minimum_required (VERSION 3.6)

project(Diligent-RenderStateNotationParser CXX)

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

execute_process(COMMAND ${PYTHON_EXECUTABLE} -m pip install libclang
                RESULT_VARIABLE PYTHON_PIP_LIBCLANG_RESULT)
if(NOT PYTHON_PIP_LIBCLANG_RESULT EQUAL "0")
    message(FATAL_ERROR "python -m pip install libclang failed with ${PYTHON_PIP_LIBCLANG_RESULT}")
endif()

execute_process(COMMAND ${PYTHON_EXECUTABLE} -m pip install jinja2
                RESULT_VARIABLE PYTHON_PIP_JINJIA_RESULT)
if(NOT PYTHON_PIP_JINJIA_RESULT EQUAL "0")
    message(FATAL_ERROR "python -m pip install jinja2 failed with ${PYTHON_PIP_JINJIA_RESULT}")
endif()

execute_process(COMMAND ${PYTHON_EXECUTABLE} "${CMAKE_CURRENT_SOURCE_DIR}/scripts/cxx_generator.py" --dir "${RSN_PARSER_GENERATED_HEADERS_DIR}" --files ${REFLECTED}
                RESULT_VARIABLE PYTHON_GENERATOR_RESULT)
if(NOT PYTHON_PIP_LIBCLANG_RESULT EQUAL "0")
    message(FATAL_ERROR "python ${CMAKE_CURRENT_SOURCE_DIR}/scripts/cxx_generator.py failed with ${PYTHON_GENERATOR_RESULT}")
endif()

file(GLOB INCLUDE    include/*)
file(GLOB INTERFACE  interface/*)
file(GLOB SOURCE     src/*)
file(GLOB GENERATED  ${RSN_PARSER_GENERATED_HEADERS_DIR}/*)

foreach(FILE ${GENERATED})
    if(CMAKE_HOST_SYSTEM_NAME STREQUAL "Windows")
        execute_process(COMMAND "${CMAKE_CURRENT_SOURCE_DIR}/../../DiligentCore/BuildTools/FormatValidation/clang-format_10.0.0.exe" -i ${FILE} --verbose)
    elseif(CMAKE_HOST_SYSTEM_NAME STREQUAL "Linux")
        execute_process(COMMAND "${CMAKE_CURRENT_SOURCE_DIR}/../../DiligentCore/BuildTools/FormatValidation/clang-format_linux_10.0.0" -i ${FILE} --verbose)
    elseif(CMAKE_HOST_SYSTEM_NAME STREQUAL "Darwin")
        execute_process(COMMAND "${CMAKE_CURRENT_SOURCE_DIR}/../../DiligentCore/BuildTools/FormatValidation/clang-format_mac_10.0.0" -i ${FILE} --verbose)
    endif()
endforeach()

source_group("include"   FILES ${INCLUDE})
source_group("interface" FILES ${INTERFACE})
source_group("generated" FILES ${GENERATED})
source_group("source"    FILES ${SOURCE})

add_library(Diligent-RenderStateNotationParser STATIC
    ${INCLUDE}
    ${INTERFACE}
    ${SOURCE}
    ${GENERATED}
)

target_include_directories(Diligent-RenderStateNotationParser
PUBLIC
    interface
PRIVATE
    include
    ../ThirdParty/json
    ${RSN_PARSER_HEADERS_DIR}
)

target_link_libraries(Diligent-RenderStateNotationParser PRIVATE    
    Diligent-BuildSettings 
    Diligent-Common 
    Diligent-PlatformInterface 
    Diligent-GraphicsEngineInterface 
    Diligent-GraphicsAccessories
    Diligent-GraphicsTools
)

target_compile_definitions(Diligent-RenderStateNotationParser PUBLIC JSON_DIAGNOSTICS=1)

set_target_properties(Diligent-RenderStateNotationParser PROPERTIES
    FOLDER DiligentTools
)

set_common_target_properties(Diligent-RenderStateNotationParser)
