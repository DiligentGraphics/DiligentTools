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
#include <unordered_set>
#include <vector>
#include <algorithm>
#include <string>

#include "GLTFLoader.hpp"
#include "GraphicsAccessories.hpp"

namespace Diligent
{

namespace GLTF
{

struct Model;
struct ModelCreateInfo;
struct Node;

// {0BF00221-593F-40CE-B5BD-E47039D77F4A}
static constexpr INTERFACE_ID IID_BufferInitData =
    {0xbf00221, 0x593f, 0x40ce, {0xb5, 0xbd, 0xe4, 0x70, 0x39, 0xd7, 0x7f, 0x4a}};

struct BufferInitData : ObjectBase<IObject>
{
    BufferInitData(IReferenceCounters* pRefCounters) :
        ObjectBase{pRefCounters}
    {
    }

    static RefCntAutoPtr<BufferInitData> Create()
    {
        return RefCntAutoPtr<BufferInitData>{MakeNewRCObj<BufferInitData>()()};
    }

    IMPLEMENT_QUERY_INTERFACE_IN_PLACE(IID_BufferInitData, ObjectBase)

    std::vector<std::vector<Uint8>> Data;
};

class ModelBuilder
{
public:
    ModelBuilder(const ModelCreateInfo& _CI, Model& _Model);
    ~ModelBuilder();

    template <typename GltfModelType>
    void Execute(const GltfModelType& GltfModel,
                 int                  SceneIndex,
                 IRenderDevice*       pDevice,
                 IDeviceContext*      pContext);

    static std::pair<FILTER_TYPE, FILTER_TYPE> GetFilterType(int32_t GltfFilterMode);

    static TEXTURE_ADDRESS_MODE GetAddressMode(int32_t GltfWrapMode);

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

    // If SceneIndex >= 0, loads only the specified scene, otherwise loads all scenes.
    // Stores the GLTF node indices as the node pointers.
    template <typename GltfModelType>
    void LoadScenes(const GltfModelType& GltfModel, int SceneIndex);

    // Recursively allocates nodes as well as meshes and cameras.
    template <typename GltfModelType>
    void AllocateNode(const GltfModelType& GltfModel,
                      int                  GltfNodeIndex);

    // Recursively loads nodes.
    template <typename GltfModelType>
    Node* LoadNode(const GltfModelType& GltfModel,
                   Node*                Parent,
                   Scene&               scene,
                   int                  GltfNodeIndex);

    template <typename GltfModelType>
    Mesh* LoadMesh(const GltfModelType& GltfModel,
                   int                  GltfMeshIndex);

    template <typename GltfModelType>
    Camera* LoadCamera(const GltfModelType& GltfModel,
                       int                  GltfCameraIndex);

    void InitIndexBuffer(IRenderDevice* pDevice, IDeviceContext* pContext);
    void InitVertexBuffers(IRenderDevice* pDevice, IDeviceContext* pContext);

    template <typename GltfModelType>
    bool LoadAnimationAndSkin(const GltfModelType& GltfModel);

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
                                      size_t                       SrcStride,
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

    // Returns the node pointer from the node index in the source GLTF model.
    Node* NodeFromGltfIndex(int GltfIndex) const
    {
        auto it = m_NodeIndexRemapping.find(GltfIndex);
        return it != m_NodeIndexRemapping.end() ?
            &m_Model.Nodes[it->second] :
            nullptr;
    }

private:
    const ModelCreateInfo& m_CI;
    Model&                 m_Model;

    // In a GLTF file, all objects are referenced by global index.
    // A model that is loaded may not contain all original objects though,
    // so we need to keep a mapping from the original index to the loaded
    // index.
    std::unordered_map<int, int> m_NodeIndexRemapping;
    std::unordered_map<int, int> m_MeshIndexRemapping;
    std::unordered_map<int, int> m_CameraIndexRemapping;

    std::unordered_set<int> m_LoadedNodes;
    std::unordered_set<int> m_LoadedMeshes;
    std::unordered_set<int> m_LoadedCameras;

    std::unordered_map<int, int> m_NodeIdToSkinId;

    std::vector<Uint8>              m_IndexData;
    std::vector<std::vector<Uint8>> m_VertexData;

    ConvertedBufferViewMap m_ConvertedBuffers;
};

template <typename GltfModelType>
void ModelBuilder::LoadScenes(const GltfModelType& GltfModel, int SceneIndex)
{
    auto AddScene = [&](int GltfSceneId) {
        const auto& GltfScene = GltfModel.GetScene(GltfSceneId);

        m_Model.Scenes.emplace_back();
        auto& scene     = m_Model.Scenes.back();
        scene.Name      = GltfScene.GetName();
        auto& RootNodes = scene.RootNodes;
        RootNodes.resize(GltfScene.GetNodeCount());

        // Temporarily store node ids as pointers
        for (size_t i = 0; i < RootNodes.size(); ++i)
            RootNodes[i] = reinterpret_cast<Node*>(static_cast<size_t>(GltfScene.GetNodeId(i)));
    };

    if (const auto SceneCount = static_cast<int>(GltfModel.GetSceneCount()))
    {
        auto SceneId = SceneIndex;
        if (SceneId >= SceneCount)
        {
            DEV_ERROR("Scene id ", SceneIndex, " is invalid: GLTF model only contains ", SceneCount, " scenes.");
            SceneId = -1;
        }

        if (SceneId >= 0)
        {
            // Load only the selected scene
            m_Model.Scenes.reserve(1);
            AddScene(SceneId);
            m_Model.DefaultSceneId = 0;
        }
        else
        {
            // Load all scenes
            m_Model.Scenes.reserve(SceneCount);
            for (int i = 0; i < SceneCount; ++i)
                AddScene(i);

            m_Model.DefaultSceneId = GltfModel.GetDefaultSceneId();
            if (m_Model.DefaultSceneId < 0)
                m_Model.DefaultSceneId = 0;

            if (m_Model.DefaultSceneId >= SceneCount)
            {
                LOG_ERROR_MESSAGE("Default scene id ", m_Model.DefaultSceneId, " is invalid: GLTF model only contains ", SceneCount, " scenes. Using scene 0 as default.");
                m_Model.DefaultSceneId = 0;
            }
        }
    }
    else
    {
        m_Model.Scenes.resize(1);
        auto& RootNodes = m_Model.Scenes[0].RootNodes;
        RootNodes.resize(GltfModel.GetNodeCount());

        // Load all nodes if there are no scenes
        for (size_t node_idx = 0; node_idx < RootNodes.size(); ++node_idx)
            RootNodes[node_idx] = reinterpret_cast<Node*>(node_idx);
    }

    m_Model.Scenes.shrink_to_fit();
}


template <typename GltfModelType>
void ModelBuilder::AllocateNode(const GltfModelType& GltfModel,
                                int                  GltfNodeIndex)
{
    {
        const auto NodeId = static_cast<int>(m_Model.Nodes.size());
        if (!m_NodeIndexRemapping.emplace(GltfNodeIndex, NodeId).second)
        {
            // The node has already been allocated.
            // Note: we iterate through the list of nodes and recursively allocate
            //       all child nodes. As a result, we may encounter a node that
            //       has already been allocated as a child of another.
            //       Besides, same node may be present in multiple scenes.
            return;
        }

        m_Model.Nodes.emplace_back(NodeId);
    }

    const auto& GltfNode = GltfModel.GetNode(GltfNodeIndex);
    for (const auto ChildNodeIdx : GltfNode.GetChildrenIds())
    {
        AllocateNode(GltfModel, ChildNodeIdx);
    }

    const auto GltfMeshIndex = GltfNode.GetMeshId();
    if (GltfMeshIndex >= 0)
    {
        const auto MeshId = static_cast<int>(m_Model.Meshes.size());
        if (m_MeshIndexRemapping.emplace(GltfMeshIndex, MeshId).second)
            m_Model.Meshes.emplace_back();
    }

    const auto GltfCameraIndex = GltfNode.GetCameraId();
    if (GltfCameraIndex >= 0)
    {
        const auto CameraId = static_cast<int>(m_Model.Cameras.size());
        if (m_CameraIndexRemapping.emplace(GltfCameraIndex, CameraId).second)
            m_Model.Cameras.emplace_back();
    }
}

template <typename GltfModelType>
Mesh* ModelBuilder::LoadMesh(const GltfModelType& GltfModel,
                             int                  GltfMeshIndex)
{
    if (GltfMeshIndex < 0)
        return nullptr;

    auto mesh_it = m_MeshIndexRemapping.find(GltfMeshIndex);
    VERIFY(mesh_it != m_MeshIndexRemapping.end(), "Mesh with GLTF index ", GltfMeshIndex, " is not present in the map. This appears to be a bug.");
    const auto LoadedMeshId = mesh_it->second;

    auto& NewMesh = m_Model.Meshes[LoadedMeshId];

    if (m_LoadedMeshes.find(LoadedMeshId) != m_LoadedMeshes.end())
    {
        // The mesh has already been loaded as it is referenced by
        // multiple nodes (e.g. '2CylinderEngine' test model).
        return &NewMesh;
    }
    m_LoadedMeshes.emplace(LoadedMeshId);

    const auto& GltfMesh = GltfModel.GetMesh(GltfMeshIndex);

    NewMesh.Name = GltfMesh.GetName();

    const size_t PrimitiveCount = GltfMesh.GetPrimitiveCount();
    NewMesh.Primitives.reserve(PrimitiveCount);
    for (size_t prim = 0; prim < PrimitiveCount; ++prim)
    {
        const auto& GltfPrimitive = GltfMesh.GetPrimitive(prim);

        const auto DstIndexSize = m_Model.IndexData.IndexSize;

        uint32_t IndexStart  = static_cast<uint32_t>(m_IndexData.size()) / DstIndexSize;
        uint32_t VertexStart = 0;
        uint32_t IndexCount  = 0;
        uint32_t VertexCount = 0;
        float3   PosMin;
        float3   PosMax;

        // Vertices
        {
            ConvertedBufferViewKey Key;

            Key.AccessorIds.resize(m_Model.GetNumVertexAttributes());
            for (Uint32 i = 0; i < m_Model.GetNumVertexAttributes(); ++i)
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

            VertexStart = StaticCast<uint32_t>(Data.Offsets[0] / m_Model.VertexData.Strides[0]);
#ifdef DILIGENT_DEBUG
            for (size_t i = 1; i < Data.Offsets.size(); ++i)
            {
                VERIFY(Data.Offsets[i] / m_Model.VertexData.Strides[i] == VertexStart, "Vertex data is misaligned");
            }
#endif
        }

        // Indices
        if (GltfPrimitive.GetIndicesId() >= 0)
        {
            IndexCount = ConvertIndexData(GltfModel, GltfPrimitive.GetIndicesId(), VertexStart);
        }

        NewMesh.Primitives.emplace_back(
            IndexStart,
            IndexCount,
            VertexCount,
            GltfPrimitive.GetMaterialId() >= 0 ? static_cast<Uint32>(GltfPrimitive.GetMaterialId()) : static_cast<Uint32>(m_Model.Materials.size() - 1),
            PosMin,
            PosMax //
        );

        if (m_CI.PrimitiveLoadCallback)
            m_CI.PrimitiveLoadCallback(&GltfPrimitive.Get(), NewMesh.Primitives.back());
    }

    if (!NewMesh.Primitives.empty())
    {
        // Mesh BB from BBs of primitives
        NewMesh.BB = NewMesh.Primitives[0].BB;
        for (size_t prim = 1; prim < NewMesh.Primitives.size(); ++prim)
        {
            const auto& PrimBB = NewMesh.Primitives[prim].BB;
            NewMesh.BB.Min     = std::min(NewMesh.BB.Min, PrimBB.Min);
            NewMesh.BB.Max     = std::max(NewMesh.BB.Max, PrimBB.Max);
        }
    }

    if (m_CI.MeshLoadCallback)
        m_CI.MeshLoadCallback(&GltfMesh.Get(), NewMesh);

    return &NewMesh;
}

template <typename GltfModelType>
Camera* ModelBuilder::LoadCamera(const GltfModelType& GltfModel,
                                 int                  GltfCameraIndex)
{
    if (GltfCameraIndex < 0)
        return nullptr;

    auto camera_it = m_CameraIndexRemapping.find(GltfCameraIndex);
    VERIFY(camera_it != m_CameraIndexRemapping.end(), "Camera with GLTF index ", GltfCameraIndex, " is not present in the map. This appears to be a bug.");
    const auto LoadedCameraId = camera_it->second;

    auto& NewCamera = m_Model.Cameras[LoadedCameraId];

    if (m_LoadedCameras.find(LoadedCameraId) != m_LoadedCameras.end())
    {
        // The camera has already been loaded
        return &NewCamera;
    }
    m_LoadedCameras.emplace(LoadedCameraId);

    const auto& GltfCam = GltfModel.GetCamera(GltfCameraIndex);

    NewCamera.Name = GltfCam.GetName();

    if (GltfCam.GetType() == "perspective")
    {
        NewCamera.Type = Camera::Projection::Perspective;

        const auto& PerspectiveCam{GltfCam.GetPerspective()};

        NewCamera.Perspective.AspectRatio = static_cast<float>(PerspectiveCam.GetAspectRatio());
        NewCamera.Perspective.YFov        = static_cast<float>(PerspectiveCam.GetYFov());
        NewCamera.Perspective.ZNear       = static_cast<float>(PerspectiveCam.GetZNear());
        NewCamera.Perspective.ZFar        = static_cast<float>(PerspectiveCam.GetZFar());
    }
    else if (GltfCam.GetType() == "orthographic")
    {
        NewCamera.Type = Camera::Projection::Orthographic;

        const auto& OrthoCam{GltfCam.GetOrthographic()};
        NewCamera.Orthographic.XMag  = static_cast<float>(OrthoCam.GetXMag());
        NewCamera.Orthographic.YMag  = static_cast<float>(OrthoCam.GetYMag());
        NewCamera.Orthographic.ZNear = static_cast<float>(OrthoCam.GetZNear());
        NewCamera.Orthographic.ZFar  = static_cast<float>(OrthoCam.GetZFar());
    }
    else
    {
        UNEXPECTED("Unexpected camera type: ", GltfCam.GetType());
    }

    return &NewCamera;
}

template <typename GltfModelType>
Node* ModelBuilder::LoadNode(const GltfModelType& GltfModel,
                             Node*                Parent,
                             Scene&               scene,
                             int                  GltfNodeIndex)
{
    auto node_it = m_NodeIndexRemapping.find(GltfNodeIndex);
    VERIFY(node_it != m_NodeIndexRemapping.end(), "Node with GLTF index ", GltfNodeIndex, " is not present in the map. This appears to be a bug.");
    const auto LoadedNodeId = node_it->second;

    auto& NewNode = m_Model.Nodes[LoadedNodeId];
    VERIFY_EXPR(NewNode.Index == LoadedNodeId);
    // Add the node to the scene's linear nodes array
    scene.LinearNodes.emplace_back(&NewNode);

    if (m_LoadedNodes.find(LoadedNodeId) != m_LoadedNodes.end())
        return &NewNode;
    m_LoadedNodes.emplace(LoadedNodeId);

    const auto& GltfNode = GltfModel.GetNode(GltfNodeIndex);

    NewNode.Name   = GltfNode.GetName();
    NewNode.Parent = Parent;

    m_NodeIdToSkinId[LoadedNodeId] = GltfNode.GetSkinId();

    // Any node can define a local space transformation either by supplying a matrix property,
    // or any of translation, rotation, and scale properties (also known as TRS properties).

    if (GltfNode.GetTranslation().size() == 3)
    {
        NewNode.Translation = float3::MakeVector(GltfNode.GetTranslation().data());
    }

    if (GltfNode.GetRotation().size() == 4)
    {
        NewNode.Rotation.q = float4::MakeVector(GltfNode.GetRotation().data());
    }

    if (GltfNode.GetScale().size() == 3)
    {
        NewNode.Scale = float3::MakeVector(GltfNode.GetScale().data());
    }

    if (GltfNode.GetMatrix().size() == 16)
    {
        NewNode.Matrix = float4x4::MakeMatrix(GltfNode.GetMatrix().data());
    }

    // Load children first
    NewNode.Children.reserve(GltfNode.GetChildrenIds().size());
    for (const auto ChildNodeIdx : GltfNode.GetChildrenIds())
    {
        NewNode.Children.push_back(LoadNode(GltfModel, &NewNode, scene, ChildNodeIdx));
    }

    // Node contains mesh data
    NewNode.pMesh   = LoadMesh(GltfModel, GltfNode.GetMeshId());
    NewNode.pCamera = LoadCamera(GltfModel, GltfNode.GetCameraId());

    return &NewNode;
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
        VERIFY((Data.Offsets[i] % m_Model.VertexData.Strides[i]) == 0, "Current offset is not a multiple of the element stride");
        m_VertexData[i].resize(m_VertexData[i].size() + size_t{VertexCount} * m_Model.VertexData.Strides[i]);
    }

    VERIFY_EXPR(Key.AccessorIds.size() == m_Model.GetNumVertexAttributes());
    for (size_t i = 0; i < m_Model.GetNumVertexAttributes(); ++i)
    {
        const auto AccessorId = Key.AccessorIds[i];
        if (AccessorId < 0)
            continue;

        const auto& Attrib       = m_Model.VertexAttributes[i];
        const auto  VertexStride = m_Model.VertexData.Strides[Attrib.BufferId];

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
                                         size_t                       SrcStride,
                                         std::vector<Uint8>::iterator dst_it,
                                         Uint32                       NumElements,
                                         Uint32                       BaseVertex)
{
    for (size_t i = 0; i < NumElements; ++i)
    {
        const auto& SrcInd = *reinterpret_cast<const SrcType*>(static_cast<const Uint8*>(pSrc) + i * SrcStride);
        auto&       DstInd = reinterpret_cast<DstType&>(*dst_it);

        DstInd = static_cast<DstType>(SrcInd + BaseVertex);
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
    const auto IndexSize   = m_Model.IndexData.IndexSize;
    const auto IndexCount  = static_cast<uint32_t>(GltfIndices.Count);

    auto IndexDataStart = m_IndexData.size();
    VERIFY((IndexDataStart % IndexSize) == 0, "Current offset is not a multiple of index size");
    m_IndexData.resize(IndexDataStart + size_t{IndexCount} * size_t{IndexSize});
    auto index_it = m_IndexData.begin() + IndexDataStart;

    const auto ComponentType = GltfIndices.Accessor.GetComponentType();
    const auto SrcStride     = static_cast<size_t>(GltfIndices.ByteStride);
    VERIFY(SrcStride >= GetValueSize(ComponentType), "Byte stride (", SrcStride, ") is too small.");
    VERIFY_EXPR(IndexSize == 4 || IndexSize == 2);
    switch (ComponentType)
    {
        case VT_UINT32:
            if (IndexSize == 4)
                WriteIndexData<Uint32, Uint32>(GltfIndices.pData, SrcStride, index_it, IndexCount, BaseVertex);
            else
                WriteIndexData<Uint32, Uint16>(GltfIndices.pData, SrcStride, index_it, IndexCount, BaseVertex);
            break;

        case VT_UINT16:
            if (IndexSize == 4)
                WriteIndexData<Uint16, Uint32>(GltfIndices.pData, SrcStride, index_it, IndexCount, BaseVertex);
            else
                WriteIndexData<Uint16, Uint16>(GltfIndices.pData, SrcStride, index_it, IndexCount, BaseVertex);
            break;

        case VT_UINT8:
            if (IndexSize == 4)
                WriteIndexData<Uint8, Uint32>(GltfIndices.pData, SrcStride, index_it, IndexCount, BaseVertex);
            else
                WriteIndexData<Uint8, Uint16>(GltfIndices.pData, SrcStride, index_it, IndexCount, BaseVertex);
            break;

        default:
            UNEXPECTED("Index component type ", GetValueTypeString(ComponentType), " is not supported!");
            return 0;
    }

    return IndexCount;
}

template <typename GltfModelType>
void ModelBuilder::LoadSkins(const GltfModelType& GltfModel)
{
    m_Model.Skins.resize(GltfModel.GetSkinCount());
    for (size_t i = 0; i < GltfModel.GetSkinCount(); ++i)
    {
        const auto& GltfSkin = GltfModel.GetSkin(i);
        auto&       NewSkin  = m_Model.Skins[i];

        NewSkin.Name = GltfSkin.GetName();

        // Find skeleton root node
        if (GltfSkin.GetSkeletonId() >= 0)
        {
            NewSkin.pSkeletonRoot = NodeFromGltfIndex(GltfSkin.GetSkeletonId());
        }

        // Find joint nodes
        for (int JointIndex : GltfSkin.GetJointIds())
        {
            if (auto* node = NodeFromGltfIndex(JointIndex))
            {
                NewSkin.Joints.push_back(node);
            }
        }

        // Get inverse bind matrices from buffer
        if (GltfSkin.GetInverseBindMatricesId() >= 0)
        {
            const auto GltfSkins = GetGltfDataInfo(GltfModel, GltfSkin.GetInverseBindMatricesId());
            NewSkin.InverseBindMatrices.resize(GltfSkins.Count);
            VERIFY(GltfSkins.ByteStride == sizeof(float4x4), "Tightly packed skin data is expected.");
            memcpy(NewSkin.InverseBindMatrices.data(), GltfSkins.pData, GltfSkins.Count * sizeof(float4x4));
        }
    }
}

template <typename GltfModelType>
void ModelBuilder::LoadAnimations(const GltfModelType& GltfModel)
{
    const auto AnimationCount = GltfModel.GetAnimationCount();
    m_Model.Animations.resize(AnimationCount);
    for (size_t anim = 0; anim < AnimationCount; ++anim)
    {
        const auto& GltfAnim = GltfModel.GetAnimation(anim);
        auto&       Anim     = m_Model.Animations[anim];

        Anim.Name = GltfAnim.GetName();
        if (Anim.Name.empty())
        {
            Anim.Name = std::to_string(anim);
        }

        // Samplers
        const auto SamplerCount = GltfAnim.GetSamplerCount();
        Anim.Samplers.reserve(SamplerCount);
        for (size_t sam = 0; sam < SamplerCount; ++sam)
        {
            const auto& GltfSam = GltfAnim.GetSampler(sam);

            Anim.Samplers.emplace_back(GltfSam.GetInterpolation());
            auto& AnimSampler = Anim.Samplers.back();

            // Read sampler input time values
            {
                const auto GltfInputs = GetGltfDataInfo(GltfModel, GltfSam.GetInputId());
                VERIFY(GltfInputs.Accessor.GetComponentType() == VT_FLOAT32, "Float32 data is expected.");
                VERIFY(GltfInputs.ByteStride == sizeof(float), "Tightly packed data is expected.");

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
                VERIFY(GltfOutputs.Accessor.GetComponentType() == VT_FLOAT32, "Float32 data is expected.");
                VERIFY(GltfOutputs.ByteStride >= static_cast<int>(GltfOutputs.Accessor.GetNumComponents() * sizeof(float)), "Byte stide is too small.");

                AnimSampler.OutputsVec4.reserve(GltfOutputs.Count);
                const auto NumComponents = GltfOutputs.Accessor.GetNumComponents();
                switch (NumComponents)
                {
                    case 3:
                    {
                        for (size_t i = 0; i < GltfOutputs.Count; ++i)
                        {
                            const auto& SrcVec3 = *reinterpret_cast<const float3*>(static_cast<const Uint8*>(GltfOutputs.pData) + GltfOutputs.ByteStride * i);
                            AnimSampler.OutputsVec4.push_back(float4{SrcVec3, 0.0f});
                        }
                        break;
                    }

                    case 4:
                    {
                        for (size_t i = 0; i < GltfOutputs.Count; ++i)
                        {
                            const auto& SrcVec4 = *reinterpret_cast<const float4*>(static_cast<const Uint8*>(GltfOutputs.pData) + GltfOutputs.ByteStride * i);
                            AnimSampler.OutputsVec4.push_back(SrcVec4);
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
        }

        const auto ChannelCount = GltfAnim.GetChannelCount();
        Anim.Channels.reserve(ChannelCount);
        for (size_t chnl = 0; chnl < ChannelCount; ++chnl)
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

            auto* pNode = NodeFromGltfIndex(NodeId);
            if (pNode == nullptr)
                continue;

            Anim.Channels.emplace_back(PathType, pNode, SamplerIndex);
        }
    }
}

template <typename GltfModelType>
bool ModelBuilder::LoadAnimationAndSkin(const GltfModelType& GltfModel)
{
    bool UsesAnimation = false;
    for (size_t i = 0; i < m_Model.GetNumVertexAttributes(); ++i)
    {
        const auto& Attrib = m_Model.GetVertexAttribute(i);

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

    // Assign skins
    for (int i = 0; i < static_cast<int>(m_Model.Nodes.size()); ++i)
    {
        VERIFY_EXPR(m_Model.Nodes[i].Index == i);
        auto skin_it = m_NodeIdToSkinId.find(i);
        if (skin_it != m_NodeIdToSkinId.end())
        {
            const auto SkinIndex = skin_it->second;
            if (SkinIndex >= 0)
            {
                auto& N               = m_Model.Nodes[i];
                N.pSkin               = &m_Model.Skins[SkinIndex];
                N.SkinTransformsIndex = m_Model.SkinTransformsCount++;
            }
        }
        else
        {
            UNEXPECTED("Node ", i, " has no assigned skin id. This appears to be a bug.");
        }
    }

    return true;
}

template <typename GltfModelType>
void ModelBuilder::Execute(const GltfModelType& GltfModel,
                           int                  SceneIndex,
                           IRenderDevice*       pDevice,
                           IDeviceContext*      pContext)
{
    LoadScenes(GltfModel, SceneIndex);

    for (const auto& scene : m_Model.Scenes)
    {
        for (const auto* pNode : scene.RootNodes)
        {
            // We temporarily store GLTF node index in the pointer
            const auto GltfNodeId = static_cast<int>(reinterpret_cast<size_t>(pNode));
            AllocateNode(GltfModel, GltfNodeId);
        }
    }

    m_Model.Nodes.shrink_to_fit();
    m_Model.Meshes.shrink_to_fit();
    m_Model.Cameras.shrink_to_fit();

    for (auto& scene : m_Model.Scenes)
    {
        for (size_t i = 0; i < scene.RootNodes.size(); ++i)
        {
            auto&      pNode      = scene.RootNodes[i];
            const auto GltfNodeId = static_cast<int>(reinterpret_cast<size_t>(pNode));
            pNode                 = LoadNode(GltfModel, nullptr, scene, GltfNodeId);
        }
        scene.LinearNodes.shrink_to_fit();
    }
    VERIFY_EXPR(m_LoadedNodes.size() == m_Model.Nodes.size());
    VERIFY_EXPR(m_LoadedMeshes.size() == m_Model.Meshes.size());
    VERIFY_EXPR(m_LoadedCameras.size() == m_Model.Cameras.size());

    LoadAnimationAndSkin(GltfModel);

    InitIndexBuffer(pDevice, pContext);
    InitVertexBuffers(pDevice, pContext);

    if (pContext != nullptr)
    {
        m_Model.PrepareGPUResources(pDevice, pContext);
    }
}

} // namespace GLTF

} // namespace Diligent
