cmake_minimum_required (VERSION 3.6)

project(Diligent-ArchiverJSON CXX)

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
)

find_package(PythonInterp REQUIRED 3.7.6)
execute_process(COMMAND ${PYTHON_EXECUTABLE} -m pip install libclang)
execute_process(COMMAND ${PYTHON_EXECUTABLE} -m pip install jinja2)
execute_process(COMMAND ${PYTHON_EXECUTABLE} "${CMAKE_CURRENT_SOURCE_DIR}/scripts/cxx_generator.py" --dir "${CMAKE_CURRENT_SOURCE_DIR}/include" --files ${REFLECTED})
execute_process(COMMAND "${CMAKE_CURRENT_SOURCE_DIR}/../../DiligentCore/BuildTools/FormatValidation/clang-format_10.0.0.exe" -i *.h *.hpp WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/include")

file(GLOB COMMON_INCLUDE include/*)
file(GLOB COMMON_SOURCE  src/*)

set(INCLUDE ${COMMON_INCLUDE})
set(SOURCE  ${COMMON_SOURCE})

source_group("source"  FILES ${SOURCE})
source_group("include" FILES ${INCLUDE})

add_executable(Diligent-ArchiverJSON 
    ${INCLUDE}
    ${SOURCE}
)

target_include_directories(Diligent-ArchiverJSON
	PRIVATE
    	include
    	../ThirdParty/tinygltf
        ../ThirdParty/json
        ../ThirdParty/argparse
)

target_link_libraries(Diligent-ArchiverJSON PUBLIC Diligent-Archiver)
add_custom_command (TARGET Diligent-ArchiverJSON  POST_BUILD
                    COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:Diligent-Archiver> $<TARGET_FILE_DIR:Diligent-ArchiverJSON>
)

set_target_properties(Diligent-ArchiverJSON PROPERTIES
    FOLDER DiligentTools
)

set_property(TARGET Diligent-ArchiverJSON PROPERTY CXX_STANDARD 17)
set_common_target_properties(Diligent-ArchiverJSON)
