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

TEST(Tools_RenderStateNotationParser, ParseSamplerEnums)
{
    DynamicLinearAllocator Allocator{DefaultRawMemoryAllocator::GetAllocator()};

    ASSERT_TRUE(TestBitwiseEnum<SAMPLER_FLAGS>(Allocator, SAMPLER_FLAG_LAST));
}

TEST(Tools_RenderStateNotationParser, ParseSamplerDesc)
{
    CHECK_STRUCT_SIZE(SamplerDesc, 56);

    DynamicLinearAllocator Allocator{DefaultRawMemoryAllocator::GetAllocator()};

    nlohmann::json JsonReference = LoadDRSNFromFile("RenderStates/Sampler/SamplerDesc.json");

    SamplerDesc DescReference{};
    DescReference.Name      = "TestName";
    DescReference.MinFilter = FILTER_TYPE_ANISOTROPIC;
    DescReference.MagFilter = FILTER_TYPE_MAXIMUM_POINT;
    DescReference.MipFilter = FILTER_TYPE_POINT;

    DescReference.AddressU = TEXTURE_ADDRESS_MIRROR;
    DescReference.AddressV = TEXTURE_ADDRESS_MIRROR_ONCE;
    DescReference.AddressW = TEXTURE_ADDRESS_BORDER;

    DescReference.ComparisonFunc     = COMPARISON_FUNC_GREATER;
    DescReference.Flags              = SAMPLER_FLAG_SUBSAMPLED | SAMPLER_FLAG_SUBSAMPLED_COARSE_RECONSTRUCTION;
    DescReference.UnnormalizedCoords = true;

    DescReference.BorderColor[0] = 0.125f;
    DescReference.BorderColor[1] = 0.5f;
    DescReference.BorderColor[2] = 0.75f;
    DescReference.BorderColor[3] = 1.0f;

    DescReference.MipLODBias    = 1.5f;
    DescReference.MinLOD        = 1.0f;
    DescReference.MaxLOD        = 4.0f;
    DescReference.MaxAnisotropy = 16;

    SamplerDesc Desc{};
    ParseRSN(JsonReference, Desc, Allocator);
    ASSERT_EQ(Desc, DescReference);
}

} // namespace
