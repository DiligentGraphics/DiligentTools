cmake_minimum_required (VERSION 3.6)

project(Diligent-AssetLoader CXX)

set(INCLUDE 
    #include/GLTFLoader.hpp
)

set(INTERFACE
    interface/GLTFLoader.hpp
    interface/GLTFBuilder.hpp
    interface/DXSDKMeshLoader.hpp
    interface/GLTFResourceManager.hpp
)

set(SOURCE 
    src/GLTFLoader.cpp
    src/GLTFBuilder.cpp
    src/DXSDKMeshLoader.cpp
    src/GLTFResourceManager.cpp
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
    Diligent-JSON
)

if (TARGET draco OR TARGET draco_static)
    if(TARGET draco)
        set(DRACO_TARGET draco)
    else()
        set(DRACO_TARGET draco_static)
    endif()
    target_link_libraries(Diligent-AssetLoader PRIVATE ${DRACO_TARGET})
    get_target_property(DRACO_SOURCE_DIR ${DRACO_TARGET} SOURCE_DIR)
    target_compile_definitions(Diligent-AssetLoader PRIVATE TINYGLTF_ENABLE_DRACO)
    target_include_directories(Diligent-AssetLoader PRIVATE "${DRACO_SOURCE_DIR}/src" "${CMAKE_BINARY_DIR}")
elseif (DRACO_PATH)
    find_library(DRACO_LIBRARY NAMES draco draco_static PATHS "${DRACO_PATH}/lib")
    if(DRACO_LIBRARY)
        target_link_libraries(Diligent-AssetLoader PRIVATE ${DRACO_LIBRARY})
        target_include_directories(Diligent-AssetLoader PRIVATE "${DRACO_PATH}/include")
        target_compile_definitions(Diligent-AssetLoader PRIVATE TINYGLTF_ENABLE_DRACO)
    else()
        message(WARNING "Unable to find draco library. Draco support will be disabled")
    endif()
endif()

set_target_properties(Diligent-AssetLoader PROPERTIES
    FOLDER DiligentTools
)

if(DILIGENT_INSTALL_TOOLS)
    install_tools_lib(Diligent-AssetLoader)
endif()
