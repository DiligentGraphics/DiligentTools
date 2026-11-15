# Tools browser tests

Both test suites reuse the Core browser harness and their existing native test
sources. CPU tests run without a graphics device; GPU tests use WebGPU with
Chrome's SwiftShader adapter. The command-line render-state packager is not built
for Web, so its tests are not included in either Web executable.

## Build

Configure the Emscripten build with `DILIGENT_BUILD_TOOLS_TESTS=ON`, then build
`DiligentToolsTest` and `DiligentToolsGPUTest`. Each target embeds its `assets`
directory in the virtual filesystem and enables JSPI for blocking test waits.
The browser preloads four workers per logical CPU, as in the Core CPU suite.

## Run

With DiligentCore and DiligentTools checked out beside each other:

```sh
python3 DiligentTools/Tests/DiligentToolsTest/browser/run_tests.py \
    --module build/DiligentTools/Tests/DiligentToolsTest/DiligentToolsTest.js \
    --output ToolsCPUBrowserTestResults

python3 DiligentTools/Tests/DiligentToolsGPUTest/browser/run_tests.py \
    --module build/DiligentTools/Tests/DiligentToolsGPUTest/DiligentToolsGPUTest.js \
    --browser /path/to/github-action/common/chrome_swiftshader.sh \
    --output ToolsGPUBrowserTestResults
```

Chrome is discovered from PATH unless `--browser` is supplied. The GPU wrapper
selects SwiftShader; `DILIGENT_BROWSER_EXECUTABLE` can select its Chrome executable.
See `--help` for test filters, timeouts, and browser options. Each command starts
one browser for the suite, streams individual test results to the console, and
saves `tests.log`, `chrome.log`, and Google Test `results.xml` in the output directory.
An abort or timeout can prevent XML from being written; the logs are still saved.

Browser exclusions remain in each suite's `browser/exclude.txt`. Only tests that
depend on C++ exception handling are excluded from the CPU run. The GPU exclusion
list is currently empty. `--no-exclusions` runs without these filters.

## CI

The Emscripten workflow builds on all its platforms and runs both suites in Linux
Chrome for Debug and Release. The `run-tools-cpu-browser-tests` and
`run-tools-gpu-browser-tests` actions in `DiligentGraphics/github-action` run the
tests and upload their logs and XML; test configuration stays in this module.
