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
class GLTFResourceManager : public ObjectBase<IObject>
{
public:
    using TBase = ObjectBase<IObject>;

    struct BufferAllocation
    {
        Int32 BufferIndex = -1;

        VariableSizeAllocationsManager::Allocation Region;

        bool IsValid() const
        {
            VERIFY_EXPR(BufferIndex >= 0 && Region.IsValid() || BufferIndex < 0 && !Region.IsValid());
            return BufferIndex >= 0;
        }
    };
    struct TextureAllocation
    {
        Int32 TextureIndex = -1;

        DynamicAtlasManager::Region Region;

        bool IsValid() const
        {
            VERIFY_EXPR(TextureIndex >= 0 && !Region.IsEmpty() || TextureIndex < 0 && Region.IsEmpty());
            return TextureIndex >= 0;
        }
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


    /// Allocates space in the buffer
    BufferAllocation AllocateBufferSpace(Uint32 BufferIndex, Uint32 Size, Uint32 Alignment);

    void FreeBufferSpace(BufferAllocation&& Allocation);

    IBuffer* GetBuffer(const BufferAllocation& Allocation)
    {
        VERIFY_EXPR(Allocation.IsValid());
        return m_Buffers[Allocation.BufferIndex].pBuffer;
    }

    TextureAllocation AllocateTextureSpace(Uint32 TextureIndex, Uint32 Width, Uint32 Height);

    void FreeTextureSpace(TextureAllocation&& Allocation);

    ITexture* GetTexture(const TextureAllocation& Allocation)
    {
        VERIFY_EXPR(Allocation.IsValid());
        return m_Textures[Allocation.TextureIndex].pTexture;
    }

private:
    template <typename AllocatorType, typename ObjectType>
    friend class MakeNewRCObj;

    GLTFResourceManager(IReferenceCounters* pRefCounters,
                        IRenderDevice*      pDevice,
                        const CreateInfo&   CI);

    struct BufferCache
    {
        BufferCache(IRenderDevice* pDevice, const BufferDesc& BuffDesc);

        // clang-format off
        BufferCache           (const BufferCache&) = delete;
        BufferCache& operator=(const BufferCache&) = delete;
        BufferCache& operator=(BufferCache&&)      = delete;
        // clang-format on

        BufferCache(BufferCache&& Cache) noexcept :
            Mgr{std::move(Cache.Mgr)},
            pBuffer{std::move(Cache.pBuffer)}
        {}

        std::mutex                     Mtx;
        VariableSizeAllocationsManager Mgr;
        RefCntAutoPtr<IBuffer>         pBuffer;
    };
    std::vector<BufferCache> m_Buffers;

    struct TextureCache
    {
        TextureCache(IRenderDevice* pDevice, const TextureCacheAttribs& CacheCI);

        // clang-format off
        TextureCache           (const TextureCache&) = delete;
        TextureCache& operator=(const TextureCache&) = delete;
        TextureCache& operator=(TextureCache&&)      = delete;
        // clang-format on

        TextureCache(TextureCache&& Cache) noexcept :
            Granularity{Cache.Granularity},
            Mgr{std::move(Cache.Mgr)},
            pTexture{std::move(Cache.pTexture)}
        {}

        const Uint32 Granularity;

        std::mutex              Mtx;
        DynamicAtlasManager     Mgr;
        RefCntAutoPtr<ITexture> pTexture;
    };
    std::vector<TextureCache> m_Textures;
};

} // namespace Diligent
