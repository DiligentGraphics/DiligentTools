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

if(MSVC)
	target_compile_definitions(LibPng PRIVATE -D_CRT_SECURE_NO_WARNINGS)

	# Enable link-time code generation for release builds (I was not able to 
	# find any way to set these settings through interface library BuildSettings)
	set_target_properties(LibPng PROPERTIES
		STATIC_LIBRARY_FLAGS_RELEASE /LTCG
		STATIC_LIBRARY_FLAGS_MINSIZEREL /LTCG
		STATIC_LIBRARY_FLAGS_RELWITHDEBINFO /LTCG
	)
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

set_target_properties(LibPng PROPERTIES
	FOLDER Tools/External
)