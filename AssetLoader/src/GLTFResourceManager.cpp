/*
 *  Copyright 2019-2020 Diligent Graphics LLC
 *  Copyright 2015-2019 Egor Yusov
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

#include "GLTFResourceManager.hpp"
#include "DefaultRawMemoryAllocator.hpp"
#include "Align.hpp"

namespace Diligent
{

GLTFResourceManager::BufferCache::BufferCache(IRenderDevice* pDevice, const BufferDesc& BuffDesc) :
    Mgr{BuffDesc.uiSizeInBytes, DefaultRawMemoryAllocator::GetAllocator()}
{
    pDevice->CreateBuffer(BuffDesc, nullptr, &pBuffer);
}

GLTFResourceManager::TextureCache::TextureCache(IRenderDevice* pDevice, const TextureCacheAttribs& CacheCI) :
    Mgr{CacheCI.Desc.Width / CacheCI.Granularity, CacheCI.Desc.Height / CacheCI.Granularity},
    Granularity{CacheCI.Granularity}
{
    DEV_CHECK_ERR(IsPowerOfTwo(CacheCI.Granularity), "Granularity (", CacheCI.Granularity, ") must be a power of two");
    DEV_CHECK_ERR((CacheCI.Desc.Width % CacheCI.Granularity) == 0, "Atlas width (", CacheCI.Desc.Width, ") is not multiple of granularity (", CacheCI.Granularity, ")");
    DEV_CHECK_ERR((CacheCI.Desc.Height % CacheCI.Granularity) == 0, "Atlas height (", CacheCI.Desc.Height, ") is not multiple of granularity (", CacheCI.Granularity, ")");

    pDevice->CreateTexture(CacheCI.Desc, nullptr, &pTexture);
}

RefCntAutoPtr<GLTFResourceManager> GLTFResourceManager::Create(IRenderDevice*    pDevice,
                                                               const CreateInfo& CI)
{
    return RefCntAutoPtr<GLTFResourceManager>{MakeNewRCObj<GLTFResourceManager>()(pDevice, CI)};
}

GLTFResourceManager::GLTFResourceManager(IReferenceCounters* pRefCounters,
                                         IRenderDevice*      pDevice,
                                         const CreateInfo&   CI) :
    TBase{pRefCounters}
{
    m_Buffers.reserve(CI.NumBuffers);
    for (Uint32 i = 0; i < CI.NumBuffers; ++i)
    {
        m_Buffers.emplace_back(pDevice, CI.Buffers[i]);
    }

    m_Textures.reserve(CI.NumTextures);
    for (Uint32 i = 0; i < CI.NumTextures; ++i)
    {
        m_Textures.emplace_back(pDevice, CI.Textures[i]);
    }
}

GLTFResourceManager::BufferAllocation GLTFResourceManager::AllocateBufferSpace(Uint32 BufferIndex, Uint32 Size, Uint32 Alignment)
{
    auto& BuffCache = m_Buffers[BufferIndex];

    std::lock_guard<std::mutex> Lock{BuffCache.Mtx};

    BufferAllocation Allocation;
    Allocation.BufferIndex = BufferIndex;
    Allocation.Region      = BuffCache.Mgr.Allocate(Size, Alignment);
    return Allocation;
}

void GLTFResourceManager::FreeBufferSpace(BufferAllocation&& Allocation)
{
    auto& BuffCache = m_Buffers[Allocation.BufferIndex];

    std::lock_guard<std::mutex> Lock{BuffCache.Mtx};
    BuffCache.Mgr.Free(std::move(Allocation.Region));
}

GLTFResourceManager::TextureAllocation GLTFResourceManager::AllocateTextureSpace(Uint32 TextureIndex, Uint32 Width, Uint32 Height)
{
    auto& TexCache = m_Textures[TextureIndex];

    Width  = (Width + TexCache.Granularity - 1) / TexCache.Granularity;
    Height = (Height + TexCache.Granularity - 1) / TexCache.Granularity;

    std::lock_guard<std::mutex> Lock{TexCache.Mtx};

    TextureAllocation Allocation;
    Allocation.TextureIndex = TextureIndex;
    Allocation.Region       = TexCache.Mgr.Allocate(Width, Height);
    if (!Allocation.Region.IsEmpty())
    {
        Allocation.Region.x *= TexCache.Granularity;
        Allocation.Region.y *= TexCache.Granularity;
        Allocation.Region.width *= TexCache.Granularity;
        Allocation.Region.height *= TexCache.Granularity;
    }
    return Allocation;
}

void GLTFResourceManager::FreeTextureSpace(TextureAllocation&& Allocation)
{
    auto& TexCache = m_Textures[Allocation.TextureIndex];

    auto& Region = Allocation.Region;
    DEV_CHECK_ERR((Region.x % TexCache.Granularity) == 0, "Allocation x (", Region.x, ") is not multiple of granularity (", TexCache.Granularity, ")");
    DEV_CHECK_ERR((Region.y % TexCache.Granularity) == 0, "Allocation y (", Region.y, ") is not multiple of granularity (", TexCache.Granularity, ")");
    DEV_CHECK_ERR((Region.width % TexCache.Granularity) == 0, "Allocation width (", Region.width, ") is not multiple of granularity (", TexCache.Granularity, ")");
    DEV_CHECK_ERR((Region.height % TexCache.Granularity) == 0, "Allocation height (", Region.height, ") is not multiple of granularity (", TexCache.Granularity, ")");

    Region.x /= TexCache.Granularity;
    Region.y /= TexCache.Granularity;
    Region.width /= TexCache.Granularity;
    Region.height /= TexCache.Granularity;

    std::lock_guard<std::mutex> Lock{TexCache.Mtx};
    TexCache.Mgr.Free(std::move(Region));
}

} // namespace Diligent
