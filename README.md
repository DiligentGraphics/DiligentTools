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


To build the module, see [build instructions](https://github.com/DiligentGraphics/DiligentEngine/blob/master/README.md) in the master repository.

| Platform                                                                                                                                    |   Build Status                    |
| --------------------------------------------------------------------------------------------------------------------------------------------| --------------------------------- |
| <img src="https://github.com/DiligentGraphics/DiligentCore/blob/master/media/windows-logo.png" width=24 valign="middle"> Windows            | [![Build Status](https://github.com/DiligentGraphics/DiligentTools/workflows/Windows/badge.svg?branch=master)](https://github.com/DiligentGraphics/DiligentTools/actions/workflows/windows.yml?query=branch%3Amaster) |
| <img src="https://github.com/DiligentGraphics/DiligentCore/blob/master/media/uwindows-logo.png" width=24 valign="middle"> Universal Windows | [![Build Status](https://github.com/DiligentGraphics/DiligentTools/workflows/UWP/badge.svg?branch=master)](https://github.com/DiligentGraphics/DiligentTools/actions/workflows/uwp.yml?query=branch%3Amaster)         |
| <img src="https://github.com/DiligentGraphics/DiligentCore/blob/master/media/linux-logo.png" width=24 valign="middle"> Linux                | [![Build Status](https://github.com/DiligentGraphics/DiligentTools/workflows/Linux/badge.svg?branch=master)](https://github.com/DiligentGraphics/DiligentTools/actions/workflows/linux.yml?query=branch%3Amaster)     |
| <img src="https://github.com/DiligentGraphics/DiligentCore/blob/master/media/macos-logo.png" width=24 valign="middle"> MacOS                | [![Build Status](https://github.com/DiligentGraphics/DiligentTools/workflows/MacOS/badge.svg?branch=master)](https://github.com/DiligentGraphics/DiligentTools/actions/workflows/macos.yml?query=branch%3Amaster)     |
| <img src="https://github.com/DiligentGraphics/DiligentCore/blob/master/media/apple-logo.png" width=24 valign="middle"> iOS                  | [![Build Status](https://github.com/DiligentGraphics/DiligentTools/workflows/iOS/badge.svg?branch=master)](https://github.com/DiligentGraphics/DiligentTools/actions/workflows/ios.yml?query=branch%3Amaster)         |

[![License](https://img.shields.io/badge/License-Apache%202.0-blue.svg)](License.txt)
[![Chat on gitter](https://badges.gitter.im/gitterHQ/gitter.png)](https://gitter.im/diligent-engine)
[![Chat on Discord](https://img.shields.io/discord/730091778081947680?logo=discord)](https://discord.gg/t7HGBK7)
[![Build Status](https://ci.appveyor.com/api/projects/status/github/DiligentGraphics/DiligentTools?svg=true)](https://ci.appveyor.com/project/DiligentGraphics/diligenttools)
[![Build Status](https://travis-ci.org/DiligentGraphics/DiligentTools.svg?branch=master)](https://travis-ci.org/DiligentGraphics/DiligentTools)
[![Codacy Badge](https://api.codacy.com/project/badge/Grade/c67b3cb18bd44720a69360b4f83ad070)](https://www.codacy.com/manual/DiligentGraphics/DiligentTools?utm_source=github.com&amp;utm_medium=referral&amp;utm_content=DiligentGraphics/DiligentTools&amp;utm_campaign=Badge_Grade)
[![Lines of Code](https://tokei.rs/b1/github.com/DiligentGraphics/DiligentTools)](https://github.com/DiligentGraphics/DiligentTools)

# License

See [Apache 2.0 license](License.txt).

This project has some third-party dependencies, each of which may have independent licensing:

* [libjpeg](http://libjpeg.sourceforge.net/): C library for reading and writing JPEG image files ([JPEG Group's open source license](https://github.com/DiligentGraphics/DiligentTools/blob/master/ThirdParty/libjpeg-9a/README)).
* [libtiff](http://www.libtiff.org/): TIFF Library and Utilities ([Sam Leffler and Silicon Graphics, Inc. MIT-like license](https://github.com/DiligentGraphics/DiligentTools/blob/master/ThirdParty/libtiff/COPYRIGHT)).
* [libpng](http://www.libpng.org/pub/png/libpng.html): Official PNG reference library ([libpng license](https://github.com/DiligentGraphics/DiligentTools/blob/master/ThirdParty/lpng-1.6.17/LICENSE)).
* [zlib](https://zlib.net/): A compression library ([Jean-loup Gailly and Mark Adler MIT-like license](https://github.com/DiligentGraphics/DiligentTools/blob/master/ThirdParty/zlib-1.2.8/README)).
* [tinygltf](https://github.com/syoyo/tinygltf): A header only C++11 glTF 2.0 library ([MIT License](https://github.com/DiligentGraphics/DiligentTools/blob/master/ThirdParty/tinygltf/LICENSE)).
* [imgui](https://github.com/ocornut/imgui): Immediate Mode Graphical User interface for C++ with minimal dependencies ([MIT license](https://github.com/DiligentGraphics/imgui/blob/master/LICENSE.txt)).
* [imGuIZMO.quat](https://github.com/BrutPitt/imGuIZMO.quat): ImGui GIZMO widget - 3D object manipulator / orientator ([BSD 2-Clause License](https://github.com/DiligentGraphics/DiligentTools/blob/master/ThirdParty/imGuIZMO.quat/license.txt)).

<a name="contributing"></a>
# Contributing

To contribute your code, submit a [Pull Request](https://github.com/DiligentGraphics/DiligentTools/pulls) 
to this repository. **Diligent Engine** is distributed under the [Apache 2.0 license](License.txt) that guarantees 
that content in the **DiligentTools** repository is free of Intellectual Property encumbrances.
In submitting any content to this repository,
[you license that content under the same terms](https://docs.github.com/en/free-pro-team@latest/github/site-policy/github-terms-of-service#6-contributions-under-repository-license),
and you agree that the content is free of any Intellectual Property claims and you have the right to license it under those terms. 

Diligent Engine uses [clang-format](https://clang.llvm.org/docs/ClangFormat.html) to ensure
consistent source code style throughout the code base. The format is validated by appveyor and travis
for each commit and pull request, and the build will fail if any code formatting issue is found. Please refer
to [this page](https://github.com/DiligentGraphics/DiligentCore/blob/master/doc/code_formatting.md) for instructions
on how to set up clang-format and automatic code formatting.

------------------------------

[diligentgraphics.com](http://diligentgraphics.com)

[![Diligent Engine on Twitter](https://github.com/DiligentGraphics/DiligentCore/blob/master/media/twitter.png)](https://twitter.com/diligentengine)
[![Diligent Engine on Facebook](https://github.com/DiligentGraphics/DiligentCore/blob/master/media/facebook.png)](https://www.facebook.com/DiligentGraphics/)
