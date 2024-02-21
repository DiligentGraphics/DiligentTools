/*
 *  Copyright 2019-2024 Diligent Graphics LLC
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

ModelBuilder::ModelBuilder(const ModelCreateInfo& _CI, Model& _Model) :
    m_CI{_CI},
    m_Model{_Model}
{
    VERIFY_EXPR(!m_Model.VertexData.Strides.empty());
    m_VertexData.resize(m_Model.VertexData.Strides.size());
}

ModelBuilder::~ModelBuilder()
{
}

size_t ModelBuilder::PrimitiveKey::Hasher::operator()(const PrimitiveKey& Key) const noexcept
{
    if (Key.Hash == 0)
    {
        Key.Hash = ComputeHash(Key.AccessorIds.size());
        for (auto Id : Key.AccessorIds)
            HashCombine(Key.Hash, Id);
    }
    return Key.Hash;
}

template <typename DstType, bool Normalize, typename SrcType>
inline DstType ConvertElement(SrcType Src)
{
    return static_cast<DstType>(Src);
}

// =========================== float -> Int8/Uint8 ============================
template <>
inline Uint8 ConvertElement<Uint8, true, float>(float Src)
{
    return static_cast<Uint8>(clamp(Src * 255.f + 0.5f, 0.f, 255.f));
}

template <>
inline Uint8 ConvertElement<Uint8, false, float>(float Src)
{
    return ConvertElement<Uint8, true>(Src);
}

template <>
inline Int8 ConvertElement<Int8, true, float>(float Src)
{
    auto r = Src > 0.f ? +0.5f : -0.5f;
    return static_cast<Int8>(clamp(Src * 127.f + r, -127.f, 127.f));
}

template <>
inline Int8 ConvertElement<Int8, false, float>(float Src)
{
    return ConvertElement<Int8, true>(Src);
}


// =========================== Int8/Uint8 -> float ============================
template <>
inline float ConvertElement<float, true, Int8>(Int8 Src)
{
    return std::max(static_cast<float>(Src), -127.f) / 127.f;
}

template <>
inline float ConvertElement<float, true, Uint8>(Uint8 Src)
{
    return static_cast<float>(Src) / 255.f;
}


// ========================== Int16/Uint16 -> float ===========================
template <>
inline float ConvertElement<float, true, Int16>(Int16 Src)
{
    return std::max(static_cast<float>(Src), -32767.f) / 32767.f;
}

template <>
inline float ConvertElement<float, true, Uint16>(Uint16 Src)
{
    return static_cast<float>(Src) / 65535.f;
}


template <typename SrcType, typename DstType, bool IsNormalized>
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
            reinterpret_cast<DstType&>(*comp_it) = ConvertElement<DstType, IsNormalized>(pSrcCmp[cmp]);
        }
    }
}

void ModelBuilder::WriteGltfData(const WriteGltfDataAttribs& Attribs)
{
    const auto NumComponentsToCopy = std::min(Attribs.NumSrcComponents, Attribs.NumDstComponents);

#define INNER_CASE(SrcType, DstType)                                            \
    case DstType:                                                               \
        if (Attribs.IsNormalized)                                               \
        {                                                                       \
            GLTF::WriteGltfData<typename VALUE_TYPE2CType<SrcType>::CType,      \
                                typename VALUE_TYPE2CType<DstType>::CType,      \
                                true>(                                          \
                Attribs.pSrc, NumComponentsToCopy, Attribs.SrcElemStride,       \
                Attribs.dst_it, Attribs.DstElementStride, Attribs.NumElements); \
        }                                                                       \
        else                                                                    \
        {                                                                       \
            GLTF::WriteGltfData<typename VALUE_TYPE2CType<SrcType>::CType,      \
                                typename VALUE_TYPE2CType<DstType>::CType,      \
                                false>(                                         \
                Attribs.pSrc, NumComponentsToCopy, Attribs.SrcElemStride,       \
                Attribs.dst_it, Attribs.DstElementStride, Attribs.NumElements); \
        }                                                                       \
        break

#define CASE(SrcType)                                      \
    case SrcType:                                          \
        switch (Attribs.DstType)                           \
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

    switch (Attribs.SrcType)
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

void ModelBuilder::WriteDefaultAttibuteValue(const void*                  pDefaultValue,
                                             std::vector<Uint8>::iterator dst_it,
                                             VALUE_TYPE                   DstType,
                                             Uint32                       NumDstComponents,
                                             Uint32                       DstElementStride,
                                             Uint32                       NumElements)
{
    auto ElementSize = GetValueSize(DstType) * NumDstComponents;
    VERIFY(DstElementStride >= ElementSize, "Destination element stride is too small");
    for (size_t elem = 0; elem < NumElements; ++elem)
    {
        //  Note: MSVC asserts when moving iterator past the end of the vector
        memcpy(&*(dst_it + DstElementStride * elem), pDefaultValue, ElementSize);
    }
}

void ModelBuilder::WriteDefaultAttibutes(Uint32 BufferId, size_t StartOffset, size_t EndOffset)
{
    const auto VertexStride = m_Model.VertexData.Strides[BufferId];
    VERIFY(StartOffset % VertexStride == 0, "Start offset is not aligned to vertex stride");
    VERIFY(EndOffset % VertexStride == 0, "End offset is not aligned to vertex stride");
    const auto NumVertices = static_cast<Uint32>((EndOffset - StartOffset) / VertexStride);
    for (Uint32 i = 0; i < m_Model.GetNumVertexAttributes(); ++i)
    {
        const auto& Attrib = m_Model.VertexAttributes[i];
        if (BufferId != Attrib.BufferId || Attrib.pDefaultValue == nullptr)
            continue;

        WriteDefaultAttibuteValue(Attrib.pDefaultValue,
                                  m_VertexData[BufferId].begin() + StartOffset + Attrib.RelativeOffset,
                                  Attrib.ValueType,
                                  Attrib.NumComponents,
                                  VertexStride,
                                  NumVertices);
    }
}

void ModelBuilder::InitIndexBuffer(IRenderDevice* pDevice)
{
    if (m_IndexData.empty())
        return;

    VERIFY_EXPR(m_Model.IndexData.IndexSize > 0);
    VERIFY_EXPR((m_IndexData.size() % m_Model.IndexData.IndexSize) == 0);
    VERIFY(!m_Model.IndexData.pBuffer && !m_Model.IndexData.pAllocation, "Index buffer has already been initialized");

    const auto DataSize = static_cast<Uint32>(m_IndexData.size());
    if (m_CI.pResourceManager != nullptr)
    {
        m_Model.IndexData.pAllocation = m_CI.pResourceManager->AllocateIndices(DataSize, 4);

        if (m_Model.IndexData.pAllocation)
        {
            auto pBuffInitData = BufferInitData::Create();
            pBuffInitData->Data.emplace_back(std::move(m_IndexData));
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
        const auto BindFlags = m_CI.IndBufferBindFlags != BIND_NONE ? m_CI.IndBufferBindFlags : BIND_INDEX_BUFFER;
        BufferDesc BuffDesc{"GLTF index buffer", DataSize, BindFlags, USAGE_IMMUTABLE};
        if (BuffDesc.BindFlags & (BIND_SHADER_RESOURCE | BIND_UNORDERED_ACCESS))
        {
            BuffDesc.Mode              = BUFFER_MODE_FORMATTED;
            BuffDesc.ElementByteStride = m_Model.IndexData.IndexSize;
        }

        BufferData BuffData{m_IndexData.data(), BuffDesc.Size};
        pDevice->CreateBuffer(BuffDesc, &BuffData, &m_Model.IndexData.pBuffer);
    }
}

void ModelBuilder::InitVertexBuffers(IRenderDevice* pDevice)
{
    if (m_VertexData.empty())
    {
        UNEXPECTED("Vertex data can't be empty.");
        return;
    }

    const auto VBCount = m_Model.GetVertexBufferCount();
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
            const auto BindFlags = m_CI.VertBufferBindFlags[i] != BIND_NONE ? m_CI.VertBufferBindFlags[i] : BIND_VERTEX_BUFFER;
            LayoutKey.Elements.emplace_back(m_Model.VertexData.Strides[i], BindFlags);
        }

        VERIFY(!m_Model.VertexData.pAllocation, "This vertex buffer has already been initialized");
        m_Model.VertexData.pAllocation = m_CI.pResourceManager->AllocateVertices(LayoutKey, static_cast<Uint32>(NumVertices));
        if (m_Model.VertexData.pAllocation)
        {
            auto pBuffInitData  = BufferInitData::Create();
            pBuffInitData->Data = std::move(m_VertexData);
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
        m_Model.VertexData.Buffers.resize(VBCount);
        for (Uint32 i = 0; i < VBCount; ++i)
        {
            const auto& Data = m_VertexData[i];
            if (Data.empty())
                continue;

            const auto DataSize  = static_cast<Uint32>(Data.size());
            const auto Name      = std::string{"GLTF vertex buffer "} + std::to_string(i);
            const auto BindFlags = m_CI.VertBufferBindFlags[i] != BIND_NONE ? m_CI.VertBufferBindFlags[i] : BIND_VERTEX_BUFFER;
            BufferDesc BuffDesc{Name.c_str(), DataSize, BindFlags, USAGE_IMMUTABLE};

            const auto ElementStride = m_Model.VertexData.Strides[i];
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
}

std::pair<FILTER_TYPE, FILTER_TYPE> ModelBuilder::GetFilterType(int32_t GltfFilterMode)
{
    switch (GltfFilterMode)
    {
        case 9728: // NEAREST
            return {FILTER_TYPE_POINT, FILTER_TYPE_POINT};
        case 9729: // LINEAR
            return {FILTER_TYPE_LINEAR, FILTER_TYPE_LINEAR};
        case 9984: // NEAREST_MIPMAP_NEAREST
            return {FILTER_TYPE_POINT, FILTER_TYPE_POINT};
        case 9985: // LINEAR_MIPMAP_NEAREST
            return {FILTER_TYPE_LINEAR, FILTER_TYPE_POINT};
        case 9986: // NEAREST_MIPMAP_LINEAR
            return {FILTER_TYPE_POINT, FILTER_TYPE_LINEAR};
        case 9987: // LINEAR_MIPMAP_LINEAR
            return {FILTER_TYPE_LINEAR, FILTER_TYPE_LINEAR};
        default:
            LOG_WARNING_MESSAGE("Unknown gltf filter mode: ", GltfFilterMode, ". Defaulting to linear.");
            return {FILTER_TYPE_LINEAR, FILTER_TYPE_LINEAR};
    }
}

TEXTURE_ADDRESS_MODE ModelBuilder::GetAddressMode(int32_t GltfWrapMode)
{
    switch (GltfWrapMode)
    {
        case 10497:
            return TEXTURE_ADDRESS_WRAP;
        case 33071:
            return TEXTURE_ADDRESS_CLAMP;
        case 33648:
            return TEXTURE_ADDRESS_MIRROR;
        default:
            LOG_WARNING_MESSAGE("Unknown gltf address wrap mode: ", GltfWrapMode, ". Defaulting to WRAP.");
            return TEXTURE_ADDRESS_WRAP;
    }
}

} // namespace GLTF

} // namespace Diligent
