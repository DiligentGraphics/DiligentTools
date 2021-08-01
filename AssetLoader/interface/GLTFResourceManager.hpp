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

#pragma once

#include <mutex>
#include <vector>
#include <unordered_map>
#include <atomic>

#include "../../../DiligentCore/Graphics/GraphicsEngine/interface/RenderDevice.h"
#include "../../../DiligentCore/Graphics/GraphicsEngine/interface/DeviceContext.h"
#include "../../../DiligentCore/Common/interface/RefCntAutoPtr.hpp"
#include "../../../DiligentCore/Common/interface/ObjectBase.hpp"
#include "../../../DiligentCore/Graphics/GraphicsTools/interface/BufferSuballocator.h"
#include "../../../DiligentCore/Graphics/GraphicsTools/interface/DynamicTextureAtlas.h"


namespace Diligent
{

namespace GLTF
{

/// GLTF resource manager
class ResourceManager final : public ObjectBase<IObject>
{
public:
    using TBase = ObjectBase<IObject>;

    struct CreateInfo
    {
        const BufferSuballocatorCreateInfo*  BuffSuballocators = nullptr; // [NumBuffSuballocators]
        const DynamicTextureAtlasCreateInfo* TexAtlases        = nullptr; // [NumTexAtlases]

        Uint32 NumBuffSuballocators = 0;
        Uint32 NumTexAtlases        = 0;

        DynamicTextureAtlasCreateInfo DefaultAtlasDesc;
    };

    static RefCntAutoPtr<ResourceManager> Create(IRenderDevice*    pDevice,
                                                 const CreateInfo& CI);

    RefCntAutoPtr<IBufferSuballocation> AllocateBufferSpace(Uint32 BufferIndex,
                                                            Uint32 Size,
                                                            Uint32 Alignment)
    {
        RefCntAutoPtr<IBufferSuballocation> pSuballoc;
        m_BufferSuballocators[BufferIndex]->Allocate(Size, Alignment, &pSuballoc);
        return pSuballoc;
    }

    RefCntAutoPtr<ITextureAtlasSuballocation> AllocateTextureSpace(TEXTURE_FORMAT Fmt,
                                                                   Uint32         Width,
                                                                   Uint32         Height,
                                                                   const char*    CacheId   = nullptr,
                                                                   IObject*       pUserData = nullptr);

    RefCntAutoPtr<ITextureAtlasSuballocation> FindAllocation(const char* CacheId);

    Uint32 GetTextureVersion()
    {
        Uint32 Version = 0;

        std::lock_guard<std::mutex> Lock{m_AtlasesMtx};
        for (auto atlas_it : m_Atlases)
            Version += atlas_it.second->GetVersion();

        return Version;
    }

    Uint32 GetBufferVersion(Uint32 Index) const
    {
        return m_BufferSuballocators[Index]->GetVersion();
    }

    IBuffer* GetBuffer(Uint32 Index, IRenderDevice* pDevice, IDeviceContext* pContext)
    {
        return m_BufferSuballocators[Index]->GetBuffer(pDevice, pContext);
    }

    ITexture* GetTexture(TEXTURE_FORMAT Fmt, IRenderDevice* pDevice, IDeviceContext* pContext)
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

    BufferSuballocatorUsageStats GetBufferUsageStats(Uint32 Index)
    {
        BufferSuballocatorUsageStats Stats;
        m_BufferSuballocators[Index]->GetUsageStats(Stats);
        return Stats;
    }

    // NB: can't return reference here!
    TextureDesc GetAtlasDesc(TEXTURE_FORMAT Fmt)
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

    DynamicTextureAtlasUsageStats GetAtlasUsageStats(TEXTURE_FORMAT Fmt = TEX_FORMAT_UNKNOWN)
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
                    Stats.Size += AtlasStats.Size;
                    Stats.TotalArea += AtlasStats.TotalArea;
                    Stats.AllocatedArea += AtlasStats.AllocatedArea;
                    Stats.UsedArea += AtlasStats.UsedArea;
                    Stats.AllocationCount += AtlasStats.AllocationCount;
                }
            }
        }

        return Stats;
    }

private:
    template <typename AllocatorType, typename ObjectType>
    friend class Diligent::MakeNewRCObj;

    ResourceManager(IReferenceCounters* pRefCounters,
                    IRenderDevice*      pDevice,
                    const CreateInfo&   CI);

    std::vector<RefCntAutoPtr<IBufferSuballocator>> m_BufferSuballocators;

    DynamicTextureAtlasCreateInfo m_DefaultAtlasDesc;
    const std::string             m_DefaultAtlasName;

    using AtlasesHashMapType = std::unordered_map<TEXTURE_FORMAT, RefCntAutoPtr<IDynamicTextureAtlas>, std::hash<Uint32>>;
    std::mutex         m_AtlasesMtx;
    AtlasesHashMapType m_Atlases;

    using TexAllocationsHashMapType = std::unordered_map<std::string, RefCntWeakPtr<ITextureAtlasSuballocation>>;
    std::mutex                m_TexAllocationsMtx;
    TexAllocationsHashMapType m_TexAllocations;
};

} // namespace GLTF

} // namespace Diligent
