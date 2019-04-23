cmake_minimum_required (VERSION 3.6)

project(AssetLoader CXX)

set(INCLUDE 
    #include/GLTFLoader.h
)

set(INTERFACE
    interface/GLTFLoader.h
)

set(SOURCE 
    src/GLTFLoader.cpp
)

add_library(AssetLoader STATIC ${SOURCE} ${INCLUDE} ${INTERFACE})
set_common_target_properties(AssetLoader)

target_include_directories(AssetLoader
PUBLIC
    interface
PRIVATE
    include
)

source_group("source" FILES ${SOURCE})
source_group("include" FILES ${INCLUDE})
source_group("interface" FILES ${INTERFACE})

target_link_libraries(AssetLoader 
PRIVATE 
    BuildSettings 
    Common 
    PlatformInterface 
    GraphicsEngineInterface 
    GraphicsAccessories
    GraphicsTools
    TextureLoader
)

set_target_properties(AssetLoader PROPERTIES
    FOLDER Tools
)
