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

GLTFResourceManager::BufferCache::BufferCache(GLTFResourceManager& Owner,
                                              IRenderDevice*       pDevice,
                                              const BufferDesc&    BuffDesc) :
    m_Owner{Owner},
    m_Mgr{BuffDesc.uiSizeInBytes, DefaultRawMemoryAllocator::GetAllocator()}
{
    pDevice->CreateBuffer(BuffDesc, nullptr, &m_pBuffer);
}

IBuffer* GLTFResourceManager::BufferCache::GetBuffer(IRenderDevice* pDevice, IDeviceContext* pContext)
{
    std::lock_guard<std::mutex> Lock{m_Mtx};

    const auto& BuffDesc = m_pBuffer->GetDesc();
    const auto  MgrSize  = m_Mgr.GetMaxSize();
    if (BuffDesc.uiSizeInBytes < MgrSize)
    {
        // Extend the buffer
        auto NewBuffDesc = BuffDesc;

        NewBuffDesc.uiSizeInBytes = static_cast<Uint32>(MgrSize);

        RefCntAutoPtr<IBuffer> pNewBuffer;
        pDevice->CreateBuffer(NewBuffDesc, nullptr, &pNewBuffer);
        pContext->CopyBuffer(m_pBuffer, 0, RESOURCE_STATE_TRANSITION_MODE_TRANSITION,
                             pNewBuffer, 0, BuffDesc.uiSizeInBytes, RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

        m_pBuffer = std::move(pNewBuffer);
    }

    return m_pBuffer.RawPtr();
}

RefCntAutoPtr<GLTFResourceManager::BufferAllocation> GLTFResourceManager::BufferCache::Allocate(Uint32 Size, Uint32 Alignment)
{
    VERIFY_EXPR(Size > 0 && IsPowerOfTwo(Alignment));

    std::lock_guard<std::mutex> Lock{m_Mtx};

    auto Region = m_Mgr.Allocate(Size, Alignment);
    while (!Region.IsValid())
    {
        m_Mgr.Extend(m_Mgr.GetMaxSize());
        Region = m_Mgr.Allocate(Size, Alignment);
    }

    return RefCntAutoPtr<BufferAllocation>{
        MakeNewRCObj<BufferAllocation>()(
            RefCntAutoPtr<GLTFResourceManager>{&m_Owner},
            *this,
            std::move(Region) //
            )                 //
    };
}

GLTFResourceManager::TextureCache::TextureCache(GLTFResourceManager&       Owner,
                                                IRenderDevice*             pDevice,
                                                const TextureCacheAttribs& CacheCI) :
    m_Owner{Owner},
    m_TexDesc{CacheCI.Desc},
    m_Mgr{CacheCI.Desc.Width / CacheCI.Granularity, CacheCI.Desc.Height / CacheCI.Granularity},
    m_Granularity{CacheCI.Granularity}
{
    m_TexDesc.Name = m_TexName.c_str();

    DEV_CHECK_ERR(IsPowerOfTwo(CacheCI.Granularity), "Granularity (", CacheCI.Granularity, ") must be a power of two");
    DEV_CHECK_ERR((CacheCI.Desc.Width % CacheCI.Granularity) == 0, "Atlas width (", CacheCI.Desc.Width, ") is not multiple of granularity (", CacheCI.Granularity, ")");
    DEV_CHECK_ERR((CacheCI.Desc.Height % CacheCI.Granularity) == 0, "Atlas height (", CacheCI.Desc.Height, ") is not multiple of granularity (", CacheCI.Granularity, ")");

    pDevice->CreateTexture(CacheCI.Desc, nullptr, &m_pTexture);
}


ITexture* GLTFResourceManager::TextureCache::GetTexture(IRenderDevice* pDevice, IDeviceContext* pContext)
{
    // TODO: handle resizes
    return m_pTexture;
}

RefCntAutoPtr<GLTFResourceManager::TextureAllocation> GLTFResourceManager::TextureCache::Allocate(Uint32 Width, Uint32 Height)
{
    VERIFY_EXPR(Width > 0 && Height > 0);

    std::lock_guard<std::mutex> Lock{m_Mtx};

    auto Region = m_Mgr.Allocate((Width + m_Granularity - 1) / m_Granularity, (Height + m_Granularity - 1) / m_Granularity);
    if (!Region.IsEmpty())
    {
        Region.x *= m_Granularity;
        Region.y *= m_Granularity;
        Region.width *= m_Granularity;
        Region.height *= m_Granularity;

        return RefCntAutoPtr<TextureAllocation>{
            MakeNewRCObj<TextureAllocation>()(
                RefCntAutoPtr<GLTFResourceManager>{&m_Owner},
                *this,
                Width,
                Height,
                std::move(Region) //
                )                 //
        };
    }
    else
    {
        // TODO: handle resize
        return RefCntAutoPtr<TextureAllocation>{};
    }
}

void GLTFResourceManager::TextureCache::FreeAllocation(DynamicAtlasManager::Region&& Allocation)
{
    VERIFY((Allocation.x % m_Granularity) == 0, "Allocation x (", Allocation.x, ") is not multiple of granularity (", m_Granularity, ")");
    VERIFY((Allocation.y % m_Granularity) == 0, "Allocation y (", Allocation.y, ") is not multiple of granularity (", m_Granularity, ")");
    VERIFY((Allocation.width % m_Granularity) == 0, "Allocation width (", Allocation.width, ") is not multiple of granularity (", m_Granularity, ")");
    VERIFY((Allocation.height % m_Granularity) == 0, "Allocation height (", Allocation.height, ") is not multiple of granularity (", m_Granularity, ")");

    Allocation.x /= m_Granularity;
    Allocation.y /= m_Granularity;
    Allocation.width /= m_Granularity;
    Allocation.height /= m_Granularity;

    std::lock_guard<std::mutex> Lock{m_Mtx};
    m_Mgr.Free(std::move(Allocation));
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
        m_Buffers.emplace_back(*this, pDevice, CI.Buffers[i]);
    }

    m_Textures.reserve(CI.NumTextures);
    for (Uint32 i = 0; i < CI.NumTextures; ++i)
    {
        m_Textures.emplace_back(*this, pDevice, CI.Textures[i]);
    }
}

RefCntAutoPtr<GLTFResourceManager::TextureAllocation> GLTFResourceManager::FindAllocation(const char* CacheId)
{
    RefCntAutoPtr<TextureAllocation> pAllocation;

    std::lock_guard<std::mutex> Lock{m_AllocationsMtx};

    auto it = m_Allocations.find(CacheId);
    if (it != m_Allocations.end())
    {
        pAllocation = it->second.Lock();
        if (!pAllocation)
            m_Allocations.erase(it);
    }

    return pAllocation;
}

RefCntAutoPtr<GLTFResourceManager::TextureAllocation> GLTFResourceManager::AllocateTextureSpace(
    Uint32      TextureIndex,
    Uint32      Width,
    Uint32      Height,
    const char* CacheId)
{
    RefCntAutoPtr<TextureAllocation> pAllocation;
    if (CacheId != nullptr && *CacheId != 0)
    {
        pAllocation = FindAllocation(CacheId);
    }

    if (!pAllocation)
    {
        pAllocation = m_Textures[TextureIndex].Allocate(Width, Height);
    }

    if (CacheId != nullptr && *CacheId != 0)
    {
        auto inserted = m_Allocations.emplace(CacheId, pAllocation).second;
        VERIFY_EXPR(inserted);
    }

    return pAllocation;
}

} // namespace Diligent
