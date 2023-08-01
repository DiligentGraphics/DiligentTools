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
#include "../../../DiligentCore/Graphics/GraphicsEngine/interface/GraphicsTypesX.hpp"
#include "../../../DiligentCore/Common/interface/RefCntAutoPtr.hpp"
#include "../../../DiligentCore/Common/interface/AdvancedMath.hpp"
#include "../../../DiligentCore/Common/interface/STDAllocator.hpp"
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

enum IMAGE_FILE_FORMAT : Uint8;

namespace GLTF
{

class ModelBuilder;

/// Texture attribute description.
struct TextureAttributeDesc
{
    /// Texture attribute name (e.g. "baseColorTexture", "metallicRoughnessTexture", etc.)
    const char* Name = nullptr;

    /// Texture attribute index in Material.ShaderAttribs (e.g. UVSelectorX, TextureSliceX, UVScaleBias[X]).
    Uint32 Index = 0;
};

static constexpr char BaseColorTextureName[]          = "baseColorTexture";
static constexpr char MetallicRoughnessTextureName[]  = "metallicRoughnessTexture";
static constexpr char NormalTextureName[]             = "normalTexture";
static constexpr char OcclusionTextureName[]          = "occlusionTexture";
static constexpr char EmissiveTextureName[]           = "emissiveTexture";
static constexpr char DiffuseTextureName[]            = "diffuseTexture";
static constexpr char SpecularGlossinessTextureName[] = "specularGlossinessTexture";

static constexpr Uint32 DefaultBaseColorTextureAttribId         = 0;
static constexpr Uint32 DefaultMetallicRoughnessTextureAttribId = 1;
static constexpr Uint32 DefaultNormalTextureAttribId            = 2;
static constexpr Uint32 DefaultOcclusionTextureAttribId         = 3;
static constexpr Uint32 DefaultEmissiveTextureAttribId          = 4;
static constexpr Uint32 DefaultDiffuseTextureAttribId           = 0;
static constexpr Uint32 DefaultSpecularGlossinessTextureAttibId = 1;

// clang-format off
static constexpr std::array<TextureAttributeDesc, 7> DefaultTextureAttributes =
    {
        // Metallic-roughness
        TextureAttributeDesc{BaseColorTextureName,         DefaultBaseColorTextureAttribId},
        TextureAttributeDesc{MetallicRoughnessTextureName, DefaultMetallicRoughnessTextureAttribId},
        TextureAttributeDesc{NormalTextureName,            DefaultNormalTextureAttribId},
        TextureAttributeDesc{OcclusionTextureName,         DefaultOcclusionTextureAttribId},
        TextureAttributeDesc{EmissiveTextureName,          DefaultEmissiveTextureAttribId},

        // Specular-glossiness
        TextureAttributeDesc{DiffuseTextureName,            DefaultDiffuseTextureAttribId},
        TextureAttributeDesc{SpecularGlossinessTextureName, DefaultSpecularGlossinessTextureAttibId}
    };
// clang-format on


struct Material
{
    Material() noexcept
    {
        TextureIds.fill(-1);
        for (size_t i = 0; i < _countof(Attribs.UVScaleBias); ++i)
            Attribs.UVScaleBias[i] = float4{1, 1, 0, 0};
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

    static constexpr Uint32 NumTextureAttributes = 5;

    // Material attributes packed in a shader-friendly format
    struct ShaderAttribs
    {
        float4 BaseColorFactor = float4{1, 1, 1, 1};
        float4 EmissiveFactor  = float4{0, 0, 0, 0};
        float4 SpecularFactor  = float4{1, 1, 1, 1};

        int   Workflow    = PBR_WORKFLOW_METALL_ROUGH;
        float UVSelector0 = -1;
        float UVSelector1 = -1;
        float UVSelector2 = -1;

        float UVSelector3   = -1;
        float UVSelector4   = -1;
        float TextureSlice0 = 0;
        float TextureSlice1 = 0;

        float TextureSlice2  = 0;
        float TextureSlice3  = 0;
        float TextureSlice4  = 0;
        float MetallicFactor = 1;

        float RoughnessFactor = 1;
        int   AlphaMode       = ALPHA_MODE_OPAQUE;
        float AlphaCutoff     = 0.5f;
        float Dummy0          = 0;

        // When texture atlas is used, UV scale and bias is applied to
        // each texture coordinate set.
        float4 UVScaleBias[NumTextureAttributes];

        // Any user-specific data
        float4 CustomData = float4{0, 0, 0, 0};
    };
    static_assert(sizeof(ShaderAttribs) % 16 == 0, "ShaderAttribs struct must be 16-byte aligned");
    ShaderAttribs Attribs;

    bool DoubleSided = false;

    // Texture indices in Model.Textures array, for each attribute.
    //  _________________            _______________________         __________________
    // |                 |          |                       |       |                   |
    // |   GLTF Model    |          |       Material        |       |       Model       |
    // |                 |          |                       |       |                   |
    // |                 |          |      TextureIds       |       |     Textures      |
    // | "normalTexture" |          | [   |   | 3 |   |   ] |       | [   |   |   |   ] |
    // |      |          |          |          A |          |       |               A   |
    // |      |_ _ _ _ _ |_ _ _2_ _ |_ _ _ _ _ | |_ _ _ _ __|_ _3_ _|_ _ _ _ _ _ _ _|   |
    // |                 |     A    |                       |       |                   |
    // |_________________|     |    |_______________________|       |___________________|
    //                         |
    //                    Defined by
    //              ModeCI.TextureAttributes
    //
    std::array<int, NumTextureAttributes> TextureIds = {};
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
              const float3& _BBMin,
              const float3& _BBMax) :
        FirstIndex{_FirstIndex},
        IndexCount{_IndexCount},
        VertexCount{_VertexCount},
        MaterialId{_MaterialId},
        BB{_BBMin, _BBMax}
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
    std::string            Name;
    std::vector<Primitive> Primitives;
    BoundBox               BB;

    // Any user-specific data. One way to set this field is from the
    // MeshLoadCallback.
    RefCntAutoPtr<IObject> pUserData;

    // There may be no primitives in the mesh, in which
    // case the bounding box will be invalid.
    bool IsValidBB() const
    {
        return !Primitives.empty();
    }
};

struct Node;
struct Skin
{
    std::string              Name;
    const Node*              pSkeletonRoot = nullptr;
    std::vector<float4x4>    InverseBindMatrices;
    std::vector<const Node*> Joints;
};

struct Camera
{
    std::string Name;

    enum class Projection
    {
        Unknown,
        Perspective,
        Orthographic
    } Type = Projection::Unknown;

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
};

struct Node
{
    // Index in Model.LinearNodes array.
    const int Index;

    // Index in ModelTransforms.Skins array.
    int SkinTransformsIndex = -1;

    std::string Name;

    const Node* Parent = nullptr;

    std::vector<const Node*> Children;

    const Mesh*   pMesh   = nullptr;
    const Camera* pCamera = nullptr;
    const Skin*   pSkin   = nullptr;

    float3      Translation;
    QuaternionF Rotation;
    float3      Scale  = float3{1, 1, 1};
    float4x4    Matrix = float4x4::Identity();

    explicit Node(int _Index) :
        Index{_Index}
    {}
};

struct Scene
{
    std::string        Name;
    std::vector<Node*> RootNodes;
    // Linear list of all nodes in the scene.
    std::vector<Node*> LinearNodes;
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

InputLayoutDescX VertexAttributesToInputLayout(const VertexAttributeDesc* pAttributes, size_t NumAttributes);


struct TextureCacheType
{
    std::mutex TexturesMtx;

    std::unordered_map<std::string, RefCntWeakPtr<ITexture>> Textures;
};

/// Model create information
struct ModelCreateInfo
{
    /// File name.
    const char* FileName = nullptr;

    /// Optional texture cache to use when loading the model.
    /// The loader will try to find all the textures in the cache
    /// and add all new textures to the cache.
    TextureCacheType* pTextureCache = nullptr;

    /// Optional resource manager to use when allocating resources for the model.
    ResourceManager* pResourceManager = nullptr;

    using MeshLoadCallbackType = std::function<void(const void*, Mesh&)>;
    /// User-provided mesh loading callback function that will be called for
    /// every mesh being loaded.
    MeshLoadCallbackType MeshLoadCallback = nullptr;

    using PrimitiveLoadCallbackType = std::function<void(const void*, Primitive&)>;
    /// User-provided primitive loading callback function that will be called for
    /// every primitive being loaded.
    PrimitiveLoadCallbackType PrimitiveLoadCallback = nullptr;

    using MaterialLoadCallbackType = std::function<void(const void*, Material&)>;
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

    static constexpr Uint32 MaxBuffers = 8;

    /// Vertex buffer bind flags for each buffer slot.
    BIND_FLAGS VertBufferBindFlags[MaxBuffers] = {};

    /// A pointer to the array of NumVertexAttributes vertex attributes defining
    /// the vertex layout.
    ///
    /// \remarks    If null is provided, default vertex attributes will be used (see DefaultVertexAttributes).
    const VertexAttributeDesc* VertexAttributes = nullptr;

    /// The number of elements in the VertexAttributes array.
    Uint32 NumVertexAttributes = 0;

    /// A pointer to the array of NumTextureAttributes texture attributes.
    ///
    /// \remarks    If null is provided, default vertex attributes will be used (see DefaultTextureAttributes).
    const TextureAttributeDesc* TextureAttributes = nullptr;

    /// The number of elements in the TextureAttributes array.
    Uint32 NumTextureAttributes = 0;

    /// Index of the scene to load. If -1, default scene will be loaded.
    Int32 SceneId = -1;

    ModelCreateInfo() = default;

    explicit ModelCreateInfo(const char*                _FileName,
                             TextureCacheType*          _pTextureCache         = nullptr,
                             ResourceManager*           _pResourceManager      = nullptr,
                             MeshLoadCallbackType       _MeshLoadCallback      = nullptr,
                             MaterialLoadCallbackType   _MaterialLoadCallback  = nullptr,
                             FileExistsCallbackType     _FileExistsCallback    = nullptr,
                             ReadWholeFileCallbackType  _ReadWholeFileCallback = nullptr,
                             const VertexAttributeDesc* _VertexAttributes      = nullptr,
                             Uint32                     _NumVertexAttributes   = 0) :
        // clang-format off
            FileName             {_FileName},
            pTextureCache        {_pTextureCache},
            pResourceManager     {_pResourceManager},
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

struct ModelTransforms
{
    // Transform matrices for each node in the model.
    std::vector<float4x4> NodeLocalMatrices;
    std::vector<float4x4> NodeGlobalMatrices;

    struct SkinTransforms
    {
        std::vector<float4x4> JointMatrices;
    };
    std::vector<SkinTransforms> Skins;

    // Animation transforms for each node in the model.
    // This is an intermediate data to compute transform matrices.
    struct AnimationTransforms
    {
        float3      Translation;
        float3      Scale{1, 1, 1};
        QuaternionF Rotation;
    };
    std::vector<AnimationTransforms> NodeAnimations;
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

    std::vector<Scene>       Scenes;
    std::vector<Node>        Nodes;
    std::vector<Mesh>        Meshes;
    std::vector<Camera>      Cameras;
    std::vector<Skin>        Skins;
    std::vector<Material>    Materials;
    std::vector<Animation>   Animations;
    std::vector<std::string> Extensions;

    std::vector<RefCntAutoPtr<ISampler>> TextureSamplers;

    // The number of nodes that have skin.
    int SkinTransformsCount = 0;
    int DefaultSceneId      = 0;

    Model(const ModelCreateInfo& CI);

    Model(IRenderDevice*         pDevice,
          IDeviceContext*        pContext,
          const ModelCreateInfo& CI);

    Model() noexcept;

    ~Model();

    void PrepareGPUResources(IRenderDevice* pDevice, IDeviceContext* pCtx);

    bool IsGPUDataInitialized() const
    {
        return GPUDataInitialized.load();
    }

    IBuffer* GetVertexBuffer(Uint32 Index, IRenderDevice* pDevice = nullptr, IDeviceContext* pCtx = nullptr) const
    {
        VERIFY_EXPR(Index < GetVertexBufferCount());
        return VertexData.pAllocation != nullptr ?
            VertexData.pAllocation->GetBuffer(Index, pDevice, pCtx) :
            VertexData.Buffers[Index];
    }

    IBuffer* GetIndexBuffer(IRenderDevice* pDevice = nullptr, IDeviceContext* pCtx = nullptr) const
    {
        return IndexData.pAllocation ?
            IndexData.pAllocation->GetBuffer(pDevice, pCtx) :
            IndexData.pBuffer;
    }

    ITexture* GetTexture(Uint32 Index, IRenderDevice* pDevice = nullptr, IDeviceContext* pCtx = nullptr) const
    {
        auto& TexInfo = Textures[Index];

        if (TexInfo.pTexture)
            return TexInfo.pTexture;

        if (TexInfo.pAtlasSuballocation)
        {
            if (auto* pAtlas = TexInfo.pAtlasSuballocation->GetAtlas())
                return pAtlas->GetTexture(pDevice, pCtx);
            else
                UNEXPECTED("Texture altas can't be null");
        }

        return nullptr;
    }

    Uint32 GetFirstIndexLocation() const
    {
        VERIFY(IndexData.IndexSize != 0, "Index size is not initialized");
        if (IndexData.pAllocation)
        {
            const auto Offset = IndexData.pAllocation->GetOffset();
            VERIFY((Offset % IndexData.IndexSize) == 0, "Index data allocation offset is not a multiple of index size (", IndexData.IndexSize, ")");
            return Offset / IndexData.IndexSize;
        }

        return 0;
    }

    Uint32 GetBaseVertex() const
    {
        return VertexData.pAllocation ?
            VertexData.pAllocation->GetStartVertex() :
            0;
    }

    struct ImageData
    {
        int Width         = 0;
        int Height        = 0;
        int NumComponents = 0;
        int ComponentSize = 0;

        TEXTURE_FORMAT    TexFormat = TEX_FORMAT_UNKNOWN;
        IMAGE_FILE_FORMAT FileFormat{};

        // Pixels are tightly packed.
        const void* pData    = nullptr;
        size_t      DataSize = 0;
    };
    Uint32 AddTexture(IRenderDevice*     pDevice,
                      TextureCacheType*  pTextureCache,
                      ResourceManager*   pResourceMgr,
                      const ImageData&   Image,
                      int                GltfSamplerId,
                      const std::string& CacheId);

    auto GetNumVertexAttributes() const { return NumVertexAttributes; }
    auto GetNumTextureAttributes() const { return NumTextureAttributes; }

    const auto& GetVertexAttribute(size_t Idx) const
    {
        VERIFY_EXPR(Idx < GetNumVertexAttributes());
        return VertexAttributes[Idx];
    }

    const auto& GetTextureAttribute(size_t Idx) const
    {
        VERIFY_EXPR(Idx < GetNumTextureAttributes());
        return TextureAttributes[Idx];
    }

    /// Returns the material texture attribute index in Material.ShaderAttribs for
    /// the given texture attribute name, or -1 if the attribute is not defined.
    /// For example, for default attributes:
    ///     "baseColorTexture"         -> 0
    ///     "metallicRoughnessTexture" -> 1
    ///     "normalTexture"            -> 2
    ///
    /// \note This index is NOT the texture index in Textures array. To get this index,
    ///       use Material.TextureIds[TextureAttributeIndex].
    int GetTextureAttibuteIndex(const char* Name) const;

    bool CompatibleWithTransforms(const ModelTransforms& Transforms) const;

    void ComputeTransforms(Uint32           SceneIndex,
                           ModelTransforms& Transforms,
                           const float4x4&  RootTransform  = float4x4::Identity(),
                           Int32            AnimationIndex = -1,
                           float            Time           = 0) const;

    BoundBox ComputeBoundingBox(Uint32 SceneIndex, const ModelTransforms& Transforms) const;

    size_t GetTextureCount() const
    {
        return Textures.size();
    }

    size_t GetVertexBufferCount() const
    {
        return VertexData.Strides.size();
    }

    void InitMaterialTextureAddressingAttribs(Material& Mat, Uint32 TextureIndex);

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
    void UpdateAnimation(Uint32 SceneIndex, Uint32 AnimationIndex, float time, ModelTransforms& Transforms) const;

    // Returns the alpha cutoff value for the given texture.
    // TextureIdx is the texture index in the GLTF file and also the Textures array.
    float GetTextureAlphaCutoffValue(int TextureIdx) const;

private:
    std::atomic_bool GPUDataInitialized{false};

    std::unique_ptr<void, STDDeleter<void, IMemoryAllocator>> pAttributesData;

    const VertexAttributeDesc*  VertexAttributes  = nullptr;
    const TextureAttributeDesc* TextureAttributes = nullptr;

    Uint32 NumVertexAttributes  = 0;
    Uint32 NumTextureAttributes = 0;

    struct VertexDataInfo
    {
        std::vector<Uint32>                  Strides;
        std::vector<RefCntAutoPtr<IBuffer>>  Buffers;
        RefCntAutoPtr<IVertexPoolAllocation> pAllocation;
    };
    VertexDataInfo VertexData;

    struct IndexDataInfo
    {
        RefCntAutoPtr<IBuffer>              pBuffer;
        RefCntAutoPtr<IBufferSuballocation> pAllocation;

        Uint32 IndexSize = 0;
    };
    IndexDataInfo IndexData;

    struct TextureInfo
    {
        RefCntAutoPtr<ITexture>                   pTexture;
        RefCntAutoPtr<ITextureAtlasSuballocation> pAtlasSuballocation;

        explicit operator bool() const
        {
            return pTexture || pAtlasSuballocation;
        }
    };
    std::vector<TextureInfo> Textures;
};

} // namespace GLTF

} // namespace Diligent
