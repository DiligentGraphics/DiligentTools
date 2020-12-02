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

#pragma once

#include <mutex>
#include <vector>

#include "../../../DiligentCore/Graphics/GraphicsEngine/interface/RenderDevice.h"
#include "../../../DiligentCore/Graphics/GraphicsEngine/interface/DeviceContext.h"
#include "../../../DiligentCore/Common/interface/RefCntAutoPtr.hpp"
#include "../../../DiligentCore/Common/interface/ObjectBase.hpp"
#include "../../../DiligentCore/Graphics/GraphicsAccessories/interface/VariableSizeAllocationsManager.hpp"
#include "../../../DiligentCore/Graphics/GraphicsAccessories/interface/DynamicAtlasManager.hpp"


namespace Diligent
{

/// GLTF resource manager
class GLTFResourceManager final : public ObjectBase<IObject>
{
    class BufferCache;
    class TextureCache;

public:
    using TBase = ObjectBase<IObject>;

    class BufferAllocation final : public ObjectBase<IObject>
    {
    public:
        BufferAllocation(IReferenceCounters*                          pRefCounters,
                         RefCntAutoPtr<GLTFResourceManager>           pResourceMg,
                         BufferCache&                                 ParentCache,
                         VariableSizeAllocationsManager::Allocation&& Region) :
            // clang-format off
            ObjectBase<IObject>{pRefCounters},
            m_pResMgr          {std::move(pResourceMg)},
            m_ParentCache      {ParentCache},
            m_Region           {std::move(Region)}
        // clang-format on
        {
            VERIFY_EXPR(m_Region.IsValid());
        }

        ~BufferAllocation()
        {
            m_ParentCache.FreeAllocation(std::move(m_Region));
        }

        IBuffer* GetBuffer(IRenderDevice* pDevice, IDeviceContext* pContext) const
        {
            return m_ParentCache.GetBuffer(pDevice, pContext);
        }

        const VariableSizeAllocationsManager::Allocation& GetRegion() const
        {
            return m_Region;
        }

    private:
        RefCntAutoPtr<GLTFResourceManager>         m_pResMgr;
        BufferCache&                               m_ParentCache;
        VariableSizeAllocationsManager::Allocation m_Region;
    };

    class TextureAllocation final : public ObjectBase<IObject>
    {
    public:
        TextureAllocation(IReferenceCounters*                pRefCounters,
                          RefCntAutoPtr<GLTFResourceManager> pResourceMg,
                          TextureCache&                      ParentCache,
                          DynamicAtlasManager::Region&&      Region) :
            // clang-format off
            ObjectBase<IObject>{pRefCounters},
            m_pResMgr          {std::move(pResourceMg)},
            m_ParentCache      {ParentCache},
            m_Region           {std::move(Region)}
        // clang-format on
        {
            VERIFY_EXPR(!m_Region.IsEmpty());
        }

        ~TextureAllocation()
        {
            m_ParentCache.FreeAllocation(std::move(m_Region));
        }

        ITexture* GetTexture(IRenderDevice* pDevice, IDeviceContext* pContext) const
        {
            return m_ParentCache.GetTexture(pDevice, pContext);
        }

        const TextureDesc& GetTexDesc() const
        {
            return m_ParentCache.GetTexDesc();
        }

        const DynamicAtlasManager::Region& GetRegion()
        {
            return m_Region;
        }

    private:
        RefCntAutoPtr<GLTFResourceManager> m_pResMgr;
        TextureCache&                      m_ParentCache;
        DynamicAtlasManager::Region        m_Region;
    };

    struct TextureCacheAttribs
    {
        TextureDesc Desc;
        Uint32      Granularity = 128;
    };
    struct CreateInfo
    {
        const BufferDesc* Buffers    = nullptr; // [NumBuffers]
        Uint32            NumBuffers = 0;

        const TextureCacheAttribs* Textures    = nullptr; // [NumTextures]
        Uint32                     NumTextures = 0;
    };

    static RefCntAutoPtr<GLTFResourceManager> Create(IRenderDevice*    pDevice,
                                                     const CreateInfo& CI);

    RefCntAutoPtr<BufferAllocation> AllocateBufferSpace(Uint32 BufferIndex, Uint32 Size, Uint32 Alignment)
    {
        return m_Buffers[BufferIndex].Allocate(Size, Alignment);
    }

    RefCntAutoPtr<TextureAllocation> AllocateTextureSpace(Uint32 TextureIndex, Uint32 Width, Uint32 Height)
    {
        return m_Textures[TextureIndex].Allocate(Width, Height);
    }

private:
    template <typename AllocatorType, typename ObjectType>
    friend class MakeNewRCObj;

    GLTFResourceManager(IReferenceCounters* pRefCounters,
                        IRenderDevice*      pDevice,
                        const CreateInfo&   CI);

    class BufferCache
    {
    public:
        BufferCache(GLTFResourceManager& Owner, IRenderDevice* pDevice, const BufferDesc& BuffDesc);

        // clang-format off
        BufferCache           (const BufferCache&) = delete;
        BufferCache& operator=(const BufferCache&) = delete;
        BufferCache& operator=(BufferCache&&)      = delete;
        // clang-format on

        BufferCache(BufferCache&& Cache) noexcept :
            // clang-format off
            m_Owner  {Cache.m_Owner},
            m_Mgr    {std::move(Cache.m_Mgr)},
            m_pBuffer{std::move(Cache.m_pBuffer)}
        // clang-format on
        {}

        IBuffer* GetBuffer(IRenderDevice* pDevice, IDeviceContext* pContext);

        RefCntAutoPtr<BufferAllocation> Allocate(Uint32 Size, Uint32 Alignment);

        void FreeAllocation(VariableSizeAllocationsManager::Allocation&& Allocation)
        {
            std::lock_guard<std::mutex> Lock{m_Mtx};
            m_Mgr.Free(std::move(Allocation));
        }

    private:
        GLTFResourceManager& m_Owner;

        std::mutex                     m_Mtx;
        VariableSizeAllocationsManager m_Mgr;
        RefCntAutoPtr<IBuffer>         m_pBuffer;
    };
    std::vector<BufferCache> m_Buffers;


    class TextureCache
    {
    public:
        TextureCache(GLTFResourceManager& Owner, IRenderDevice* pDevice, const TextureCacheAttribs& CacheCI);

        // clang-format off
        TextureCache           (const TextureCache&) = delete;
        TextureCache& operator=(const TextureCache&) = delete;
        TextureCache& operator=(TextureCache&&)      = delete;
        // clang-format on

        TextureCache(TextureCache&& Cache) noexcept :
            // clang-format off
            m_Owner      {Cache.m_Owner},
            m_TexDesc    {m_TexDesc},
            m_TexName    {std::move(m_TexName)},
            m_Granularity{Cache.m_Granularity},
            m_Mgr        {std::move(Cache.m_Mgr)},
            m_pTexture   {std::move(Cache.m_pTexture)}
        // clang-format on
        {
            m_TexDesc.Name = m_TexName.c_str();
        }

        ITexture* GetTexture(IRenderDevice* pDevice, IDeviceContext* pContext);

        const TextureDesc& GetTexDesc() const
        {
            return m_TexDesc;
        }

        RefCntAutoPtr<TextureAllocation> Allocate(Uint32 Width, Uint32 Height);

        void FreeAllocation(DynamicAtlasManager::Region&& Allocation);

    private:
        GLTFResourceManager& m_Owner;

        TextureDesc m_TexDesc;
        std::string m_TexName;

        const Uint32 m_Granularity;

        std::mutex              m_Mtx;
        DynamicAtlasManager     m_Mgr;
        RefCntAutoPtr<ITexture> m_pTexture;
    };
    std::vector<TextureCache> m_Textures;
};

} // namespace Diligent
