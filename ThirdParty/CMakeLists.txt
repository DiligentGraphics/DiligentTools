cmake_minimum_required (VERSION 3.6)

add_subdirectory(libjpeg-9a)
add_subdirectory(libtiff)
add_subdirectory(zlib-1.2.8)
add_subdirectory(lpng-1.6.17)
add_subdirectory(lua-5.2.3)

file(RELATIVE_PATH TOOLS_DIR_PATH "${CMAKE_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}/..")
set(LICENSE_INSTALL_PATH "${TOOLS_DIR_PATH}/Licenses/ThirdParty")

install(FILES libjpeg-9a/README DESTINATION ${LICENSE_INSTALL_PATH} RENAME libjpeg-readme.txt)
install(FILES libtiff/COPYRIGHT DESTINATION ${LICENSE_INSTALL_PATH} RENAME libtiff-copyright.txt)
install(FILES lpng-1.6.17/LICENSE DESTINATION ${LICENSE_INSTALL_PATH} RENAME lpng-license.txt)
install(FILES lua-5.2.3/doc/readme.html DESTINATION ${LICENSE_INSTALL_PATH} RENAME lua-readme.html)
install(FILES tinygltf/LICENSE DESTINATION ${LICENSE_INSTALL_PATH} RENAME tinygltf-license.txt)
install(FILES zlib-1.2.8/README DESTINATION ${LICENSE_INSTALL_PATH} RENAME zlib-readme.txt)
