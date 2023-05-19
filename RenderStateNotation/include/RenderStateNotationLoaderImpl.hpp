/*
 *  Copyright 2019-2022 Diligent Graphics LLC
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

#include <unordered_map>

#include "RenderStateNotationLoader.h"
#include "RefCntAutoPtr.hpp"
#include "ObjectBase.hpp"
#include "HashUtils.hpp"
#include "RenderStateCache.hpp"

namespace Diligent
{

/// Implementation of IRenderStateNotationLoader
class RenderStateNotationLoaderImpl final : public ObjectBase<IRenderStateNotationLoader>
{
public:
    using TBase = ObjectBase<IRenderStateNotationLoader>;

public:
    RenderStateNotationLoaderImpl(IReferenceCounters*                        pRefCounters,
                                  const RenderStateNotationLoaderCreateInfo& CreateInfo);

    IMPLEMENT_QUERY_INTERFACE_IN_PLACE(IID_RenderStateNotationLoader, TBase)

    virtual void DILIGENT_CALL_TYPE LoadPipelineState(const LoadPipelineStateInfo& LoadInfo, IPipelineState** ppPSO) override final;

    virtual void DILIGENT_CALL_TYPE LoadResourceSignature(const LoadResourceSignatureInfo& LoadInfo, IPipelineResourceSignature** ppSignature) override final;

    virtual void DILIGENT_CALL_TYPE LoadRenderPass(const LoadRenderPassInfo& LoadInfo, IRenderPass** ppRenderPass) override final;

    virtual void DILIGENT_CALL_TYPE LoadShader(const LoadShaderInfo& LoadInfo, IShader** ppShader) override final;

    virtual bool DILIGENT_CALL_TYPE Reload() override final;

private:
    struct PipelineHasher
    {
        size_t operator()(const std::pair<HashMapStringKey, PIPELINE_TYPE>& Key) const
        {
            return ComputeHash(Key.first.GetHash(), static_cast<size_t>(Key.second));
        }
    };

    template <typename Type>
    using TNamedObjectHashMap = std::unordered_map<HashMapStringKey, Type>;

    template <typename Type>
    using TNamedPipelineHashMap = std::unordered_map<std::pair<HashMapStringKey, PIPELINE_TYPE>, Type, PipelineHasher>;

    TNamedPipelineHashMap<RefCntAutoPtr<IPipelineState>>           m_PipelineStateCache;
    TNamedObjectHashMap<RefCntAutoPtr<IPipelineResourceSignature>> m_ResourceSignatureCache;
    TNamedObjectHashMap<RefCntAutoPtr<IRenderPass>>                m_RenderPassCache;
    TNamedObjectHashMap<RefCntAutoPtr<IShader>>                    m_ShaderCache;

    RenderDeviceWithCache<true>                    m_DeviceWithCache;
    RefCntAutoPtr<IRenderStateNotationParser>      m_pParser;
    RefCntAutoPtr<IShaderSourceInputStreamFactory> m_pStreamFactory;
};

} // namespace Diligent
