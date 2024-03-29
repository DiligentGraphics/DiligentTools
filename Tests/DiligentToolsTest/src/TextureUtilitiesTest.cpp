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

#include "../interface/TextureUtilities.h"

#include <limits>

#include "gtest/gtest.h"

#include "GraphicsAccessories.hpp"

using namespace Diligent;

namespace
{

template <typename DataType>
void VerifyCopyPixelsData(const CopyPixelsAttribs& CopyAttribs, const DataType& TestData, const DataType& RefData)
{
    const auto NumComponents = CopyAttribs.DstCompCount;
    VERIFY_EXPR(CopyAttribs.DstStride % (CopyAttribs.DstComponentSize * CopyAttribs.DstCompCount) == 0);
    const auto StrideInPixels = CopyAttribs.DstStride / (CopyAttribs.DstComponentSize * CopyAttribs.DstCompCount);
    for (Uint32 y = 0; y < CopyAttribs.Height; ++y)
    {
        for (Uint32 x = 0; x < CopyAttribs.Width; ++x)
        {
            for (Uint32 c = 0; c < NumComponents; ++c)
            {
                const auto src_y   = CopyAttribs.FlipVertically ? CopyAttribs.Height - y - 1 : y;
                const auto TestVal = TestData[(y * StrideInPixels + x) * NumComponents + c];
                const auto RefVal  = RefData[(src_y * StrideInPixels + x) * NumComponents + c];
                EXPECT_EQ(TestVal, RefVal);
            }
        }
    }
}

template <typename SrcType, typename DstType>
void TestComponentSizeChange()
{
    static constexpr SrcType SrcShift = 8 * (sizeof(SrcType) - 1);
    static constexpr SrcType DstShift = 8 * (sizeof(DstType) - 1);

    // clang-format off
    const std::vector<SrcType> SrcData =
    {
         1u << SrcShift,  2u << SrcShift,  3u << SrcShift,  4u << SrcShift,
         5u << SrcShift,  6u << SrcShift,  7u << SrcShift,  8u << SrcShift,
         9u << SrcShift, 10u << SrcShift, 11u << SrcShift, 12u << SrcShift,
        13u << SrcShift, 14u << SrcShift, 15u << SrcShift, 16u << SrcShift,
    };

    const std::vector<DstType> RefData =
    {
         1u << DstShift,  2u << DstShift,  3u << DstShift,  4u << DstShift,
         5u << DstShift,  6u << DstShift,  7u << DstShift,  8u << DstShift,
         9u << DstShift, 10u << DstShift, 11u << DstShift, 12u << DstShift,
        13u << DstShift, 14u << DstShift, 15u << DstShift, 16u << DstShift,
    };
    // clang-format on

    std::vector<DstType> TestData(SrcData.size());

    CopyPixelsAttribs CopyAttribs;
    CopyAttribs.Width            = 2;
    CopyAttribs.Height           = 4;
    CopyAttribs.SrcComponentSize = sizeof(SrcType);
    CopyAttribs.pSrcPixels       = SrcData.data();
    CopyAttribs.SrcStride        = 2 * 2 * sizeof(SrcType);
    CopyAttribs.SrcCompCount     = 2;
    CopyAttribs.pDstPixels       = TestData.data();
    CopyAttribs.DstComponentSize = sizeof(DstType);
    CopyAttribs.DstStride        = 2 * 2 * sizeof(DstType);
    CopyAttribs.DstCompCount     = 2;
    CopyPixels(CopyAttribs);

    VerifyCopyPixelsData(CopyAttribs, TestData, RefData);
}

template <typename DataType>
void TestCopyPixels()
{
    // clang-format off
    const std::vector<DataType> SrcData =
    {
         1,  2,  3,  4,
         5,  6,  7,  8,
         9, 10, 11, 12,
        13, 14, 15, 16,
    };
    // clang-format on

    constexpr auto MaxVal = std::numeric_limits<DataType>::max();

    // 1 : 1
    {
        std::vector<DataType> TestData(SrcData.size());

        CopyPixelsAttribs CopyAttribs;
        CopyAttribs.Width            = 4;
        CopyAttribs.Height           = 4;
        CopyAttribs.SrcComponentSize = sizeof(DataType);
        CopyAttribs.pSrcPixels       = SrcData.data();
        CopyAttribs.SrcStride        = 4 * sizeof(DataType);
        CopyAttribs.SrcCompCount     = 1;
        CopyAttribs.pDstPixels       = TestData.data();
        CopyAttribs.DstComponentSize = sizeof(DataType);
        CopyAttribs.DstStride        = 4 * sizeof(DataType);
        CopyAttribs.DstCompCount     = 1;
        CopyPixels(CopyAttribs);

        VerifyCopyPixelsData(CopyAttribs, TestData, SrcData);

        CopyAttribs.FlipVertically = true;
        CopyPixels(CopyAttribs);

        VerifyCopyPixelsData(CopyAttribs, TestData, SrcData);
    }

    // Different strides
    {
        // clang-format off
        const std::vector<DataType> RefData =
        {
             1,  2,  3,  4,  0,
             5,  6,  7,  8,  0,
             9, 10, 11, 12,  0,
            13, 14, 15, 16,  0,
        };
        // clang-format on

        std::vector<DataType> TestData(RefData.size());

        CopyPixelsAttribs CopyAttribs;
        CopyAttribs.Width            = 4;
        CopyAttribs.Height           = 4;
        CopyAttribs.SrcComponentSize = sizeof(DataType);
        CopyAttribs.pSrcPixels       = SrcData.data();
        CopyAttribs.SrcStride        = 4 * sizeof(DataType);
        CopyAttribs.SrcCompCount     = 1;
        CopyAttribs.pDstPixels       = TestData.data();
        CopyAttribs.DstComponentSize = sizeof(DataType);
        CopyAttribs.DstStride        = 5 * sizeof(DataType);
        CopyAttribs.DstCompCount     = 1;
        CopyPixels(CopyAttribs);

        VerifyCopyPixelsData(CopyAttribs, TestData, RefData);

        CopyAttribs.FlipVertically = true;
        CopyPixels(CopyAttribs);
        VerifyCopyPixelsData(CopyAttribs, TestData, RefData);
    }


    // R -> RG
    {
        // clang-format off
        const std::vector<DataType> RefData =
        {
             1, 0,   2, 0,   3, 0,   4, 0,  0, 0,
             5, 0,   6, 0,   7, 0,   8, 0,  0, 0,
             9, 0,  10, 0,  11, 0,  12, 0,  0, 0,
            13, 0,  14, 0,  15, 0,  16, 0,  0, 0,
        };
        // clang-format on

        std::vector<DataType> TestData(RefData.size());

        CopyPixelsAttribs CopyAttribs;
        CopyAttribs.Width            = 4;
        CopyAttribs.Height           = 4;
        CopyAttribs.SrcComponentSize = sizeof(DataType);
        CopyAttribs.pSrcPixels       = SrcData.data();
        CopyAttribs.SrcStride        = 4 * sizeof(DataType);
        CopyAttribs.SrcCompCount     = 1;
        CopyAttribs.pDstPixels       = TestData.data();
        CopyAttribs.DstComponentSize = sizeof(DataType);
        CopyAttribs.DstStride        = 10 * sizeof(DataType);
        CopyAttribs.DstCompCount     = 2;
        CopyPixels(CopyAttribs);

        VerifyCopyPixelsData(CopyAttribs, TestData, RefData);

        CopyAttribs.FlipVertically = true;
        CopyPixels(CopyAttribs);
        VerifyCopyPixelsData(CopyAttribs, TestData, RefData);
    }


    // RG -> R
    {
        // clang-format off
        const std::vector<DataType> RefData =
        {
             1,  3,  0,
             5,  7,  0,
             9, 11,  0,
            13, 15,  0,
        };
        // clang-format on

        std::vector<DataType> TestData(RefData.size());

        CopyPixelsAttribs CopyAttribs;
        CopyAttribs.Width            = 2;
        CopyAttribs.Height           = 4;
        CopyAttribs.SrcComponentSize = sizeof(DataType);
        CopyAttribs.pSrcPixels       = SrcData.data();
        CopyAttribs.SrcStride        = 4 * sizeof(DataType);
        CopyAttribs.SrcCompCount     = 2;
        CopyAttribs.pDstPixels       = TestData.data();
        CopyAttribs.DstComponentSize = sizeof(DataType);
        CopyAttribs.DstStride        = 3 * sizeof(DataType);
        CopyAttribs.DstCompCount     = 1;
        CopyPixels(CopyAttribs);

        VerifyCopyPixelsData(CopyAttribs, TestData, RefData);

        CopyAttribs.FlipVertically = true;
        CopyPixels(CopyAttribs);
        VerifyCopyPixelsData(CopyAttribs, TestData, RefData);
    }

    // RG -> RGBA
    {
        // clang-format off
        const std::vector<DataType> RefData =
        {
             1,  2, 0, MaxVal,   3,  4, 0, MaxVal,  0, 0, 0, 0,
             5,  6, 0, MaxVal,   7,  8, 0, MaxVal,  0, 0, 0, 0,
             9, 10, 0, MaxVal,  11, 12, 0, MaxVal,  0, 0, 0, 0,
            13, 14, 0, MaxVal,  15, 16, 0, MaxVal,  0, 0, 0, 0,
        };
        // clang-format on

        std::vector<DataType> TestData(RefData.size());

        CopyPixelsAttribs CopyAttribs;
        CopyAttribs.Width            = 2;
        CopyAttribs.Height           = 4;
        CopyAttribs.SrcComponentSize = sizeof(DataType);
        CopyAttribs.pSrcPixels       = SrcData.data();
        CopyAttribs.SrcStride        = 4 * sizeof(DataType);
        CopyAttribs.SrcCompCount     = 2;
        CopyAttribs.pDstPixels       = TestData.data();
        CopyAttribs.DstComponentSize = sizeof(DataType);
        CopyAttribs.DstStride        = 12 * sizeof(DataType);
        CopyAttribs.DstCompCount     = 4;
        CopyAttribs.Swizzle.A        = TEXTURE_COMPONENT_SWIZZLE_ONE;
        CopyPixels(CopyAttribs);

        VerifyCopyPixelsData(CopyAttribs, TestData, RefData);

        CopyAttribs.FlipVertically = true;
        CopyPixels(CopyAttribs);
        VerifyCopyPixelsData(CopyAttribs, TestData, RefData);
    }


    // RGB -> RGBA
    {
        // clang-format off
        const std::vector<DataType> RefData =
        {
             1,  2,  3,  MaxVal,  0, 0, 0, 0,
             5,  6,  7,  MaxVal,  0, 0, 0, 0,
             9, 10, 11,  MaxVal,  0, 0, 0, 0,
            13, 14, 15,  MaxVal,  0, 0, 0, 0,
        };
        // clang-format on

        std::vector<DataType> TestData(RefData.size());

        CopyPixelsAttribs CopyAttribs;
        CopyAttribs.Width            = 1;
        CopyAttribs.Height           = 4;
        CopyAttribs.SrcComponentSize = sizeof(DataType);
        CopyAttribs.pSrcPixels       = SrcData.data();
        CopyAttribs.SrcStride        = 4 * sizeof(DataType);
        CopyAttribs.SrcCompCount     = 3;
        CopyAttribs.pDstPixels       = TestData.data();
        CopyAttribs.DstComponentSize = sizeof(DataType);
        CopyAttribs.DstStride        = 8 * sizeof(DataType);
        CopyAttribs.DstCompCount     = 4;
        CopyAttribs.Swizzle.A        = TEXTURE_COMPONENT_SWIZZLE_ONE;
        CopyPixels(CopyAttribs);

        VerifyCopyPixelsData(CopyAttribs, TestData, RefData);

        CopyAttribs.FlipVertically = true;
        CopyPixels(CopyAttribs);
        VerifyCopyPixelsData(CopyAttribs, TestData, RefData);
    }

    // Swizzle
    for (Uint32 Comp = 0; Comp < 4; ++Comp)
    {
        for (Uint32 Swizzle = 0; Swizzle < TEXTURE_COMPONENT_SWIZZLE_COUNT; ++Swizzle)
        {
            std::vector<DataType> RefData = SrcData;
            for (Uint32 row = 0; row < 4; ++row)
            {
                DataType& Val = RefData[row * 4 + Comp];
                switch (Swizzle)
                {
                    // clang-format off
                    case TEXTURE_COMPONENT_SWIZZLE_IDENTITY:                             break;
                    case TEXTURE_COMPONENT_SWIZZLE_ZERO:     Val = 0;                    break;
                    case TEXTURE_COMPONENT_SWIZZLE_ONE:      Val = MaxVal;               break;
                    case TEXTURE_COMPONENT_SWIZZLE_R:        Val = SrcData[row * 4 + 0]; break;
                    case TEXTURE_COMPONENT_SWIZZLE_G:        Val = SrcData[row * 4 + 1]; break;
                    case TEXTURE_COMPONENT_SWIZZLE_B:        Val = SrcData[row * 4 + 2]; break;
                    case TEXTURE_COMPONENT_SWIZZLE_A:        Val = SrcData[row * 4 + 3]; break;
                    // clang-format on
                    default:
                        UNEXPECTED("Unexpected swizzle");
                }
            }

            std::vector<DataType> TestData(SrcData.size());

            CopyPixelsAttribs CopyAttribs;
            CopyAttribs.Width            = 1;
            CopyAttribs.Height           = 4;
            CopyAttribs.SrcComponentSize = sizeof(DataType);
            CopyAttribs.pSrcPixels       = SrcData.data();
            CopyAttribs.SrcStride        = 4 * sizeof(DataType);
            CopyAttribs.SrcCompCount     = 4;
            CopyAttribs.pDstPixels       = TestData.data();
            CopyAttribs.DstComponentSize = sizeof(DataType);
            CopyAttribs.DstStride        = 4 * sizeof(DataType);
            CopyAttribs.DstCompCount     = 4;
            CopyAttribs.Swizzle[Comp]    = static_cast<TEXTURE_COMPONENT_SWIZZLE>(Swizzle);
            CopyPixels(CopyAttribs);

            VerifyCopyPixelsData(CopyAttribs, TestData, RefData);
        }
    }

    TestComponentSizeChange<DataType, Uint8>();
    TestComponentSizeChange<DataType, Uint16>();
    TestComponentSizeChange<DataType, Uint32>();
}

TEST(Tools_TextureUtilities, CopyPixels8)
{
    TestCopyPixels<Uint8>();
}

TEST(Tools_TextureUtilities, CopyPixels16)
{
    TestCopyPixels<Uint16>();
}

TEST(Tools_TextureUtilities, CopyPixels32)
{
    TestCopyPixels<Uint32>();
}



template <typename DataType>
void VerifyExpandPixelsData(const ExpandPixelsAttribs& Attribs, const DataType& TestData, const DataType& RefData)
{
    const auto NumComponents = Attribs.ComponentCount;
    VERIFY_EXPR(Attribs.DstStride % (Attribs.ComponentSize * Attribs.ComponentCount) == 0);
    const auto StrideInPixels = Attribs.DstStride / (Attribs.ComponentSize * Attribs.ComponentCount);
    for (Uint32 y = 0; y < Attribs.DstHeight; ++y)
    {
        for (Uint32 x = 0; x < Attribs.DstWidth; ++x)
        {
            for (Uint32 c = 0; c < NumComponents; ++c)
            {
                const auto TestVal = TestData[(y * StrideInPixels + x) * NumComponents + c];
                const auto RefVal  = RefData[(y * StrideInPixels + x) * NumComponents + c];
                EXPECT_EQ(TestVal, RefVal);
            }
        }
    }
}

template <typename DataType>
void TestExpandPixels()
{
    // clang-format off
    const std::vector<DataType> SrcData =
    {
         1,  2,  3,  4,
         5,  6,  7,  8,
         9, 10, 11, 12,
        13, 14, 15, 16,
    };
    // clang-format on

    // Row only
    {

        // clang-format off
        const std::vector<DataType> RefData =
            {
                1, 2, 3, 4,   4, 4, 4
            };
        // clang-format on

        std::vector<DataType> TestData(RefData.size());

        ExpandPixelsAttribs ExpandAttribs;
        ExpandAttribs.SrcWidth       = 4;
        ExpandAttribs.SrcHeight      = 1;
        ExpandAttribs.ComponentSize  = sizeof(DataType);
        ExpandAttribs.ComponentCount = 1;
        ExpandAttribs.pSrcPixels     = SrcData.data();
        ExpandAttribs.SrcStride      = 0;
        ExpandAttribs.DstWidth       = 7;
        ExpandAttribs.DstHeight      = 1;
        ExpandAttribs.pDstPixels     = TestData.data();
        ExpandAttribs.DstStride      = 0;
        ExpandPixels(ExpandAttribs);

        VerifyExpandPixelsData(ExpandAttribs, TestData, RefData);
    }

    // Two rows
    {

        // clang-format off
        const std::vector<DataType> RefData =
            {
                1, 2, 3, 4,   3, 4, 3, 4,
                5, 6, 7, 8,   7, 8, 7, 8,
            };
        // clang-format on

        std::vector<DataType> TestData(RefData.size());

        ExpandPixelsAttribs ExpandAttribs;
        ExpandAttribs.SrcWidth       = 2;
        ExpandAttribs.SrcHeight      = 2;
        ExpandAttribs.ComponentSize  = sizeof(DataType);
        ExpandAttribs.ComponentCount = 2;
        ExpandAttribs.pSrcPixels     = SrcData.data();
        ExpandAttribs.SrcStride      = 4 * sizeof(DataType);
        ExpandAttribs.DstWidth       = 4;
        ExpandAttribs.DstHeight      = 2;
        ExpandAttribs.pDstPixels     = TestData.data();
        ExpandAttribs.DstStride      = 8 * sizeof(DataType);
        ExpandPixels(ExpandAttribs);

        VerifyExpandPixelsData(ExpandAttribs, TestData, RefData);
    }

    // Column only
    {

        // clang-format off
        const std::vector<DataType> RefData =
            {
                1, 5, 9, 13,   13, 13, 13
            };
        // clang-format on

        std::vector<DataType> TestData(RefData.size());

        ExpandPixelsAttribs ExpandAttribs;
        ExpandAttribs.SrcWidth       = 1;
        ExpandAttribs.SrcHeight      = 4;
        ExpandAttribs.ComponentSize  = sizeof(DataType);
        ExpandAttribs.ComponentCount = 1;
        ExpandAttribs.pSrcPixels     = SrcData.data();
        ExpandAttribs.SrcStride      = 4 * sizeof(DataType);
        ExpandAttribs.DstWidth       = 1;
        ExpandAttribs.DstHeight      = 7;
        ExpandAttribs.pDstPixels     = TestData.data();
        ExpandAttribs.DstStride      = 1 * sizeof(DataType);
        ExpandPixels(ExpandAttribs);

        VerifyExpandPixelsData(ExpandAttribs, TestData, RefData);
    }

    // 2x3 -> 4x5
    {

        // clang-format off
        const std::vector<DataType> RefData =
            {
                1,  2,  3,  4,    3,  4,  3,  4,
                5,  6,  7,  8,    7,  8,  7,  8,
                9, 10, 11, 12,   11, 12, 11, 12,
                9, 10, 11, 12,   11, 12, 11, 12,
                9, 10, 11, 12,   11, 12, 11, 12,
            };
        // clang-format on

        std::vector<DataType> TestData(RefData.size());

        ExpandPixelsAttribs ExpandAttribs;
        ExpandAttribs.SrcWidth       = 2;
        ExpandAttribs.SrcHeight      = 3;
        ExpandAttribs.ComponentSize  = sizeof(DataType);
        ExpandAttribs.ComponentCount = 2;
        ExpandAttribs.pSrcPixels     = SrcData.data();
        ExpandAttribs.SrcStride      = 4 * sizeof(DataType);
        ExpandAttribs.DstWidth       = 4;
        ExpandAttribs.DstHeight      = 5;
        ExpandAttribs.pDstPixels     = TestData.data();
        ExpandAttribs.DstStride      = 8 * sizeof(DataType);
        ExpandPixels(ExpandAttribs);

        VerifyExpandPixelsData(ExpandAttribs, TestData, RefData);
    }
}

TEST(Tools_TextureUtilities, ExpandPixels8)
{
    TestExpandPixels<Uint8>();
}

TEST(Tools_TextureUtilities, ExpandPixels16)
{
    TestExpandPixels<Uint16>();
}

TEST(Tools_TextureUtilities, ExpandPixels32)
{
    TestExpandPixels<Uint32>();
}


template <typename DataType>
void VerifyPremultiplyAlphaData(const PremultiplyAlphaAttribs& Attribs, const DataType& TestData, const DataType& RefData)
{
    auto       ComponentSize = GetValueSize(Attribs.ComponentType);
    const auto NumComponents = Attribs.ComponentCount;
    VERIFY_EXPR(Attribs.Stride % (ComponentSize * Attribs.ComponentCount) == 0);
    const auto StrideInPixels = Attribs.Stride / (ComponentSize * Attribs.ComponentCount);
    for (Uint32 y = 0; y < Attribs.Height; ++y)
    {
        for (Uint32 x = 0; x < Attribs.Width; ++x)
        {
            for (Uint32 c = 0; c < NumComponents; ++c)
            {
                const auto TestVal = TestData[(y * StrideInPixels + x) * NumComponents + c];
                const auto RefVal  = RefData[(y * StrideInPixels + x) * NumComponents + c];
                EXPECT_EQ(TestVal, RefVal) << " row=" << y << " col=" << x << " c=" << c;
            }
        }
    }
}

template <typename DataType>
void TestPremultiplyAlpha(VALUE_TYPE ComponentType)
{
    constexpr auto MaxVal = std::numeric_limits<DataType>::max();

    // clang-format off
    const std::vector<DataType> SrcData =
    {
        1,  2,  3,           0,    3,  4, MaxVal,          0,
        5,  6,  7,  MaxVal / 2,    7,  8, MaxVal, MaxVal / 2,
        9, 10, 11,  MaxVal / 1,   11, 12, MaxVal, MaxVal / 1,
        9, 10, 11,  MaxVal / 4,   11, 12, MaxVal, MaxVal / 4,
    };

    const std::vector<DataType> RefData =
    {
        0,  0,  0,           0,    0,  0,          0,          0,
        2,  3,  3,  MaxVal / 2,    3,  4, MaxVal / 2, MaxVal / 2,
        9, 10, 11,  MaxVal / 1,   11, 12, MaxVal / 1, MaxVal / 1,
        2,  2,  3,  MaxVal / 4,    3,  3, MaxVal / 4, MaxVal / 4,    
    };
    // clang-format on

    {
        std::vector<DataType> TestData = SrcData;

        PremultiplyAlphaAttribs Attribs;
        Attribs.Width          = 2;
        Attribs.Height         = 4;
        Attribs.ComponentType  = ComponentType;
        Attribs.ComponentCount = 4;
        Attribs.Stride         = 8 * sizeof(DataType);
        Attribs.pPixels        = TestData.data();
        PremultiplyAlpha(Attribs);

        VerifyPremultiplyAlphaData(Attribs, TestData, RefData);

        TestData       = SrcData;
        Attribs.IsSRGB = true;
        PremultiplyAlpha(Attribs);
    }
}


template <>
void TestPremultiplyAlpha<float>(VALUE_TYPE ComponentType)
{
    // clang-format off
    const std::vector<float> SrcData =
    {
        0.125,  0.25, 0.375,   0.0,    0.5,  0.75,  1.0,  0.0,
        0.125,  0.25, 0.375,   0.25,   0.5,  0.75,  1.0,  0.25,
        0.125,  0.25, 0.375,   0.5,    0.5,  0.75,  1.0,  0.5,
        0.125,  0.25, 0.375,   1.0,    0.5,  0.75,  1.0,  1.0,
    };

    const std::vector<float> RefData =
    {
                 0.0,         0.0,         0.0,    0.0,           0.0,          0.0,         0.0,  0.0,
        0.125 * 0.25,  0.25* 0.25, 0.375* 0.25,   0.25,    0.5 * 0.25,  0.75 * 0.25,  1.0 * 0.25,  0.25,
        0.125 * 0.5,   0.25 * 0.5, 0.375 * 0.5,    0.5,    0.5 * 0.5,   0.75 * 0.5,   1.0 * 0.5,   0.5,
        0.125,  0.25,                    0.375,    1.0,           0.5,        0.75,         1.0,   1.0,
    };
    // clang-format on

    {
        std::vector<float> TestData = SrcData;

        PremultiplyAlphaAttribs Attribs;
        Attribs.Width          = 2;
        Attribs.Height         = 4;
        Attribs.ComponentType  = ComponentType;
        Attribs.ComponentCount = 4;
        Attribs.Stride         = 8 * sizeof(float);
        Attribs.pPixels        = TestData.data();
        PremultiplyAlpha(Attribs);

        VerifyPremultiplyAlphaData(Attribs, TestData, RefData);

        TestData       = SrcData;
        Attribs.IsSRGB = true;
        PremultiplyAlpha(Attribs);
    }
}

TEST(Tools_TextureUtilities, PremultiplyAlpha8)
{
    TestPremultiplyAlpha<Uint8>(VT_UINT8);
}

TEST(Tools_TextureUtilities, PremultiplyAlpha16)
{
    TestPremultiplyAlpha<Uint16>(VT_UINT16);
}

TEST(Tools_TextureUtilities, PremultiplyAlpha32)
{
    TestPremultiplyAlpha<Uint32>(VT_UINT32);
}

TEST(Tools_TextureUtilities, PremultiplyAlphaFloat)
{
    TestPremultiplyAlpha<float>(VT_FLOAT32);
}

} // namespace
