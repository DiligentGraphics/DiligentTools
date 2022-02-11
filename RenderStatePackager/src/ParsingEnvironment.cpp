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

#include "ParsingEnvironment.hpp"
#include "DynamicLinearAllocator.hpp"
#include "DefaultRawMemoryAllocator.hpp"
#include "DataBlobImpl.hpp"
#include "FileWrapper.hpp"

#include "json.hpp"
#include "BasicMath.hpp"
#include "generated/CommonParser.hpp"
#include "generated/GraphicsTypesParser.hpp"
#include "generated/ArchiverFactoryParser.hpp"


namespace Diligent
{

IArchiverFactory* ParsingEnvironment::GetArchiveFactory()
{
    return m_pArchiveBuilderFactory;
}

ISerializationDevice* ParsingEnvironment::GetSerializationDevice()
{
    return m_pSerializationDevice;
}

IShaderSourceInputStreamFactory* ParsingEnvironment::GetShaderSourceInputStreamFactory()
{
    return m_pShaderStreamFactory;
}

IShaderSourceInputStreamFactory* ParsingEnvironment::GetParserImportInputStreamFactory()
{
    return m_pRenderStateStreamFactory;
}

RenderStatePackager* ParsingEnvironment::GetDeviceObjectConverter()
{
    return m_pDeviceReflection.get();
}

IThreadPool* ParsingEnvironment::GetThreadPool()
{
    return m_pThreadPool;
}

ParsingEnvironment::ParsingEnvironment(const ParsingEnvironmentCreateInfo& CreateInfo) :
    m_CreateInfo{CreateInfo}
{
}

bool ParsingEnvironment::Initilize()
{
    try
    {
#if EXPLICITLY_LOAD_ARCHIVER_FACTORY_DLL
        auto GetArchiverFactory  = LoadArchiverFactory();
        m_pArchiveBuilderFactory = GetArchiverFactory();
        if (m_pArchiveBuilderFactory == nullptr)
            LOG_ERROR_AND_THROW("Failed to load archive factory");
#else
        m_pArchiveBuilderFactory = Diligent::GetArchiverFactory();
#endif

        DynamicLinearAllocator        Allocator{DefaultRawMemoryAllocator::GetAllocator()};
        SerializationDeviceCreateInfo DeviceCI = {};
        if (!m_CreateInfo.ConfigFilePath.empty())
        {
            FileWrapper File{m_CreateInfo.ConfigFilePath.c_str(), EFileAccessMode::Read};
            if (!File)
                LOG_ERROR_AND_THROW("Failed to open file: '", m_CreateInfo.ConfigFilePath.c_str(), "'.");

            auto pFileData = DataBlobImpl::Create(0);
            File->Read(pFileData);

            String Source{reinterpret_cast<const char*>(pFileData->GetConstDataPtr()), pFileData->GetSize()};

            nlohmann::json Json = nlohmann::json::parse(Source);
            Deserialize(Json, DeviceCI, Allocator);
        }


        auto ConstructString = [](std::vector<std::string> const& Paths) {
            std::stringstream Stream;
            for (auto const& Path : Paths)
                Stream << Path << ";";
            return Stream.str();
        };

        auto ShaderPaths      = ConstructString(m_CreateInfo.ShaderDirs);
        auto RenderStatePaths = ConstructString(m_CreateInfo.RenderStateDirs);

        m_pArchiveBuilderFactory->CreateSerializationDevice(DeviceCI, &m_pSerializationDevice);
        if (!m_pSerializationDevice)
            LOG_ERROR_AND_THROW("Failed to create SerializationDevice");

        m_pArchiveBuilderFactory->CreateDefaultShaderSourceStreamFactory(ShaderPaths.empty() ? nullptr : ShaderPaths.c_str(), &m_pShaderStreamFactory);
        if (!m_pShaderStreamFactory)
            LOG_ERROR_AND_THROW("Failed to create DefaultShaderSourceStreamFactory from paths: '", ShaderPaths, "'.");

        m_pArchiveBuilderFactory->CreateDefaultShaderSourceStreamFactory(RenderStatePaths.empty() ? nullptr : RenderStatePaths.c_str(), &m_pRenderStateStreamFactory);
        if (!m_pRenderStateStreamFactory)
            LOG_ERROR_AND_THROW("Failed to create DefaultShaderSourceStreamFactory from paths: '", RenderStatePaths, "'.");

        Uint32 ThreadCount = m_CreateInfo.ThreadCount > 0 ? m_CreateInfo.ThreadCount : std::thread::hardware_concurrency();

        ThreadPoolCreateInfo ThreadPoolCI{ThreadCount};
        m_pThreadPool = CreateThreadPool(ThreadPoolCI);

        m_pDeviceReflection = std::make_unique<RenderStatePackager>(m_pSerializationDevice, m_pShaderStreamFactory, m_pRenderStateStreamFactory, m_pThreadPool, m_CreateInfo.DeviceFlags, m_CreateInfo.PSOArchiveFlags);

        return true;
    }
    catch (...)
    {
        return false;
    }
}

ParsingEnvironment::~ParsingEnvironment()
{}

} // namespace Diligent
