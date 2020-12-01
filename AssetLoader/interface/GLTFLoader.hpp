/*
 *  Copyright 2019-2020 Diligent Graphics LLC
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

#include "../../../DiligentCore/Graphics/GraphicsEngine/interface/RenderDevice.h"
#include "../../../DiligentCore/Graphics/GraphicsEngine/interface/DeviceContext.h"
#include "../../../DiligentCore/Common/interface/RefCntAutoPtr.hpp"
#include "../../../DiligentCore/Common/interface/AdvancedMath.hpp"
#include "GLTFResourceManager.hpp"

namespace tinygltf
{

class Node;
class Model;

} // namespace tinygltf

namespace Diligent
{

namespace GLTF
{

struct GLTFCacheInfo
{
    GLTFResourceManager* pResourceMgr = nullptr;

    Uint8 IndexBufferIdx   = 0;
    Uint8 VertexBuffer0Idx = 0;
    Uint8 VertexBuffer1Idx = 0;
};

struct Material
{
    Material() noexcept {}

    enum ALPHA_MODE
    {
        ALPHAMODE_OPAQUE,
        ALPHAMODE_MASK,
        ALPHAMODE_BLEND
    };
    ALPHA_MODE AlphaMode = ALPHAMODE_OPAQUE;

    bool DoubleSided = false;

    float AlphaCutoff     = 1.0f;
    float MetallicFactor  = 1.0f;
    float RoughnessFactor = 1.0f;

    float4 BaseColorFactor = float4{1, 1, 1, 1};
    float4 EmissiveFactor  = float4{1, 1, 1, 1};
    float4 DiffuseFactor   = float4{1, 1, 1, 1};
    float4 SpecularFactor  = float4{1, 1, 1, 1};

    struct TextureAttribs
    {
        // Texture index in Model.Textures array
        Int16 Index = -1;

        // Texture coordinates set (UV0 / UV1)
        Uint8 CoordSet = 0;

        void SetIndex(int Idx)
        {
            // clang-format off
            DEV_CHECK_ERR(Idx >= static_cast<int>(std::numeric_limits<decltype(Index)>::min()) &&
                          Idx <= static_cast<int>(std::numeric_limits<decltype(Index)>::max()),
                          "Texture index (", Idx, ") is out of representable range");
            // clang-format on
            Index = static_cast<decltype(Index)>(Idx);
        }

        void SetCoordSet(int Set)
        {
            // clang-format off
            DEV_CHECK_ERR(Set >= static_cast<int>(std::numeric_limits<decltype(CoordSet)>::min()) &&
                          Set <= static_cast<int>(std::numeric_limits<decltype(CoordSet)>::max()),
                          "Texture coordinate set (", Set, ") is out of representable range");
            // clang-format on
            CoordSet = static_cast<decltype(CoordSet)>(Set);
        }
    };
    enum TEXTURE_ID
    {
        TEXTURE_ID_BASE_COLOR = 0,
        TEXTURE_ID_METALL_ROUGHNESS,
        TEXTURE_ID_NORMAL_MAP,
        TEXTURE_ID_OCCLUSION,
        TEXTURE_ID_EMISSIVE,
        TEXTURE_ID_SPEC_GLOSS,
        TEXTURE_ID_DIFFUSE,
        TEXTURE_ID_NUM_TEXTURES
    };
    std::array<TextureAttribs, TEXTURE_ID_NUM_TEXTURES> Textures;

    enum class PbrWorkflow
    {
        MetallicRoughness,
        SpecularGlossiness
    };
    PbrWorkflow workflow = PbrWorkflow::MetallicRoughness;
};


struct Primitive
{
    Uint32    FirstIndex  = 0;
    Uint32    IndexCount  = 0;
    Uint32    VertexCount = 0;
    Material& material;
    bool      hasIndices;

    BoundBox BB;
    bool     IsValidBB = false;

    Primitive(Uint32    _FirstIndex,
              Uint32    _IndexCount,
              Uint32    _VertexCount,
              Material& _material) :
        FirstIndex{_FirstIndex},
        IndexCount{_IndexCount},
        VertexCount{_VertexCount},
        material{_material},
        hasIndices{_IndexCount > 0}
    {
    }

    void SetBoundingBox(const float3& min, const float3& max)
    {
        BB.Min    = min;
        BB.Max    = max;
        IsValidBB = true;
    }
};



struct Mesh
{
    std::vector<std::unique_ptr<Primitive>> Primitives;

    BoundBox BB;

    bool IsValidBB = false;

    struct TransformData
    {
        static constexpr Uint32 MaxNumJoints = 128u;

        float4x4 matrix;
        float4x4 jointMatrix[MaxNumJoints] = {};
        int      jointcount                = 0;
    };

    TransformData Transforms;

    Mesh(IRenderDevice* pDevice, const float4x4& matrix);
    void SetBoundingBox(const float3& min, const float3& max);
};


struct Node;
struct Skin
{
    std::string           Name;
    Node*                 pSkeletonRoot = nullptr;
    std::vector<float4x4> InverseBindMatrices;
    std::vector<Node*>    Joints;
};


struct Node
{
    std::string Name;
    Node*       Parent = nullptr;
    Uint32      Index;

    std::vector<std::unique_ptr<Node>> Children;

    float4x4              Matrix;
    std::unique_ptr<Mesh> _Mesh;
    Skin*                 _Skin     = nullptr;
    Int32                 SkinIndex = -1;
    float3                Translation;
    float3                Scale = float3(1.0f, 1.0f, 1.0f);
    Quaternion            Rotation;
    BoundBox              BVH;
    BoundBox              AABB;
    bool                  IsValidBVH = false;

    float4x4 LocalMatrix() const;
    float4x4 GetMatrix() const;
    void     Update();
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
    Node*     node         = nullptr;
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
    struct VertexAttribs0
    {
        float3 pos;
        float3 normal;
        float2 uv0;
        float2 uv1;
    };

    struct VertexAttribs1
    {
        float4 joint0;
        float4 weight0;
    };

    enum BUFFER_ID
    {
        BUFFER_ID_VERTEX0 = 0,
        BUFFER_ID_VERTEX1,
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

        struct TextureInfo
        {
            RefCntWeakPtr<ITexture> wpTexture;

            const float4 UVScaleBias;

            TextureInfo(ITexture* pTex, const float4& _UVScaleBias) :
                wpTexture{pTex},
                UVScaleBias{_UVScaleBias}
            {}
        };
        std::unordered_map<std::string, TextureInfo> Textures;
    };

    Model(IRenderDevice*     pDevice,
          IDeviceContext*    pContext,
          const std::string& filename,
          TextureCacheType*  pTextureCache = nullptr,
          GLTFCacheInfo*     pCache        = nullptr);

    ~Model();

    void UpdateAnimation(Uint32 index, float time);

    void PrepareGPUResources(IDeviceContext* pCtx);

    IBuffer* GetBuffer(BUFFER_ID BuffId, Uint32& Offset)
    {
        VERIFY_EXPR(BuffId < BUFFER_ID_NUM_BUFFERS);
        auto& Buff = Buffers[BuffId];
        if (Buff.pBuffer)
        {
            Offset = 0;
            return Buff.pBuffer;
        }
        else if (CacheInfo.pResourceMgr != nullptr)
        {
            Offset = static_cast<Uint32>(Buff.CacheAllocation.Region.UnalignedOffset);
            return CacheInfo.pResourceMgr->GetBuffer(Buff.CacheAllocation);
        }

        Offset = 0;
        return nullptr;
    }

    ITexture* GetTexture(Uint32 Index)
    {
        auto& TexInfo = Textures[Index];
        if (TexInfo.pTexture)
            return TexInfo.pTexture;
        else if (CacheInfo.pResourceMgr)
            return CacheInfo.pResourceMgr->GetTexture(TexInfo.CacheAllocation);
        else
            return nullptr;
    }

    const float4& GetUVScaleBias(Uint32 Index)
    {
        return Textures[Index].UVScaleBias;
    }

private:
    void LoadFromFile(IRenderDevice*     pDevice,
                      IDeviceContext*    pContext,
                      const std::string& filename,
                      TextureCacheType*  pTextureCache,
                      GLTFCacheInfo*     pCache);

    void LoadNode(IRenderDevice*         pDevice,
                  Node*                  parent,
                  const tinygltf::Node&  gltf_node,
                  uint32_t               nodeIndex,
                  const tinygltf::Model& gltf_model);

    void LoadSkins(const tinygltf::Model& gltf_model);

    void LoadTextures(IRenderDevice*         pDevice,
                      const tinygltf::Model& gltf_model,
                      const std::string&     BaseDir,
                      TextureCacheType*      pTextureCache);

    void  LoadTextureSamplers(IRenderDevice* pDevice, const tinygltf::Model& gltf_model);
    void  LoadMaterials(const tinygltf::Model& gltf_model);
    void  LoadAnimations(const tinygltf::Model& gltf_model);
    void  CalculateBoundingBox(Node* node, const Node* parent);
    void  GetSceneDimensions();
    Node* FindNode(Node* parent, Uint32 index);
    Node* NodeFromIndex(uint32_t index);

    GLTFCacheInfo CacheInfo;

    struct ResourceInitData;
    std::unique_ptr<ResourceInitData> InitData;

    struct BufferInfo
    {
        RefCntAutoPtr<IBuffer> pBuffer;

        GLTFResourceManager::BufferAllocation CacheAllocation;
    };
    std::array<BufferInfo, BUFFER_ID_NUM_BUFFERS> Buffers;

    struct TextureInfo
    {
        RefCntAutoPtr<ITexture>                pTexture;
        GLTFResourceManager::TextureAllocation CacheAllocation;
        float4                                 UVScaleBias{1, 1, 0, 0};
    };
    std::vector<TextureInfo> Textures;
};

} // namespace GLTF

} // namespace Diligent
