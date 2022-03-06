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

TEST(Tools_RenderStateNotationParser, ParseRenderPassEnums)
{
    DynamicLinearAllocator Allocator{DefaultRawMemoryAllocator::GetAllocator()};

    ASSERT_TRUE(TestEnum<ATTACHMENT_LOAD_OP>(Allocator, ATTACHMENT_LOAD_OP_LOAD, ATTACHMENT_LOAD_OP_DISCARD));

    ASSERT_TRUE(TestEnum<ATTACHMENT_STORE_OP>(Allocator, ATTACHMENT_STORE_OP_STORE, ATTACHMENT_STORE_OP_DISCARD));
}

TEST(Tools_RenderStateNotationParser, ParseRenderPassAttachmentDesc)
{
    CHECK_STRUCT_SIZE(RenderPassAttachmentDesc, 16);

    DynamicLinearAllocator Allocator{DefaultRawMemoryAllocator::GetAllocator()};

    nlohmann::json JsonReference = LoadDRSNFromFile("RenderStates/RenderPass/RenderPassAttachmentDesc.json");

    RenderPassAttachmentDesc DescReference{};
    DescReference.Format         = TEX_FORMAT_RGBA8_UNORM;
    DescReference.SampleCount    = 4;
    DescReference.LoadOp         = ATTACHMENT_LOAD_OP_CLEAR;
    DescReference.StoreOp        = ATTACHMENT_STORE_OP_DISCARD;
    DescReference.StencilLoadOp  = ATTACHMENT_LOAD_OP_LOAD;
    DescReference.StencilStoreOp = ATTACHMENT_STORE_OP_STORE;
    DescReference.InitialState   = RESOURCE_STATE_SHADER_RESOURCE;
    DescReference.FinalState     = RESOURCE_STATE_RENDER_TARGET;

    RenderPassAttachmentDesc Desc{};
    ParseRSN(JsonReference, Desc, Allocator);
    ASSERT_EQ(Desc, DescReference);
}

TEST(Tools_RenderStateNotationParser, ParseAttachmentReference)
{
    CHECK_STRUCT_SIZE(AttachmentReference, 8);

    DynamicLinearAllocator Allocator{DefaultRawMemoryAllocator::GetAllocator()};

    nlohmann::json JsonReference = LoadDRSNFromFile("RenderStates/RenderPass/AttachmentReference.json");

    AttachmentReference DescReference{};
    DescReference.AttachmentIndex = 1;
    DescReference.State           = RESOURCE_STATE_RENDER_TARGET;

    AttachmentReference Desc{};
    ParseRSN(JsonReference, Desc, Allocator);
    ASSERT_EQ(Desc, DescReference);
}

TEST(Tools_RenderStateNotationParser, ParseShadingRateAttachment)
{
    CHECK_STRUCT_SIZE(ShadingRateAttachment, 16);

    DynamicLinearAllocator Allocator{DefaultRawMemoryAllocator::GetAllocator()};

    nlohmann::json JsonReference = LoadDRSNFromFile("RenderStates/RenderPass/ShadingRateAttachment.json");

    ShadingRateAttachment DescReference{};
    DescReference.Attachment.AttachmentIndex = 0;
    DescReference.Attachment.State           = RESOURCE_STATE_SHADING_RATE;
    DescReference.TileSize[0]                = 8;
    DescReference.TileSize[1]                = 16;

    ShadingRateAttachment Desc{};
    ParseRSN(JsonReference, Desc, Allocator);
    ASSERT_EQ(Desc, DescReference);
}

TEST(Tools_RenderStateNotationParser, ParseSubpassDesc)
{
    CHECK_STRUCT_SIZE(SubpassDesc, 72);

    DynamicLinearAllocator Allocator{DefaultRawMemoryAllocator::GetAllocator()};

    nlohmann::json JsonReference = LoadDRSNFromFile("RenderStates/RenderPass/SubpassDesc.json");

    constexpr AttachmentReference InputAttachments[] = {
        {0, RESOURCE_STATE_INPUT_ATTACHMENT},
        {1, RESOURCE_STATE_INPUT_ATTACHMENT}};

    constexpr AttachmentReference RenderTargetAttachments[] = {
        {2, RESOURCE_STATE_RENDER_TARGET}};

    constexpr AttachmentReference DepthTargetAttachment[] = {
        {2, RESOURCE_STATE_DEPTH_WRITE}};

    constexpr AttachmentReference ResolveAttachments[] = {
        {2, RESOURCE_STATE_RESOLVE_SOURCE}};

    constexpr ShadingRateAttachment ShadingRateAttachment[] = {
        {{3, RESOURCE_STATE_SHADING_RATE}, 4, 8}};

    constexpr Uint32 PreserveAttachments[] = {2, 4};

    SubpassDesc DescReference{};
    DescReference.InputAttachmentCount        = _countof(InputAttachments);
    DescReference.pInputAttachments           = InputAttachments;
    DescReference.RenderTargetAttachmentCount = _countof(RenderTargetAttachments);
    DescReference.pRenderTargetAttachments    = RenderTargetAttachments;
    DescReference.pResolveAttachments         = ResolveAttachments;
    DescReference.pDepthStencilAttachment     = DepthTargetAttachment;
    DescReference.PreserveAttachmentCount     = _countof(PreserveAttachments);
    DescReference.pPreserveAttachments        = PreserveAttachments;
    DescReference.pShadingRateAttachment      = ShadingRateAttachment;

    SubpassDesc Desc{};
    ParseRSN(JsonReference, Desc, Allocator);
    ASSERT_EQ(Desc, DescReference);
}

TEST(Tools_RenderStateNotationParser, ParseSubpassDependencyDesc)
{
    CHECK_STRUCT_SIZE(SubpassDependencyDesc, 24);

    DynamicLinearAllocator Allocator{DefaultRawMemoryAllocator::GetAllocator()};

    nlohmann::json JsonReference = LoadDRSNFromFile("RenderStates/RenderPass/SubpassDependencyDesc.json");

    SubpassDependencyDesc DescReference{};
    DescReference.SrcSubpass    = 0;
    DescReference.DstSubpass    = 1;
    DescReference.SrcAccessMask = ACCESS_FLAG_MEMORY_READ | ACCESS_FLAG_MEMORY_WRITE;
    DescReference.SrcStageMask  = PIPELINE_STAGE_FLAG_BOTTOM_OF_PIPE;
    DescReference.DstAccessMask = ACCESS_FLAG_MEMORY_READ;
    DescReference.DstStageMask  = PIPELINE_STAGE_FLAG_EARLY_FRAGMENT_TESTS | PIPELINE_STAGE_FLAG_PIXEL_SHADER;

    SubpassDependencyDesc Desc{};
    ParseRSN(JsonReference, Desc, Allocator);
    ASSERT_EQ(Desc, DescReference);
}

TEST(Tools_RenderStateNotationParser, ParseRenderPassDesc)
{
    CHECK_STRUCT_SIZE(RenderPassDesc, 56);

    DynamicLinearAllocator Allocator{DefaultRawMemoryAllocator::GetAllocator()};

    nlohmann::json JsonReference = LoadDRSNFromFile("RenderStates/RenderPass/RenderPassDesc.json");

    RenderPassAttachmentDesc Attachments[4]{};
    Attachments[0].Format = TEX_FORMAT_RGBA8_UNORM;
    Attachments[1].Format = TEX_FORMAT_R32_FLOAT;
    Attachments[2].Format = TEX_FORMAT_D32_FLOAT;
    Attachments[3].Format = TEX_FORMAT_RGBA8_UNORM;

    constexpr AttachmentReference RTAttachmentRefs0[] = {
        {0, RESOURCE_STATE_RENDER_TARGET},
        {1, RESOURCE_STATE_RENDER_TARGET}};

    constexpr AttachmentReference DepthAttachmentRef0[] = {
        {2, RESOURCE_STATE_DEPTH_WRITE}};

    constexpr AttachmentReference RTAttachmentRefs1[] = {
        {3, RESOURCE_STATE_RENDER_TARGET}};

    constexpr AttachmentReference DepthAttachmentRef1[] = {
        {2, RESOURCE_STATE_DEPTH_WRITE}};

    constexpr AttachmentReference InputAttachmentRefs1[] = {
        {0, RESOURCE_STATE_INPUT_ATTACHMENT},
        {1, RESOURCE_STATE_INPUT_ATTACHMENT}};

    SubpassDesc Subpasses[2]{};
    Subpasses[0].RenderTargetAttachmentCount = _countof(RTAttachmentRefs0);
    Subpasses[0].pRenderTargetAttachments    = RTAttachmentRefs0;
    Subpasses[0].pDepthStencilAttachment     = DepthAttachmentRef0;

    Subpasses[1].RenderTargetAttachmentCount = _countof(RTAttachmentRefs1);
    Subpasses[1].pRenderTargetAttachments    = RTAttachmentRefs1;
    Subpasses[1].pDepthStencilAttachment     = DepthAttachmentRef1;
    Subpasses[1].InputAttachmentCount        = _countof(InputAttachmentRefs1);
    Subpasses[1].pInputAttachments           = InputAttachmentRefs1;

    SubpassDependencyDesc Dependencies[1]{};
    Dependencies[0].SrcSubpass = 0;
    Dependencies[0].DstSubpass = 1;

    RenderPassDesc DescReference{};
    DescReference.Name            = "TestName";
    DescReference.AttachmentCount = _countof(Attachments);
    DescReference.pAttachments    = Attachments;
    DescReference.SubpassCount    = _countof(Subpasses);
    DescReference.pSubpasses      = Subpasses;
    DescReference.DependencyCount = _countof(Dependencies);
    DescReference.pDependencies   = Dependencies;

    RenderPassDesc Desc{};
    ParseRSN(JsonReference, Desc, Allocator);
    ASSERT_EQ(Desc, DescReference);
}

} // namespace
