/*
 *  Copyright 2019-2021 Diligent Graphics LLC
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
#include "RefCntAutoPtr.hpp"
#include "RenderStateNotationParser.h"
#include "DefaultShaderSourceStreamFactory.h"

using namespace Diligent;

namespace
{

RefCntAutoPtr<IRenderStateNotationParser> LoadFromFile(const Char* Path)
{
    RefCntAutoPtr<IShaderSourceInputStreamFactory> pStreamFactory;
    CreateDefaultShaderSourceStreamFactory("RenderStates/RenderStateNotationParser", &pStreamFactory);

    RefCntAutoPtr<IRenderStateNotationParser> pParser;
    RenderStateNotationParserCreateInfo       ParserCI{};
    ParserCI.FilePath       = Path;
    ParserCI.pStreamFactory = pStreamFactory;

    CreateRenderStateNotationParser(ParserCI, &pParser);
    return pParser;
}

TEST(Tools_RenderStateNotationParser, GraphicsPipelineNotationTest)
{
    RefCntAutoPtr<IRenderStateNotationParser> pParser = LoadFromFile("GraphicsPipelineNotation.json");
    ASSERT_NE(pParser, nullptr);

    const Char* ResourceSignatures[] = {
        "TestName0",
        "TestName1"};

    GraphicsPipelineNotation DescReference{};
    DescReference.Desc.PrimitiveTopology      = PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    DescReference.PSODesc.Name                = "TestName";
    DescReference.PSODesc.PipelineType        = PIPELINE_TYPE_GRAPHICS;
    DescReference.Flags                       = PSO_CREATE_FLAG_IGNORE_MISSING_VARIABLES;
    DescReference.ppResourceSignatureNames    = ResourceSignatures;
    DescReference.ResourceSignaturesNameCount = 2;
    DescReference.pRenderPassName             = "RenderPassTest";
    DescReference.pVSName                     = "Shader-VS";
    DescReference.pPSName                     = "Shader-PS";
    DescReference.pDSName                     = "Shader-DS";
    DescReference.pHSName                     = "Shader-HS";
    DescReference.pGSName                     = "Shader-GS";
    DescReference.pASName                     = "Shader-AS";
    DescReference.pMSName                     = "Shader-MS";

    auto pDesc = pParser->GetGraphicsPipelineStateByName("TestName");
    ASSERT_NE(pDesc, nullptr);
    ASSERT_EQ(*pDesc, DescReference);
}

TEST(Tools_RenderStateNotationParser, ComputePipelineNotationTest)
{
    RefCntAutoPtr<IRenderStateNotationParser> pParser = LoadFromFile("ComputePipelineNotation.json");
    ASSERT_NE(pParser, nullptr);

    const Char* ResourceSignatures[] = {
        "TestName0"};

    ComputePipelineNotation DescReference{};
    DescReference.PSODesc.Name                = "TestName";
    DescReference.PSODesc.PipelineType        = PIPELINE_TYPE_COMPUTE;
    DescReference.ppResourceSignatureNames    = ResourceSignatures;
    DescReference.ResourceSignaturesNameCount = 1;
    DescReference.pCSName                     = "Shader-CS";

    auto pDesc = pParser->GetComputePipelineStateByName("TestName");
    ASSERT_NE(pDesc, nullptr);
    ASSERT_EQ(*pDesc, DescReference);
}

TEST(Tools_RenderStateNotationParser, RayTracingPipelineNotationTest)
{
    RefCntAutoPtr<IRenderStateNotationParser> pParser = LoadFromFile("RayTracingPipelineNotation.json");
    ASSERT_NE(pParser, nullptr);

    RTGeneralShaderGroupNotation GeneralShaders[] = {
        RTGeneralShaderGroupNotation{"Name0", "Shader0"}};

    RTTriangleHitShaderGroupNotation TriangleHitShaders[] = {
        RTTriangleHitShaderGroupNotation{"Name0", "ClosestHitShader0", "AnyHitShader0"},
        RTTriangleHitShaderGroupNotation{"Name1", "ClosestHitShader1", "AnyHitShader1"}};

    RTProceduralHitShaderGroupNotation ProceduralHitShaders[] = {
        RTProceduralHitShaderGroupNotation{"Name0", "IntersectionShader0", "ClosestHitShader0", "AnyHitShader0"},
        RTProceduralHitShaderGroupNotation{"Name1", "IntersectionShader1", "ClosestHitShader1", "AnyHitShader1"},
        RTProceduralHitShaderGroupNotation{"Name2", "IntersectionShader2", "ClosestHitShader2", "AnyHitShader2"}};

    RayTracingPipelineNotation DescReference{};
    DescReference.RayTracingPipeline.ShaderRecordSize  = 256;
    DescReference.RayTracingPipeline.MaxRecursionDepth = 8;

    DescReference.PSODesc.Name             = "TestName";
    DescReference.PSODesc.PipelineType     = PIPELINE_TYPE_RAY_TRACING;
    DescReference.MaxAttributeSize         = 2;
    DescReference.MaxPayloadSize           = 4;
    DescReference.pShaderRecordName        = "TestNameRecord";
    DescReference.pGeneralShaders          = GeneralShaders;
    DescReference.GeneralShaderCount       = _countof(GeneralShaders);
    DescReference.pTriangleHitShaders      = TriangleHitShaders;
    DescReference.TriangleHitShaderCount   = _countof(TriangleHitShaders);
    DescReference.pProceduralHitShaders    = ProceduralHitShaders;
    DescReference.ProceduralHitShaderCount = _countof(ProceduralHitShaders);

    auto pDesc = pParser->GetRayTracingPipelineStateByName("TestName");
    ASSERT_NE(pDesc, nullptr);
    ASSERT_EQ(*pDesc, DescReference);
}

TEST(Tools_RenderStateNotationParser, TilePipelineNotationTest)
{
    RefCntAutoPtr<IRenderStateNotationParser> pParser = LoadFromFile("TilePipelineNotation.json");
    ASSERT_NE(pParser, nullptr);

    TilePipelineNotation DescReference{};
    DescReference.PSODesc.Name         = "TestName";
    DescReference.PSODesc.PipelineType = PIPELINE_TYPE_TILE;
    DescReference.pTSName              = "Shader-TS";

    auto pDesc = pParser->GetTilePipelineStateByName("TestName");
    ASSERT_NE(pDesc, nullptr);
    ASSERT_EQ(*pDesc, DescReference);
}

TEST(Tools_RenderStateNotationParser, InlinePipelineStatesTest)
{
    RefCntAutoPtr<IRenderStateNotationParser> pParser = LoadFromFile("InlinePipelineStates.json");
    ASSERT_NE(pParser, nullptr);

    {
        auto pDesc = pParser->GetGraphicsPipelineStateByName("Graphics-TestName");
        ASSERT_NE(pDesc, nullptr);

        ShaderCreateInfo VSShaderReference{};
        VSShaderReference.Desc.Name       = "Shader0-VS";
        VSShaderReference.Desc.ShaderType = SHADER_TYPE_VERTEX;

        auto pVSShader = pParser->GetShaderByName(pDesc->pVSName);
        ASSERT_NE(pVSShader, nullptr);
        ASSERT_EQ(pVSShader->Desc, VSShaderReference.Desc);

        ShaderCreateInfo PSShaderReference{};
        PSShaderReference.Desc.Name       = "Shader0-PS";
        PSShaderReference.Desc.ShaderType = SHADER_TYPE_PIXEL;

        auto pPSShader = pParser->GetShaderByName(pDesc->pPSName);
        ASSERT_NE(pPSShader, nullptr);
        ASSERT_EQ(pPSShader->Desc, PSShaderReference.Desc);

        PipelineResourceSignatureDesc ResourceSignatureReference{};
        ResourceSignatureReference.Name = "Signature0";

        ASSERT_EQ(pDesc->ResourceSignaturesNameCount, 1u);
        auto pSignature = pParser->GetResourceSignatureByName(pDesc->ppResourceSignatureNames[0]);
        ASSERT_NE(pSignature, nullptr);
        ASSERT_EQ(*pSignature, ResourceSignatureReference);

        RenderPassDesc RenderPassReference{};
        RenderPassReference.Name = "RenderPass0";
        auto pRenderPass         = pParser->GetRenderPassByName("RenderPass0");
        ASSERT_NE(pRenderPass, nullptr);
        ASSERT_EQ(*pRenderPass, RenderPassReference);
    }

    {
        auto pDesc = pParser->GetComputePipelineStateByName("Compute-TestName");
        ASSERT_NE(pDesc, nullptr);

        ShaderCreateInfo CSShaderReference{};
        CSShaderReference.Desc.Name       = "Shader0-CS";
        CSShaderReference.Desc.ShaderType = SHADER_TYPE_COMPUTE;

        auto pCSShader = pParser->GetShaderByName(pDesc->pCSName);
        ASSERT_NE(pCSShader, nullptr);
        ASSERT_EQ(pCSShader->Desc, CSShaderReference.Desc);
    }

    {
        auto pDesc = pParser->GetTilePipelineStateByName("Tile-TestName");
        ASSERT_NE(pDesc, nullptr);

        ShaderCreateInfo TSShaderReference{};
        TSShaderReference.Desc.Name       = "Shader0-TS";
        TSShaderReference.Desc.ShaderType = SHADER_TYPE_TILE;

        auto pTSShader = pParser->GetShaderByName(pDesc->pTSName);
        ASSERT_NE(pTSShader, nullptr);
        ASSERT_EQ(pTSShader->Desc, TSShaderReference.Desc);
    }

    {
        auto pDesc = pParser->GetRayTracingPipelineStateByName("RayTracing-TestName");
        ASSERT_NE(pDesc, nullptr);

        {
            ASSERT_EQ(pDesc->GeneralShaderCount, 1u);

            ShaderCreateInfo RTShader0Reference{};
            RTShader0Reference.Desc.Name       = "Shader0-RayGen";
            RTShader0Reference.Desc.ShaderType = SHADER_TYPE_RAY_GEN;

            auto pRTShader0 = pParser->GetShaderByName(pDesc->pGeneralShaders[0].pShaderName);
            ASSERT_NE(pRTShader0, nullptr);
            ASSERT_EQ(pRTShader0->Desc, RTShader0Reference.Desc);
        }

        {
            ASSERT_EQ(pDesc->TriangleHitShaderCount, 1u);

            ShaderCreateInfo RTShader0Reference{};
            RTShader0Reference.Desc.Name       = "Shader0-RayClosestHit";
            RTShader0Reference.Desc.ShaderType = SHADER_TYPE_RAY_CLOSEST_HIT;

            auto pRTShader0 = pParser->GetShaderByName(pDesc->pTriangleHitShaders[0].pClosestHitShaderName);
            ASSERT_NE(pRTShader0, nullptr);
            ASSERT_EQ(pRTShader0->Desc, RTShader0Reference.Desc);

            ShaderCreateInfo RTShader1Reference{};
            RTShader1Reference.Desc.Name       = "Shader0-RayAnyHit";
            RTShader1Reference.Desc.ShaderType = SHADER_TYPE_RAY_ANY_HIT;

            auto pRTShader1 = pParser->GetShaderByName(pDesc->pTriangleHitShaders[0].pAnyHitShaderName);
            ASSERT_NE(pRTShader1, nullptr);
            ASSERT_EQ(pRTShader1->Desc, RTShader1Reference.Desc);
        }

        {
            ASSERT_EQ(pDesc->ProceduralHitShaderCount, 1u);

            ShaderCreateInfo RTShader0Reference{};
            RTShader0Reference.Desc.Name       = "Shader0-RayIntersection";
            RTShader0Reference.Desc.ShaderType = SHADER_TYPE_RAY_INTERSECTION;

            auto pRTShader0 = pParser->GetShaderByName(pDesc->pProceduralHitShaders[0].pIntersectionShaderName);
            ASSERT_NE(pRTShader0, nullptr);
            ASSERT_EQ(pRTShader0->Desc, RTShader0Reference.Desc);

            ShaderCreateInfo RTShader1Reference{};
            RTShader1Reference.Desc.Name       = "Shader0-RayClosestHit";
            RTShader1Reference.Desc.ShaderType = SHADER_TYPE_RAY_CLOSEST_HIT;

            auto pRTShader1 = pParser->GetShaderByName(pDesc->pProceduralHitShaders[0].pClosestHitShaderName);
            ASSERT_NE(pRTShader1, nullptr);
            ASSERT_EQ(pRTShader1->Desc, RTShader1Reference.Desc);

            ShaderCreateInfo RTShader2Reference{};
            RTShader2Reference.Desc.Name       = "Shader0-RayAnyHit";
            RTShader2Reference.Desc.ShaderType = SHADER_TYPE_RAY_ANY_HIT;

            auto pRTShader2 = pParser->GetShaderByName(pDesc->pProceduralHitShaders[0].pAnyHitShaderName);
            ASSERT_NE(pRTShader2, nullptr);
            ASSERT_EQ(pRTShader2->Desc, RTShader2Reference.Desc);
        }
    }
}

TEST(Tools_RenderStateNotationParser, RenderStateNotationParserTest)
{
    RefCntAutoPtr<IRenderStateNotationParser> pParser = LoadFromFile("RenderStatesLibrary.json");
    ASSERT_NE(pParser, nullptr);

    auto const& ParserInfo = pParser->GetInfo();
    ASSERT_EQ(ParserInfo.ShaderCount, 3u);
    ASSERT_EQ(ParserInfo.RenderPassCount, 4u);
    ASSERT_EQ(ParserInfo.ResourceSignatureCount, 2u);
    ASSERT_EQ(ParserInfo.GraphicsPipelineStateCount, 1u);
    ASSERT_EQ(ParserInfo.ComputePipelineStateCount, 1u);
    ASSERT_EQ(ParserInfo.RayTracingPipelineStateCount, 1u);
    ASSERT_EQ(ParserInfo.TilePipelineStateCount, 1u);

    auto Iterate = [](Uint32 ResourceCount, std::function<void(Uint32)> const& Callback) {
        for (Uint32 ResourceID = 0; ResourceID < ResourceCount; ResourceID++)
            Callback(ResourceID);
    };

    Iterate(ParserInfo.ShaderCount, [&](Uint32 Index) {
        auto pResourceSrc = pParser->GetShaderByIndex(Index);
        ASSERT_NE(pResourceSrc, nullptr);

        auto pResourceDst = pParser->GetShaderByName(pResourceSrc->Desc.Name);
        ASSERT_EQ(pResourceSrc, pResourceDst);
    });

    Iterate(ParserInfo.ResourceSignatureCount, [&](Uint32 Index) {
        auto pResourceSrc = pParser->GetResourceSignatureByIndex(Index);
        ASSERT_NE(pResourceSrc, nullptr);

        auto pResourceDst = pParser->GetResourceSignatureByName(pResourceSrc->Name);
        ASSERT_EQ(pResourceSrc, pResourceDst);
    });

    Iterate(ParserInfo.RenderPassCount, [&](Uint32 Index) {
        auto pResourceSrc = pParser->GetRenderPassByIndex(Index);
        ASSERT_NE(pResourceSrc, nullptr);

        auto pResourceDst = pParser->GetRenderPassByName(pResourceSrc->Name);
        ASSERT_EQ(pResourceSrc, pResourceDst);
    });

    Iterate(ParserInfo.GraphicsPipelineStateCount, [&](Uint32 Index) {
        auto pResourceSrc = pParser->GetGraphicsPipelineStateByIndex(Index);
        ASSERT_NE(pResourceSrc, nullptr);

        auto pResourceDst = pParser->GetGraphicsPipelineStateByName(pResourceSrc->PSODesc.Name);
        ASSERT_EQ(pResourceSrc, pResourceDst);
    });

    Iterate(ParserInfo.ComputePipelineStateCount, [&](Uint32 Index) {
        auto pResourceSrc = pParser->GetComputePipelineStateByIndex(Index);
        ASSERT_NE(pResourceSrc, nullptr);

        auto pResourceDst = pParser->GetComputePipelineStateByName(pResourceSrc->PSODesc.Name);
        ASSERT_EQ(pResourceSrc, pResourceDst);
    });

    Iterate(ParserInfo.RayTracingPipelineStateCount, [&](Uint32 Index) {
        auto pResourceSrc = pParser->GetRayTracingPipelineStateByIndex(Index);
        ASSERT_NE(pResourceSrc, nullptr);

        auto pResourceDst = pParser->GetRayTracingPipelineStateByName(pResourceSrc->PSODesc.Name);
        ASSERT_EQ(pResourceSrc, pResourceDst);
    });

    Iterate(ParserInfo.TilePipelineStateCount, [&](Uint32 Index) {
        auto pResourceSrc = pParser->GetTilePipelineStateByIndex(Index);
        ASSERT_NE(pResourceSrc, nullptr);

        auto pResourceDst = pParser->GetTilePipelineStateByName(pResourceSrc->PSODesc.Name);
        ASSERT_EQ(pResourceSrc, pResourceDst);
    });
}

} // namespace