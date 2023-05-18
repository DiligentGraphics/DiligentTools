# DiligentTools <img src="https://github.com/DiligentGraphics/DiligentCore/blob/master/media/diligentgraphics-logo.png" height=64 align="right" valign="middle">

This module implements additional functionality on top of the [Diligent Engine](https://github.com/DiligentGraphics/DiligentEngine)'s core module
and contains the following libraries:

* [Texture loader](TextureLoader): a texture loading library. The following formats are currently supported: jpg, png, tiff, dds, ktx.
* [Asset Loader](AssetLoader): an asset loading library. The library currently supports GLTF 2.0.
  * To enable Draco compression, download [Draco repository](https://github.com/google/draco) and include it into
    your project. Make sure that Draco source folder is processed by CMake *before* DiligentTools folder.
    Alternatively, you can specify a path to the Draco installation folder using `DRACO_PATH` CMake variable.
* [Imgui](Imgui): implementation of [dear imgui](https://github.com/ocornut/imgui) with Diligent API.
* [NativeApp](NativeApp): implementation of native application on supported platforms.
* [HLSL2GLSLConverter](HLSL2GLSLConverter): HLSL->GLSL off-line converter utility.
* [RenderStateNotation](RenderStateNotation): Diligent Render State notation parsing library.
* [RenderStatePackager](RenderStatePackager): Render state packaging tool.


To build the module, see [build instructions](https://github.com/DiligentGraphics/DiligentEngine/blob/master/README.md) in the master repository.

| Platform                                                                                                                                    |   Build Status                    |
| --------------------------------------------------------------------------------------------------------------------------------------------| --------------------------------- |
| <img src="https://github.com/DiligentGraphics/DiligentCore/blob/master/media/windows-logo.png" width=24 valign="middle"> Windows            | [![Build Status](https://github.com/DiligentGraphics/DiligentTools/actions/workflows/build-windows.yml/badge.svg?branch=master)](https://github.com/DiligentGraphics/DiligentTools/actions/workflows/build-windows.yml?query=branch%3Amaster) |
| <img src="https://github.com/DiligentGraphics/DiligentCore/blob/master/media/uwindows-logo.png" width=24 valign="middle"> Universal Windows | [![Build Status](https://github.com/DiligentGraphics/DiligentTools/actions/workflows/build-windows.yml/badge.svg?branch=master)](https://github.com/DiligentGraphics/DiligentTools/actions/workflows/build-windows.yml?query=branch%3Amaster) |
| <img src="https://github.com/DiligentGraphics/DiligentCore/blob/master/media/linux-logo.png" width=24 valign="middle"> Linux                | [![Build Status](https://github.com/DiligentGraphics/DiligentTools/actions/workflows/build-linux.yml/badge.svg?branch=master)](https://github.com/DiligentGraphics/DiligentTools/actions/workflows/build-linux.yml?query=branch%3Amaster) |
| <img src="https://github.com/DiligentGraphics/DiligentCore/blob/master/media/macos-logo.png" width=24 valign="middle"> MacOS                | [![Build Status](https://github.com/DiligentGraphics/DiligentTools/actions/workflows/build-apple.yml/badge.svg?branch=master)](https://github.com/DiligentGraphics/DiligentTools/actions/workflows/build-apple.yml?query=branch%3Amaster) |
| <img src="https://github.com/DiligentGraphics/DiligentCore/blob/master/media/apple-logo.png" width=24 valign="middle"> iOS                  | [![Build Status](https://github.com/DiligentGraphics/DiligentTools/actions/workflows/build-apple.yml/badge.svg?branch=master)](https://github.com/DiligentGraphics/DiligentTools/actions/workflows/build-apple.yml?query=branch%3Amaster) |
| <img src="https://github.com/DiligentGraphics/DiligentCore/blob/master/media/tvos-logo.png" width=24 valign="middle"> tvOS                  | [![Build Status](https://github.com/DiligentGraphics/DiligentTools/actions/workflows/build-apple.yml/badge.svg?branch=master)](https://github.com/DiligentGraphics/DiligentTools/actions/workflows/build-apple.yml?query=branch%3Amaster) |
| <img src="https://github.com/DiligentGraphics/DiligentCore/blob/master/media/emscripten-logo.png" width=24 valign="middle"> Emscripten      | [![Build Status](https://github.com/DiligentGraphics/DiligentTools/actions/workflows/build-emscripten.yml/badge.svg?branch=master)](https://github.com/DiligentGraphics/DiligentTools/actions/workflows/build-emscripten.yml?query=branch%3Amaster) | 


[![License](https://img.shields.io/badge/License-Apache%202.0-blue.svg)](License.txt)
[![Chat on Discord](https://img.shields.io/discord/730091778081947680?logo=discord)](https://discord.gg/t7HGBK7)
[![Appveyor Build Status](https://ci.appveyor.com/api/projects/status/github/DiligentGraphics/DiligentTools?svg=true)](https://ci.appveyor.com/project/DiligentGraphics/diligenttools)
[![MSVC Code Analysis](https://github.com/DiligentGraphics/DiligentTools/actions/workflows/msvc_analysis.yml/badge.svg?branch=master)](https://github.com/DiligentGraphics/DiligentTools/actions/workflows/msvc_analysis.yml?query=branch%3Amaster)
[![Lines of Code](https://tokei.rs/b1/github.com/DiligentGraphics/DiligentTools)](https://github.com/DiligentGraphics/DiligentTools)

# License

See [Apache 2.0 license](License.txt).

This project has some third-party dependencies, each of which may have independent licensing:

* [args](https://github.com/Taywee/args): A simple header-only C++ argument parser library. ([MIT License](https://github.com/DiligentGraphics/args/blob/master/LICENSE)).
* [libjpeg](http://libjpeg.sourceforge.net/): C library for reading and writing JPEG image files ([JPEG Group's open source license](https://github.com/DiligentGraphics/DiligentTools/blob/master/ThirdParty/libjpeg-9a/README)).
* [libtiff](http://www.libtiff.org/): TIFF Library and Utilities ([Sam Leffler and Silicon Graphics, Inc. MIT-like license](https://github.com/DiligentGraphics/DiligentTools/blob/master/ThirdParty/libtiff/COPYRIGHT)).
* [libpng](http://www.libpng.org/pub/png/libpng.html): Official PNG reference library ([libpng license](https://github.com/DiligentGraphics/DiligentTools/blob/master/ThirdParty/lpng-1.6.17/LICENSE)).
* [zlib](https://zlib.net/): A compression library ([Jean-loup Gailly and Mark Adler MIT-like license](https://github.com/DiligentGraphics/DiligentTools/blob/master/ThirdParty/zlib-1.2.8/README)).
* [tinygltf](https://github.com/syoyo/tinygltf): A header only C++11 glTF 2.0 library ([MIT License](https://github.com/DiligentGraphics/DiligentTools/blob/master/ThirdParty/tinygltf/LICENSE)).
* [draco](https://github.com/google/draco): A library for compressing and decompressing 3D geometric meshes and point clouds. ([Apache License 2.0](https://github.com/google/draco/blob/master/LICENSE)).
* [imgui](https://github.com/ocornut/imgui): Immediate Mode Graphical User interface for C++ with minimal dependencies ([MIT license](https://github.com/DiligentGraphics/imgui/blob/master/LICENSE.txt)).
* [imGuIZMO.quat](https://github.com/BrutPitt/imGuIZMO.quat): ImGui GIZMO widget - 3D object manipulator / orientator ([BSD 2-Clause License](https://github.com/DiligentGraphics/DiligentTools/blob/master/ThirdParty/imGuIZMO.quat/license.txt)).
* [stb](https://github.com/nothings/stb): stb single-file public domain libraries for C/C++ ([MIT License or Public Domain License](https://github.com/DiligentGraphics/DiligentTools/blob/master/ThirdParty/stb/LICENSE.txt)).
* [json](https://github.com/nlohmann/json): JSON for Modern C++ ([MIT License](https://github.com/DiligentGraphics/DiligentTools/blob/master/ThirdParty/json/LICENSE.MIT)).

<a name="contributing"></a>
# Contributing

To contribute your code, submit a [Pull Request](https://github.com/DiligentGraphics/DiligentTools/pulls) 
to this repository. **Diligent Engine** is distributed under the [Apache 2.0 license](License.txt) that guarantees 
that content in the **DiligentTools** repository is free of Intellectual Property encumbrances.
In submitting any content to this repository,
[you license that content under the same terms](https://docs.github.com/en/free-pro-team@latest/github/site-policy/github-terms-of-service#6-contributions-under-repository-license),
and you agree that the content is free of any Intellectual Property claims and you have the right to license it under those terms. 

Diligent Engine uses [clang-format](https://clang.llvm.org/docs/ClangFormat.html) to ensure
consistent source code style throughout the code base. The format is validated by CI
for each commit and pull request, and the build will fail if any code formatting issue is found. Please refer
to [this page](https://github.com/DiligentGraphics/DiligentCore/blob/master/doc/code_formatting.md) for instructions
on how to set up clang-format and automatic code formatting.

------------------------------

[diligentgraphics.com](http://diligentgraphics.com)

[![Diligent Engine on Twitter](https://github.com/DiligentGraphics/DiligentCore/blob/master/media/twitter.png)](https://twitter.com/diligentengine)
[![Diligent Engine on Facebook](https://github.com/DiligentGraphics/DiligentCore/blob/master/media/facebook.png)](https://www.facebook.com/DiligentGraphics/)
