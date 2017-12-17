cmake_minimum_required (VERSION 3.6)

project(Lua CXX)

set(SOURCE 
	src/lapi.c
	src/lauxlib.c
	src/lbaselib.c
	src/lbitlib.c
	src/lcode.c
	src/lcorolib.c
	src/lctype.c
	src/ldblib.c
	src/ldebug.c
	src/ldo.c
	src/ldump.c
	src/lfunc.c
	src/lgc.c
	src/linit.c
	src/liolib.c
	src/llex.c
	src/lmathlib.c
	src/lmem.c
	src/loadlib.c
	src/lobject.c
	src/lopcodes.c
	src/loslib.c
	src/lparser.c
	src/lstate.c
	src/lstring.c
	src/lstrlib.c
	src/ltable.c
	src/ltablib.c
	src/ltm.c
#	src/lua.c
#	src/luac.c
	src/lundump.c
	src/lvm.c
	src/lzio.c
)

# We need to compile all source files with c++ compiler
set_source_files_properties(${SOURCE} PROPERTIES LANGUAGE CXX)

set(INCLUDE 
	src/lapi.h
	src/lauxlib.h
	src/lcode.h
	src/lctype.h
	src/ldebug.h
	src/ldo.h
	src/lfunc.h
	src/lgc.h
	src/llex.h
	src/llimits.h
	src/lmem.h
	src/lobject.h
	src/lopcodes.h
	src/lparser.h
	src/lstate.h
	src/lstring.h
	src/ltable.h
	src/ltm.h
	src/lua.h
	src/lua.hpp
	src/luaconf.h
	src/lualib.h
	src/lundump.h
	src/lvm.h
	src/lzio.h
	src/winappstubs.h
)

add_library(Lua STATIC ${SOURCE} ${INCLUDE})
set_common_target_properties(Lua)

target_include_directories(Lua PUBLIC
	src
)

target_link_libraries(Lua PRIVATE BuildSettings)

if(MSVC)
	# Disable MSVC-specific warnings
	target_compile_definitions(Lua PRIVATE -D_CRT_SECURE_NO_WARNINGS)
	target_compile_options(Lua PRIVATE /W3 /wd4244 /wd4334 /wd4702)
endif(MSVC)

if (CMAKE_CXX_COMPILER_ID MATCHES "Clang" OR 
    CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
	# Setting LANGUAGE to CXX is not enough for clang, so use -x c++ to compile c files as c++ and suppress 
	# the following warning:
	# clang++.exe: warning: treating 'c' input as 'c++' when in C++ mode, this behavior is deprecated [-Wdeprecated]
	target_compile_options(Lua PRIVATE -x c++)
	
	# For loslib.c, disable the following
	# warning: 'tmpnam' is deprecated: tempnam is unsafe, use mkstemp or tmpfile instead [-Wdeprecated-declarations]
	set_source_files_properties(src/loslib.c PROPERTIES COMPILE_FLAGS -Wno-deprecated-declarations)
endif()

source_group("src" FILES ${SOURCE})
source_group("include" FILES ${INCLUDE})

set_target_properties(Lua PROPERTIES
	FOLDER Tools/External
)