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

#include "RenderStateNotationLoaderImpl.hpp"
#include "DefaultRawMemoryAllocator.hpp"

namespace Diligent
{
RenderStateNotationLoaderImpl::RenderStateNotationLoaderImpl(IReferenceCounters* pRefCounters, const RenderStateNotationLoaderCreateInfo& CreateInfo) :
    TBase{pRefCounters},
    m_pDevice{CreateInfo.pDevice},
    m_pParser{CreateInfo.pParser},
    m_pStreamFactory{CreateInfo.pStreamFactory}
{
    VERIFY_EXPR(CreateInfo.pDevice != nullptr && CreateInfo.pParser != nullptr);
}

void RenderStateNotationLoaderImpl::LoadPipelineState(const LoadPipelineStateInfo& LoadInfo, IPipelineState** ppPSO)
{
    DEV_CHECK_ERR(LoadInfo.Name != nullptr, "LoadInfo.Name  must not be null");
    DEV_CHECK_ERR(ppPSO != nullptr, "ppPSO must not be null");

    try
    {
        auto FindPipeline = [this](const Char* Name, PIPELINE_TYPE PipelineType) //
        {
            const auto Iter = m_PipelineStateCache.find(std::make_pair(HashMapStringKey{Name, true}, PipelineType));
            if (Iter != m_PipelineStateCache.end())
                return Iter->second;
            return RefCntAutoPtr<IPipelineState>{};
        };

        RefCntAutoPtr<IPipelineState> pPipeline;

        if (LoadInfo.PipelineType != PIPELINE_TYPE_INVALID)
        {
            pPipeline = FindPipeline(LoadInfo.Name, LoadInfo.PipelineType);
        }
        else
        {
            PIPELINE_TYPE PipelineTypes[] = {
                PIPELINE_TYPE_GRAPHICS,
                PIPELINE_TYPE_MESH,
                PIPELINE_TYPE_COMPUTE,
                PIPELINE_TYPE_RAY_TRACING,
                PIPELINE_TYPE_TILE};

            for (Uint32 i = 0; i < _countof(PipelineTypes) && pPipeline == nullptr; i++)
                pPipeline = FindPipeline(LoadInfo.Name, PipelineTypes[i]);
        }

        if (pPipeline == nullptr)
        {
            DynamicLinearAllocator Allocator{DefaultRawMemoryAllocator::GetAllocator()};

            std::vector<RefCntAutoPtr<IShader>>                    PipelineShaders;
            std::vector<RefCntAutoPtr<IPipelineResourceSignature>> PipelineSignatures;
            RefCntAutoPtr<IRenderPass>                             pRenderPass;

            auto FindShader = [&](const char* Name) -> IShader* //
            {
                if (Name == nullptr)
                    return nullptr;

                RefCntAutoPtr<IShader> pShader;
                LoadShader(LoadShaderInfo{Name, LoadInfo.AddToCache}, &pShader);

                if (!pShader)
                    LOG_ERROR_AND_THROW("Failed to load shader '", Name, "' for pipeline '", LoadInfo.Name, "'.");

                PipelineShaders.push_back(pShader);
                return pShader;
            };

            auto FindRenderPass = [&](const char* Name) -> IRenderPass* //
            {
                if (Name == nullptr)
                    return nullptr;

                VERIFY_EXPR(!pRenderPass);
                LoadRenderPass(LoadRenderPassInfo{Name, LoadInfo.AddToCache}, &pRenderPass);

                if (!pRenderPass)
                    LOG_ERROR_AND_THROW("Failed to load render pass '", Name, "' for pipeline '", LoadInfo.Name, "'.");
                return pRenderPass;
            };

            auto FindResourceSignature = [&](const char* Name) -> IPipelineResourceSignature* //
            {
                if (Name == nullptr)
                    return nullptr;

                RefCntAutoPtr<IPipelineResourceSignature> pResourceSignature;
                LoadResourceSignature(LoadResourceSignatureInfo{Name, LoadInfo.AddToCache}, &pResourceSignature);

                if (!pResourceSignature)
                    LOG_ERROR_AND_THROW("Failed to load resource signature '", Name, "' for pipeline '", LoadInfo.Name, "'.");

                PipelineSignatures.push_back(pResourceSignature);
                return pResourceSignature;
            };

            auto UnpackPipelineStateCreateInfo = [&](PipelineStateNotation const& DescRSN, PipelineStateCreateInfo& PipelineCI) //
            {
                PipelineCI.PSODesc                 = DescRSN.PSODesc;
                PipelineCI.Flags                   = DescRSN.Flags;
                PipelineCI.ResourceSignaturesCount = DescRSN.ResourceSignaturesNameCount;
                PipelineCI.ppResourceSignatures    = Allocator.ConstructArray<IPipelineResourceSignature*>(DescRSN.ResourceSignaturesNameCount);
                for (Uint32 SignatureID = 0; SignatureID < PipelineCI.ResourceSignaturesCount; ++SignatureID)
                    PipelineCI.ppResourceSignatures[SignatureID] = FindResourceSignature(DescRSN.ppResourceSignatureNames[SignatureID]);
            };

            const auto* pDescRSN = m_pParser->GetPipelineStateByName(LoadInfo.Name, LoadInfo.PipelineType);
            if (!pDescRSN)
                LOG_ERROR_AND_THROW("Failed to find pipeline '", LoadInfo.Name, "'.");

            const auto PipelineType = pDescRSN->PSODesc.PipelineType;
            switch (PipelineType)
            {
                case PIPELINE_TYPE_GRAPHICS:
                case PIPELINE_TYPE_MESH:
                {
                    const auto* pPipelineDescRSN = static_cast<const GraphicsPipelineNotation*>(pDescRSN);

                    GraphicsPipelineStateCreateInfo PipelineCI{};
                    UnpackPipelineStateCreateInfo(*pPipelineDescRSN, PipelineCI);
                    PipelineCI.GraphicsPipeline             = static_cast<const GraphicsPipelineDesc&>(pPipelineDescRSN->Desc);
                    PipelineCI.GraphicsPipeline.pRenderPass = FindRenderPass(pPipelineDescRSN->pRenderPassName);

                    PipelineCI.pVS = FindShader(pPipelineDescRSN->pVSName);
                    PipelineCI.pPS = FindShader(pPipelineDescRSN->pPSName);
                    PipelineCI.pDS = FindShader(pPipelineDescRSN->pDSName);
                    PipelineCI.pHS = FindShader(pPipelineDescRSN->pHSName);
                    PipelineCI.pGS = FindShader(pPipelineDescRSN->pGSName);
                    PipelineCI.pAS = FindShader(pPipelineDescRSN->pASName);
                    PipelineCI.pMS = FindShader(pPipelineDescRSN->pMSName);

                    if (LoadInfo.Modify != nullptr)
                        LoadInfo.Modify(&PipelineCI, LoadInfo.pUserData);

                    m_pDevice->CreateGraphicsPipelineState(PipelineCI, &pPipeline);
                    if (!pPipeline)
                        LOG_ERROR_AND_THROW("Failed to create graphics pipeline '", PipelineCI.PSODesc.Name, "'.");

                    break;
                }
                case PIPELINE_TYPE_COMPUTE:
                {
                    const auto* pPipelineDescRSN = static_cast<const ComputePipelineNotation*>(pDescRSN);

                    ComputePipelineStateCreateInfo PipelineCI{};
                    UnpackPipelineStateCreateInfo(*pPipelineDescRSN, PipelineCI);
                    PipelineCI.pCS = FindShader(pPipelineDescRSN->pCSName);

                    if (LoadInfo.Modify != nullptr)
                        LoadInfo.Modify(&PipelineCI, LoadInfo.pUserData);

                    m_pDevice->CreateComputePipelineState(PipelineCI, &pPipeline);
                    if (!pPipeline)
                        LOG_ERROR_AND_THROW("Failed to create compute pipeline '", PipelineCI.PSODesc.Name, "'.");

                    break;
                }
                case PIPELINE_TYPE_TILE:
                {
                    const auto* pPipelineDescRSN = static_cast<const TilePipelineNotation*>(pDescRSN);

                    TilePipelineStateCreateInfo PipelineCI{};
                    UnpackPipelineStateCreateInfo(*pPipelineDescRSN, PipelineCI);
                    PipelineCI.pTS = FindShader(pPipelineDescRSN->pTSName);

                    if (LoadInfo.Modify != nullptr)
                        LoadInfo.Modify(&PipelineCI, LoadInfo.pUserData);

                    m_pDevice->CreateTilePipelineState(PipelineCI, &pPipeline);
                    if (!pPipeline)
                        LOG_ERROR_AND_THROW("Failed to create tile pipeline '", PipelineCI.PSODesc.Name, "'.");

                    break;
                }
                case PIPELINE_TYPE_RAY_TRACING:
                {
                    const auto* pPipelineDescRSN = static_cast<const RayTracingPipelineNotation*>(pDescRSN);

                    RayTracingPipelineStateCreateInfo PipelineCI = {};
                    UnpackPipelineStateCreateInfo(*pPipelineDescRSN, PipelineCI);

                    PipelineCI.RayTracingPipeline = pPipelineDescRSN->RayTracingPipeline;
                    PipelineCI.pShaderRecordName  = pPipelineDescRSN->pShaderRecordName;
                    PipelineCI.MaxAttributeSize   = pPipelineDescRSN->MaxAttributeSize;
                    PipelineCI.MaxPayloadSize     = pPipelineDescRSN->MaxPayloadSize;

                    {
                        auto pData = Allocator.ConstructArray<RayTracingGeneralShaderGroup>(pPipelineDescRSN->GeneralShaderCount);

                        for (Uint32 ShaderID = 0; ShaderID < pPipelineDescRSN->GeneralShaderCount; ShaderID++)
                        {
                            pData[ShaderID].Name    = pPipelineDescRSN->pGeneralShaders[ShaderID].Name;
                            pData[ShaderID].pShader = FindShader(pPipelineDescRSN->pGeneralShaders[ShaderID].pShaderName);
                        }

                        PipelineCI.pGeneralShaders    = pData;
                        PipelineCI.GeneralShaderCount = pPipelineDescRSN->GeneralShaderCount;
                    }

                    {
                        auto pData = Allocator.ConstructArray<RayTracingTriangleHitShaderGroup>(pPipelineDescRSN->TriangleHitShaderCount);

                        for (Uint32 ShaderID = 0; ShaderID < pPipelineDescRSN->TriangleHitShaderCount; ++ShaderID)
                        {
                            pData[ShaderID].Name              = pPipelineDescRSN->pTriangleHitShaders[ShaderID].Name;
                            pData[ShaderID].pAnyHitShader     = FindShader(pPipelineDescRSN->pTriangleHitShaders[ShaderID].pAnyHitShaderName);
                            pData[ShaderID].pClosestHitShader = FindShader(pPipelineDescRSN->pTriangleHitShaders[ShaderID].pClosestHitShaderName);
                        }

                        PipelineCI.pTriangleHitShaders    = pData;
                        PipelineCI.TriangleHitShaderCount = pPipelineDescRSN->TriangleHitShaderCount;
                    }

                    {
                        auto pData = Allocator.ConstructArray<RayTracingProceduralHitShaderGroup>(pPipelineDescRSN->ProceduralHitShaderCount);

                        for (Uint32 ShaderID = 0; ShaderID < pPipelineDescRSN->ProceduralHitShaderCount; ++ShaderID)
                        {
                            pData[ShaderID].Name                = pPipelineDescRSN->pProceduralHitShaders[ShaderID].Name;
                            pData[ShaderID].pAnyHitShader       = FindShader(pPipelineDescRSN->pProceduralHitShaders[ShaderID].pAnyHitShaderName);
                            pData[ShaderID].pIntersectionShader = FindShader(pPipelineDescRSN->pProceduralHitShaders[ShaderID].pClosestHitShaderName);
                            pData[ShaderID].pClosestHitShader   = FindShader(pPipelineDescRSN->pProceduralHitShaders[ShaderID].pClosestHitShaderName);
                        }

                        PipelineCI.pProceduralHitShaders    = pData;
                        PipelineCI.ProceduralHitShaderCount = pPipelineDescRSN->ProceduralHitShaderCount;
                    }

                    m_pDevice->CreateRayTracingPipelineState(PipelineCI, &pPipeline);
                    if (!pPipeline)
                        LOG_ERROR_AND_THROW("Failed to create ray tracing pipeline '", PipelineCI.PSODesc.Name, "'.");

                    break;
                }
                default:
                    LOG_ERROR_AND_THROW("Pipeline type is incorrect: '", PipelineType, "'.");
                    break;
            }

            if (LoadInfo.AddToCache)
                m_PipelineStateCache.emplace(std::make_pair(HashMapStringKey{pPipeline->GetDesc().Name, false}, pPipeline->GetDesc().PipelineType), pPipeline);
        }

        *ppPSO = pPipeline.Detach();
    }
    catch (...)
    {
        LOG_ERROR_MESSAGE("Failed to load pipeline state '", LoadInfo.Name, "'.");
    }
}

void RenderStateNotationLoaderImpl::LoadResourceSignature(const LoadResourceSignatureInfo& LoadInfo, IPipelineResourceSignature** ppSignature)
{
    DEV_CHECK_ERR(LoadInfo.Name != nullptr, "LoadInfo.Name  must not be null");
    DEV_CHECK_ERR(ppSignature != nullptr, "ppSignature must not be null");

    try
    {
        auto Iter = m_ResourceSignatureCache.find(LoadInfo.Name);
        if (Iter != m_ResourceSignatureCache.end())
        {
            *ppSignature = Iter->second;
        }
        else
        {
            const auto* pRSNDesc = m_pParser->GetResourceSignatureByName(LoadInfo.Name);
            if (!pRSNDesc)
                LOG_ERROR_AND_THROW("Failed to find resource signature '", LoadInfo.Name, "'.");

            PipelineResourceSignatureDesc RSDesc = *pRSNDesc;
            if (LoadInfo.Modify != nullptr)
                LoadInfo.Modify(&RSDesc, LoadInfo.pUserData);

            RefCntAutoPtr<IPipelineResourceSignature> pSignature;
            m_pDevice->CreatePipelineResourceSignature(RSDesc, &pSignature);

            if (!pSignature)
                LOG_ERROR_AND_THROW("Failed to create resource signature '", RSDesc.Name, "'.");

            if (LoadInfo.AddToCache)
                m_ResourceSignatureCache.emplace(HashMapStringKey{RSDesc.Name, false}, pSignature);

            *ppSignature = pSignature.Detach();
        }
    }
    catch (...)
    {
        LOG_ERROR_MESSAGE("Failed to load resource signature '", LoadInfo.Name, "'.");
    }
}

void RenderStateNotationLoaderImpl::LoadRenderPass(const LoadRenderPassInfo& LoadInfo, IRenderPass** ppRenderPass)
{
    DEV_CHECK_ERR(LoadInfo.Name != nullptr, "LoadInfo.Name  must not be null");
    DEV_CHECK_ERR(ppRenderPass != nullptr, "ppRenderPass must not be null");

    try
    {
        auto Iter = m_RenderPassCache.find(LoadInfo.Name);
        if (Iter != m_RenderPassCache.end())
        {
            *ppRenderPass = Iter->second;
        }
        else
        {
            const auto* pRSNDesc = m_pParser->GetRenderPassByName(LoadInfo.Name);
            if (!pRSNDesc)
                LOG_ERROR_AND_THROW("Failed to find render pass '", LoadInfo.Name, "'.");

            RenderPassDesc RPDesc = *pRSNDesc;
            if (LoadInfo.Modify != nullptr)
                LoadInfo.Modify(&RPDesc, LoadInfo.pUserData);

            RefCntAutoPtr<IRenderPass> pRenderPass;
            m_pDevice->CreateRenderPass(RPDesc, &pRenderPass);

            if (!pRenderPass)
                LOG_ERROR_AND_THROW("Failed to create render pass '", RPDesc.Name, "'.");

            if (LoadInfo.AddToCache)
                m_RenderPassCache.emplace(HashMapStringKey{RPDesc.Name, false}, pRenderPass);

            *ppRenderPass = pRenderPass.Detach();
        }
    }
    catch (...)
    {
        LOG_ERROR_MESSAGE("Failed to load render pass '", LoadInfo.Name, "'.");
    }
}

void RenderStateNotationLoaderImpl::LoadShader(const LoadShaderInfo& LoadInfo, IShader** ppShader)
{
    DEV_CHECK_ERR(LoadInfo.Name != nullptr, "LoadInfo.Name  must not be null");
    DEV_CHECK_ERR(ppShader != nullptr, "ppShader must not be null");

    try
    {
        auto Iter = m_ShaderCache.find(LoadInfo.Name);
        if (Iter != m_ShaderCache.end())
        {
            *ppShader = Iter->second;
        }
        else
        {
            const auto* pRSNDesc = m_pParser->GetShaderByName(LoadInfo.Name);
            if (!pRSNDesc)
                LOG_ERROR_AND_THROW("Failed to find shader '", LoadInfo.Name, "'.");

            ShaderCreateInfo ShaderCI           = *pRSNDesc;
            ShaderCI.pShaderSourceStreamFactory = m_pStreamFactory;
            if (LoadInfo.Modify != nullptr)
                LoadInfo.Modify(&ShaderCI, LoadInfo.pUserData);

            RefCntAutoPtr<IShader> pShader;
            m_pDevice->CreateShader(ShaderCI, &pShader);

            if (!pShader)
                LOG_ERROR_AND_THROW("Failed to create shader from file '", ShaderCI.FilePath, "'.");

            if (LoadInfo.AddToCache)
                m_ShaderCache.emplace(HashMapStringKey{ShaderCI.Desc.Name, false}, pShader);

            *ppShader = pShader.Detach();
        }
    }
    catch (...)
    {
        LOG_ERROR_MESSAGE("Failed to load shader '", LoadInfo.Name, "'.");
    }
}

void CreateRenderStateNotationLoader(const RenderStateNotationLoaderCreateInfo& CreateInfo,
                                     IRenderStateNotationLoader**               ppLoader)
{
    try
    {
        RefCntAutoPtr<IRenderStateNotationLoader> pLoader{MakeNewRCObj<RenderStateNotationLoaderImpl>()(CreateInfo)};
        if (pLoader)
            pLoader->QueryInterface(IID_RenderStateNotationLoader, reinterpret_cast<IObject**>(ppLoader));
    }
    catch (...)
    {
        LOG_ERROR("Failed create render state notation loader");
    }
}

} // namespace Diligent

extern "C"
{
    void Diligent_CreateRenderStateNotationLoader(const Diligent::RenderStateNotationLoaderCreateInfo& CreateInfo,
                                                  Diligent::IRenderStateNotationLoader**               ppLoader)
    {
        Diligent::CreateRenderStateNotationLoader(CreateInfo, ppLoader);
    }
}
