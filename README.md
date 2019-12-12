# DiligentTools <img src="https://github.com/DiligentGraphics/DiligentCore/blob/master/media/diligentgraphics-logo.png" height=64 align="right" valign="middle">

This module implements additional functionality on top of the [Diligent Engine](https://github.com/DiligentGraphics/DiligentEngine)'s core module
and contains the following libraries:

* [Texture loader](TextureLoader): a texture loading libary. The following formats are currently supported: jpg, png, tiff, dds, ktx.
* [Asset Loader](AssetLoader): an asset loading libary. The library currently supports GLTF 2.0.
* [Imgui](Imgui): implementation of [dear imgui](https://github.com/ocornut/imgui) with Diligent API.
* [NativeApp](NativeApp): implementation of native application on supported platforms.


To build the module, see [build instrcutions](https://github.com/DiligentGraphics/DiligentEngine/blob/master/README.md) in the master repository.


[![License](https://img.shields.io/badge/License-Apache%202.0-blue.svg)](License.txt)
[![Chat on gitter](https://badges.gitter.im/gitterHQ/gitter.png)](https://gitter.im/diligent-engine)
[![Build Status](https://ci.appveyor.com/api/projects/status/github/DiligentGraphics/DiligentTools?svg=true)](https://ci.appveyor.com/project/DiligentGraphics/diligenttools)
[![Build Status](https://travis-ci.org/DiligentGraphics/DiligentTools.svg?branch=master)](https://travis-ci.org/DiligentGraphics/DiligentTools)

# License

See [Apache 2.0 license](License.txt).

This project has some third-party dependencies, each of which may have independent licensing:

* [libjpeg](http://libjpeg.sourceforge.net/): C library for reading and writing JPEG image files.
* [libtiff](http://www.libtiff.org/): TIFF Library and Utilities.
* [libpng](http://www.libpng.org/pub/png/libpng.html): Official PNG reference library.
* [zlib](https://zlib.net/): A compression library.
* [tinygltf](https://github.com/syoyo/tinygltf): A header only C++11 glTF 2.0 library.
* [imgui](https://github.com/ocornut/imgui): Immediate Mode Graphical User interface for C++ with minimal dependencies.

<a name="contributing"></a>
# Contributing

To contribute your code, submit a [Pull Request](https://github.com/DiligentGraphics/DiligentTools/pulls) 
to this repository. **Diligent Engine** is distributed under the [Apache 2.0 license](License.txt) that guarantees 
that code in the **DiligentTools** repository is free of Intellectual Property encumbrances. In submitting code to
this repository, you are agreeing that the code is free of any Intellectual Property claims.

Diligent Engine uses [clang-format](https://clang.llvm.org/docs/ClangFormat.html) to ensure
consistent source code style throught the code base. The format is validated by appveyor and travis
for each commit and pull request, and the build will fail if any code formatting issue is found. Please refer
to [this page](https://github.com/DiligentGraphics/DiligentCore/blob/master/doc/code_formatting.md) for instructions
on how to set up clang-format and automatic code formatting.

------------------------------

[diligentgraphics.com](http://diligentgraphics.com)

[![Diligent Engine on Twitter](https://github.com/DiligentGraphics/DiligentCore/blob/master/media/twitter.png)](https://twitter.com/diligentengine)
[![Diligent Engine on Facebook](https://github.com/DiligentGraphics/DiligentCore/blob/master/media/facebook.png)](https://www.facebook.com/DiligentGraphics/)
