cmake_minimum_required (VERSION 3.3)

project(ZLib C)

set(SOURCE 
	adler32.c
	compress.c
	crc32.c
	deflate.c
	gzclose.c
	gzlib.c
	gzread.c
	gzwrite.c
	infback.c
	inffast.c
	inflate.c
	inftrees.c
	trees.c
	uncompr.c
	zutil.c
)

set(INCLUDE 
	crc32.h
	deflate.h
	gzguts.h
	inffast.h
	inffixed.h
	inflate.h
	inftrees.h
	trees.h
	zconf.h
	zlib.h
	zutil.h
)

add_library(
	ZLib STATIC ${SOURCE} ${INCLUDE}
)

target_link_libraries(ZLib PRIVATE BuildSettings)

if(MSVC)
	# Enable link-time code generation for release builds (I was not able to 
	# find any way to set these settings through interface library BuildSettings)
	set_target_properties(ZLib PROPERTIES
		STATIC_LIBRARY_FLAGS_RELEASE /LTCG
		STATIC_LIBRARY_FLAGS_MINSIZEREL /LTCG
		STATIC_LIBRARY_FLAGS_RELWITHDEBINFO /LTCG
	)
endif()

target_include_directories(ZLib PUBLIC .)

if(MSVC)
    target_compile_definitions(ZLib PRIVATE -D_CRT_SECURE_NO_DEPRECATE)
	target_compile_options(ZLib PRIVATE /W3 /wd4131 /wd4127 /wd4244 /wd4996)
endif()

source_group("source" FILES ${SOURCE})
source_group("include" FILES ${INCLUDE})

set_target_properties(ZLib PROPERTIES
	FOLDER Tools/External
)