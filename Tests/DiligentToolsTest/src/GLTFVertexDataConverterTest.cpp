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

#include "TestingEnvironment.hpp"
#include "gtest/gtest.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstring>
#include <vector>

using namespace Diligent;
using namespace Diligent::Testing;

namespace
{

template <typename ValueType>
ValueType ReadValue(const std::vector<Uint8>& Data, size_t Offset)
{
    ValueType Value{};
    EXPECT_LE(Offset + sizeof(ValueType), Data.size());
    if (Offset + sizeof(ValueType) <= Data.size())
        std::memcpy(&Value, Data.data() + Offset, sizeof(ValueType));
    return Value;
}

template <typename ValueType>
void WriteValue(std::vector<Uint8>& Data, size_t Offset, ValueType Value)
{
    ASSERT_LE(Offset + sizeof(ValueType), Data.size());
    if (Offset + sizeof(ValueType) <= Data.size())
        std::memcpy(Data.data() + Offset, &Value, sizeof(ValueType));
}

template <typename ValueType>
ValueType MakeSourceValue(Uint32 Element, Uint32 Component)
{
    return static_cast<ValueType>(1 + Element * 3 + Component);
}

template <>
Float32 MakeSourceValue<Float32>(Uint32 Element, Uint32 Component)
{
    return 0.25f * static_cast<Float32>(1 + Element * 3 + Component);
}

template <typename ValueType>
ValueType MakeDefaultValue(Uint32 Component)
{
    return static_cast<ValueType>(Component + 7);
}

template <>
Float32 MakeDefaultValue<Float32>(Uint32 Component)
{
    return 0.25f * static_cast<Float32>(Component + 1);
}

template <typename ValueType>
void ExpectValueEq(ValueType Actual, ValueType Expected)
{
    EXPECT_EQ(Actual, Expected);
}

void ExpectValueEq(Float32 Actual, Float32 Expected)
{
    EXPECT_NEAR(Actual, Expected, 1e-6f);
}

template <typename ValueType>
void ExpectBytesUnchanged(const std::vector<Uint8>& Data, size_t Offset, size_t Size, Uint8 Sentinel)
{
    ASSERT_LE(Offset + Size, Data.size());
    for (size_t Byte = 0; Byte < Size && Offset + Byte < Data.size(); ++Byte)
        EXPECT_EQ(Data[Offset + Byte], Sentinel);
}

template <typename DstType, bool Normalize, typename SrcType>
DstType ExpectedConvert(SrcType Src)
{
    return static_cast<DstType>(Src);
}

template <>
Uint8 ExpectedConvert<Uint8, true, Float32>(Float32 Src)
{
    const Float32 Value = std::max(0.f, std::min(Src * 255.f + 0.5f, 255.f));
    return static_cast<Uint8>(Value);
}

template <>
Uint8 ExpectedConvert<Uint8, false, Float32>(Float32 Src)
{
    return ExpectedConvert<Uint8, true>(Src);
}

template <>
Int8 ExpectedConvert<Int8, true, Float32>(Float32 Src)
{
    const Float32 Round = Src > 0.f ? +0.5f : -0.5f;
    const Float32 Value = std::max(-127.f, std::min(Src * 127.f + Round, 127.f));
    return static_cast<Int8>(Value);
}

template <>
Int8 ExpectedConvert<Int8, false, Float32>(Float32 Src)
{
    return ExpectedConvert<Int8, true>(Src);
}

template <>
Float32 ExpectedConvert<Float32, true, Int8>(Int8 Src)
{
    return std::max(static_cast<Float32>(Src), -127.f) / 127.f;
}

template <>
Float32 ExpectedConvert<Float32, true, Uint8>(Uint8 Src)
{
    return static_cast<Float32>(Src) / 255.f;
}

template <>
Float32 ExpectedConvert<Float32, true, Int16>(Int16 Src)
{
    return std::max(static_cast<Float32>(Src), -32767.f) / 32767.f;
}

template <>
Float32 ExpectedConvert<Float32, true, Uint16>(Uint16 Src)
{
    return static_cast<Float32>(Src) / 65535.f;
}

template <VALUE_TYPE SrcValueType, VALUE_TYPE DstValueType, bool IsNormalized>
void TestWriteTypePair(Uint32 NumSrcComponents = 3, Uint32 NumDstComponents = 4)
{
    using SrcType = typename VALUE_TYPE2CType<SrcValueType>::CType;
    using DstType = typename VALUE_TYPE2CType<DstValueType>::CType;

    constexpr Uint32 NumElements = 2;
    constexpr Uint8  Sentinel    = 0xA5;

    const Uint32 NumComponentsToCopy = std::min(NumSrcComponents, NumDstComponents);

    const Uint32 SrcStride = sizeof(SrcType) * NumSrcComponents + 5;
    const Uint32 DstStride = sizeof(DstType) * NumDstComponents + 7;

    std::vector<Uint8> SrcData(size_t{NumElements} * SrcStride, 0xCD);
    for (Uint32 Elem = 0; Elem < NumElements; ++Elem)
    {
        for (Uint32 Cmp = 0; Cmp < NumSrcComponents; ++Cmp)
            WriteValue(SrcData, size_t{Elem} * SrcStride + size_t{Cmp} * sizeof(SrcType), MakeSourceValue<SrcType>(Elem, Cmp));
    }

    std::vector<Uint8> DstData(size_t{NumElements} * DstStride, Sentinel);

    const bool Written = GLTF::VertexDataConverter::Write({
        SrcData.data(),
        SrcValueType,
        NumSrcComponents,
        SrcStride,
        DstData.data(),
        DstValueType,
        NumDstComponents,
        DstStride,
        NumElements,
        IsNormalized,
    });
    ASSERT_TRUE(Written);

    for (Uint32 Elem = 0; Elem < NumElements; ++Elem)
    {
        const size_t DstElemOffset = size_t{Elem} * DstStride;
        for (Uint32 Cmp = 0; Cmp < NumComponentsToCopy; ++Cmp)
        {
            const SrcType SrcValue = MakeSourceValue<SrcType>(Elem, Cmp);
            const DstType Expected = ExpectedConvert<DstType, IsNormalized>(SrcValue);
            const DstType Actual   = ReadValue<DstType>(DstData, DstElemOffset + size_t{Cmp} * sizeof(DstType));
            ExpectValueEq(Actual, Expected);
        }

        if (NumDstComponents > NumComponentsToCopy)
        {
            ExpectBytesUnchanged<DstType>(DstData,
                                          DstElemOffset + size_t{NumComponentsToCopy} * sizeof(DstType),
                                          size_t{NumDstComponents - NumComponentsToCopy} * sizeof(DstType),
                                          Sentinel);
        }
        ExpectBytesUnchanged<DstType>(DstData,
                                      DstElemOffset + size_t{NumDstComponents} * sizeof(DstType),
                                      DstStride - size_t{NumDstComponents} * sizeof(DstType),
                                      Sentinel);
    }
}

template <VALUE_TYPE SrcValueType, bool IsNormalized>
void TestWriteAllDestinationsForSource(Uint32 NumSrcComponents, Uint32 NumDstComponents)
{
    TestWriteTypePair<SrcValueType, VT_INT8, IsNormalized>(NumSrcComponents, NumDstComponents);
    TestWriteTypePair<SrcValueType, VT_INT16, IsNormalized>(NumSrcComponents, NumDstComponents);
    TestWriteTypePair<SrcValueType, VT_INT32, IsNormalized>(NumSrcComponents, NumDstComponents);
    TestWriteTypePair<SrcValueType, VT_UINT8, IsNormalized>(NumSrcComponents, NumDstComponents);
    TestWriteTypePair<SrcValueType, VT_UINT16, IsNormalized>(NumSrcComponents, NumDstComponents);
    TestWriteTypePair<SrcValueType, VT_UINT32, IsNormalized>(NumSrcComponents, NumDstComponents);
    TestWriteTypePair<SrcValueType, VT_FLOAT32, IsNormalized>(NumSrcComponents, NumDstComponents);
}

template <bool IsNormalized>
void TestWriteAllSupportedTypePairs(Uint32 NumSrcComponents, Uint32 NumDstComponents)
{
    TestWriteAllDestinationsForSource<VT_INT8, IsNormalized>(NumSrcComponents, NumDstComponents);
    TestWriteAllDestinationsForSource<VT_INT16, IsNormalized>(NumSrcComponents, NumDstComponents);
    TestWriteAllDestinationsForSource<VT_INT32, IsNormalized>(NumSrcComponents, NumDstComponents);
    TestWriteAllDestinationsForSource<VT_UINT8, IsNormalized>(NumSrcComponents, NumDstComponents);
    TestWriteAllDestinationsForSource<VT_UINT16, IsNormalized>(NumSrcComponents, NumDstComponents);
    TestWriteAllDestinationsForSource<VT_UINT32, IsNormalized>(NumSrcComponents, NumDstComponents);
    TestWriteAllDestinationsForSource<VT_FLOAT32, IsNormalized>(NumSrcComponents, NumDstComponents);
}

template <VALUE_TYPE DstValueType>
void TestWriteDefaultType()
{
    using DstType = typename VALUE_TYPE2CType<DstValueType>::CType;

    constexpr Uint32 NumElements      = 3;
    constexpr Uint32 NumDstComponents = 3;
    constexpr Uint8  Sentinel         = 0x5A;

    const Uint32 DstStride = sizeof(DstType) * NumDstComponents + 9;

    std::array<DstType, NumDstComponents> DefaultValue{};
    for (Uint32 Cmp = 0; Cmp < NumDstComponents; ++Cmp)
        DefaultValue[Cmp] = MakeDefaultValue<DstType>(Cmp);

    std::vector<Uint8> DstData(size_t{NumElements} * DstStride, Sentinel);

    const bool Written = GLTF::VertexDataConverter::WriteDefault({
        DefaultValue.data(),
        DstData.data(),
        DstValueType,
        NumDstComponents,
        DstStride,
        NumElements,
    });
    ASSERT_TRUE(Written);

    for (Uint32 Elem = 0; Elem < NumElements; ++Elem)
    {
        const size_t DstElemOffset = size_t{Elem} * DstStride;
        for (Uint32 Cmp = 0; Cmp < NumDstComponents; ++Cmp)
        {
            const DstType Actual = ReadValue<DstType>(DstData, DstElemOffset + size_t{Cmp} * sizeof(DstType));
            ExpectValueEq(Actual, DefaultValue[Cmp]);
        }

        ExpectBytesUnchanged<DstType>(DstData,
                                      DstElemOffset + size_t{NumDstComponents} * sizeof(DstType),
                                      DstStride - size_t{NumDstComponents} * sizeof(DstType),
                                      Sentinel);
    }
}

} // namespace

TEST(Tools_GLTFVertexDataConverter, WritesEverySupportedTypePair)
{
    TestWriteAllSupportedTypePairs<false>(3, 4);
}

TEST(Tools_GLTFVertexDataConverter, WritesEverySupportedNormalizedTypePair)
{
    TestWriteAllSupportedTypePairs<true>(3, 4);
}

TEST(Tools_GLTFVertexDataConverter, WritesEverySupportedTypePairWhenSourceHasMoreComponents)
{
    TestWriteAllSupportedTypePairs<false>(4, 3);
}

TEST(Tools_GLTFVertexDataConverter, WritesEverySupportedNormalizedTypePairWhenSourceHasMoreComponents)
{
    TestWriteAllSupportedTypePairs<true>(4, 3);
}

TEST(Tools_GLTFVertexDataConverter, WriteDefaultSupportsEveryValueType)
{
    TestWriteDefaultType<VT_INT8>();
    TestWriteDefaultType<VT_INT16>();
    TestWriteDefaultType<VT_INT32>();
    TestWriteDefaultType<VT_UINT8>();
    TestWriteDefaultType<VT_UINT16>();
    TestWriteDefaultType<VT_UINT32>();
    TestWriteDefaultType<VT_FLOAT16>();
    TestWriteDefaultType<VT_FLOAT32>();
    TestWriteDefaultType<VT_FLOAT64>();
}

TEST(Tools_GLTFVertexDataConverter, WriteRejectsInvalidAttribs)
{
    constexpr Uint32 NumComponents = 3;
    constexpr Uint32 NumElements   = 2;
    constexpr Uint32 Stride        = sizeof(Float32) * NumComponents;

    std::array<Float32, NumComponents * NumElements> SrcData{};
    std::array<Float32, NumComponents * NumElements> DstData{};

    const GLTF::VertexDataConverter::WriteAttribs ValidAttribs{
        SrcData.data(),
        VT_FLOAT32,
        NumComponents,
        Stride,
        DstData.data(),
        VT_FLOAT32,
        NumComponents,
        Stride,
        NumElements,
        false,
    };

    {
        auto Attribs = ValidAttribs;
        Attribs.pSrc = nullptr;
        EXPECT_FALSE(GLTF::VertexDataConverter::Write(Attribs));
    }
    {
        auto Attribs = ValidAttribs;
        Attribs.pDst = nullptr;
        EXPECT_FALSE(GLTF::VertexDataConverter::Write(Attribs));
    }
    {
        auto Attribs             = ValidAttribs;
        Attribs.NumSrcComponents = 0;
        EXPECT_FALSE(GLTF::VertexDataConverter::Write(Attribs));
    }
    {
        auto Attribs             = ValidAttribs;
        Attribs.NumDstComponents = 0;
        EXPECT_FALSE(GLTF::VertexDataConverter::Write(Attribs));
    }
    {
        auto Attribs             = ValidAttribs;
        Attribs.SrcElementStride = sizeof(Float32) * (NumComponents - 1);
        EXPECT_FALSE(GLTF::VertexDataConverter::Write(Attribs));
    }
    {
        auto Attribs             = ValidAttribs;
        Attribs.DstElementStride = sizeof(Float32) * (NumComponents - 1);
        EXPECT_FALSE(GLTF::VertexDataConverter::Write(Attribs));
    }
    {
        TestingEnvironment::ErrorScope ExpectedErrors{"Unexpected vertex data conversion type"};

        auto Attribs    = ValidAttribs;
        Attribs.SrcType = VT_FLOAT16;
        EXPECT_FALSE(GLTF::VertexDataConverter::Write(Attribs));
    }
    {
        TestingEnvironment::ErrorScope ExpectedErrors{"Unexpected vertex data conversion type"};

        auto Attribs    = ValidAttribs;
        Attribs.DstType = VT_FLOAT64;
        EXPECT_FALSE(GLTF::VertexDataConverter::Write(Attribs));
    }
}

TEST(Tools_GLTFVertexDataConverter, WriteDefaultRejectsInvalidAttribs)
{
    constexpr Uint32 NumComponents = 3;
    constexpr Uint32 NumElements   = 2;
    constexpr Uint32 Stride        = sizeof(Float32) * NumComponents;

    std::array<Float32, NumComponents>               DefaultValue{};
    std::array<Float32, NumComponents * NumElements> DstData{};

    const GLTF::VertexDataConverter::WriteDefaultAttribs ValidAttribs{
        DefaultValue.data(),
        DstData.data(),
        VT_FLOAT32,
        NumComponents,
        Stride,
        NumElements,
    };

    {
        auto Attribs          = ValidAttribs;
        Attribs.pDefaultValue = nullptr;
        EXPECT_FALSE(GLTF::VertexDataConverter::WriteDefault(Attribs));
    }
    {
        auto Attribs = ValidAttribs;
        Attribs.pDst = nullptr;
        EXPECT_FALSE(GLTF::VertexDataConverter::WriteDefault(Attribs));
    }
    {
        auto Attribs    = ValidAttribs;
        Attribs.DstType = VT_UNDEFINED;
        EXPECT_FALSE(GLTF::VertexDataConverter::WriteDefault(Attribs));
    }
    {
        auto Attribs             = ValidAttribs;
        Attribs.NumDstComponents = 0;
        EXPECT_FALSE(GLTF::VertexDataConverter::WriteDefault(Attribs));
    }
    {
        auto Attribs             = ValidAttribs;
        Attribs.DstElementStride = sizeof(Float32) * (NumComponents - 1);
        EXPECT_FALSE(GLTF::VertexDataConverter::WriteDefault(Attribs));
    }
}
