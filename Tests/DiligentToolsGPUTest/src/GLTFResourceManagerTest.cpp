/*
 *  Copyright 2026 Diligent Graphics LLC
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
#include "GLTFResourceManager.hpp"
#include "GraphicsAccessories.hpp"
#include "GPUTestingEnvironment.hpp"

using namespace Diligent;
using namespace Diligent::Testing;

namespace
{

GLTF::ResourceManager::CreateInfo MakeResourceManagerCI()
{
    GLTF::ResourceManager::CreateInfo CI;
    CI.IndexAllocatorCI.Desc.Size      = 1024;
    CI.IndexAllocatorCI.Desc.BindFlags = BIND_INDEX_BUFFER;
    CI.DefaultAtlasDesc.Desc.Type      = RESOURCE_DIM_TEX_2D_ARRAY;
    CI.DefaultAtlasDesc.Desc.Width     = 512;
    CI.DefaultAtlasDesc.Desc.Height    = 512;
    CI.DefaultAtlasDesc.Desc.ArraySize = 0;
    CI.DefaultAtlasDesc.Desc.MipLevels = 0;
    CI.DefaultAtlasDesc.MinAlignment   = 0;
    CI.DefaultAtlasDesc.MaxSliceCount  = 1;
    return CI;
}

TEST(Tools_GLTFResourceManager, DefaultAtlasMipLevel0Size)
{
    struct TestCase
    {
        TEXTURE_FORMAT Format;
        Uint32         Width;
        Uint32         Height;
        Uint64         Budget;
        Uint32         ExpectedWidth;
        Uint32         ExpectedHeight;
    };
    const TestCase Cases[] = {
        {TEX_FORMAT_RGBA8_UNORM, 1512, 760, 0, 1512, 760},              // Unset limit.
        {TEX_FORMAT_RGBA8_UNORM, 1512, 760, 1512 * 760 * 4, 1512, 760}, // Already fits.
        {TEX_FORMAT_RGBA8_UNORM, 1512, 760, 756 * 380 * 4, 756, 380},   // No power-of-two requirement.
        {TEX_FORMAT_R8_UNORM, 512, 512, 128 * 1024, 256, 256},
        {TEX_FORMAT_RGBA8_UNORM, 512, 512, 128 * 1024, 128, 128},
        {TEX_FORMAT_RGBA16_FLOAT, 512, 512, 128 * 1024, 128, 128},
        {TEX_FORMAT_RGBA32_FLOAT, 512, 512, 128 * 1024, 64, 64},
        {TEX_FORMAT_BC1_UNORM, 512, 512, 128 * 1024, 512, 512},
        {TEX_FORMAT_BC3_UNORM, 512, 512, 128 * 1024, 256, 256},
        {TEX_FORMAT_RGBA8_UNORM, 1024, 256, 128 * 1024, 256, 64}, // Both dimensions halve.
        {TEX_FORMAT_RGBA8_UNORM, 512, 512, 1, 16, 16},            // Minimum overrides budget.
        {TEX_FORMAT_RGBA8_UNORM, 1024, 16, 1, 16, 16},            // One dimension reaches the floor first.
    };

    auto* pEnvironment = GPUTestingEnvironment::GetInstance();
    for (const auto& Test : Cases)
    {
        SCOPED_TRACE(::testing::Message{} << Test.Format << ": " << Test.Width << "x" << Test.Height << ", budget " << Test.Budget);
        auto CI                         = MakeResourceManagerCI();
        CI.DefaultAtlasDesc.Desc.Width  = Test.Width;
        CI.DefaultAtlasDesc.Desc.Height = Test.Height;
        CI.DefaultAtlasMipLevel0Size    = Test.Budget;
        auto pManager                   = GLTF::ResourceManager::Create(pEnvironment->GetDevice(), CI);
        ASSERT_NE(pManager, nullptr);

        const TextureDesc Desc = pManager->GetAtlasDesc(Test.Format);
        EXPECT_EQ(Desc.Width, Test.ExpectedWidth);
        EXPECT_EQ(Desc.Height, Test.ExpectedHeight);
        EXPECT_EQ(Desc.MipLevels, ComputeMipLevelsCount(Test.ExpectedWidth, Test.ExpectedHeight));

        auto pAllocation = pManager->AllocateTextureSpace(Test.Format, 4, 4);
        ASSERT_NE(pAllocation, nullptr);
        EXPECT_EQ(pAllocation->GetAlignment(), 1u);
        auto* pTexture = pAllocation->GetAtlas()->Update(pEnvironment->GetDevice(), pEnvironment->GetDeviceContext());
        ASSERT_NE(pTexture, nullptr);
        EXPECT_EQ(pTexture->GetDesc().Width, Desc.Width);
        EXPECT_EQ(pTexture->GetDesc().Height, Desc.Height);
        EXPECT_EQ(pTexture->GetDesc().MipLevels, Desc.MipLevels);
    }
}

TEST(Tools_GLTFResourceManager, SizeLimitPreservesOtherDefaultSettings)
{
    auto CI                            = MakeResourceManagerCI();
    CI.DefaultAtlasDesc.MinAlignment   = 64;
    CI.DefaultAtlasDesc.Desc.MipLevels = 3;
    CI.DefaultAtlasMipLevel0Size       = 128 * 1024;
    auto pManager                      = GLTF::ResourceManager::Create(GPUTestingEnvironment::GetInstance()->GetDevice(), CI);
    ASSERT_NE(pManager, nullptr);

    auto ExpectedDesc   = CI.DefaultAtlasDesc.Desc;
    ExpectedDesc.Format = TEX_FORMAT_RGBA8_UNORM;
    ExpectedDesc.Width  = 128;
    ExpectedDesc.Height = 128;
    EXPECT_EQ(pManager->GetAtlasDesc(TEX_FORMAT_RGBA8_UNORM), ExpectedDesc);
    EXPECT_EQ(pManager->GetAllocationAlignment(TEX_FORMAT_RGBA8_UNORM, 4, 4), 64u);
    EXPECT_EQ(pManager->GetAtlasDesc(TEX_FORMAT_UNKNOWN), CI.DefaultAtlasDesc.Desc);

    auto pAllocation = pManager->AllocateTextureSpace(TEX_FORMAT_RGBA8_UNORM, 4, 4);
    ASSERT_NE(pAllocation, nullptr);
    EXPECT_EQ(pAllocation->GetAlignment(), 64u);
    EXPECT_EQ(pAllocation->GetAtlas()->GetAtlasDesc().MipLevels, 3u);
}

TEST(Tools_GLTFResourceManager, SizeLimitDoesNotAffectExplicitAtlases)
{
    auto CI                      = MakeResourceManagerCI();
    auto AtlasCI                 = CI.DefaultAtlasDesc;
    AtlasCI.Desc.Format          = TEX_FORMAT_RGBA8_UNORM;
    AtlasCI.Desc.Width           = 128;
    AtlasCI.Desc.Height          = 64;
    AtlasCI.Desc.MipLevels       = 3;
    CI.pTexAtlasCIs              = &AtlasCI;
    CI.NumTexAtlases             = 1;
    CI.DefaultAtlasMipLevel0Size = 1;

    auto pManager = GLTF::ResourceManager::Create(GPUTestingEnvironment::GetInstance()->GetDevice(), CI);
    ASSERT_NE(pManager, nullptr);
    EXPECT_EQ(pManager->GetAtlasDesc(TEX_FORMAT_RGBA8_UNORM), AtlasCI.Desc);
}

} // namespace
