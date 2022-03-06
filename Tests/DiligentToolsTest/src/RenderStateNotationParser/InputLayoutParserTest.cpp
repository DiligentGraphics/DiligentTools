/*
 *  Copyright 2019-2022 Diligent Graphics LLC
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

#include "gtest/gtest.h"
#include "DRSNLoader.hpp"
#include "GraphicsTypesOutputInserters.hpp"

using namespace Diligent;

namespace
{

TEST(Tools_RenderStateNotationParser, ParseInputLayoutEnums)
{
    DynamicLinearAllocator Allocator{DefaultRawMemoryAllocator::GetAllocator()};

    ASSERT_TRUE(TestEnum<INPUT_ELEMENT_FREQUENCY>(Allocator, INPUT_ELEMENT_FREQUENCY_UNDEFINED, INPUT_ELEMENT_FREQUENCY_NUM_FREQUENCIES));
}

TEST(Tools_RenderStateNotationParser, ParseLayoutElement)
{
    CHECK_STRUCT_SIZE(LayoutElement, 40);

    DynamicLinearAllocator Allocator{DefaultRawMemoryAllocator::GetAllocator()};

    nlohmann::json JsonReference = LoadDRSNFromFile("RenderStates/InputLayout/LayoutElement.json");

    LayoutElement DescReference{};
    DescReference.InputIndex           = 1;
    DescReference.BufferSlot           = 1;
    DescReference.NumComponents        = 3;
    DescReference.ValueType            = VT_FLOAT32;
    DescReference.IsNormalized         = false;
    DescReference.RelativeOffset       = 16;
    DescReference.Stride               = 8;
    DescReference.InstanceDataStepRate = 12;
    DescReference.Frequency            = INPUT_ELEMENT_FREQUENCY_PER_INSTANCE;
    DescReference.HLSLSemantic         = "TestSemantic0";

    LayoutElement Desc{};
    ParseRSN(JsonReference, Desc, Allocator);
    ASSERT_EQ(Desc, DescReference);
}

TEST(Tools_RenderStateNotationParser, ParseInputLayoutDesc)
{
    CHECK_STRUCT_SIZE(InputLayoutDesc, 16);

    DynamicLinearAllocator Allocator{DefaultRawMemoryAllocator::GetAllocator()};

    nlohmann::json JsonReference = LoadDRSNFromFile("RenderStates/InputLayout/InputLayoutDesc.json");

    constexpr LayoutElement LayoutElements[] = {
        LayoutElement{0, 0, 3, VT_FLOAT32, False},
        LayoutElement{1, 1, 4, VT_FLOAT32, False},
        LayoutElement{2, 2, 3, VT_FLOAT16},
    };

    InputLayoutDesc DescReference{};
    DescReference.LayoutElements = LayoutElements;
    DescReference.NumElements    = _countof(LayoutElements);

    InputLayoutDesc Desc{};
    ParseRSN(JsonReference, Desc, Allocator);
    ASSERT_EQ(Desc, DescReference);
}

} // namespace
