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

#include "../interface/TextureUtilities.h"

#include <limits>

#include "gtest/gtest.h"

using namespace Diligent;

namespace
{

template <typename DataType>
void VerifyData(const CopyPixelsAttribs& CopyAttribs, const DataType& TestData, const DataType& RefData)
{
    const auto NumComponents = CopyAttribs.DstCompCount;
    VERIFY_EXPR(CopyAttribs.DstStride % (CopyAttribs.ComponentSize * CopyAttribs.DstCompCount) == 0);
    const auto StrideInPixels = CopyAttribs.DstStride / (CopyAttribs.ComponentSize * CopyAttribs.DstCompCount);
    for (Uint32 y = 0; y < CopyAttribs.Height; ++y)
    {
        for (Uint32 x = 0; x < CopyAttribs.Width; ++x)
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


    // 1 : 1
    {
        std::vector<DataType> TestData(SrcData.size());

        CopyPixelsAttribs CopyAttribs;
        CopyAttribs.Width         = 4;
        CopyAttribs.Height        = 4;
        CopyAttribs.ComponentSize = sizeof(DataType);
        CopyAttribs.pSrcPixels    = SrcData.data();
        CopyAttribs.SrcStride     = 4 * sizeof(DataType);
        CopyAttribs.SrcCompCount  = 1;
        CopyAttribs.pDstPixels    = TestData.data();
        CopyAttribs.DstStride     = 4 * sizeof(DataType);
        CopyAttribs.DstCompCount  = 1;
        CopyPixels(CopyAttribs);

        VerifyData(CopyAttribs, TestData, SrcData);
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

        std::vector<DataType> TestData(RefData.size());

        CopyPixelsAttribs CopyAttribs;
        CopyAttribs.Width         = 4;
        CopyAttribs.Height        = 4;
        CopyAttribs.ComponentSize = sizeof(DataType);
        CopyAttribs.pSrcPixels    = SrcData.data();
        CopyAttribs.SrcStride     = 4 * sizeof(DataType);
        CopyAttribs.SrcCompCount  = 1;
        CopyAttribs.pDstPixels    = TestData.data();
        CopyAttribs.DstStride     = 5 * sizeof(DataType);
        CopyAttribs.DstCompCount  = 1;
        CopyPixels(CopyAttribs);

        // clang-format on
        VerifyData(CopyAttribs, TestData, RefData);
    }


    // R -> RG
    {
        // clang-format off
        const std::vector<DataType> RefData =
        {
             1,  1,   2,  2,   3,  3,   4,  4,  0, 0,
             5,  5,   6,  6,   7,  7,   8,  8,  0, 0,
             9,  9,  10, 10,  11, 11,  12, 12,  0, 0,
            13, 13,  14, 14,  15, 15,  16, 16,  0, 0,
        };

        std::vector<DataType> TestData(RefData.size());

        CopyPixelsAttribs CopyAttribs;
        CopyAttribs.Width         = 4;
        CopyAttribs.Height        = 4;
        CopyAttribs.ComponentSize = sizeof(DataType);
        CopyAttribs.pSrcPixels    = SrcData.data();
        CopyAttribs.SrcStride     = 4 * sizeof(DataType);
        CopyAttribs.SrcCompCount  = 1;
        CopyAttribs.pDstPixels    = TestData.data();
        CopyAttribs.DstStride     = 10 * sizeof(DataType);
        CopyAttribs.DstCompCount  = 2;
        CopyPixels(CopyAttribs);

        // clang-format on
        VerifyData(CopyAttribs, TestData, RefData);
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

        std::vector<DataType> TestData(RefData.size());

        CopyPixelsAttribs CopyAttribs;
        CopyAttribs.Width         = 2;
        CopyAttribs.Height        = 4;
        CopyAttribs.ComponentSize = sizeof(DataType);
        CopyAttribs.pSrcPixels    = SrcData.data();
        CopyAttribs.SrcStride     = 4 * sizeof(DataType);
        CopyAttribs.SrcCompCount  = 2;
        CopyAttribs.pDstPixels    = TestData.data();
        CopyAttribs.DstStride     = 3 * sizeof(DataType);
        CopyAttribs.DstCompCount  = 1;
        CopyPixels(CopyAttribs);

        // clang-format on
        VerifyData(CopyAttribs, TestData, RefData);
    }

    // RG -> RGBA
    {
        constexpr auto MaxVal = std::numeric_limits<DataType>::max();
        // clang-format off
        const std::vector<DataType> RefData =
        {
             1,  2, 0, MaxVal,   3,  4, 0, MaxVal,  0, 0, 0, 0,
             5,  6, 0, MaxVal,   7,  8, 0, MaxVal,  0, 0, 0, 0,
             9, 10, 0, MaxVal,  11, 12, 0, MaxVal,  0, 0, 0, 0,
            13, 14, 0, MaxVal,  15, 16, 0, MaxVal,  0, 0, 0, 0,
        };

        std::vector<DataType> TestData(RefData.size());

        CopyPixelsAttribs CopyAttribs;
        CopyAttribs.Width         = 2;
        CopyAttribs.Height        = 4;
        CopyAttribs.ComponentSize = sizeof(DataType);
        CopyAttribs.pSrcPixels    = SrcData.data();
        CopyAttribs.SrcStride     = 4 * sizeof(DataType);
        CopyAttribs.SrcCompCount  = 2;
        CopyAttribs.pDstPixels    = TestData.data();
        CopyAttribs.DstStride     = 12 * sizeof(DataType);
        CopyAttribs.DstCompCount  = 4;
        CopyPixels(CopyAttribs);

        // clang-format on
        VerifyData(CopyAttribs, TestData, RefData);
    }


    // RGB -> RGBA
    {
        constexpr auto MaxVal = std::numeric_limits<DataType>::max();
        // clang-format off
        const std::vector<DataType> RefData =
        {
             1,  2,  3,  MaxVal,  0, 0, 0, 0,
             5,  6,  7,  MaxVal,  0, 0, 0, 0,
             9, 10, 11,  MaxVal,  0, 0, 0, 0,
            13, 14, 15,  MaxVal,  0, 0, 0, 0,
        };

        std::vector<DataType> TestData(RefData.size());

        CopyPixelsAttribs CopyAttribs;
        CopyAttribs.Width         = 1;
        CopyAttribs.Height        = 4;
        CopyAttribs.ComponentSize = sizeof(DataType);
        CopyAttribs.pSrcPixels    = SrcData.data();
        CopyAttribs.SrcStride     = 4 * sizeof(DataType);
        CopyAttribs.SrcCompCount  = 3;
        CopyAttribs.pDstPixels    = TestData.data();
        CopyAttribs.DstStride     = 8 * sizeof(DataType);
        CopyAttribs.DstCompCount  = 4;
        CopyPixels(CopyAttribs);

        // clang-format on
        VerifyData(CopyAttribs, TestData, RefData);
    }
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

} // namespace
