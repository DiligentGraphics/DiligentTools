cmake_minimum_required (VERSION 3.6)

option(DILIGENT_BUILD_RENDER_SCRIPT "Build render script" ON)

file(RELATIVE_PATH DILIGENT_TOOLS_DIR "${CMAKE_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}")

if(PLATFORM_WIN32 OR PLATFORM_UNIVERSAL_WINDOWS OR PLATFORM_LINUX OR PLATFORM_MACOS OR PLATFORM_IOS)
    option(DILIGENT_INSTALL_TOOLS "Install DiligentTool module headers and libraries" ON)
else()
    set(DILIGENT_INSTALL_TOOLS OFF)
endif()

function(install_tools_lib _TARGET)
    get_target_relative_dir(${_TARGET} TARGET_RELATIVE_PATH)

    get_target_property(TARGET_TYPE ${_TARGET} TYPE)
    if(TARGET_TYPE STREQUAL STATIC_LIBRARY)
        list(APPEND DILIGENT_TOOLS_INSTALL_LIBS_LIST ${_TARGET})
        set(DILIGENT_TOOLS_INSTALL_LIBS_LIST ${DILIGENT_TOOLS_INSTALL_LIBS_LIST} CACHE INTERNAL "Diligent tools libraries installation list")
    elseif(TARGET_TYPE STREQUAL SHARED_LIBRARY)
        install(TARGETS				 ${_TARGET}
                ARCHIVE DESTINATION "lib/${DILIGENT_TOOLS_DIR}/$<CONFIG>"
                LIBRARY DESTINATION "bin/${DILIGENT_TOOLS_DIR}/$<CONFIG>"
                RUNTIME DESTINATION "bin/${DILIGENT_TOOLS_DIR}/$<CONFIG>"
        )
        if (DILIGENT_INSTALL_PDB)
            install(FILES $<TARGET_PDB_FILE:${_TARGET}> DESTINATION "bin/${DILIGENT_TOOLS_DIR}/$<CONFIG>" OPTIONAL)
        endif()
    endif()

    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/interface")
        install(DIRECTORY    interface
                DESTINATION  "headers/${TARGET_RELATIVE_PATH}/"
        )
    endif()
endfunction()

add_subdirectory(ThirdParty)
add_subdirectory(TextureLoader)
add_subdirectory(AssetLoader)
add_subdirectory(Imgui)
add_subdirectory(NativeApp)

if(${DILIGENT_BUILD_RENDER_SCRIPT})
    add_subdirectory(RenderScript)
endif()

if(PLATFORM_WIN32 AND GL_SUPPORTED)
    add_subdirectory(HLSL2GLSLConverter)
endif()

# Installation instructions
if(DILIGENT_INSTALL_TOOLS)

    install_combined_static_lib(
        "${CMAKE_STATIC_LIBRARY_PREFIX}DiligentTools${CMAKE_STATIC_LIBRARY_SUFFIX}"
        "${DILIGENT_TOOLS_INSTALL_LIBS_LIST}"
        DiligentTools-static                     # Custom target name
        DiligentTools                            # Folder
        "lib/${DILIGENT_TOOLS_DIR}/$<CONFIG>"    # Install destination
    )

endif()


# Create a custom target to run source code formatting validation command
if(CMAKE_HOST_SYSTEM_NAME STREQUAL "Windows")
    add_custom_target(DiligentTools-ValidateFormatting ALL
        COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/../DiligentCore/.clang-format" "${CMAKE_CURRENT_SOURCE_DIR}/.clang-format"
    )

    add_custom_command(TARGET DiligentTools-ValidateFormatting
        COMMAND validate_format_win.bat
        WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/BuildTools/FormatValidation"
        COMMENT "Validating DiligentTools module's source code formatting..."
        VERBATIM
    )
endif()

if(TARGET DiligentTools-ValidateFormatting)
    set_target_properties(DiligentTools-ValidateFormatting PROPERTIES
        FOLDER DiligentTools
    )
endif()
