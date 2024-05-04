cmake_minimum_required (VERSION 3.6)

set(LICENSE_INSTALL_PATH "Licenses/ThirdParty/${DILIGENT_TOOLS_DIR}")

# Helper interface library to disable -Werror for the third-party libs
add_library(NO_WERROR INTERFACE)
if(MSVC)
    target_compile_options(NO_WERROR INTERFACE /WX-)
elseif (CMAKE_CXX_COMPILER_ID MATCHES "Clang" OR
        CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    target_compile_options(NO_WERROR INTERFACE -Wno-error)
endif()


if (NOT TARGET JPEG::JPEG)
    add_subdirectory(libjpeg-9e)
    install(FILES libjpeg-9e/README DESTINATION ${LICENSE_INSTALL_PATH} RENAME libjpeg-readme.txt)
    list(APPEND THIRD_PARTY_TARGETS LibJpeg)
endif()

if (NOT TARGET TIFF::TIFF)
    add_subdirectory(libtiff)
    install(FILES libtiff/COPYRIGHT DESTINATION ${LICENSE_INSTALL_PATH} RENAME libtiff-copyright.txt)
    list(APPEND THIRD_PARTY_TARGETS LibTiff)
    if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        target_compile_options(LibTiff PRIVATE -Wno-shorten-64-to-32)
    endif()
endif()

if (NOT TARGET ZLIB::ZLIB)
    include(zlib.cmake)
    install(FILES zlib/README DESTINATION ${LICENSE_INSTALL_PATH} RENAME zlib-readme.txt)
    list(APPEND THIRD_PARTY_TARGETS ZLib)
    if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        target_compile_options(ZLib PRIVATE -Wno-shorten-64-to-32)
    endif()
endif()

if (NOT TARGET PNG::PNG)
    set(PNG_BUILD_ZLIB ON CACHE BOOL "Use find_package(zlib) to find ZLib location")
    set(PNG_SHARED OFF CACHE BOOL "Build shared version of libpng")
    set(PNG_TOOLS OFF CACHE BOOL "Build libpng tools")
    set(PNG_TESTS OFF CACHE BOOL "Build libpng tests")
    set(SKIP_INSTALL_ALL ON)
    set(PNG_FRAMEWORK OFF)
    if(PLATFORM_TVOS)
        # libpng does not support tvOS, but it does support iOS, so we can use the iOS target
        set(IOS TRUE)
    endif()
    add_subdirectory(libpng)
    set_target_properties(png_static PROPERTIES POSITION_INDEPENDENT_CODE ON)
    set_directory_root_folder(libpng DiligentTools/ThirdParty/png)
    install(FILES libpng/LICENSE DESTINATION ${LICENSE_INSTALL_PATH} RENAME lpng-license.txt)
    add_library(PNG::PNG ALIAS png_static)
    list(APPEND THIRD_PARTY_TARGETS png_static)
endif()


if (NOT DILIGENT_ARGS_DIR)
    set(DILIGENT_ARGS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/args" CACHE PATH "args source directory")
endif()
if (NOT EXISTS "${DILIGENT_ARGS_DIR}/args.hxx")
    message(FATAL_ERROR "${DILIGENT_ARGS_DIR}/args.hxx does not exist. This indicates that args submodule is not checked out or DILIGENT_ARGS_DIR variable contains invalid path.")
endif()
install(FILES "${DILIGENT_ARGS_DIR}/LICENSE" DESTINATION ${LICENSE_INSTALL_PATH} RENAME args-license.txt)

if(NOT IMGUIZMO_QUAT_PATH)
    set(IMGUIZMO_QUAT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/imGuIZMO.quat CACHE PATH "imGuIZMO.quat source directory")
endif()

install(FILES tinygltf/LICENSE DESTINATION ${LICENSE_INSTALL_PATH} RENAME tinygltf-license.txt)
install(FILES imGuIZMO.quat/license.txt DESTINATION ${LICENSE_INSTALL_PATH} RENAME imGuIZMO.quat-license.txt)
install(FILES stb/LICENSE.txt DESTINATION "${LICENSE_INSTALL_PATH}" RENAME stb-license.txt)
install(FILES json/LICENSE.MIT DESTINATION ${LICENSE_INSTALL_PATH} RENAME json-license.txt)


add_library(Diligent-JSON INTERFACE)
target_include_directories(Diligent-JSON INTERFACE json)
# WARNING: JSON_DIAGNOSTICS must be consistently defined for all inclusions of json.hpp,
#          or there will be hard-to-diagnose run-time crashes.
#          It is required by RenderStatePackager even in release builds.
target_compile_definitions(Diligent-JSON INTERFACE JSON_DIAGNOSTICS=1)


if (NOT DILIGENT_DEAR_IMGUI_PATH)
    set(DILIGENT_DEAR_IMGUI_PATH "${CMAKE_CURRENT_SOURCE_DIR}/imgui" CACHE PATH "imgui source directory")
endif()
if (NOT EXISTS "${DILIGENT_DEAR_IMGUI_PATH}/imgui.h")
    message(FATAL_ERROR "${DILIGENT_DEAR_IMGUI_PATH}/imgui.h does not exist. This inidcates that imgui submodule is not checked out or DILIGENT_DEAR_IMGUI_PATH variable contains invalid path.")
endif()
install(FILES "${DILIGENT_DEAR_IMGUI_PATH}/LICENSE.txt" DESTINATION ${LICENSE_INSTALL_PATH} RENAME imgui-license.txt)

if (DILIGENT_ENABLE_DRACO AND (NOT TARGET draco) AND (NOT TARGET draco_static))
    message("Fetching Draco repository - this may take a few moments...")
    include(FetchContent)
    FetchContent_Declare(
        draco
        GIT_REPOSITORY https://github.com/google/draco
        GIT_TAG        1.5.6
    )
    FetchContent_MakeAvailable(draco)
    set_directory_root_folder(${draco_SOURCE_DIR} DiligentTools/ThirdParty/draco)
endif()

if(DILIGENT_INSTALL_TOOLS)
    install(TARGETS ${THIRD_PARTY_TARGETS}
            ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}/${DILIGENT_TOOLS_DIR}/$<CONFIG>"
            LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}/${DILIGENT_TOOLS_DIR}/$<CONFIG>"
            RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}/${DILIGENT_TOOLS_DIR}/$<CONFIG>"
    )

    list(APPEND DILIGENT_TOOLS_INSTALL_LIBS_LIST ${THIRD_PARTY_TARGETS})
    set(DILIGENT_TOOLS_INSTALL_LIBS_LIST ${DILIGENT_TOOLS_INSTALL_LIBS_LIST} CACHE INTERNAL "Diligent tools libraries installation list")

    install(
        FILES
            ${DILIGENT_DEAR_IMGUI_PATH}/imconfig.h
            ${DILIGENT_DEAR_IMGUI_PATH}/imgui.h
            ${DILIGENT_DEAR_IMGUI_PATH}/imgui_internal.h
            ${DILIGENT_DEAR_IMGUI_PATH}/imstb_rectpack.h
            ${DILIGENT_DEAR_IMGUI_PATH}/imstb_textedit.h
            ${DILIGENT_DEAR_IMGUI_PATH}/imstb_truetype.h
        DESTINATION
            "${CMAKE_INSTALL_INCLUDEDIR}/${DILIGENT_TOOLS_DIR}/ThirdParty/imgui"
    )
endif()
