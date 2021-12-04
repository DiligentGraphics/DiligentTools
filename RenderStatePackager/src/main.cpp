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

#include "FileWrapper.hpp"
#include "RenderStateNotationParser.h"
#include "ParsingEnvironment.hpp"
#include "args.hxx"

using namespace Diligent;

ParsingEnvironmentCreateInfo ParseCommandLine(int argc, char* argv[])
{
    args::ArgumentParser Parser{"JSON Parser"};

    args::HelpFlag Help(Parser, "help", "Help menu", {'h', "help"});

    args::ValueFlag<std::string> ArgumentOutput(Parser, "output", "Output Binary Archive", {'o'}, "Archive.bin");
    args::ValueFlag<std::string> ArgumentShaderDir(Parser, "shader_dir", "Shader Directory", {'s'}, ".");
    args::ValueFlag<std::string> ArgumentShaderConfig(Parser, "config", "Path to Config", {'c'}, "");

    args::Group GroupDeviceBits(Parser, "Device Bits:", args::Group::Validators::DontCare);

    args::Flag ArgumentDeviceBitDx11(GroupDeviceBits, "dx11", "D3D11", {"dx11"});
    args::Flag ArgumentDeviceBitDx12(GroupDeviceBits, "dx12", "D3D12", {"dx12"});
    args::Flag ArgumentDeviceBitVulkan(GroupDeviceBits, "vulkan", "Vulkan", {"vulkan"});
    args::Flag ArgumentDeviceBitOpenGL(GroupDeviceBits, "opengl", "OpenGL", {"opengl"});
    args::Flag ArgumentDeviceBitOpenGLES(GroupDeviceBits, "opengles", "OpenGLES", {"opengles"});
    args::Flag ArgumentDeviceBitMetalMacOS(GroupDeviceBits, "metal_macos", "Metal_MacOS", {"metal_macos"});
    args::Flag ArgumentDeviceBitMetalIOS(GroupDeviceBits, "metal_ios", "Metal_IOS", {"metal_ios"});

    args::ValueFlagList<std::string> ArgumentInputs(Parser, "input", "Input Json-s", {'i'}, {}, args::Options::Required);

    try
    {
        Parser.ParseCLI(argc, argv);
    }
    catch (const args::ValidationError& e)
    {
        LOG_ERROR_MESSAGE(e.what());
        LOG_ERROR_MESSAGE(Parser.Help());
        std::exit(1);
    }
    catch (const args::Help& e)
    {
        LOG_ERROR_MESSAGE(e.what());
        std::exit(0);
    }

    auto GetDeviceBitsFromParser = [&]() {
        ARCHIVE_DEVICE_DATA_FLAGS DeviceBits = {};
        if (ArgumentDeviceBitDx11)
            DeviceBits |= ARCHIVE_DEVICE_DATA_FLAG_D3D11;
        if (ArgumentDeviceBitDx12)
            DeviceBits |= ARCHIVE_DEVICE_DATA_FLAG_D3D12;
        if (ArgumentDeviceBitVulkan)
            DeviceBits |= ARCHIVE_DEVICE_DATA_FLAG_VULKAN;
        if (ArgumentDeviceBitOpenGL)
            DeviceBits |= ARCHIVE_DEVICE_DATA_FLAG_GL;
        if (ArgumentDeviceBitOpenGLES)
            DeviceBits |= ARCHIVE_DEVICE_DATA_FLAG_GLES;
        if (ArgumentDeviceBitMetalMacOS)
            DeviceBits |= ARCHIVE_DEVICE_DATA_FLAG_METAL_MACOS;
        if (ArgumentDeviceBitMetalIOS)
            DeviceBits |= ARCHIVE_DEVICE_DATA_FLAG_METAL_IOS;
        return DeviceBits;
    };

    auto ShaderFilePath = args::get(ArgumentShaderDir);

    ParsingEnvironmentCreateInfo CreateInfo = {};

    CreateInfo.DeviceBits      = GetDeviceBitsFromParser();
    CreateInfo.ShadersFilePath = args::get(ArgumentShaderDir);
    CreateInfo.ConfigFilePath  = args::get(ArgumentShaderConfig);
    CreateInfo.OuputFilePath   = args::get(ArgumentOutput);
    CreateInfo.InputFilePaths  = args::get(ArgumentInputs);

    return CreateInfo;
}

int main(int argc, char* argv[])
{
    auto EnvironmentCI = ParseCommandLine(argc, argv);
    auto pEnvironment  = std::make_unique<ParsingEnvironment>(EnvironmentCI);

    auto pArchiveFactory = pEnvironment->GetArchiveFactory();
    auto pConverter      = pEnvironment->GetDeviceObjectConverter();

    RefCntAutoPtr<IArchiver> pArchive;
    pArchiveFactory->CreateArchiver(pEnvironment->GetSerializationDevice(), &pArchive);

    auto const& OutputFilePath = EnvironmentCI.OuputFilePath;
    auto const& InputFilePaths = EnvironmentCI.InputFilePaths;

    for (auto const& Path : InputFilePaths)
    {
        RefCntAutoPtr<IRenderStateNotationParser> pDescriptorParser;
        CreateRenderStateNotationParserFromFile(Path.c_str(), &pDescriptorParser);
        pConverter->Execute(pDescriptorParser, pArchive);
    }

    FileWrapper File{OutputFilePath.c_str(), EFileAccessMode::Overwrite};
    if (!File)
        LOG_ERROR_AND_THROW("Failed to open file '", OutputFilePath, "'.");

    RefCntAutoPtr<IDataBlob> pData;
    pArchive->SerializeToBlob(&pData);
    File->Write(pData->GetDataPtr(), pData->GetSize());
}
