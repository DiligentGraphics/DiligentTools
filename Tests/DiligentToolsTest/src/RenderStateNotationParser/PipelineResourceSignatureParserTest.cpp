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

TEST(Tools_RenderStateNotationParser, ParsePipelineResourceSignatureEnums)
{
    DynamicLinearAllocator Allocator{DefaultRawMemoryAllocator::GetAllocator()};

    ASSERT_TRUE(TestBitwiseEnum<PIPELINE_RESOURCE_FLAGS>(Allocator, PIPELINE_RESOURCE_FLAG_LAST));
}

TEST(Tools_RenderStateNotationParser, ParsePipelineResourceDesc)
{
    CHECK_STRUCT_SIZE(PipelineResourceDesc, 24);

    DynamicLinearAllocator Allocator{DefaultRawMemoryAllocator::GetAllocator()};

    nlohmann::json JsonReference = LoadDRSNFromFile("RenderStates/PipelineResourceSignature/PipelineResourceDesc.json");

    PipelineResourceDesc DescReference{};
    DescReference.Name         = "TestName";
    DescReference.ShaderStages = SHADER_TYPE_VERTEX | SHADER_TYPE_PIXEL;
    DescReference.VarType      = SHADER_RESOURCE_VARIABLE_TYPE_DYNAMIC;
    DescReference.ResourceType = SHADER_RESOURCE_TYPE_CONSTANT_BUFFER;
    DescReference.ArraySize    = 16;
    DescReference.Flags        = PIPELINE_RESOURCE_FLAG_RUNTIME_ARRAY;

    PipelineResourceDesc Desc{};
    ParseRSN(JsonReference, Desc, Allocator);
    ASSERT_EQ(Desc, DescReference);
}

TEST(Tools_RenderStateNotationParser, ParseImmutableSamplerDesc)
{
    CHECK_STRUCT_SIZE(ImmutableSamplerDesc, 72);

    DynamicLinearAllocator Allocator{DefaultRawMemoryAllocator::GetAllocator()};

    nlohmann::json JsonReference = LoadDRSNFromFile("RenderStates/PipelineResourceSignature/ImmutableSamplerDesc.json");

    ImmutableSamplerDesc DescReference{};
    DescReference.ShaderStages         = SHADER_TYPE_VERTEX | SHADER_TYPE_PIXEL;
    DescReference.SamplerOrTextureName = "TestName";
    DescReference.Desc.Flags           = SAMPLER_FLAG_SUBSAMPLED;

    ImmutableSamplerDesc Desc{};
    ParseRSN(JsonReference, Desc, Allocator);
    ASSERT_EQ(Desc, DescReference);
}

TEST(Tools_RenderStateNotationParser, ParsePipelineResourceSignatureDesc)
{
    CHECK_STRUCT_SIZE(PipelineResourceSignatureDesc, 56);

    DynamicLinearAllocator Allocator{DefaultRawMemoryAllocator::GetAllocator()};

    nlohmann::json JsonReference = LoadDRSNFromFile("RenderStates/PipelineResourceSignature/PipelineResourceSignatureDesc.json");

    constexpr PipelineResourceDesc Resources[] = {
        PipelineResourceDesc{SHADER_TYPE_VERTEX, "TestName0", 1, SHADER_RESOURCE_TYPE_BUFFER_UAV},
        PipelineResourceDesc{SHADER_TYPE_ALL_MESH, "TestName1", 2, SHADER_RESOURCE_TYPE_TEXTURE_SRV},
        PipelineResourceDesc{SHADER_TYPE_ALL_GRAPHICS, "TestName2", 3, SHADER_RESOURCE_TYPE_INPUT_ATTACHMENT}};

    constexpr ImmutableSamplerDesc Samplers[] = {
        ImmutableSamplerDesc{SHADER_TYPE_ALL_RAY_TRACING, "TestName0", {}},
        ImmutableSamplerDesc{SHADER_TYPE_PIXEL, "TestName1", {}}};

    PipelineResourceSignatureDesc DescReference{};
    DescReference.Name                       = "TestName0";
    DescReference.Resources                  = Resources;
    DescReference.NumResources               = _countof(Resources);
    DescReference.ImmutableSamplers          = Samplers;
    DescReference.NumImmutableSamplers       = _countof(Samplers);
    DescReference.BindingIndex               = 1;
    DescReference.UseCombinedTextureSamplers = true;
    DescReference.CombinedSamplerSuffix      = "_sampler_test";
    DescReference.SRBAllocationGranularity   = 16;

    PipelineResourceSignatureDesc Desc{};
    ParseRSN(JsonReference, Desc, Allocator);
    ASSERT_EQ(Desc, DescReference);
}

} // namespace
