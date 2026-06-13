/*
 *  Copyright 2026 Diligent Graphics LLC
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

#include "GLTFVertexDataConverter.hpp"

#include "DebugUtilities.hpp"

#include <algorithm>
#include <cstring>
#include <type_traits>

namespace Diligent
{

namespace GLTF
{

namespace
{

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
    float r = Src > 0.f ? +0.5f : -0.5f;
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
bool WriteAttributeData(const VertexDataConverter::WriteAttribs& Attribs)
{
    const Uint32 NumComponentsToCopy = std::min(Attribs.NumSrcComponents, Attribs.NumDstComponents);
    if (Attribs.NumElements == 0)
        return true;

    if (Attribs.pSrc == nullptr ||
        Attribs.pDst == nullptr ||
        NumComponentsToCopy == 0 ||
        Attribs.SrcElementStride < sizeof(SrcType) * NumComponentsToCopy ||
        Attribs.DstElementStride < sizeof(DstType) * Attribs.NumDstComponents)
    {
        return false;
    }

    const Uint8* pSrcBytes = static_cast<const Uint8*>(Attribs.pSrc);
    Uint8*       pDstBytes = static_cast<Uint8*>(Attribs.pDst);

    if constexpr (std::is_same<SrcType, DstType>::value)
    {
        const size_t NumBytesToCopy = sizeof(SrcType) * size_t{NumComponentsToCopy};

        for (Uint32 Elem = 0; Elem < Attribs.NumElements; ++Elem)
        {
            const Uint8* pSrcCmpBytes = pSrcBytes + size_t{Attribs.SrcElementStride} * Elem;
            Uint8*       pDstCmpBytes = pDstBytes + size_t{Attribs.DstElementStride} * Elem;
            std::memcpy(pDstCmpBytes, pSrcCmpBytes, NumBytesToCopy);
        }
    }
    else
    {
        for (Uint32 Elem = 0; Elem < Attribs.NumElements; ++Elem)
        {
            const Uint8* pSrcCmpBytes = pSrcBytes + size_t{Attribs.SrcElementStride} * Elem;
            Uint8*       pDstCmpBytes = pDstBytes + size_t{Attribs.DstElementStride} * Elem;
            for (Uint32 Cmp = 0; Cmp < NumComponentsToCopy; ++Cmp)
            {
                SrcType SrcValue{};
                std::memcpy(&SrcValue, pSrcCmpBytes + size_t{Cmp} * sizeof(SrcType), sizeof(SrcValue));

                const DstType DstValue = ConvertElement<DstType, IsNormalized>(SrcValue);
                std::memcpy(pDstCmpBytes + size_t{Cmp} * sizeof(DstType), &DstValue, sizeof(DstValue));
            }
        }
    }

    return true;
}

bool IsSupportedValueType(VALUE_TYPE Type)
{
    switch (Type)
    {
        case VT_INT8:
        case VT_INT16:
        case VT_INT32:
        case VT_UINT8:
        case VT_UINT16:
        case VT_UINT32:
        case VT_FLOAT32:
            return true;

        default:
            return false;
    }
}

} // namespace

bool VertexDataConverter::Write(const WriteAttribs& Attribs)
{
    if (!IsSupportedValueType(Attribs.SrcType) ||
        !IsSupportedValueType(Attribs.DstType))
    {
        UNEXPECTED("Unexpected vertex data conversion type");
        return false;
    }

#define INNER_CASE(SrcType, DstType)                                            \
    case DstType:                                                               \
        return Attribs.IsNormalized ?                                           \
            GLTF::WriteAttributeData<typename VALUE_TYPE2CType<SrcType>::CType, \
                                     typename VALUE_TYPE2CType<DstType>::CType, \
                                     true>(Attribs) :                           \
            GLTF::WriteAttributeData<typename VALUE_TYPE2CType<SrcType>::CType, \
                                     typename VALUE_TYPE2CType<DstType>::CType, \
                                     false>(Attribs)

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
            INNER_CASE(SrcType, VT_FLOAT32);               \
            default:                                       \
                UNEXPECTED("Unexpected destination type"); \
                return false;                              \
        }

    switch (Attribs.SrcType)
    {
        CASE(VT_INT8);
        CASE(VT_INT16);
        CASE(VT_INT32);
        CASE(VT_UINT8);
        CASE(VT_UINT16);
        CASE(VT_UINT32);
        CASE(VT_FLOAT32);
        default:
            UNEXPECTED("Unexpected source type");
            return false;
    }
#undef CASE
#undef INNER_CASE
}

bool VertexDataConverter::WriteDefault(const WriteDefaultAttribs& Attribs)
{
    if (Attribs.NumElements == 0)
        return true;

    const Uint32 ElementSize = GetValueSize(Attribs.DstType) * Attribs.NumDstComponents;
    if (Attribs.pDefaultValue == nullptr ||
        Attribs.pDst == nullptr ||
        ElementSize == 0 ||
        Attribs.DstElementStride < ElementSize)
    {
        return false;
    }

    const Uint8* pDefaultBytes = static_cast<const Uint8*>(Attribs.pDefaultValue);
    Uint8*       pDstBytes     = static_cast<Uint8*>(Attribs.pDst);
    for (Uint32 Elem = 0; Elem < Attribs.NumElements; ++Elem)
        std::memcpy(pDstBytes + size_t{Attribs.DstElementStride} * Elem, pDefaultBytes, ElementSize);

    return true;
}

} // namespace GLTF

} // namespace Diligent
