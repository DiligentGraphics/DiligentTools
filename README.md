Tools module provides additional functionality on top of the [Diligent Engine](https://github.com/DiligentGraphics/DiligentEngine)'s core module. It implements render script and texture loading utilities. For more information, please visit [diligentgraphics.com/diligent-engine](http://diligentgraphics.com/diligent-engine/).

To build the module, see [build instrcutions](https://github.com/DiligentGraphics/DiligentEngine/blob/master/README.md) in the master repository.

## Version History

### v2.3

* Implemented Vulkan backend

### v2.2

* Added MacOS and iOS support

### v2.1.b

* Removed legacy VS projects and solutions
* Added HLSL2GLSLConverter project

### v2.1.a

* Refactored build system to use CMake
* Added support for Linux platform

### v2.0.alpha

Alpha release of Diligent Engine 2.0. The engine has been updated to take advantages of Direct3D12:

* Pipeline State Object encompasses all coarse-grain state objects like Depth-Stencil State, Blend State, Rasterizer State, shader states etc.
* New shader resource binding model implemented to leverage Direct3D12

Release notes:

* Diligent Engine 2.0 also implements OpenGL and Direct3D11 back-ends
* Alpha release is only available on Windows platform
* Direct3D11 back-end is very thoroughly optimized and has very low overhead compared to native D3D11 implementation
* Direct3D12 implementation, to the contrary, is preliminary and not yet optimized.

### v1.0.0

Initial release

For more details on the release, please visit [diligentgraphics.com](http://diligentgraphics.com/2016/03/17/diligent-engine-2-0-powered-by-direct3d12/)

## License

Copyright 2015-2018 Egor Yusov.
Licensed under the [Apache License, Version 2.0](License.txt)
