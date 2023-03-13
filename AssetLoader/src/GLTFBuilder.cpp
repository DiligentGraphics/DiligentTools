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

#include "GLTFBuilder.hpp"
#include "GLTFLoader.hpp"
#include "GraphicsAccessories.hpp"
#include "DataBlobImpl.hpp"

namespace Diligent
{

namespace GLTF
{

ModelBuilder::ModelBuilder(const ModelCreateInfo& _CI, Model& _Model) :
    m_CI{_CI},
    m_Model{_Model}
{
    VERIFY_EXPR(!m_Model.Buffers.empty());
    m_VertexData.resize(m_Model.Buffers.size() - 1);
}

ModelBuilder::~ModelBuilder()
{
}

bool ModelBuilder::ConvertedBufferViewKey::operator==(const ConvertedBufferViewKey& Rhs) const noexcept
{
    return AccessorIds == Rhs.AccessorIds;
}

size_t ModelBuilder::ConvertedBufferViewKey::Hasher::operator()(const ConvertedBufferViewKey& Key) const noexcept
{
    if (Key.Hash == 0)
    {
        Key.Hash = ComputeHash(Key.AccessorIds.size());
        for (auto Id : Key.AccessorIds)
            HashCombine(Key.Hash, Id);
    }
    return Key.Hash;
}


template <typename SrcType, typename DstType>
inline void WriteGltfData(const void*                  pSrc,
                          Uint32                       NumComponents,
                          Uint32                       SrcElemStride,
                          std::vector<Uint8>::iterator dst_it,
                          Uint32                       DstElementStride,
                          Uint32                       NumElements)
{
    for (size_t elem = 0; elem < NumElements; ++elem)
    {
        const auto* pSrcCmp = reinterpret_cast<const SrcType*>(static_cast<const Uint8*>(pSrc) + SrcElemStride * elem);

        auto comp_it = dst_it + DstElementStride * elem;
        for (Uint32 cmp = 0; cmp < NumComponents; ++cmp, comp_it += sizeof(DstType))
        {
            reinterpret_cast<DstType&>(*comp_it) = static_cast<DstType>(pSrcCmp[cmp]);
        }
    }
}

void ModelBuilder::WriteGltfData(const void*                  pSrc,
                                 VALUE_TYPE                   SrcType,
                                 Uint32                       NumSrcComponents,
                                 Uint32                       SrcElemStride,
                                 std::vector<Uint8>::iterator dst_it,
                                 VALUE_TYPE                   DstType,
                                 Uint32                       NumDstComponents,
                                 Uint32                       DstElementStride,
                                 Uint32                       NumElements)
{
    const auto NumComponentsToCopy = std::min(NumSrcComponents, NumDstComponents);

#define INNER_CASE(SrcType, DstType)                                                          \
    case DstType:                                                                             \
        GLTF::WriteGltfData<typename VALUE_TYPE2CType<SrcType>::CType,                        \
                            typename VALUE_TYPE2CType<DstType>::CType>(                       \
            pSrc, NumComponentsToCopy, SrcElemStride, dst_it, DstElementStride, NumElements); \
        break

#define CASE(SrcType)                                      \
    case SrcType:                                          \
        switch (DstType)                                   \
        {                                                  \
            INNER_CASE(SrcType, VT_INT8);                  \
            INNER_CASE(SrcType, VT_INT16);                 \
            INNER_CASE(SrcType, VT_INT32);                 \
            INNER_CASE(SrcType, VT_UINT8);                 \
            INNER_CASE(SrcType, VT_UINT16);                \
            INNER_CASE(SrcType, VT_UINT32);                \
            /*INNER_CASE(SrcType, VT_FLOAT16);*/           \
            INNER_CASE(SrcType, VT_FLOAT32);               \
            /*INNER_CASE(SrcType, VT_FLOAT64);*/           \
            default:                                       \
                UNEXPECTED("Unexpected destination type"); \
        }                                                  \
        break

    switch (SrcType)
    {
        CASE(VT_INT8);
        CASE(VT_INT16);
        CASE(VT_INT32);
        CASE(VT_UINT8);
        CASE(VT_UINT16);
        CASE(VT_UINT32);
        //CASE(VT_FLOAT16);
        CASE(VT_FLOAT32);
        //CASE(VT_FLOAT64);
        default:
            UNEXPECTED("Unexpected source type");
    }
#undef CASE
#undef INNER_CASE
}


void ModelBuilder::InitBuffers(IRenderDevice* pDevice, IDeviceContext* pContext)
{
    auto& Buffers = m_Model.Buffers;
    for (Uint32 BuffId = 0; BuffId < Buffers.size(); ++BuffId)
    {
        const auto IsIndexBuff = (BuffId == Buffers.size() - 1);

        const auto& Data = IsIndexBuff ? m_IndexData : m_VertexData[BuffId];
        if (Data.empty())
            continue;

        std::string Name = IsIndexBuff ?
            "GLTF index buffer" :
            std::string{"GLTF vertex buffer "} + std::to_string(BuffId);
        const auto ElementStride = Buffers[BuffId].ElementStride;
        VERIFY_EXPR(ElementStride > 0);
        VERIFY_EXPR(Data.size() % ElementStride == 0);

        VERIFY(!Buffers[BuffId].pSuballocation && !Buffers[BuffId].pBuffer, "This buffer has already been initialized");

        const auto BufferSize = StaticCast<Uint32>(Data.size());
        if (auto* const pResourceMgr = m_CI.pCacheInfo != nullptr ? m_CI.pCacheInfo->pResourceMgr : nullptr)
        {
            Uint32 CacheBufferIndex = IsIndexBuff ?
                m_CI.pCacheInfo->IndexBufferIdx :
                m_CI.pCacheInfo->VertexBufferIdx[BuffId];

            Buffers[BuffId].pSuballocation = pResourceMgr->AllocateBufferSpace(CacheBufferIndex, BufferSize, 1);

            auto pBuffInitData = DataBlobImpl::Create(BufferSize);
            memcpy(pBuffInitData->GetDataPtr(), Data.data(), BufferSize);
            Buffers[BuffId].pSuballocation->SetUserData(pBuffInitData);
        }
        else
        {
            BufferDesc BuffDesc;
            BuffDesc.Name      = Name.c_str();
            BuffDesc.Size      = BufferSize;
            BuffDesc.BindFlags = IsIndexBuff ? m_CI.IndBufferBindFlags : m_CI.VertBufferBindFlags;
            BuffDesc.Usage     = USAGE_IMMUTABLE;
            if (BuffDesc.BindFlags & (BIND_SHADER_RESOURCE | BIND_UNORDERED_ACCESS))
            {
                BuffDesc.Mode = IsIndexBuff ?
                    BUFFER_MODE_FORMATTED :
                    BUFFER_MODE_STRUCTURED;

                BuffDesc.ElementByteStride = ElementStride;
            }

            BufferData BuffData{Data.data(), BuffDesc.Size};
            pDevice->CreateBuffer(BuffDesc, &BuffData, &Buffers[BuffId].pBuffer);
        }
    }
}

} // namespace GLTF

} // namespace Diligent
