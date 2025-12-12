/*
 *  Copyright 2019-2025 Diligent Graphics LLC
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

#include <algorithm>

#include "DefaultRawMemoryAllocator.hpp"
#include "Align.hpp"
#include "GraphicsAccessories.hpp"

namespace Diligent
{

namespace GLTF
{

size_t ResourceManager::VertexLayoutKey::Hasher::operator()(const VertexLayoutKey& Key) const
{
    size_t Hash = ComputeHash(Key.Elements.size());
    for (const ElementDesc& Elem : Key.Elements)
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
    m_DefaultAtlasDesc{CI.DefaultAtlasDesc},
    m_IndexAllocatorCI{CI.IndexAllocatorCI}
{
    m_DefaultVertPoolDesc.Name   = m_DefaultVertPoolName.c_str();
    m_DefaultAtlasDesc.Desc.Name = m_DefaultAtlasName.c_str();

    m_IndexAllocators.emplace_back(CreateIndexBufferAllocator(pDevice));

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
    m_VertexPoolCIs.reserve(CI.NumVertexPools);
    for (Uint32 pool = 0; pool < CI.NumVertexPools; ++pool)
    {
        const VertexPoolCreateInfo& PoolCI = CI.pVertexPoolCIs[pool];

        VertexLayoutKey Key;
        Key.Elements.reserve(PoolCI.Desc.NumElements);
        for (size_t i = 0; i < PoolCI.Desc.NumElements; ++i)
        {
            const VertexPoolElementDesc& PoolElem = PoolCI.Desc.pElements[i];
            Key.Elements.emplace_back(PoolElem.Size, PoolElem.BindFlags);
        }

        if (!m_VertexPoolCIs.emplace(Key, PoolCI).second)
            LOG_ERROR_AND_THROW("Different vertex pools with the same layout key are not allowed.");

        RefCntAutoPtr<IVertexPool> pVtxPool;
        CreateVertexPool(pDevice, PoolCI, &pVtxPool);
        VERIFY_EXPR(pVtxPool);

        m_VertexPools[Key].emplace_back(std::move(pVtxPool));
        VERIFY_EXPR(m_VertexPools[Key].size() == 1);
    }

    m_Atlases.reserve(CI.NumTexAtlases);
    for (Uint32 i = 0; i < CI.NumTexAtlases; ++i)
    {
        const DynamicTextureAtlasCreateInfo& AtlasCI = CI.pTexAtlasCIs[i];

        RefCntAutoPtr<IDynamicTextureAtlas> pAtlas;
        CreateDynamicTextureAtlas(pDevice, AtlasCI, &pAtlas);
        VERIFY_EXPR(pAtlas);
        m_Atlases.emplace(AtlasCI.Desc.Format, std::move(pAtlas));
    }
}


RefCntAutoPtr<ITextureAtlasSuballocation> ResourceManager::FindTextureAllocation(const char* CacheId)
{
    if (CacheId != nullptr && *CacheId != 0)
    {
        bool AllocationExpired = false;

        // First, try to find the allocation with a shared lock
        {
            std::shared_lock<std::shared_mutex> SharedLock{m_TexAllocationsMtx};

            auto it = m_TexAllocations.find(CacheId);
            if (it != m_TexAllocations.end())
            {
                if (RefCntAutoPtr<ITextureAtlasSuballocation> pAllocation = it->second.Lock())
                    return pAllocation;
                else
                    AllocationExpired = true;
            }
        }

        // If the allocation was found but has expired, acquire a unique lock to erase it
        if (AllocationExpired)
        {
            std::unique_lock<std::shared_mutex> UniqueLock{m_TexAllocationsMtx};

            auto it = m_TexAllocations.find(CacheId);
            if (it != m_TexAllocations.end())
            {
                if (RefCntAutoPtr<ITextureAtlasSuballocation> pAllocation = it->second.Lock())
                    return pAllocation;
                else
                    m_TexAllocations.erase(it);
            }
        }
    }

    return {};
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
        RefCntAutoPtr<IDynamicTextureAtlas> pAtlas;
        {
            std::shared_lock<std::shared_mutex> SharedLock{m_AtlasesMtx};

            auto cache_it = m_Atlases.find(Fmt);
            if (cache_it != m_Atlases.end())
            {
                pAtlas = cache_it->second;
            }
        }

        if (!pAtlas)
        {
            std::unique_lock<std::shared_mutex> UniqueLock{m_AtlasesMtx};

            auto cache_it = m_Atlases.find(Fmt);
            if (cache_it != m_Atlases.end())
            {
                pAtlas = cache_it->second;
            }
            else
            {
                if (m_DefaultAtlasDesc.Desc.Width == 0 ||
                    m_DefaultAtlasDesc.Desc.Height == 0 ||
                    m_DefaultAtlasDesc.Desc.Type == RESOURCE_DIM_UNDEFINED)
                {
                    // Creating additional texture atlases is not allowed.
                    return {};
                }

                DynamicTextureAtlasCreateInfo AtalsCreateInfo = m_DefaultAtlasDesc;
                AtalsCreateInfo.Desc.Format                   = Fmt;

                CreateDynamicTextureAtlas(nullptr, AtalsCreateInfo, &pAtlas);
                if (pAtlas)
                {
                    cache_it = m_Atlases.emplace(Fmt, pAtlas).first;
                }
                else
                {
                    LOG_ERROR_MESSAGE("Failed to allocate texture atlas for format ", GetTextureFormatAttribs(Fmt).Name);
                    return {};
                }
            }
        }

        // Allocate outside of the mutex lock
        pAtlas->Allocate(Width, Height, &pAllocation);
        if (!pAllocation)
        {
            LOG_ERROR_MESSAGE("Failed to allocate texture space of size ", Width, "x", Height, " in atlas of format ", GetTextureFormatAttribs(Fmt).Name);
            return {};
        }
        pAllocation->SetUserData(pUserData);
    }

    if (CacheId != nullptr && *CacheId != 0)
    {
        std::unique_lock<std::shared_mutex> UniqueLock{m_TexAllocationsMtx};
        // Note that the same allocation may potentially be created by more
        // than one thread if it has not been found in the cache originally
        auto [it, inserted] = m_TexAllocations.emplace(CacheId, pAllocation);
        if (!inserted)
        {
            if (auto pExistingAllocation = it->second.Lock())
                pAllocation = pExistingAllocation;
            else
                it->second = pAllocation;
        }
    }

    return pAllocation;
}

std::vector<IDynamicTextureAtlas*>& ResourceManager::GetAtlasSnapshot()
{
    m_AtlasSnapshot.clear();

    {
        std::shared_lock<std::shared_mutex> SharedLock{m_AtlasesMtx};
        for (const auto& it : m_Atlases)
        {
            m_AtlasSnapshot.emplace_back(it.second);
        }
    }

    return m_AtlasSnapshot;
}

std::vector<IVertexPool*>& ResourceManager::GetVertexPoolSnapshot()
{
    m_VertexPoolSnapshot.clear();

    {
        std::shared_lock<std::shared_mutex> SharedLock{m_VertexPoolsMtx};
        for (const auto& pools_it : m_VertexPools)
        {
            for (const auto& Pool : pools_it.second)
                m_VertexPoolSnapshot.emplace_back(Pool);
        }
    }

    return m_VertexPoolSnapshot;
}

std::vector<IBufferSuballocator*>& ResourceManager::GetIndexAllocatorSnapshot()
{
    m_IndexAllocatorSnapshot.clear();

    {
        std::shared_lock<std::shared_mutex> SharedLock{m_IndexAllocatorsMtx};
        for (const RefCntAutoPtr<IBufferSuballocator>& pAllocator : m_IndexAllocators)
        {
            m_IndexAllocatorSnapshot.emplace_back(pAllocator);
        }
    }

    return m_IndexAllocatorSnapshot;
}

RefCntAutoPtr<IBufferSuballocator> ResourceManager::CreateIndexBufferAllocator(IRenderDevice* pDevice) const
{
    RefCntAutoPtr<IBufferSuballocator> pIndexBufferAllocator;
    CreateBufferSuballocator(pDevice, m_IndexAllocatorCI, &pIndexBufferAllocator);
    VERIFY_EXPR(pIndexBufferAllocator);

    return pIndexBufferAllocator;
}

RefCntAutoPtr<IBufferSuballocation> ResourceManager::AllocateIndices(Uint32 Size, Uint32 Alignment)
{
    RefCntAutoPtr<IBufferSuballocation> pIndices;
    for (Uint32 AllocatorIdx = 0; !pIndices; ++AllocatorIdx)
    {
        IBufferSuballocator* pAllocator = nullptr;
        {
            std::shared_lock<std::shared_mutex> SharedLock{m_IndexAllocatorsMtx};
            if (AllocatorIdx < m_IndexAllocators.size())
                pAllocator = m_IndexAllocators[AllocatorIdx];
        }

        if (pAllocator == nullptr)
        {
            std::unique_lock<std::shared_mutex> UniqueLock{m_IndexAllocatorsMtx};
            if (AllocatorIdx == m_IndexAllocators.size())
            {
                m_IndexAllocators.emplace_back(CreateIndexBufferAllocator(nullptr));
            }
            pAllocator = m_IndexAllocators[AllocatorIdx];
        }

        if (pAllocator == nullptr)
            break;

        pAllocator->Allocate(Size, Alignment, &pIndices);
    }

    return pIndices;
}

RefCntAutoPtr<IVertexPool> ResourceManager::CreateVertexPoolForLayout(const VertexLayoutKey& Key) const
{
    RefCntAutoPtr<IVertexPool> pVtxPool;

    auto pool_ci_it = m_VertexPoolCIs.find(Key);
    if (pool_ci_it != m_VertexPoolCIs.end())
    {
        // Create additional vertex pool using the existing CI.
        CreateVertexPool(nullptr, pool_ci_it->second, &pVtxPool);
        DEV_CHECK_ERR(pVtxPool, "Failed to create vertex pool");
    }
    else if (m_DefaultVertPoolDesc.VertexCount != 0)
    {
        // Create additional vertex pool using the default CI.
        VertexPoolCreateInfo PoolCI;
        PoolCI.Desc.Name        = m_DefaultVertPoolDesc.Name;
        PoolCI.Desc.VertexCount = m_DefaultVertPoolDesc.VertexCount;

        std::vector<VertexPoolElementDesc> PoolElems(Key.Elements.size());
        for (size_t i = 0; i < PoolElems.size(); ++i)
        {
            VertexPoolElementDesc& ElemDesc = PoolElems[i];

            ElemDesc.Size           = Key.Elements[i].Size;
            ElemDesc.BindFlags      = Key.Elements[i].BindFlags;
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

        CreateVertexPool(nullptr, PoolCI, &pVtxPool);
        DEV_CHECK_ERR(pVtxPool, "Failed to create vertex pool");
    }

    return pVtxPool;
}

RefCntAutoPtr<IVertexPoolAllocation> ResourceManager::AllocateVertices(const VertexLayoutKey& LayoutKey, Uint32 VertexCount)
{
#ifdef DILIGENT_DEVELOPMENT
    DEV_CHECK_ERR(!LayoutKey.Elements.empty(), "The key must not be empty.");
    for (const VertexLayoutKey::ElementDesc& Elem : LayoutKey.Elements)
    {
        DEV_CHECK_ERR(Elem.Size != 0, "Element size must not be zero.");
        DEV_CHECK_ERR(Elem.BindFlags != BIND_NONE, "Bind flags must not be NONE.");
    }
#endif

    RefCntAutoPtr<IVertexPoolAllocation> pVertices;
    for (Uint32 PoolIdx = 0; !pVertices; ++PoolIdx)
    {
        IVertexPool* pPool = nullptr;

        {
            std::shared_lock<std::shared_mutex> SharedLock{m_VertexPoolsMtx};

            auto pools_it = m_VertexPools.find(LayoutKey);
            if (pools_it != m_VertexPools.end() && PoolIdx < pools_it->second.size())
            {
                pPool = pools_it->second[PoolIdx];
            }
        }

        if (pPool == nullptr)
        {
            std::unique_lock<std::shared_mutex> UniqueLock{m_VertexPoolsMtx};

            auto pools_it = m_VertexPools.find(LayoutKey);
            if (pools_it != m_VertexPools.end() && PoolIdx < pools_it->second.size())
            {
                pPool = pools_it->second[PoolIdx];
            }
            else
            {
                // All pools have been checked or there is no pool for the key. Add a new pool.
                if (RefCntAutoPtr<IVertexPool> pNewVtxPool = CreateVertexPoolForLayout(LayoutKey))
                {
                    pPool = pNewVtxPool;
                    m_VertexPools[LayoutKey].push_back(std::move(pNewVtxPool));
                }
            }
        }

        if (pPool == nullptr)
            break;

        pPool->Allocate(VertexCount, &pVertices);
    }

    return pVertices;
}


Uint32 ResourceManager::GetTextureVersion() const
{
    Uint32 Version = 0;

    std::shared_lock<std::shared_mutex> SharedLock{m_AtlasesMtx};
    for (const auto& atlas_it : m_Atlases)
        Version += atlas_it.second->GetVersion();

    return Version;
}


Uint32 ResourceManager::GetIndexBufferVersion() const
{
    Uint32 Version = 0;

    std::shared_lock<std::shared_mutex> SharedLock{m_IndexAllocatorsMtx};
    for (const RefCntAutoPtr<IBufferSuballocator>& pAllocator : m_IndexAllocators)
        Version += pAllocator ? pAllocator->GetVersion() : 0;

    return Version;
}

Uint32 ResourceManager::GetVertexPoolsVersion() const
{
    Uint32 Version = 0;

    std::shared_lock<std::shared_mutex> SharedLock{m_VertexPoolsMtx};
    for (const auto& pools_it : m_VertexPools)
    {
        for (const auto& Pool : pools_it.second)
            Version += Pool->GetVersion();
    }
    return Version;
}

IBuffer* ResourceManager::UpdateIndexBuffer(IRenderDevice* pDevice, IDeviceContext* pContext, Uint32 Index)
{
    IBufferSuballocator* pIndexBufferAllocator = nullptr;
    {
        std::shared_lock<std::shared_mutex> SharedLock{m_IndexAllocatorsMtx};
        pIndexBufferAllocator = Index < m_IndexAllocators.size() ? m_IndexAllocators[Index].RawPtr() : nullptr;
    }

    return pIndexBufferAllocator != nullptr ?
        pIndexBufferAllocator->Update(pDevice, pContext) :
        nullptr;
}

void ResourceManager::UpdateIndexBuffers(IRenderDevice* pDevice, IDeviceContext* pContext)
{
    for (IBufferSuballocator* pAllocator : GetIndexAllocatorSnapshot())
        pAllocator->Update(pDevice, pContext);

    m_IndexAllocatorSnapshot.clear();
}

IBuffer* ResourceManager::GetIndexBuffer(Uint32 Index) const
{
    std::shared_lock<std::shared_mutex> SharedLock{m_IndexAllocatorsMtx};

    if (Index >= m_IndexAllocators.size())
        return nullptr;

    if (const auto& pAllocator = m_IndexAllocators[Index])
        return pAllocator->GetBuffer();
    else
        return nullptr;
}

size_t ResourceManager::GetIndexBufferCount() const
{
    std::shared_lock<std::shared_mutex> SharedLock{m_IndexAllocatorsMtx};
    return m_IndexAllocators.size();
}

Uint32 ResourceManager::GetIndexAllocatorIndex(IBufferSuballocator* pAllocator) const
{
    std::shared_lock<std::shared_mutex> SharedLock{m_IndexAllocatorsMtx};
    for (Uint32 i = 0; i < m_IndexAllocators.size(); ++i)
    {
        if (pAllocator == m_IndexAllocators[i])
            return i;
    }
    return ~0u;
}

void ResourceManager::UpdateVertexBuffers(IRenderDevice* pDevice, IDeviceContext* pContext)
{
    for (IVertexPool* Pool : GetVertexPoolSnapshot())
        Pool->UpdateAll(pDevice, pContext);

    m_VertexPoolSnapshot.clear();
}

IVertexPool* ResourceManager::GetVertexPool(const VertexLayoutKey& Key, Uint32 Index)
{
    std::shared_lock<std::shared_mutex> SharedLock{m_VertexPoolsMtx};

    const auto pools_it = m_VertexPools.find(Key);
    if (pools_it != m_VertexPools.end())
        return Index < pools_it->second.size() ? pools_it->second[Index].RawPtr() : nullptr;
    else
        return nullptr;
}

size_t ResourceManager::GetVertexPoolCount(const VertexLayoutKey& Key) const
{
    std::shared_lock<std::shared_mutex> SharedLock{m_VertexPoolsMtx};

    const auto pools_it = m_VertexPools.find(Key);
    return pools_it != m_VertexPools.end() ? pools_it->second.size() : 0;
}

std::vector<IVertexPool*> ResourceManager::GetVertexPools(const VertexLayoutKey& Key) const
{
    std::vector<IVertexPool*> Pools;
    {
        std::shared_lock<std::shared_mutex> SharedLock{m_VertexPoolsMtx};

        const auto pools_it = m_VertexPools.find(Key);
        if (pools_it != m_VertexPools.end())
        {
            Pools.reserve(pools_it->second.size());
            for (const auto& Pool : pools_it->second)
                Pools.emplace_back(Pool);
        }
    }
    return Pools;
}

Uint32 ResourceManager::GetVertexPoolIndex(const VertexLayoutKey& Key, IVertexPool* pPool) const
{
    std::shared_lock<std::shared_mutex> SharedLock{m_VertexPoolsMtx};

    const auto pools_it = m_VertexPools.find(Key);
    if (pools_it != m_VertexPools.end())
    {
        for (Uint32 i = 0; i < pools_it->second.size(); ++i)
        {
            if (pPool == pools_it->second[i])
                return i;
        }
    }

    return ~0u;
}

ITexture* ResourceManager::UpdateTexture(TEXTURE_FORMAT Fmt, IRenderDevice* pDevice, IDeviceContext* pContext)
{
    IDynamicTextureAtlas* pAtlas = nullptr;
    {
        std::shared_lock<std::shared_mutex> SharedLock{m_AtlasesMtx};

        auto cache_it = m_Atlases.find(Fmt);
        if (cache_it != m_Atlases.end())
            pAtlas = cache_it->second;
    }

    return pAtlas ? pAtlas->Update(pDevice, pContext) : nullptr;
}

void ResourceManager::UpdateTextures(IRenderDevice* pDevice, IDeviceContext* pContext)
{
    for (IDynamicTextureAtlas* pAtlas : GetAtlasSnapshot())
        pAtlas->Update(pDevice, pContext);

    m_AtlasSnapshot.clear();
}

ITexture* ResourceManager::GetTexture(TEXTURE_FORMAT Fmt) const
{
    IDynamicTextureAtlas* pAtlas = nullptr;
    {
        std::shared_lock<std::shared_mutex> SharedLock{m_AtlasesMtx};

        auto cache_it = m_Atlases.find(Fmt);
        if (cache_it != m_Atlases.end())
            pAtlas = cache_it->second;
    }

    return pAtlas != nullptr ? pAtlas->GetTexture() : nullptr;
}

void ResourceManager::UpdateAllResources(IRenderDevice* pDevice, IDeviceContext* pContext)
{
    UpdateIndexBuffer(pDevice, pContext);
    UpdateVertexBuffers(pDevice, pContext);
    UpdateTextures(pDevice, pContext);
}

TextureDesc ResourceManager::GetAtlasDesc(TEXTURE_FORMAT Fmt)
{
    {
        std::shared_lock<std::shared_mutex> SharedLock{m_AtlasesMtx};

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
        std::shared_lock<std::shared_mutex> SharedLock{m_AtlasesMtx};

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

    std::shared_lock<std::shared_mutex> SharedLock{m_IndexAllocatorsMtx};
    for (const auto& pAllocator : m_IndexAllocators)
    {
        if (pAllocator)
        {
            BufferSuballocatorUsageStats AllocatorStats;
            pAllocator->GetUsageStats(AllocatorStats);
            Stats += AllocatorStats;
        }
    }
    return Stats;
}

DynamicTextureAtlasUsageStats ResourceManager::GetAtlasUsageStats(TEXTURE_FORMAT Fmt)
{
    DynamicTextureAtlasUsageStats Stats;
    {
        std::shared_lock<std::shared_mutex> SharedLock{m_AtlasesMtx};
        if (Fmt != TEX_FORMAT_UNKNOWN)
        {
            auto cache_it = m_Atlases.find(Fmt);
            if (cache_it != m_Atlases.end())
                cache_it->second->GetUsageStats(Stats);
        }
        else
        {
            for (const auto& it : m_Atlases)
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

std::vector<TEXTURE_FORMAT> ResourceManager::GetAllocatedAtlasFormats() const
{
    std::vector<TEXTURE_FORMAT> Formats;

    {
        std::shared_lock<std::shared_mutex> SharedLock{m_AtlasesMtx};
        Formats.reserve(m_Atlases.size());
        for (const auto& it : m_Atlases)
        {
            if (it.second->GetTexture() != nullptr)
                Formats.push_back(it.first);
        }
    }
    std::sort(Formats.begin(), Formats.end());

    return Formats;
}

VertexPoolUsageStats ResourceManager::GetVertexPoolUsageStats(const VertexLayoutKey& Key)
{
    VertexPoolUsageStats Stats;

    auto UpdateStats = [&Stats](const std::vector<RefCntAutoPtr<IVertexPool>>& Pools) {
        for (const auto& Pool : Pools)
        {
            VertexPoolUsageStats PoolStats;
            Pool->GetUsageStats(PoolStats);
            Stats += PoolStats;
        }
    };

    {
        std::shared_lock<std::shared_mutex> SharedLock{m_VertexPoolsMtx};
        if (Key != VertexLayoutKey{})
        {
            const auto pools_it = m_VertexPools.find(Key);
            if (pools_it != m_VertexPools.end())
                UpdateStats(pools_it->second);
        }
        else
        {
            for (const auto& it : m_VertexPools)
                UpdateStats(it.second);
        }
    }

    return Stats;
}

void ResourceManager::TransitionResourceStates(IRenderDevice* pDevice, IDeviceContext* pContext, const TransitionResourceStatesInfo& Info)
{
    m_Barriers.clear();

    if (Info.VertexBuffers.NewState != RESOURCE_STATE_UNKNOWN)
    {
        for (IVertexPool* pPool : GetVertexPoolSnapshot())
        {
            const VertexPoolDesc& Desc = pPool->GetDesc();
            for (Uint32 elem = 0; elem < Desc.NumElements; ++elem)
            {
                IBuffer* pVertBuffer = Info.VertexBuffers.Update ?
                    pPool->Update(elem, pDevice, pContext) :
                    pPool->GetBuffer(elem);
                if (pVertBuffer != nullptr)
                {
                    m_Barriers.emplace_back(pVertBuffer, Info.VertexBuffers.OldState, Info.VertexBuffers.NewState, Info.VertexBuffers.Flags);
                }
            }
        }

        m_VertexPoolSnapshot.clear();
    }

    if (Info.IndexBuffer.NewState != RESOURCE_STATE_UNKNOWN)
    {
        for (IBufferSuballocator* pAllocator : GetIndexAllocatorSnapshot())
        {
            IBuffer* pIndexBuffer = Info.IndexBuffer.Update ?
                pAllocator->Update(pDevice, pContext) :
                pAllocator->GetBuffer();
            if (pIndexBuffer != nullptr)
            {
                m_Barriers.emplace_back(pIndexBuffer, Info.IndexBuffer.OldState, Info.IndexBuffer.NewState, Info.IndexBuffer.Flags);
            }
        }

        m_IndexAllocatorSnapshot.clear();
    }

    if (Info.TextureAtlases.NewState != RESOURCE_STATE_UNKNOWN)
    {
        for (IDynamicTextureAtlas* pAtlas : GetAtlasSnapshot())
        {
            ITexture* pTexture = Info.TextureAtlases.Update ?
                pAtlas->Update(pDevice, pContext) :
                pAtlas->GetTexture();
            if (pTexture != nullptr)
            {
                m_Barriers.emplace_back(pTexture, Info.TextureAtlases.OldState, Info.TextureAtlases.NewState, Info.TextureAtlases.Flags);
            }
        }

        m_AtlasSnapshot.clear();
    }

    if (!m_Barriers.empty())
    {
        pContext->TransitionResourceStates(static_cast<Uint32>(m_Barriers.size()), m_Barriers.data());
    }
}

} // namespace GLTF

} // namespace Diligent
