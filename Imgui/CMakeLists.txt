cmake_minimum_required (VERSION 3.6)

project(Diligent-Imgui CXX)

set(SOURCE
    src/ImGuiDiligentRenderer.cpp
    src/ImGuiImplDiligent.cpp
    src/ImGuiUtils.cpp
)

set(IMGUIZMO_QUAT_SOURCE
    ${IMGUIZMO_QUAT_PATH}/imGuIZMO.cpp
    ${IMGUIZMO_QUAT_PATH}/imGuIZMO.h
)

set(INTERFACE
    interface/ImGuiDiligentRenderer.hpp
    interface/ImGuiImplDiligent.hpp
    interface/ImGuiUtils.hpp
)

if(PLATFORM_WIN32)
    list(APPEND SOURCE src/ImGuiImplWin32.cpp)
    list(APPEND INTERFACE interface/ImGuiImplWin32.hpp)
elseif(PLATFORM_UNIVERSAL_WINDOWS)
    list(APPEND SOURCE src/ImGuiImplUWP.cpp)
    list(APPEND INTERFACE interface/ImGuiImplUWP.hpp)
elseif(PLATFORM_LINUX)
    list(APPEND SOURCE src/ImGuiImplLinuxXCB.cpp src/ImGuiImplLinuxX11.cpp)
    list(APPEND INTERFACE interface/ImGuiImplLinuxXCB.hpp interface/ImGuiImplLinuxX11.hpp)
elseif(PLATFORM_ANDROID)
    list(APPEND SOURCE src/ImGuiImplAndroid.cpp)
    list(APPEND INTERFACE interface/ImGuiImplAndroid.hpp)
elseif(PLATFORM_MACOS)
    list(APPEND SOURCE src/ImGuiImplMacOS.mm)
    list(APPEND INTERFACE interface/ImGuiImplMacOS.hpp)
elseif(PLATFORM_IOS)
    list(APPEND SOURCE src/ImGuiImplIOS.mm)
    list(APPEND INTERFACE interface/ImGuiImplIOS.hpp)
elseif(PLATFORM_TVOS)
    list(APPEND SOURCE src/ImGuiImplTVOS.mm)
    list(APPEND INTERFACE interface/ImGuiImplTVOS.hpp)
elseif(PLATFORM_EMSCRIPTEN)
    list(APPEND SOURCE src/ImGuiImplEmscripten.cpp)
    list(APPEND INTERFACE interface/ImGuiImplEmscripten.hpp)
endif()

add_library(Diligent-Imgui STATIC
    ${SOURCE}
    ${INCLUDE}
    ${INTERFACE}
    ${IMGUIZMO_QUAT_SOURCE}
)

if(TARGET imgui)
    target_link_libraries(Diligent-Imgui PRIVATE imgui)
else()
    set(DEAR_IMGUI_INTERFACE
        ${DILIGENT_DEAR_IMGUI_PATH}/imgui.h
    )

    set(DEAR_IMGUI_INCLUDE
        ${DILIGENT_DEAR_IMGUI_PATH}/imconfig.h
        ${DILIGENT_DEAR_IMGUI_PATH}/imgui_internal.h
        ${DILIGENT_DEAR_IMGUI_PATH}/imstb_rectpack.h
        ${DILIGENT_DEAR_IMGUI_PATH}/imstb_textedit.h
        ${DILIGENT_DEAR_IMGUI_PATH}/imstb_truetype.h
        ${DILIGENT_DEAR_IMGUI_PATH}/misc/cpp/imgui_stdlib.h
    )

    set(DEAR_IMGUI_SOURCE
        ${DILIGENT_DEAR_IMGUI_PATH}/imgui.cpp
        ${DILIGENT_DEAR_IMGUI_PATH}/imgui_draw.cpp
        ${DILIGENT_DEAR_IMGUI_PATH}/imgui_tables.cpp
        ${DILIGENT_DEAR_IMGUI_PATH}/imgui_widgets.cpp
        ${DILIGENT_DEAR_IMGUI_PATH}/misc/cpp/imgui_stdlib.cpp
    )

    if(PLATFORM_WIN32)
        list(APPEND DEAR_IMGUI_SOURCE ${DILIGENT_DEAR_IMGUI_PATH}/backends/imgui_impl_win32.cpp)
        list(APPEND DEAR_IMGUI_INCLUDE ${DILIGENT_DEAR_IMGUI_PATH}/backends/imgui_impl_win32.h)
    elseif(PLATFORM_MACOS)
        set(DEAR_IMGUI_V185_SOURCE
            ../ThirdParty/imgui_v1.85/imgui_impl_osx_v1.85.mm
            ../ThirdParty/imgui_v1.85/imgui_impl_osx_v1.85.h
        )
        target_sources(Diligent-Imgui PRIVATE ${DEAR_IMGUI_V185_SOURCE})
        source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}/../ThirdParty/imgui_v1.85 PREFIX "dear_imgui_v1.85" FILES ${DEAR_IMGUI_V185_SOURCE})
    endif()

    target_sources(Diligent-Imgui PRIVATE
        ${DEAR_IMGUI_SOURCE}
        ${DEAR_IMGUI_INCLUDE}
        ${DEAR_IMGUI_INTERFACE}
    )

    target_include_directories(Diligent-Imgui PUBLIC ${DILIGENT_DEAR_IMGUI_PATH})

    source_group(TREE ${DILIGENT_DEAR_IMGUI_PATH} PREFIX "dear_imgui" FILES ${DEAR_IMGUI_SOURCE} ${DEAR_IMGUI_INCLUDE} ${DEAR_IMGUI_INTERFACE})
endif()

set_common_target_properties(Diligent-Imgui)

target_include_directories(Diligent-Imgui
PUBLIC
    interface
PRIVATE
    include
)

if(PLATFORM_LINUX)
    target_link_libraries(Diligent-Imgui PRIVATE XCBKeySyms)
endif()

if(PLATFORM_UNIVERSAL_WINDOWS)
    target_compile_definitions(Diligent-Imgui PRIVATE IMGUI_DISABLE_WIN32_FUNCTIONS)
endif()

if(PLATFORM_WIN32 AND MINGW_BUILD)
    # Link with dwmapi.lib as imgui_impl_win32.cpp skips
    # '#pragma comment(lib, "dwmapi")'
    # when compiler is not MSVC
    target_link_libraries(Diligent-Imgui PRIVATE dwmapi.lib)
endif()

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
