cmake_minimum_required (VERSION 3.3)

project(LibPng C)

set(SOURCE 
	png.c
	pngerror.c
	pngget.c
	pngmem.c
	pngpread.c
	pngread.c
	pngrio.c
	pngrtran.c
	pngrutil.c
	pngset.c
	pngtrans.c
	pngwio.c
	pngwrite.c
	pngwtran.c
	pngwutil.c
)

set(INCLUDE 
	png.h
	pngconf.h
	pngdebug.h
	pnginfo.h
	pngpriv.h
	pngstruct.h
)

add_library(
	LibPng ${SOURCE} ${INCLUDE}
)

if(WIN32)
	target_compile_definitions(LibPng PRIVATE -D_CRT_SECURE_NO_WARNINGS)
endif()

target_include_directories(LibPng 
INTERFACE
	.
)

target_link_libraries(LibPng 
PRIVATE 
	BuildSettings 
PUBLIC
	ZLib
)

source_group("source" FILES ${SOURCE})
source_group("include" FILES ${INCLUDE})
