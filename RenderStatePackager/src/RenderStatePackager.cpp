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

#include "RenderStatePackager.hpp"
#include "DefaultRawMemoryAllocator.hpp"
#include "DynamicLinearAllocator.hpp"

namespace Diligent
{

RenderStatePackager::RenderStatePackager(RefCntAutoPtr<ISerializationDevice>            pDevice,
                                         RefCntAutoPtr<IShaderSourceInputStreamFactory> pStreamFactory,
                                         ARCHIVE_DEVICE_DATA_FLAGS                      DeviceBits) :
    m_pDevice{pDevice},
    m_pStreamFactory{pStreamFactory},
    m_DeviceBits{DeviceBits}
{
}

bool RenderStatePackager::Execute(IArchiver* pArchive, const IRenderStateNotationParser* pNotationParser)
{
    DEV_CHECK_ERR(pArchive != nullptr, "pArchive must not be null");
    DEV_CHECK_ERR(pNotationParser != nullptr, "pNotationParser must not be null");
    if (pArchive == nullptr || pNotationParser == nullptr)
        return false;

    try
    {
        DynamicLinearAllocator Allocator{DefaultRawMemoryAllocator::GetAllocator()};

        auto const& ParserInfo = pNotationParser->GetInfo();

        for (Uint32 ShaderID = 0; ShaderID < ParserInfo.ShaderCount; ShaderID++)
        {
            ShaderCreateInfo ShaderCI           = *pNotationParser->GetShaderByIndex(ShaderID);
            ShaderCI.pShaderSourceStreamFactory = m_pStreamFactory;

            RefCntAutoPtr<IShader> pShader;
            m_pDevice->CreateShader(ShaderCI, m_DeviceBits, &pShader);
            if (!pShader)
                LOG_ERROR_AND_THROW("Failed to create shader from file '", ShaderCI.FilePath, "'.");
            m_Shaders.emplace(ShaderCI.Desc.Name, pShader);
        }

        for (Uint32 RenderPassID = 0; RenderPassID < ParserInfo.RenderPassCount; RenderPassID++)
        {
            RenderPassDesc             RPDesc = *pNotationParser->GetRenderPassByIndex(RenderPassID);
            RefCntAutoPtr<IRenderPass> pRenderPass;
            m_pDevice->CreateRenderPass(RPDesc, &pRenderPass);
            if (!pRenderPass)
                LOG_ERROR_AND_THROW("Failed to create render pass '", RPDesc.Name, "'.");
            m_RenderPasses.emplace(RPDesc.Name, pRenderPass);
        }

        for (Uint32 SignatureID = 0; SignatureID < ParserInfo.ResourceSignatureCount; SignatureID++)
        {
            PipelineResourceSignatureDesc             SignDesc = *pNotationParser->GetResourceSignatureByIndex(SignatureID);
            RefCntAutoPtr<IPipelineResourceSignature> pSignature;
            m_pDevice->CreatePipelineResourceSignature(SignDesc, m_DeviceBits, &pSignature);
            if (!pSignature)
                LOG_ERROR_AND_THROW("Failed to create resource signature '", SignDesc.Name, "'.");
            m_ResourceSignatures.emplace(SignDesc.Name, pSignature);
        }

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

        for (Uint32 PipelineID = 0; PipelineID < ParserInfo.GraphicsPipelineStateCount; PipelineID++)
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

        for (Uint32 PipelineID = 0; PipelineID < ParserInfo.ComputePipelineStateCount; PipelineID++)
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

        for (Uint32 PipelineID = 0; PipelineID < ParserInfo.TilePipelineStateCount; PipelineID++)
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

        for (Uint32 PipelineID = 0; PipelineID < ParserInfo.RayTracingPipelineStateCount; PipelineID++)
        {
            auto pResourceDescRSN = pNotationParser->GetRayTracingPipelineStateByIndex(PipelineID);

            RayTracingPipelineStateCreateInfo ResourceDesc = {};
            UnpackPipelineStateCreateInfo(*pResourceDescRSN, ResourceDesc);

            ResourceDesc.RayTracingPipeline = pResourceDescRSN->Desc;
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

                for (Uint32 ShaderID = 0; ShaderID < pResourceDescRSN->TriangleHitShaderCount; ShaderID++)
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

                for (Uint32 ShaderID = 0; ShaderID < pResourceDescRSN->ProceduralHitShaderCount; ShaderID++)
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

        return true;
    }
    catch (...)
    {
        return false;
    }
}

void RenderStatePackager::Flush()
{
    m_RenderPasses.clear();
    m_Shaders.clear();
    m_ResourceSignatures.clear();
}

} // namespace Diligent
