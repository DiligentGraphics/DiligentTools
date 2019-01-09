cmake_minimum_required (VERSION 3.6)

add_subdirectory(External)
add_subdirectory(RenderScript)
add_subdirectory(TextureLoader)
if(PLATFORM_WIN32 AND GL_SUPPORTED)
    add_subdirectory(HLSL2GLSLConverter)
endif()