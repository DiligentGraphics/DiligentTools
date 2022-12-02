cmake_minimum_required (VERSION 3.6)

project(ZLib C)

set(ZLIB_DIR ${CMAKE_CURRENT_SOURCE_DIR}/zlib)

set(ZLIB_SOURCE 
    ${ZLIB_DIR}/adler32.c
    ${ZLIB_DIR}/compress.c
    ${ZLIB_DIR}/crc32.c
    ${ZLIB_DIR}/deflate.c
    ${ZLIB_DIR}/gzclose.c
    ${ZLIB_DIR}/gzlib.c
    ${ZLIB_DIR}/gzread.c
    ${ZLIB_DIR}/gzwrite.c
    ${ZLIB_DIR}/infback.c
    ${ZLIB_DIR}/inffast.c
    ${ZLIB_DIR}/inflate.c
    ${ZLIB_DIR}/inftrees.c
    ${ZLIB_DIR}/trees.c
    ${ZLIB_DIR}/uncompr.c
    ${ZLIB_DIR}/zutil.c
)

set(ZLIB_INCLUDE 
    ${ZLIB_DIR}/crc32.h
    ${ZLIB_DIR}/deflate.h
    ${ZLIB_DIR}/gzguts.h
    ${ZLIB_DIR}/inffast.h
    ${ZLIB_DIR}/inffixed.h
    ${ZLIB_DIR}/inflate.h
    ${ZLIB_DIR}/inftrees.h
    ${ZLIB_DIR}/trees.h
    ${ZLIB_DIR}/zconf.h
    ${ZLIB_DIR}/zlib.h
    ${ZLIB_DIR}/zutil.h
)

add_library(ZLib STATIC ${ZLIB_SOURCE} ${ZLIB_INCLUDE})
add_library(ZLIB::ZLIB ALIAS ZLib)
set_common_target_properties(ZLib)

# Use interface libarary to add -Wno-error at the end of the command line.
# Using target_compile_options() adds it before options of the linked libraries.
target_link_libraries(ZLib PRIVATE Diligent-BuildSettings NO_WERROR)

if(MSVC)
    target_compile_definitions(ZLib PRIVATE -D_CRT_SECURE_NO_DEPRECATE)
    target_compile_options(ZLib PRIVATE /W3 /wd4131 /wd4127 /wd4244 /wd4996)
endif()

if(PLATFORM_LINUX OR PLATFORM_ANDROID OR PLATFORM_MACOS OR PLATFORM_IOS OR PLATFORM_TVOS OR PLATFORM_EMSCRIPTEN)
    target_compile_definitions(ZLib PRIVATE HAVE_UNISTD_H)
endif()

if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    # Disable the following warning:
    # shifting a negative signed value is undefined [-Wshift-negative-value]
    set_property(SOURCE ${ZLIB_DIR}/inflate.c APPEND_STRING PROPERTY COMPILE_FLAGS "-Wno-shift-negative-value")

    # Disable the following warning:
    # implicit conversion loses integer precision: 'ssize_t' (aka 'long') to 'int'
    set_property(SOURCE ${ZLIB_DIR}/gzwrite.c ${ZLIB_DIR}/gzread.c APPEND_STRING PROPERTY COMPILE_FLAGS "-Wno-shorten-64-to-32")
endif()


target_include_directories(ZLib PUBLIC ${ZLIB_DIR})

set(ZLIB_INCLUDE_DIRS "${ZLIB_DIR}" CACHE INTERNAL "ZLib include directory")
set(ZLIB_LIBRARIES ZLib CACHE INTERNAL "ZLib library")

source_group("src" FILES ${ZLIB_SOURCE})
source_group("include" FILES ${ZLIB_INCLUDE})

set_target_properties(ZLib PROPERTIES
    FOLDER DiligentTools/ThirdParty
)


option(ZLIB_BUILD_MINIZIP "Build minizip library" OFF)
if(${ZLIB_BUILD_MINIZIP})
    set(MINIZIP_SOURCE
       ${ZLIB_DIR}/contrib/minizip/ioapi.c
       ${ZLIB_DIR}/contrib/minizip/mztools.c
       ${ZLIB_DIR}/contrib/minizip/unzip.c
       ${ZLIB_DIR}/contrib/minizip/zip.c
    )
    set(MINIZIP_INCLUDE
       ${ZLIB_DIR}/contrib/minizip/crypt.h
       ${ZLIB_DIR}/contrib/minizip/ioapi.h
       ${ZLIB_DIR}/contrib/minizip/mztools.h
       ${ZLIB_DIR}/contrib/minizip/unzip.h
       ${ZLIB_DIR}/contrib/minizip/zip.h
    )
    
    if(PLATFORM_WIN32)
        list(APPEND MINIZIP_SOURCE ${ZLIB_DIR}/contrib/minizip/iowin32.c)
        list(APPEND MINIZIP_SOURCE ${ZLIB_DIR}/contrib/minizip/iowin32.h)
    endif()

    add_library(MiniZip STATIC ${MINIZIP_SOURCE} ${MINIZIP_INCLUDE})
    set_common_target_properties(MiniZip)

    target_link_libraries(MiniZip PRIVATE Diligent-BuildSettings PUBLIC ZLib)
    target_compile_definitions(MiniZip PUBLIC STRICTZIPUNZIP)

    if(MSVC)
        target_compile_definitions(MiniZip PRIVATE -D_CRT_SECURE_NO_DEPRECATE)
        target_compile_options(MiniZip PRIVATE /W3 /wd4131 /wd4189 /wd4456 /wd4244 /wd4701 /wd4703)
    endif()

    target_include_directories(MiniZip PUBLIC ${ZLIB_DIR}/contrib/minizip/)

    source_group("src" FILES ${MINIZIP_SOURCE})
    source_group("include" FILES ${MINIZIP_INCLUDE})

    set_target_properties(MiniZip PROPERTIES
        FOLDER DiligentTools/ThirdParty
    )
endif()
