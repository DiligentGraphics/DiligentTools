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

#include "RenderStatePackager.hpp"
#include "DefaultRawMemoryAllocator.hpp"
#include "DynamicLinearAllocator.hpp"

namespace Diligent
{

RenderStatePackager::RenderStatePackager(RefCntAutoPtr<ISerializationDevice>            pDevice,
                                         RefCntAutoPtr<IShaderSourceInputStreamFactory> pShaderStreamFactory,
                                         RefCntAutoPtr<IShaderSourceInputStreamFactory> pRenderStateStreamFactory,
                                         RefCntAutoPtr<IThreadPool>                     pThreadPool,
                                         ARCHIVE_DEVICE_DATA_FLAGS                      DeviceBits) :
    m_pDevice{pDevice},
    m_pShaderStreamFactory{pShaderStreamFactory},
    m_pRenderStateStreamFactory{pRenderStateStreamFactory},
    m_pThreadPool{pThreadPool},
    m_DeviceBits{DeviceBits}
{
}

bool RenderStatePackager::ParseFiles(std::vector<std::string> const& DRSNPaths)
{
    DEV_CHECK_ERR(!DRSNPaths.empty(), "DRSNPaths must not be empty");

    m_RSNParsers.resize(DRSNPaths.size());
    std::atomic<bool> ParseResult{true};
    for (Uint32 ParserID = 0; ParserID < DRSNPaths.size(); ++ParserID)
    {
        EnqueueAsyncWork(m_pThreadPool, [ParserID, this, &DRSNPaths, &ParseResult](Uint32 ThreadId) {
            auto& pDescriptorParser = m_RSNParsers[ParserID];

            RenderStateNotationParserCreateInfo RSNParserCI{};
            RSNParserCI.FilePath       = DRSNPaths[ParserID].c_str();
            RSNParserCI.pStreamFactory = m_pRenderStateStreamFactory;
            CreateRenderStateNotationParser(RSNParserCI, &pDescriptorParser);
            if (!pDescriptorParser)
            {
                LOG_ERROR_MESSAGE("Failed to parse file '", DRSNPaths[ParserID].c_str(), "'.");
                ParseResult.store(false);
            }
        });
    }

    m_pThreadPool->WaitForAllTasks();
    return ParseResult;
}

bool RenderStatePackager::Execute(RefCntAutoPtr<IArchiver> pArchive)
{
    DEV_CHECK_ERR(pArchive != nullptr, "pArchive must not be null");

    if (pArchive == nullptr)
        return false;

    try
    {
        std::atomic<bool> Result{true};

        std::vector<std::vector<RefCntAutoPtr<IShader>>>                    Shaders(m_RSNParsers.size());
        std::vector<std::vector<RefCntAutoPtr<IRenderPass>>>                RenderPasses(m_RSNParsers.size());
        std::vector<std::vector<RefCntAutoPtr<IPipelineResourceSignature>>> ResourceSignatures(m_RSNParsers.size());

        for (Uint32 ParserID = 0; ParserID < m_RSNParsers.size(); ++ParserID)
        {
            auto const& ParserInfo = m_RSNParsers[ParserID]->GetInfo();

            Shaders[ParserID].resize(ParserInfo.ShaderCount);
            RenderPasses[ParserID].resize(ParserInfo.RenderPassCount);
            ResourceSignatures[ParserID].resize(ParserInfo.ResourceSignatureCount);

            for (Uint32 ShaderID = 0; ShaderID < ParserInfo.ShaderCount; ++ShaderID)
            {
                EnqueueAsyncWork(m_pThreadPool, [ParserID, ShaderID, this, &Result, &Shaders](Uint32 ThreadId) {
                    ShaderCreateInfo ShaderCI           = *m_RSNParsers[ParserID]->GetShaderByIndex(ShaderID);
                    ShaderCI.pShaderSourceStreamFactory = m_pShaderStreamFactory;

                    auto& pShader = Shaders[ParserID][ShaderID];
                    m_pDevice->CreateShader(ShaderCI, m_DeviceBits, &pShader);
                    if (!pShader)
                    {
                        LOG_ERROR_MESSAGE("Failed to create shader from file '", ShaderCI.FilePath, "'.");
                        Result.store(false);
                    }
                });
            }

            for (Uint32 RenderPassID = 0; RenderPassID < ParserInfo.RenderPassCount; ++RenderPassID)
            {
                EnqueueAsyncWork(m_pThreadPool, [ParserID, RenderPassID, this, &Result, &RenderPasses](Uint32 ThreadId) {
                    RenderPassDesc RPDesc      = *m_RSNParsers[ParserID]->GetRenderPassByIndex(RenderPassID);
                    auto&          pRenderPass = RenderPasses[ParserID][RenderPassID];
                    m_pDevice->CreateRenderPass(RPDesc, &pRenderPass);
                    if (!pRenderPass)
                    {
                        LOG_ERROR_MESSAGE("Failed to create render pass '", RPDesc.Name, "'.");
                        Result.store(false);
                    }
                });
            }

            for (Uint32 SignatureID = 0; SignatureID < ParserInfo.ResourceSignatureCount; ++SignatureID)
            {
                EnqueueAsyncWork(m_pThreadPool, [ParserID, SignatureID, this, &Result, &ResourceSignatures](Uint32 ThreadId) {
                    PipelineResourceSignatureDesc SignDesc   = *m_RSNParsers[ParserID]->GetResourceSignatureByIndex(SignatureID);
                    auto&                         pSignature = ResourceSignatures[ParserID][SignatureID];
                    m_pDevice->CreatePipelineResourceSignature(SignDesc, m_DeviceBits, &pSignature);
                    if (!pSignature)
                    {
                        LOG_ERROR_MESSAGE("Failed to create resource signature '", SignDesc.Name, "'.");
                        Result.store(false);
                    }
                });
            }
        }

        m_pThreadPool->WaitForAllTasks();

        if (!Result.load())
            LOG_ERROR_AND_THROW("Failed to create state objects");

        for (Uint32 ParserID = 0; ParserID < m_RSNParsers.size(); ++ParserID)
        {
            for (auto& pResouce : Shaders[ParserID])
                m_Shaders.emplace(HashMapStringKey{pResouce->GetDesc().Name, false}, pResouce);

            for (auto& pResouce : RenderPasses[ParserID])
                m_RenderPasses.emplace(HashMapStringKey{pResouce->GetDesc().Name, false}, pResouce);

            for (auto& pResouce : ResourceSignatures[ParserID])
                m_ResourceSignatures.emplace(HashMapStringKey{pResouce->GetDesc().Name, false}, pResouce);
        }

        DynamicLinearAllocator Allocator{DefaultRawMemoryAllocator::GetAllocator()};
        for (auto const& pNotationParser : m_RSNParsers)
        {
            auto const& ParserInfo = pNotationParser->GetInfo();

            auto FindShader = [&](const char* Name) -> IShader* {
                if (Name == nullptr)
                    return nullptr;

                auto Iter = m_Shaders.find(Name);
                if (Iter == m_Shaders.end())
                    LOG_ERROR_AND_THROW("Unable to find shader '", Name, "'.");
                return Iter->second;
            };

            auto FindRenderPass = [&](const char* Name) -> IRenderPass* {
                if (Name == nullptr)
                    return nullptr;

                auto Iter = m_RenderPasses.find(Name);
                if (Iter == m_RenderPasses.end())
                    LOG_ERROR_AND_THROW("Unable to find render pass '", Name, "'.");
                return Iter->second;
            };

            auto FindResourceSignature = [&](const char* Name) -> IPipelineResourceSignature* {
                if (Name == nullptr)
                    return nullptr;

                auto Iter = m_ResourceSignatures.find(Name);
                if (Iter == m_ResourceSignatures.end())
                    LOG_ERROR_AND_THROW("Unable to find resource signature '", Name, "'.");
                return Iter->second;
            };

            auto UnpackPipelineStateCreateInfo = [&](PipelineStateNotation const& ResourceDescRSN, PipelineStateCreateInfo& ResourceDesc) {
                ResourceDesc.PSODesc                 = ResourceDescRSN.PSODesc;
                ResourceDesc.Flags                   = ResourceDescRSN.Flags;
                ResourceDesc.ResourceSignaturesCount = ResourceDescRSN.ResourceSignaturesNameCount;
                ResourceDesc.ppResourceSignatures    = Allocator.ConstructArray<IPipelineResourceSignature*>(ResourceDescRSN.ResourceSignaturesNameCount);
                for (Uint32 SignatureID = 0; SignatureID < ResourceDesc.ResourceSignaturesCount; SignatureID++)
                    ResourceDesc.ppResourceSignatures[SignatureID] = FindResourceSignature(ResourceDescRSN.ppResourceSignatureNames[SignatureID]);
            };

            for (Uint32 PipelineID = 0; PipelineID < ParserInfo.GraphicsPipelineStateCount; ++PipelineID)
            {
                auto pResourceDescRSN = pNotationParser->GetGraphicsPipelineStateByIndex(PipelineID);

                GraphicsPipelineStateCreateInfo ResourceDesc = {};
                UnpackPipelineStateCreateInfo(*pResourceDescRSN, ResourceDesc);
                ResourceDesc.GraphicsPipeline             = static_cast<GraphicsPipelineDesc>(pResourceDescRSN->Desc);
                ResourceDesc.GraphicsPipeline.pRenderPass = FindRenderPass(pResourceDescRSN->pRenderPassName);

                ResourceDesc.pVS = FindShader(pResourceDescRSN->pVSName);
                ResourceDesc.pPS = FindShader(pResourceDescRSN->pPSName);
                ResourceDesc.pDS = FindShader(pResourceDescRSN->pDSName);
                ResourceDesc.pHS = FindShader(pResourceDescRSN->pHSName);
                ResourceDesc.pGS = FindShader(pResourceDescRSN->pGSName);
                ResourceDesc.pAS = FindShader(pResourceDescRSN->pASName);
                ResourceDesc.pMS = FindShader(pResourceDescRSN->pMSName);

                PipelineStateArchiveInfo ArchiveInfo = {};
                ArchiveInfo.DeviceFlags              = m_DeviceBits;

                if (!pArchive->AddGraphicsPipelineState(ResourceDesc, ArchiveInfo))
                    LOG_ERROR_AND_THROW("Failed to archive graphics pipeline '", ResourceDesc.PSODesc.Name, "'.");
            }

            for (Uint32 PipelineID = 0; PipelineID < ParserInfo.ComputePipelineStateCount; ++PipelineID)
            {
                auto                           pResourceDescRSN = pNotationParser->GetComputePipelineStateByIndex(PipelineID);
                ComputePipelineStateCreateInfo ResourceDesc     = {};

                UnpackPipelineStateCreateInfo(*pResourceDescRSN, ResourceDesc);
                ResourceDesc.pCS = FindShader(pResourceDescRSN->pCSName);

                PipelineStateArchiveInfo ArchiveInfo = {};
                ArchiveInfo.DeviceFlags              = m_DeviceBits;

                if (!pArchive->AddComputePipelineState(ResourceDesc, ArchiveInfo))
                    LOG_ERROR_AND_THROW("Failed to archive compute pipeline '", ResourceDesc.PSODesc.Name, "'.");
            }

            for (Uint32 PipelineID = 0; PipelineID < ParserInfo.TilePipelineStateCount; ++PipelineID)
            {
                auto pResourceDescRSN = pNotationParser->GetTilePipelineStateByIndex(PipelineID);

                TilePipelineStateCreateInfo ResourceDesc = {};
                UnpackPipelineStateCreateInfo(*pResourceDescRSN, ResourceDesc);

                ResourceDesc.pTS = FindShader(pResourceDescRSN->pTSName);

                PipelineStateArchiveInfo ArchiveInfo = {};
                ArchiveInfo.DeviceFlags              = m_DeviceBits;
                if (!pArchive->AddTilePipelineState(ResourceDesc, ArchiveInfo))
                    LOG_ERROR_AND_THROW("Failed to archive tile pipeline '", ResourceDesc.PSODesc.Name, "'.");
            }

            for (Uint32 PipelineID = 0; PipelineID < ParserInfo.RayTracingPipelineStateCount; ++PipelineID)
            {
                auto pResourceDescRSN = pNotationParser->GetRayTracingPipelineStateByIndex(PipelineID);

                RayTracingPipelineStateCreateInfo ResourceDesc = {};
                UnpackPipelineStateCreateInfo(*pResourceDescRSN, ResourceDesc);

                ResourceDesc.RayTracingPipeline = pResourceDescRSN->RayTracingPipeline;
                ResourceDesc.pShaderRecordName  = pResourceDescRSN->pShaderRecordName;
                ResourceDesc.MaxAttributeSize   = pResourceDescRSN->MaxAttributeSize;
                ResourceDesc.MaxPayloadSize     = pResourceDescRSN->MaxPayloadSize;

                {
                    auto pData = Allocator.ConstructArray<RayTracingGeneralShaderGroup>(pResourceDescRSN->GeneralShaderCount);

                    for (Uint32 ShaderID = 0; ShaderID < pResourceDescRSN->GeneralShaderCount; ShaderID++)
                    {
                        pData[ShaderID].Name    = pResourceDescRSN->pGeneralShaders[ShaderID].Name;
                        pData[ShaderID].pShader = FindShader(pResourceDescRSN->pGeneralShaders[ShaderID].pShaderName);
                    }

                    ResourceDesc.pGeneralShaders    = pData;
                    ResourceDesc.GeneralShaderCount = pResourceDescRSN->GeneralShaderCount;
                }

                {
                    auto pData = Allocator.ConstructArray<RayTracingTriangleHitShaderGroup>(pResourceDescRSN->TriangleHitShaderCount);

                    for (Uint32 ShaderID = 0; ShaderID < pResourceDescRSN->TriangleHitShaderCount; ++ShaderID)
                    {
                        pData[ShaderID].Name              = pResourceDescRSN->pTriangleHitShaders[ShaderID].Name;
                        pData[ShaderID].pAnyHitShader     = FindShader(pResourceDescRSN->pTriangleHitShaders[ShaderID].pAnyHitShaderName);
                        pData[ShaderID].pClosestHitShader = FindShader(pResourceDescRSN->pTriangleHitShaders[ShaderID].pClosestHitShaderName);
                    }

                    ResourceDesc.pTriangleHitShaders    = pData;
                    ResourceDesc.TriangleHitShaderCount = pResourceDescRSN->TriangleHitShaderCount;
                }

                {
                    auto pData = Allocator.ConstructArray<RayTracingProceduralHitShaderGroup>(pResourceDescRSN->ProceduralHitShaderCount);

                    for (Uint32 ShaderID = 0; ShaderID < pResourceDescRSN->ProceduralHitShaderCount; ++ShaderID)
                    {
                        pData[ShaderID].Name                = pResourceDescRSN->pProceduralHitShaders[ShaderID].Name;
                        pData[ShaderID].pAnyHitShader       = FindShader(pResourceDescRSN->pProceduralHitShaders[ShaderID].pAnyHitShaderName);
                        pData[ShaderID].pIntersectionShader = FindShader(pResourceDescRSN->pProceduralHitShaders[ShaderID].pClosestHitShaderName);
                        pData[ShaderID].pClosestHitShader   = FindShader(pResourceDescRSN->pProceduralHitShaders[ShaderID].pClosestHitShaderName);
                    }

                    ResourceDesc.pProceduralHitShaders    = pData;
                    ResourceDesc.ProceduralHitShaderCount = pResourceDescRSN->ProceduralHitShaderCount;
                }

                PipelineStateArchiveInfo ArchiveInfo = {};
                ArchiveInfo.DeviceFlags              = m_DeviceBits;
                if (!pArchive->AddRayTracingPipelineState(ResourceDesc, ArchiveInfo))
                    LOG_ERROR_AND_THROW("Failed to archive ray tracing pipeline '", ResourceDesc.PSODesc.Name, "'.");
            }
        }
    }
    catch (...)
    {
        return false;
    }
    return true;
}

void RenderStatePackager::Reset()
{
    m_RSNParsers.clear();
    m_RenderPasses.clear();
    m_Shaders.clear();
    m_ResourceSignatures.clear();
}

} // namespace Diligent
