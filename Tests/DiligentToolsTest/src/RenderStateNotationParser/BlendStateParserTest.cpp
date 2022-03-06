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

TEST(Tools_RenderStateNotationParser, ParseBlendStateEnums)
{
    DynamicLinearAllocator Allocator{DefaultRawMemoryAllocator::GetAllocator()};

    ASSERT_TRUE(TestEnum<BLEND_FACTOR>(Allocator, BLEND_FACTOR_UNDEFINED, BLEND_FACTOR_NUM_FACTORS));

    ASSERT_TRUE(TestEnum<BLEND_OPERATION>(Allocator, BLEND_OPERATION_UNDEFINED, BLEND_OPERATION_NUM_OPERATIONS));

    ASSERT_TRUE(TestEnum<LOGIC_OPERATION>(Allocator, LOGIC_OP_CLEAR, LOGIC_OP_NUM_OPERATIONS));

    ASSERT_TRUE(TestBitwiseEnum<COLOR_MASK>(Allocator, COLOR_MASK_ALPHA));
}

TEST(Tools_RenderStateNotationParser, ParseRenderTargetBlendDesc)
{
    CHECK_STRUCT_SIZE(RenderTargetBlendDesc, 10);

    DynamicLinearAllocator Allocator{DefaultRawMemoryAllocator::GetAllocator()};

    nlohmann::json JsonReference = LoadDRSNFromFile("RenderStates/BlendState/RenderTargetBlendDesc.json");

    RenderTargetBlendDesc DescReference{};
    DescReference.DestBlend = BLEND_FACTOR_INV_DEST_ALPHA;
    DescReference.LogicOp   = LOGIC_OP_AND_REVERSE;

    RenderTargetBlendDesc Desc{};
    ParseRSN(JsonReference, Desc, Allocator);
    ASSERT_EQ(Desc, DescReference);
}

TEST(Tools_RenderStateNotationParser, ParseBlendStateDesc)
{
    CHECK_STRUCT_SIZE(BlendStateDesc, 82);

    DynamicLinearAllocator Allocator{DefaultRawMemoryAllocator::GetAllocator()};

    nlohmann::json JsonReference = LoadDRSNFromFile("RenderStates/BlendState/BlendStateDesc.json");

    BlendStateDesc DescReference{};
    DescReference.AlphaToCoverageEnable                  = true;
    DescReference.IndependentBlendEnable                 = true;
    DescReference.RenderTargets[0].DestBlend             = BLEND_FACTOR_INV_DEST_ALPHA;
    DescReference.RenderTargets[0].LogicOp               = LOGIC_OP_AND_REVERSE;
    DescReference.RenderTargets[1].BlendEnable           = true;
    DescReference.RenderTargets[1].SrcBlend              = BLEND_FACTOR_DEST_ALPHA;
    DescReference.RenderTargets[2].RenderTargetWriteMask = COLOR_MASK_RED;

    BlendStateDesc Desc = {};
    ParseRSN(JsonReference, Desc, Allocator);
    ASSERT_EQ(Desc, DescReference);
}

} // namespace
