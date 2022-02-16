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
#include "GraphicsAccessories.hpp"
#include "BasicMath.hpp"
#include "DefaultRawMemoryAllocator.hpp"
#include "DynamicLinearAllocator.hpp"
#include "SerializedPipelineState.h"
#include "FileSystem.hpp"
#include "FileWrapper.hpp"

namespace Diligent
{

struct BytecodeDumper
{
    class WorkingDirectory
    {
    public:
        WorkingDirectory(const Char* Path) :
            m_CurrentPath{Path} {}


        void Push(const Char* Path)
        {
            m_CurrentPath = m_CurrentPath + FileSystem::GetSlashSymbol() + Path;

            if (!FileSystem::PathExists(m_CurrentPath.c_str()))
                FileSystem::CreateDirectory(m_CurrentPath.c_str());
        }

        void Pop()
        {
            m_CurrentPath = m_CurrentPath.substr(0, m_CurrentPath.find_last_of((FileSystem::GetSlashSymbol())));
        }

        String ComputePathFor(const Char* FileName) const
        {
            return m_CurrentPath + FileSystem::GetSlashSymbol() + FileName;
        }

    private:
        String m_CurrentPath;
    };

    class WorkingDirectoryScope
    {
    public:
        WorkingDirectoryScope(WorkingDirectory& Directory, const Char* Path) :
            m_Directory{Directory}
        {
            m_Directory.Push(Path);
        }

        ~WorkingDirectoryScope()
        {
            m_Directory.Pop();
        }

    private:
        WorkingDirectory& m_Directory;
    };


    static bool Execute(const std::vector<RefCntAutoPtr<IPipelineState>>& Pipelines, ARCHIVE_DEVICE_DATA_FLAGS DeviceFlags, const char* Path)
    {
        auto GetFileExtension = [](ARCHIVE_DEVICE_DATA_FLAGS DeviceFlag, SHADER_SOURCE_LANGUAGE Language, bool UseBytecode) //
        {
            if (UseBytecode)
            {
                switch (DeviceFlag)
                {
                    case ARCHIVE_DEVICE_DATA_FLAG_D3D11:
                    case ARCHIVE_DEVICE_DATA_FLAG_D3D12:
                        return ".dxbc";
                    case ARCHIVE_DEVICE_DATA_FLAG_VULKAN:
                        return ".spv";
                    case ARCHIVE_DEVICE_DATA_FLAG_METAL_IOS:
                    case ARCHIVE_DEVICE_DATA_FLAG_METAL_MACOS:
                        return ".air";
                    default:
                        UNEXPECTED("Unexpected device data flag (", static_cast<Uint32>(DeviceFlag), ")");
                        return "";
                }
            }
            else
            {
                switch (Language)
                {
                    case SHADER_SOURCE_LANGUAGE_HLSL:
                        return ".hlsl";
                    case SHADER_SOURCE_LANGUAGE_GLSL:
                    case SHADER_SOURCE_LANGUAGE_GLSL_VERBATIM:
                        return ".glsl";
                    case SHADER_SOURCE_LANGUAGE_MSL:
                        return ".msl";
                    default:
                        UNEXPECTED("Unexpected source language (", static_cast<Uint32>(Language), ")");
                        return "";
                }
            }
        };

        try
        {
            WorkingDirectory RootDirectory{Path};

            WorkingDirectoryScope BytecodeDirectory{RootDirectory, "BytecodeDump"};
            for (auto Flags = DeviceFlags; Flags != ARCHIVE_DEVICE_DATA_FLAG_NONE;)
            {
                const auto            Flag = ExtractLSB(Flags);
                WorkingDirectoryScope DeviceDirectory{RootDirectory, GetArchiveDeviceDataFlagString(Flag)};
                for (auto& pPipeline : Pipelines)
                {
                    auto ShaderStagesIterate = [&RootDirectory, &GetFileExtension](RefCntAutoPtr<IPipelineState> pPSO, ARCHIVE_DEVICE_DATA_FLAGS DeviceFlag) //
                    {
                        WorkingDirectoryScope PipelineDirectory{RootDirectory, pPSO->GetDesc().Name};

                        auto pSerializedPSO = pPSO.Cast<ISerializedPipelineState>(IID_SerializedPipelineState);
                        for (Uint32 ShaderID = 0; ShaderID < pSerializedPSO->GetPatchedShaderCount(DeviceFlag); ShaderID++)
                        {
                            const auto ShaderCI = pSerializedPSO->GetPatchedShaderCreateInfo(DeviceFlag, ShaderID);

                            const auto UseBytecode = ShaderCI.ByteCode != nullptr ? true : false;

                            const auto ShaderPath = RootDirectory.ComputePathFor(ShaderCI.Desc.Name) + GetFileExtension(DeviceFlag, ShaderCI.SourceLanguage, UseBytecode);

                            FileWrapper File{ShaderPath.c_str(), EFileAccessMode::Overwrite};
                            if (!File)
                                LOG_FATAL_ERROR_AND_THROW("Failed to open file: '", ShaderPath, "'.");

                            if (UseBytecode)
                                File->Write(ShaderCI.ByteCode, ShaderCI.ByteCodeSize);
                            else
                                File->Write(ShaderCI.Source, ShaderCI.SourceLength);
                        }
                    };

                    const auto PipelineType = pPipeline->GetDesc().PipelineType;

                    WorkingDirectoryScope PipelineTypeDirectory{RootDirectory, GetPipelineTypeString(PipelineType)};
                    ShaderStagesIterate(pPipeline, Flag);
                }
            }
            return true;
        }
        catch (...)
        {
            return false;
        }
    }
};

RenderStatePackager::RenderStatePackager(RefCntAutoPtr<ISerializationDevice>            pDevice,
                                         RefCntAutoPtr<IShaderSourceInputStreamFactory> pShaderStreamFactory,
                                         RefCntAutoPtr<IShaderSourceInputStreamFactory> pRenderStateStreamFactory,
                                         RefCntAutoPtr<IThreadPool>                     pThreadPool,
                                         ARCHIVE_DEVICE_DATA_FLAGS                      DeviceFlags,
                                         PSO_ARCHIVE_FLAGS                              PSOArchiveFlags) :
    m_pDevice{pDevice},
    m_pShaderStreamFactory{pShaderStreamFactory},
    m_pRenderStateStreamFactory{pRenderStateStreamFactory},
    m_pThreadPool{pThreadPool},
    m_DeviceFlags{DeviceFlags},
    m_PSOArchiveFlags{PSOArchiveFlags}
{
}

bool RenderStatePackager::ParseFiles(std::vector<std::string> const& DRSNPaths)
{
    DEV_CHECK_ERR(!DRSNPaths.empty(), "DRSNPaths must not be empty");
    CreateRenderStateNotationParser({}, &m_pRSNParser);

    for (auto const& Path : DRSNPaths)
        if (!m_pRSNParser->ParseFile(Path.c_str(), m_pRenderStateStreamFactory))
            return false;
    return true;
}

bool RenderStatePackager::Execute(RefCntAutoPtr<IArchiver> pArchive, const char* DumpPath)
{
    DEV_CHECK_ERR(pArchive != nullptr, "pArchive must not be null");

    if (pArchive == nullptr)
        return false;

    try
    {
        auto const& ParserInfo = m_pRSNParser->GetInfo();

        std::vector<RefCntAutoPtr<IShader>>                    Shaders(ParserInfo.ShaderCount);
        std::vector<RefCntAutoPtr<IRenderPass>>                RenderPasses(ParserInfo.RenderPassCount);
        std::vector<RefCntAutoPtr<IPipelineResourceSignature>> ResourceSignatures(ParserInfo.ResourceSignatureCount);
        std::vector<RefCntAutoPtr<IPipelineState>>             Pipelines(ParserInfo.PipelineStateCount);

        std::atomic<bool> Result{true};

        for (Uint32 ShaderID = 0; ShaderID < ParserInfo.ShaderCount; ++ShaderID)
        {
            EnqueueAsyncWork(m_pThreadPool, [ShaderID, this, &Result, &Shaders](Uint32 ThreadId) {
                ShaderCreateInfo ShaderCI           = *m_pRSNParser->GetShaderByIndex(ShaderID);
                ShaderCI.pShaderSourceStreamFactory = m_pShaderStreamFactory;

                auto& pShader = Shaders[ShaderID];
                m_pDevice->CreateShader(ShaderCI, ShaderArchiveInfo{m_DeviceFlags}, &pShader);
                if (!pShader)
                {
                    LOG_ERROR_MESSAGE("Failed to create shader from file '", ShaderCI.FilePath, "'.");
                    Result.store(false);
                }
            });
        }

        for (Uint32 RenderPassID = 0; RenderPassID < ParserInfo.RenderPassCount; ++RenderPassID)
        {
            EnqueueAsyncWork(m_pThreadPool, [RenderPassID, this, &Result, &RenderPasses](Uint32 ThreadId) {
                auto  RPDesc      = *m_pRSNParser->GetRenderPassByIndex(RenderPassID);
                auto& pRenderPass = RenderPasses[RenderPassID];
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
            EnqueueAsyncWork(m_pThreadPool, [&, SignatureID](Uint32 ThreadId) {
                auto  SignDesc   = *m_pRSNParser->GetResourceSignatureByIndex(SignatureID);
                auto& pSignature = ResourceSignatures[SignatureID];
                m_pDevice->CreatePipelineResourceSignature(SignDesc, {m_DeviceFlags}, &pSignature);
                if (!pSignature)
                {
                    LOG_ERROR_MESSAGE("Failed to create resource signature '", SignDesc.Name, "'.");
                    Result.store(false);
                }
            });
        }

        m_pThreadPool->WaitForAllTasks();

        if (!Result.load())
            LOG_ERROR_AND_THROW("Failed to create state objects");

        for (auto& pResource : Shaders)
            m_Shaders.emplace(HashMapStringKey{pResource->GetDesc().Name, false}, pResource);

        for (auto& pResource : RenderPasses)
            m_RenderPasses.emplace(HashMapStringKey{pResource->GetDesc().Name, false}, pResource);

        for (auto& pResource : ResourceSignatures)
            m_ResourceSignatures.emplace(HashMapStringKey{pResource->GetDesc().Name, false}, pResource);

        auto FindShader = [&](const char* Name) -> IShader* //
        {
            if (Name == nullptr)
                return nullptr;

            auto Iter = m_Shaders.find(Name);
            if (Iter == m_Shaders.end())
            {
                LOG_ERROR_AND_THROW("Unable to find shader '", Name, "'.");
            }
            return Iter->second;
        };

        auto FindRenderPass = [&](const char* Name) -> IRenderPass* //
        {
            if (Name == nullptr)
                return nullptr;

            auto Iter = m_RenderPasses.find(Name);
            if (Iter == m_RenderPasses.end())
            {
                LOG_ERROR_AND_THROW("Unable to find render pass '", Name, "'.");
            }
            return Iter->second;
        };

        auto FindResourceSignature = [&](const char* Name) -> IPipelineResourceSignature* //
        {
            if (Name == nullptr)
                return nullptr;

            auto Iter = m_ResourceSignatures.find(Name);
            if (Iter == m_ResourceSignatures.end())
            {
                LOG_ERROR_AND_THROW("Unable to find resource signature '", Name, "'.");
            }
            return Iter->second;
        };

        auto UnpackPipelineStateCreateInfo = [&](DynamicLinearAllocator& Allocator, PipelineStateNotation const& DescRSN, PipelineStateCreateInfo& PipelineCI) //
        {
            PipelineCI.PSODesc                 = DescRSN.PSODesc;
            PipelineCI.Flags                   = DescRSN.Flags;
            PipelineCI.ResourceSignaturesCount = DescRSN.ResourceSignaturesNameCount;
            PipelineCI.ppResourceSignatures    = Allocator.ConstructArray<IPipelineResourceSignature*>(DescRSN.ResourceSignaturesNameCount);
            for (Uint32 SignatureID = 0; SignatureID < PipelineCI.ResourceSignaturesCount; ++SignatureID)
                PipelineCI.ppResourceSignatures[SignatureID] = FindResourceSignature(DescRSN.ppResourceSignatureNames[SignatureID]);
        };

        for (Uint32 PipelineID = 0; PipelineID < ParserInfo.PipelineStateCount; ++PipelineID)
        {
            EnqueueAsyncWork(m_pThreadPool, [&, PipelineID](Uint32 ThreadId) {
                try
                {
                    DynamicLinearAllocator Allocator{DefaultRawMemoryAllocator::GetAllocator()};

                    const PipelineStateArchiveInfo ArchiveInfo{m_PSOArchiveFlags, m_DeviceFlags};

                    auto pDescRSN = m_pRSNParser->GetPipelineStateByIndex(PipelineID);

                    auto& pPipeline = Pipelines[PipelineID];

                    const auto PipelineType = pDescRSN->PSODesc.PipelineType;
                    switch (PipelineType)
                    {
                        case PIPELINE_TYPE_GRAPHICS:
                        case PIPELINE_TYPE_MESH:
                        {
                            const auto* pPipelineDescRSN = static_cast<const GraphicsPipelineNotation*>(pDescRSN);
                            VERIFY_EXPR(pPipelineDescRSN != nullptr);

                            GraphicsPipelineStateCreateInfo PipelineCI{};
                            UnpackPipelineStateCreateInfo(Allocator, *pPipelineDescRSN, PipelineCI);
                            PipelineCI.GraphicsPipeline             = static_cast<GraphicsPipelineDesc>(pPipelineDescRSN->Desc);
                            PipelineCI.GraphicsPipeline.pRenderPass = FindRenderPass(pPipelineDescRSN->pRenderPassName);

                            PipelineCI.pVS = FindShader(pPipelineDescRSN->pVSName);
                            PipelineCI.pPS = FindShader(pPipelineDescRSN->pPSName);
                            PipelineCI.pDS = FindShader(pPipelineDescRSN->pDSName);
                            PipelineCI.pHS = FindShader(pPipelineDescRSN->pHSName);
                            PipelineCI.pGS = FindShader(pPipelineDescRSN->pGSName);
                            PipelineCI.pAS = FindShader(pPipelineDescRSN->pASName);
                            PipelineCI.pMS = FindShader(pPipelineDescRSN->pMSName);

                            m_pDevice->CreateGraphicsPipelineState(PipelineCI, ArchiveInfo, &pPipeline);
                            break;
                        }
                        case PIPELINE_TYPE_COMPUTE:
                        {
                            const auto* pPipelineDescRSN = static_cast<const ComputePipelineNotation*>(pDescRSN);

                            ComputePipelineStateCreateInfo PipelineCI{};
                            UnpackPipelineStateCreateInfo(Allocator, *pPipelineDescRSN, PipelineCI);
                            PipelineCI.pCS = FindShader(pPipelineDescRSN->pCSName);

                            m_pDevice->CreateComputePipelineState(PipelineCI, ArchiveInfo, &pPipeline);
                            break;
                        }
                        case PIPELINE_TYPE_TILE:
                        {
                            const auto* pPipelineDescRSN = static_cast<const TilePipelineNotation*>(pDescRSN);

                            TilePipelineStateCreateInfo PipelineCI{};
                            UnpackPipelineStateCreateInfo(Allocator, *pPipelineDescRSN, PipelineCI);
                            PipelineCI.pTS = FindShader(pPipelineDescRSN->pTSName);

                            m_pDevice->CreateTilePipelineState(PipelineCI, ArchiveInfo, &pPipeline);
                            break;
                        }
                        case PIPELINE_TYPE_RAY_TRACING:
                        {
                            const auto* pPipelineDescRSN = static_cast<const RayTracingPipelineNotation*>(pDescRSN);

                            RayTracingPipelineStateCreateInfo PipelineCI{};
                            UnpackPipelineStateCreateInfo(Allocator, *pPipelineDescRSN, PipelineCI);
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
                                auto* pData = Allocator.ConstructArray<RayTracingTriangleHitShaderGroup>(pPipelineDescRSN->TriangleHitShaderCount);

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
                                auto* pData = Allocator.ConstructArray<RayTracingProceduralHitShaderGroup>(pPipelineDescRSN->ProceduralHitShaderCount);

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

                            m_pDevice->CreateRayTracingPipelineState(PipelineCI, ArchiveInfo, &pPipeline);
                            break;
                        }
                        default:
                            break;
                    }

                    if (!pPipeline)
                        LOG_ERROR_AND_THROW("Failed to create pipeline '", pDescRSN->PSODesc.Name, "'.");
                }
                catch (...)
                {
                    Result.store(false);
                }
            });
        }

        m_pThreadPool->WaitForAllTasks();
        if (!Result.load())
            LOG_ERROR_AND_THROW("Failed to create state objects");

        for (auto& pSignature : ResourceSignatures)
            if (!pArchive->AddPipelineResourceSignature(pSignature))
                LOG_ERROR_AND_THROW("Failed to archive resource signature '", pSignature->GetDesc().Name, "'.");

        for (auto& pPipeline : Pipelines)
            if (!pArchive->AddPipelineState(pPipeline))
                LOG_ERROR_AND_THROW("Failed to archive pipeline '", pPipeline->GetDesc().Name, "'.");

        if (DumpPath != nullptr && !BytecodeDumper::Execute(Pipelines, m_DeviceFlags, DumpPath))
            LOG_ERROR_AND_THROW("Failed to dump the bytecode");
    }
    catch (...)
    {
        return false;
    }
    return true;
}

void RenderStatePackager::Reset()
{
    m_pRSNParser.Release();
    m_RenderPasses.clear();
    m_Shaders.clear();
    m_ResourceSignatures.clear();
}

} // namespace Diligent
