cmake_minimum_required (VERSION 3.6)

option(DILIGENT_BUILD_RENDER_SCRIPT "Build render script" ON)

add_subdirectory(ThirdParty)
add_subdirectory(TextureLoader)
add_subdirectory(AssetLoader)
add_subdirectory(Imgui)
add_subdirectory(NativeApp)

if(${DILIGENT_BUILD_RENDER_SCRIPT})
    add_subdirectory(RenderScript)
endif()

if(PLATFORM_WIN32 AND GL_SUPPORTED)
    add_subdirectory(HLSL2GLSLConverter)
endif()
