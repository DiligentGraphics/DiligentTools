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
#include "RefCntAutoPtr.hpp"
#include "RenderStateNotationParser.h"
#include "DefaultShaderSourceStreamFactory.h"
#include "TestingEnvironment.hpp"
#include "GraphicsTypesOutputInserters.hpp"

using namespace Diligent;
using namespace Diligent::Testing;

namespace
{

RefCntAutoPtr<IRenderStateNotationParser> LoadFromFile(const Char* Path, IShaderSourceInputStreamFactory* pReloadFactory = nullptr)
{
    RefCntAutoPtr<IShaderSourceInputStreamFactory> pStreamFactory;
    CreateDefaultShaderSourceStreamFactory("RenderStates/RenderStateNotationParser", &pStreamFactory);

    RefCntAutoPtr<IRenderStateNotationParser> pParser;
    CreateRenderStateNotationParser({pReloadFactory != nullptr}, &pParser);

    if (pParser)
        pParser->ParseFile(Path, pStreamFactory, pReloadFactory);

    return pParser;
}

TEST(Tools_RenderStateNotationParser, LoadFromStringLiteral)
{
    const char*                               Json = "{}";
    RefCntAutoPtr<IRenderStateNotationParser> pParser;
    CreateRenderStateNotationParser({}, &pParser);
    ASSERT_NE(pParser, nullptr);
    EXPECT_EQ(pParser->ParseString(Json, 0, nullptr), true);
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
    DescReference.Desc.NumRenderTargets       = 2;
    DescReference.Desc.RTVFormats[0]          = TEX_FORMAT_RGBA8_UNORM;
    DescReference.Desc.RTVFormats[1]          = TEX_FORMAT_RG16_FLOAT;
    DescReference.PSODesc.Name                = "TestName";
    DescReference.PSODesc.PipelineType        = PIPELINE_TYPE_MESH;
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

    const auto* pDesc = static_cast<const GraphicsPipelineNotation*>(pParser->GetPipelineStateByName("TestName"));
    ASSERT_NE(pDesc, nullptr);
    EXPECT_EQ(*pDesc, DescReference);
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

    const auto* pDesc = static_cast<const ComputePipelineNotation*>(pParser->GetPipelineStateByName("TestName"));
    ASSERT_NE(pDesc, nullptr);
    EXPECT_EQ(*pDesc, DescReference);
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

    const auto* pDesc = static_cast<const RayTracingPipelineNotation*>(pParser->GetPipelineStateByName("TestName"));
    ASSERT_NE(pDesc, nullptr);
    EXPECT_EQ(*pDesc, DescReference);
}

TEST(Tools_RenderStateNotationParser, TilePipelineNotationTest)
{
    RefCntAutoPtr<IRenderStateNotationParser> pParser = LoadFromFile("TilePipelineNotation.json");
    ASSERT_NE(pParser, nullptr);

    TilePipelineNotation DescReference{};
    DescReference.PSODesc.Name         = "TestName";
    DescReference.PSODesc.PipelineType = PIPELINE_TYPE_TILE;
    DescReference.pTSName              = "Shader-TS";

    const auto* pDesc = static_cast<const TilePipelineNotation*>(pParser->GetPipelineStateByName("TestName"));
    ASSERT_NE(pDesc, nullptr);
    EXPECT_EQ(*pDesc, DescReference);
}

TEST(Tools_RenderStateNotationParser, InlinePipelineStatesTest)
{
    RefCntAutoPtr<IRenderStateNotationParser> pParser = LoadFromFile("InlinePipelineStates.json");
    ASSERT_NE(pParser, nullptr);

    {
        const auto* pDesc = static_cast<const GraphicsPipelineNotation*>(pParser->GetPipelineStateByName("Graphics-TestName"));
        ASSERT_NE(pDesc, nullptr);

        ShaderCreateInfo VSShaderReference{};
        VSShaderReference.Desc.Name       = "Shader0-VS";
        VSShaderReference.Desc.ShaderType = SHADER_TYPE_VERTEX;

        auto pVSShader = pParser->GetShaderByName(pDesc->pVSName);
        ASSERT_NE(pVSShader, nullptr);
        EXPECT_EQ(pVSShader->Desc, VSShaderReference.Desc);

        ShaderCreateInfo PSShaderReference{};
        PSShaderReference.Desc.Name       = "Shader0-PS";
        PSShaderReference.Desc.ShaderType = SHADER_TYPE_PIXEL;

        auto pPSShader = pParser->GetShaderByName(pDesc->pPSName);
        ASSERT_NE(pPSShader, nullptr);
        EXPECT_EQ(pPSShader->Desc, PSShaderReference.Desc);

        PipelineResourceSignatureDesc ResourceSignatureReference{};
        ResourceSignatureReference.Name = "Signature0";

        ASSERT_EQ(pDesc->ResourceSignaturesNameCount, 1u);
        auto pSignature = pParser->GetResourceSignatureByName(pDesc->ppResourceSignatureNames[0]);
        ASSERT_NE(pSignature, nullptr);
        EXPECT_EQ(*pSignature, ResourceSignatureReference);

        RenderPassDesc RenderPassReference{};
        RenderPassReference.Name = "RenderPass0";
        auto pRenderPass         = pParser->GetRenderPassByName("RenderPass0");
        ASSERT_NE(pRenderPass, nullptr);
        EXPECT_EQ(*pRenderPass, RenderPassReference);
    }

    {
        const auto* pDesc = static_cast<const ComputePipelineNotation*>(pParser->GetPipelineStateByName("Compute-TestName"));
        ASSERT_NE(pDesc, nullptr);

        ShaderCreateInfo CSShaderReference{};
        CSShaderReference.Desc.Name       = "Shader0-CS";
        CSShaderReference.Desc.ShaderType = SHADER_TYPE_COMPUTE;

        auto pCSShader = pParser->GetShaderByName(pDesc->pCSName);
        ASSERT_NE(pCSShader, nullptr);
        EXPECT_EQ(pCSShader->Desc, CSShaderReference.Desc);
    }

    {
        const auto* pDesc = static_cast<const TilePipelineNotation*>(pParser->GetPipelineStateByName("Tile-TestName"));
        ASSERT_NE(pDesc, nullptr);

        ShaderCreateInfo TSShaderReference{};
        TSShaderReference.Desc.Name       = "Shader0-TS";
        TSShaderReference.Desc.ShaderType = SHADER_TYPE_TILE;

        auto pTSShader = pParser->GetShaderByName(pDesc->pTSName);
        ASSERT_NE(pTSShader, nullptr);
        EXPECT_EQ(pTSShader->Desc, TSShaderReference.Desc);
    }

    {
        const auto* pDesc = static_cast<const RayTracingPipelineNotation*>(pParser->GetPipelineStateByName("RayTracing-TestName"));
        ASSERT_NE(pDesc, nullptr);

        {
            ASSERT_EQ(pDesc->GeneralShaderCount, 1u);

            ShaderCreateInfo RTShader0Reference{};
            RTShader0Reference.Desc.Name       = "Shader0-RayGen";
            RTShader0Reference.Desc.ShaderType = SHADER_TYPE_RAY_GEN;

            auto pRTShader0 = pParser->GetShaderByName(pDesc->pGeneralShaders[0].pShaderName);
            ASSERT_NE(pRTShader0, nullptr);
            EXPECT_EQ(pRTShader0->Desc, RTShader0Reference.Desc);
        }

        {
            ASSERT_EQ(pDesc->TriangleHitShaderCount, 1u);

            ShaderCreateInfo RTShader0Reference{};
            RTShader0Reference.Desc.Name       = "Shader0-RayClosestHit";
            RTShader0Reference.Desc.ShaderType = SHADER_TYPE_RAY_CLOSEST_HIT;

            auto pRTShader0 = pParser->GetShaderByName(pDesc->pTriangleHitShaders[0].pClosestHitShaderName);
            ASSERT_NE(pRTShader0, nullptr);
            EXPECT_EQ(pRTShader0->Desc, RTShader0Reference.Desc);

            ShaderCreateInfo RTShader1Reference{};
            RTShader1Reference.Desc.Name       = "Shader0-RayAnyHit";
            RTShader1Reference.Desc.ShaderType = SHADER_TYPE_RAY_ANY_HIT;

            auto pRTShader1 = pParser->GetShaderByName(pDesc->pTriangleHitShaders[0].pAnyHitShaderName);
            ASSERT_NE(pRTShader1, nullptr);
            EXPECT_EQ(pRTShader1->Desc, RTShader1Reference.Desc);
        }

        {
            ASSERT_EQ(pDesc->ProceduralHitShaderCount, 1u);

            ShaderCreateInfo RTShader0Reference{};
            RTShader0Reference.Desc.Name       = "Shader0-RayIntersection";
            RTShader0Reference.Desc.ShaderType = SHADER_TYPE_RAY_INTERSECTION;

            auto pRTShader0 = pParser->GetShaderByName(pDesc->pProceduralHitShaders[0].pIntersectionShaderName);
            ASSERT_NE(pRTShader0, nullptr);
            EXPECT_EQ(pRTShader0->Desc, RTShader0Reference.Desc);

            ShaderCreateInfo RTShader1Reference{};
            RTShader1Reference.Desc.Name       = "Shader0-RayClosestHit";
            RTShader1Reference.Desc.ShaderType = SHADER_TYPE_RAY_CLOSEST_HIT;

            auto pRTShader1 = pParser->GetShaderByName(pDesc->pProceduralHitShaders[0].pClosestHitShaderName);
            ASSERT_NE(pRTShader1, nullptr);
            EXPECT_EQ(pRTShader1->Desc, RTShader1Reference.Desc);

            ShaderCreateInfo RTShader2Reference{};
            RTShader2Reference.Desc.Name       = "Shader0-RayAnyHit";
            RTShader2Reference.Desc.ShaderType = SHADER_TYPE_RAY_ANY_HIT;

            auto pRTShader2 = pParser->GetShaderByName(pDesc->pProceduralHitShaders[0].pAnyHitShaderName);
            ASSERT_NE(pRTShader2, nullptr);
            EXPECT_EQ(pRTShader2->Desc, RTShader2Reference.Desc);
        }
    }
}

TEST(Tools_RenderStateNotationParser, ImplicitPipelineStatesTest)
{
    RefCntAutoPtr<IRenderStateNotationParser> pParser = LoadFromFile("ImplicitPipelineStates.json");
    ASSERT_NE(pParser, nullptr);

    {
        GraphicsPipelineNotation PipelineReference{};
        PipelineReference.PSODesc.PipelineType = PIPELINE_TYPE_GRAPHICS;
        PipelineReference.PSODesc.Name         = "Graphics-TestName";
        PipelineReference.pVSName              = "Shader0-VS";
        PipelineReference.pPSName              = "Shader0-PS";

        auto pPipeline = static_cast<const GraphicsPipelineNotation*>(pParser->GetPipelineStateByName(PipelineReference.PSODesc.Name));
        ASSERT_NE(pPipeline, nullptr);
        EXPECT_EQ(*pPipeline, PipelineReference);

        ShaderCreateInfo VSShaderReference{};
        VSShaderReference.Desc.Name       = PipelineReference.pVSName;
        VSShaderReference.Desc.ShaderType = SHADER_TYPE_VERTEX;

        auto pVSShader = pParser->GetShaderByName(PipelineReference.pVSName);
        ASSERT_NE(pVSShader, nullptr);
        EXPECT_EQ(pVSShader->Desc, VSShaderReference.Desc);

        ShaderCreateInfo PSShaderReference{};
        PSShaderReference.Desc.Name       = PipelineReference.pPSName;
        PSShaderReference.Desc.ShaderType = SHADER_TYPE_PIXEL;

        auto pPSShader = pParser->GetShaderByName(PipelineReference.pPSName);
        ASSERT_NE(pPSShader, nullptr);
        EXPECT_EQ(pPSShader->Desc, PSShaderReference.Desc);
    }

    {
        GraphicsPipelineNotation PipelineReference{};
        PipelineReference.PSODesc.PipelineType = PIPELINE_TYPE_MESH;
        PipelineReference.PSODesc.Name         = "Mesh-TestName";
        PipelineReference.pASName              = "Shader0-AS";
        PipelineReference.pMSName              = "Shader0-MS";

        auto pPipeline = static_cast<const GraphicsPipelineNotation*>(pParser->GetPipelineStateByName(PipelineReference.PSODesc.Name));
        ASSERT_NE(pPipeline, nullptr);
        EXPECT_EQ(*pPipeline, PipelineReference);

        ShaderCreateInfo ASShaderReference{};
        ASShaderReference.Desc.Name       = PipelineReference.pASName;
        ASShaderReference.Desc.ShaderType = SHADER_TYPE_AMPLIFICATION;

        auto pASShader = pParser->GetShaderByName(PipelineReference.pASName);
        ASSERT_NE(pASShader, nullptr);
        EXPECT_EQ(pASShader->Desc, ASShaderReference.Desc);

        ShaderCreateInfo MSShaderReference{};
        MSShaderReference.Desc.Name       = PipelineReference.pMSName;
        MSShaderReference.Desc.ShaderType = SHADER_TYPE_MESH;

        auto pMSShader = pParser->GetShaderByName(PipelineReference.pMSName);
        ASSERT_NE(pMSShader, nullptr);
        EXPECT_EQ(pMSShader->Desc, MSShaderReference.Desc);
    }

    {
        ComputePipelineNotation PipelineReference{};
        PipelineReference.PSODesc.PipelineType = PIPELINE_TYPE_COMPUTE;
        PipelineReference.PSODesc.Name         = "Compute-TestName";
        PipelineReference.pCSName              = "Shader0-CS";

        auto pPipeline = static_cast<const ComputePipelineNotation*>(pParser->GetPipelineStateByName(PipelineReference.PSODesc.Name));
        ASSERT_NE(pPipeline, nullptr);
        EXPECT_EQ(*pPipeline, PipelineReference);

        ShaderCreateInfo CSShaderReference{};
        CSShaderReference.Desc.Name       = PipelineReference.pCSName;
        CSShaderReference.Desc.ShaderType = SHADER_TYPE_COMPUTE;

        auto pCSShader = pParser->GetShaderByName(PipelineReference.pCSName);
        ASSERT_NE(pCSShader, nullptr);
        EXPECT_EQ(pCSShader->Desc, CSShaderReference.Desc);
    }

    {
        TilePipelineNotation PipelineReference{};
        PipelineReference.PSODesc.PipelineType = PIPELINE_TYPE_TILE;
        PipelineReference.PSODesc.Name         = "Tile-TestName";
        PipelineReference.pTSName              = "Shader0-TS";

        auto pPipeline = static_cast<const TilePipelineNotation*>(pParser->GetPipelineStateByName(PipelineReference.PSODesc.Name));
        ASSERT_NE(pPipeline, nullptr);
        EXPECT_EQ(*pPipeline, PipelineReference);

        ShaderCreateInfo TSShaderReference{};
        TSShaderReference.Desc.Name       = PipelineReference.pTSName;
        TSShaderReference.Desc.ShaderType = SHADER_TYPE_TILE;

        auto pTSShader = pParser->GetShaderByName(PipelineReference.pTSName);
        ASSERT_NE(pTSShader, nullptr);
        EXPECT_EQ(pTSShader->Desc, TSShaderReference.Desc);
    }

    {
        RTGeneralShaderGroupNotation GenearalShaders[] = {
            {"Name0", "Shader0-RayGen"}};

        RTTriangleHitShaderGroupNotation TriangleShaders[] = {
            {"Name0", "Shader0-RayClosestHit", "Shader0-RayAnyHit"}};

        RTProceduralHitShaderGroupNotation ProceduralShaders[] = {
            {"Name0", "Shader0-RayIntersection", "Shader0-RayClosestHit", "Shader0-RayAnyHit"}};

        RayTracingPipelineNotation PipelineReference{};
        PipelineReference.PSODesc.PipelineType     = PIPELINE_TYPE_RAY_TRACING;
        PipelineReference.PSODesc.Name             = "RayTracing-TestName";
        PipelineReference.pGeneralShaders          = GenearalShaders;
        PipelineReference.pTriangleHitShaders      = TriangleShaders;
        PipelineReference.pProceduralHitShaders    = ProceduralShaders;
        PipelineReference.GeneralShaderCount       = _countof(GenearalShaders);
        PipelineReference.TriangleHitShaderCount   = _countof(TriangleShaders);
        PipelineReference.ProceduralHitShaderCount = _countof(ProceduralShaders);

        auto pPipeline = static_cast<const RayTracingPipelineNotation*>(pParser->GetPipelineStateByName(PipelineReference.PSODesc.Name));
        ASSERT_NE(pPipeline, nullptr);
        EXPECT_EQ(*pPipeline, PipelineReference);

        {
            ASSERT_EQ(pPipeline->GeneralShaderCount, _countof(GenearalShaders));

            ShaderCreateInfo RTShader0Reference{};
            RTShader0Reference.Desc.Name       = GenearalShaders[0].pShaderName;
            RTShader0Reference.Desc.ShaderType = SHADER_TYPE_RAY_GEN;

            auto pRTShader0 = pParser->GetShaderByName(GenearalShaders[0].pShaderName);
            ASSERT_NE(pRTShader0, nullptr);
            EXPECT_EQ(pRTShader0->Desc, RTShader0Reference.Desc);
        }

        {
            ASSERT_EQ(pPipeline->TriangleHitShaderCount, _countof(TriangleShaders));

            ShaderCreateInfo RTShader0Reference{};
            RTShader0Reference.Desc.Name       = "Shader0-RayClosestHit";
            RTShader0Reference.Desc.ShaderType = SHADER_TYPE_RAY_CLOSEST_HIT;

            auto pRTShader0 = pParser->GetShaderByName(TriangleShaders[0].pClosestHitShaderName);
            ASSERT_NE(pRTShader0, nullptr);
            EXPECT_EQ(pRTShader0->Desc, RTShader0Reference.Desc);

            ShaderCreateInfo RTShader1Reference{};
            RTShader1Reference.Desc.Name       = "Shader0-RayAnyHit";
            RTShader1Reference.Desc.ShaderType = SHADER_TYPE_RAY_ANY_HIT;

            auto pRTShader1 = pParser->GetShaderByName(TriangleShaders[0].pAnyHitShaderName);
            ASSERT_NE(pRTShader1, nullptr);
            EXPECT_EQ(pRTShader1->Desc, RTShader1Reference.Desc);
        }

        {
            ASSERT_EQ(pPipeline->ProceduralHitShaderCount, _countof(ProceduralShaders));

            ShaderCreateInfo RTShader0Reference{};
            RTShader0Reference.Desc.Name       = "Shader0-RayIntersection";
            RTShader0Reference.Desc.ShaderType = SHADER_TYPE_RAY_INTERSECTION;

            auto pRTShader0 = pParser->GetShaderByName(ProceduralShaders[0].pIntersectionShaderName);
            ASSERT_NE(pRTShader0, nullptr);
            EXPECT_EQ(pRTShader0->Desc, RTShader0Reference.Desc);

            ShaderCreateInfo RTShader1Reference{};
            RTShader1Reference.Desc.Name       = "Shader0-RayClosestHit";
            RTShader1Reference.Desc.ShaderType = SHADER_TYPE_RAY_CLOSEST_HIT;

            auto pRTShader1 = pParser->GetShaderByName(ProceduralShaders[0].pClosestHitShaderName);
            ASSERT_NE(pRTShader1, nullptr);
            EXPECT_EQ(pRTShader1->Desc, RTShader1Reference.Desc);

            ShaderCreateInfo RTShader2Reference{};
            RTShader2Reference.Desc.Name       = "Shader0-RayAnyHit";
            RTShader2Reference.Desc.ShaderType = SHADER_TYPE_RAY_ANY_HIT;

            auto pRTShader2 = pParser->GetShaderByName(ProceduralShaders[0].pAnyHitShaderName);
            ASSERT_NE(pRTShader2, nullptr);
            EXPECT_EQ(pRTShader2->Desc, RTShader2Reference.Desc);
        }
    }
}

TEST(Tools_RenderStateNotationParser, DefaultPipelineStatesTest)
{
    RefCntAutoPtr<IRenderStateNotationParser> pParser = LoadFromFile("DefaultPipelineStates.json");
    ASSERT_NE(pParser, nullptr);

    const Char* Signatures[] = {
        "Signature0"};

    GraphicsPipelineNotation PipelineReference{};
    PipelineReference.PSODesc.Name                = "Graphics-TestName";
    PipelineReference.PSODesc.PipelineType        = PIPELINE_TYPE_GRAPHICS;
    PipelineReference.Flags                       = PSO_CREATE_FLAG_IGNORE_MISSING_VARIABLES;
    PipelineReference.ResourceSignaturesNameCount = 1;
    PipelineReference.ppResourceSignatureNames    = Signatures;
    PipelineReference.pVSName                     = "Shader0-VS";
    PipelineReference.pPSName                     = "Shader0-PS";
    PipelineReference.pRenderPassName             = "RenderPass0";

    auto pPipeline = static_cast<const GraphicsPipelineNotation*>(pParser->GetPipelineStateByName("Graphics-TestName"));
    ASSERT_NE(pPipeline, nullptr);
    EXPECT_EQ(*pPipeline, PipelineReference);

    ShaderCreateInfo VSShaderReference{};
    VSShaderReference.Desc           = {"Shader0-VS", SHADER_TYPE_VERTEX, true};
    VSShaderReference.SourceLanguage = SHADER_SOURCE_LANGUAGE_HLSL;

    auto pVSShader = pParser->GetShaderByName(pPipeline->pVSName);
    ASSERT_NE(pVSShader, nullptr);
    EXPECT_STREQ(pVSShader->Desc.Name, VSShaderReference.Desc.Name);
    EXPECT_EQ(pVSShader->Desc, VSShaderReference.Desc);
    EXPECT_EQ(pVSShader->SourceLanguage, VSShaderReference.SourceLanguage);

    ShaderCreateInfo PSShaderReference{};
    PSShaderReference.Desc           = {"Shader0-PS", SHADER_TYPE_PIXEL, true};
    PSShaderReference.SourceLanguage = SHADER_SOURCE_LANGUAGE_HLSL;

    auto pPSShader = pParser->GetShaderByName(pPipeline->pPSName);
    ASSERT_NE(pPSShader, nullptr);
    EXPECT_STREQ(pPSShader->Desc.Name, PSShaderReference.Desc.Name);
    EXPECT_EQ(pPSShader->Desc, PSShaderReference.Desc);
    EXPECT_EQ(pPSShader->SourceLanguage, PSShaderReference.SourceLanguage);

    PipelineResourceSignatureDesc ResourceSignatureReference{};
    ResourceSignatureReference.CombinedSamplerSuffix      = "TestSuffix";
    ResourceSignatureReference.UseCombinedTextureSamplers = true;

    auto pResourceSignature = pParser->GetResourceSignatureByName(pPipeline->ppResourceSignatureNames[0]);
    ASSERT_NE(pResourceSignature, nullptr);
    EXPECT_EQ(*pResourceSignature, ResourceSignatureReference);

    RenderPassAttachmentDesc Attachments[] = {
        {TEX_FORMAT_RGBA16_FLOAT}};

    RenderPassDesc RenderPassReference{};
    RenderPassReference.AttachmentCount = _countof(Attachments);
    RenderPassReference.pAttachments    = Attachments;

    auto pRenderPass = pParser->GetRenderPassByName(pPipeline->pRenderPassName);
    ASSERT_NE(pRenderPass, nullptr);
    EXPECT_EQ(*pRenderPass, RenderPassReference);
}

TEST(Tools_RenderStateNotationParser, RenderStateNotationParserTest)
{
    RefCntAutoPtr<IRenderStateNotationParser> pParser = LoadFromFile("RenderStatesLibrary.json");
    ASSERT_NE(pParser, nullptr);

    auto const& ParserInfo = pParser->GetInfo();
    EXPECT_EQ(ParserInfo.ShaderCount, 3u);
    EXPECT_EQ(ParserInfo.RenderPassCount, 4u);
    EXPECT_EQ(ParserInfo.ResourceSignatureCount, 2u);
    EXPECT_EQ(ParserInfo.PipelineStateCount, 4u);

    auto Iterate = [](Uint32 ResourceCount, std::function<void(Uint32)> const& Callback) {
        for (Uint32 ResourceID = 0; ResourceID < ResourceCount; ResourceID++)
            Callback(ResourceID);
    };

    Iterate(ParserInfo.ShaderCount, [&](Uint32 Index) {
        auto pResourceSrc = pParser->GetShaderByIndex(Index);
        ASSERT_NE(pResourceSrc, nullptr);

        auto pResourceDst = pParser->GetShaderByName(pResourceSrc->Desc.Name);
        EXPECT_EQ(pResourceSrc, pResourceDst);
    });

    Iterate(ParserInfo.ResourceSignatureCount, [&](Uint32 Index) {
        auto pResourceSrc = pParser->GetResourceSignatureByIndex(Index);
        ASSERT_NE(pResourceSrc, nullptr);

        auto pResourceDst = pParser->GetResourceSignatureByName(pResourceSrc->Name);
        EXPECT_EQ(pResourceSrc, pResourceDst);
    });

    Iterate(ParserInfo.RenderPassCount, [&](Uint32 Index) {
        auto pResourceSrc = pParser->GetRenderPassByIndex(Index);
        ASSERT_NE(pResourceSrc, nullptr);

        auto pResourceDst = pParser->GetRenderPassByName(pResourceSrc->Name);
        EXPECT_EQ(pResourceSrc, pResourceDst);
    });

    Iterate(ParserInfo.PipelineStateCount, [&](Uint32 Index) {
        auto pResourceSrc = pParser->GetPipelineStateByIndex(Index);
        ASSERT_NE(pResourceSrc, nullptr);

        auto pResourceDst = pParser->GetPipelineStateByName(pResourceSrc->PSODesc.Name);
        EXPECT_EQ(pResourceSrc, pResourceDst);
    });
}

TEST(Tools_RenderStateNotationParser, DuplicationResorcesTest)
{
    RefCntAutoPtr<IRenderStateNotationParser> pParser = LoadFromFile("DuplicationResources.json");
    ASSERT_NE(pParser, nullptr);
}

TEST(Tools_RenderStateNotationParser, RedefinitionResourcesTest)
{
    {
        TestingEnvironment::ErrorScope TestScope{
            "Failed to parse file: 'RedefinitionResourceSignature.json'.",
            "Redefinition of resource signature 'TestName'.",
            "Redefinition of resource signature 'TestName'."};

        RefCntAutoPtr<IRenderStateNotationParser> pParser = LoadFromFile("RedefinitionResourceSignature.json");
        EXPECT_NE(pParser, nullptr);
    }

    {
        TestingEnvironment::ErrorScope TestScope{
            "Failed to parse file: 'RedefinitionShader.json'.",
            "Redefinition of shader 'TestName'.",
            "Redefinition of shader 'TestName'."};

        RefCntAutoPtr<IRenderStateNotationParser> pParser = LoadFromFile("RedefinitionShader.json");
        EXPECT_NE(pParser, nullptr);
    }

    {
        TestingEnvironment::ErrorScope TestScope{
            "Failed to parse file: 'RedefinitionRenderPass.json'.",
            "Redefinition of render pass 'TestName'.",
            "Redefinition of render pass 'TestName'."};

        RefCntAutoPtr<IRenderStateNotationParser> pParser = LoadFromFile("RedefinitionRenderPass.json");
        EXPECT_NE(pParser, nullptr);
    }
}

TEST(Tools_RenderStateNotationParser, InvalidJsonTest)
{
    TestingEnvironment::ErrorScope TestScope{
        "Failed create render state notation parser",
        "Failed to parse file: 'InvalidJson.json'.",
        "[json.exception.parse_error.101] parse error at line"};

    RefCntAutoPtr<IRenderStateNotationParser> pParser = LoadFromFile("InvalidJson.json");
    ASSERT_NE(pParser, nullptr);
}

TEST(Tools_RenderStateNotationParser, InvalidEnumTest)
{
    TestingEnvironment::ErrorScope TestScope{
        "Failed to parse file: 'InvalidEnum.json'.",
        "[json.exception.other_error.501] (/Pipelines/0/PSODesc/PipelineType) invalid enum value for PIPELINE_TYPE: TEST_TYPE"};

    RefCntAutoPtr<IRenderStateNotationParser> pParser = LoadFromFile("InvalidEnum.json");
    ASSERT_NE(pParser, nullptr);
}

TEST(Tools_RenderStateNotationParser, InvalidTypeTest)
{
    {
        TestingEnvironment::ErrorScope TestScope{
            "Failed to parse file: 'InvalidTypeBase.json'.",
            "[json.exception.type_error.302] (/Pipelines/0/PSODesc/PipelineType) type must be string, but is number"};

        RefCntAutoPtr<IRenderStateNotationParser> pParser = LoadFromFile("InvalidTypeBase.json");
        EXPECT_NE(pParser, nullptr);
    }

    {
        TestingEnvironment::ErrorScope TestScope{
            "Failed to parse file: 'InvalidTypeBitfield.json'.",
            "[json.exception.type_error.302] (/ResourceSignatures/0/Resources/0/ShaderStages) type must be array or string, but is object"};

        RefCntAutoPtr<IRenderStateNotationParser> pParser = LoadFromFile("InvalidTypeBitfield.json");
        EXPECT_NE(pParser, nullptr);
    }

    {
        TestingEnvironment::ErrorScope TestScope{
            "Failed to parse file: 'InvalidTypeConstArray.json'.",
            "[json.exception.type_error.302] (/Pipelines/0/GraphicsPipeline/RTVFormats) type must be object, but is array"};

        RefCntAutoPtr<IRenderStateNotationParser> pParser = LoadFromFile("InvalidTypeConstArray.json");
        EXPECT_NE(pParser, nullptr);
    }
}

TEST(Tools_RenderStateNotationParser, InvalidKey)
{
    TestingEnvironment::ErrorScope TestScope{
        "Failed to parse file: 'InvalidKey.json'.",
        "[json.exception.other_error.501] (/Pipelines/0/PSODesc) unexpected key: TestKey"};

    RefCntAutoPtr<IRenderStateNotationParser> pParser = LoadFromFile("InvalidKey.json");
    ASSERT_NE(pParser, nullptr);
}

TEST(Tools_RenderStateNotationParser, Reload)
{
    RefCntAutoPtr<IShaderSourceInputStreamFactory> pReloadFactory;
    CreateDefaultShaderSourceStreamFactory("RenderStates/RenderStateNotationParser/Reload", &pReloadFactory);
    ASSERT_TRUE(pReloadFactory);

    RefCntAutoPtr<IRenderStateNotationParser> pParser = LoadFromFile("GraphicsPipelineNotation.json", pReloadFactory);
    ASSERT_NE(pParser, nullptr);

    {
        const Char* ResourceSignatures[] = {
            "TestName0",
            "TestName1"};

        GraphicsPipelineNotation DescReference{};
        DescReference.Desc.PrimitiveTopology      = PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        DescReference.Desc.NumRenderTargets       = 2;
        DescReference.Desc.RTVFormats[0]          = TEX_FORMAT_RGBA8_UNORM;
        DescReference.Desc.RTVFormats[1]          = TEX_FORMAT_RG16_FLOAT;
        DescReference.PSODesc.Name                = "TestName";
        DescReference.PSODesc.PipelineType        = PIPELINE_TYPE_MESH;
        DescReference.Flags                       = PSO_CREATE_FLAG_IGNORE_MISSING_VARIABLES;
        DescReference.ppResourceSignatureNames    = ResourceSignatures;
        DescReference.ResourceSignaturesNameCount = _countof(ResourceSignatures);
        DescReference.pRenderPassName             = "RenderPassTest";
        DescReference.pVSName                     = "Shader-VS";
        DescReference.pPSName                     = "Shader-PS";
        DescReference.pDSName                     = "Shader-DS";
        DescReference.pHSName                     = "Shader-HS";
        DescReference.pGSName                     = "Shader-GS";
        DescReference.pASName                     = "Shader-AS";
        DescReference.pMSName                     = "Shader-MS";

        const auto* pDesc = static_cast<const GraphicsPipelineNotation*>(pParser->GetPipelineStateByName("TestName"));
        ASSERT_NE(pDesc, nullptr);
        EXPECT_EQ(*pDesc, DescReference);
    }

    EXPECT_TRUE(pParser->Reload());

    {
        const Char* ResourceSignatures[] = {
            "TestNameA",
            "TestNameB",
            "TestNameC"};

        GraphicsPipelineNotation DescReference{};
        DescReference.Desc.PrimitiveTopology      = PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
        DescReference.Desc.NumRenderTargets       = 3;
        DescReference.Desc.RTVFormats[0]          = TEX_FORMAT_RGBA8_UNORM_SRGB;
        DescReference.Desc.RTVFormats[1]          = TEX_FORMAT_RGBA32_FLOAT;
        DescReference.Desc.RTVFormats[2]          = TEX_FORMAT_R8_SINT;
        DescReference.PSODesc.Name                = "TestName";
        DescReference.PSODesc.PipelineType        = PIPELINE_TYPE_GRAPHICS;
        DescReference.Flags                       = PSO_CREATE_FLAG_IGNORE_MISSING_VARIABLES;
        DescReference.ppResourceSignatureNames    = ResourceSignatures;
        DescReference.ResourceSignaturesNameCount = _countof(ResourceSignatures);
        DescReference.pRenderPassName             = "RenderPassTest2";
        DescReference.pVSName                     = "Shader2-VS";
        DescReference.pPSName                     = "Shader2-PS";
        DescReference.pDSName                     = "Shader2-DS";
        DescReference.pHSName                     = "Shader2-HS";
        DescReference.pGSName                     = "Shader2-GS";

        const auto* pDesc = static_cast<const GraphicsPipelineNotation*>(pParser->GetPipelineStateByName("TestName"));
        ASSERT_NE(pDesc, nullptr);
        EXPECT_EQ(*pDesc, DescReference);
    }
}

} // namespace
