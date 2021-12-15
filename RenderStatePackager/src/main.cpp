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

enum class ParseStatus
{
    Success,
    SuccessHelp,
    Failed,
};

ParseStatus ParseCommandLine(int argc, char* argv[], ParsingEnvironmentCreateInfo& CreateInfo)
{
    args::ArgumentParser Parser{"DRSN Parser"};
    args::HelpFlag       Help(Parser, "help", "Help menu", {'h', "help"});

    args::ValueFlag<std::string>     ArgumentOutput(Parser, "path", "Output Binary Archive", {'o', "output"}, "Archive.bin");
    args::ValueFlag<std::string>     ArgumentShaderDir(Parser, "dir", "Shader Directory", {'s', "shader_dir"}, "");
    args::ValueFlag<std::string>     ArgumentRenderStateDir(Parser, "dir", "Render State Directory", {'r', "render_state_dir"}, "");
    args::ValueFlag<std::string>     ArgumentShaderConfig(Parser, "path", "Path to Config", {'c', "config"}, "");
    args::ValueFlag<Uint32>          ArgumentThreadCount(Parser, "count", "Count of threads", {'t', "thread"}, 0);
    args::ValueFlagList<std::string> ArgumentInputs(Parser, "path", "Input render state notation files", {'i', "input"}, {}, args::Options::Required);

    args::Group GroupDeviceBits(Parser, "Device Bits:", args::Group::Validators::AtLeastOne);
    args::Flag  ArgumentDeviceBitDx11(GroupDeviceBits, "dx11", "D3D11", {"dx11"});
    args::Flag  ArgumentDeviceBitDx12(GroupDeviceBits, "dx12", "D3D12", {"dx12"});
    args::Flag  ArgumentDeviceBitVulkan(GroupDeviceBits, "vulkan", "Vulkan", {"vulkan"});
    args::Flag  ArgumentDeviceBitOpenGL(GroupDeviceBits, "opengl", "OpenGL", {"opengl"});
    args::Flag  ArgumentDeviceBitOpenGLES(GroupDeviceBits, "opengles", "OpenGLES", {"opengles"});
    args::Flag  ArgumentDeviceBitMetalMacOS(GroupDeviceBits, "metal_macos", "Metal_MacOS", {"metal_macos"});
    args::Flag  ArgumentDeviceBitMetalIOS(GroupDeviceBits, "metal_ios", "Metal_IOS", {"metal_ios"});

    try
    {
        Parser.ParseCLI(argc, argv);
    }
    catch (const args::Help&)
    {
        LOG_INFO_MESSAGE(Parser.Help());
        return ParseStatus::SuccessHelp;
    }
    catch (const args::Error& e)
    {
        LOG_ERROR(e.what());
        LOG_INFO_MESSAGE(Parser.Help());
        return ParseStatus::Failed;
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

    CreateInfo.DeviceBits     = GetDeviceBitsFromParser();
    CreateInfo.ShaderDir      = args::get(ArgumentShaderDir);
    CreateInfo.RenderStateDir = args::get(ArgumentRenderStateDir);
    CreateInfo.ConfigFilePath = args::get(ArgumentShaderConfig);
    CreateInfo.OuputFilePath  = args::get(ArgumentOutput);
    CreateInfo.InputFilePaths = args::get(ArgumentInputs);
    CreateInfo.ThreadCount    = args::get(ArgumentThreadCount);

    return ParseStatus::Success;
}

int main(int argc, char* argv[])
{
    ParsingEnvironmentCreateInfo EnvironmentCI{};

    switch (ParseCommandLine(argc, argv, EnvironmentCI))
    {
        case ParseStatus::Success:
            break;
        case ParseStatus::SuccessHelp:
            return EXIT_SUCCESS;
        case ParseStatus::Failed:
            LOG_FATAL_ERROR("Failed to parse command line");
            return EXIT_FAILURE;
        default:
            UNEXPECTED("Unexpected parse status");
            break;
    }

    auto pEnvironment = std::make_unique<ParsingEnvironment>(EnvironmentCI);
    if (!pEnvironment->Initilize())
    {
        LOG_FATAL_ERROR("Failed to initialize ParsingEnvironment");
        return EXIT_FAILURE;
    }

    auto pArchiveFactory = pEnvironment->GetArchiveFactory();
    auto pConverter      = pEnvironment->GetDeviceObjectConverter();

    RefCntAutoPtr<IArchiver> pArchive;
    pArchiveFactory->CreateArchiver(pEnvironment->GetSerializationDevice(), &pArchive);
    DEV_CHECK_ERR(pArchive != nullptr, "pArchive must not be null");

    auto const& OutputFilePath = EnvironmentCI.OuputFilePath;
    auto const& InputFilePaths = EnvironmentCI.InputFilePaths;

    if (!pConverter->ParseFiles(InputFilePaths))
    {
        LOG_FATAL_ERROR("Failed to parse files");
        return EXIT_FAILURE;
    }

    if (!pConverter->Execute(pArchive))
    {
        LOG_FATAL_ERROR("Failed to archive");
        return EXIT_FAILURE;
    }

    RefCntAutoPtr<IDataBlob> pData;
    if (!pArchive->SerializeToBlob(&pData))
    {
        LOG_FATAL_ERROR("Failed to serialize to Data Blob");
        return EXIT_FAILURE;
    }

    FileWrapper File{OutputFilePath.c_str(), EFileAccessMode::Overwrite};
    if (!File)
    {
        LOG_FATAL_ERROR("Failed to open file: '", OutputFilePath, "'.");
        return EXIT_FAILURE;
    }

    File->Write(pData->GetDataPtr(), pData->GetSize());
}
