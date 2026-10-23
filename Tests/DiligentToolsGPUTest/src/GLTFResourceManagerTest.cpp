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

#include <array>
#include <atomic>
#include <limits>
#include <thread>

#include "gtest/gtest.h"
#include "GLTFResourceManager.hpp"
#include "GraphicsAccessories.hpp"
#include "GPUTestingEnvironment.hpp"
#include "ThreadSignal.hpp"

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

TEST(Tools_GLTFResourceManager, RejectsIndexAllocationsExceedingBufferCapacity)
{
    struct TestCase
    {
        Uint64 InitialSize;
        Uint64 MaxSize;
        Uint32 Size;
        Uint32 Alignment;
    };
    constexpr Uint32 MaxSize32 = (std::numeric_limits<Uint32>::max)();
    const TestCase   Cases[]   = {
        {16, 16, 24, 4},               // The request exceeds every allocator's capacity.
        {16, 0, 24, 4},                // A zero maximum keeps the initial capacity.
        {8, 15, 13, 4},                // Alignment rounds the request up to 16 bytes.
        {16, MaxSize32, MaxSize32, 4}, // Rounding must not overflow Uint32.
    };

    for (const TestCase& Case : Cases)
    {
        SCOPED_TRACE(::testing::Message{} << "initial " << Case.InitialSize << ", maximum " << Case.MaxSize
                                          << ", size " << Case.Size << ", alignment " << Case.Alignment);
        GLTF::ResourceManager::CreateInfo CI = MakeResourceManagerCI();
        CI.IndexAllocatorCI.Desc.Size        = Case.InitialSize;
        CI.IndexAllocatorCI.MaxSize          = Case.MaxSize;
        const RefCntAutoPtr<GLTF::ResourceManager> pManager =
            GLTF::ResourceManager::Create(GPUTestingEnvironment::GetInstance()->GetDevice(), CI);
        ASSERT_NE(pManager, nullptr);
        ASSERT_EQ(pManager->GetIndexBufferCount(), 1u);

        {
            TestingEnvironment::ErrorScope ExpectedErrors{"exceeds maximum buffer size"};
            EXPECT_EQ(pManager->AllocateIndices(Case.Size, Case.Alignment), nullptr);
        }
        EXPECT_EQ(pManager->GetIndexBufferCount(), 1u);

        // Rejection leaves the existing allocator available for valid requests.
        const RefCntAutoPtr<IBufferSuballocation> pAllocation = pManager->AllocateIndices(4, 4);
        ASSERT_NE(pAllocation, nullptr);
        EXPECT_EQ(pAllocation->GetOffset(), 0u);
        EXPECT_EQ(pManager->GetIndexBufferCount(), 1u);
    }
}

TEST(Tools_GLTFResourceManager, RejectsInvalidIndexAllocationArguments)
{
    GLTF::ResourceManager::CreateInfo CI = MakeResourceManagerCI();
    CI.IndexAllocatorCI.Desc.Size        = 16;
    const RefCntAutoPtr<GLTF::ResourceManager> pManager =
        GLTF::ResourceManager::Create(GPUTestingEnvironment::GetInstance()->GetDevice(), CI);
    ASSERT_NE(pManager, nullptr);

    struct TestCase
    {
        Uint32 Size;
        Uint32 Alignment;
    };
    const TestCase Cases[] = {{0, 4}, {4, 0}, {4, 3}};
    for (const TestCase& Case : Cases)
    {
        SCOPED_TRACE(::testing::Message{} << "size " << Case.Size << ", alignment " << Case.Alignment);
        TestingEnvironment::ErrorScope ExpectedErrors{"Index allocation size must be non-zero and alignment must be a power of two"};
        EXPECT_EQ(pManager->AllocateIndices(Case.Size, Case.Alignment), nullptr);
        EXPECT_EQ(pManager->GetIndexBufferCount(), 1u);
    }
}

TEST(Tools_GLTFResourceManager, SpillsIndexAllocationsIntoNewBuffers)
{
    GLTF::ResourceManager::CreateInfo CI = MakeResourceManagerCI();
    CI.IndexAllocatorCI.Desc.Size        = 8;
    CI.IndexAllocatorCI.MaxSize          = 16;
    const RefCntAutoPtr<GLTF::ResourceManager> pManager =
        GLTF::ResourceManager::Create(GPUTestingEnvironment::GetInstance()->GetDevice(), CI);
    ASSERT_NE(pManager, nullptr);

    // Each request fits exactly after growth, but the second requires a new pool.
    const RefCntAutoPtr<IBufferSuballocation> pFirst = pManager->AllocateIndices(16, 4);
    ASSERT_NE(pFirst, nullptr);
    EXPECT_EQ(pManager->GetIndexBufferCount(), 1u);
    const RefCntAutoPtr<IBufferSuballocation> pSecond = pManager->AllocateIndices(16, 4);
    ASSERT_NE(pSecond, nullptr);
    EXPECT_EQ(pManager->GetIndexBufferCount(), 2u);
    EXPECT_NE(pFirst->GetAllocator(), pSecond->GetAllocator());
    EXPECT_EQ(pFirst->GetOffset(), 0u);
    EXPECT_EQ(pSecond->GetOffset(), 0u);
    EXPECT_EQ(pFirst->GetSize(), 16u);
    EXPECT_EQ(pSecond->GetSize(), 16u);
}

TEST(Tools_GLTFResourceManager, UpdatesAllIndexBuffers)
{
    GPUTestingEnvironment* const      pEnvironment      = GPUTestingEnvironment::GetInstance();
    IRenderDevice* const              pDevice           = pEnvironment->GetDevice();
    IDeviceContext* const             pContext          = pEnvironment->GetDeviceContext();
    GLTF::ResourceManager::CreateInfo CI                = MakeResourceManagerCI();
    CI.IndexAllocatorCI.Desc.Size                       = 16;
    CI.IndexAllocatorCI.MaxSize                         = 32;
    const RefCntAutoPtr<GLTF::ResourceManager> pManager = GLTF::ResourceManager::Create(pDevice, CI);
    ASSERT_NE(pManager, nullptr);

    // Fill the first allocator and force a second one whose GPU buffer has not
    // been created yet. Updating all resources must materialize both buffers.
    const RefCntAutoPtr<IBufferSuballocation> pFirst  = pManager->AllocateIndices(32, 4);
    const RefCntAutoPtr<IBufferSuballocation> pSecond = pManager->AllocateIndices(16, 4);
    ASSERT_NE(pFirst, nullptr);
    ASSERT_NE(pSecond, nullptr);
    ASSERT_EQ(pManager->GetIndexBufferCount(), 2u);
    EXPECT_EQ(pManager->GetIndexBuffer(1), nullptr);
    pManager->UpdateAllResources(pDevice, pContext);
    ASSERT_NE(pManager->GetIndexBuffer(0), nullptr);
    ASSERT_NE(pManager->GetIndexBuffer(1), nullptr);
    EXPECT_EQ(pManager->GetIndexBuffer(0)->GetDesc().Size, 32u);
    EXPECT_EQ(pManager->GetIndexBuffer(1)->GetDesc().Size, 16u);
    EXPECT_EQ(pSecond->GetBuffer(), pManager->GetIndexBuffer(1));

    // Grow the second allocator after its GPU buffer is created. The update path
    // must resize its GPU buffer without adding another allocator.
    const RefCntAutoPtr<IBufferSuballocation> pThird = pManager->AllocateIndices(16, 4);
    ASSERT_NE(pThird, nullptr);
    EXPECT_EQ(pThird->GetAllocator(), pSecond->GetAllocator());
    EXPECT_EQ(pManager->GetIndexBuffer(1)->GetDesc().Size, 16u);
    pManager->UpdateAllResources(pDevice, pContext);
    ASSERT_NE(pManager->GetIndexBuffer(1), nullptr);
    EXPECT_EQ(pManager->GetIndexBuffer(1)->GetDesc().Size, 32u);
    EXPECT_EQ(pThird->GetBuffer(), pManager->GetIndexBuffer(1));
    EXPECT_EQ(pManager->GetIndexBufferCount(), 2u);
}

TEST(Tools_GLTFResourceManager, SharesIndexBuffersAcrossConcurrentAllocations)
{
    GLTF::ResourceManager::CreateInfo CI = MakeResourceManagerCI();
    CI.IndexAllocatorCI.Desc.Size        = 16;
    const RefCntAutoPtr<GLTF::ResourceManager> pManager =
        GLTF::ResourceManager::Create(GPUTestingEnvironment::GetInstance()->GetDevice(), CI);
    ASSERT_NE(pManager, nullptr);

    constexpr Uint32                                             WorkerCount = 16;
    std::array<RefCntAutoPtr<IBufferSuballocation>, WorkerCount> Allocations;
    std::array<std::thread, WorkerCount>                         Workers;
    std::atomic<Uint32>                                          ReadyCount{0};
    Threading::Signal                                            AllReady;
    Threading::Signal                                            Start;
    for (Uint32 WorkerIndex = 0; WorkerIndex < WorkerCount; ++WorkerIndex)
    {
        Workers[WorkerIndex] = std::thread{[&, WorkerIndex] {
            if (ReadyCount.fetch_add(1) + 1 == WorkerCount)
                AllReady.Trigger();
            Start.Wait();
            Allocations[WorkerIndex] = pManager->AllocateIndices(4, 4);
        }};
    }
    AllReady.Wait();
    Start.Trigger(true);
    for (std::thread& Worker : Workers)
    {
        Worker.join();
    }

    // Retain every allocation: four allocations fill each buffer. Concurrent
    // requests must reuse newly published buffers instead of each adding one.
    EXPECT_EQ(pManager->GetIndexBufferCount(), WorkerCount / 4);
    for (Uint32 Index = 0; Index < WorkerCount; ++Index)
    {
        SCOPED_TRACE(Index);
        const RefCntAutoPtr<IBufferSuballocation>& pAllocation = Allocations[Index];
        ASSERT_NE(pAllocation, nullptr);
        EXPECT_EQ(pAllocation->GetSize(), 4u);
        EXPECT_EQ(pAllocation->GetOffset() % 4, 0u);
        EXPECT_LE(pAllocation->GetOffset(), 12u);
        for (Uint32 Previous = 0; Previous < Index; ++Previous)
        {
            if (pAllocation->GetAllocator() == Allocations[Previous]->GetAllocator())
            {
                EXPECT_NE(pAllocation->GetOffset(), Allocations[Previous]->GetOffset());
            }
        }
    }
}

TEST(Tools_GLTFResourceManager, StopsWhenFreshIndexAllocatorCannotSatisfyAlignment)
{
    GLTF::ResourceManager::CreateInfo CI = MakeResourceManagerCI();
    CI.IndexAllocatorCI.Desc.Size        = 4;
    CI.IndexAllocatorCI.MaxSize          = 16;
    const RefCntAutoPtr<GLTF::ResourceManager> pManager =
        GLTF::ResourceManager::Create(GPUTestingEnvironment::GetInstance()->GetDevice(), CI);
    ASSERT_NE(pManager, nullptr);

    // The underlying allocator retains its initial alignment estimate when it
    // grows. This request fits the byte limit, but its extra alignment reserve
    // cannot fit even in a fresh allocator. Do not keep creating identical pools.
    {
        TestingEnvironment::ErrorScope ExpectedErrors{"in a new index buffer"};
        EXPECT_EQ(pManager->AllocateIndices(16, 16), nullptr);
    }
    EXPECT_EQ(pManager->GetIndexBufferCount(), 1u);
    EXPECT_NE(pManager->AllocateIndices(16, 4), nullptr);
}

} // namespace
