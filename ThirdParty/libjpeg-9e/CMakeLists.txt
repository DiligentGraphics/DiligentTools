cmake_minimum_required (VERSION 3.6)

project(LibJpeg C)

set(SOURCE 
    jaricom.c
    jcapimin.c
    jcapistd.c
    jcarith.c
    jccoefct.c
    jccolor.c
    jcdctmgr.c
    jchuff.c
    jcinit.c
    jcmainct.c
    jcmarker.c
    jcmaster.c
    jcomapi.c
    jcparam.c
    jcprepct.c
    jcsample.c
    jctrans.c
    jdapimin.c
    jdapistd.c
    jdarith.c
    jdatadst.c
    jdatasrc.c
    jdcoefct.c
    jdcolor.c
    jddctmgr.c
    jdhuff.c
    jdinput.c
    jdmainct.c
    jdmarker.c
    jdmaster.c
    jdmerge.c
    jdpostct.c
    jdsample.c
    jdtrans.c
    jerror.c
    jfdctflt.c
    jfdctfst.c
    jfdctint.c
    jidctflt.c
    jidctfst.c
    jidctint.c
    jmemmgr.c
    jmemnobs.c
    jquant1.c
    jquant2.c
    jutils.c
)

set(INCLUDE 
    jconfig.h
    jdct.h
    jerror.h
    jinclude.h
    jmemsys.h
    jmorecfg.h
    jpegint.h
    jpeglib.h
    jversion.h
)

add_library(LibJpeg STATIC ${SOURCE} ${INCLUDE})
add_library(JPEG::JPEG ALIAS LibJpeg)
set_common_target_properties(LibJpeg)

target_include_directories(LibJpeg PUBLIC .)

# Use special interface library NO_WERROR to add -Wno-error at the end of the command line.
# Using target_compile_options() adds it before options of the linked libraries.
target_link_libraries(LibJpeg PRIVATE Diligent-BuildSettings NO_WERROR)

# If libjpeg-turbo is also present in the project, the following symbols will be
# defined multiple times and produce linker errors. To avoid this, we rename them.
target_compile_definitions(LibJpeg
PRIVATE
    jpeg_natural_order=lj_jpeg_natural_order
    jdiv_round_up=lj_div_round_up
    jround_up=lj_round_up
    jcopy_sample_rows=lj_copy_sample_rows
    jcopy_block_row=lj_copy_block_row
    jinit_huff_decoder=lj_init_huff_decoder
    jinit_huff_encoder=lj_init_huff_encoder
)

if(MSVC)
    # Disable MSVC-specific warnings
    # - w4244: '+=': conversion from 'int' to 'JCOEF', possible loss of data
    # - w4127: conditional expression is constant
    target_compile_options(LibJpeg PRIVATE /wd4244 /wd4127)
    target_compile_definitions(LibJpeg PRIVATE -D_CRT_SECURE_NO_WARNINGS)
    if(PLATFORM_UNIVERSAL_WINDOWS)
        target_compile_definitions(LibJpeg PRIVATE -DNO_GETENV)
    endif()
endif(MSVC)

source_group("src" FILES ${SOURCE})
source_group("include" FILES ${INCLUDE})

set_target_properties(LibJpeg PROPERTIES
    FOLDER DiligentTools/ThirdParty
)