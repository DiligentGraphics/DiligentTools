/*
 *  Copyright 2019-2023 Diligent Graphics LLC
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
#include "GraphicsAccessories.hpp"

namespace Diligent
{

namespace GLTF
{

size_t ResourceManager::VertexLayoutKey::Hasher::operator()(const VertexLayoutKey& Key) const
{
    auto Hash = ComputeHash(Key.Elements.size());
    for (const auto& Elem : Key.Elements)
        HashCombine(Hash, Elem.Size, Elem.BindFlags);
    return Hash;
}

RefCntAutoPtr<ResourceManager> ResourceManager::Create(IRenderDevice*    pDevice,
                                                       const CreateInfo& CI)
{
    return RefCntAutoPtr<ResourceManager>{MakeNewRCObj<ResourceManager>()(pDevice, CI)};
}

ResourceManager::ResourceManager(IReferenceCounters* pRefCounters,
                                 IRenderDevice*      pDevice,
                                 const CreateInfo&   CI) :
    TBase{pRefCounters},
    m_DeviceType{pDevice->GetDeviceInfo().Type},
    m_DefaultVertPoolName{CI.DefaultPoolDesc.Name != nullptr ? CI.DefaultPoolDesc.Name : "GLTF vertex pool"},
    m_DefaultVertPoolDesc{CI.DefaultPoolDesc},
    m_DefaultAtlasName{CI.DefaultAtlasDesc.Desc.Name != nullptr ? CI.DefaultAtlasDesc.Desc.Name : "GLTF texture atlas"},
    m_DefaultAtlasDesc{CI.DefaultAtlasDesc}
{
    m_DefaultVertPoolDesc.Name   = m_DefaultVertPoolName.c_str();
    m_DefaultAtlasDesc.Desc.Name = m_DefaultAtlasName.c_str();

    CreateBufferSuballocator(pDevice, CI.IndexAllocatorCI, &m_pIndexBufferAllocator);
    VERIFY_EXPR(m_pIndexBufferAllocator);

    if (m_DefaultAtlasDesc.Desc.Type != RESOURCE_DIM_TEX_2D &&
        m_DefaultAtlasDesc.Desc.Type != RESOURCE_DIM_TEX_2D_ARRAY &&
        m_DefaultAtlasDesc.Desc.Type != RESOURCE_DIM_UNDEFINED)
    {
        LOG_ERROR_AND_THROW(GetResourceDimString(m_DefaultAtlasDesc.Desc.Type), " is not a valid resource dimension for a texture atlas");
    }

    if (m_DefaultAtlasDesc.Desc.Width > 0 &&
        m_DefaultAtlasDesc.Desc.Height > 0 &&
        m_DefaultAtlasDesc.Desc.Type != RESOURCE_DIM_UNDEFINED &&
        m_DefaultAtlasDesc.Desc.MipLevels == 0)
    {
        m_DefaultAtlasDesc.Desc.MipLevels = ComputeMipLevelsCount(m_DefaultAtlasDesc.Desc.Width, m_DefaultAtlasDesc.Desc.Height);
    }

    m_VertexPools.reserve(CI.NumVertexPools);
    for (Uint32 pool = 0; pool < CI.NumVertexPools; ++pool)
    {
        const auto& PoolCI = CI.pVertexPoolCIs[pool];

        RefCntAutoPtr<IVertexPool> pVtxPool;
        CreateVertexPool(pDevice, PoolCI, &pVtxPool);
        VERIFY_EXPR(pVtxPool);

        VertexLayoutKey Key;
        Key.Elements.reserve(PoolCI.Desc.NumElements);
        for (size_t i = 0; i < PoolCI.Desc.NumElements; ++i)
        {
            const auto& PoolElem = PoolCI.Desc.pElements[i];
            Key.Elements.emplace_back(PoolElem.Size, PoolElem.BindFlags);
        }

        if (!m_VertexPools.emplace(Key, std::move(pVtxPool)).second)
            LOG_ERROR_AND_THROW("Different vertex pools with the same layout key are not allowed.");
    }

    m_Atlases.reserve(CI.NumTexAtlases);
    for (Uint32 i = 0; i < CI.NumTexAtlases; ++i)
    {
        const auto& AtlasCI = CI.pTexAtlasCIs[i];

        RefCntAutoPtr<IDynamicTextureAtlas> pAtlas;
        CreateDynamicTextureAtlas(pDevice, AtlasCI, &pAtlas);
        VERIFY_EXPR(pAtlas);
        m_Atlases.emplace(AtlasCI.Desc.Format, std::move(pAtlas));
    }
}


RefCntAutoPtr<ITextureAtlasSuballocation> ResourceManager::FindTextureAllocation(const char* CacheId)
{
    RefCntAutoPtr<ITextureAtlasSuballocation> pAllocation;

    if (CacheId != nullptr && *CacheId != 0)
    {
        std::lock_guard<std::mutex> Lock{m_TexAllocationsMtx};

        auto it = m_TexAllocations.find(CacheId);
        if (it != m_TexAllocations.end())
        {
            pAllocation = it->second.Lock();
            if (!pAllocation)
                m_TexAllocations.erase(it);
        }
    }

    return pAllocation;
}

RefCntAutoPtr<ITextureAtlasSuballocation> ResourceManager::AllocateTextureSpace(
    TEXTURE_FORMAT Fmt,
    Uint32         Width,
    Uint32         Height,
    const char*    CacheId,
    IObject*       pUserData)
{
    RefCntAutoPtr<ITextureAtlasSuballocation> pAllocation;
    if (CacheId != nullptr && *CacheId != 0)
    {
        pAllocation = FindTextureAllocation(CacheId);
    }

    if (!pAllocation)
    {
        decltype(m_Atlases)::iterator cache_it; // NB: can't initialize it without locking the mutex
        {
            std::lock_guard<std::mutex> Lock{m_AtlasesMtx};
            cache_it = m_Atlases.find(Fmt);
            if (cache_it == m_Atlases.end())
            {
                if (m_DefaultAtlasDesc.Desc.Width == 0 ||
                    m_DefaultAtlasDesc.Desc.Height == 0 ||
                    m_DefaultAtlasDesc.Desc.Type == RESOURCE_DIM_UNDEFINED)
                {
                    // Creating additional texture atlases is not allowed.
                    return {};
                }

                auto AtalsCreateInfo        = m_DefaultAtlasDesc;
                AtalsCreateInfo.Desc.Format = Fmt;

                RefCntAutoPtr<IDynamicTextureAtlas> pAtlas;
                CreateDynamicTextureAtlas(nullptr, AtalsCreateInfo, &pAtlas);
                if (pAtlas)
                {
                    cache_it = m_Atlases.emplace(Fmt, std::move(pAtlas)).first;
                }
                else
                {
                    DEV_ERROR("Failed to create new texture atlas");
                    return {};
                }
            }
        }
        // Allocate outside of the mutex lock
        cache_it->second->Allocate(Width, Height, &pAllocation);
        pAllocation->SetUserData(pUserData);
    }

    if (CacheId != nullptr && *CacheId != 0)
    {
        std::lock_guard<std::mutex> Lock{m_TexAllocationsMtx};
        // Note that the same allocation may potentially be created by more
        // than one thread if it has not been found in the cache originally
        m_TexAllocations.emplace(CacheId, pAllocation);
    }

    return pAllocation;
}


RefCntAutoPtr<IBufferSuballocation> ResourceManager::AllocateIndices(Uint32 Size, Uint32 Alignment)
{
    RefCntAutoPtr<IBufferSuballocation> pIndices;
    if (!m_pIndexBufferAllocator)
    {
        UNEXPECTED("Index buffer allocator is not initialized");
        return pIndices;
    }
    m_pIndexBufferAllocator->Allocate(Size, Alignment, &pIndices);
    return pIndices;
}


RefCntAutoPtr<IVertexPoolAllocation> ResourceManager::AllocateVertices(const VertexLayoutKey& LayoutKey, Uint32 VertexCount)
{
#ifdef DILIGENT_DEVELOPMENT
    DEV_CHECK_ERR(!LayoutKey.Elements.empty(), "The key must not be empty.");
    for (const auto& Elem : LayoutKey.Elements)
    {
        DEV_CHECK_ERR(Elem.Size != 0, "Element size must not be zero.");
        DEV_CHECK_ERR(Elem.BindFlags != BIND_NONE, "Bind flags must not be NONE.");
    }
#endif

    decltype(m_VertexPools)::iterator pool_it; // NB: can't initialize it without locking the mutex
    {
        std::lock_guard<std::mutex> Lock{m_VertexPoolsMtx};
        pool_it = m_VertexPools.find(LayoutKey);
        if (pool_it == m_VertexPools.end())
        {
            if (m_DefaultVertPoolDesc.VertexCount == 0)
            {
                // Creating additional vertex pools is not allowed.
                return {};
            }

            VertexPoolCreateInfo PoolCI;
            PoolCI.Desc.Name        = m_DefaultVertPoolDesc.Name;
            PoolCI.Desc.VertexCount = m_DefaultVertPoolDesc.VertexCount;

            std::vector<VertexPoolElementDesc> PoolElems(LayoutKey.Elements.size());
            for (size_t i = 0; i < PoolElems.size(); ++i)
            {
                auto& ElemDesc = PoolElems[i];

                ElemDesc.Size           = LayoutKey.Elements[i].Size;
                ElemDesc.BindFlags      = LayoutKey.Elements[i].BindFlags;
                ElemDesc.Usage          = m_DefaultVertPoolDesc.Usage;
                ElemDesc.CPUAccessFlags = m_DefaultVertPoolDesc.CPUAccessFlags;
                ElemDesc.Mode           = m_DefaultVertPoolDesc.Mode;
                if (ElemDesc.Usage == USAGE_SPARSE && (ElemDesc.BindFlags & (BIND_VERTEX_BUFFER | BIND_INDEX_BUFFER)) != 0 && m_DeviceType == RENDER_DEVICE_TYPE_D3D11)
                {
                    // Direct3D11 does not support sparse vertex or index buffers
                    ElemDesc.Usage = USAGE_DEFAULT;
                }
            }
            PoolCI.Desc.NumElements = static_cast<Uint32>(PoolElems.size());
            PoolCI.Desc.pElements   = PoolElems.data();

            RefCntAutoPtr<IVertexPool> pVtxPool;
            CreateVertexPool(nullptr, PoolCI, &pVtxPool);
            if (pVtxPool)
            {
                pool_it = m_VertexPools.emplace(LayoutKey, std::move(pVtxPool)).first;
            }
            else
            {
                DEV_ERROR("Failed to create new vertex pool");
                return {};
            }
        }
    }

    // Allocate outside of the mutex lock
    RefCntAutoPtr<IVertexPoolAllocation> pVertices;
    pool_it->second->Allocate(VertexCount, &pVertices);
    return pVertices;
}


Uint32 ResourceManager::GetTextureVersion()
{
    Uint32 Version = 0;

    std::lock_guard<std::mutex> Lock{m_AtlasesMtx};
    for (auto atlas_it : m_Atlases)
        Version += atlas_it.second->GetVersion();

    return Version;
}


Uint32 ResourceManager::GetIndexBufferVersion(Uint32 Index) const
{
    return m_pIndexBufferAllocator ? m_pIndexBufferAllocator->GetVersion() : 0;
}

Uint32 ResourceManager::GetVertexPoolsVersion()
{
    Uint32 Version = 0;

    std::lock_guard<std::mutex> Lock{m_VertexPoolsMtx};
    for (auto pool_it : m_VertexPools)
        Version += pool_it.second->GetVersion();

    return Version;
}

IBuffer* ResourceManager::GetIndexBuffer(IRenderDevice* pDevice, IDeviceContext* pContext)
{
    return m_pIndexBufferAllocator ?
        m_pIndexBufferAllocator->GetBuffer(pDevice, pContext) :
        nullptr;
}

IVertexPool* ResourceManager::GetVertexPool(const VertexLayoutKey& Key)
{
    decltype(m_VertexPools)::iterator pool_it; // NB: can't initialize it without locking the mutex
    {
        std::lock_guard<std::mutex> Lock{m_VertexPoolsMtx};
        pool_it = m_VertexPools.find(Key);
        if (pool_it == m_VertexPools.end())
            return nullptr;
    }

    return pool_it->second;
}

ITexture* ResourceManager::GetTexture(TEXTURE_FORMAT Fmt, IRenderDevice* pDevice, IDeviceContext* pContext)
{
    decltype(m_Atlases)::iterator cache_it; // NB: can't initialize it without locking the mutex
    {
        std::lock_guard<std::mutex> Lock{m_AtlasesMtx};
        cache_it = m_Atlases.find(Fmt);
        if (cache_it == m_Atlases.end())
            return nullptr;
    }

    return cache_it->second->GetTexture(pDevice, pContext);
}

TextureDesc ResourceManager::GetAtlasDesc(TEXTURE_FORMAT Fmt)
{
    {
        std::lock_guard<std::mutex> Lock{m_AtlasesMtx};

        auto cache_it = m_Atlases.find(Fmt);
        if (cache_it != m_Atlases.end())
            return cache_it->second->GetAtlasDesc();
    }

    // Atlas is not present in the map - use default description
    TextureDesc Desc = m_DefaultAtlasDesc.Desc;
    Desc.Format      = Fmt;
    return Desc;
}

Uint32 ResourceManager::GetAllocationAlignment(TEXTURE_FORMAT Fmt, Uint32 Width, Uint32 Height)
{
    {
        std::lock_guard<std::mutex> Lock{m_AtlasesMtx};

        auto cache_it = m_Atlases.find(Fmt);
        if (cache_it != m_Atlases.end())
            return cache_it->second->GetAllocationAlignment(Width, Height);
    }

    // Atlas is not present in the map - use default description
    return ComputeTextureAtlasSuballocationAlignment(Width, Height, m_DefaultAtlasDesc.MinAlignment);
}


BufferSuballocatorUsageStats ResourceManager::GetIndexBufferUsageStats()
{
    BufferSuballocatorUsageStats Stats;
    if (m_pIndexBufferAllocator)
        m_pIndexBufferAllocator->GetUsageStats(Stats);
    return Stats;
}

DynamicTextureAtlasUsageStats ResourceManager::GetAtlasUsageStats(TEXTURE_FORMAT Fmt)
{
    DynamicTextureAtlasUsageStats Stats;
    {
        std::lock_guard<std::mutex> Lock{m_AtlasesMtx};
        if (Fmt != TEX_FORMAT_UNKNOWN)
        {
            auto cache_it = m_Atlases.find(Fmt);
            if (cache_it != m_Atlases.end())
                cache_it->second->GetUsageStats(Stats);
        }
        else
        {
            for (auto it : m_Atlases)
            {
                DynamicTextureAtlasUsageStats AtlasStats;
                it.second->GetUsageStats(AtlasStats);
                Stats.CommittedSize += AtlasStats.CommittedSize;
                Stats.TotalArea += AtlasStats.TotalArea;
                Stats.AllocatedArea += AtlasStats.AllocatedArea;
                Stats.UsedArea += AtlasStats.UsedArea;
                Stats.AllocationCount += AtlasStats.AllocationCount;
            }
        }
    }

    return Stats;
}

VertexPoolUsageStats ResourceManager::GetVertexPoolUsageStats(const VertexLayoutKey& Key)
{
    VertexPoolUsageStats Stats;
    {
        std::lock_guard<std::mutex> Lock{m_VertexPoolsMtx};
        if (Key != VertexLayoutKey{})
        {
            auto pool_it = m_VertexPools.find(Key);
            if (pool_it != m_VertexPools.end())
                pool_it->second->GetUsageStats(Stats);
        }
        else
        {
            for (auto it : m_VertexPools)
            {
                VertexPoolUsageStats PoolStats;
                it.second->GetUsageStats(PoolStats);

                Stats.TotalVertexCount += PoolStats.TotalVertexCount;
                Stats.AllocatedVertexCount += PoolStats.AllocatedVertexCount;
                Stats.CommittedMemorySize += PoolStats.CommittedMemorySize;
                Stats.UsedMemorySize += PoolStats.UsedMemorySize;
                Stats.AllocationCount += PoolStats.AllocationCount;
            }
        }
    }

    return Stats;
}

} // namespace GLTF

} // namespace Diligent
