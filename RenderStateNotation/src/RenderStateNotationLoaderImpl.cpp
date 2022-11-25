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
#include "CallbackWrapper.hpp"
#include "DynamicLinearAllocator.hpp"

namespace Diligent
{

RenderStateNotationLoaderImpl::RenderStateNotationLoaderImpl(IReferenceCounters* pRefCounters, const RenderStateNotationLoaderCreateInfo& CreateInfo) :
    TBase{pRefCounters},
    m_DeviceWithCache{CreateInfo.pDevice, CreateInfo.pStateCache},
    m_pParser{CreateInfo.pParser},
    m_pStreamFactory{CreateInfo.pStreamFactory}
{
    VERIFY_EXPR(CreateInfo.pDevice != nullptr && CreateInfo.pParser != nullptr);
}

void RenderStateNotationLoaderImpl::LoadPipelineState(const LoadPipelineStateInfo& LoadInfo, IPipelineState** ppPSO)
{
    DEV_CHECK_ERR(LoadInfo.Name != nullptr, "LoadInfo.Name  must not be null");
    DEV_CHECK_ERR(ppPSO != nullptr, "ppPSO must not be null");
    DEV_CHECK_ERR(*ppPSO == nullptr, "*ppPSO is not null. Make sure you are not overwriting reference to an existing object as this may result in memory leaks.");

    try
    {
        auto FindPipeline = [this](const Char* Name, PIPELINE_TYPE PipelineType) -> RefCntAutoPtr<IPipelineState> //
        {
            const auto Iter = m_PipelineStateCache.find(std::make_pair(HashMapStringKey{Name, true}, PipelineType));
            if (Iter != m_PipelineStateCache.end())
                return Iter->second;
            return {};
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

            auto FindShader = [&](const char* Name, SHADER_TYPE ShaderType) -> IShader* //
            {
                if (Name == nullptr)
                    return nullptr;

                auto AddToCache = LoadInfo.AddToCache;
                auto Callback   = MakeCallback([&](ShaderCreateInfo& ShaderCI) {
                    if (LoadInfo.ModifyShader != nullptr)
                        LoadInfo.ModifyShader(ShaderCI, ShaderType, AddToCache, LoadInfo.pModifyShaderData);
                });

                RefCntAutoPtr<IShader> pShader;
                LoadShader({Name, false, Callback, Callback}, &pShader);

                if (!pShader)
                    LOG_ERROR_AND_THROW("Failed to load shader '", Name, "' for pipeline '", LoadInfo.Name, "'.");

                if (AddToCache)
                    m_ShaderCache.emplace(HashMapStringKey{pShader->GetDesc().Name, false}, pShader);
                else
                    PipelineShaders.push_back(pShader);

                return pShader;
            };

            auto FindRenderPass = [&](const char* Name) -> IRenderPass* //
            {
                if (Name == nullptr)
                    return nullptr;

                auto AddToCache = LoadInfo.AddToCache;
                auto Callback   = MakeCallback([&](RenderPassDesc& RenderPassCI) {
                    if (LoadInfo.ModifyRenderPass != nullptr)
                        LoadInfo.ModifyRenderPass(RenderPassCI, AddToCache, LoadInfo.pModifyRenderPassData);
                });

                VERIFY_EXPR(!pRenderPass);
                LoadRenderPass({Name, false, Callback, Callback}, &pRenderPass);

                if (!pRenderPass)
                    LOG_ERROR_AND_THROW("Failed to load render pass '", Name, "' for pipeline '", LoadInfo.Name, "'.");

                if (AddToCache)
                    m_RenderPassCache.emplace(HashMapStringKey{pRenderPass->GetDesc().Name, false}, pRenderPass);

                return pRenderPass;
            };

            auto FindResourceSignature = [&](const char* Name) -> IPipelineResourceSignature* //
            {
                if (Name == nullptr)
                    return nullptr;

                auto AddToCache = LoadInfo.AddToCache;
                auto Callback   = MakeCallback([&](PipelineResourceSignatureDesc& ResourceSignatureCI) {
                    if (LoadInfo.ModifyResourceSignature != nullptr)
                        LoadInfo.ModifyResourceSignature(ResourceSignatureCI, AddToCache, LoadInfo.pModifyResourceSignatureData);
                });

                RefCntAutoPtr<IPipelineResourceSignature> pResourceSignature;
                LoadResourceSignature({Name, false, Callback, Callback}, &pResourceSignature);

                if (!pResourceSignature)
                    LOG_ERROR_AND_THROW("Failed to load resource signature '", Name, "' for pipeline '", LoadInfo.Name, "'.");

                if (AddToCache)
                    m_ResourceSignatureCache.emplace(HashMapStringKey{pResourceSignature->GetDesc().Name, false}, pResourceSignature);
                else
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

            static_assert(PIPELINE_TYPE_LAST == 4, "Please handle the new pipeline type below.");
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

                    PipelineCI.pVS = FindShader(pPipelineDescRSN->pVSName, SHADER_TYPE_VERTEX);
                    PipelineCI.pPS = FindShader(pPipelineDescRSN->pPSName, SHADER_TYPE_PIXEL);
                    PipelineCI.pDS = FindShader(pPipelineDescRSN->pDSName, SHADER_TYPE_DOMAIN);
                    PipelineCI.pHS = FindShader(pPipelineDescRSN->pHSName, SHADER_TYPE_HULL);
                    PipelineCI.pGS = FindShader(pPipelineDescRSN->pGSName, SHADER_TYPE_GEOMETRY);
                    PipelineCI.pAS = FindShader(pPipelineDescRSN->pASName, SHADER_TYPE_AMPLIFICATION);
                    PipelineCI.pMS = FindShader(pPipelineDescRSN->pMSName, SHADER_TYPE_MESH);

                    if (LoadInfo.ModifyPipeline != nullptr)
                        LoadInfo.ModifyPipeline(PipelineCI, LoadInfo.pModifyPipelineData);

                    pPipeline = m_DeviceWithCache.CreateGraphicsPipelineState(PipelineCI);
                    break;
                }
                case PIPELINE_TYPE_COMPUTE:
                {
                    const auto* pPipelineDescRSN = static_cast<const ComputePipelineNotation*>(pDescRSN);

                    ComputePipelineStateCreateInfo PipelineCI{};
                    UnpackPipelineStateCreateInfo(*pPipelineDescRSN, PipelineCI);
                    PipelineCI.pCS = FindShader(pPipelineDescRSN->pCSName, SHADER_TYPE_COMPUTE);

                    if (LoadInfo.ModifyPipeline != nullptr)
                        LoadInfo.ModifyPipeline(PipelineCI, LoadInfo.pModifyPipelineData);

                    pPipeline = m_DeviceWithCache.CreateComputePipelineState(PipelineCI);
                    break;
                }
                case PIPELINE_TYPE_TILE:
                {
                    const auto* pPipelineDescRSN = static_cast<const TilePipelineNotation*>(pDescRSN);

                    TilePipelineStateCreateInfo PipelineCI{};
                    UnpackPipelineStateCreateInfo(*pPipelineDescRSN, PipelineCI);
                    PipelineCI.pTS = FindShader(pPipelineDescRSN->pTSName, SHADER_TYPE_TILE);

                    if (LoadInfo.ModifyPipeline != nullptr)
                        LoadInfo.ModifyPipeline(PipelineCI, LoadInfo.pModifyPipelineData);

                    pPipeline = m_DeviceWithCache.CreateTilePipelineState(PipelineCI);
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
                            pData[ShaderID].pShader = FindShader(pPipelineDescRSN->pGeneralShaders[ShaderID].pShaderName, SHADER_TYPE_RAY_GEN);
                        }

                        PipelineCI.pGeneralShaders    = pData;
                        PipelineCI.GeneralShaderCount = pPipelineDescRSN->GeneralShaderCount;
                    }

                    {
                        auto pData = Allocator.ConstructArray<RayTracingTriangleHitShaderGroup>(pPipelineDescRSN->TriangleHitShaderCount);

                        for (Uint32 ShaderID = 0; ShaderID < pPipelineDescRSN->TriangleHitShaderCount; ++ShaderID)
                        {
                            pData[ShaderID].Name              = pPipelineDescRSN->pTriangleHitShaders[ShaderID].Name;
                            pData[ShaderID].pAnyHitShader     = FindShader(pPipelineDescRSN->pTriangleHitShaders[ShaderID].pAnyHitShaderName, SHADER_TYPE_RAY_ANY_HIT);
                            pData[ShaderID].pClosestHitShader = FindShader(pPipelineDescRSN->pTriangleHitShaders[ShaderID].pClosestHitShaderName, SHADER_TYPE_RAY_CLOSEST_HIT);
                        }

                        PipelineCI.pTriangleHitShaders    = pData;
                        PipelineCI.TriangleHitShaderCount = pPipelineDescRSN->TriangleHitShaderCount;
                    }

                    {
                        auto pData = Allocator.ConstructArray<RayTracingProceduralHitShaderGroup>(pPipelineDescRSN->ProceduralHitShaderCount);

                        for (Uint32 ShaderID = 0; ShaderID < pPipelineDescRSN->ProceduralHitShaderCount; ++ShaderID)
                        {
                            pData[ShaderID].Name                = pPipelineDescRSN->pProceduralHitShaders[ShaderID].Name;
                            pData[ShaderID].pAnyHitShader       = FindShader(pPipelineDescRSN->pProceduralHitShaders[ShaderID].pAnyHitShaderName, SHADER_TYPE_RAY_ANY_HIT);
                            pData[ShaderID].pIntersectionShader = FindShader(pPipelineDescRSN->pProceduralHitShaders[ShaderID].pIntersectionShaderName, SHADER_TYPE_RAY_INTERSECTION);
                            pData[ShaderID].pClosestHitShader   = FindShader(pPipelineDescRSN->pProceduralHitShaders[ShaderID].pClosestHitShaderName, SHADER_TYPE_RAY_CLOSEST_HIT);
                        }

                        PipelineCI.pProceduralHitShaders    = pData;
                        PipelineCI.ProceduralHitShaderCount = pPipelineDescRSN->ProceduralHitShaderCount;
                    }

                    if (LoadInfo.ModifyPipeline != nullptr)
                        LoadInfo.ModifyPipeline(PipelineCI, LoadInfo.pModifyPipelineData);

                    pPipeline = m_DeviceWithCache.CreateRayTracingPipelineState(PipelineCI);
                    break;
                }
                default:
                    UNEXPECTED("Unexpected pipeline type");
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
    DEV_CHECK_ERR(*ppSignature == nullptr, "*ppSignature is not null. Make sure you are not overwriting reference to an existing object as this may result in memory leaks.");

    try
    {
        auto Iter = m_ResourceSignatureCache.find(LoadInfo.Name);
        if (Iter != m_ResourceSignatureCache.end())
        {
            *ppSignature = Iter->second;
            (*ppSignature)->AddRef();
        }
        else
        {
            const auto* pRSNDesc = m_pParser->GetResourceSignatureByName(LoadInfo.Name);
            if (!pRSNDesc)
                LOG_ERROR_AND_THROW("Failed to find resource signature '", LoadInfo.Name, "'.");

            PipelineResourceSignatureDesc RSDesc = *pRSNDesc;
            if (LoadInfo.Modify != nullptr)
                LoadInfo.Modify(RSDesc, LoadInfo.pUserData);

            auto pSignature = m_DeviceWithCache.CreatePipelineResourceSignature(RSDesc);

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
    DEV_CHECK_ERR(*ppRenderPass == nullptr, "*ppRenderPass is not null. Make sure you are not overwriting reference to an existing object as this may result in memory leaks.");

    try
    {
        auto Iter = m_RenderPassCache.find(LoadInfo.Name);
        if (Iter != m_RenderPassCache.end())
        {
            *ppRenderPass = Iter->second;
            (*ppRenderPass)->AddRef();
        }
        else
        {
            const auto* pRSNDesc = m_pParser->GetRenderPassByName(LoadInfo.Name);
            if (!pRSNDesc)
                LOG_ERROR_AND_THROW("Failed to find render pass '", LoadInfo.Name, "'.");

            RenderPassDesc RPDesc = *pRSNDesc;
            if (LoadInfo.Modify != nullptr)
                LoadInfo.Modify(RPDesc, LoadInfo.pUserData);

            auto pRenderPass = m_DeviceWithCache.CreateRenderPass(RPDesc);

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
    DEV_CHECK_ERR(*ppShader == nullptr, "*ppShader is not null. Make sure you are not overwriting reference to an existing object as this may result in memory leaks.");

    try
    {
        auto Iter = m_ShaderCache.find(LoadInfo.Name);
        if (Iter != m_ShaderCache.end())
        {
            *ppShader = Iter->second;
            (*ppShader)->AddRef();
        }
        else
        {
            const auto* pRSNDesc = m_pParser->GetShaderByName(LoadInfo.Name);
            if (!pRSNDesc)
                LOG_ERROR_AND_THROW("Failed to find shader '", LoadInfo.Name, "'.");

            ShaderCreateInfo ShaderCI           = *pRSNDesc;
            ShaderCI.pShaderSourceStreamFactory = m_pStreamFactory;
            if (LoadInfo.Modify != nullptr)
                LoadInfo.Modify(ShaderCI, LoadInfo.pUserData);

            auto pShader = m_DeviceWithCache.CreateShader(ShaderCI);

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

bool RenderStateNotationLoaderImpl::Reload()
{
    if (!m_pParser->Reload())
        return false;
    if (auto* pCache = m_DeviceWithCache.GetCache())
    {
        auto Callback = MakeCallback(
            [this](const char* PipelineName, GraphicsPipelineDesc& GraphicsDesc) {
                const auto* pPsoNotation = m_pParser->GetPipelineStateByName(PipelineName);
                if (pPsoNotation == nullptr)
                {
                    LOG_WARNING_MESSAGE("Unable to find pipeline state '", PipelineName, "' after reloading states.");
                    return;
                }

                VERIFY_EXPR(pPsoNotation->PSODesc.PipelineType == PIPELINE_TYPE_GRAPHICS || pPsoNotation->PSODesc.PipelineType == PIPELINE_TYPE_MESH);
                const auto* pGraphicsNotation = static_cast<const GraphicsPipelineNotation*>(pPsoNotation);

                GraphicsDesc = pGraphicsNotation->Desc;
            });
        pCache->Reload(Callback, Callback);
    }
    return true;
}

void CreateRenderStateNotationLoader(const RenderStateNotationLoaderCreateInfo& CreateInfo,
                                     IRenderStateNotationLoader**               ppLoader)
{
    DEV_CHECK_ERR(ppLoader != nullptr, "ppLoader must not be null");
    DEV_CHECK_ERR(*ppLoader == nullptr, "*ppLoader is not null. Make sure you are not overwriting reference to an existing object as this may result in memory leaks.");

    try
    {
        RefCntAutoPtr<IRenderStateNotationLoader> pLoader{MakeNewRCObj<RenderStateNotationLoaderImpl>()(CreateInfo)};
        if (pLoader)
            pLoader->QueryInterface(IID_RenderStateNotationLoader, reinterpret_cast<IObject**>(ppLoader));
    }
    catch (...)
    {
        LOG_ERROR("Failed to create render state notation loader");
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
