cmake_minimum_required (VERSION 3.10)

project(XCBKeySyms C)

set(INTERFACE
    xcb_keysyms/xcb_keysyms.h
)

set(SOURCE 
    xcb_keysyms/xcb_keysyms.c
)

add_library(XCBKeySyms STATIC ${SOURCE} ${INTERFACE})
set_common_target_properties(XCBKeySyms)

target_include_directories(XCBKeySyms PUBLIC .)

source_group("source" FILES ${SOURCE})
source_group("interface" FILES ${INTERFACE})

find_library(XCB_LIBRARY xcb)
target_link_libraries(XCBKeySyms 
PRIVATE 
    Diligent-BuildSettings
    ${XCB_LIBRARY}
)

set_target_properties(XCBKeySyms PROPERTIES
    FOLDER Common
)

