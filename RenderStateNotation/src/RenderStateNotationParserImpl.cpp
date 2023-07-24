/*
 *  Copyright 2019-2023 Diligent Graphics LLC
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
#include <array>

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
void ParseRSN(const nlohmann::json& Json, const Type*& pObjects, TypeSize& NumElements, DynamicLinearAllocator& Allocator, const InlineStructureCallbacks& Callbacks)
{
    if (!Json.is_array())
        throw nlohmann::json::type_error::create(JsonTypeError, std::string("type must be array, but is ") + Json.type_name(), Json);

    auto* pData = Allocator.ConstructArray<Type>(Json.size());
    for (size_t i = 0; i < Json.size(); i++)
        ParseRSN(Json[i], pData[i], Allocator, Callbacks);

    pObjects    = pData;
    NumElements = static_cast<TypeSize>(Json.size());
}

void ParseRSN(const nlohmann::json& Json, PipelineStateNotation& Type, DynamicLinearAllocator& Allocator, const InlineStructureCallbacks& Callbacks)
{
    if (Json.contains("PSODesc"))
        ParseRSN(Json["PSODesc"], Type.PSODesc, Allocator);

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

void ParseRSN(const nlohmann::json& Json, GraphicsPipelineNotation& Type, DynamicLinearAllocator& Allocator, const InlineStructureCallbacks& Callbacks)
{
    NLOHMANN_JSON_VALIDATE_KEYS(Json, {"PSODesc", "Flags", "ppResourceSignatures", "GraphicsPipeline", "pVS", "pPS", "pDS", "pHS", "pGS", "pAS", "pMS"});

    ParseRSN(Json, static_cast<PipelineStateNotation&>(Type), Allocator, Callbacks);

    if (Json.contains("GraphicsPipeline"))
    {
        auto& GraphicsPipeline = Json["GraphicsPipeline"];
        ParseRSN(GraphicsPipeline, Type.Desc, Allocator);

        if (GraphicsPipeline.contains("pRenderPass"))
            Callbacks.RenderPassCallback(GraphicsPipeline["pRenderPass"], &Type.pRenderPassName, Allocator);

        if (!GraphicsPipeline.contains("NumRenderTargets"))
        {
            for (Uint8 i = 0; i < _countof(Type.Desc.RTVFormats); i++)
                if (Type.Desc.RTVFormats[i] != TEX_FORMAT_UNKNOWN)
                    Type.Desc.NumRenderTargets = i + 1;
        }
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

void ParseRSN(const nlohmann::json& Json, ComputePipelineNotation& Type, DynamicLinearAllocator& Allocator, const InlineStructureCallbacks& Callbacks)
{
    NLOHMANN_JSON_VALIDATE_KEYS(Json, {"PSODesc", "Flags", "ppResourceSignatures", "pCS"});

    ParseRSN(Json, static_cast<PipelineStateNotation&>(Type), Allocator, Callbacks);

    Callbacks.ShaderCallback(Json.at("pCS"), SHADER_TYPE_COMPUTE, &Type.pCSName, Allocator);
}

void ParseRSN(const nlohmann::json& Json, TilePipelineNotation& Type, DynamicLinearAllocator& Allocator, const InlineStructureCallbacks& Callbacks)
{
    NLOHMANN_JSON_VALIDATE_KEYS(Json, {"PSODesc", "Flags", "ppResourceSignatures", "pTS"});

    ParseRSN(Json, static_cast<PipelineStateNotation&>(Type), Allocator, Callbacks);

    Callbacks.ShaderCallback(Json.at("pTS"), SHADER_TYPE_TILE, &Type.pTSName, Allocator);
}

void ParseRSN(const nlohmann::json& Json, RTGeneralShaderGroupNotation& Type, DynamicLinearAllocator& Allocator, const InlineStructureCallbacks& Callbacks)
{
    NLOHMANN_JSON_VALIDATE_KEYS(Json, {"PSODesc", "Flags", "ppResourceSignatures", "Name", "pShader"});

    ParseRSN(Json.at("Name"), Type.Name, Allocator);

    Callbacks.ShaderCallback(Json.at("pShader"), SHADER_TYPE_RAY_GEN, &Type.pShaderName, Allocator);
}

void ParseRSN(const nlohmann::json& Json, RTTriangleHitShaderGroupNotation& Type, DynamicLinearAllocator& Allocator, const InlineStructureCallbacks& Callbacks)
{
    NLOHMANN_JSON_VALIDATE_KEYS(Json, {"PSODesc", "Flags", "ppResourceSignatures", "Name", "pClosestHitShader", "pAnyHitShader"});

    ParseRSN(Json.at("Name"), Type.Name, Allocator);

    if (Json.contains("pClosestHitShader"))
        Callbacks.ShaderCallback(Json["pClosestHitShader"], SHADER_TYPE_RAY_CLOSEST_HIT, &Type.pClosestHitShaderName, Allocator);

    if (Json.contains("pAnyHitShader"))
        Callbacks.ShaderCallback(Json["pAnyHitShader"], SHADER_TYPE_RAY_ANY_HIT, &Type.pAnyHitShaderName, Allocator);
}

void ParseRSN(const nlohmann::json& Json, RTProceduralHitShaderGroupNotation& Type, DynamicLinearAllocator& Allocator, const InlineStructureCallbacks& Callbacks)
{
    NLOHMANN_JSON_VALIDATE_KEYS(Json, {"PSODesc", "Flags", "ppResourceSignatures", "Name", "pIntersectionShader", "pClosestHitShader", "pAnyHitShader"});

    ParseRSN(Json.at("Name"), Type.Name, Allocator);

    if (Json.contains("pIntersectionShader"))
        Callbacks.ShaderCallback(Json["pIntersectionShader"], SHADER_TYPE_RAY_INTERSECTION, &Type.pIntersectionShaderName, Allocator);

    if (Json.contains("pClosestHitShader"))
        Callbacks.ShaderCallback(Json["pClosestHitShader"], SHADER_TYPE_RAY_CLOSEST_HIT, &Type.pClosestHitShaderName, Allocator);

    if (Json.contains("pAnyHitShader"))
        Callbacks.ShaderCallback(Json["pAnyHitShader"], SHADER_TYPE_RAY_ANY_HIT, &Type.pAnyHitShaderName, Allocator);
}

void ParseRSN(const nlohmann::json& Json, RayTracingPipelineNotation& Type, DynamicLinearAllocator& Allocator, const InlineStructureCallbacks& Callbacks)
{
    NLOHMANN_JSON_VALIDATE_KEYS(Json, {"PSODesc", "Flags", "ppResourceSignatures", "RayTracingPipeline", "pGeneralShaders", "pTriangleHitShaders", "pProceduralHitShaders", "pShaderRecordName", "MaxAttributeSize", "MaxPayloadSize"});

    ParseRSN(Json, static_cast<PipelineStateNotation&>(Type), Allocator, Callbacks);

    if (Json.contains("RayTracingPipeline"))
        ParseRSN(Json["RayTracingPipeline"], Type.RayTracingPipeline, Allocator);

    if (Json.contains("pGeneralShaders"))
        ParseRSN(Json["pGeneralShaders"], Type.pGeneralShaders, Type.GeneralShaderCount, Allocator, Callbacks);

    if (Json.contains("pTriangleHitShaders"))
        ParseRSN(Json["pTriangleHitShaders"], Type.pTriangleHitShaders, Type.TriangleHitShaderCount, Allocator, Callbacks);

    if (Json.contains("pProceduralHitShaders"))
        ParseRSN(Json["pProceduralHitShaders"], Type.pProceduralHitShaders, Type.ProceduralHitShaderCount, Allocator, Callbacks);

    if (Json.contains("pShaderRecordName"))
        ParseRSN(Json["pShaderRecordName"], Type.pShaderRecordName, Allocator);

    if (Json.contains("MaxAttributeSize"))
        ParseRSN(Json["MaxAttributeSize"], Type.MaxAttributeSize, Allocator);

    if (Json.contains("MaxPayloadSize"))
        ParseRSN(Json["MaxPayloadSize"], Type.MaxPayloadSize, Allocator);
}

PIPELINE_TYPE GetPipelineType(const nlohmann::json& Json)
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

    if (Json.at("PSODesc").contains("PipelineType"))
        return Json["PSODesc"]["PipelineType"].get<PIPELINE_TYPE>();

    return PIPELINE_TYPE_INVALID;
}

} // namespace

void ParseRSNDeviceCreateInfo(const Char* Data, Uint32 Size, SerializationDeviceCreateInfo& Type, DynamicLinearAllocator& Allocator)
{
    nlohmann::json Json = nlohmann::json::parse(Data, Data + Size);
    ParseRSN(Json, Type, Allocator);
}

RenderStateNotationParserImpl::RenderStateNotationParserImpl(IReferenceCounters*                        pRefCounters,
                                                             const RenderStateNotationParserCreateInfo& CreateInfo) :
    TBase{pRefCounters},
    m_CI{CreateInfo}
{
    m_pAllocator = std::make_unique<DynamicLinearAllocator>(DefaultRawMemoryAllocator::GetAllocator());
}

Bool RenderStateNotationParserImpl::ParseFile(const Char*                      FilePath,
                                              IShaderSourceInputStreamFactory* pStreamFactory,
                                              IShaderSourceInputStreamFactory* pReloadFactory)

{
    if (FilePath == nullptr || FilePath[0] == '\0')
    {
        DEV_ERROR("FilePath must not be null or empty");
        return false;
    }

    const auto res = ParseFileInternal(FilePath, pStreamFactory);
    if (m_CI.EnableReload && res)
    {
        ReloadInfo Info;
        Info.Path     = FilePath;
        Info.pFactory = pReloadFactory != nullptr ? pReloadFactory : pStreamFactory;
        m_ReloadInfo.emplace_back(std::move(Info));
    }
    return res;
}

Bool RenderStateNotationParserImpl::ParseFileInternal(const Char*                      FilePath,
                                                      IShaderSourceInputStreamFactory* pStreamFactory)
{
    VERIFY_EXPR(FilePath != nullptr && pStreamFactory != nullptr);

    try
    {
        // TODO: use absolute path
        if (m_Includes.insert(FilePath).second)
        {
            RefCntAutoPtr<IFileStream> pFileStream;
            pStreamFactory->CreateInputStream(FilePath, &pFileStream);

            if (!pFileStream)
                LOG_ERROR_AND_THROW("Failed to open file: '", FilePath, "'.");

            auto pFileData = DataBlobImpl::Create();
            pFileStream->ReadBlob(pFileData);

            if (!ParseStringInternal(static_cast<const char*>(pFileData->GetConstDataPtr()), StaticCast<Uint32>(pFileData->GetSize()), pStreamFactory))
                LOG_ERROR_AND_THROW("Failed to parse file: '", FilePath, "'.");
        }

        return true;
    }
    catch (...)
    {
        return false;
    }
}

Bool RenderStateNotationParserImpl::ParseString(const Char*                      Source,
                                                Uint32                           Length,
                                                IShaderSourceInputStreamFactory* pStreamFactory,
                                                IShaderSourceInputStreamFactory* pReloadFactory)
{
    if (Source == nullptr || Source[0] == '\0')
    {
        DEV_ERROR("Source must not be null or empty");
        return false;
    }

    const auto res = ParseStringInternal(Source, Length, pStreamFactory);
    if (m_CI.EnableReload && res)
    {
        ReloadInfo Info;
        Info.Source   = Length != 0 ? std::string{Source, Source + Length} : std::string{Source};
        Info.pFactory = pReloadFactory != nullptr ? pReloadFactory : pStreamFactory;
        m_ReloadInfo.emplace_back(std::move(Info));
    }
    return res;
}

Bool RenderStateNotationParserImpl::ParseStringInternal(const Char*                      Source,
                                                        Uint32                           Length,
                                                        IShaderSourceInputStreamFactory* pStreamFactory)
{
    VERIFY_EXPR(Source != nullptr);

    auto ParseJSON = [this](const Char* Source, Uint32 Length, IShaderSourceInputStreamFactory* pStreamFactory) -> bool //
    {
        try
        {
            String StrSource;
            if (Length != 0)
                StrSource.assign(Source, Length);
            else
                StrSource.assign(Source);

            nlohmann::json Json = nlohmann::json::parse(StrSource);
            NLOHMANN_JSON_VALIDATE_KEYS(Json, {"Imports", "Defaults", "Shaders", "RenderPasses", "ResourceSignatures", "Pipelines", "Ignore"});

            for (auto const& Import : Json["Imports"])
            {
                VERIFY_EXPR(pStreamFactory != nullptr);
                auto Path = Import.get<std::string>();

                if (!ParseFileInternal(Path.c_str(), pStreamFactory))
                    LOG_ERROR_AND_THROW("Failed to import file: '", Path, "'.");
            }

            if (Json.contains("Ignore"))
            {
                const auto& Ignored = Json["Ignore"];
                NLOHMANN_JSON_VALIDATE_KEYS(Ignored, {"Signatures"});
                for (auto const& IgnoredSign : Ignored["Signatures"])
                {
                    auto SignName = IgnoredSign.get<std::string>();
                    m_IgnoredSignatures.emplace(std::move(SignName));
                }
            }

            ShaderCreateInfo              DefaultShader{};
            PipelineStateNotation         DefaultPipeline{};
            RenderPassDesc                DefaultRenderPass{};
            PipelineResourceSignatureDesc DefaultResourceSignature{};

            InlineStructureCallbacks Callbacks{};
            Callbacks.ShaderCallback = [this, &DefaultShader](const nlohmann::json& Json, SHADER_TYPE ShaderType, const char** Name, DynamicLinearAllocator& Allocator) //
            {
                if (Json.is_string())
                {
                    VERIFY_EXPR(Name != nullptr);
                    ParseRSN(Json, *Name, Allocator);
                }
                else if (Json.is_object())
                {
                    ShaderCreateInfo ResourceDesc{DefaultShader};
                    ParseRSN(Json, ResourceDesc, Allocator);
                    VERIFY_EXPR(ResourceDesc.Desc.Name != nullptr);

                    if (ShaderType != SHADER_TYPE_UNKNOWN && ResourceDesc.Desc.ShaderType != SHADER_TYPE_UNKNOWN && ResourceDesc.Desc.ShaderType != ShaderType)
                        throw nlohmann::json::other_error::create(JsonInvalidEnum, std::string("shader type must be ") + GetShaderTypeLiteralName(ShaderType) + std::string(", but is ") + Json.at("Desc").at("ShaderType").get<std::string>(), Json);

                    if (ShaderType != SHADER_TYPE_UNKNOWN)
                        ResourceDesc.Desc.ShaderType = ShaderType;

                    auto const Iter = m_ShaderNames.emplace(HashMapStringKey{ResourceDesc.Desc.Name, false}, StaticCast<Uint32>(m_Shaders.size()));
                    if (Iter.second)
                    {
                        m_Shaders.push_back(ResourceDesc);
                    }
                    else
                    {
                        auto CompareShaderCI = [](const ShaderCreateInfo& LHS, const ShaderCreateInfo& RHS) //
                        {
                            return LHS.Desc == RHS.Desc &&
                                LHS.SourceLanguage == RHS.SourceLanguage &&
                                LHS.HLSLVersion == RHS.HLSLVersion &&
                                LHS.GLSLVersion == RHS.GLSLVersion &&
                                LHS.GLESSLVersion == RHS.GLESSLVersion &&
                                LHS.CompileFlags == RHS.CompileFlags &&
                                LHS.ShaderCompiler == RHS.ShaderCompiler &&
                                SafeStrEqual(LHS.EntryPoint, RHS.EntryPoint) &&
                                SafeStrEqual(LHS.FilePath, RHS.FilePath) &&
                                LHS.Macros == RHS.Macros;
                        };

                        if (!CompareShaderCI(m_Shaders[Iter.first->second], ResourceDesc))
                            LOG_ERROR_AND_THROW("Redefinition of shader '", ResourceDesc.Desc.Name, "'.");
                    }

                    if (Name != nullptr)
                        *Name = ResourceDesc.Desc.Name;
                }
                else
                {
                    throw nlohmann::json::type_error::create(JsonTypeError, std::string("type must be object or string, but is ") + Json.type_name(), Json);
                }
            };

            Callbacks.RenderPassCallback = [this, &DefaultRenderPass](const nlohmann::json& Json, const char** Name, DynamicLinearAllocator& Allocator) //
            {
                if (Json.is_string())
                {
                    VERIFY_EXPR(Name != nullptr);
                    ParseRSN(Json, *Name, Allocator);
                }
                else if (Json.is_object())
                {
                    RenderPassDesc ResourceDesc{DefaultRenderPass};
                    ParseRSN(Json, ResourceDesc, Allocator);
                    VERIFY_EXPR(ResourceDesc.Name != nullptr);

                    auto const Iter = m_RenderPassNames.emplace(HashMapStringKey{ResourceDesc.Name, false}, StaticCast<Uint32>(m_RenderPasses.size()));
                    if (Iter.second)
                        m_RenderPasses.push_back(ResourceDesc);
                    else if (!(m_RenderPasses[Iter.first->second] == ResourceDesc))
                        LOG_ERROR_AND_THROW("Redefinition of render pass '", ResourceDesc.Name, "'.");

                    if (Name != nullptr)
                        *Name = ResourceDesc.Name;
                }
                else
                {
                    throw nlohmann::json::type_error::create(JsonTypeError, std::string("type must be object or string, but is ") + Json.type_name(), Json);
                }
            };

            Callbacks.ResourceSignatureCallback = [this, &DefaultResourceSignature](const nlohmann::json& Json, const char** Name, DynamicLinearAllocator& Allocator) //
            {
                if (Json.is_string())
                {
                    VERIFY_EXPR(Name != nullptr);
                    ParseRSN(Json, *Name, Allocator);
                }
                else if (Json.is_object())
                {
                    PipelineResourceSignatureDesc ResourceDesc{DefaultResourceSignature};
                    ParseRSN(Json, ResourceDesc, Allocator);
                    VERIFY_EXPR(ResourceDesc.Name != nullptr);

                    auto const Iter = m_ResourceSignatureNames.emplace(HashMapStringKey{ResourceDesc.Name, false}, StaticCast<Uint32>(m_ResourceSignatures.size()));
                    if (Iter.second)
                        m_ResourceSignatures.push_back(ResourceDesc);
                    else if (!(m_ResourceSignatures[Iter.first->second] == ResourceDesc))
                        LOG_ERROR_AND_THROW("Redefinition of resource signature '", ResourceDesc.Name, "'.");

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

                NLOHMANN_JSON_VALIDATE_KEYS(Default, {"Shader", "RenderPass", "ResourceSignature", "Pipeline"});

                if (Default.contains("Shader"))
                    ParseRSN(Default["Shader"], DefaultShader, *m_pAllocator);

                if (Default.contains("RenderPass"))
                    ParseRSN(Default["RenderPass"], DefaultRenderPass, *m_pAllocator);

                if (Default.contains("ResourceSignature"))
                    ParseRSN(Default["ResourceSignature"], DefaultResourceSignature, *m_pAllocator);

                if (Default.contains("Pipeline"))
                    ParseRSN(Default["Pipeline"], DefaultPipeline, *m_pAllocator, Callbacks);
            }

            for (auto const& Shader : Json["Shaders"])
                Callbacks.ShaderCallback(Shader, SHADER_TYPE_UNKNOWN, nullptr, *m_pAllocator);

            for (auto const& RenderPass : Json["RenderPasses"])
                Callbacks.RenderPassCallback(RenderPass, nullptr, *m_pAllocator);

            for (auto const& Signature : Json["ResourceSignatures"])
                Callbacks.ResourceSignatureCallback(Signature, nullptr, *m_pAllocator);

            for (auto const& Pipeline : Json["Pipelines"])
            {
                auto AddPipelineState = [&](PIPELINE_TYPE PipelineType, auto& PSONotation) //
                {
                    static_cast<PipelineStateNotation&>(PSONotation) = DefaultPipeline;
                    PSONotation.PSODesc.PipelineType                 = PipelineType;
                    ParseRSN(Pipeline, PSONotation, *m_pAllocator, Callbacks);
                    VERIFY_EXPR(PSONotation.PSODesc.Name != nullptr);

                    if (m_PipelineStateNames.emplace(std::make_pair(HashMapStringKey{PSONotation.PSODesc.Name, false}, PipelineType), StaticCast<Uint32>(m_PipelineStates.size())).second)
                        m_PipelineStates.emplace_back(PSONotation);
                    else
                        LOG_ERROR_AND_THROW("Redefinition of pipeline '", PSONotation.PSODesc.Name, "'.");
                };

                static_assert(PIPELINE_TYPE_LAST == 4, "Please handle the new pipeline type below.");
                const auto PipelineType = GetPipelineType(Pipeline);
                switch (PipelineType)
                {
                    case PIPELINE_TYPE_GRAPHICS:
                    case PIPELINE_TYPE_MESH:
                        AddPipelineState(PipelineType, *m_pAllocator->Construct<GraphicsPipelineNotation>());
                        break;

                    case PIPELINE_TYPE_COMPUTE:
                        AddPipelineState(PipelineType, *m_pAllocator->Construct<ComputePipelineNotation>());
                        break;

                    case PIPELINE_TYPE_RAY_TRACING:
                        AddPipelineState(PipelineType, *m_pAllocator->Construct<RayTracingPipelineNotation>());
                        break;

                    case PIPELINE_TYPE_TILE:
                        AddPipelineState(PipelineType, *m_pAllocator->Construct<TilePipelineNotation>());
                        break;
                    case PIPELINE_TYPE_INVALID:
                        LOG_ERROR_AND_THROW("Pipeline type isn't set for '", Json["PSODesc"]["Name"].get<std::string>(), "'.");
                        break;
                    default:
                        UNEXPECTED("Unexpected pipeline type.");
                }
            }
            return true;
        }
        catch (std::exception& e)
        {
            LOG_ERROR(e.what());
            return false;
        }
    };

    if (!ParseJSON(Source, Length, pStreamFactory))
        return false;

    m_ParseInfo.ResourceSignatureCount = StaticCast<Uint32>(m_ResourceSignatures.size());
    m_ParseInfo.ShaderCount            = StaticCast<Uint32>(m_Shaders.size());
    m_ParseInfo.RenderPassCount        = StaticCast<Uint32>(m_RenderPasses.size());
    m_ParseInfo.PipelineStateCount     = StaticCast<Uint32>(m_PipelineStates.size());

    return true;
}

const PipelineStateNotation* RenderStateNotationParserImpl::GetPipelineStateByName(const Char* Name, PIPELINE_TYPE PipelineType) const
{
    auto FindPipeline = [this](const Char* Name, PIPELINE_TYPE PipelineType) -> const PipelineStateNotation* //
    {
        const auto Iter = m_PipelineStateNames.find(std::make_pair(HashMapStringKey{Name, true}, PipelineType));
        if (Iter != m_PipelineStateNames.end())
            return &m_PipelineStates[Iter->second].get();
        return nullptr;
    };

    if (PipelineType != PIPELINE_TYPE_INVALID)
    {
        return FindPipeline(Name, PipelineType);
    }
    else
    {
        constexpr std::array<PIPELINE_TYPE, 5> PipelineTypes = {
            PIPELINE_TYPE_GRAPHICS,
            PIPELINE_TYPE_COMPUTE,
            PIPELINE_TYPE_MESH,
            PIPELINE_TYPE_RAY_TRACING,
            PIPELINE_TYPE_TILE};

        for (auto const& Type : PipelineTypes)
        {
            if (const auto* pPipeline = FindPipeline(Name, Type))
                return pPipeline;
        }
    }
    return nullptr;
}

const PipelineResourceSignatureDesc* RenderStateNotationParserImpl::GetResourceSignatureByName(const Char* Name) const
{
    const auto Iter = m_ResourceSignatureNames.find(Name);
    return Iter != m_ResourceSignatureNames.end() ? &m_ResourceSignatures[Iter->second] : nullptr;
}

const ShaderCreateInfo* RenderStateNotationParserImpl::GetShaderByName(const Char* Name) const
{
    const auto Iter = m_ShaderNames.find(Name);
    return Iter != m_ShaderNames.end() ? &m_Shaders[Iter->second] : nullptr;
}

const RenderPassDesc* RenderStateNotationParserImpl::GetRenderPassByName(const Char* Name) const
{
    const auto Iter = m_RenderPassNames.find(Name);
    return Iter != m_RenderPassNames.end() ? &m_RenderPasses[Iter->second] : nullptr;
}

const PipelineStateNotation* RenderStateNotationParserImpl::GetPipelineStateByIndex(Uint32 Index) const
{
    return Index < m_PipelineStates.size() ? &m_PipelineStates[Index].get() : nullptr;
}

const PipelineResourceSignatureDesc* RenderStateNotationParserImpl::GetResourceSignatureByIndex(Uint32 Index) const
{
    return Index < m_ResourceSignatures.size() ? &m_ResourceSignatures[Index] : nullptr;
}

const ShaderCreateInfo* RenderStateNotationParserImpl::GetShaderByIndex(Uint32 Index) const
{
    return Index < m_Shaders.size() ? &m_Shaders[Index] : nullptr;
}

const RenderPassDesc* RenderStateNotationParserImpl::GetRenderPassByIndex(Uint32 Index) const
{
    return Index < m_RenderPasses.size() ? &m_RenderPasses[Index] : nullptr;
}

Bool RenderStateNotationParserImpl::IsSignatureIgnored(const Char* Name) const
{
    return m_IgnoredSignatures.find(Name) != m_IgnoredSignatures.end();
}

const RenderStateNotationParserInfo& RenderStateNotationParserImpl::GetInfo() const
{
    return m_ParseInfo;
}

void RenderStateNotationParserImpl::Reset()
{
    m_Includes.clear();
    m_IgnoredSignatures.clear();

    m_ResourceSignatures.clear();
    m_Shaders.clear();
    m_RenderPasses.clear();
    m_PipelineStates.clear();

    m_ResourceSignatureNames.clear();
    m_ShaderNames.clear();
    m_RenderPassNames.clear();
    m_PipelineStateNames.clear();

    m_ParseInfo = {};
}

bool RenderStateNotationParserImpl::Reload()
{
    if (!m_CI.EnableReload)
    {
        DEV_ERROR("State reloading is not enabled. Set EnableReload member of RenderStateNotationParserCreateInfo to true when creating the parser.");
        return false;
    }

    {
        auto ReloadInfo = std::move(m_ReloadInfo);
        Reset();
        m_ReloadInfo = std::move(ReloadInfo);
    }

    bool res = true;
    for (const auto& Reload : m_ReloadInfo)
    {
        if (!Reload.Path.empty())
        {
            if (!ParseFileInternal(Reload.Path.c_str(), Reload.pFactory))
                res = false;
        }
        else if (!Reload.Source.empty())
        {
            if (!ParseStringInternal(Reload.Source.c_str(), static_cast<Uint32>(Reload.Source.length()), Reload.pFactory))
                res = false;
        }
        else
        {
            UNEXPECTED("Either path or source must not be null.");
        }
    }
    return res;
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
