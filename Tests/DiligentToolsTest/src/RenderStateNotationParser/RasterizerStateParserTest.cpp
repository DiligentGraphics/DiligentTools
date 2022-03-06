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

TEST(Tools_RenderStateNotationParser, ParseRasterizerStateEnums)
{
    DynamicLinearAllocator Allocator{DefaultRawMemoryAllocator::GetAllocator()};

    ASSERT_TRUE(TestEnum<FILL_MODE>(Allocator, FILL_MODE_UNDEFINED, FILL_MODE_NUM_MODES));

    ASSERT_TRUE(TestEnum<CULL_MODE>(Allocator, CULL_MODE_UNDEFINED, CULL_MODE_NUM_MODES));
}

TEST(Tools_RenderStateNotationParser, ParseRasterizerStateDesc)
{
    CHECK_STRUCT_SIZE(RasterizerStateDesc, 20);

    DynamicLinearAllocator Allocator{DefaultRawMemoryAllocator::GetAllocator()};

    nlohmann::json JsonReference = LoadDRSNFromFile("RenderStates/RasterizerState/RasterizerStateDesc.json");

    RasterizerStateDesc DescReference{};
    DescReference.FillMode              = FILL_MODE_WIREFRAME;
    DescReference.CullMode              = CULL_MODE_FRONT;
    DescReference.FrontCounterClockwise = true;
    DescReference.DepthClipEnable       = true;
    DescReference.ScissorEnable         = true;
    DescReference.AntialiasedLineEnable = true;
    DescReference.DepthBias             = 1;
    DescReference.DepthBiasClamp        = 0.25f;
    DescReference.SlopeScaledDepthBias  = 0.75f;

    RasterizerStateDesc Desc{};
    ParseRSN(JsonReference, Desc, Allocator);
    ASSERT_EQ(Desc, DescReference);
}

} // namespace
