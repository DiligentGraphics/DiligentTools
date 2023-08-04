cmake_minimum_required (VERSION 3.6)

project(Diligent-RenderStatePackager CXX)

set(INCLUDE
    include/ParsingEnvironment.hpp
    include/RenderStatePackager.hpp
)
set(SOURCE
    src/ParsingEnvironment.cpp
    src/RenderStatePackager.cpp
)

source_group("include" FILES ${INCLUDE})
source_group("src"     FILES ${SOURCE})

add_library(Diligent-RenderStatePackagerLib STATIC
    ${INCLUDE}
    ${SOURCE}
)

target_include_directories(Diligent-RenderStatePackagerLib
PUBLIC
    include
)

target_link_libraries(Diligent-RenderStatePackagerLib
PRIVATE
    Diligent-BuildSettings
    Diligent-GraphicsAccessories
PUBLIC
    Diligent-Archiver-static
    Diligent-RenderStateNotation
)

set_common_target_properties(Diligent-RenderStatePackagerLib)

add_executable(Diligent-RenderStatePackager
    src/main.cpp
    README.md
)
set_common_target_properties(Diligent-RenderStatePackager)

target_link_libraries(Diligent-RenderStatePackager
PRIVATE
    Diligent-BuildSettings
    Diligent-Common
    Diligent-RenderStatePackagerLib
)
target_include_directories(Diligent-RenderStatePackager
PRIVATE
    include
    ${DILIGENT_ARGS_DIR}
)

if (DILIGENT_INSTALL_TOOLS)
    install(TARGETS Diligent-RenderStatePackager RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}/${DILIGENT_TOOLS_DIR}/$<CONFIG>" OPTIONAL)
endif()


set_target_properties(Diligent-RenderStatePackagerLib Diligent-RenderStatePackager PROPERTIES
    FOLDER DiligentTools
)
