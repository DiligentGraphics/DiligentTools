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

#include <cstring>
#include "RenderStatePackager.hpp"
#include "DefaultRawMemoryAllocator.hpp"
#include "DynamicLinearAllocator.hpp"

namespace Diligent
{

RenderStatePackager::RenderStatePackager(RefCntAutoPtr<ISerializationDevice> pDevice, RefCntAutoPtr<IShaderSourceInputStreamFactory> pStreamFactory, RENDER_DEVICE_TYPE_FLAGS DeviceBits) :
    m_pDevice(pDevice), m_pStreamFactory(pStreamFactory), m_DeviceBits(DeviceBits)
{
}

void RenderStatePackager::Execute(const IRenderStateMarkupParser* pDescriptorParser, IArchiver* pArchive)
{
    DynamicLinearAllocator Allocator{DefaultRawMemoryAllocator::GetAllocator()};

    for (Uint32 ShaderID = 0; ShaderID < pDescriptorParser->GetShaderCount(); ShaderID++)
    {
        ShaderCreateInfo ResourceDesc = {};
        pDescriptorParser->GetShaderByIndex(ShaderID, ResourceDesc);
        ResourceDesc.pShaderSourceStreamFactory = m_pStreamFactory;

        RefCntAutoPtr<IShader> pResource;
        m_pDevice->CreateShader(ResourceDesc, m_DeviceBits, &pResource);
        m_Shaders.emplace(ResourceDesc.Desc.Name, pResource);
    }

    for (Uint32 RenderPassID = 0; RenderPassID < pDescriptorParser->GetRenderPassCount(); RenderPassID++)
    {
        RenderPassDesc ResourceDesc = {};
        pDescriptorParser->GetRenderPassByIndex(RenderPassID, ResourceDesc);

        RefCntAutoPtr<IRenderPass> pResource;
        m_pDevice->CreateRenderPass(ResourceDesc, &pResource);
        m_RenderPasses.emplace(ResourceDesc.Name, pResource);
    }

    for (Uint32 SignatureID = 0; SignatureID < pDescriptorParser->GetResourceSignatureCount(); SignatureID++)
    {
        PipelineResourceSignatureDesc ResourceDesc = {};
        pDescriptorParser->GetResourceSignatureByIndex(SignatureID, ResourceDesc);

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
            LOG_ERROR_AND_THROW("Shader isn't founded -> '", Name, "'.");
        return nullptr;
    };

    auto FindRenderPass = [&](const char* Name) -> IRenderPass* {
        if (Name == nullptr)
            return nullptr;

        auto Iter = m_RenderPasses.find(Name);
        if (Iter != m_RenderPasses.end())
            return Iter->second;
        else
            LOG_ERROR_AND_THROW("Render Pass isn't founded -> '", Name, "'.");
        return nullptr;
    };

    auto FindResourceSignature = [&](const char* Name) -> IPipelineResourceSignature* {
        if (Name == nullptr)
            return nullptr;

        auto Iter = m_ResourceSignatures.find(Name);
        if (Iter != m_ResourceSignatures.end())
            return Iter->second;
        else
            LOG_ERROR_AND_THROW("Resource Signature isn't founded -> '", Name, "'.");
        return nullptr;
    };

    auto UnpackResourceSignatures = [&](PipelineStateCreateMarkup const& ResourceDescJSON, PipelineStateCreateInfo& ResourceDesc) {
        ResourceDesc.ResourceSignaturesCount = ResourceDescJSON.ResourceSignaturesCount;
        ResourceDesc.ppResourceSignatures    = Allocator.ConstructArray<IPipelineResourceSignature*>(ResourceDescJSON.ResourceSignaturesCount);
        for (Uint32 SignatureID = 0; SignatureID < ResourceDesc.ResourceSignaturesCount; SignatureID++)
            ResourceDesc.ppResourceSignatures[SignatureID] = FindResourceSignature(ResourceDescJSON.ppResourceSignatures[SignatureID]);
    };


    for (Uint32 PipelineID = 0; PipelineID < pDescriptorParser->GetGraphicsPipelineStateCount(); PipelineID++)
    {
        GraphicsPipelineStateCreateInfo   ResourceDesc     = {};
        GraphicsPipelineStateCreateMarkup ResourceDescJSON = {};
        pDescriptorParser->GetGraphicsPipelineStateByIndex(PipelineID, ResourceDesc, ResourceDescJSON);

        UnpackResourceSignatures(ResourceDescJSON, ResourceDesc);

        ResourceDesc.pVS = FindShader(ResourceDescJSON.pVS);
        ResourceDesc.pPS = FindShader(ResourceDescJSON.pPS);
        ResourceDesc.pDS = FindShader(ResourceDescJSON.pDS);
        ResourceDesc.pHS = FindShader(ResourceDescJSON.pHS);
        ResourceDesc.pGS = FindShader(ResourceDescJSON.pGS);
        ResourceDesc.pAS = FindShader(ResourceDescJSON.pAS);
        ResourceDesc.pMS = FindShader(ResourceDescJSON.pMS);

        ResourceDesc.GraphicsPipeline.pRenderPass = FindRenderPass(ResourceDescJSON.GraphicsPipeline.pRenderPass);

        PipelineStateArchiveInfo ArchiveInfo = {};
        ArchiveInfo.DeviceFlags              = m_DeviceBits;
        pArchive->AddGraphicsPipelineState(ResourceDesc, ArchiveInfo);
    }

    for (Uint32 PipelineID = 0; PipelineID < pDescriptorParser->GetComputePipelineStateCount(); PipelineID++)
    {
        ComputePipelineStateCreateInfo   ResourceDesc     = {};
        ComputePipelineStateCreateMarkup ResourceDescJSON = {};
        pDescriptorParser->GetComputePipelineStateByIndex(PipelineID, ResourceDesc, ResourceDescJSON);

        UnpackResourceSignatures(ResourceDescJSON, ResourceDesc);

        ResourceDesc.pCS = FindShader(ResourceDescJSON.pCS);

        PipelineStateArchiveInfo ArchiveInfo = {};
        ArchiveInfo.DeviceFlags              = m_DeviceBits;
        pArchive->AddComputePipelineState(ResourceDesc, ArchiveInfo);
    }

    for (Uint32 PipelineID = 0; PipelineID < pDescriptorParser->GetTilePipelineStateCount(); PipelineID++)
    {
        TilePipelineStateCreateInfo   ResourceDesc     = {};
        TilePipelineStateCreateMarkup ResourceDescJSON = {};
        pDescriptorParser->GetTilePipelineStateByIndex(PipelineID, ResourceDesc, ResourceDescJSON);

        UnpackResourceSignatures(ResourceDescJSON, ResourceDesc);

        ResourceDesc.pTS = FindShader(ResourceDescJSON.pTS);

        PipelineStateArchiveInfo ArchiveInfo = {};
        ArchiveInfo.DeviceFlags              = m_DeviceBits;
        pArchive->AddTilePipelineState(ResourceDesc, ArchiveInfo);
    }

    for (Uint32 PipelineID = 0; PipelineID < pDescriptorParser->GetRayTracingPipelineStateCount(); PipelineID++)
    {
        RayTracingPipelineStateCreateInfo   ResourceDesc     = {};
        RayTracingPipelineStateCreateMarkup ResourceDescJSON = {};
        pDescriptorParser->GetRayTracingPipelineStateByIndex(PipelineID, ResourceDesc, ResourceDescJSON);

        UnpackResourceSignatures(ResourceDescJSON, ResourceDesc);

        VERIFY_EXPR(ResourceDesc.GeneralShaderCount == ResourceDescJSON.GeneralShaderCount);
        for (Uint32 ShaderID = 0; ShaderID < ResourceDescJSON.GeneralShaderCount; ShaderID++)
        {
            const_cast<RayTracingGeneralShaderGroup*>(ResourceDesc.pGeneralShaders)[ShaderID].pShader = FindShader(ResourceDescJSON.pGeneralShaders[ShaderID].pShader);
        }

        VERIFY_EXPR(ResourceDesc.TriangleHitShaderCount == ResourceDescJSON.TriangleHitShaderCount);
        for (Uint32 ShaderID = 0; ShaderID < ResourceDescJSON.TriangleHitShaderCount; ShaderID++)
        {
            const_cast<RayTracingTriangleHitShaderGroup*>(ResourceDesc.pTriangleHitShaders)[ShaderID].pAnyHitShader     = FindShader(ResourceDescJSON.pTriangleHitShaders[ShaderID].pAnyHitShader);
            const_cast<RayTracingTriangleHitShaderGroup*>(ResourceDesc.pTriangleHitShaders)[ShaderID].pClosestHitShader = FindShader(ResourceDescJSON.pTriangleHitShaders[ShaderID].pClosestHitShader);
        }

        VERIFY_EXPR(ResourceDesc.ProceduralHitShaderCount == ResourceDescJSON.ProceduralHitShaderCount);
        for (Uint32 ShaderID = 0; ShaderID < ResourceDescJSON.ProceduralHitShaderCount; ShaderID++)
        {
            const_cast<RayTracingProceduralHitShaderGroup*>(ResourceDesc.pProceduralHitShaders)[ShaderID].pAnyHitShader       = FindShader(ResourceDescJSON.pProceduralHitShaders[ShaderID].pAnyHitShader);
            const_cast<RayTracingProceduralHitShaderGroup*>(ResourceDesc.pProceduralHitShaders)[ShaderID].pIntersectionShader = FindShader(ResourceDescJSON.pProceduralHitShaders[ShaderID].pIntersectionShader);
            const_cast<RayTracingProceduralHitShaderGroup*>(ResourceDesc.pProceduralHitShaders)[ShaderID].pClosestHitShader   = FindShader(ResourceDescJSON.pProceduralHitShaders[ShaderID].pClosestHitShader);
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
