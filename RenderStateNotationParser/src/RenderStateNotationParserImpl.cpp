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

#include "pch.h"

#include "RenderStateNotationParserImpl.hpp"

#include <unordered_set>
#include <functional>

#include "DataBlobImpl.hpp"
#include "FileWrapper.hpp"
#include "DefaultRawMemoryAllocator.hpp"
#include "GraphicsAccessories.hpp"

namespace Diligent
{

namespace
{

struct InlineStructureCallbacks
{
    std::function<void(const nlohmann::json&, SHADER_TYPE, const char**, DynamicLinearAllocator&)> ShaderCallback;
    std::function<void(const nlohmann::json&, const char**, DynamicLinearAllocator&)>              RenderPassCallback;
    std::function<void(const nlohmann::json&, const char**, DynamicLinearAllocator&)>              ResourceSignatureCallback;
};

template <typename Type, typename TypeSize>
void Deserialize(const nlohmann::json& Json, const Type*& pObjects, TypeSize& NumElements, DynamicLinearAllocator& Allocator, const InlineStructureCallbacks& Callbacks)
{
    if (!Json.is_array())
        throw nlohmann::json::type_error::create(JsonTypeError, std::string("type must be array, but is ") + Json.type_name(), Json);

    auto* pData = Allocator.ConstructArray<Type>(Json.size());
    for (size_t i = 0; i < Json.size(); i++)
        Deserialize(Json[i], pData[i], Allocator, Callbacks);

    pObjects    = pData;
    NumElements = static_cast<TypeSize>(Json.size());
}

void Deserialize(const nlohmann::json& Json, PipelineStateNotation& Type, DynamicLinearAllocator& Allocator, const InlineStructureCallbacks& Callbacks)
{
    if (Json.contains("PSODesc"))
        Deserialize(Json["PSODesc"], Type.PSODesc, Allocator);

    if (Json.contains("Flags"))
        DeserializeBitwiseEnum(Json["Flags"], Type.Flags, Allocator);

    if (Json.contains("ppResourceSignatures"))
    {
        auto const& Signatures = Json["ppResourceSignatures"];

        if (!Signatures.is_array())
            throw nlohmann::json::type_error::create(JsonTypeError, std::string("type must be array, but is ") + Signatures.type_name(), Signatures);

        auto* pData = Allocator.ConstructArray<const char*>(Signatures.size());
        for (size_t i = 0; i < Signatures.size(); i++)
            Callbacks.ResourceSignatureCallback(Signatures[i], &pData[i], Allocator);

        Type.ppResourceSignatureNames    = pData;
        Type.ResourceSignaturesNameCount = StaticCast<Uint32>(Signatures.size());
    }
}

void Deserialize(const nlohmann::json& Json, GraphicsPipelineNotation& Type, DynamicLinearAllocator& Allocator, const InlineStructureCallbacks& Callbacks)
{
    Deserialize(Json, static_cast<PipelineStateNotation&>(Type), Allocator, Callbacks);

    if (Json.contains("GraphicsPipeline"))
    {
        auto& GraphicsPipeline = Json["GraphicsPipeline"];
        Deserialize(GraphicsPipeline, Type.Desc, Allocator);

        if (GraphicsPipeline.contains("pRenderPass"))
            Callbacks.RenderPassCallback(GraphicsPipeline["pRenderPass"], &Type.pRenderPassName, Allocator);
    }

    if (Json.contains("pVS"))
        Callbacks.ShaderCallback(Json["pVS"], SHADER_TYPE_VERTEX, &Type.pVSName, Allocator);

    if (Json.contains("pPS"))
        Callbacks.ShaderCallback(Json["pPS"], SHADER_TYPE_PIXEL, &Type.pPSName, Allocator);

    if (Json.contains("pDS"))
        Callbacks.ShaderCallback(Json["pDS"], SHADER_TYPE_DOMAIN, &Type.pDSName, Allocator);

    if (Json.contains("pHS"))
        Callbacks.ShaderCallback(Json["pHS"], SHADER_TYPE_HULL, &Type.pHSName, Allocator);

    if (Json.contains("pGS"))
        Callbacks.ShaderCallback(Json["pGS"], SHADER_TYPE_GEOMETRY, &Type.pGSName, Allocator);

    if (Json.contains("pAS"))
        Callbacks.ShaderCallback(Json["pAS"], SHADER_TYPE_AMPLIFICATION, &Type.pASName, Allocator);

    if (Json.contains("pMS"))
        Callbacks.ShaderCallback(Json["pMS"], SHADER_TYPE_MESH, &Type.pMSName, Allocator);
}

void Deserialize(const nlohmann::json& Json, ComputePipelineNotation& Type, DynamicLinearAllocator& Allocator, const InlineStructureCallbacks& Callbacks)
{
    Deserialize(Json, static_cast<PipelineStateNotation&>(Type), Allocator, Callbacks);
    Callbacks.ShaderCallback(Json.at("pCS"), SHADER_TYPE_COMPUTE, &Type.pCSName, Allocator);
}

void Deserialize(const nlohmann::json& Json, TilePipelineNotation& Type, DynamicLinearAllocator& Allocator, const InlineStructureCallbacks& Callbacks)
{
    Deserialize(Json, static_cast<PipelineStateNotation&>(Type), Allocator, Callbacks);
    Callbacks.ShaderCallback(Json.at("pTS"), SHADER_TYPE_TILE, &Type.pTSName, Allocator);
}

void Deserialize(const nlohmann::json& Json, RTGeneralShaderGroupNotation& Type, DynamicLinearAllocator& Allocator, const InlineStructureCallbacks& Callbacks)
{
    Deserialize(Json.at("Name"), Type.Name, Allocator);
    Callbacks.ShaderCallback(Json.at("pShader"), SHADER_TYPE_RAY_GEN, &Type.pShaderName, Allocator);
}

void Deserialize(const nlohmann::json& Json, RTTriangleHitShaderGroupNotation& Type, DynamicLinearAllocator& Allocator, const InlineStructureCallbacks& Callbacks)
{
    Deserialize(Json.at("Name"), Type.Name, Allocator);

    if (Json.contains("pClosestHitShader"))
        Callbacks.ShaderCallback(Json["pClosestHitShader"], SHADER_TYPE_RAY_CLOSEST_HIT, &Type.pClosestHitShaderName, Allocator);

    if (Json.contains("pAnyHitShader"))
        Callbacks.ShaderCallback(Json["pAnyHitShader"], SHADER_TYPE_RAY_ANY_HIT, &Type.pAnyHitShaderName, Allocator);
}

void Deserialize(const nlohmann::json& Json, RTProceduralHitShaderGroupNotation& Type, DynamicLinearAllocator& Allocator, const InlineStructureCallbacks& Callbacks)
{
    Deserialize(Json.at("Name"), Type.Name, Allocator);

    if (Json.contains("pIntersectionShader"))
        Callbacks.ShaderCallback(Json["pIntersectionShader"], SHADER_TYPE_RAY_INTERSECTION, &Type.pIntersectionShaderName, Allocator);

    if (Json.contains("pClosestHitShader"))
        Callbacks.ShaderCallback(Json["pClosestHitShader"], SHADER_TYPE_RAY_CLOSEST_HIT, &Type.pClosestHitShaderName, Allocator);

    if (Json.contains("pAnyHitShader"))
        Callbacks.ShaderCallback(Json["pAnyHitShader"], SHADER_TYPE_RAY_ANY_HIT, &Type.pAnyHitShaderName, Allocator);
}

void Deserialize(const nlohmann::json& Json, RayTracingPipelineNotation& Type, DynamicLinearAllocator& Allocator, const InlineStructureCallbacks& Callbacks)
{
    Deserialize(Json, static_cast<PipelineStateNotation&>(Type), Allocator, Callbacks);

    if (Json.contains("RayTracingPipeline"))
        Deserialize(Json["RayTracingPipeline"], Type.RayTracingPipeline, Allocator);

    if (Json.contains("pGeneralShaders"))
        Deserialize(Json["pGeneralShaders"], Type.pGeneralShaders, Type.GeneralShaderCount, Allocator, Callbacks);

    if (Json.contains("pTriangleHitShaders"))
        Deserialize(Json["pTriangleHitShaders"], Type.pTriangleHitShaders, Type.TriangleHitShaderCount, Allocator, Callbacks);

    if (Json.contains("pProceduralHitShaders"))
        Deserialize(Json["pProceduralHitShaders"], Type.pProceduralHitShaders, Type.ProceduralHitShaderCount, Allocator, Callbacks);

    if (Json.contains("pShaderRecordName"))
        Deserialize(Json["pShaderRecordName"], Type.pShaderRecordName, Allocator);

    if (Json.contains("MaxAttributeSize"))
        Deserialize(Json["MaxAttributeSize"], Type.MaxAttributeSize, Allocator);

    if (Json.contains("MaxPayloadSize"))
        Deserialize(Json["MaxPayloadSize"], Type.MaxPayloadSize, Allocator);
}

PIPELINE_TYPE GetImplicitPipelineType(const nlohmann::json& Json)
{
    auto VerifyAndReturn = [](const nlohmann::json& Json, PIPELINE_TYPE Type, const Char* MessageError) -> PIPELINE_TYPE {
        if (Json.at("PSODesc").contains("PipelineType") && Json["PSODesc"]["PipelineType"].get<PIPELINE_TYPE>() != Type)
            throw nlohmann::json::other_error::create(JsonInvalidEnum, std::string(MessageError) + Json["PSODesc"]["PipelineType"].get<std::string>(), Json);
        return Type;
    };

    if (Json.contains("pAS") || Json.contains("pMS"))
        return VerifyAndReturn(Json, PIPELINE_TYPE_MESH, "pipeline type must be MESH, but is ");

    if (Json.contains("pVS") || Json.contains("pPS") || Json.contains("pDS") || Json.contains("pHS") || Json.contains("pGS"))
        return VerifyAndReturn(Json, PIPELINE_TYPE_GRAPHICS, "pipeline type must be GRAPHICS, but is ");

    if (Json.contains("pCS"))
        return VerifyAndReturn(Json, PIPELINE_TYPE_COMPUTE, "pipeline type must be COMPUTE, but is ");

    if (Json.contains("pTS"))
        return VerifyAndReturn(Json, PIPELINE_TYPE_TILE, "pipeline type must be TILE, but is ");

    if (Json.contains("pGeneralShaders") || Json.contains("pTriangleHitShaders") || Json.contains("pProceduralHitShaders"))
        return VerifyAndReturn(Json, PIPELINE_TYPE_RAY_TRACING, "pipeline type must be RAY_TRACING, but is ");

    return PIPELINE_TYPE_INVALID;
}

} // namespace

RenderStateNotationParserImpl::RenderStateNotationParserImpl(IReferenceCounters*                        pRefCounters,
                                                             const RenderStateNotationParserCreateInfo& CreateInfo) :
    TBase{pRefCounters}
{
    VERIFY_EXPR(CreateInfo.StrData != nullptr || (CreateInfo.FilePath != nullptr && CreateInfo.pStreamFactory != nullptr));

    m_pAllocator = std::make_unique<DynamicLinearAllocator>(DefaultRawMemoryAllocator::GetAllocator());

    std::unordered_set<std::string>                                 Includes;
    std::function<void(const RenderStateNotationParserCreateInfo&)> ParseJSON;

    ParseJSON = [this, &ParseJSON, &Includes](const RenderStateNotationParserCreateInfo& ParserCI) {
        try
        {
            String Source;
            if (ParserCI.FilePath != nullptr && ParserCI.pStreamFactory != nullptr)
            {
                VERIFY_EXPR(ParserCI.StrData == nullptr);

                RefCntAutoPtr<IFileStream> pFileStream;
                ParserCI.pStreamFactory->CreateInputStream(ParserCI.FilePath, &pFileStream);

                if (!pFileStream)
                    LOG_ERROR_AND_THROW("Failed to open file: '", ParserCI.FilePath, "'.");

                auto pFileData = DataBlobImpl::Create();
                pFileStream->ReadBlob(pFileData);
                Source.assign(reinterpret_cast<const char*>(pFileData->GetConstDataPtr()), pFileData->GetSize());
            }
            else
            {
                Source.assign(ParserCI.StrData);
            }

            nlohmann::json Json = nlohmann::json::parse(Source);

            for (auto const& Import : Json["Imports"])
            {
                VERIFY_EXPR(ParserCI.pStreamFactory != nullptr);
                auto Path = Import.get<std::string>();

                if (Includes.find(Path) == Includes.end())
                {
                    Includes.insert(Path);
                    ParseJSON({Path.c_str(), nullptr, ParserCI.pStreamFactory});
                }
            }

            ShaderCreateInfo              DefaultShader{};
            PipelineStateNotation         DefaultPipeline{};
            RenderPassDesc                DefaultRenderPass{};
            PipelineResourceSignatureDesc DefaultResourceSignature{};

            InlineStructureCallbacks Callbacks{};
            Callbacks.ShaderCallback = [this, &DefaultShader](const nlohmann::json& Json, SHADER_TYPE ShaderType, const char** Name, DynamicLinearAllocator& Allocator) {
                if (Json.is_string())
                {
                    VERIFY_EXPR(Name != nullptr);
                    Deserialize(Json, *Name, Allocator);
                }
                else if (Json.is_object())
                {
                    ShaderCreateInfo ResourceDesc{DefaultShader};
                    Deserialize(Json, ResourceDesc, Allocator);
                    VERIFY_EXPR(ResourceDesc.Desc.Name != nullptr);

                    if (ShaderType != SHADER_TYPE_UNKNOWN && ResourceDesc.Desc.ShaderType != SHADER_TYPE_UNKNOWN && ResourceDesc.Desc.ShaderType != ShaderType)
                        throw nlohmann::json::other_error::create(JsonInvalidEnum, std::string("shader type must be ") + GetShaderTypeLiteralName(ShaderType) + std::string(", but is ") + Json.at("Desc").at("ShaderType").get<std::string>(), Json);

                    if (ShaderType != SHADER_TYPE_UNKNOWN)
                        ResourceDesc.Desc.ShaderType = ShaderType;

                    m_ShaderNames.emplace(HashMapStringKey{ResourceDesc.Desc.Name, false}, StaticCast<Uint32>(m_Shaders.size()));
                    m_Shaders.push_back(ResourceDesc);
                    if (Name != nullptr)
                        *Name = ResourceDesc.Desc.Name;
                }
                else
                {
                    throw nlohmann::json::type_error::create(JsonTypeError, std::string("type must be object or string, but is ") + Json.type_name(), Json);
                }
            };

            Callbacks.RenderPassCallback = [this, &DefaultRenderPass](const nlohmann::json& Json, const char** Name, DynamicLinearAllocator& Allocator) {
                if (Json.is_string())
                {
                    VERIFY_EXPR(Name != nullptr);
                    Deserialize(Json, *Name, Allocator);
                }
                else if (Json.is_object())
                {
                    RenderPassDesc ResourceDesc{DefaultRenderPass};
                    Deserialize(Json, ResourceDesc, Allocator);
                    VERIFY_EXPR(ResourceDesc.Name != nullptr);

                    m_RenderPassNames.emplace(HashMapStringKey{ResourceDesc.Name, false}, StaticCast<Uint32>(m_RenderPasses.size()));
                    m_RenderPasses.push_back(ResourceDesc);
                    if (Name != nullptr)
                        *Name = ResourceDesc.Name;
                }
                else
                {
                    throw nlohmann::json::type_error::create(JsonTypeError, std::string("type must be object or string, but is ") + Json.type_name(), Json);
                }
            };

            Callbacks.ResourceSignatureCallback = [this, &DefaultResourceSignature](const nlohmann::json& Json, const char** Name, DynamicLinearAllocator& Allocator) {
                if (Json.is_string())
                {
                    VERIFY_EXPR(Name != nullptr);
                    Deserialize(Json, *Name, Allocator);
                }
                else if (Json.is_object())
                {
                    PipelineResourceSignatureDesc ResourceDesc{DefaultResourceSignature};
                    Deserialize(Json, ResourceDesc, Allocator);
                    VERIFY_EXPR(ResourceDesc.Name != nullptr);

                    m_ResourceSignatureNames.emplace(HashMapStringKey{ResourceDesc.Name, false}, StaticCast<Uint32>(m_ResourceSignatures.size()));
                    m_ResourceSignatures.push_back(ResourceDesc);
                    if (Name != nullptr)
                        *Name = ResourceDesc.Name;
                }
                else
                {
                    throw nlohmann::json::type_error::create(JsonTypeError, std::string("type must be object or string, but is ") + Json.type_name(), Json);
                }
            };

            if (Json.contains("Defaults"))
            {
                auto const& Default = Json["Defaults"];

                if (Default.contains("Shader"))
                    Deserialize(Default["Shader"], DefaultShader, *m_pAllocator);

                if (Default.contains("RenderPass"))
                    Deserialize(Default["RenderPass"], DefaultRenderPass, *m_pAllocator);

                if (Default.contains("ResourceSignature"))
                    Deserialize(Default["ResourceSignature"], DefaultResourceSignature, *m_pAllocator);

                if (Default.contains("Pipeline"))
                    Deserialize(Default["Pipeline"], DefaultPipeline, *m_pAllocator, Callbacks);
            }

            for (auto const& Shader : Json["Shaders"])
                Callbacks.ShaderCallback(Shader, SHADER_TYPE_UNKNOWN, nullptr, *m_pAllocator);

            for (auto const& RenderPass : Json["RenderPasses"])
                Callbacks.RenderPassCallback(RenderPass, nullptr, *m_pAllocator);

            for (auto const& Signature : Json["ResourceSignatures"])
                Callbacks.ResourceSignatureCallback(Signature, nullptr, *m_pAllocator);

            for (auto const& Pipeline : Json["Pipelines"])
            {
                auto AssignPipelineState = [](PipelineStateNotation& LHS, const PipelineStateNotation& RHS, PIPELINE_TYPE Type) {
                    LHS                      = RHS;
                    LHS.PSODesc.PipelineType = Type;
                };

                auto PipelineType = GetImplicitPipelineType(Pipeline);

                switch (PipelineType)
                {
                    case PIPELINE_TYPE_GRAPHICS:
                    case PIPELINE_TYPE_MESH:
                    {
                        GraphicsPipelineNotation ResourceDesc{};
                        AssignPipelineState(ResourceDesc, DefaultPipeline, PipelineType);
                        Deserialize(Pipeline, ResourceDesc, *m_pAllocator, Callbacks);
                        VERIFY_EXPR(ResourceDesc.PSODesc.Name != nullptr);

                        m_GraphicsPipelineNames.emplace(HashMapStringKey{ResourceDesc.PSODesc.Name, false}, StaticCast<Uint32>(m_GraphicsPipelineStates.size()));
                        m_GraphicsPipelineStates.push_back(ResourceDesc);
                        break;
                    }
                    case PIPELINE_TYPE_COMPUTE:
                    {
                        ComputePipelineNotation ResourceDesc{};
                        AssignPipelineState(ResourceDesc, DefaultPipeline, PipelineType);
                        Deserialize(Pipeline, ResourceDesc, *m_pAllocator, Callbacks);
                        VERIFY_EXPR(ResourceDesc.PSODesc.Name != nullptr);

                        m_ComputePipelineNames.emplace(HashMapStringKey{ResourceDesc.PSODesc.Name, false}, StaticCast<Uint32>(m_ComputePipelineStates.size()));
                        m_ComputePipelineStates.push_back(ResourceDesc);
                        break;
                    }
                    case PIPELINE_TYPE_RAY_TRACING:
                    {
                        RayTracingPipelineNotation ResourceDesc{};
                        AssignPipelineState(ResourceDesc, DefaultPipeline, PipelineType);
                        Deserialize(Pipeline, ResourceDesc, *m_pAllocator, Callbacks);
                        VERIFY_EXPR(ResourceDesc.PSODesc.Name != nullptr);

                        m_RayTracingPipelineNames.emplace(HashMapStringKey{ResourceDesc.PSODesc.Name, false}, StaticCast<Uint32>(m_RayTracingPipelineStates.size()));
                        m_RayTracingPipelineStates.push_back(ResourceDesc);
                        break;
                    }
                    case PIPELINE_TYPE_TILE:
                    {
                        TilePipelineNotation ResourceDesc{};
                        AssignPipelineState(ResourceDesc, DefaultPipeline, PipelineType);
                        Deserialize(Pipeline, ResourceDesc, *m_pAllocator, Callbacks);
                        VERIFY_EXPR(ResourceDesc.PSODesc.Name != nullptr);

                        m_TilePipelineNames.emplace(HashMapStringKey{ResourceDesc.PSODesc.Name, false}, StaticCast<Uint32>(m_TilePipelineStates.size()));
                        m_TilePipelineStates.push_back(ResourceDesc);
                        break;
                    }

                    default:
                        LOG_ERROR_AND_THROW("Pipeline type is incorrect: '", PipelineType, "'.");
                        break;
                }
            }
        }
        catch (std::exception& e)
        {
            LOG_ERROR(e.what());

            if (ParserCI.FilePath != nullptr)
                LOG_ERROR_AND_THROW("Failed to parse file: '", ParserCI.FilePath, "'.");
            else
                LOG_ERROR_AND_THROW("Failed to parse string: '", ParserCI.StrData, "'.");
        }
    };

    ParseJSON(CreateInfo);

    m_ParseInfo.ResourceSignatureCount       = StaticCast<Uint32>(m_ResourceSignatures.size());
    m_ParseInfo.ShaderCount                  = StaticCast<Uint32>(m_Shaders.size());
    m_ParseInfo.RenderPassCount              = StaticCast<Uint32>(m_RenderPasses.size());
    m_ParseInfo.GraphicsPipelineStateCount   = StaticCast<Uint32>(m_GraphicsPipelineStates.size());
    m_ParseInfo.ComputePipelineStateCount    = StaticCast<Uint32>(m_ComputePipelineStates.size());
    m_ParseInfo.RayTracingPipelineStateCount = StaticCast<Uint32>(m_RayTracingPipelineStates.size());
    m_ParseInfo.TilePipelineStateCount       = StaticCast<Uint32>(m_TilePipelineStates.size());
}

const GraphicsPipelineNotation* RenderStateNotationParserImpl::GetGraphicsPipelineStateByName(const Char* Name) const
{
    auto Iter = m_GraphicsPipelineNames.find(Name);
    if (Iter != m_GraphicsPipelineNames.end())
        return &m_GraphicsPipelineStates[Iter->second];
    return nullptr;
}

const ComputePipelineNotation* RenderStateNotationParserImpl::GetComputePipelineStateByName(const Char* Name) const
{
    auto Iter = m_ComputePipelineNames.find(Name);
    if (Iter != m_ComputePipelineNames.end())
        return &m_ComputePipelineStates[Iter->second];
    return nullptr;
}

const RayTracingPipelineNotation* RenderStateNotationParserImpl::GetRayTracingPipelineStateByName(const Char* Name) const
{
    auto Iter = m_RayTracingPipelineNames.find(Name);
    if (Iter != m_RayTracingPipelineNames.end())
        return &m_RayTracingPipelineStates[Iter->second];
    return nullptr;
}

const TilePipelineNotation* RenderStateNotationParserImpl::GetTilePipelineStateByName(const Char* Name) const
{
    auto Iter = m_TilePipelineNames.find(Name);
    if (Iter != m_TilePipelineNames.end())
        return &m_TilePipelineStates[Iter->second];
    return nullptr;
}

const PipelineResourceSignatureDesc* RenderStateNotationParserImpl::GetResourceSignatureByName(const Char* Name) const
{
    auto Iter = m_ResourceSignatureNames.find(Name);
    if (Iter != m_ResourceSignatureNames.end())
        return &m_ResourceSignatures[Iter->second];
    return nullptr;
}

const ShaderCreateInfo* RenderStateNotationParserImpl::GetShaderByName(const Char* Name) const
{
    auto Iter = m_ShaderNames.find(Name);
    if (Iter != m_ShaderNames.end())
        return &m_Shaders[Iter->second];
    return nullptr;
}

const RenderPassDesc* RenderStateNotationParserImpl::GetRenderPassByName(const Char* Name) const
{
    auto Iter = m_RenderPassNames.find(Name);
    if (Iter != m_RenderPassNames.end())
        return &m_RenderPasses[Iter->second];
    return nullptr;
}

const GraphicsPipelineNotation* RenderStateNotationParserImpl::GetGraphicsPipelineStateByIndex(Uint32 Index) const
{
    if (Index < m_GraphicsPipelineStates.size())
        return &m_GraphicsPipelineStates[Index];
    return nullptr;
}

const ComputePipelineNotation* RenderStateNotationParserImpl::GetComputePipelineStateByIndex(Uint32 Index) const
{
    if (Index < m_ComputePipelineStates.size())
        return &m_ComputePipelineStates[Index];
    return nullptr;
}

const RayTracingPipelineNotation* RenderStateNotationParserImpl::GetRayTracingPipelineStateByIndex(Uint32 Index) const
{
    if (Index < m_RayTracingPipelineStates.size())
        return &m_RayTracingPipelineStates[Index];
    return nullptr;
}

const TilePipelineNotation* RenderStateNotationParserImpl::GetTilePipelineStateByIndex(Uint32 Index) const
{
    if (Index < m_TilePipelineStates.size())
        return &m_TilePipelineStates[Index];
    return nullptr;
}

const PipelineResourceSignatureDesc* RenderStateNotationParserImpl::GetResourceSignatureByIndex(Uint32 Index) const
{
    if (Index < m_ResourceSignatures.size())
        return &m_ResourceSignatures[Index];
    return nullptr;
}

const ShaderCreateInfo* RenderStateNotationParserImpl::GetShaderByIndex(Uint32 Index) const
{
    if (Index < m_Shaders.size())
        return &m_Shaders[Index];
    return nullptr;
}

const RenderPassDesc* RenderStateNotationParserImpl::GetRenderPassByIndex(Uint32 Index) const
{
    if (Index < m_RenderPasses.size())
        return &m_RenderPasses[Index];
    return nullptr;
}

const RenderStateNotationParserInfo& RenderStateNotationParserImpl::GetInfo() const
{
    return m_ParseInfo;
}

void CreateRenderStateNotationParser(const RenderStateNotationParserCreateInfo& CreateInfo,
                                     IRenderStateNotationParser**               ppParser)
{
    try
    {
        RefCntAutoPtr<IRenderStateNotationParser> pParser{MakeNewRCObj<RenderStateNotationParserImpl>()(CreateInfo)};
        if (pParser)
            pParser->QueryInterface(IID_RenderStateNotationParser, reinterpret_cast<IObject**>(ppParser));
    }
    catch (...)
    {
        LOG_ERROR("Failed create render state notation parser");
    }
}

} // namespace Diligent

extern "C"
{
    void Diligent_CreateRenderStateNotationParser(const Diligent::RenderStateNotationParserCreateInfo& CreateInfo,
                                                  Diligent::IRenderStateNotationParser**               ppLoader)
    {
        Diligent::CreateRenderStateNotationParser(CreateInfo, ppLoader);
    }
}
