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

void RenderStatePackager::Execute(const IRenderStateNotationParser* pDescriptorParser, IArchiver* pArchive)
{
    DynamicLinearAllocator Allocator{DefaultRawMemoryAllocator::GetAllocator()};

    auto const& ParserInfo = pDescriptorParser->GetInfo();

    for (Uint32 ShaderID = 0; ShaderID < ParserInfo.ShaderCount; ShaderID++)
    {
        auto pResourceNotation = pDescriptorParser->GetShaderByIndex(ShaderID);

        ShaderCreateInfo ResourceDesc           = {};
        ResourceDesc.FilePath                   = pResourceNotation->FilePath;
        ResourceDesc.EntryPoint                 = pResourceNotation->EntryPoint;
        ResourceDesc.Macros                     = pResourceNotation->Macros;
        ResourceDesc.UseCombinedTextureSamplers = pResourceNotation->UseCombinedTextureSamplers;
        ResourceDesc.CombinedSamplerSuffix      = pResourceNotation->CombinedSamplerSuffix;
        ResourceDesc.Desc                       = pResourceNotation->Desc;
        ResourceDesc.SourceLanguage             = pResourceNotation->SourceLanguage;
        ResourceDesc.ShaderCompiler             = pResourceNotation->ShaderCompiler;
        ResourceDesc.HLSLVersion                = pResourceNotation->HLSLVersion;
        ResourceDesc.GLSLVersion                = pResourceNotation->GLSLVersion;
        ResourceDesc.GLESSLVersion              = pResourceNotation->GLESSLVersion;
        ResourceDesc.CompileFlags               = pResourceNotation->CompileFlags;
        ResourceDesc.pShaderSourceStreamFactory = m_pStreamFactory;

        RefCntAutoPtr<IShader> pResource;
        m_pDevice->CreateShader(ResourceDesc, m_DeviceBits, &pResource);
        m_Shaders.emplace(ResourceDesc.Desc.Name, pResource);
    }

    for (Uint32 RenderPassID = 0; RenderPassID < ParserInfo.RenderPassCount; RenderPassID++)
    {
        auto pResourceNotation = pDescriptorParser->GetRenderPassByIndex(RenderPassID);

        RenderPassDesc             ResourceDesc = *pResourceNotation;
        RefCntAutoPtr<IRenderPass> pResource;
        m_pDevice->CreateRenderPass(ResourceDesc, &pResource);
        m_RenderPasses.emplace(ResourceDesc.Name, pResource);
    }

    for (Uint32 SignatureID = 0; SignatureID < ParserInfo.ResourceSignatureCount; SignatureID++)
    {
        auto pResourceNotation = pDescriptorParser->GetResourceSignatureByIndex(SignatureID);

        PipelineResourceSignatureDesc             ResourceDesc = *pResourceNotation;
        RefCntAutoPtr<IPipelineResourceSignature> pResource;
        m_pDevice->CreatePipelineResourceSignature(ResourceDesc, m_DeviceBits, &pResource);
        m_ResourceSignatures.emplace(ResourceDesc.Name, pResource);
    }

    auto FindShader = [&](const char* Name) -> IShader* {
        if (Name == nullptr)
            return nullptr;

        auto Iter = m_Shaders.find(Name);
        if (Iter != m_Shaders.end())
            return Iter->second;
        else
            LOG_ERROR_AND_THROW("Shader isn't found -> '", Name, "'.");
        return nullptr;
    };

    auto FindRenderPass = [&](const char* Name) -> IRenderPass* {
        if (Name == nullptr)
            return nullptr;

        auto Iter = m_RenderPasses.find(Name);
        if (Iter != m_RenderPasses.end())
            return Iter->second;
        else
            LOG_ERROR_AND_THROW("Render Pass isn't found -> '", Name, "'.");
        return nullptr;
    };

    auto FindResourceSignature = [&](const char* Name) -> IPipelineResourceSignature* {
        if (Name == nullptr)
            return nullptr;

        auto Iter = m_ResourceSignatures.find(Name);
        if (Iter != m_ResourceSignatures.end())
            return Iter->second;
        else
            LOG_ERROR_AND_THROW("Resource Signature isn't found -> '", Name, "'.");
        return nullptr;
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
        auto pResourceNotation = pDescriptorParser->GetGraphicsPipelineStateByIndex(PipelineID);

        GraphicsPipelineStateCreateInfo ResourceDesc = {};
        UnpackPipelineStateCreateInfo(*pResourceNotation, ResourceDesc);
        ResourceDesc.GraphicsPipeline             = static_cast<GraphicsPipelineDesc>(pResourceNotation->Desc);
        ResourceDesc.GraphicsPipeline.pRenderPass = FindRenderPass(pResourceNotation->pRenderPassName);

        ResourceDesc.pVS = FindShader(pResourceNotation->pVSName);
        ResourceDesc.pPS = FindShader(pResourceNotation->pPSName);
        ResourceDesc.pDS = FindShader(pResourceNotation->pDSName);
        ResourceDesc.pHS = FindShader(pResourceNotation->pHSName);
        ResourceDesc.pGS = FindShader(pResourceNotation->pGSName);
        ResourceDesc.pAS = FindShader(pResourceNotation->pASName);
        ResourceDesc.pMS = FindShader(pResourceNotation->pMSName);

        PipelineStateArchiveInfo ArchiveInfo = {};
        ArchiveInfo.DeviceFlags              = m_DeviceBits;
        pArchive->AddGraphicsPipelineState(ResourceDesc, ArchiveInfo);
    }

    for (Uint32 PipelineID = 0; PipelineID < ParserInfo.ComputePipelineStateCount; PipelineID++)
    {
        auto                           pResourceNotation = pDescriptorParser->GetComputePipelineStateByIndex(PipelineID);
        ComputePipelineStateCreateInfo ResourceDesc      = {};

        UnpackPipelineStateCreateInfo(*pResourceNotation, ResourceDesc);
        ResourceDesc.pCS = FindShader(pResourceNotation->pCSName);

        PipelineStateArchiveInfo ArchiveInfo = {};
        ArchiveInfo.DeviceFlags              = m_DeviceBits;
        pArchive->AddComputePipelineState(ResourceDesc, ArchiveInfo);
    }

    for (Uint32 PipelineID = 0; PipelineID < ParserInfo.TilePipelineStateCount; PipelineID++)
    {
        auto pResourceNotation = pDescriptorParser->GetTilePipelineStateByIndex(PipelineID);

        TilePipelineStateCreateInfo ResourceDesc = {};
        UnpackPipelineStateCreateInfo(*pResourceNotation, ResourceDesc);

        ResourceDesc.pTS = FindShader(pResourceNotation->pTSName);

        PipelineStateArchiveInfo ArchiveInfo = {};
        ArchiveInfo.DeviceFlags              = m_DeviceBits;
        pArchive->AddTilePipelineState(ResourceDesc, ArchiveInfo);
    }

    for (Uint32 PipelineID = 0; PipelineID < ParserInfo.RayTracingPipelineStateCount; PipelineID++)
    {
        auto pResourceNotation = pDescriptorParser->GetRayTracingPipelineStateByIndex(PipelineID);

        RayTracingPipelineStateCreateInfo ResourceDesc = {};
        UnpackPipelineStateCreateInfo(*pResourceNotation, ResourceDesc);

        ResourceDesc.RayTracingPipeline = pResourceNotation->Desc;
        ResourceDesc.pShaderRecordName  = pResourceNotation->pShaderRecordName;
        ResourceDesc.MaxAttributeSize   = pResourceNotation->MaxAttributeSize;
        ResourceDesc.MaxPayloadSize     = pResourceNotation->MaxPayloadSize;

        {
            auto pData = Allocator.ConstructArray<RayTracingGeneralShaderGroup>(pResourceNotation->GeneralShaderCount);

            for (Uint32 ShaderID = 0; ShaderID < pResourceNotation->GeneralShaderCount; ShaderID++)
            {
                pData[ShaderID].Name    = pResourceNotation->pGeneralShaders[ShaderID].Name;
                pData[ShaderID].pShader = FindShader(pResourceNotation->pGeneralShaders[ShaderID].pShaderName);
            }

            ResourceDesc.pGeneralShaders    = pData;
            ResourceDesc.GeneralShaderCount = pResourceNotation->GeneralShaderCount;
        }

        {
            auto pData = Allocator.ConstructArray<RayTracingTriangleHitShaderGroup>(pResourceNotation->TriangleHitShaderCount);

            for (Uint32 ShaderID = 0; ShaderID < pResourceNotation->TriangleHitShaderCount; ShaderID++)
            {
                pData[ShaderID].Name              = pResourceNotation->pTriangleHitShaders[ShaderID].Name;
                pData[ShaderID].pAnyHitShader     = FindShader(pResourceNotation->pTriangleHitShaders[ShaderID].pAnyHitShaderName);
                pData[ShaderID].pClosestHitShader = FindShader(pResourceNotation->pTriangleHitShaders[ShaderID].pClosestHitShaderName);
            }

            ResourceDesc.pTriangleHitShaders    = pData;
            ResourceDesc.TriangleHitShaderCount = pResourceNotation->TriangleHitShaderCount;
        }

        {
            auto pData = Allocator.ConstructArray<RayTracingProceduralHitShaderGroup>(pResourceNotation->ProceduralHitShaderCount);

            for (Uint32 ShaderID = 0; ShaderID < pResourceNotation->ProceduralHitShaderCount; ShaderID++)
            {
                pData[ShaderID].Name                = pResourceNotation->pProceduralHitShaders[ShaderID].Name;
                pData[ShaderID].pAnyHitShader       = FindShader(pResourceNotation->pProceduralHitShaders[ShaderID].pAnyHitShaderName);
                pData[ShaderID].pIntersectionShader = FindShader(pResourceNotation->pProceduralHitShaders[ShaderID].pClosestHitShaderName);
                pData[ShaderID].pClosestHitShader   = FindShader(pResourceNotation->pProceduralHitShaders[ShaderID].pClosestHitShaderName);
            }

            ResourceDesc.pProceduralHitShaders    = pData;
            ResourceDesc.ProceduralHitShaderCount = pResourceNotation->ProceduralHitShaderCount;
        }

        PipelineStateArchiveInfo ArchiveInfo = {};
        ArchiveInfo.DeviceFlags              = m_DeviceBits;
        pArchive->AddRayTracingPipelineState(ResourceDesc, ArchiveInfo);
    }
}

void RenderStatePackager::Flush()
{
    m_RenderPasses.clear();
    m_Shaders.clear();
    m_ResourceSignatures.clear();
}

} // namespace Diligent
