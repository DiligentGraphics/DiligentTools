cmake_minimum_required (VERSION 3.10)

# Both suites use the shared Core browser harness, with module-local assets.
function(configure_tools_browser_test TARGET)
    if(PLATFORM_WEB)
        # Keep invoke_* wrappers synchronous during static initialization, before
        # main() enters through WebAssembly.promising and can suspend for GPU waits.
        target_link_options(${TARGET} PRIVATE
            "-sJSPI=1"
            "-sASYNCIFY_IGNORE_INDIRECT=1"
            "-sALLOW_MEMORY_GROWTH=1"
            "-sPTHREAD_POOL_SIZE=Math.max(1,navigator.hardwareConcurrency)*4"
            "-sSTACK_SIZE=8388608"
            "-sEXIT_RUNTIME=1"
            "-sEXPORTED_RUNTIME_METHODS=FS"
            "SHELL: --preload-file '${CMAKE_CURRENT_SOURCE_DIR}/assets@/'"
        )
    endif()
endfunction()

if(TARGET gtest)
    if(DILIGENT_BUILD_TOOLS_TESTS)
        add_subdirectory(DiligentToolsTest)
        add_subdirectory(DiligentToolsGPUTest)
    endif()
endif()

if(DILIGENT_BUILD_TOOLS_INCLUDE_TEST)
    add_subdirectory(IncludeTest)
endif()
