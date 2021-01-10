/*
 *  Copyright 2019-2021 Diligent Graphics LLC
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

RefCntAutoPtr<ResourceManager> ResourceManager::Create(IRenderDevice*    pDevice,
                                                       const CreateInfo& CI)
{
    return RefCntAutoPtr<ResourceManager>{MakeNewRCObj<ResourceManager>()(pDevice, CI)};
}

ResourceManager::ResourceManager(IReferenceCounters* pRefCounters,
                                 IRenderDevice*      pDevice,
                                 const CreateInfo&   CI) :
    TBase{pRefCounters},
    m_DefaultAtlasDesc{CI.DefaultAtlasDesc},
    m_DefaultAtlasName{CI.DefaultAtlasDesc.Desc.Name != nullptr ? CI.DefaultAtlasDesc.Desc.Name : "GLTF texture atlas"}
{
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

    m_DefaultAtlasDesc.Desc.Name = m_DefaultAtlasName.c_str();
    m_BufferSuballocators.resize(CI.NumBuffSuballocators);
    for (Uint32 i = 0; i < CI.NumBuffSuballocators; ++i)
    {
        CreateBufferSuballocator(pDevice, CI.BuffSuballocators[i], &m_BufferSuballocators[i]);
    }

    m_Atlases.reserve(CI.NumTexAtlases);
    for (Uint32 i = 0; i < CI.NumTexAtlases; ++i)
    {
        RefCntAutoPtr<IDynamicTextureAtlas> pAtlas;
        CreateDynamicTextureAtlas(pDevice, CI.TexAtlases[i], &pAtlas);
        m_Atlases.emplace(CI.TexAtlases[i].Desc.Format, std::move(pAtlas));
    }
}

RefCntAutoPtr<ITextureAtlasSuballocation> ResourceManager::FindAllocation(const char* CacheId)
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
        pAllocation = FindAllocation(CacheId);
    }

    if (!pAllocation)
    {
        decltype(m_Atlases)::iterator cache_it; // NB: can't initialize it without locking the mutex
        {
            std::lock_guard<std::mutex> Lock{m_AtlasesMtx};
            cache_it = m_Atlases.find(Fmt);
            if (cache_it == m_Atlases.end())
            {
                // clang-format off
                DEV_CHECK_ERR(m_DefaultAtlasDesc.Desc.Width  > 0 &&
                              m_DefaultAtlasDesc.Desc.Height > 0 &&
                              m_DefaultAtlasDesc.Desc.Type   != RESOURCE_DIM_UNDEFINED,
                              "Default texture description is not initialized");
                // clang-format on

                auto AtalsCreateInfo        = m_DefaultAtlasDesc;
                AtalsCreateInfo.Desc.Format = Fmt;

                RefCntAutoPtr<IDynamicTextureAtlas> pAtlas;
                CreateDynamicTextureAtlas(nullptr, AtalsCreateInfo, &pAtlas);
                DEV_CHECK_ERR(pAtlas, "Failed to create new texture atlas");

                cache_it = m_Atlases.emplace(Fmt, std::move(pAtlas)).first;
            }
        }
        // Allocate outside of mutex
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

} // namespace GLTF

} // namespace Diligent
