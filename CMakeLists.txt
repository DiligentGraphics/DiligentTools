cmake_minimum_required (VERSION 3.6)

option(DILIGENT_BUILD_RENDER_SCRIPT "Build render script" ON)

add_subdirectory(External)
add_subdirectory(TextureLoader)

if(${DILIGENT_BUILD_RENDER_SCRIPT})
    add_subdirectory(RenderScript)
endif()

if(PLATFORM_WIN32 AND GL_SUPPORTED)
    add_subdirectory(HLSL2GLSLConverter)
endif()
