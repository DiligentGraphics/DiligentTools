cmake_minimum_required (VERSION 3.6)

project(Diligent-RenderStatePackager CXX)

file(GLOB COMMON_INCLUDE    include/*)
file(GLOB COMMON_SOURCE     src/*)

set(INCLUDE   ${COMMON_INCLUDE})
set(SOURCE    ${COMMON_SOURCE})

source_group("include"   FILES ${INCLUDE})
source_group("source"    FILES ${SOURCE})

add_executable(Diligent-RenderStatePackager 
    ${INCLUDE}
    ${SOURCE}
    README.md
)

target_include_directories(Diligent-RenderStatePackager
	PRIVATE
    	include
        ${DILIGENT_ARGS_DIR}
)

target_link_libraries(Diligent-RenderStatePackager
PUBLIC
    Diligent-BuildSettings
    Diligent-GraphicsAccessories
    Diligent-Common
    Diligent-Archiver-static
    Diligent-RenderStateNotation
)

set_target_properties(Diligent-RenderStatePackager PROPERTIES
    FOLDER DiligentTools
)

set_common_target_properties(Diligent-RenderStatePackager)

#set_target_properties(Diligent-RenderStatePackager PROPERTIES
#    INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}/${DILIGENT_CORE_DIR}/${CMAKE_BUILD_TYPE}"
#)

target_compile_definitions(Diligent-RenderStatePackager PUBLIC JSON_DIAGNOSTICS=1)

if (DILIGENT_INSTALL_TOOLS)
    install(TARGETS Diligent-RenderStatePackager RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}/${DILIGENT_TOOLS_DIR}/$<CONFIG>" OPTIONAL)
endif()
