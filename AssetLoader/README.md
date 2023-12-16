# Asset Loader

The asset loading library currently supports GLTF 2.0 format.

## GLTF 2.0

![](media/flight_helmet.jpg)

GLTF loader uses [tiny gltf](https://github.com/syoyo/tinygltf) library and is based on
[Vulkan-glTF-PBR](https://github.com/SaschaWillems/Vulkan-glTF-PBR) project by [Sascha Willems](https://github.com/SaschaWillems).

The following features are currently supported:

* ASCII, Binary, and Embedded GLTF specifications
* Draco Mesh Compression (automatically enabled when Draco is included into the project)
* PBR Materials (Metallic-Roughness and Specular-Glossiness workflows)
* Skinning
* Extensions:
  * [KHR_materials_anisotropy](https://github.com/KhronosGroup/glTF/tree/main/extensions/2.0/Khronos/KHR_materials_anisotropy)
  * [KHR_materials_clearcoat](https://github.com/KhronosGroup/glTF/tree/main/extensions/2.0/Khronos/KHR_materials_clearcoat)
  * [KHR_materials_sheen](https://github.com/KhronosGroup/glTF/tree/main/extensions/2.0/Khronos/KHR_materials_sheen)
  * [KHR_materials_iridescence](https://github.com/KhronosGroup/glTF/tree/main/extensions/2.0/Khronos/KHR_materials_iridescence)
  * [KHR_materials_emissive_strength](https://github.com/KhronosGroup/glTF/tree/main/extensions/2.0/Khronos/KHR_materials_emissive_strength)
  * [KHR_materials_transmission](https://github.com/KhronosGroup/glTF/tree/main/extensions/2.0/Khronos/KHR_materials_transmission)
  * [KHR_materials_volume](https://github.com/KhronosGroup/glTF/tree/main/extensions/2.0/Khronos/KHR_materials_volume)
  * [KHR_materials_unlit](https://github.com/KhronosGroup/glTF/tree/main/extensions/2.0/Khronos/KHR_materials_unlit)
  * [KHR_texture_transform](https://github.com/KhronosGroup/glTF/tree/main/extensions/2.0/Khronos/KHR_texture_transform)

The loading functionality is implemented in `Diligent::GLTF::Model` class
that initializes all Diligent Engine objects required to render the model.

The following code snippet shows a basic usage of the loader:

```cpp
GLTF::ModelCreateInfo ModelCI;
ModelCI.FileName = "MyAsset.gltf";

m_Model = std::make_unique<GLTF::Model>(m_pDevice, m_pImmediateContext, ModelCI);
```

The loader is very flexible and provides multiple ways to customize the loading process.
Among others, the following parameters can be specified:

* Texture attribute configuration
* Vertex layout
* Node, Mesh, Primitive, and Material loading callbacks
* GPU resource cache

The loader does have any rendering capabilities. Please see
[Diligent GLTF PBR Renderer](https://github.com/DiligentGraphics/DiligentFX/tree/master/PBR).

## References

[GLTF2.0 Format Specification](https://github.com/KhronosGroup/glTF)

[Vulkan-glTF-PBR](https://github.com/SaschaWillems/Vulkan-glTF-PBR)

[tinygltf](https://github.com/syoyo/tinygltf)
