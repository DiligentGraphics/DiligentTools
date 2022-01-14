cmake_minimum_required (VERSION 3.6)

if(TARGET gtest)
    if(DILIGENT_BUILD_TOOLS_TESTS)
        add_subdirectory(DiligentToolsTest)
        add_subdirectory(DiligentToolsGPUTest)
    endif()
endif()

if(DILIGENT_BUILD_TOOLS_INCLUDE_TEST)
    add_subdirectory(IncludeTest)
endif()
