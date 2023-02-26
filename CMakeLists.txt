cmake_minimum_required (VERSION 3.6)

file(RELATIVE_PATH DILIGENT_TOOLS_DIR "${CMAKE_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}")

if(PLATFORM_WIN32 OR PLATFORM_UNIVERSAL_WINDOWS OR PLATFORM_LINUX OR PLATFORM_MACOS OR PLATFORM_IOS OR PLATFORM_TVOS OR PLATFORM_EMSCRIPTEN)
    option(DILIGENT_INSTALL_TOOLS "Install DiligentTool module headers and libraries" ON)
else()
    set(DILIGENT_INSTALL_TOOLS OFF)
endif()

option(DILIGENT_NO_RENDER_STATE_PACKAGER "Do not build Render State Packager" OFF)
option(DILIGENT_ENABLE_DRACO "Enable Draco compression support in GLTF loader" OFF)

# Clear the list
set(DILIGENT_TOOLS_INSTALL_LIBS_LIST "" CACHE INTERNAL "Diligent tools libraries installation list")
function(install_tools_lib _TARGET)
    get_target_relative_dir(${_TARGET} TARGET_RELATIVE_PATH)

    get_target_property(TARGET_TYPE ${_TARGET} TYPE)
    if(TARGET_TYPE STREQUAL STATIC_LIBRARY)
        list(APPEND DILIGENT_TOOLS_INSTALL_LIBS_LIST ${_TARGET})
        set(DILIGENT_TOOLS_INSTALL_LIBS_LIST ${DILIGENT_TOOLS_INSTALL_LIBS_LIST} CACHE INTERNAL "Diligent tools libraries installation list")
    elseif(TARGET_TYPE STREQUAL SHARED_LIBRARY)
        install(TARGETS				 ${_TARGET}
                ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}/${DILIGENT_TOOLS_DIR}/$<CONFIG>"
                LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}/${DILIGENT_TOOLS_DIR}/$<CONFIG>"
                RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}/${DILIGENT_TOOLS_DIR}/$<CONFIG>"
        )
        if (DILIGENT_INSTALL_PDB)
            install(FILES $<TARGET_PDB_FILE:${_TARGET}> DESTINATION "${CMAKE_INSTALL_BINDIR}/${DILIGENT_TOOLS_DIR}/$<CONFIG>" OPTIONAL)
        endif()
    endif()

    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/interface")
        install(DIRECTORY    interface
                DESTINATION  "${CMAKE_INSTALL_INCLUDEDIR}/${TARGET_RELATIVE_PATH}/"
        )
    endif()
endfunction()

add_subdirectory(ThirdParty)
add_subdirectory(TextureLoader)
add_subdirectory(AssetLoader)
add_subdirectory(Imgui)
add_subdirectory(NativeApp)

if((PLATFORM_WIN32 OR PLATFORM_LINUX OR PLATFORM_MACOS) AND GL_SUPPORTED)
    add_subdirectory(HLSL2GLSLConverter)
endif()

add_subdirectory(RenderStateNotation)

if((PLATFORM_WIN32 OR PLATFORM_LINUX OR PLATFORM_MACOS) AND ARCHIVER_SUPPORTED AND NOT DILIGENT_NO_RENDER_STATE_PACKAGER)
    add_subdirectory(RenderStatePackager)
endif()

add_subdirectory(Tests)

# Installation instructions
if(DILIGENT_INSTALL_TOOLS)

    install_combined_static_lib(
        "${CMAKE_STATIC_LIBRARY_PREFIX}DiligentTools${CMAKE_STATIC_LIBRARY_SUFFIX}"
        "${DILIGENT_TOOLS_INSTALL_LIBS_LIST}"
        DiligentTools-static                     # Custom target name
        DiligentTools                            # Folder
        "${CMAKE_INSTALL_LIBDIR}/${DILIGENT_TOOLS_DIR}/$<CONFIG>"    # Install destination
    )

endif()

# Create a custom target to run source code formatting validation command
add_format_validation_target(DiligentTools "${CMAKE_CURRENT_SOURCE_DIR}" DiligentTools)
