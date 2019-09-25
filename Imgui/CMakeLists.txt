cmake_minimum_required (VERSION 3.6)

project(Diligent-Imgui CXX)

set(DEAR_IMGUI_PATH ../ThirdParty/imgui)
set(DEAR_IMGUI_INCLUDE 
    ${DEAR_IMGUI_PATH}/imconfig.h
    ${DEAR_IMGUI_PATH}/imgui_internal.h
    ${DEAR_IMGUI_PATH}/imstb_rectpack.h
    ${DEAR_IMGUI_PATH}/imstb_textedit.h
    ${DEAR_IMGUI_PATH}/imstb_truetype.h
)

set(DEAR_IMGUI_INTERFACE
    ${DEAR_IMGUI_PATH}/imgui.h
)

set(DEAR_IMGUI_SOURCE 
    ${DEAR_IMGUI_PATH}/imgui.cpp
    ${DEAR_IMGUI_PATH}/imgui_draw.cpp
    ${DEAR_IMGUI_PATH}/imgui_widgets.cpp
)

if(PLATFORM_WIN32)
    list(APPEND DEAR_IMGUI_SOURCE ${DEAR_IMGUI_PATH}/examples/imgui_impl_win32.cpp)
    list(APPEND DEAR_IMGUI_INCLUDE ${DEAR_IMGUI_PATH}/examples/imgui_impl_win32.h)
endif()

set(SOURCE
    src/ImGuiImplDiligent.cpp
    src/ImGuiImplWin32.cpp
)

set(INTERFACE
    interface/ImGuiImplDiligent.h
    interface/ImGuiImplWin32.h
)

add_library(Diligent-Imgui STATIC ${SOURCE} ${INCLUDE} ${INTERFACE} ${DEAR_IMGUI_SOURCE} ${DEAR_IMGUI_INCLUDE} ${DEAR_IMGUI_INTERFACE})
set_common_target_properties(Diligent-Imgui)

target_include_directories(Diligent-Imgui
PUBLIC
    interface
    ${DEAR_IMGUI_PATH}
PRIVATE
    include
)

source_group("dear_imgui" FILES ${DEAR_IMGUI_SOURCE} ${DEAR_IMGUI_INCLUDE} ${DEAR_IMGUI_INTERFACE})
source_group("src" FILES ${SOURCE})
source_group("include" FILES ${INCLUDE})
source_group("interface" FILES ${INTERFACE})

target_link_libraries(Diligent-Imgui 
PRIVATE 
    Diligent-BuildSettings 
    Diligent-Common 
    Diligent-PlatformInterface 
    Diligent-GraphicsEngineInterface 
    Diligent-GraphicsAccessories
)

set_target_properties(Diligent-Imgui PROPERTIES
    FOLDER Tools
)
