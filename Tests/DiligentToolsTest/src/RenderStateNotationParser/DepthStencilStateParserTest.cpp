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

TEST(Tools_RenderStateNotationParser, ParseDepthStencilStateEnums)
{
    DynamicLinearAllocator Allocator{DefaultRawMemoryAllocator::GetAllocator()};

    ASSERT_TRUE(TestEnum<STENCIL_OP>(Allocator, STENCIL_OP_UNDEFINED, STENCIL_OP_NUM_OPS));
}

TEST(Tools_RenderStateNotationParser, ParserStencilOpDesc)
{
    CHECK_STRUCT_SIZE(StencilOpDesc, 4);

    DynamicLinearAllocator Allocator{DefaultRawMemoryAllocator::GetAllocator()};

    nlohmann::json JsonReference = LoadDRSNFromFile("RenderStates/DepthStencilState/StencilOpDesc.json");

    StencilOpDesc DescReference{};
    DescReference.StencilFailOp      = STENCIL_OP_ZERO;
    DescReference.StencilDepthFailOp = STENCIL_OP_DECR_WRAP;
    DescReference.StencilPassOp      = STENCIL_OP_INCR_SAT;
    DescReference.StencilFunc        = COMPARISON_FUNC_LESS_EQUAL;

    StencilOpDesc Desc = {};
    ParseRSN(JsonReference, Desc, Allocator);
    ASSERT_EQ(Desc, DescReference);
}

TEST(Tools_RenderStateNotationParser, ParseDepthStencilStateDesc)
{
    CHECK_STRUCT_SIZE(DepthStencilStateDesc, 14);

    DynamicLinearAllocator Allocator{DefaultRawMemoryAllocator::GetAllocator()};

    nlohmann::json JsonReference = LoadDRSNFromFile("RenderStates/DepthStencilState/DepthStencilStateDesc.json");

    DepthStencilStateDesc DescReference{};
    DescReference.DepthEnable           = false;
    DescReference.DepthWriteEnable      = false;
    DescReference.DepthFunc             = COMPARISON_FUNC_GREATER;
    DescReference.StencilEnable         = true;
    DescReference.StencilReadMask       = 0x0F;
    DescReference.StencilWriteMask      = 0x07;
    DescReference.FrontFace.StencilFunc = COMPARISON_FUNC_NEVER;
    DescReference.BackFace.StencilFunc  = COMPARISON_FUNC_NOT_EQUAL;

    DepthStencilStateDesc Desc{};
    ParseRSN(JsonReference, Desc, Allocator);
    ASSERT_EQ(Desc, DescReference);
}

} // namespace
