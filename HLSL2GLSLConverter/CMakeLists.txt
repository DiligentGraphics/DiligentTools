cmake_minimum_required (VERSION 3.6)

project(HLSL2GLSLConverter CXX)

set(SOURCE 
    src/HLSL2GLSLConverterApp.cpp
)

if(PLATFORM_WIN32)
    add_executable(HLSL2GLSLConverter ${SOURCE})
else()
    message(FATAL_ERROR "Unsupported platform")
endif()

set_common_target_properties(HLSL2GLSLConverter)

target_include_directories(HLSL2GLSLConverter PRIVATE
    ../../DiligentCore/Graphics/HLSL2GLSLConverterLib/include
)

target_compile_definitions(HLSL2GLSLConverter PRIVATE UNICODE)

target_link_libraries(HLSL2GLSLConverter 
PRIVATE 
    Diligent-BuildSettings 
    Diligent-TargetPlatform
    Diligent-Common
    Diligent-GraphicsTools
    Diligent-HLSL2GLSLConverterLib 
    Diligent-GraphicsEngineOpenGL-static
)

source_group("source" FILES ${SOURCE})

set_target_properties(HLSL2GLSLConverter PROPERTIES
    FOLDER DiligentTools
)

if(DILIGENT_INSTALL_TOOLS)
    install(TARGETS HLSL2GLSLConverter
            ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}/${DILIGENT_TOOLS_DIR}/$<CONFIG>"
            LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}/${DILIGENT_TOOLS_DIR}/$<CONFIG>"
            RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}/${DILIGENT_TOOLS_DIR}/$<CONFIG>"
    )
endif()
