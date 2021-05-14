cmake_minimum_required (VERSION 3.6)

project(NDKHelper CXX)

set(SOURCE 
	src/gestureDetector.cpp
	src/JNIHelper.cpp
	src/perfMonitor.cpp
	src/sensorManager.cpp
	src/tapCamera.cpp
	src/vecmath.cpp
)

set(INCLUDE 
	include/gestureDetector.h
	include/JNIHelper.h
	include/perfMonitor.h
	include/sensorManager.h
	include/tapCamera.h
)

add_library(NDKHelper STATIC ${SOURCE} ${INCLUDE})
set_common_target_properties(NDKHelper)

target_include_directories(NDKHelper
PUBLIC
	include
)

target_link_libraries(NDKHelper 
PRIVATE 
	Diligent-BuildSettings
PUBLIC
    native_app_glue
)

if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    # WARNING: set_source_files_properties() does not append
    #          properties, but overwrites them!
	# NOTE:    target_compile_options() adds the option before -Werror, which has no effect
	set_property(SOURCE ${SOURCE}
    APPEND_STRING PROPERTY
        COMPILE_FLAGS " -Wno-deprecated-copy"
    )
endif()

source_group("src" FILES ${SOURCE})
source_group("include" FILES ${INCLUDE})

set_target_properties(NDKHelper PROPERTIES
	FOLDER Core/External
)
