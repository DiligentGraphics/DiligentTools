/*
 *  Copyright 2019-2026 Diligent Graphics LLC
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

#include "GLTFBuilder.hpp"
#include "GLTFLoader.hpp"
#include "GraphicsAccessories.hpp"

namespace Diligent
{

namespace GLTF
{

MeshLoader::MeshLoader(const ModelCreateInfo& _CI, Model& _Model) :
    m_CI{_CI},
    m_Model{_Model}
{
    VERIFY_EXPR(!m_Model.VertexData.Strides.empty());
    m_VertexData.resize(m_Model.VertexData.Strides.size());
}

Mesh* MeshLoader::GetLoadedMesh(int LoadedMeshId)
{
    return &m_Model.Meshes[LoadedMeshId];
}

size_t MeshLoader::PrimitiveKey::Hasher::operator()(const PrimitiveKey& Key) const noexcept
{
    if (Key.Hash == 0)
    {
        Key.Hash = ComputeHash(Key.AccessorIds.size());
        for (int Id : Key.AccessorIds)
            HashCombine(Key.Hash, Id);
    }
    return Key.Hash;
}

void MeshLoader::WriteDefaultAttibutes(Uint32 BufferId, size_t StartOffset, size_t EndOffset)
{
    const Uint32 VertexStride = m_Model.VertexData.Strides[BufferId];
    VERIFY(StartOffset % VertexStride == 0, "Start offset is not aligned to vertex stride");
    VERIFY(EndOffset % VertexStride == 0, "End offset is not aligned to vertex stride");
    const Uint32 NumVertices = static_cast<Uint32>((EndOffset - StartOffset) / VertexStride);
    for (Uint32 i = 0; i < m_Model.GetNumVertexAttributes(); ++i)
    {
        const VertexAttributeDesc& Attrib = m_Model.VertexAttributes[i];
        if (BufferId != Attrib.BufferId || Attrib.pDefaultValue == nullptr)
            continue;

        const bool Written = VertexDataConverter::WriteDefault({
            Attrib.pDefaultValue,
            &m_VertexData[BufferId][StartOffset + Attrib.RelativeOffset],
            Attrib.ValueType,
            Attrib.NumComponents,
            VertexStride,
            NumVertices,
        });
        VERIFY_EXPR(Written);
    }
}

template <typename GetDstBufferFn, typename GetDstOffsetFn>
static void ScheduleBufferUpdate(IGPUUploadManager*            pUploadMgr,
                                 RefCntAutoPtr<BufferInitData> pBuffInitData,
                                 Uint32                        ChunkId,
                                 GetDstBufferFn                GetDstBuffer,
                                 GetDstOffsetFn                GetDstOffset)
{
    BufferInitData::ChunkData& Chunk = pBuffInitData->Chunks[ChunkId];

    Chunk.CopyStatus = BufferInitData::AsyncCopyStatus::Pending;

    struct UploadData
    {
        RefCntAutoPtr<BufferInitData> pBuffInitData;
        const Uint32                  ChunkId;
        GetDstBufferFn                GetDstBuffer;
        GetDstOffsetFn                GetDstOffset;
    };

    UploadData* pUploadData = new UploadData{
        std::move(pBuffInitData),
        ChunkId,
        std::move(GetDstBuffer),
        std::move(GetDstOffset),
    };

    auto CopyBufferCallback = [](IDeviceContext* pContext,
                                 IBuffer*        pSrcBuffer,
                                 Uint32          SrcOffset,
                                 Uint32          NumBytes,
                                 void*           pData) {
        std::unique_ptr<UploadData> Data{reinterpret_cast<UploadData*>(pData)};

        IBuffer* pDstBuffer = Data->GetDstBuffer(pContext);
        if (pDstBuffer == nullptr)
        {
            UNEXPECTED("Failed to update the buffer allocation");
            return;
        }

        const Uint32 DstOffset = Data->GetDstOffset();
        pContext->CopyBuffer(pSrcBuffer, SrcOffset, RESOURCE_STATE_TRANSITION_MODE_TRANSITION,
                             pDstBuffer, DstOffset, NumBytes, RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

        Data->pBuffInitData->Chunks[Data->ChunkId].CopyStatus = BufferInitData::AsyncCopyStatus::Enqueued;
    };

    pUploadMgr->ScheduleBufferUpdate(
        {
            nullptr,
            static_cast<Uint32>(Chunk.Bytes.size()),
            Chunk.Bytes.data(),
            CopyBufferCallback,
            pUploadData,
        });

    // Release CPU-side staging bytes
    Chunk.Bytes = std::vector<Uint8>{};
}

static void ScheduleIndexBufferUpdate(IRenderDevice*                      pDevice,
                                      IGPUUploadManager*                  pUploadMgr,
                                      RefCntAutoPtr<IBufferSuballocation> pAllocation,
                                      RefCntAutoPtr<BufferInitData>       pBuffInitData)
{
    VERIFY_EXPR(pAllocation && pBuffInitData);
    ScheduleBufferUpdate(
        pUploadMgr, pBuffInitData, 0,
        [pDevice, pAllocation](IDeviceContext* pContext) -> IBuffer* {
            return pAllocation->Update(pDevice, pContext);
        },
        [pAllocation]() -> Uint32 {
            return pAllocation->GetOffset();
        });
}

static void ScheduleVertexBufferUpdate(IRenderDevice*                       pDevice,
                                       IGPUUploadManager*                   pUploadMgr,
                                       RefCntAutoPtr<IVertexPoolAllocation> pAllocation,
                                       RefCntAutoPtr<BufferInitData>        pBuffInitData,
                                       Uint32                               BufferId,
                                       Uint32                               VertexStride)
{
    VERIFY_EXPR(pAllocation && pBuffInitData);
    ScheduleBufferUpdate(
        pUploadMgr, pBuffInitData, BufferId,
        [pDevice, pAllocation, BufferId](IDeviceContext* pContext) -> IBuffer* {
            return pAllocation->Update(BufferId, pDevice, pContext);
        },
        [pAllocation, VertexStride]() -> Uint32 {
            return pAllocation->GetStartVertex() * VertexStride;
        });
}

void MeshLoader::InitIndexBuffer(IRenderDevice* pDevice)
{
    if (m_IndexData.empty())
        return;

    VERIFY_EXPR(m_Model.IndexData.IndexSize > 0);
    VERIFY_EXPR((m_IndexData.size() % m_Model.IndexData.IndexSize) == 0);
    VERIFY(!m_Model.IndexData.pBuffer && !m_Model.IndexData.pAllocation, "Index buffer has already been initialized");

    const Uint32 DataSize = static_cast<Uint32>(m_IndexData.size());
    if (m_CI.pResourceManager != nullptr)
    {
        m_Model.IndexData.pAllocation = m_CI.pResourceManager->AllocateIndices(DataSize, 4);

        if (m_Model.IndexData.pAllocation)
        {
            RefCntAutoPtr<BufferInitData> pBuffInitData = BufferInitData::Create();
            pBuffInitData->Add(std::move(m_IndexData));
            if (m_CI.pUploadMgr)
            {
                ScheduleIndexBufferUpdate(pDevice, m_CI.pUploadMgr, m_Model.IndexData.pAllocation, pBuffInitData);
            }
            m_Model.IndexData.pAllocation->SetUserData(pBuffInitData);

            m_Model.IndexData.AllocatorId = m_CI.pResourceManager->GetIndexAllocatorIndex(m_Model.IndexData.pAllocation->GetAllocator());
            VERIFY_EXPR(m_Model.IndexData.AllocatorId != ~0u);
        }
        else
        {
            UNEXPECTED("Failed to allocate indices from the pool.");
        }
    }
    else
    {
        if (pDevice != nullptr)
        {
            const BIND_FLAGS BindFlags = m_CI.IndBufferBindFlags != BIND_NONE ? m_CI.IndBufferBindFlags : BIND_INDEX_BUFFER;
            BufferDesc       BuffDesc{"GLTF index buffer", DataSize, BindFlags, USAGE_IMMUTABLE};
            if (BuffDesc.BindFlags & (BIND_SHADER_RESOURCE | BIND_UNORDERED_ACCESS))
            {
                BuffDesc.Mode              = BUFFER_MODE_FORMATTED;
                BuffDesc.ElementByteStride = m_Model.IndexData.IndexSize;
            }

            BufferData BuffData{m_IndexData.data(), BuffDesc.Size};
            pDevice->CreateBuffer(BuffDesc, &BuffData, &m_Model.IndexData.pBuffer);
        }
        else
        {
            // CPU-only metadata load: primitives still describe index ranges,
            // but no GPU index buffer object is created.
            m_IndexData.clear();
        }
    }
}

void MeshLoader::InitVertexBuffers(IRenderDevice* pDevice)
{
    if (m_VertexData.empty())
    {
        UNEXPECTED("Vertex data can't be empty.");
        return;
    }

    const size_t VBCount = m_Model.GetVertexBufferCount();
    VERIFY_EXPR(m_VertexData.size() == VBCount);

    size_t NumVertices = 0;
    for (Uint32 i = 0; i < VBCount; ++i)
    {
        if (!m_VertexData[i].empty())
        {
            NumVertices = m_VertexData[i].size() / m_Model.VertexData.Strides[i];
            break;
        }
    }
#ifdef DILIGENT_DEBUG
    for (Uint32 i = 0; i < VBCount; ++i)
    {
        VERIFY(m_VertexData[i].empty() || NumVertices == m_VertexData[i].size() / m_Model.VertexData.Strides[i], "Inconsistent number of vertices in different buffers.");
    }
#endif

    if (NumVertices == 0)
    {
        m_Model.VertexData.Buffers.resize(VBCount);
        return;
    }

    if (m_CI.pResourceManager != nullptr)
    {
        ResourceManager::VertexLayoutKey LayoutKey;
        LayoutKey.Elements.reserve(VBCount);
        for (Uint32 i = 0; i < VBCount; ++i)
        {
            const BIND_FLAGS BindFlags = m_CI.VertBufferBindFlags[i] != BIND_NONE ? m_CI.VertBufferBindFlags[i] : BIND_VERTEX_BUFFER;
            LayoutKey.Elements.emplace_back(m_Model.VertexData.Strides[i], BindFlags);
        }

        VERIFY(!m_Model.VertexData.pAllocation, "This vertex buffer has already been initialized");
        m_Model.VertexData.pAllocation = m_CI.pResourceManager->AllocateVertices(LayoutKey, static_cast<Uint32>(NumVertices));
        if (m_Model.VertexData.pAllocation)
        {
            RefCntAutoPtr<BufferInitData> pBuffInitData = BufferInitData::Create();

            pBuffInitData->Reserve(m_VertexData.size());
            for (Uint32 i = 0; i < m_VertexData.size(); ++i)
            {
                pBuffInitData->Add(std::move(m_VertexData[i]));
                if (m_CI.pUploadMgr)
                {
                    ScheduleVertexBufferUpdate(pDevice, m_CI.pUploadMgr, m_Model.VertexData.pAllocation, pBuffInitData, i, m_Model.VertexData.Strides[i]);
                }
            }
            m_Model.VertexData.pAllocation->SetUserData(pBuffInitData);
            m_Model.VertexData.PoolId = m_CI.pResourceManager->GetVertexPoolIndex(LayoutKey, m_Model.VertexData.pAllocation->GetPool());
            VERIFY_EXPR(m_Model.VertexData.PoolId != ~0u);
        }
        else
        {
            UNEXPECTED("Failed to allocate vertices from the pool. Make sure that you proived the required layout when creating the pool.");
        }
    }
    else
    {
        VERIFY(m_Model.VertexData.Buffers.empty(), "Vertex buffers have already been initialized");
        if (pDevice != nullptr)
        {
            m_Model.VertexData.Buffers.resize(VBCount);
            for (Uint32 i = 0; i < VBCount; ++i)
            {
                const std::vector<Uint8>& Data = m_VertexData[i];
                if (Data.empty())
                    continue;

                const Uint32      DataSize  = static_cast<Uint32>(Data.size());
                const std::string Name      = std::string{"GLTF vertex buffer "} + std::to_string(i);
                const BIND_FLAGS  BindFlags = m_CI.VertBufferBindFlags[i] != BIND_NONE ? m_CI.VertBufferBindFlags[i] : BIND_VERTEX_BUFFER;
                BufferDesc        BuffDesc{Name.c_str(), DataSize, BindFlags, USAGE_IMMUTABLE};

                const Uint32 ElementStride = m_Model.VertexData.Strides[i];
                VERIFY_EXPR(ElementStride > 0);
                VERIFY_EXPR(Data.size() % ElementStride == 0);

                if (BuffDesc.BindFlags & (BIND_SHADER_RESOURCE | BIND_UNORDERED_ACCESS))
                {
                    BuffDesc.Mode              = BUFFER_MODE_STRUCTURED;
                    BuffDesc.ElementByteStride = ElementStride;
                }

                VERIFY_EXPR(!m_Model.VertexData.Buffers[i]);
                BufferData BuffData{Data.data(), DataSize};
                pDevice->CreateBuffer(BuffDesc, &BuffData, &m_Model.VertexData.Buffers[i]);
            }
        }
        else
        {
            // CPU-only metadata load: preserve the expected buffer slots without
            // creating GPU vertex buffer objects.
            m_VertexData.clear();
        }
    }
}

} // namespace GLTF

} // namespace Diligent
