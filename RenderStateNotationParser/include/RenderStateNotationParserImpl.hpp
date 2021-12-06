/*
 *  Copyright 2019-2021 Diligent Graphics LLC
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

#include "RenderStateNotationParser.h"
#include "RefCntAutoPtr.hpp"
#include "ObjectBase.hpp"
#include "DynamicLinearAllocator.hpp"
#include "HashUtils.hpp"

namespace Diligent
{

/// Implementation of IRenderStateNotationParser
class IRenderStateNotationParserImpl final : public ObjectBase<IRenderStateNotationParser>
{
public:
    using TBase = ObjectBase<IRenderStateNotationParser>;

public:
    IRenderStateNotationParserImpl(IReferenceCounters* pRefCounters, const Char* StrData);

    IMPLEMENT_QUERY_INTERFACE_IN_PLACE(IID_RenderStateNotationParser, TBase);

    virtual const GraphicsPipelineNotation* DILIGENT_CALL_TYPE GetGraphicsPipelineStateByName(const Char* Name) const override final;

    virtual const ComputePipelineNotation* DILIGENT_CALL_TYPE GetComputePipelineStateByName(const Char* Name) const override final;

    virtual const RayTracingPipelineNotation* DILIGENT_CALL_TYPE GetRayTracingPipelineStateByName(const Char* Name) const override final;

    virtual const TilePipelineNotation* DILIGENT_CALL_TYPE GetTilePipelineStateByName(const Char* Name) const override final;

    virtual const ResourceSignatureNotation* DILIGENT_CALL_TYPE GetResourceSignatureByName(const Char* Name) const override final;

    virtual const ShaderNotation* DILIGENT_CALL_TYPE GetShaderByName(const Char* Name) const override final;

    virtual const RenderPassNotation* DILIGENT_CALL_TYPE GetRenderPassByName(const Char* Name) const override final;

    virtual const GraphicsPipelineNotation* DILIGENT_CALL_TYPE GetGraphicsPipelineStateByIndex(Uint32 Index) const override final;

    virtual const ComputePipelineNotation* DILIGENT_CALL_TYPE GetComputePipelineStateByIndex(Uint32 Index) const override final;

    virtual const RayTracingPipelineNotation* DILIGENT_CALL_TYPE GetRayTracingPipelineStateByIndex(Uint32 Index) const override final;

    virtual const TilePipelineNotation* DILIGENT_CALL_TYPE GetTilePipelineStateByIndex(Uint32 Index) const override final;

    virtual const ResourceSignatureNotation* DILIGENT_CALL_TYPE GetResourceSignatureByIndex(Uint32 Index) const override final;

    virtual const ShaderNotation* DILIGENT_CALL_TYPE GetShaderByIndex(Uint32 Index) const override final;

    virtual const RenderPassNotation* DILIGENT_CALL_TYPE GetRenderPassByIndex(Uint32 Index) const override final;

    virtual const RenderStateNotationParserInfo& DILIGENT_CALL_TYPE GetInfo() const override final;

private:
    std::unique_ptr<DynamicLinearAllocator> m_pAllocator;

    std::vector<ResourceSignatureNotation> m_ResourceSignatures;
    std::vector<ShaderNotation>            m_Shaders;
    std::vector<RenderPassNotation>        m_RenderPasses;

    std::vector<GraphicsPipelineNotation>   m_GraphicsPipelineStates;
    std::vector<ComputePipelineNotation>    m_ComputePipelineStates;
    std::vector<RayTracingPipelineNotation> m_RayTracingPipelineStates;
    std::vector<TilePipelineNotation>       m_TilePipelineStates;

    template <typename Type>
    using TNamedObjectHashMap = std::unordered_map<HashMapStringKey, Type, HashMapStringKey::Hasher>;

    TNamedObjectHashMap<Uint32> m_ResourceSignatureNames;
    TNamedObjectHashMap<Uint32> m_ShaderNames;
    TNamedObjectHashMap<Uint32> m_RenderPassNames;

    TNamedObjectHashMap<Uint32> m_GraphicsPipelineNames;
    TNamedObjectHashMap<Uint32> m_ComputePipelineNames;
    TNamedObjectHashMap<Uint32> m_RayTracingPipelineNames;
    TNamedObjectHashMap<Uint32> m_TilePipelineNames;

    RenderStateNotationParserInfo m_ParseInfo;
};

} // namespace Diligent
