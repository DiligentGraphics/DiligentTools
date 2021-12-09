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
        ../RenderStateNotationParser/generated
        ../RenderStateNotationParser/include
        ../ThirdParty/args      
        ../ThirdParty/json
)

target_link_libraries(Diligent-RenderStatePackager PUBLIC Diligent-Common Diligent-Archiver Diligent-RenderStateNotationParser)
add_custom_command (TARGET Diligent-RenderStatePackager  POST_BUILD
                    COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:Diligent-Archiver> $<TARGET_FILE_DIR:Diligent-RenderStatePackager>
)

set_target_properties(Diligent-RenderStatePackager PROPERTIES
    FOLDER DiligentTools
)

set_common_target_properties(Diligent-RenderStatePackager)

if (DILIGENT_INSTALL_TOOLS)
    install_tools_lib(Diligent-Archiver)
    install(TARGETS Diligent-RenderStatePackager RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}/${DILIGENT_TOOLS_DIR}/$<CONFIG>" OPTIONAL)
endif()
