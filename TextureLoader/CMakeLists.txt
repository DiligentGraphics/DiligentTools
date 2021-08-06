cmake_minimum_required (VERSION 3.6)

project(Diligent-TextureLoader CXX)

set(INCLUDE 
    include/dxgiformat.h
    include/pch.h
    include/TextureLoaderImpl.hpp
)

set(INTERFACE
    interface/JPEGCodec.h
    interface/PNGCodec.h
    interface/BCTools.h
    interface/Image.h
    interface/TextureLoader.h
    interface/TextureUtilities.h
)

set(SOURCE 
    src/BCTools.cpp
    src/DDSLoader.cpp
    src/JPEGCodec.c
    src/Image.cpp
    src/KTXLoader.cpp
    src/PNGCodec.c
    src/STBImpl.cpp
    src/TextureLoaderImpl.cpp
    src/TextureUtilities.cpp
)

add_library(Diligent-TextureLoader STATIC ${SOURCE} ${INCLUDE} ${INTERFACE})
set_common_target_properties(Diligent-TextureLoader)

target_include_directories(Diligent-TextureLoader
PUBLIC
    interface
PRIVATE
    include
)

source_group("source" FILES ${SOURCE})
source_group("include" FILES ${INCLUDE})
source_group("interface" FILES ${INTERFACE})

target_link_libraries(Diligent-TextureLoader 
PRIVATE 
    Diligent-BuildSettings 
    Diligent-Common 
    Diligent-PlatformInterface 
    Diligent-GraphicsEngineInterface 
    Diligent-GraphicsAccessories
    Diligent-GraphicsTools
    LibJpeg 
    LibPng 
    LibTiff 
    ZLib
)

set_target_properties(Diligent-TextureLoader PROPERTIES
    FOLDER DiligentTools
)

if(DILIGENT_INSTALL_TOOLS)
    install_tools_lib(Diligent-TextureLoader)
endif()
