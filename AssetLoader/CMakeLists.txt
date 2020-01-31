cmake_minimum_required (VERSION 3.6)

project(Diligent-AssetLoader CXX)

set(INCLUDE 
    #include/GLTFLoader.hpp
)

set(INTERFACE
    interface/GLTFLoader.hpp
    interface/DXSDKMeshLoader.hpp
)

set(SOURCE 
    src/GLTFLoader.cpp
    src/DXSDKMeshLoader.cpp
)

add_library(Diligent-AssetLoader STATIC ${SOURCE} ${INCLUDE} ${INTERFACE})
set_common_target_properties(Diligent-AssetLoader)

target_include_directories(Diligent-AssetLoader
PUBLIC
    interface
PRIVATE
    include
)

source_group("source" FILES ${SOURCE})
source_group("include" FILES ${INCLUDE})
source_group("interface" FILES ${INTERFACE})

target_link_libraries(Diligent-AssetLoader 
PRIVATE 
    Diligent-BuildSettings 
    Diligent-Common 
    Diligent-PlatformInterface 
    Diligent-GraphicsEngineInterface 
    Diligent-GraphicsAccessories
    Diligent-GraphicsTools
    Diligent-TextureLoader
)

set_target_properties(Diligent-AssetLoader PROPERTIES
    FOLDER DiligentTools
)

if(DILIGENT_INSTALL_TOOLS)
    install_tools_lib(Diligent-AssetLoader)
endif()
