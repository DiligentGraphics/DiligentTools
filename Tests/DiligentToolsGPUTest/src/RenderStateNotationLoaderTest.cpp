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
#include "RenderStateNotationLoader.h"
#include "DefaultShaderSourceStreamFactory.h"
#include "RenderStateCache.h"
#include "GPUTestingEnvironment.hpp"

using namespace Diligent;
using namespace Diligent::Testing;

namespace
{

RefCntAutoPtr<IRenderStateNotationParser> CreateParser(const Char* Path)
{
    RefCntAutoPtr<IShaderSourceInputStreamFactory> pStreamFactory;
    CreateDefaultShaderSourceStreamFactory("RenderStates", &pStreamFactory);

    RefCntAutoPtr<IRenderStateNotationParser> pParser;
    CreateRenderStateNotationParser({}, &pParser);

    if (pParser)
        pParser->ParseFile(Path, pStreamFactory);

    return pParser;
}

RefCntAutoPtr<IShaderSourceInputStreamFactory> CreateShaderFactory(const char* Path = nullptr)
{
    if (Path == nullptr)
        Path = "Shaders";
    RefCntAutoPtr<IShaderSourceInputStreamFactory> pStreamFactory;
    CreateDefaultShaderSourceStreamFactory(Path, &pStreamFactory);
    return pStreamFactory;
}

GraphicsPipelineDesc GetGraphicsPipelineRefDesc()
{
    GraphicsPipelineDesc Desc;

    Desc.DepthStencilDesc.DepthEnable      = true;
    Desc.DepthStencilDesc.DepthWriteEnable = true;
    Desc.DepthStencilDesc.DepthFunc        = COMPARISON_FUNC_LESS;

    Desc.RasterizerDesc.FillMode              = FILL_MODE_SOLID;
    Desc.RasterizerDesc.CullMode              = CULL_MODE_BACK;
    Desc.RasterizerDesc.FrontCounterClockwise = true;
    Desc.RasterizerDesc.DepthClipEnable       = true;

    Desc.NumRenderTargets  = 1;
    Desc.RTVFormats[0]     = TEX_FORMAT_RGBA8_UNORM_SRGB;
    Desc.DSVFormat         = TEX_FORMAT_D32_FLOAT;
    Desc.PrimitiveTopology = PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

    return Desc;
}

TEST(Tools_RenderStateNotationLoader, BasicTest)
{
    auto* pEnvironment = GPUTestingEnvironment::GetInstance();
    ASSERT_NE(pEnvironment, nullptr);

    auto* pDevice        = pEnvironment->GetDevice();
    auto  pParser        = CreateParser("PSO.json");
    auto  pStreamFactory = CreateShaderFactory();

    RenderStateNotationLoaderCreateInfo LoaderCI{};
    LoaderCI.pDevice        = pDevice;
    LoaderCI.pParser        = pParser;
    LoaderCI.pStreamFactory = pStreamFactory;

    RefCntAutoPtr<IRenderStateNotationLoader> pLoader;
    CreateRenderStateNotationLoader(LoaderCI, &pLoader);
    ASSERT_NE(pLoader, nullptr);

    LoadPipelineStateInfo PipelineLI{};
    PipelineLI.Name         = "GeometryOpaque";
    PipelineLI.PipelineType = PIPELINE_TYPE_GRAPHICS;
    PipelineLI.AddToCache   = true;

    RefCntAutoPtr<IPipelineState> pPSO;
    pLoader->LoadPipelineState(PipelineLI, &pPSO);
    ASSERT_NE(pPSO, nullptr);

    PipelineStateDesc PipelineStateDescReference{};
    PipelineStateDescReference.Name         = "GeometryOpaque";
    PipelineStateDescReference.PipelineType = PIPELINE_TYPE_GRAPHICS;
    EXPECT_EQ(PipelineStateDescReference, pPSO->GetDesc());

    const auto GraphicsDescReference = GetGraphicsPipelineRefDesc();
    EXPECT_EQ(GraphicsDescReference, pPSO->GetGraphicsPipelineDesc());
}


TEST(Tools_RenderStateNotationLoader, ResourceSignature)
{
    auto* pEnvironment = GPUTestingEnvironment::GetInstance();
    ASSERT_NE(pEnvironment, nullptr);

    auto* pDevice        = pEnvironment->GetDevice();
    auto  pParser        = CreateParser("PSO_Sign.json");
    auto  pStreamFactory = CreateShaderFactory();

    RenderStateNotationLoaderCreateInfo LoaderCI;
    LoaderCI.pDevice        = pDevice;
    LoaderCI.pParser        = pParser;
    LoaderCI.pStreamFactory = pStreamFactory;

    RefCntAutoPtr<IRenderStateNotationLoader> pLoader;
    CreateRenderStateNotationLoader(LoaderCI, &pLoader);
    ASSERT_NE(pLoader, nullptr);

    {
        LoadShaderInfo ShaderLI;
        ShaderLI.Name       = "GeometryOpaque-VS";
        ShaderLI.AddToCache = true;

        RefCntAutoPtr<IShader> pVS;
        pLoader->LoadShader(ShaderLI, &pVS);
        ASSERT_NE(pVS, nullptr);
        const auto& Desc = pVS->GetDesc();
        EXPECT_STREQ(Desc.Name, ShaderLI.Name);
        EXPECT_EQ(Desc.ShaderType, SHADER_TYPE_VERTEX);
    }

    {
        LoadShaderInfo ShaderLI;
        ShaderLI.Name       = "GeometryOpaque-PS";
        ShaderLI.AddToCache = true;

        RefCntAutoPtr<IShader> pPS;
        pLoader->LoadShader(ShaderLI, &pPS);
        ASSERT_NE(pPS, nullptr);
        const auto& Desc = pPS->GetDesc();
        EXPECT_STREQ(Desc.Name, ShaderLI.Name);
        EXPECT_EQ(Desc.ShaderType, SHADER_TYPE_PIXEL);
    }

    RefCntAutoPtr<IPipelineResourceSignature> pPRS;
    {
        LoadResourceSignatureInfo SignLI;
        SignLI.Name       = "TestSignature";
        SignLI.AddToCache = true;

        pLoader->LoadResourceSignature(SignLI, &pPRS);
        ASSERT_NE(pPRS, nullptr);
    }

    RefCntAutoPtr<IRenderPass> pRenderPass;
    {
        LoadRenderPassInfo RenderPassLI;
        RenderPassLI.Name       = "TestRenderPass";
        RenderPassLI.AddToCache = true;

        pLoader->LoadRenderPass(RenderPassLI, &pRenderPass);
        ASSERT_NE(pRenderPass, nullptr);
        const auto& Desc = pRenderPass->GetDesc();
        EXPECT_EQ(Desc.AttachmentCount, 2u);
        EXPECT_EQ(Desc.SubpassCount, 1u);
    }

    {
        LoadPipelineStateInfo PipelineLI;
        PipelineLI.Name         = "GeometryOpaque";
        PipelineLI.PipelineType = PIPELINE_TYPE_GRAPHICS;
        PipelineLI.AddToCache   = true;

        RefCntAutoPtr<IPipelineState> pPSO;
        pLoader->LoadPipelineState(PipelineLI, &pPSO);
        ASSERT_NE(pPSO, nullptr);

        PipelineStateDesc PipelineStateDescReference{};
        PipelineStateDescReference.Name         = "GeometryOpaque";
        PipelineStateDescReference.PipelineType = PIPELINE_TYPE_GRAPHICS;
        EXPECT_EQ(PipelineStateDescReference, pPSO->GetDesc());

        EXPECT_EQ(pPSO->GetResourceSignatureCount(), 1u);
        EXPECT_EQ(pPSO->GetResourceSignature(0), pPRS);

        GraphicsPipelineDesc GraphDescReference;
        GraphDescReference.NumRenderTargets  = 1;
        GraphDescReference.RTVFormats[0]     = TEX_FORMAT_RGBA8_UNORM_SRGB;
        GraphDescReference.DSVFormat         = TEX_FORMAT_D32_FLOAT;
        GraphDescReference.PrimitiveTopology = PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        GraphDescReference.pRenderPass       = pRenderPass;

        const auto& GraphDesc = pPSO->GetGraphicsPipelineDesc();
        EXPECT_EQ(GraphDesc, GraphDescReference);
        EXPECT_EQ(GraphDesc.pRenderPass, pRenderPass);
    }
}

TEST(Tools_RenderStateNotationLoader, Reload)
{
    auto* pEnvironment = GPUTestingEnvironment::GetInstance();
    ASSERT_NE(pEnvironment, nullptr);

    auto* pDevice = pEnvironment->GetDevice();

    auto pShaderFactory       = CreateShaderFactory("Shaders");
    auto pShaderReloadFactory = CreateShaderFactory("Shaders/Reload");
    auto pStatesFactory       = CreateShaderFactory("RenderStates");
    auto pStateReloadFactory  = CreateShaderFactory("RenderStates/Reload");
    ASSERT_TRUE(pShaderFactory);
    ASSERT_TRUE(pShaderReloadFactory);
    ASSERT_TRUE(pStatesFactory);
    ASSERT_TRUE(pStateReloadFactory);

    RefCntAutoPtr<IRenderStateNotationParser> pParser;
    CreateRenderStateNotationParser({true}, &pParser);
    ASSERT_TRUE(pParser);

    pParser->ParseFile("PSO.json", pStatesFactory, pStateReloadFactory);

    RenderStateCacheCreateInfo CacheCI;
    CacheCI.pDevice         = pDevice;
    CacheCI.LogLevel        = RENDER_STATE_CACHE_LOG_LEVEL_VERBOSE;
    CacheCI.EnableHotReload = true;
    CacheCI.pReloadSource   = pShaderReloadFactory;
    RefCntAutoPtr<IRenderStateCache> pStateCache;
    CreateRenderStateCache(CacheCI, &pStateCache);
    ASSERT_TRUE(pStateCache);

    RenderStateNotationLoaderCreateInfo LoaderCI{};
    LoaderCI.pDevice        = pDevice;
    LoaderCI.pParser        = pParser;
    LoaderCI.pStreamFactory = pShaderFactory;
    LoaderCI.pStateCache    = pStateCache;

    RefCntAutoPtr<IRenderStateNotationLoader> pLoader;
    CreateRenderStateNotationLoader(LoaderCI, &pLoader);
    ASSERT_NE(pLoader, nullptr);

    LoadPipelineStateInfo PipelineLI{};
    PipelineLI.Name         = "GeometryOpaque";
    PipelineLI.PipelineType = PIPELINE_TYPE_GRAPHICS;
    PipelineLI.AddToCache   = true;

    RefCntAutoPtr<IPipelineState> pPSO;
    pLoader->LoadPipelineState(PipelineLI, &pPSO);
    ASSERT_NE(pPSO, nullptr);

    {
        PipelineStateDesc PipelineStateDescReference{};
        PipelineStateDescReference.Name         = "GeometryOpaque";
        PipelineStateDescReference.PipelineType = PIPELINE_TYPE_GRAPHICS;
        EXPECT_EQ(PipelineStateDescReference, pPSO->GetDesc());

        const auto GraphicsDescReference = GetGraphicsPipelineRefDesc();
        EXPECT_EQ(GraphicsDescReference, pPSO->GetGraphicsPipelineDesc());
    }

    EXPECT_TRUE(pLoader->Reload());

    {
        PipelineStateDesc PipelineStateDescReference;
        PipelineStateDescReference.Name         = "GeometryOpaque";
        PipelineStateDescReference.PipelineType = PIPELINE_TYPE_GRAPHICS;
        EXPECT_EQ(PipelineStateDescReference, pPSO->GetDesc());

        GraphicsPipelineDesc GraphicsDescReference;
        GraphicsDescReference.DepthStencilDesc.DepthEnable      = true;
        GraphicsDescReference.DepthStencilDesc.DepthWriteEnable = true;
        GraphicsDescReference.DepthStencilDesc.DepthFunc        = COMPARISON_FUNC_LESS_EQUAL;

        GraphicsDescReference.RasterizerDesc.FillMode              = FILL_MODE_SOLID;
        GraphicsDescReference.RasterizerDesc.CullMode              = CULL_MODE_NONE;
        GraphicsDescReference.RasterizerDesc.FrontCounterClockwise = true;
        GraphicsDescReference.RasterizerDesc.DepthClipEnable       = true;

        GraphicsDescReference.NumRenderTargets  = 2;
        GraphicsDescReference.RTVFormats[0]     = TEX_FORMAT_RGBA8_UNORM_SRGB;
        GraphicsDescReference.RTVFormats[1]     = TEX_FORMAT_RGBA32_FLOAT;
        GraphicsDescReference.DSVFormat         = TEX_FORMAT_D32_FLOAT;
        GraphicsDescReference.PrimitiveTopology = PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
        EXPECT_EQ(GraphicsDescReference, pPSO->GetGraphicsPipelineDesc());
    }
}

} // namespace
