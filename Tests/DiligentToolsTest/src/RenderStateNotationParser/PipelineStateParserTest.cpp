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

TEST(Tools_RenderStateNotationParser, ParsePipelineStateEnums)
{
    DynamicLinearAllocator Allocator{DefaultRawMemoryAllocator::GetAllocator()};

    ASSERT_TRUE(TestEnum<PIPELINE_TYPE>(Allocator, PIPELINE_TYPE_GRAPHICS, PIPELINE_TYPE_LAST));

    ASSERT_TRUE(TestBitwiseEnum<SHADER_VARIABLE_FLAGS>(Allocator, SHADER_VARIABLE_FLAG_LAST));

    ASSERT_TRUE(TestBitwiseEnum<PIPELINE_SHADING_RATE_FLAGS>(Allocator, PIPELINE_SHADING_RATE_FLAG_LAST));

    ASSERT_TRUE(TestBitwiseEnum<SHADER_VARIABLE_FLAGS>(Allocator, SHADER_VARIABLE_FLAG_LAST));

    ASSERT_TRUE(TestBitwiseEnum<PSO_CREATE_FLAGS>(Allocator, PSO_CREATE_FLAG_LAST));
}

TEST(Tools_RenderStateNotationParser, ParseSampleDesc)
{
    CHECK_STRUCT_SIZE(SampleDesc, 2);

    DynamicLinearAllocator Allocator{DefaultRawMemoryAllocator::GetAllocator()};

    nlohmann::json JsonReference = LoadDRSNFromFile("RenderStates/PipelineState/SampleDesc.json");

    SampleDesc DescReference{};
    DescReference.Count   = 4;
    DescReference.Quality = 1;

    SampleDesc Desc{};
    ParseRSN(JsonReference, Desc, Allocator);
    ASSERT_EQ(Desc, DescReference);
}

TEST(Tools_RenderStateNotationParser, ParseShaderResourceVariableDesc)
{
    CHECK_STRUCT_SIZE(ShaderResourceVariableDesc, 16);

    DynamicLinearAllocator Allocator{DefaultRawMemoryAllocator::GetAllocator()};

    nlohmann::json JsonReference = LoadDRSNFromFile("RenderStates/PipelineState/ShaderResourceVariableDesc.json");

    ShaderResourceVariableDesc DescReference{};
    DescReference.Name         = "TestName";
    DescReference.Type         = SHADER_RESOURCE_VARIABLE_TYPE_DYNAMIC;
    DescReference.ShaderStages = SHADER_TYPE_VERTEX | SHADER_TYPE_PIXEL;
    DescReference.Flags        = SHADER_VARIABLE_FLAG_NO_DYNAMIC_BUFFERS | SHADER_VARIABLE_FLAG_GENERAL_INPUT_ATTACHMENT;

    ShaderResourceVariableDesc Desc{};
    ParseRSN(JsonReference, Desc, Allocator);
    ASSERT_EQ(Desc, DescReference);
}

TEST(Tools_RenderStateNotationParser, ParsePipelineResourceLayoutDesc)
{
    CHECK_STRUCT_SIZE(PipelineResourceLayoutDesc, 40);

    DynamicLinearAllocator Allocator{DefaultRawMemoryAllocator::GetAllocator()};

    nlohmann::json JsonReference = LoadDRSNFromFile("RenderStates/PipelineState/PipelineResourceLayoutDesc.json");

    constexpr ShaderResourceVariableDesc Variables[] = {
        {SHADER_TYPE_VERTEX | SHADER_TYPE_PIXEL, "TestName0", SHADER_RESOURCE_VARIABLE_TYPE_STATIC},
        {SHADER_TYPE_VERTEX | SHADER_TYPE_PIXEL, "TestName1", SHADER_RESOURCE_VARIABLE_TYPE_DYNAMIC}};

    constexpr ImmutableSamplerDesc Samplers[] = {
        ImmutableSamplerDesc{SHADER_TYPE_ALL_RAY_TRACING, "TestName0", {FILTER_TYPE_POINT, FILTER_TYPE_MAXIMUM_POINT, FILTER_TYPE_ANISOTROPIC}},
        ImmutableSamplerDesc{SHADER_TYPE_PIXEL, "TestName1", {FILTER_TYPE_COMPARISON_POINT, FILTER_TYPE_COMPARISON_LINEAR, FILTER_TYPE_COMPARISON_ANISOTROPIC}}};

    PipelineResourceLayoutDesc DescReference{};
    DescReference.DefaultVariableMergeStages = SHADER_TYPE_ALL_GRAPHICS;
    DescReference.DefaultVariableType        = SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE;
    DescReference.Variables                  = Variables;
    DescReference.NumVariables               = _countof(Variables);
    DescReference.ImmutableSamplers          = Samplers;
    DescReference.NumImmutableSamplers       = _countof(Samplers);

    PipelineResourceLayoutDesc Desc{};
    ParseRSN(JsonReference, Desc, Allocator);
    ASSERT_EQ(Desc, DescReference);
}

TEST(Tools_RenderStateNotationParser, ParseGraphicsPipelineDesc)
{
    CHECK_STRUCT_SIZE(GraphicsPipelineDesc, 192);

    DynamicLinearAllocator Allocator{DefaultRawMemoryAllocator::GetAllocator()};

    nlohmann::json JsonReference = LoadDRSNFromFile("RenderStates/PipelineState/GraphicsPipelineDesc.json");

    constexpr LayoutElement InputLayoutElemets[] = {
        LayoutElement{0, 0, 3, VT_FLOAT32},
        LayoutElement{1, 0, 4, VT_FLOAT32}};

    GraphicsPipelineDesc DescReference{};
    DescReference.SampleMask                   = 1245678;
    DescReference.PrimitiveTopology            = PRIMITIVE_TOPOLOGY_POINT_LIST;
    DescReference.NumViewports                 = 2;
    DescReference.SubpassIndex                 = 1;
    DescReference.NodeMask                     = 1;
    DescReference.DepthStencilDesc.DepthEnable = false;
    DescReference.RasterizerDesc.CullMode      = CULL_MODE_FRONT;
    DescReference.ShadingRateFlags             = PIPELINE_SHADING_RATE_FLAG_PER_PRIMITIVE | PIPELINE_SHADING_RATE_FLAG_TEXTURE_BASED;

    DescReference.BlendDesc.RenderTargets[0].BlendEnable = true;
    DescReference.InputLayout.LayoutElements             = InputLayoutElemets;
    DescReference.InputLayout.NumElements                = _countof(InputLayoutElemets);

    DescReference.DSVFormat        = TEX_FORMAT_D32_FLOAT;
    DescReference.NumRenderTargets = 2;
    DescReference.RTVFormats[0]    = TEX_FORMAT_RGBA8_UNORM;
    DescReference.RTVFormats[1]    = TEX_FORMAT_RG16_FLOAT;
    DescReference.SmplDesc.Count   = 4;
    DescReference.SmplDesc.Quality = 1;

    GraphicsPipelineDesc Desc{};
    ParseRSN(JsonReference, Desc, Allocator);
    ASSERT_EQ(Desc, DescReference);
}

TEST(Tools_RenderStateNotationParser, ParseRayTracingPipelineDesc)
{
    CHECK_STRUCT_SIZE(RayTracingPipelineDesc, 4);

    DynamicLinearAllocator Allocator{DefaultRawMemoryAllocator::GetAllocator()};

    nlohmann::json JsonReference = LoadDRSNFromFile("RenderStates/PipelineState/RayTracingPipelineDesc.json");

    RayTracingPipelineDesc DescReference{};
    DescReference.MaxRecursionDepth = 7;
    DescReference.ShaderRecordSize  = 4096;

    RayTracingPipelineDesc Desc{};
    ParseRSN(JsonReference, Desc, Allocator);
    ASSERT_EQ(Desc, DescReference);
}

TEST(Tools_RenderStateNotationParser, ParsePipelineStateDesc)
{
    CHECK_STRUCT_SIZE(PipelineStateDesc, 64);

    DynamicLinearAllocator Allocator{DefaultRawMemoryAllocator::GetAllocator()};

    nlohmann::json JsonReference = LoadDRSNFromFile("RenderStates/PipelineState/PipelineStateDesc.json");

    PipelineStateDesc DescReference{};
    DescReference.PipelineType                       = PIPELINE_TYPE_COMPUTE;
    DescReference.Name                               = "TestName";
    DescReference.SRBAllocationGranularity           = 16;
    DescReference.ImmediateContextMask               = 1;
    DescReference.ResourceLayout.DefaultVariableType = SHADER_RESOURCE_VARIABLE_TYPE_DYNAMIC;

    PipelineStateDesc Desc{};
    ParseRSN(JsonReference, Desc, Allocator);
    ASSERT_EQ(Desc, DescReference);
}

TEST(Tools_RenderStateNotationParser, ParseTilePipelineDesc)
{
    CHECK_STRUCT_SIZE(TilePipelineDesc, 18);

    DynamicLinearAllocator Allocator{DefaultRawMemoryAllocator::GetAllocator()};

    nlohmann::json JsonReference = LoadDRSNFromFile("RenderStates/PipelineState/TilePipelineDesc.json");

    TilePipelineDesc DescReference{};
    DescReference.NumRenderTargets = 2;
    DescReference.RTVFormats[0]    = TEX_FORMAT_RGBA8_UNORM;
    DescReference.RTVFormats[1]    = TEX_FORMAT_RG16_FLOAT;
    DescReference.SampleCount      = 4;

    TilePipelineDesc Desc{};
    ParseRSN(JsonReference, Desc, Allocator);
    ASSERT_EQ(Desc, DescReference);
}

} // namespace
