cmake_minimum_required (VERSION 3.6)

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

add_library(LibPng ${SOURCE} ${INCLUDE})
set_common_target_properties(LibPng)

if(MSVC)
	target_compile_definitions(LibPng PRIVATE -D_CRT_SECURE_NO_WARNINGS)
	if(PLATFORM_UNIVERSAL_WINDOWS)
		target_compile_definitions(LibPng PRIVATE PNG_ABORT=abort)
	endif()
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

source_group("src" FILES ${SOURCE})
source_group("include" FILES ${INCLUDE})

set_target_properties(LibPng PROPERTIES
	FOLDER Tools/External
)