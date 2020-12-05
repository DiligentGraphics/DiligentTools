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
    m_Attribs{CacheCI}
{
    m_Attribs.Desc.Name = m_TexName.c_str();

    const auto& Desc = m_Attribs.Desc;
    for (Uint32 slice = 0; slice < Desc.ArraySize; ++slice)
    {
        m_Slices.emplace_back(new SliceManager{Desc.Width / m_Attribs.Granularity, Desc.Height / m_Attribs.Granularity});
    }

    DEV_CHECK_ERR(IsPowerOfTwo(m_Attribs.Granularity), "Granularity (", m_Attribs.Granularity, ") must be a power of two");
    DEV_CHECK_ERR((Desc.Width % m_Attribs.Granularity) == 0, "Atlas width (", Desc.Width, ") is not multiple of granularity (", m_Attribs.Granularity, ")");
    DEV_CHECK_ERR((Desc.Height % m_Attribs.Granularity) == 0, "Atlas height (", Desc.Height, ") is not multiple of granularity (", m_Attribs.Granularity, ")");

    if (pDevice != nullptr)
    {
        pDevice->CreateTexture(Desc, nullptr, &m_pTexture);
    }
}


ITexture* GLTFResourceManager::TextureCache::GetTexture(IRenderDevice* pDevice, IDeviceContext* pContext)
{
    RefCntAutoPtr<ITexture> pNewTexture;
    {
        std::lock_guard<std::mutex> Lock{m_SlicesMtx};
        if (!m_pTexture || m_pTexture->GetDesc().ArraySize < m_Slices.size())
        {
            m_Attribs.Desc.ArraySize = static_cast<Uint32>(m_Slices.size());
            VERIFY_EXPR(pDevice != nullptr);
            pDevice->CreateTexture(m_Attribs.Desc, nullptr, &pNewTexture);
        }
        if (pNewTexture)
        {
            if (m_pTexture)
            {
                const auto& OldDesc = m_pTexture->GetDesc();

                CopyTextureAttribs CopyAttribs;
                CopyAttribs.pSrcTexture              = m_pTexture;
                CopyAttribs.pDstTexture              = pNewTexture;
                CopyAttribs.SrcTextureTransitionMode = RESOURCE_STATE_TRANSITION_MODE_TRANSITION;
                CopyAttribs.DstTextureTransitionMode = RESOURCE_STATE_TRANSITION_MODE_TRANSITION;

                for (Uint32 slice = 0; slice < OldDesc.ArraySize; ++slice)
                {
                    for (Uint32 mip = 0; mip < OldDesc.MipLevels; ++mip)
                    {
                        CopyAttribs.SrcSlice    = slice;
                        CopyAttribs.DstSlice    = slice;
                        CopyAttribs.SrcMipLevel = mip;
                        CopyAttribs.DstMipLevel = mip;
                        pContext->CopyTexture(CopyAttribs);
                    }
                }
            }
            m_pTexture = std::move(pNewTexture);
            m_Owner.m_ResourceVersion.fetch_add(1);
        }
    }

    return m_pTexture;
}

RefCntAutoPtr<GLTFResourceManager::TextureAllocation> GLTFResourceManager::TextureCache::Allocate(Uint32 Width, Uint32 Height)
{
    VERIFY_EXPR(Width > 0 && Height > 0);
    const auto& TexDesc = m_Attribs.Desc;
    if (Width > TexDesc.Width || Height > TexDesc.Height)
    {
        LOG_ERROR_MESSAGE("Requested size ", Width, " x ", Height, " exceeds the texture size ", TexDesc.Width, " x ", TexDesc.Height);
        return {};
    }

    const auto Granularity = m_Attribs.Granularity;
    for (Uint32 Slice = 0; Slice < m_Attribs.MaxSlices; ++Slice)
    {
        SliceManager* pSliceMgr = nullptr;
        {
            std::lock_guard<std::mutex> Lock{m_SlicesMtx};
            if (Slice == m_Slices.size())
            {
                for (Uint32 ExtraSlice = 0; ExtraSlice < m_Attribs.ExtraSliceCount && Slice + ExtraSlice < m_Attribs.MaxSlices; ++ExtraSlice)
                {
                    m_Slices.emplace_back(new SliceManager{TexDesc.Width / Granularity, TexDesc.Height / Granularity});
                }
            }
            pSliceMgr = m_Slices[Slice].get();
        }

        auto Region = pSliceMgr->Allocate((Width + Granularity - 1) / Granularity, (Height + Granularity - 1) / Granularity);
        if (!Region.IsEmpty())
        {
            Region.x *= Granularity;
            Region.y *= Granularity;
            Region.width *= Granularity;
            Region.height *= Granularity;

            return RefCntAutoPtr<TextureAllocation>{
                MakeNewRCObj<TextureAllocation>()(
                    RefCntAutoPtr<GLTFResourceManager>{&m_Owner},
                    *this,
                    Width,
                    Height,
                    Slice,
                    std::move(Region) //
                    )                 //
            };
        }
    }

    return RefCntAutoPtr<TextureAllocation>{};
}

void GLTFResourceManager::TextureCache::FreeAllocation(Uint32 Slice, DynamicAtlasManager::Region&& Allocation)
{
    const auto Granularity = m_Attribs.Granularity;
    VERIFY((Allocation.x % Granularity) == 0, "Allocation x (", Allocation.x, ") is not multiple of granularity (", Granularity, ")");
    VERIFY((Allocation.y % Granularity) == 0, "Allocation y (", Allocation.y, ") is not multiple of granularity (", Granularity, ")");
    VERIFY((Allocation.width % Granularity) == 0, "Allocation width (", Allocation.width, ") is not multiple of granularity (", Granularity, ")");
    VERIFY((Allocation.height % Granularity) == 0, "Allocation height (", Allocation.height, ") is not multiple of granularity (", Granularity, ")");

    Allocation.x /= Granularity;
    Allocation.y /= Granularity;
    Allocation.width /= Granularity;
    Allocation.height /= Granularity;

    SliceManager* pSliceMgr = nullptr;
    {
        std::lock_guard<std::mutex> Lock{m_SlicesMtx};
        pSliceMgr = m_Slices[Slice].get();
    }
    pSliceMgr->Free(std::move(Allocation));
}



RefCntAutoPtr<GLTFResourceManager> GLTFResourceManager::Create(IRenderDevice*    pDevice,
                                                               const CreateInfo& CI)
{
    return RefCntAutoPtr<GLTFResourceManager>{MakeNewRCObj<GLTFResourceManager>()(pDevice, CI)};
}

GLTFResourceManager::GLTFResourceManager(IReferenceCounters* pRefCounters,
                                         IRenderDevice*      pDevice,
                                         const CreateInfo&   CI) :
    TBase{pRefCounters},
    m_DefaultTexDesc{CI.DefaultTexDesc},
    m_DefaultExtraSliceCount{CI.DefaultExtraSliceCount}
{
    m_Buffers.reserve(CI.NumBuffers);
    for (Uint32 i = 0; i < CI.NumBuffers; ++i)
    {
        m_Buffers.emplace_back(*this, pDevice, CI.Buffers[i]);
    }

    m_Textures.reserve(CI.NumTextures);
    for (Uint32 i = 0; i < CI.NumTextures; ++i)
    {
        m_Textures.emplace(CI.Textures[i].Desc.Format, TextureCache{*this, pDevice, CI.Textures[i]});
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
    TEXTURE_FORMAT Fmt,
    Uint32         Width,
    Uint32         Height,
    const char*    CacheId)
{
    RefCntAutoPtr<TextureAllocation> pAllocation;
    if (CacheId != nullptr && *CacheId != 0)
    {
        pAllocation = FindAllocation(CacheId);
    }

    if (!pAllocation)
    {
        decltype(m_Textures)::iterator cache_it; // NB: can't initialize it without locking the mutex
        {
            std::lock_guard<std::mutex> Lock{m_TexturesMtx};
            cache_it = m_Textures.find(Fmt);
            if (cache_it == m_Textures.end())
            {
                TextureCacheAttribs NewCacheAttribs;
                NewCacheAttribs.Desc = m_DefaultTexDesc;

                NewCacheAttribs.Desc.Name   = "GLTF Texture cache";
                NewCacheAttribs.Desc.Format = Fmt;

                cache_it = m_Textures.emplace(Fmt, TextureCache{*this, nullptr, NewCacheAttribs}).first;
            }
        }
        // Allocate outside of mutex
        pAllocation = cache_it->second.Allocate(Width, Height);
    }

    if (CacheId != nullptr && *CacheId != 0)
    {
        auto inserted = m_Allocations.emplace(CacheId, pAllocation).second;
        VERIFY_EXPR(inserted);
    }

    return pAllocation;
}

} // namespace Diligent
