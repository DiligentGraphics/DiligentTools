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

#include "pch.h"
#include "RenderStateMarkupParserImpl.hpp"
#include "FileWrapper.hpp"
#include "DataBlobImpl.hpp"
#include "DefaultRawMemoryAllocator.hpp"

namespace Diligent
{

RenderStateMarkupParserImpl::RenderStateMarkupParserImpl(IReferenceCounters* pRefCounters, const Char* StrData) :
    TBase{pRefCounters}
{
    m_pAllocator        = std::make_unique<DynamicLinearAllocator>(DefaultRawMemoryAllocator::GetAllocator());
    nlohmann::json Json = nlohmann::json::parse(StrData);

    for (auto const& Signature : Json["ResourceSignatures"])
    {
        PipelineResourceSignatureDesc ResourceDesc = {};
        Deserialize(Signature, ResourceDesc, *m_pAllocator);
        VERIFY_EXPR(ResourceDesc.Name != nullptr);
        m_ResourceSignatureNames.emplace(HashMapStringKey{ResourceDesc.Name, false}, static_cast<Uint32>(m_ResourceSignatures.size()));
        m_ResourceSignatures.push_back(ResourceDesc);
    }

    for (auto const& Shader : Json["Shaders"])
    {
        ShaderCreateInfo ResourceDesc = {};
        Deserialize(Shader, ResourceDesc, *m_pAllocator);
        VERIFY_EXPR(ResourceDesc.Desc.Name != nullptr);
        m_ShaderNames.emplace(HashMapStringKey{ResourceDesc.Desc.Name, false}, static_cast<Uint32>(m_Shaders.size()));
        m_Shaders.push_back(ResourceDesc);
    }

    for (auto const& Shader : Json["RenderPasses"])
    {
        RenderPassDesc ResourceDesc = {};
        Deserialize(Shader, ResourceDesc, *m_pAllocator);
        VERIFY_EXPR(ResourceDesc.Name != nullptr);
        m_RenderPassNames.emplace(HashMapStringKey{ResourceDesc.Name, false}, static_cast<Uint32>(m_RenderPasses.size()));
        m_RenderPasses.push_back(ResourceDesc);
    }

    for (auto const& Pipeline : Json["Pipeleines"])
    {
        auto& PipelineType = Pipeline["PSODesc"]["PipelineType"];

        switch (PipelineType.get<PIPELINE_TYPE>())
        {
            case PIPELINE_TYPE_GRAPHICS:
            case PIPELINE_TYPE_MESH:
            {
                GraphicsPipelineStateCreateInfo ResourceDesc = {};
                Deserialize(Pipeline, ResourceDesc, *m_pAllocator);

                GraphicsPipelineStateCreateMarkup JSONResourceDesc = {};
                Deserialize(Pipeline, JSONResourceDesc, *m_pAllocator);

                VERIFY_EXPR(ResourceDesc.PSODesc.Name != nullptr);
                m_GraphicsPipelineStates.push_back(ResourceDesc);
                m_JSONGraphicsPipelineStates.push_back(JSONResourceDesc);
                m_GraphicsPipelineNames.emplace(HashMapStringKey{ResourceDesc.PSODesc.Name, false}, static_cast<Uint32>(m_GraphicsPipelineStates.size()));
                break;
            }
            case PIPELINE_TYPE_COMPUTE:
            {
                ComputePipelineStateCreateInfo ResourceDesc = {};
                Deserialize(Pipeline, ResourceDesc, *m_pAllocator);

                ComputePipelineStateCreateMarkup JSONResourceDesc = {};
                Deserialize(Pipeline, JSONResourceDesc, *m_pAllocator);

                VERIFY_EXPR(ResourceDesc.PSODesc.Name != nullptr);
                m_ComputePipelineStates.push_back(ResourceDesc);
                m_JSONComputePipelineStates.push_back(JSONResourceDesc);
                m_ComputePipelineNames.emplace(HashMapStringKey{ResourceDesc.PSODesc.Name, false}, static_cast<Uint32>(m_ComputePipelineStates.size()));
                break;
            }
            case PIPELINE_TYPE_RAY_TRACING:
            {
                RayTracingPipelineStateCreateInfo ResourceDesc = {};
                Deserialize(Pipeline, ResourceDesc, *m_pAllocator);

                RayTracingPipelineStateCreateMarkup JSONResourceDesc = {};
                Deserialize(Pipeline, JSONResourceDesc, *m_pAllocator);

                VERIFY_EXPR(ResourceDesc.PSODesc.Name != nullptr);
                m_RayTracingPipelineStates.push_back(ResourceDesc);
                m_JSONRayTracingPipelineStates.push_back(JSONResourceDesc);
                m_RayTracingPipelineNames.emplace(HashMapStringKey{ResourceDesc.PSODesc.Name, false}, static_cast<Uint32>(m_RayTracingPipelineStates.size()));
                break;
            }
            case PIPELINE_TYPE_TILE:
            {
                TilePipelineStateCreateInfo ResourceDesc = {};
                Deserialize(Pipeline, ResourceDesc, *m_pAllocator);

                TilePipelineStateCreateMarkup JSONResourceDesc = {};
                Deserialize(Pipeline, JSONResourceDesc, *m_pAllocator);

                VERIFY_EXPR(ResourceDesc.PSODesc.Name != nullptr);
                m_TilePipelineStates.push_back(ResourceDesc);
                m_JSONTilePipelineStates.push_back(JSONResourceDesc);
                m_TilePipelineNames.emplace(HashMapStringKey{ResourceDesc.PSODesc.Name, false}, static_cast<Uint32>(m_TilePipelineStates.size()));
                break;
            }

            default:
                LOG_FATAL_ERROR("Don't correct PipelineType -> '", PipelineType.get<std::string>(), "'.");
                break;
        }
    }
}

Bool RenderStateMarkupParserImpl::GetGraphicsPipelineStateByName(const Char* Name, GraphicsPipelineStateCreateInfo& CreateInfo, GraphicsPipelineStateCreateMarkup& CreateMarkup) const
{
    auto Iter = m_GraphicsPipelineNames.find(Name);

    if (Iter != m_GraphicsPipelineNames.end())
    {
        CreateInfo   = m_GraphicsPipelineStates[Iter->second];
        CreateMarkup = m_JSONGraphicsPipelineStates[Iter->second];
        return true;
    }

    return false;
}

Bool RenderStateMarkupParserImpl::GetComputePipelineStateByName(const Char* Name, ComputePipelineStateCreateInfo& CreateInfo, ComputePipelineStateCreateMarkup& CreateMarkup) const
{
    auto Iter = m_ComputePipelineNames.find(Name);

    if (Iter != m_ComputePipelineNames.end())
    {
        CreateInfo   = m_ComputePipelineStates[Iter->second];
        CreateMarkup = m_JSONComputePipelineStates[Iter->second];
        return true;
    }

    return false;
}

Bool RenderStateMarkupParserImpl::GetTilePipelineStateByName(const Char* Name, TilePipelineStateCreateInfo& CreateInfo, TilePipelineStateCreateMarkup& CreateMarkup) const
{
    auto Iter = m_TilePipelineNames.find(Name);

    if (Iter != m_TilePipelineNames.end())
    {
        CreateInfo   = m_TilePipelineStates[Iter->second];
        CreateMarkup = m_JSONTilePipelineStates[Iter->second];
        return true;
    }

    return false;
}

Bool RenderStateMarkupParserImpl::GetRayTracingPipelineStateByName(const Char* Name, RayTracingPipelineStateCreateInfo& CreateInfo, RayTracingPipelineStateCreateMarkup& CreateMarkup) const
{
    auto Iter = m_RayTracingPipelineNames.find(Name);

    if (Iter != m_RayTracingPipelineNames.end())
    {
        CreateInfo   = m_RayTracingPipelineStates[Iter->second];
        CreateMarkup = m_JSONRayTracingPipelineStates[Iter->second];
        return true;
    }

    return false;
}

Bool RenderStateMarkupParserImpl::GetResourceSignatureByName(const Char* Name, PipelineResourceSignatureDesc& CreateInfo) const
{
    auto Iter = m_ResourceSignatureNames.find(Name);

    if (Iter != m_ResourceSignatureNames.end())
    {
        CreateInfo = m_ResourceSignatures[Iter->second];
        return true;
    }

    return false;
}

Bool RenderStateMarkupParserImpl::GetShaderByName(const Char* Name, ShaderCreateInfo& CreateInfo) const
{
    auto Iter = m_ShaderNames.find(Name);

    if (Iter != m_ShaderNames.end())
    {
        CreateInfo = m_Shaders[Iter->second];
        return true;
    }

    return false;
}

Bool RenderStateMarkupParserImpl::GetRenderPassByName(const Char* Name, RenderPassDesc& CreateInfo) const
{
    auto Iter = m_RenderPassNames.find(Name);

    if (Iter != m_RenderPassNames.end())
    {
        CreateInfo = m_RenderPasses[Iter->second];
        return true;
    }

    return false;
}

Bool RenderStateMarkupParserImpl::GetGraphicsPipelineStateByIndex(Uint32 Index, GraphicsPipelineStateCreateInfo& CreateInfo, GraphicsPipelineStateCreateMarkup& CreateMarkup) const
{
    if (Index < m_GraphicsPipelineStates.size())
    {
        CreateInfo   = m_GraphicsPipelineStates[Index];
        CreateMarkup = m_JSONGraphicsPipelineStates[Index];
        return true;
    }

    return false;
}

Bool RenderStateMarkupParserImpl::GetComputePipelineStateByIndex(Uint32 Index, ComputePipelineStateCreateInfo& CreateInfo, ComputePipelineStateCreateMarkup& CreateMarkup) const
{
    if (Index < m_ComputePipelineStates.size())
    {
        CreateInfo   = m_ComputePipelineStates[Index];
        CreateMarkup = m_JSONComputePipelineStates[Index];
        return true;
    }

    return false;
}

Bool RenderStateMarkupParserImpl::GetTilePipelineStateByIndex(Uint32 Index, TilePipelineStateCreateInfo& CreateInfo, TilePipelineStateCreateMarkup& CreateMarkup) const
{
    if (Index < m_TilePipelineStates.size())
    {
        CreateInfo   = m_TilePipelineStates[Index];
        CreateMarkup = m_JSONTilePipelineStates[Index];
        return true;
    }

    return false;
}

Bool RenderStateMarkupParserImpl::GetRayTracingPipelineStateByIndex(Uint32 Index, RayTracingPipelineStateCreateInfo& CreateInfo, RayTracingPipelineStateCreateMarkup& CreateMarkup) const
{
    if (Index < m_RayTracingPipelineStates.size())
    {
        CreateInfo   = m_RayTracingPipelineStates[Index];
        CreateMarkup = m_JSONRayTracingPipelineStates[Index];
        return true;
    }

    return false;
}

Bool RenderStateMarkupParserImpl::GetResourceSignatureByIndex(Uint32 Index, PipelineResourceSignatureDesc& CreateInfo) const
{
    if (Index < m_ResourceSignatures.size())
    {
        CreateInfo = m_ResourceSignatures[Index];
        return true;
    }

    return false;
}

Bool RenderStateMarkupParserImpl::GetShaderByIndex(Uint32 Index, ShaderCreateInfo& CreateInfo) const
{
    if (Index < m_Shaders.size())
    {
        CreateInfo = m_Shaders[Index];
        return true;
    }

    return false;
}

Bool RenderStateMarkupParserImpl::GetRenderPassByIndex(Uint32 Index, RenderPassDesc& CreateInfo) const
{
    if (Index < m_RenderPasses.size())
    {
        CreateInfo = m_RenderPasses[Index];
        return true;
    }

    return false;
}

Uint32 RenderStateMarkupParserImpl::GetGraphicsPipelineStateCount() const
{
    return static_cast<Uint32>(m_GraphicsPipelineNames.size());
}

Uint32 RenderStateMarkupParserImpl::GetComputePipelineStateCount() const
{
    return static_cast<Uint32>(m_ComputePipelineNames.size());
}

Uint32 RenderStateMarkupParserImpl::GetRayTracingPipelineStateCount() const
{
    return static_cast<Uint32>(m_RayTracingPipelineNames.size());
}

Uint32 RenderStateMarkupParserImpl::GetTilePipelineStateCount() const
{
    return static_cast<Uint32>(m_TilePipelineNames.size());
}

Uint32 RenderStateMarkupParserImpl::GetResourceSignatureCount() const
{
    return static_cast<Uint32>(m_ResourceSignatureNames.size());
}

Uint32 RenderStateMarkupParserImpl::GetShaderCount() const
{
    return static_cast<Uint32>(m_ShaderNames.size());
}

Uint32 RenderStateMarkupParserImpl::GetRenderPassCount() const
{
    return static_cast<Uint32>(m_RenderPassNames.size());
}

void CreateRenderStateMarkupParserFromFile(const char*                FilePath,
                                           IRenderStateMarkupParser** ppParser)
{
    try
    {
        FileWrapper File{FilePath, EFileAccessMode::Read};
        if (!File)
            LOG_ERROR_AND_THROW("Failed to open file '", FilePath, "'.");

        RefCntAutoPtr<IDataBlob> pFileData{MakeNewRCObj<DataBlobImpl>()(0)};
        File->Read(pFileData);

        String                                  Source(reinterpret_cast<const char*>(pFileData->GetConstDataPtr()), pFileData->GetSize());
        RefCntAutoPtr<IRenderStateMarkupParser> pParser{MakeNewRCObj<RenderStateMarkupParserImpl>()(Source.c_str())};
        if (pParser)
            pParser->QueryInterface(IID_DeviceObjectDescriptorParser, reinterpret_cast<IObject**>(ppParser));
    }
    catch (std::runtime_error& err)
    {
        LOG_ERROR("Failed to create descriptor parser from file: ", err.what());
    }
}

void CreateRenderStateMarkupParserFromString(const char*                pData,
                                             IRenderStateMarkupParser** ppParser)
{
    VERIFY_EXPR(pData != nullptr);
    try
    {
        RefCntAutoPtr<IRenderStateMarkupParser> pParser{MakeNewRCObj<RenderStateMarkupParserImpl>()(pData)};
        if (pParser)
            pParser->QueryInterface(IID_DeviceObjectDescriptorParser, reinterpret_cast<IObject**>(ppParser));
    }
    catch (std::runtime_error& err)
    {
        LOG_ERROR("Failed to create descriptor parser from string: ", err.what());
    }
}

} // namespace Diligent

extern "C"
{
    void Diligent_CreateRenderStateMarkupParserFromFile(const char*                          FilePath,
                                                        Diligent::IRenderStateMarkupParser** ppLoader)
    {
        Diligent::CreateRenderStateMarkupParserFromFile(FilePath, ppLoader);
    }

    void Diligent_CreateRenderStateMarkupParserFromString(const char*                          pData,
                                                          Diligent::IRenderStateMarkupParser** ppLoader)
    {
        Diligent::CreateRenderStateMarkupParserFromString(pData, ppLoader);
    }
}
