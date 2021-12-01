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
    ${CMAKE_CURRENT_SOURCE_DIR}/../../DiligentCore/Graphics/GraphicsEngine/interface/Sampler.h
    ${CMAKE_CURRENT_SOURCE_DIR}/../../DiligentCore/Graphics/Archiver/interface/SerializationDevice.h
)

find_package(PythonInterp REQUIRED 3.7.6)
execute_process(COMMAND ${PYTHON_EXECUTABLE} -m pip install libclang)
execute_process(COMMAND ${PYTHON_EXECUTABLE} -m pip install jinja2)
execute_process(COMMAND ${PYTHON_EXECUTABLE} "${CMAKE_CURRENT_SOURCE_DIR}/scripts/cxx_generator.py" --dir "${CMAKE_CURRENT_SOURCE_DIR}/generated" --files ${REFLECTED})

if(PLATFORM_WIN32)
    execute_process(COMMAND "${CMAKE_CURRENT_SOURCE_DIR}/../../DiligentCore/BuildTools/FormatValidation/clang-format_10.0.0.exe" -i *.h *.hpp WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/generated")
elseif(PLATFORM_LINUX)
    execute_process(COMMAND "${CMAKE_CURRENT_SOURCE_DIR}/../../DiligentCore/BuildTools/FormatValidation/clang-format_linux_10.0.0" -i *.h *.hpp WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/generated")
elseif(PLATFORM_MACOS)    
    execute_process(COMMAND "${CMAKE_CURRENT_SOURCE_DIR}/../../DiligentCore/BuildTools/FormatValidation/clang-format_mac_10.0.0" -i *.h *.hpp WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/generated")
else()
    message(FATAL_ERROR "Invalid OS-platform" )
endif()

file(GLOB COMMON_INCLUDE    include/*)
file(GLOB COMMON_INTERFACE  interface/*)
file(GLOB COMMON_SOURCE     src/*)
file(GLOB COMMON_GENERATED  generated/*)

set(INCLUDE   ${COMMON_INCLUDE})
set(INTERFACE ${COMMON_INTERFACE})
set(SOURCE    ${COMMON_SOURCE})
set(GENERATED ${COMMON_GENERATED})

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
)

target_link_libraries(Diligent-RenderStateNotationParser PRIVATE    
    Diligent-BuildSettings 
    Diligent-Common 
    Diligent-PlatformInterface 
    Diligent-GraphicsEngineInterface 
    Diligent-GraphicsAccessories
    Diligent-GraphicsTools
    Diligent-Archiver
)

set_target_properties(Diligent-RenderStateNotationParser PROPERTIES
    FOLDER DiligentTools
)

set_common_target_properties(Diligent-RenderStateNotationParser)
