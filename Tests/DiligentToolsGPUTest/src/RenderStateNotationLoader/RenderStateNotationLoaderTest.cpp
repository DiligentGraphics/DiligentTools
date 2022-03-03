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

RefCntAutoPtr<IShaderSourceInputStreamFactory> CreateShaderFactory()
{
    RefCntAutoPtr<IShaderSourceInputStreamFactory> pStreamFactory;
    CreateDefaultShaderSourceStreamFactory("Shaders", &pStreamFactory);
    return pStreamFactory;
}

TEST(Tools_RenderStateNotationLoader, BasicTest)
{
    auto* pEnvironment = GPUTestingEnvironment::GetInstance();
    ASSERT_NE(pEnvironment, nullptr);

    auto* pDevice        = pEnvironment->GetDevice();
    auto  pParser        = CreateParser("GeometryOpaque.json");
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

    GraphicsPipelineDesc GraphicsPipelineDescReference{};
    GraphicsPipelineDescReference.DepthStencilDesc.DepthEnable      = true;
    GraphicsPipelineDescReference.DepthStencilDesc.DepthWriteEnable = true;
    GraphicsPipelineDescReference.DepthStencilDesc.DepthFunc        = COMPARISON_FUNC_LESS;

    GraphicsPipelineDescReference.RasterizerDesc.FillMode              = FILL_MODE_SOLID;
    GraphicsPipelineDescReference.RasterizerDesc.CullMode              = CULL_MODE_BACK;
    GraphicsPipelineDescReference.RasterizerDesc.FrontCounterClockwise = true;
    GraphicsPipelineDescReference.RasterizerDesc.DepthClipEnable       = true;

    GraphicsPipelineDescReference.NumRenderTargets  = 1;
    GraphicsPipelineDescReference.RTVFormats[0]     = TEX_FORMAT_RGBA8_UNORM_SRGB;
    GraphicsPipelineDescReference.DSVFormat         = TEX_FORMAT_D32_FLOAT;
    GraphicsPipelineDescReference.PrimitiveTopology = PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    EXPECT_EQ(GraphicsPipelineDescReference, pPSO->GetGraphicsPipelineDesc());
}

} // namespace
