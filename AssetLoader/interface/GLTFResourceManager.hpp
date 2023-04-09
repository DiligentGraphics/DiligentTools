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
#include "../../../DiligentCore/Graphics/GraphicsTools/interface/VertexPool.h"

namespace Diligent
{

namespace GLTF
{

/// GLTF resource manager
class ResourceManager final : public ObjectBase<IObject>
{
public:
    using TBase = ObjectBase<IObject>;

    struct VertexLayoutKey
    {
        struct ElementDesc
        {
            const Uint32     Size;
            const BIND_FLAGS BindFlags;

            constexpr ElementDesc(Uint32 _Size, BIND_FLAGS _BindFlags) noexcept :
                Size{_Size},
                BindFlags{_BindFlags}
            {}

            constexpr bool operator==(const ElementDesc& RHS) const
            {
                return Size == RHS.Size && BindFlags == RHS.BindFlags;
            }
            constexpr bool operator!=(const ElementDesc& RHS) const
            {
                return !(*this == RHS);
            }
        };
        std::vector<ElementDesc> Elements;

        bool operator==(const VertexLayoutKey& rhs) const
        {
            return Elements == rhs.Elements;
        }
        bool operator!=(const VertexLayoutKey& rhs) const
        {
            return Elements != rhs.Elements;
        }

        struct Hasher
        {
            size_t operator()(const VertexLayoutKey& Key) const;
        };
    };

    struct DefaultVertexPoolDesc
    {
        const char*      Name           = nullptr;
        Uint32           VertexCount    = 32768;
        USAGE            Usage          = USAGE_DEFAULT;
        CPU_ACCESS_FLAGS CPUAccessFlags = CPU_ACCESS_NONE;
        BUFFER_MODE      Mode           = BUFFER_MODE_UNDEFINED;
    };

    struct CreateInfo
    {
        BufferSuballocatorCreateInfo IndexAllocatorCI;

        const VertexPoolCreateInfo*          pVertexPoolCIs = nullptr; // [NumVertexPools]
        const DynamicTextureAtlasCreateInfo* pTexAtlasCIs   = nullptr; // [NumTexAtlases]

        Uint32 NumVertexPools = 0;
        Uint32 NumTexAtlases  = 0;

        DynamicTextureAtlasCreateInfo DefaultAtlasDesc;
        DefaultVertexPoolDesc         DefaultPoolDesc;
    };

    static RefCntAutoPtr<ResourceManager> Create(IRenderDevice*    pDevice,
                                                 const CreateInfo& CI);

    RefCntAutoPtr<ITextureAtlasSuballocation> AllocateTextureSpace(TEXTURE_FORMAT Fmt,
                                                                   Uint32         Width,
                                                                   Uint32         Height,
                                                                   const char*    CacheId   = nullptr,
                                                                   IObject*       pUserData = nullptr);

    RefCntAutoPtr<ITextureAtlasSuballocation> FindTextureAllocation(const char* CacheId);

    RefCntAutoPtr<IBufferSuballocation>  AllocateIndices(Uint32 Size, Uint32 Alignment);
    RefCntAutoPtr<IVertexPoolAllocation> AllocateVertices(const VertexLayoutKey& LayoutKey, Uint32 VertexCount);

    Uint32 GetTextureVersion();
    Uint32 GetIndexBufferVersion(Uint32 Index) const;
    Uint32 GetVertexPoolsVersion();

    IBuffer*     GetIndexBuffer(IRenderDevice* pDevice, IDeviceContext* pContext);
    IVertexPool* GetVertexPool(const VertexLayoutKey& Key);
    ITexture*    GetTexture(TEXTURE_FORMAT Fmt, IRenderDevice* pDevice, IDeviceContext* pContext);

    // NB: can't return reference here!
    TextureDesc GetAtlasDesc(TEXTURE_FORMAT Fmt);

    Uint32 GetAllocationAlignment(TEXTURE_FORMAT Fmt, Uint32 Width, Uint32 Height);

    BufferSuballocatorUsageStats  GetIndexBufferUsageStats();
    DynamicTextureAtlasUsageStats GetAtlasUsageStats(TEXTURE_FORMAT Fmt = TEX_FORMAT_UNKNOWN);

private:
    template <typename AllocatorType, typename ObjectType>
    friend class Diligent::MakeNewRCObj;

    ResourceManager(IReferenceCounters* pRefCounters,
                    IRenderDevice*      pDevice,
                    const CreateInfo&   CI);

    const RENDER_DEVICE_TYPE m_DeviceType;

    const std::string     m_DefaultVertPoolName;
    DefaultVertexPoolDesc m_DefaultVertPoolDesc;

    const std::string             m_DefaultAtlasName;
    DynamicTextureAtlasCreateInfo m_DefaultAtlasDesc;

    RefCntAutoPtr<IBufferSuballocator> m_pIndexBufferAllocator;

    using VertexPoolsHashMapType = std::unordered_map<VertexLayoutKey, RefCntAutoPtr<IVertexPool>, VertexLayoutKey::Hasher>;
    std::mutex             m_VertexPoolsMtx;
    VertexPoolsHashMapType m_VertexPools;

    using AtlasesHashMapType = std::unordered_map<TEXTURE_FORMAT, RefCntAutoPtr<IDynamicTextureAtlas>, std::hash<Uint32>>;
    std::mutex         m_AtlasesMtx;
    AtlasesHashMapType m_Atlases;

    using TexAllocationsHashMapType = std::unordered_map<std::string, RefCntWeakPtr<ITextureAtlasSuballocation>>;
    std::mutex                m_TexAllocationsMtx;
    TexAllocationsHashMapType m_TexAllocations;
};

} // namespace GLTF

} // namespace Diligent
