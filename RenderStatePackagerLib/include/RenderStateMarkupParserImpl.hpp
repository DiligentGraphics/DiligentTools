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

#include "RenderStateMarkupParser.h"
#include "RefCntAutoPtr.hpp"
#include "ObjectBase.hpp"
#include "DynamicLinearAllocator.hpp"
#include "HashUtils.hpp"

namespace Diligent
{

/// Implementation of IDeviceObjectDescriptorParser
class RenderStateMarkupParserImpl final : public ObjectBase<IRenderStateMarkupParser>
{
public:
    using TBase = ObjectBase<IRenderStateMarkupParser>;

    template <typename Type>
    using TNamedObjectHashMap = std::unordered_map<HashMapStringKey, Type, HashMapStringKey::Hasher>;

public:
    RenderStateMarkupParserImpl(IReferenceCounters* pRefCounters, const Char* StrData);

    IMPLEMENT_QUERY_INTERFACE_IN_PLACE(IID_DeviceObjectDescriptorParser, TBase);

    virtual Bool DILIGENT_CALL_TYPE GetGraphicsPipelineStateByName(const Char* Name, GraphicsPipelineStateCreateInfo& CreateInfo, GraphicsPipelineStateCreateMarkup& CreateMarkup) const override final;

    virtual Bool DILIGENT_CALL_TYPE GetComputePipelineStateByName(const Char* Name, ComputePipelineStateCreateInfo& CreateInfo, ComputePipelineStateCreateMarkup& CreateMarkup) const override final;

    virtual Bool DILIGENT_CALL_TYPE GetTilePipelineStateByName(const Char* Name, TilePipelineStateCreateInfo& CreateInfo, TilePipelineStateCreateMarkup& CreateMarkup) const override final;

    virtual Bool DILIGENT_CALL_TYPE GetRayTracingPipelineStateByName(const Char* Name, RayTracingPipelineStateCreateInfo& CreateInfo, RayTracingPipelineStateCreateMarkup& CreateMarkup) const override final;

    virtual Bool DILIGENT_CALL_TYPE GetResourceSignatureByName(const Char* Name, PipelineResourceSignatureDesc& CreateInfo) const override final;

    virtual Bool DILIGENT_CALL_TYPE GetShaderByName(const Char* Name, ShaderCreateInfo& CreateInfo) const override final;

    virtual Bool DILIGENT_CALL_TYPE GetRenderPassByName(const Char* Name, RenderPassDesc& CreateInfo) const override final;

    virtual Bool DILIGENT_CALL_TYPE GetGraphicsPipelineStateByIndex(Uint32 Index, GraphicsPipelineStateCreateInfo& CreateInfo, GraphicsPipelineStateCreateMarkup& CreateMarkup) const override final;

    virtual Bool DILIGENT_CALL_TYPE GetComputePipelineStateByIndex(Uint32 Index, ComputePipelineStateCreateInfo& CreateInfo, ComputePipelineStateCreateMarkup& CreateMarkup) const override final;

    virtual Bool DILIGENT_CALL_TYPE GetTilePipelineStateByIndex(Uint32 Index, TilePipelineStateCreateInfo& CreateInfo, TilePipelineStateCreateMarkup& CreateMarkup) const override final;

    virtual Bool DILIGENT_CALL_TYPE GetRayTracingPipelineStateByIndex(Uint32 Index, RayTracingPipelineStateCreateInfo& CreateInfo, RayTracingPipelineStateCreateMarkup& CreateMarkup) const override final;

    virtual Bool DILIGENT_CALL_TYPE GetResourceSignatureByIndex(Uint32 Index, PipelineResourceSignatureDesc& CreateInfo) const override final;

    virtual Bool DILIGENT_CALL_TYPE GetShaderByIndex(Uint32 Index, ShaderCreateInfo& CreateInfo) const override final;

    virtual Bool DILIGENT_CALL_TYPE GetRenderPassByIndex(Uint32 Index, RenderPassDesc& CreateInfo) const override final;

    virtual Uint32 DILIGENT_CALL_TYPE GetGraphicsPipelineStateCount() const override final;

    virtual Uint32 DILIGENT_CALL_TYPE GetComputePipelineStateCount() const override final;

    virtual Uint32 DILIGENT_CALL_TYPE GetRayTracingPipelineStateCount() const override final;

    virtual Uint32 DILIGENT_CALL_TYPE GetTilePipelineStateCount() const override final;

    virtual Uint32 DILIGENT_CALL_TYPE GetResourceSignatureCount() const override final;

    virtual Uint32 DILIGENT_CALL_TYPE GetShaderCount() const override final;

    virtual Uint32 DILIGENT_CALL_TYPE GetRenderPassCount() const override final;

private:
    std::unique_ptr<DynamicLinearAllocator> m_pAllocator;

    std::vector<GraphicsPipelineStateCreateInfo>   m_GraphicsPipelineStates;
    std::vector<ComputePipelineStateCreateInfo>    m_ComputePipelineStates;
    std::vector<RayTracingPipelineStateCreateInfo> m_RayTracingPipelineStates;
    std::vector<TilePipelineStateCreateInfo>       m_TilePipelineStates;

    std::vector<PipelineResourceSignatureDesc> m_ResourceSignatures;
    std::vector<ShaderCreateInfo>              m_Shaders;
    std::vector<RenderPassDesc>                m_RenderPasses;

    std::vector<GraphicsPipelineStateCreateMarkup>   m_JSONGraphicsPipelineStates;
    std::vector<ComputePipelineStateCreateMarkup>    m_JSONComputePipelineStates;
    std::vector<RayTracingPipelineStateCreateMarkup> m_JSONRayTracingPipelineStates;
    std::vector<TilePipelineStateCreateMarkup>       m_JSONTilePipelineStates;

    TNamedObjectHashMap<Uint32> m_ResourceSignatureNames;
    TNamedObjectHashMap<Uint32> m_ShaderNames;
    TNamedObjectHashMap<Uint32> m_RenderPassNames;

    TNamedObjectHashMap<Uint32> m_GraphicsPipelineNames;
    TNamedObjectHashMap<Uint32> m_ComputePipelineNames;
    TNamedObjectHashMap<Uint32> m_RayTracingPipelineNames;
    TNamedObjectHashMap<Uint32> m_TilePipelineNames;
};

} // namespace Diligent
