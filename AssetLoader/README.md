# Asset Loader

The asset loading library currently supports GLTF 2.0 format.

## GLTF 2.0

![](media/flight_helmet.jpg)

GLTF loader uses [tiny gltf](https://github.com/syoyo/tinygltf) library and is based on
[Vulkan-glTF-PBR](https://github.com/SaschaWillems/Vulkan-glTF-PBR) project by [Sascha Willems](https://github.com/SaschaWillems).

The following features are currently supported:

* [X] ASCII, Binary, and Embedded GLTF specifications
* [X] Dracor Mesh Compression (automatically enabled when Draco is included into the project)
* [X] PBR Materials (Metallic-Roughness and Specular-Glossiness workflows)
* [X] Skinning

Extensions:

* [X] [KHR_materials_clearcoat](https://github.com/KhronosGroup/glTF/tree/main/extensions/2.0/Khronos/KHR_materials_clearcoat)
* [X] [KHR_materials_sheen](https://github.com/KhronosGroup/glTF/tree/main/extensions/2.0/Khronos/KHR_materials_sheen)
* [X] [KHR_materials_emissive_strength](https://github.com/KhronosGroup/glTF/tree/main/extensions/2.0/Khronos/KHR_materials_emissive_strength)
* [X] [KHR_materials_unlit](https://github.com/KhronosGroup/glTF/tree/main/extensions/2.0/Khronos/KHR_materials_unlit)
* [X] [KHR_texture_transform](https://github.com/KhronosGroup/glTF/tree/main/extensions/2.0/Khronos/KHR_texture_transform)

The loading functionality is implemented in `Diligent::GLTF::Model` class
that initializes all Diligent Engine objects required to render the model.

The loader does have any rendering capabilities. Please see
[Diligent GLTF PBR Renderer](https://github.com/DiligentGraphics/DiligentFX/tree/master/PBR).

## References

[GLTF2.0 Format Specification](https://github.com/KhronosGroup/glTF)

[Vulkan-glTF-PBR](https://github.com/SaschaWillems/Vulkan-glTF-PBR)

[tinygltf](https://github.com/syoyo/tinygltf)
