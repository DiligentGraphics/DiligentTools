cmake_minimum_required (VERSION 3.6)

set(LICENSE_INSTALL_PATH "Licenses/ThirdParty/${DILIGENT_TOOLS_DIR}")

if (NOT TARGET JPEG::JPEG)
    add_subdirectory(libjpeg-9a)
    install(FILES libjpeg-9a/README DESTINATION ${LICENSE_INSTALL_PATH} RENAME libjpeg-readme.txt)
    list(APPEND THIRD_PARTY_TARGETS LibJpeg)
endif()

if (NOT TARGET TIFF::TIFF)
    add_subdirectory(libtiff)
    install(FILES libtiff/COPYRIGHT DESTINATION ${LICENSE_INSTALL_PATH} RENAME libtiff-copyright.txt)
    list(APPEND THIRD_PARTY_TARGETS LibTiff)
endif()

if (NOT TARGET ZLIB::ZLIB)
    add_subdirectory(zlib-1.2.8)
    install(FILES zlib-1.2.8/README DESTINATION ${LICENSE_INSTALL_PATH} RENAME zlib-readme.txt)
    list(APPEND THIRD_PARTY_TARGETS ZLib)
endif()

if (NOT TARGET PNG::PNG)
    add_subdirectory(lpng-1.6.17)
    install(FILES lpng-1.6.17/LICENSE DESTINATION ${LICENSE_INSTALL_PATH} RENAME lpng-license.txt)
    list(APPEND THIRD_PARTY_TARGETS LibPng)
endif()


if (NOT DILIGENT_ARGS_DIR)
    set(DILIGENT_ARGS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/args" CACHE PATH "args source directory")
endif()
if (NOT EXISTS "${DILIGENT_ARGS_DIR}/args.hxx")
    message(FATAL_ERROR "${DILIGENT_ARGS_DIR}/args.hxx does not exist. This indicates that args submodule is not checked out or DILIGENT_ARGS_DIR variable contains invalid path.")
endif()
install(FILES "${DILIGENT_ARGS_DIR}/LICENSE" DESTINATION ${LICENSE_INSTALL_PATH} RENAME args-license.txt)


install(FILES tinygltf/LICENSE DESTINATION ${LICENSE_INSTALL_PATH} RENAME tinygltf-license.txt)
install(FILES imGuIZMO.quat/license.txt DESTINATION ${LICENSE_INSTALL_PATH} RENAME imGuIZMO.quat-license.txt)
install(FILES stb/LICENSE.txt DESTINATION "${LICENSE_INSTALL_PATH}" RENAME stb-license.txt)
install(FILES json/LICENSE.MIT DESTINATION ${LICENSE_INSTALL_PATH} RENAME json-license.txt)

if (NOT DILIGENT_DEAR_IMGUI_PATH)
    set(DILIGENT_DEAR_IMGUI_PATH "${CMAKE_CURRENT_SOURCE_DIR}/imgui" CACHE PATH "imgui source directory")
endif()
if (NOT EXISTS "${DILIGENT_DEAR_IMGUI_PATH}/imgui.h")
    message(FATAL_ERROR "${DILIGENT_DEAR_IMGUI_PATH}/imgui.h does not exist. This inidcates that imgui submodule is not checked out or DILIGENT_DEAR_IMGUI_PATH variable contains invalid path.")
endif()
install(FILES "${DILIGENT_DEAR_IMGUI_PATH}/LICENSE.txt" DESTINATION ${LICENSE_INSTALL_PATH} RENAME imgui-license.txt)

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
