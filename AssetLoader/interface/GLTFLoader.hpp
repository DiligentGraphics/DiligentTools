/*
 *  Copyright 2019-2023 Diligent Graphics LLC
 *  Copyright 2015-2019 Egor Yusov
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 *  In no event and under no legal theory, whether in tort (including negligence),
 *  contract, or otherwise, unless required by applicable law (such as deliberate
 *  and grossly negligent acts) or agreed to in writing, shall any Contributor be
 *  liable for any damages, including any direct, indirect, special, incidental,
 *  or consequential damages of any character arising as a result of this License or
 *  out of the use or inability to use the software (including but not limited to damages
 *  for loss of goodwill, work stoppage, computer failure or malfunction, or any and
 *  all other commercial damages or losses), even if such Contributor has been advised
 *  of the possibility of such damages.
 */

#pragma once

#include <vector>
#include <array>
#include <cfloat>
#include <unordered_map>
#include <mutex>
#include <atomic>
#include <functional>
#include <string>
#include <limits>

#include "../../../DiligentCore/Graphics/GraphicsEngine/interface/RenderDevice.h"
#include "../../../DiligentCore/Graphics/GraphicsEngine/interface/DeviceContext.h"
#include "../../../DiligentCore/Common/interface/RefCntAutoPtr.hpp"
#include "../../../DiligentCore/Common/interface/AdvancedMath.hpp"
#include "GLTFResourceManager.hpp"

namespace tinygltf
{

class Node;
class Model;
struct Mesh;
struct Material;
struct Image;

} // namespace tinygltf

namespace Diligent
{

namespace GLTF
{

class ModelBuilder;

/// GLTF resource cache use information.
struct ResourceCacheUseInfo
{
    /// A pointer to the resource manager.
    ResourceManager* pResourceMgr = nullptr;

    /// Index to provide to the AllocateBufferSpace function when allocating space for the index buffer.
    Uint8 IndexBufferIdx = 0;

    /// Indices to provide to the AllocateBufferSpace function when allocating space for each vertex buffer.
    Uint8 VertexBufferIdx[8] = {};

    /// Base color texture format.
    TEXTURE_FORMAT BaseColorFormat = TEX_FORMAT_RGBA8_UNORM;

    /// Base color texture format for alpha-cut and alpha-blend materials.
    TEXTURE_FORMAT BaseColorAlphaFormat = TEX_FORMAT_RGBA8_UNORM;

    /// Physical descriptor texture format.
    TEXTURE_FORMAT PhysicalDescFormat = TEX_FORMAT_RGBA8_UNORM;

    /// Normal map format.
    TEXTURE_FORMAT NormalFormat = TEX_FORMAT_RGBA8_UNORM;

    /// Occlusion texture format.
    TEXTURE_FORMAT OcclusionFormat = TEX_FORMAT_RGBA8_UNORM;

    /// Emissive texture format.
    TEXTURE_FORMAT EmissiveFormat = TEX_FORMAT_RGBA8_UNORM;
};

struct Material
{
    Material() noexcept
    {
        TextureIds.fill(-1);
    }

    enum PBR_WORKFLOW
    {
        PBR_WORKFLOW_METALL_ROUGH = 0,
        PBR_WORKFLOW_SPEC_GLOSS
    };

    enum ALPHA_MODE
    {
        ALPHA_MODE_OPAQUE = 0,
        ALPHA_MODE_MASK,
        ALPHA_MODE_BLEND,
        ALPHA_MODE_NUM_MODES
    };

    // Material attributes packed in a shader-friendly format
    struct ShaderAttribs
    {
        float4 BaseColorFactor = float4{1, 1, 1, 1};
        float4 EmissiveFactor  = float4{1, 1, 1, 1};
        float4 SpecularFactor  = float4{1, 1, 1, 1};

        int   Workflow                     = PBR_WORKFLOW_METALL_ROUGH;
        float BaseColorUVSelector          = -1;
        float PhysicalDescriptorUVSelector = -1;
        float NormalUVSelector             = -1;

        float OcclusionUVSelector     = -1;
        float EmissiveUVSelector      = -1;
        float BaseColorSlice          = 0;
        float PhysicalDescriptorSlice = 0;

        float NormalSlice    = 0;
        float OcclusionSlice = 0;
        float EmissiveSlice  = 0;
        float MetallicFactor = 1;

        float RoughnessFactor = 1;
        int   AlphaMode       = ALPHA_MODE_OPAQUE;
        float AlphaCutoff     = 0.5f;
        float Dummy0          = 0;

        // When texture atlas is used, UV scale and bias applied to
        // each texture coordinate set
        float4 BaseColorUVScaleBias          = float4{1, 1, 0, 0};
        float4 PhysicalDescriptorUVScaleBias = float4{1, 1, 0, 0};
        float4 NormalUVScaleBias             = float4{1, 1, 0, 0};
        float4 OcclusionUVScaleBias          = float4{1, 1, 0, 0};
        float4 EmissiveUVScaleBias           = float4{1, 1, 0, 0};

        // Any user-specific data
        float4 CustomData = float4{0, 0, 0, 0};
    };
    static_assert(sizeof(ShaderAttribs) % 16 == 0, "ShaderAttribs struct must be 16-byte aligned");
    ShaderAttribs Attribs;

    bool DoubleSided = false;

    enum TEXTURE_ID
    {
        // Base color for metallic-roughness workflow or
        // diffuse color for specular-glossinees workflow
        TEXTURE_ID_BASE_COLOR = 0,

        // Metallic-roughness or specular-glossinees map
        TEXTURE_ID_PHYSICAL_DESC,

        TEXTURE_ID_NORMAL_MAP,
        TEXTURE_ID_OCCLUSION,
        TEXTURE_ID_EMISSIVE,
        TEXTURE_ID_NUM_TEXTURES
    };
    // Texture indices in Model.Textures array
    std::array<int, TEXTURE_ID_NUM_TEXTURES> TextureIds = {};
};


struct Primitive
{
    const Uint32 FirstIndex;
    const Uint32 IndexCount;
    const Uint32 VertexCount;
    const Uint32 MaterialId;

    const BoundBox BB;

    Primitive(Uint32        _FirstIndex,
              Uint32        _IndexCount,
              Uint32        _VertexCount,
              Uint32        _MaterialId,
              const float3& BBMin,
              const float3& BBMax) :
        FirstIndex{_FirstIndex},
        IndexCount{_IndexCount},
        VertexCount{_VertexCount},
        MaterialId{_MaterialId},
        BB{BBMin, BBMax}
    {
    }

    Primitive(Primitive&&) = default;

    bool HasIndices() const
    {
        return IndexCount > 0;
    }
};



struct Mesh
{
    std::vector<Primitive> Primitives;

    BoundBox BB;

    // There may be no primitives in the mesh, in which
    // case the bounding box will be invalid.
    bool IsValidBB() const
    {
        return !Primitives.empty();
    }

    struct TransformData
    {
        float4x4              matrix;
        std::vector<float4x4> jointMatrices;
    };

    TransformData Transforms;
};


struct Node;
struct Skin
{
    std::string           Name;
    Node*                 pSkeletonRoot = nullptr;
    std::vector<float4x4> InverseBindMatrices;
    std::vector<Node*>    Joints;
};

struct Camera
{
    enum class Projection
    {
        Unknown,
        Perspective,
        Orthographic
    } Type = Projection::Unknown;

    std::string Name;

    struct PerspectiveAttribs
    {
        float AspectRatio;
        float YFov;
        float ZNear;
        float ZFar;
    };
    struct OrthographicAttribs
    {
        float XMag;
        float YMag;
        float ZNear;
        float ZFar;
    };
    union
    {
        PerspectiveAttribs  Perspective = {};
        OrthographicAttribs Orthographic;
    };

    float4x4 matrix;
};

struct Node
{
    std::string Name;

    Node* Parent = nullptr;

    std::vector<Node*> Children;

    float4x4   Matrix;
    Mesh*      pMesh   = nullptr;
    Camera*    pCamera = nullptr;
    Skin*      pSkin   = nullptr;
    float3     Translation;
    float3     Scale = float3{1, 1, 1};
    Quaternion Rotation;
    BoundBox   BVH;
    BoundBox   AABB;

    bool IsValidBVH = false;

    float4x4 LocalMatrix() const;
    float4x4 GetMatrix() const;
    void     UpdateTransforms();
};


struct AnimationChannel
{
    enum class PATH_TYPE
    {
        TRANSLATION,
        ROTATION,
        SCALE,
        WEIGHTS
    };
    PATH_TYPE const PathType;
    Node* const     pNode;
    Uint32 const    SamplerIndex;

    AnimationChannel(PATH_TYPE _PathType,
                     Node*     _pNode,
                     Uint32    _SamplerIndex) :
        PathType{_PathType},
        pNode{_pNode},
        SamplerIndex{_SamplerIndex}
    {}
};


struct AnimationSampler
{
    enum class INTERPOLATION_TYPE
    {
        LINEAR,
        STEP,
        CUBICSPLINE
    };
    const INTERPOLATION_TYPE Interpolation;

    std::vector<float>  Inputs;
    std::vector<float4> OutputsVec4;

    explicit AnimationSampler(INTERPOLATION_TYPE _Interpolation) :
        Interpolation{_Interpolation}
    {}
};

struct Animation
{
    std::string                   Name;
    std::vector<AnimationSampler> Samplers;
    std::vector<AnimationChannel> Channels;

    float Start = +(std::numeric_limits<float>::max)();
    float End   = -(std::numeric_limits<float>::max)();
};



/// Vertex attribute description.
struct VertexAttributeDesc
{
    /// Attribute name ("POSITION", "NORMAL", "TEXCOORD_0", "TEXCOORD_1", "JOINTS_0", "WEIGHTS_0", etc.).
    const char* Name = nullptr;

    /// Index of the vertex buffer that stores this attribute.
    Uint8 BufferId = 0;

    /// The type of the attribute's components.
    VALUE_TYPE ValueType = VT_UNDEFINED;

    /// The number of components in the attribute.
    Uint8 NumComponents = 0;

    /// Relative offset, in bytes, from the start of the vertex data to the start of the attribute.
    /// If this value is set to 0xFFFFFFFF (the default value), the offset will
    /// be computed automatically by placing the attribute right after the previous one.
    Uint32 RelativeOffset = ~0U;

    constexpr VertexAttributeDesc() noexcept {}

    constexpr VertexAttributeDesc(const char* _Name,
                                  Uint8       _BufferId,
                                  VALUE_TYPE  _ValueType,
                                  Uint8       _NumComponents,
                                  Uint32      _RelativeOffset = VertexAttributeDesc{}.RelativeOffset) noexcept :
        Name{_Name},
        BufferId{_BufferId},
        ValueType{_ValueType},
        NumComponents{_NumComponents},
        RelativeOffset{_RelativeOffset}
    {}
};

/// Default vertex attributes.
// clang-format off
static constexpr std::array<VertexAttributeDesc, 6> DefaultVertexAttributes =
    {
        // VertexBasicAttribs
        VertexAttributeDesc{"POSITION",   0, VT_FLOAT32, 3},
        VertexAttributeDesc{"NORMAL",     0, VT_FLOAT32, 3},
        VertexAttributeDesc{"TEXCOORD_0", 0, VT_FLOAT32, 2},
        VertexAttributeDesc{"TEXCOORD_1", 0, VT_FLOAT32, 2},

        // VertexSkinAttribs
        VertexAttributeDesc{"JOINTS_0",  1, VT_FLOAT32, 4},
        VertexAttributeDesc{"WEIGHTS_0", 1, VT_FLOAT32, 4},
    };
// clang-format on


struct TextureCacheType
{
    std::mutex TexturesMtx;

    std::unordered_map<std::string, RefCntWeakPtr<ITexture>> Textures;
};

/// Model create information
struct ModelCreateInfo
{
    /// File name
    const char* FileName = nullptr;

    /// Optional texture cache to use when loading the model.
    /// The loader will try to find all the textures in the cache
    /// and add all new textures to the cache.
    TextureCacheType* pTextureCache = nullptr;

    /// Optional resource cache usage info.
    ResourceCacheUseInfo* pCacheInfo = nullptr;

    using MeshLoadCallbackType = std::function<void(const void*, Mesh&)>;
    /// User-provided mesh loading callback function that will be called for
    /// every mesh being loaded.
    MeshLoadCallbackType MeshLoadCallback = nullptr;

    using MaterialLoadCallbackType = std::function<void(const tinygltf::Material&, Material&)>;
    /// User-provided material loading callback function that will be called for
    /// every material being loaded.
    MaterialLoadCallbackType MaterialLoadCallback = nullptr;

    using FileExistsCallbackType = std::function<bool(const char* FilePath)>;
    /// Optional callback function that will be called by the loader to check if the file exists.
    FileExistsCallbackType FileExistsCallback = nullptr;

    using ReadWholeFileCallbackType = std::function<bool(const char* FilePath, std::vector<unsigned char>& Data, std::string& Error)>;
    /// Optional callback function that will be called by the loader to read the whole file.
    ReadWholeFileCallbackType ReadWholeFileCallback = nullptr;

    /// Index data type.
    VALUE_TYPE IndexType = VT_UINT32;

    /// Index buffer bind flags
    BIND_FLAGS IndBufferBindFlags = BIND_INDEX_BUFFER;

    /// Vertex buffer bind flags
    BIND_FLAGS VertBufferBindFlags = BIND_VERTEX_BUFFER;

    /// A pointer to the array of NumVertexAttributes vertex attributes defining
    /// the vertex layout.
    ///
    /// \remarks    If null is provided, default vertex attributes will be used (see DefaultVertexAttributes).
    const VertexAttributeDesc* VertexAttributes = nullptr;

    /// The number of elements in the VertexAttributes array.
    Uint32 NumVertexAttributes = 0;

    /// Index of the scene to load. If -1, default scene will be loaded.
    Int32 SceneId = -1;

    ModelCreateInfo() = default;

    explicit ModelCreateInfo(const char*                _FileName,
                             TextureCacheType*          _pTextureCache         = nullptr,
                             ResourceCacheUseInfo*      _pCacheInfo            = nullptr,
                             MeshLoadCallbackType       _MeshLoadCallback      = nullptr,
                             MaterialLoadCallbackType   _MaterialLoadCallback  = nullptr,
                             FileExistsCallbackType     _FileExistsCallback    = nullptr,
                             ReadWholeFileCallbackType  _ReadWholeFileCallback = nullptr,
                             const VertexAttributeDesc* _VertexAttributes      = nullptr,
                             Uint32                     _NumVertexAttributes   = 0) :
        // clang-format off
            FileName             {_FileName},
            pTextureCache        {_pTextureCache},
            pCacheInfo           {_pCacheInfo},
            MeshLoadCallback     {_MeshLoadCallback},
            MaterialLoadCallback {_MaterialLoadCallback},
            FileExistsCallback   {_FileExistsCallback},
            ReadWholeFileCallback{_ReadWholeFileCallback},
            VertexAttributes     {_VertexAttributes},
            NumVertexAttributes  {_NumVertexAttributes}
    // clang-format on
    {
    }
};

struct Model
{
    struct VertexBasicAttribs
    {
        float3 pos;
        float3 normal;
        float2 uv0;
        float2 uv1;
    };

    struct VertexSkinAttribs
    {
        float4 joint0;
        float4 weight0;
    };

    enum VERTEX_BUFFER_ID
    {
        VERTEX_BUFFER_ID_BASIC_ATTRIBS = 0,
        VERTEX_BUFFER_ID_SKIN_ATTRIBS,
    };

    /// Transformation matrix that transforms unit cube [0,1]x[0,1]x[0,1] into
    /// axis-aligned bounding box in model space.
    float4x4 AABBTransform;

    /// Node hierarchy.
    std::vector<Node*> RootNodes;

    std::vector<Node>        LinearNodes;
    std::vector<Mesh>        Meshes;
    std::vector<Camera>      Cameras;
    std::vector<Skin>        Skins;
    std::vector<Material>    Materials;
    std::vector<Animation>   Animations;
    std::vector<std::string> Extensions;

    std::vector<RefCntAutoPtr<ISampler>> TextureSamplers;

    struct Dimensions
    {
        float3 min = float3{+FLT_MAX, +FLT_MAX, +FLT_MAX};
        float3 max = float3{-FLT_MAX, -FLT_MAX, -FLT_MAX};
    } dimensions;

    Model(const ModelCreateInfo& CI);

    Model(IRenderDevice*         pDevice,
          IDeviceContext*        pContext,
          const ModelCreateInfo& CI);

    Model() noexcept;

    ~Model();

    void UpdateAnimation(Uint32 index, float time);

    void PrepareGPUResources(IRenderDevice* pDevice, IDeviceContext* pCtx);

    bool IsGPUDataInitialized() const
    {
        return GPUDataInitialized.load();
    }

    void Transform(const float4x4& Matrix);

    IBuffer* GetVertexBuffer(Uint32 Index)
    {
        VERIFY_EXPR(size_t{Index} + 1 < Buffers.size());
        return Buffers[Index].pBuffer;
    }

    IBuffer* GetIndexBuffer()
    {
        VERIFY_EXPR(!Buffers.empty());
        return Buffers.back().pBuffer;
    }

    ITexture* GetTexture(Uint32 Index)
    {
        return Textures[Index].pTexture;
    }

    Uint32 GetFirstIndexLocation() const
    {
        VERIFY_EXPR(!Buffers.empty());
        auto& IndBuff = Buffers.back();
        VERIFY(IndBuff.ElementStride != 0, "Index data stride is not initialized");
        VERIFY(!IndBuff.pSuballocation || (IndBuff.pSuballocation->GetOffset() % IndBuff.ElementStride) == 0,
               "Allocation offset is not multiple of index size (", IndBuff.ElementStride, ")");
        return IndBuff.pSuballocation ?
            static_cast<Uint32>(IndBuff.pSuballocation->GetOffset() / IndBuff.ElementStride) :
            0;
    }

    Uint32 GetBaseVertex(Uint32 Index = 0) const
    {
        VERIFY_EXPR(size_t{Index} + 1 < Buffers.size());
        auto& VertBuff = Buffers[Index];
        VERIFY(VertBuff.ElementStride != 0, "Vertex data stride is not initialized");
        VERIFY(!VertBuff.pSuballocation || (VertBuff.pSuballocation->GetOffset() % VertBuff.ElementStride) == 0,
               "Allocation offset is not multiple of the element stride (", VertBuff.ElementStride, ")");
        return VertBuff.pSuballocation ?
            static_cast<Uint32>(VertBuff.pSuballocation->GetOffset() / VertBuff.ElementStride) :
            0;
    }

    void AddTexture(IRenderDevice*                         pDevice,
                    TextureCacheType*                      pTextureCache,
                    ResourceManager*                       pResourceMgr,
                    const tinygltf::Image&                 gltf_image,
                    int                                    gltf_sampler,
                    const std::vector<tinygltf::Material>& gltf_materials,
                    const std::string&                     CacheId);

    const auto& GetVertexAttributes() const
    {
        return VertexAttributes;
    }

    void CalculateSceneDimensions();

private:
    friend ModelBuilder;

    void LoadFromFile(IRenderDevice*         pDevice,
                      IDeviceContext*        pContext,
                      const ModelCreateInfo& CI);

    void LoadTextures(IRenderDevice*         pDevice,
                      const tinygltf::Model& gltf_model,
                      const std::string&     BaseDir,
                      TextureCacheType*      pTextureCache,
                      ResourceManager*       pResourceMgr);

    void LoadTextureSamplers(IRenderDevice* pDevice, const tinygltf::Model& gltf_model);
    void LoadMaterials(const tinygltf::Model& gltf_model, const ModelCreateInfo::MaterialLoadCallbackType& MaterialLoadCallback);
    void CalculateBoundingBox(Node* node, const Node* parent);

    std::atomic_bool GPUDataInitialized{false};

    std::vector<VertexAttributeDesc> VertexAttributes;

    struct BufferInfo
    {
        RefCntAutoPtr<IBuffer>              pBuffer;
        RefCntAutoPtr<IBufferSuballocation> pSuballocation;

        Uint32 ElementStride = 0;
    };
    std::vector<BufferInfo> Buffers;

    struct TextureInfo
    {
        RefCntAutoPtr<ITexture>                   pTexture;
        RefCntAutoPtr<ITextureAtlasSuballocation> pAtlasSuballocation;

        bool IsValid() const
        {
            return pTexture || pAtlasSuballocation;
        }
    };
    std::vector<TextureInfo> Textures;
};

} // namespace GLTF

} // namespace Diligent
