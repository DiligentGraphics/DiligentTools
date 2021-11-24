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


#include "pch.h"
#include "argparse.hpp"
#include <fstream>

namespace Diligent
{

EngineEnvironment* EngineEnvironment::m_pEnvironment = nullptr;

IArchiverFactory* EngineEnvironment::GetArchiveFactory()
{
    return m_pArchiveBuilderFactory;
}

ISerializationDevice* EngineEnvironment::GetSerializationDevice()
{
    return m_pSerializationDevice;
}

IShaderSourceInputStreamFactory* EngineEnvironment::GetShaderSourceInputStreamFactory()
{
    return m_pShaderStreamFactory;
}

DeviceObjectReflection* EngineEnvironment::GetDeviceObjectReflection()
{
    return m_pDeviceReflection.get();
}

const EngineEnvironment::EngineEnvironmentDesc& EngineEnvironment::GetDesc() const
{
    return m_Desc;
}

EngineEnvironment::EngineEnvironment(const EngineEnvironmentDesc& Desc) :
    m_Desc(Desc)
{

#if EXPLICITLY_LOAD_ARCHIVER_FACTORY_DLL
    auto GetArchiverFactory = LoadArchiverFactory();
    if (GetArchiverFactory != nullptr)
        m_pArchiveBuilderFactory = GetArchiverFactory();
#else
    m_pArchiveBuilderFactory = Diligent::GetArchiverFactory();
#endif

    SerializationDeviceCreateInfo DeviceCI       = {};
    auto                          pAllocatorHack = std::make_unique<DeviceObjectReflection>(RefCntAutoPtr<ISerializationDevice>(nullptr), RefCntAutoPtr<IShaderSourceInputStreamFactory>(nullptr), RENDER_DEVICE_TYPE_FLAG_NONE);

    if (!Desc.ConfigFilePath.empty())
    {
        std::ifstream  stream(Desc.ConfigFilePath);
        nlohmann::json Json = nlohmann::json::parse(stream);
        Deserialize(Json, DeviceCI, pAllocatorHack.get());
    }

    m_pArchiveBuilderFactory->CreateSerializationDevice(DeviceCI, &m_pSerializationDevice);
    m_pArchiveBuilderFactory->CreateDefaultShaderSourceStreamFactory(m_Desc.ShadersFilePath.c_str(), &m_pShaderStreamFactory);
    m_pDeviceReflection = std::make_unique<DeviceObjectReflection>(m_pSerializationDevice, m_pShaderStreamFactory, m_Desc.DeviceBits);
}

EngineEnvironment::~EngineEnvironment()
{}

void EngineEnvironment::Initialize(int argc, char** argv)
{
    argparse::ArgumentParser Parser{"JSON Archiver"};

    Parser.add_argument("-o", "--output")
        .required()
        .help("Output Binary Archive");

    Parser.add_argument("-s", "--shader_dir")
        .default_value(std::string("."))
        .help("Shaders directory");

    Parser.add_argument("-c", "--config")
        .default_value("")
        .help("Path to config");

    Parser.add_argument("-dx11")
        .default_value(false)
        .implicit_value(true);

    Parser.add_argument("-dx12")
        .default_value(false)
        .implicit_value(true);

    Parser.add_argument("-vulkan")
        .default_value(false)
        .implicit_value(true);

    Parser.add_argument("-opengl")
        .default_value(false)
        .implicit_value(true);

    Parser.add_argument("-metal")
        .default_value(false)
        .implicit_value(true);

    Parser.add_argument("-i", "--input")
        .required()
        .remaining()
        .help("Input Json Archive");

    try
    {
        Parser.parse_args(argc, argv);
    }
    catch (const std::runtime_error& err)
    {
        std::cerr << err.what() << std::endl;
        std::cerr << Parser;
        std::exit(1);
    }

    auto GetDeviceBitsFromParser = [](argparse::ArgumentParser const& Parser) {
        RENDER_DEVICE_TYPE_FLAGS DeviceBits = {};
        if (Parser.get<bool>("-dx11"))
            DeviceBits |= RENDER_DEVICE_TYPE_FLAG_D3D11;
        if (Parser.get<bool>("-dx12"))
            DeviceBits |= RENDER_DEVICE_TYPE_FLAG_D3D12;
        if (Parser.get<bool>("-vulkan"))
            DeviceBits |= RENDER_DEVICE_TYPE_FLAG_VULKAN;
        if (Parser.get<bool>("-metal"))
            DeviceBits |= RENDER_DEVICE_TYPE_FLAG_METAL;
        if (Parser.get<bool>("-opengl"))
            DeviceBits |= RENDER_DEVICE_TYPE_FLAG_GL;
        return DeviceBits;
    };

    auto DeviceBits     = GetDeviceBitsFromParser(Parser);
    auto ShaderFilePath = Parser.get<std::string>("--shader_dir");
    auto ConfigFilePath = Parser.get<std::string>("--config");
    auto OutputFilePath = Parser.get<std::string>("--output");
    auto InputFilePaths = Parser.get<std::vector<std::string>>("--input");


    if (!m_pEnvironment)
    {
        EngineEnvironmentDesc Desc = {};

        Desc.DeviceBits      = DeviceBits;
        Desc.ShadersFilePath = std::move(ShaderFilePath);
        Desc.ConfigFilePath  = std::move(ConfigFilePath);
        Desc.OuputFilePath   = std::move(OutputFilePath);
        Desc.InputFilePaths  = std::move(InputFilePaths);

        m_pEnvironment = new EngineEnvironment{Desc};
        VERIFY_EXPR(m_pEnvironment != nullptr);
    }
}

void EngineEnvironment::Shutdown()
{
    if (m_pEnvironment)
        delete m_pEnvironment;
}

EngineEnvironment* EngineEnvironment::GetInstance()
{
    return m_pEnvironment;
}

} // namespace Diligent
