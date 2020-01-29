cmake_minimum_required (VERSION 3.6)

project(Diligent-Imgui CXX)

set(DEAR_IMGUI_PATH ../ThirdParty/imgui)
set(IMGUIZMO_QUAT_PATH ../ThirdParty/imGuIZMO.quat)

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

set(SOURCE
    src/ImGuiImplDiligent.cpp
)

set(IMGUIZMO_QUAT_SOURCE
    ${IMGUIZMO_QUAT_PATH}/imGuIZMO.cpp
    ${IMGUIZMO_QUAT_PATH}/imGuIZMO.h
)

set(INTERFACE
    interface/ImGuiImplDiligent.h
    interface/ImGuiUtils.h
)

if(PLATFORM_WIN32)
    list(APPEND DEAR_IMGUI_SOURCE ${DEAR_IMGUI_PATH}/examples/imgui_impl_win32.cpp)
    list(APPEND DEAR_IMGUI_INCLUDE ${DEAR_IMGUI_PATH}/examples/imgui_impl_win32.h)

    list(APPEND SOURCE src/ImGuiImplWin32.cpp)
    list(APPEND INTERFACE interface/ImGuiImplWin32.h)
elseif(PLATFORM_UNIVERSAL_WINDOWS)
    list(APPEND SOURCE src/ImGuiImplUWP.cpp)
    list(APPEND INTERFACE interface/ImGuiImplUWP.h)
elseif(PLATFORM_LINUX)
    list(APPEND SOURCE src/ImGuiImplLinuxXCB.cpp src/ImGuiImplLinuxX11.cpp)
    list(APPEND INTERFACE interface/ImGuiImplLinuxXCB.h interface/ImGuiImplLinuxX11.h)
elseif(PLATFORM_ANDROID)
    list(APPEND SOURCE src/ImGuiImplAndroid.cpp)
    list(APPEND INTERFACE interface/ImGuiImplAndroid.h)
elseif(PLATFORM_MACOS)
    list(APPEND DEAR_IMGUI_SOURCE ${DEAR_IMGUI_PATH}/examples/imgui_impl_osx.mm)
    list(APPEND DEAR_IMGUI_INCLUDE ${DEAR_IMGUI_PATH}/examples/imgui_impl_osx.h)

    list(APPEND SOURCE src/ImGuiImplMacOS.mm)
    list(APPEND INTERFACE interface/ImGuiImplMacOS.h)
elseif(PLATFORM_IOS)
    list(APPEND SOURCE src/ImGuiImplIOS.mm)
    list(APPEND INTERFACE interface/ImGuiImplIOS.h)
endif()

add_library(Diligent-Imgui STATIC ${SOURCE} ${INCLUDE} ${INTERFACE} ${DEAR_IMGUI_SOURCE} ${DEAR_IMGUI_INCLUDE} ${DEAR_IMGUI_INTERFACE} ${IMGUIZMO_QUAT_SOURCE})
set_common_target_properties(Diligent-Imgui)

target_include_directories(Diligent-Imgui
PUBLIC
    interface
    ${IMGUIZMO_QUAT_PATH}
    ${DEAR_IMGUI_PATH}
PRIVATE
    include
)

if(PLATFORM_LINUX)
    target_link_libraries(Diligent-Imgui PRIVATE XCBKeySyms)
endif()

if(PLATFORM_UNIVERSAL_WINDOWS)
    target_compile_definitions(Diligent-Imgui PRIVATE IMGUI_DISABLE_WIN32_FUNCTIONS)
endif()

source_group("dear_imgui" FILES ${DEAR_IMGUI_SOURCE} ${DEAR_IMGUI_INCLUDE} ${DEAR_IMGUI_INTERFACE})
source_group("src" FILES ${SOURCE})
source_group("include" FILES ${INCLUDE})
source_group("interface" FILES ${INTERFACE})
source_group("imGuIZMO.quat" FILES ${IMGUIZMO_QUAT_SOURCE})

target_link_libraries(Diligent-Imgui 
PRIVATE 
    Diligent-BuildSettings 
    Diligent-Common 
    Diligent-PlatformInterface 
    Diligent-GraphicsEngineInterface 
    Diligent-GraphicsAccessories
    Diligent-GraphicsTools 
)

set_target_properties(Diligent-Imgui PROPERTIES
    FOLDER DiligentTools
)

if(DILIGENT_INSTALL_TOOLS)
    install_tools_lib(Diligent-Imgui)
endif()
