cmake_minimum_required (VERSION 3.3)

project(LibPng C)

if(MSVC)
	add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif()

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

include_directories(
	../zlib-1.2.8
)

add_library(
	LibPng ${SOURCE} ${INCLUDE}
)

source_group("source" FILES ${SOURCE})
source_group("include" FILES ${INCLUDE})
