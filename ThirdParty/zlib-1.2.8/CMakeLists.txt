cmake_minimum_required (VERSION 3.6)

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

add_library(ZLib STATIC ${SOURCE} ${INCLUDE})
set_common_target_properties(ZLib)

target_link_libraries(ZLib PRIVATE Diligent-BuildSettings)

if(MSVC)
    target_compile_definitions(ZLib PRIVATE -D_CRT_SECURE_NO_DEPRECATE)
    target_compile_options(ZLib PRIVATE /W3 /wd4131 /wd4127 /wd4244 /wd4996)
endif()

if(PLATFORM_LINUX OR PLATFORM_ANDROID OR PLATFORM_MACOS OR PLATFORM_IOS)
    target_compile_definitions(ZLib PRIVATE HAVE_UNISTD_H)
endif()

if (CMAKE_CXX_COMPILER_ID MATCHES "Clang" OR 
    CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    # For inflate.c, disable the following
    # warning: shifting a negative signed value is undefined [-Wshift-negative-value]
    set_source_files_properties(inflate.c PROPERTIES COMPILE_FLAGS -Wno-shift-negative-value)
endif()


target_include_directories(ZLib PUBLIC .)

source_group("src" FILES ${SOURCE})
source_group("include" FILES ${INCLUDE})

set_target_properties(ZLib PROPERTIES
    FOLDER DiligentTools/ThirdParty
)
