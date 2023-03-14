/*
 *  Copyright 2019-2023 Diligent Graphics LLC
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

#include <unordered_map>
#include <vector>
#include <memory>

#include "GLTFLoader.hpp"

namespace Diligent
{

namespace GLTF
{

struct Model;
struct ModelCreateInfo;
struct Node;

class ModelBuilder
{
public:
    ModelBuilder(const ModelCreateInfo& _CI, Model& _Model);
    ~ModelBuilder();

    template <typename GltfModelType>
    void LoadNode(const GltfModelType& GltfModel,
                  Node*                parent,
                  int                  NodeIndex);

    void InitBuffers(IRenderDevice* pDevice, IDeviceContext* pContext);

    template <typename GltfModelType>
    bool LoadAnimationAndSkin(const GltfModelType& GltfModel);

private:
    struct ConvertedBufferViewKey
    {
        std::vector<int> AccessorIds;
        mutable size_t   Hash = 0;

        bool operator==(const ConvertedBufferViewKey& Rhs) const noexcept;

        struct Hasher
        {
            size_t operator()(const ConvertedBufferViewKey& Key) const noexcept;
        };
    };

    struct ConvertedBufferViewData
    {
        std::vector<size_t> Offsets;
    };

    using ConvertedBufferViewMap = std::unordered_map<ConvertedBufferViewKey, ConvertedBufferViewData, ConvertedBufferViewKey::Hasher>;

    static void WriteGltfData(const void*                  pSrc,
                              VALUE_TYPE                   SrcType,
                              Uint32                       NumSrcComponents,
                              Uint32                       SrcElemStride,
                              std::vector<Uint8>::iterator dst_it,
                              VALUE_TYPE                   DstType,
                              Uint32                       NumDstComponents,
                              Uint32                       DstElementStride,
                              Uint32                       NumElements);

    template <typename GltfModelType>
    void ConvertVertexData(const GltfModelType&          GltfModel,
                           const ConvertedBufferViewKey& Key,
                           ConvertedBufferViewData&      Data,
                           Uint32                        VertexCount);

    template <typename SrcType, typename DstType>
    inline static void WriteIndexData(const void*                  pSrc,
                                      std::vector<Uint8>::iterator dst_it,
                                      Uint32                       NumElements,
                                      Uint32                       BaseVertex);

    template <typename GltfModelType>
    Uint32 ConvertIndexData(const GltfModelType& GltfModel,
                            int                  AccessorId,
                            Uint32               BaseVertex);

    template <typename GltfModelType>
    void LoadSkins(const GltfModelType& GltfModel);

    template <typename GltfModelType>
    void LoadAnimations(const GltfModelType& GltfModel);

    template <typename GltfModelType>
    auto GetGltfDataInfo(const GltfModelType& GltfModel, int AccessorId);

    Node* NodeFromIndex(int Index) const
    {
        auto it = m_NodeIndexRemapping.find(Index);
        return it != m_NodeIndexRemapping.end() ?
            m_Model.LinearNodes[it->second] :
            nullptr;
    }

private:
    const ModelCreateInfo& m_CI;
    Model&                 m_Model;

    // In a GLTF file, nodes are referenced by index.
    // A model that is loaded may not contain all original nodes though,
    // so we need to have a mapping from original index to the
    // index in Model.Nodes.
    std::unordered_map<int, int> m_NodeIndexRemapping;

    std::vector<Uint8>              m_IndexData;
    std::vector<std::vector<Uint8>> m_VertexData;

    ConvertedBufferViewMap m_ConvertedBuffers;
};


template <typename GltfModelType>
void ModelBuilder::LoadNode(const GltfModelType& GltfModel,
                            Node*                parent,
                            int                  NodeIndex)
{
    const auto& GltfNode = GltfModel.GetNode(NodeIndex);

    auto NewNode = std::make_unique<Node>(GltfNode.GetName(), parent);

    NewNode->SkinIndex = GltfNode.GetSkin();
    NewNode->Matrix    = float4x4::Identity();

    // Any node can define a local space transformation either by supplying a matrix property,
    // or any of translation, rotation, and scale properties (also known as TRS properties).

    // Generate local node matrix
    //float3 Translation;
    if (GltfNode.GetTranslation().size() == 3)
    {
        NewNode->Translation = float3::MakeVector(GltfNode.GetTranslation().data());
    }

    if (GltfNode.GetRotation().size() == 4)
    {
        NewNode->Rotation.q = float4::MakeVector(GltfNode.GetRotation().data());
        //NewNode->rotation = glm::mat4(q);
    }

    if (GltfNode.GetScale().size() == 3)
    {
        NewNode->Scale = float3::MakeVector(GltfNode.GetScale().data());
    }

    if (GltfNode.GetMatrix().size() == 16)
    {
        NewNode->Matrix = float4x4::MakeMatrix(GltfNode.GetMatrix().data());
    }

    // Load children first
    for (const auto ChildNodeIdx : GltfNode.GetChildrenIds())
    {
        LoadNode(GltfModel, NewNode.get(), ChildNodeIdx);
    }

    // Node contains mesh data
    if (GltfNode.GetMeshId() >= 0)
    {
        const auto& GltfMesh = GltfModel.GetMesh(GltfNode.GetMeshId());
        auto        pNewMesh = std::make_unique<Mesh>(NewNode->Matrix);
        for (size_t prim = 0; prim < GltfMesh.GetPrimitiveCount(); ++prim)
        {
            const auto& GltfPrimitive = GltfMesh.GetPrimitive(prim);

            const auto DstIndexSize = m_Model.Buffers.back().ElementStride;

            uint32_t IndexStart  = static_cast<uint32_t>(m_IndexData.size()) / DstIndexSize;
            uint32_t VertexStart = 0;
            uint32_t IndexCount  = 0;
            uint32_t VertexCount = 0;
            float3   PosMin;
            float3   PosMax;

            // Vertices
            {
                ConvertedBufferViewKey Key;

                Key.AccessorIds.resize(m_Model.VertexAttributes.size());
                for (Uint32 i = 0; i < m_Model.VertexAttributes.size(); ++i)
                {
                    const auto& Attrib = m_Model.VertexAttributes[i];
                    VERIFY_EXPR(Attrib.Name != nullptr);
                    auto* pAttribId    = GltfPrimitive.GetAttribute(Attrib.Name);
                    Key.AccessorIds[i] = pAttribId != nullptr ? *pAttribId : -1;
                }

                {
                    auto* pPosAttribId = GltfPrimitive.GetAttribute("POSITION");
                    VERIFY(pPosAttribId != nullptr, "Position attribute is required");

                    const auto& PosAccessor = GltfModel.GetAccessor(*pPosAttribId);

                    PosMin      = PosAccessor.GetMinValues();
                    PosMax      = PosAccessor.GetMaxValues();
                    VertexCount = static_cast<uint32_t>(PosAccessor.GetCount());
                }

                auto& Data = m_ConvertedBuffers[Key];
                if (Data.Offsets.empty())
                {
                    ConvertVertexData(GltfModel, Key, Data, VertexCount);
                }

                VertexStart = StaticCast<uint32_t>(Data.Offsets[0] / m_Model.Buffers[0].ElementStride);
            }


            // Indices
            if (GltfPrimitive.GetIndicesId() >= 0)
            {
                IndexCount = ConvertIndexData(GltfModel, GltfPrimitive.GetIndicesId(), VertexStart);
            }

            pNewMesh->Primitives.emplace_back(
                IndexStart,
                IndexCount,
                VertexCount,
                GltfPrimitive.GetMaterialId() >= 0 ? static_cast<Uint32>(GltfPrimitive.GetMaterialId()) : static_cast<Uint32>(m_Model.Materials.size() - 1),
                PosMin,
                PosMax //
            );
        }

        if (!pNewMesh->Primitives.empty())
        {
            // Mesh BB from BBs of primitives
            pNewMesh->BB = pNewMesh->Primitives[0].BB;
            for (size_t prim = 1; prim < pNewMesh->Primitives.size(); ++prim)
            {
                const auto& PrimBB = pNewMesh->Primitives[prim].BB;
                pNewMesh->BB.Min   = std::min(pNewMesh->BB.Min, PrimBB.Min);
                pNewMesh->BB.Max   = std::max(pNewMesh->BB.Max, PrimBB.Max);
            }
        }

        if (m_CI.MeshLoadCallback)
            m_CI.MeshLoadCallback(&GltfMesh.Get(), *pNewMesh);

        NewNode->pMesh = std::move(pNewMesh);
    }

    // Node contains camera
    if (GltfNode.GetCameraId() >= 0)
    {
        const auto& GltfCam = GltfModel.GetCamera(GltfNode.GetCameraId());

        auto pNewCamera  = std::make_unique<Camera>();
        pNewCamera->Name = GltfCam.GetName();

        if (GltfCam.GetType() == "perspective")
        {
            pNewCamera->Type = Camera::Projection::Perspective;

            const auto& PerspectiveCam{GltfCam.GetPerspective()};

            pNewCamera->Perspective.AspectRatio = static_cast<float>(PerspectiveCam.GetAspectRatio());
            pNewCamera->Perspective.YFov        = static_cast<float>(PerspectiveCam.GetYFov());
            pNewCamera->Perspective.ZNear       = static_cast<float>(PerspectiveCam.GetZNear());
            pNewCamera->Perspective.ZFar        = static_cast<float>(PerspectiveCam.GetZFar());
        }
        else if (GltfCam.GetType() == "orthographic")
        {
            pNewCamera->Type = Camera::Projection::Orthographic;

            const auto& OrthoCam{GltfCam.GetOrthographic()};
            pNewCamera->Orthographic.XMag  = static_cast<float>(OrthoCam.GetXMag());
            pNewCamera->Orthographic.YMag  = static_cast<float>(OrthoCam.GetYMag());
            pNewCamera->Orthographic.ZNear = static_cast<float>(OrthoCam.GetZNear());
            pNewCamera->Orthographic.ZFar  = static_cast<float>(OrthoCam.GetZFar());
        }
        else
        {
            UNEXPECTED("Unexpected camera type: ", GltfCam.GetType());
            pNewCamera.reset();
        }

        if (pNewCamera)
            NewNode->pCamera = std::move(pNewCamera);
    }

    m_NodeIndexRemapping[NodeIndex] = static_cast<int>(m_Model.LinearNodes.size());
    m_Model.LinearNodes.push_back(NewNode.get());
    if (parent)
    {
        parent->Children.push_back(std::move(NewNode));
    }
    else
    {
        m_Model.Nodes.push_back(std::move(NewNode));
    }
}

template <typename GltfModelType>
auto ModelBuilder::GetGltfDataInfo(const GltfModelType& GltfModel, int AccessorId)
{
    const auto  GltfAccessor  = GltfModel.GetAccessor(AccessorId);
    const auto  GltfView      = GltfModel.GetBufferView(GltfAccessor.GetBufferViewId());
    const auto  GltfBuffer    = GltfModel.GetBuffer(GltfView.GetBufferId());
    const auto* pSrcData      = GltfBuffer.GetData(GltfAccessor.GetByteOffset() + GltfView.GetByteOffset());
    const auto  SrcCount      = GltfAccessor.GetCount();
    const auto  SrcByteStride = GltfAccessor.GetByteStride(GltfView);

    struct GltfDataInfo
    {
        decltype(GltfAccessor) Accessor;

        const void* const             pData;
        const decltype(SrcCount)      Count;
        const decltype(SrcByteStride) ByteStride;
    };

    return GltfDataInfo{GltfAccessor, pSrcData, SrcCount, SrcByteStride};
}

template <typename GltfModelType>
void ModelBuilder::ConvertVertexData(const GltfModelType&          GltfModel,
                                     const ConvertedBufferViewKey& Key,
                                     ConvertedBufferViewData&      Data,
                                     Uint32                        VertexCount)
{
    VERIFY_EXPR(Data.Offsets.empty());
    Data.Offsets.resize(m_VertexData.size());
    for (size_t i = 0; i < Data.Offsets.size(); ++i)
    {
        Data.Offsets[i] = m_VertexData[i].size();
        VERIFY((Data.Offsets[i] % m_Model.Buffers[i].ElementStride) == 0, "Current offset is not a multiple of element stride");
        m_VertexData[i].resize(m_VertexData[i].size() + size_t{VertexCount} * m_Model.Buffers[i].ElementStride);
    }

    VERIFY_EXPR(Key.AccessorIds.size() == m_Model.VertexAttributes.size());
    for (size_t i = 0; i < m_Model.VertexAttributes.size(); ++i)
    {
        const auto AccessorId = Key.AccessorIds[i];
        if (AccessorId < 0)
            continue;

        const auto& Attrib       = m_Model.VertexAttributes[i];
        const auto  VertexStride = m_Model.Buffers[Attrib.BufferId].ElementStride;

        const auto GltfVerts     = GetGltfDataInfo(GltfModel, AccessorId);
        const auto ValueType     = GltfVerts.Accessor.GetComponentType();
        const auto NumComponents = GltfVerts.Accessor.GetNumComponents();
        const auto SrcStride     = GltfVerts.ByteStride;
        VERIFY_EXPR(SrcStride > 0);

        auto dst_it = m_VertexData[Attrib.BufferId].begin() + Data.Offsets[Attrib.BufferId] + Attrib.RelativeOffset;

        VERIFY_EXPR(static_cast<Uint32>(GltfVerts.Count) == VertexCount);
        WriteGltfData(GltfVerts.pData, ValueType, NumComponents, SrcStride, dst_it, Attrib.ValueType, Attrib.NumComponents, VertexStride, VertexCount);
    }
}

template <typename SrcType, typename DstType>
inline void ModelBuilder::WriteIndexData(const void*                  pSrc,
                                         std::vector<Uint8>::iterator dst_it,
                                         Uint32                       NumElements,
                                         Uint32                       BaseVertex)
{
    for (size_t i = 0; i < NumElements; ++i)
    {
        reinterpret_cast<DstType&>(*dst_it) = static_cast<DstType>(static_cast<const SrcType*>(pSrc)[i] + BaseVertex);
        dst_it += sizeof(DstType);
    }
}

template <typename GltfModelType>
Uint32 ModelBuilder::ConvertIndexData(const GltfModelType& GltfModel,
                                      int                  AccessorId,
                                      Uint32               BaseVertex)
{
    VERIFY_EXPR(AccessorId >= 0);

    const auto GltfIndices = GetGltfDataInfo(GltfModel, AccessorId);
    const auto IndexSize   = m_Model.Buffers.back().ElementStride;
    const auto IndexCount  = static_cast<uint32_t>(GltfIndices.Count);

    auto IndexDataStart = m_IndexData.size();
    VERIFY((IndexDataStart % IndexSize) == 0, "Current offset is not a multiple of index size");
    m_IndexData.resize(IndexDataStart + size_t{IndexCount} * size_t{IndexSize});
    auto index_it = m_IndexData.begin() + IndexDataStart;

    VERIFY_EXPR(IndexSize == 4 || IndexSize == 2);
    switch (GltfIndices.Accessor.GetComponentType())
    {
        case VT_UINT32:
            if (IndexSize == 4)
                WriteIndexData<Uint32, Uint32>(GltfIndices.pData, index_it, IndexCount, BaseVertex);
            else
                WriteIndexData<Uint32, Uint16>(GltfIndices.pData, index_it, IndexCount, BaseVertex);
            break;

        case VT_UINT16:
            if (IndexSize == 4)
                WriteIndexData<Uint16, Uint32>(GltfIndices.pData, index_it, IndexCount, BaseVertex);
            else
                WriteIndexData<Uint16, Uint16>(GltfIndices.pData, index_it, IndexCount, BaseVertex);
            break;

        case VT_UINT8:
            if (IndexSize == 4)
                WriteIndexData<Uint8, Uint32>(GltfIndices.pData, index_it, IndexCount, BaseVertex);
            else
                WriteIndexData<Uint8, Uint16>(GltfIndices.pData, index_it, IndexCount, BaseVertex);
            break;

        default:
            std::cerr << "Index component type " << GltfIndices.Accessor.GetComponentType() << " is not supported!" << std::endl;
            return 0;
    }

    return IndexCount;
}

template <typename GltfModelType>
void ModelBuilder::LoadSkins(const GltfModelType& GltfModel)
{
    for (size_t i = 0; i < GltfModel.GetSkinCount(); ++i)
    {
        const auto& GltfSkin = GltfModel.GetSkin(i);

        auto NewSkin  = std::make_unique<Skin>();
        NewSkin->Name = GltfSkin.GetName();

        // Find skeleton root node
        if (GltfSkin.GetSkeletonId() >= 0)
        {
            NewSkin->pSkeletonRoot = NodeFromIndex(GltfSkin.GetSkeletonId());
        }

        // Find joint nodes
        for (int JointIndex : GltfSkin.GetJointIds())
        {
            if (auto* node = NodeFromIndex(JointIndex))
            {
                NewSkin->Joints.push_back(node);
            }
        }

        // Get inverse bind matrices from buffer
        if (GltfSkin.GetInverseBindMatricesId() >= 0)
        {
            const auto GltfSkins = GetGltfDataInfo(GltfModel, GltfSkin.GetInverseBindMatricesId());
            NewSkin->InverseBindMatrices.resize(GltfSkins.Count);
            memcpy(NewSkin->InverseBindMatrices.data(), GltfSkins.pData, GltfSkins.Count * sizeof(float4x4));
        }

        m_Model.Skins.push_back(std::move(NewSkin));
    }
}

template <typename GltfModelType>
void ModelBuilder::LoadAnimations(const GltfModelType& GltfModel)
{
    for (size_t anim = 0; anim < GltfModel.GetAnimationCount(); ++anim)
    {
        const auto& GltfAnim = GltfModel.GetAnimation(anim);

        Animation Anim;
        Anim.Name = GltfAnim.GetName();
        if (Anim.Name.empty())
        {
            Anim.Name = std::to_string(m_Model.Animations.size());
        }

        // Samplers
        for (size_t sam = 0; sam < GltfAnim.GetSamplerCount(); ++sam)
        {
            const auto& GltfSam = GltfAnim.GetSampler(sam);

            AnimationSampler AnimSampler{GltfSam.GetInterpolation()};

            // Read sampler input time values
            {
                const auto GltfInputs = GetGltfDataInfo(GltfModel, GltfSam.GetInputId());
                VERIFY_EXPR(GltfInputs.Accessor.GetComponentType() == VT_FLOAT32);

                AnimSampler.Inputs.resize(GltfInputs.Count);
                memcpy(AnimSampler.Inputs.data(), GltfInputs.pData, sizeof(float) * GltfInputs.Count);

                for (auto input : AnimSampler.Inputs)
                {
                    if (input < Anim.Start)
                    {
                        Anim.Start = input;
                    }
                    if (input > Anim.End)
                    {
                        Anim.End = input;
                    }
                }
            }


            // Read sampler output T/R/S values
            {
                const auto GltfOutputs = GetGltfDataInfo(GltfModel, GltfSam.GetOutputId());
                VERIFY_EXPR(GltfOutputs.Accessor.GetComponentType() == VT_FLOAT32);

                AnimSampler.OutputsVec4.reserve(GltfOutputs.Count);
                const auto NumComponents = GltfOutputs.Accessor.GetNumComponents();
                switch (NumComponents)
                {
                    case 3:
                    {
                        const auto* pSrcVec3 = static_cast<const float3*>(GltfOutputs.pData);
                        for (size_t i = 0; i < GltfOutputs.Count; ++i)
                        {
                            AnimSampler.OutputsVec4.push_back(float4{pSrcVec3[i], 0.0f});
                        }
                        break;
                    }

                    case 4:
                    {
                        const auto* pSrcVec4 = static_cast<const float4*>(GltfOutputs.pData);
                        for (size_t i = 0; i < GltfOutputs.Count; ++i)
                        {
                            AnimSampler.OutputsVec4.push_back(pSrcVec4[i]);
                        }
                        break;
                    }

                    default:
                    {
                        LOG_WARNING_MESSAGE("Unsupported component count: ", NumComponents);
                        break;
                    }
                }
            }

            Anim.Samplers.push_back(AnimSampler);
        }

        for (size_t chnl = 0; chnl < GltfAnim.GetChannelCount(); ++chnl)
        {
            const auto& GltfChannel = GltfAnim.GetChannel(chnl);

            const auto PathType = GltfChannel.GetPathType();
            if (PathType == AnimationChannel::PATH_TYPE::WEIGHTS)
            {
                LOG_WARNING_MESSAGE("Weights are not yet supported, skipping channel");
                continue;
            }

            const auto SamplerIndex = GltfChannel.GetSamplerId();
            if (SamplerIndex < 0)
                continue;

            const auto NodeId = GltfChannel.GetTargetNodeId();
            if (NodeId < 0)
                continue;

            auto* pNode = NodeFromIndex(NodeId);
            if (pNode == nullptr)
                continue;

            Anim.Channels.emplace_back(PathType, pNode, SamplerIndex);
        }

        m_Model.Animations.push_back(std::move(Anim));
    }
}

template <typename GltfModelType>
bool ModelBuilder::LoadAnimationAndSkin(const GltfModelType& GltfModel)
{
    bool UsesAnimation = false;
    for (const auto& Attrib : m_Model.VertexAttributes)
    {
        if (strncmp(Attrib.Name, "WEIGHTS", 7) == 0 ||
            strncmp(Attrib.Name, "JOINTS", 6) == 0)
        {
            UsesAnimation = true;
            break;
        }
    }

    if (!UsesAnimation)
        return false;

    LoadAnimations(GltfModel);
    LoadSkins(GltfModel);

    return true;
}

} // namespace GLTF

} // namespace Diligent
