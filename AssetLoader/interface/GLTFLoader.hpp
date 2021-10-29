/*
 *  Copyright 2019-2021 Diligent Graphics LLC
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
#include <memory>
#include <cfloat>
#include <unordered_map>
#include <mutex>
#include <atomic>
#include <functional>

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

} // namespace tinygltf

namespace Diligent
{

namespace GLTF
{

struct ResourceCacheUseInfo
{
    ResourceManager* pResourceMgr = nullptr;

    Uint8 IndexBufferIdx   = 0;
    Uint8 VertexBuffer0Idx = 0;
    Uint8 VertexBuffer1Idx = 0;

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
        float Dummy0;

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

    Mesh(const float4x4& matrix);
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
    Node*       Parent = nullptr;
    Uint32      Index;

    std::vector<std::unique_ptr<Node>> Children;

    float4x4                Matrix;
    std::unique_ptr<Mesh>   pMesh;
    std::unique_ptr<Camera> pCamera;
    Skin*                   pSkin     = nullptr;
    Int32                   SkinIndex = -1;
    float3                  Translation;
    float3                  Scale = float3{1, 1, 1};
    Quaternion              Rotation;
    BoundBox                BVH;
    BoundBox                AABB;

    bool IsValidBVH = false;

    float4x4 LocalMatrix() const;
    float4x4 GetMatrix() const;
    void     UpdateTransforms();
};


struct AnimationChannel
{
    enum PATH_TYPE
    {
        TRANSLATION,
        ROTATION,
        SCALE
    };
    PATH_TYPE PathType;
    Node*     pNode        = nullptr;
    Uint32    SamplerIndex = static_cast<Uint32>(-1);
};


struct AnimationSampler
{
    enum INTERPOLATION_TYPE
    {
        LINEAR,
        STEP,
        CUBICSPLINE
    };
    INTERPOLATION_TYPE  Interpolation;
    std::vector<float>  Inputs;
    std::vector<float4> OutputsVec4;
};

struct Animation
{
    std::string                   Name;
    std::vector<AnimationSampler> Samplers;
    std::vector<AnimationChannel> Channels;

    float Start = std::numeric_limits<float>::max();
    float End   = std::numeric_limits<float>::min();
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

    enum BUFFER_ID
    {
        BUFFER_ID_VERTEX_BASIC_ATTRIBS = 0,
        BUFFER_ID_VERTEX_SKIN_ATTRIBS,
        BUFFER_ID_INDEX,
        BUFFER_ID_NUM_BUFFERS
    };

    Uint32 IndexCount = 0;

    /// Transformation matrix that transforms unit cube [0,1]x[0,1]x[0,1] into
    /// axis-aligned bounding box in model space.
    float4x4 AABBTransform;

    std::vector<std::unique_ptr<Node>> Nodes;
    std::vector<Node*>                 LinearNodes;

    std::vector<std::unique_ptr<Skin>> Skins;

    std::vector<RefCntAutoPtr<ISampler>> TextureSamplers;
    std::vector<Material>                Materials;
    std::vector<Animation>               Animations;
    std::vector<std::string>             Extensions;

    struct Dimensions
    {
        float3 min = float3{+FLT_MAX, +FLT_MAX, +FLT_MAX};
        float3 max = float3{-FLT_MAX, -FLT_MAX, -FLT_MAX};
    } dimensions;

    struct TextureCacheType
    {
        std::mutex TexturesMtx;

        std::unordered_map<std::string, RefCntWeakPtr<ITexture>> Textures;
    };

    /// Model create information
    struct CreateInfo
    {
        /// File name
        const char* FileName = nullptr;

        /// Optional texture cache to use when loading the model.
        /// The loader will try to find all the textures in the cache
        /// and add all new textures to the cache.
        TextureCacheType* pTextureCache = nullptr;

        /// Optional resource cache usage info.
        ResourceCacheUseInfo* pCacheInfo = nullptr;

        /// Whether to load animation and initialize skin attributes
        /// buffer.
        bool LoadAnimationAndSkin = true;

        using MeshLoadCallbackType = std::function<void(const tinygltf::Mesh&, Mesh&)>;
        /// User-provided mesh loading callback function that will be called for
        /// every mesh being loaded.
        MeshLoadCallbackType MeshLoadCallback = nullptr;

        using MaterialLoadCallbackType = std::function<void(const tinygltf::Material&, Material&)>;
        /// User-provided material loading callback function that will be called for
        /// every material being loaded.
        MaterialLoadCallbackType MaterialLoadCallback = nullptr;

        /// Index buffer bind flags
        BIND_FLAGS IndBufferBindFlags = BIND_INDEX_BUFFER;

        /// Vertex buffer bind flags
        BIND_FLAGS VertBufferBindFlags = BIND_VERTEX_BUFFER;

        CreateInfo() = default;

        explicit CreateInfo(const char*              _FileName,
                            TextureCacheType*        _pTextureCache        = nullptr,
                            ResourceCacheUseInfo*    _pCacheInfo           = nullptr,
                            bool                     _LoadAnimationAndSkin = true,
                            MeshLoadCallbackType     _MeshLoadCallback     = nullptr,
                            MaterialLoadCallbackType _MaterialLoadCallback = nullptr) :
            // clang-format off
            FileName            {_FileName},
            pTextureCache       {_pTextureCache},
            pCacheInfo          {_pCacheInfo},
            LoadAnimationAndSkin{_LoadAnimationAndSkin},
            MeshLoadCallback    {_MeshLoadCallback},
            MaterialLoadCallback{_MaterialLoadCallback}
        // clang-format on
        {
        }
    };
    Model(IRenderDevice*    pDevice,
          IDeviceContext*   pContext,
          const CreateInfo& CI);

    ~Model();

    void UpdateAnimation(Uint32 index, float time);

    void PrepareGPUResources(IRenderDevice* pDevice, IDeviceContext* pCtx);

    bool IsGPUDataInitialized() const
    {
        return GPUDataInitialized.load();
    }

    void Transform(const float4x4& Matrix);

    IBuffer* GetBuffer(BUFFER_ID BuffId)
    {
        return Buffers[BuffId].pBuffer;
    }

    ITexture* GetTexture(Uint32 Index)
    {
        return Textures[Index].pTexture;
    }

    Uint32 GetFirstIndexLocation() const
    {
        auto& IndBuff = Buffers[BUFFER_ID_INDEX];
        VERIFY(!IndBuff.pSuballocation || IndBuff.pSuballocation->GetOffset() % sizeof(Uint32) == 0,
               "Allocation offset is not multiple of sizeof(Uint32)");
        return IndBuff.pSuballocation ?
            static_cast<Uint32>(IndBuff.pSuballocation->GetOffset() / sizeof(Uint32)) :
            0;
    }

    Uint32 GetBaseVertex() const
    {
        auto& VertBuff = Buffers[BUFFER_ID_VERTEX_BASIC_ATTRIBS];
        VERIFY(!VertBuff.pSuballocation || VertBuff.pSuballocation->GetOffset() % sizeof(VertexBasicAttribs) == 0,
               "Allocation offset is not multiple of sizeof(VertexAttribs0)");
        return VertBuff.pSuballocation ?
            static_cast<Uint32>(VertBuff.pSuballocation->GetOffset() / sizeof(VertexBasicAttribs)) :
            0;
    }

private:
    void LoadFromFile(IRenderDevice*    pDevice,
                      IDeviceContext*   pContext,
                      const CreateInfo& CI);

    struct ConvertedBufferViewKey
    {
        int PosAccess    = -1;
        int UV0Access    = -1;
        int UV1Access    = -1;
        int NormAccess   = -1;
        int JointAccess  = -1;
        int WeightAccess = -1;

        bool operator==(const ConvertedBufferViewKey& Rhs) const;

        struct Hasher
        {
            size_t operator()(const ConvertedBufferViewKey& Key) const;
        };
    };

    struct ConvertedBufferViewData
    {
        size_t VertexBasicDataOffset = ~size_t(0);
        size_t VertexSkinDataOffset  = ~size_t(0);

        bool IsInitialized() const { return VertexBasicDataOffset != ~size_t(0); }
    };

    using ConvertedBufferViewMap = std::unordered_map<ConvertedBufferViewKey, ConvertedBufferViewData, ConvertedBufferViewKey::Hasher>;

    void LoadNode(Node*                                          parent,
                  const tinygltf::Node&                          gltf_node,
                  uint32_t                                       nodeIndex,
                  const tinygltf::Model&                         gltf_model,
                  std::vector<Uint32>&                           IndexData,
                  std::vector<VertexBasicAttribs>&               VertexBasicData,
                  std::vector<VertexSkinAttribs>*                pVertexSkinData,
                  const Model::CreateInfo::MeshLoadCallbackType& MeshLoadCallback,
                  ConvertedBufferViewMap&                        ConvertedBuffers);

    void ConvertBuffers(const ConvertedBufferViewKey&    Key,
                        ConvertedBufferViewData&         Data,
                        const tinygltf::Model&           gltf_model,
                        std::vector<VertexBasicAttribs>& VertexBasicData,
                        std::vector<VertexSkinAttribs>*  pVertexSkinData) const;

    void LoadSkins(const tinygltf::Model& gltf_model);

    void LoadTextures(IRenderDevice*         pDevice,
                      const tinygltf::Model& gltf_model,
                      const std::string&     BaseDir,
                      TextureCacheType*      pTextureCache,
                      ResourceManager*       pResourceMgr);

    void  LoadTextureSamplers(IRenderDevice* pDevice, const tinygltf::Model& gltf_model);
    void  LoadMaterials(const tinygltf::Model& gltf_model, const Model::CreateInfo::MaterialLoadCallbackType& MaterialLoadCallback);
    void  LoadAnimations(const tinygltf::Model& gltf_model);
    void  CalculateBoundingBox(Node* node, const Node* parent);
    void  CalculateSceneDimensions();
    Node* FindNode(Node* parent, Uint32 index);
    Node* NodeFromIndex(uint32_t index);

    std::atomic_bool GPUDataInitialized{false};

    struct BufferInfo
    {
        RefCntAutoPtr<IBuffer>              pBuffer;
        RefCntAutoPtr<IBufferSuballocation> pSuballocation;
    };
    std::array<BufferInfo, BUFFER_ID_NUM_BUFFERS> Buffers;

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
