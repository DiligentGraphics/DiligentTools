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

#include <memory>
#include <vector>
#include <string>

#include "gtest/gtest.h"
#include "RenderStatePackager.hpp"
#include "ParsingEnvironment.hpp"
#include "TestingEnvironment.hpp"
#include "FileSystem.hpp"
#include "BasicMath.hpp"
#include "GraphicsAccessories.hpp"

using namespace Diligent;
using namespace Diligent::Testing;

namespace
{

static constexpr Uint32 ContentVersion = 135;

static constexpr ARCHIVE_DEVICE_DATA_FLAGS GetDeviceFlags()
{
    ARCHIVE_DEVICE_DATA_FLAGS DeviceFlags = ARCHIVE_DEVICE_DATA_FLAG_NONE;
#if D3D11_SUPPORTED
    DeviceFlags = DeviceFlags | ARCHIVE_DEVICE_DATA_FLAG_D3D11;
#endif
#if D3D12_SUPPORTED
    DeviceFlags = DeviceFlags | ARCHIVE_DEVICE_DATA_FLAG_D3D12;
#endif
#if VULKAN_SUPPORTED
    DeviceFlags = DeviceFlags | ARCHIVE_DEVICE_DATA_FLAG_VULKAN;
#endif
#if GL_SUPPORTED
    DeviceFlags = DeviceFlags | ARCHIVE_DEVICE_DATA_FLAG_GL;
    DeviceFlags = DeviceFlags | ARCHIVE_DEVICE_DATA_FLAG_GLES;
#endif
#if METAL_SUPPORTED
    DeviceFlags = DeviceFlags | ARCHIVE_DEVICE_DATA_FLAG_METAL_MACOS;
    DeviceFlags = DeviceFlags | ARCHIVE_DEVICE_DATA_FLAG_METAL_IOS;
#endif
    return DeviceFlags;
}

TEST(Tools_RenderStatePackager, BasicTest)
{
    ParsingEnvironmentCreateInfo EnvironmentCI{};
    EnvironmentCI.DeviceFlags = GetDeviceFlags();
#if PLATFORM_MACOS
    // Compute shader are not supported in OpenGL on MacOS
    EnvironmentCI.DeviceFlags &= ~(ARCHIVE_DEVICE_DATA_FLAG_GL | ARCHIVE_DEVICE_DATA_FLAG_GLES);
#endif
    EnvironmentCI.RenderStateDirs = {"RenderStates/RenderStatePackager"};
    EnvironmentCI.ShaderDirs      = {"Shaders"};

    auto pEnvironment = std::make_unique<ParsingEnvironment>(EnvironmentCI);
    ASSERT_TRUE(pEnvironment->Initialize());

    auto  pArchiverFactory = pEnvironment->GetArchiverFactory();
    auto& Packager         = pEnvironment->GetPackager();

    std::vector<std::string> InputFilePaths{"RenderStatesLibrary.json"};
    ASSERT_TRUE(Packager.ParseFiles(InputFilePaths));

    RefCntAutoPtr<IArchiver> pArchiver;
    pArchiverFactory->CreateArchiver(pEnvironment->GetSerializationDevice(), &pArchiver);
    ASSERT_TRUE(Packager.Execute(pArchiver));

    RefCntAutoPtr<IDataBlob> pData;
    ASSERT_TRUE(pArchiver->SerializeToBlob(ContentVersion, &pData));
}

TEST(Tools_RenderStatePackager, ResourceSignatureTest)
{
    ParsingEnvironmentCreateInfo EnvironmentCI{};
    EnvironmentCI.DeviceFlags     = GetDeviceFlags();
    EnvironmentCI.RenderStateDirs = {"RenderStates/RenderStatePackager"};
    EnvironmentCI.ShaderDirs      = {"Shaders"};

    auto pEnvironment = std::make_unique<ParsingEnvironment>(EnvironmentCI);
    ASSERT_TRUE(pEnvironment->Initialize());

    auto  pArchiverFactory = pEnvironment->GetArchiverFactory();
    auto& Packager         = pEnvironment->GetPackager();

    std::vector<std::string> InputFilePaths{"ResourceSignature.json"};
    ASSERT_TRUE(Packager.ParseFiles(InputFilePaths));

    RefCntAutoPtr<IArchiver> pArchiver;
    pArchiverFactory->CreateArchiver(pEnvironment->GetSerializationDevice(), &pArchiver);
    ASSERT_TRUE(Packager.Execute(pArchiver));

    RefCntAutoPtr<IDataBlob> pData;
    ASSERT_TRUE(pArchiver->SerializeToBlob(ContentVersion, &pData));
}

TEST(Tools_RenderStatePackager, ImportTest)
{
    ParsingEnvironmentCreateInfo EnvironmentCI{};
    EnvironmentCI.DeviceFlags     = GetDeviceFlags();
    EnvironmentCI.RenderStateDirs = {"RenderStates/RenderStatePackager"};
    EnvironmentCI.ShaderDirs      = {"Shaders"};

    auto pEnvironment = std::make_unique<ParsingEnvironment>(EnvironmentCI);
    ASSERT_TRUE(pEnvironment->Initialize());

    auto  pArchiverFactory = pEnvironment->GetArchiverFactory();
    auto& Packager         = pEnvironment->GetPackager();

    std::vector<std::string> InputFilePaths{"ResourceSignature.json", "Import0.json", "Import1.json"};
    ASSERT_TRUE(Packager.ParseFiles(InputFilePaths));

    RefCntAutoPtr<IArchiver> pArchiver;
    pArchiverFactory->CreateArchiver(pEnvironment->GetSerializationDevice(), &pArchiver);
    ASSERT_TRUE(Packager.Execute(pArchiver));

    RefCntAutoPtr<IDataBlob> pData;
    ASSERT_TRUE(pArchiver->SerializeToBlob(ContentVersion, &pData));
}

TEST(Tools_RenderStatePackager, IncorrectShaderPathTest)
{
    ParsingEnvironmentCreateInfo EnvironmentCI{};
    EnvironmentCI.DeviceFlags = GetDeviceFlags();
#if PLATFORM_MACOS
    // Compute shader are not supported in OpenGL on MacOS
    EnvironmentCI.DeviceFlags &= ~(ARCHIVE_DEVICE_DATA_FLAG_GL | ARCHIVE_DEVICE_DATA_FLAG_GLES);
#endif
    EnvironmentCI.RenderStateDirs = {"RenderStates/RenderStatePackager"};
    EnvironmentCI.ShaderDirs      = {""};
    EnvironmentCI.ThreadCount     = 1;

    auto pEnvironment = std::make_unique<ParsingEnvironment>(EnvironmentCI);
    ASSERT_TRUE(pEnvironment->Initialize());

    auto  pArchiverFactory = pEnvironment->GetArchiverFactory();
    auto& Packager         = pEnvironment->GetPackager();

    std::vector<std::string> InputFilePaths{"RenderStatesLibrary.json"};
    ASSERT_TRUE(Packager.ParseFiles(InputFilePaths));

    RefCntAutoPtr<IArchiver> pArchiver;
    pArchiverFactory->CreateArchiver(pEnvironment->GetSerializationDevice(), &pArchiver);

    const char* StackTrace[] = {
        "Failed to create shader from file 'ComputePrimitives.hlsl'",
        "Failed to load shader source file 'ComputePrimitives.hlsl'",
        "Failed to create input stream for source file ComputePrimitives.hlsl",
        "Failed to create shader from file 'GraphicsPrimitives.hlsl'",
        "Failed to load shader source file 'GraphicsPrimitives.hlsl'",
        "Failed to create input stream for source file GraphicsPrimitives.hlsl",
    };

    TestingEnvironment::ErrorScope TestScope{
        "Failed to create state objects",
        StackTrace[0], "Failed to create Shader object 'ClearBufferCounter-CS'", StackTrace[1], StackTrace[2],
        StackTrace[0], "Failed to create Shader object 'ClearUnorderedAccessViewUint-CS'", StackTrace[1], StackTrace[2],
        StackTrace[3], "Failed to create Shader object 'BlitTexture-PS'", StackTrace[4], StackTrace[5],
        StackTrace[3], "Failed to create Shader object 'BlitTexture-VS'", StackTrace[4], StackTrace[5]};
    EXPECT_FALSE(Packager.Execute(pArchiver));
}


TEST(Tools_RenderStatePackager, IncorrectShaderTest)
{
    ParsingEnvironmentCreateInfo EnvironmentCI{};
    EnvironmentCI.DeviceFlags     = GetDeviceFlags();
    EnvironmentCI.RenderStateDirs = {"RenderStates/RenderStatePackager"};
    EnvironmentCI.ShaderDirs      = {"Shaders"};
    EnvironmentCI.ThreadCount     = 1;

    auto pEnvironment = std::make_unique<ParsingEnvironment>(EnvironmentCI);
    ASSERT_TRUE(pEnvironment->Initialize());

    auto  pArchiverFactory = pEnvironment->GetArchiverFactory();
    auto& Packager         = pEnvironment->GetPackager();

    std::vector<std::string> InputFilePaths{"InvalidResources.json"};
    ASSERT_TRUE(Packager.ParseFiles(InputFilePaths));

    RefCntAutoPtr<IArchiver> pArchiver;
    pArchiverFactory->CreateArchiver(pEnvironment->GetSerializationDevice(), &pArchiver);

    TestingEnvironment::ErrorScope TestScope
    {
        "Failed to create state objects",
            "Failed to create shader from file 'BrokenShader.hlsl'",
            "Failed to create Shader object 'BrokenShader-VS'",
            "Failed to compile shader 'BrokenShader-VS'",
#if !D3D11_SUPPORTED && !D3D12_SUPPORTED
            "Failed to parse shader source"
#endif
    };
    EXPECT_FALSE(Packager.Execute(pArchiver));
}

TEST(Tools_RenderStatePackager, IncorrectRenderStatePath)
{
    ParsingEnvironmentCreateInfo EnvironmentCI{};
    EnvironmentCI.DeviceFlags     = GetDeviceFlags();
    EnvironmentCI.RenderStateDirs = {""};
    EnvironmentCI.ShaderDirs      = {"Shaders"};
    EnvironmentCI.ThreadCount     = 1;

    auto pEnvironment = std::make_unique<ParsingEnvironment>(EnvironmentCI);
    ASSERT_TRUE(pEnvironment->Initialize());

    auto& Packager = pEnvironment->GetPackager();

    std::vector<std::string> InputFilePaths{"RenderStatesLibrary.json"};

    TestingEnvironment::ErrorScope TestScope{
        "Failed create render state notation parser",
        "Failed to parse file: 'RenderStatesLibrary.json'",
        "Failed to open file: 'RenderStatesLibrary.json'",
        "Failed to open file: 'RenderStatesLibrary.json'",
        "Failed to create input stream for source file RenderStatesLibrary.json"};
    EXPECT_FALSE(Packager.ParseFiles(InputFilePaths));
}

TEST(Tools_RenderStatePackager, MissingObjectsTest)
{
    ParsingEnvironmentCreateInfo EnvironmentCI{};
    EnvironmentCI.DeviceFlags     = GetDeviceFlags();
    EnvironmentCI.RenderStateDirs = {"RenderStates/RenderStatePackager"};
    EnvironmentCI.ShaderDirs      = {"Shaders"};
    EnvironmentCI.ThreadCount     = 1;

    auto pEnvironment = std::make_unique<ParsingEnvironment>(EnvironmentCI);
    ASSERT_TRUE(pEnvironment->Initialize());

    auto  pArchiverFactory = pEnvironment->GetArchiverFactory();
    auto& Packager         = pEnvironment->GetPackager();

    {
        std::vector<std::string> InputFilePaths{"MissingShader.json"};
        ASSERT_TRUE(Packager.ParseFiles(InputFilePaths));

        RefCntAutoPtr<IArchiver> pArchiver;
        pArchiverFactory->CreateArchiver(pEnvironment->GetSerializationDevice(), &pArchiver);

        TestingEnvironment::ErrorScope TestScope{
            "Failed to create state objects",
            "Unable to find shader 'ClearUnorderedAccessViewUint-CS'"};
        EXPECT_FALSE(Packager.Execute(pArchiver));
        Packager.Reset();
    }

    {
        std::vector<std::string> InputFilePaths{"MissingRenderPass.json"};
        ASSERT_TRUE(Packager.ParseFiles(InputFilePaths));

        RefCntAutoPtr<IArchiver> pArchiver;
        pArchiverFactory->CreateArchiver(pEnvironment->GetSerializationDevice(), &pArchiver);

        TestingEnvironment::ErrorScope TestScope{"Failed to create state objects",
                                                 "Unable to find render pass 'TestRenderPass'"};
        EXPECT_FALSE(Packager.Execute(pArchiver));
        Packager.Reset();
    }

    {
        std::vector<std::string> InputFilePaths{"MissingResourceSignature.json"};
        ASSERT_TRUE(Packager.ParseFiles(InputFilePaths));

        RefCntAutoPtr<IArchiver> pArchiver;
        pArchiverFactory->CreateArchiver(pEnvironment->GetSerializationDevice(), &pArchiver);

        TestingEnvironment::ErrorScope TestScope{"Failed to create state objects",
                                                 "Unable to find resource signature 'TestResourceSignature'"};
        EXPECT_FALSE(Packager.Execute(pArchiver));
        Packager.Reset();
    }
}


TEST(Tools_RenderStatePackager, BytecodeDumpTest)
{
    ParsingEnvironmentCreateInfo EnvironmentCI{};
    EnvironmentCI.DeviceFlags = GetDeviceFlags();
#if PLATFORM_MACOS
    // Compute shader are not supported in OpenGL on MacOS
    EnvironmentCI.DeviceFlags &= ~(ARCHIVE_DEVICE_DATA_FLAG_GL | ARCHIVE_DEVICE_DATA_FLAG_GLES);
#endif
    EnvironmentCI.RenderStateDirs = {"RenderStates/RenderStatePackager"};
    EnvironmentCI.ShaderDirs      = {"Shaders"};
    EnvironmentCI.ConfigFilePath  = "RenderStatePackagerConfig.json";

    auto pEnvironment = std::make_unique<ParsingEnvironment>(EnvironmentCI);
    ASSERT_TRUE(pEnvironment->Initialize());

    auto  pArchiverFactory = pEnvironment->GetArchiverFactory();
    auto& Packager         = pEnvironment->GetPackager();

    std::vector<std::string> InputFilePaths{"GraphicsPrimitivesDump.json"};
    ASSERT_TRUE(Packager.ParseFiles(InputFilePaths));

    constexpr const char* TempFolder = "./PackagerBytecodeTemp/";

    constexpr const char* PipelineNames[] = {
        "/compute/Clear Buffer Counter/ClearBufferCounter CS",
        "/compute/Clear Unordered Access View Uint/ClearUnorderedAccessViewUint CS",
        "/graphics/Blit Texture/BlitTexture PS",
        "/graphics/Blit Texture/BlitTexture VS"};

    RefCntAutoPtr<IArchiver> pArchiver;
    pArchiverFactory->CreateArchiver(pEnvironment->GetSerializationDevice(), &pArchiver);
    ASSERT_TRUE(Packager.Execute(pArchiver, TempFolder));

    for (auto Flags = EnvironmentCI.DeviceFlags; Flags != ARCHIVE_DEVICE_DATA_FLAG_NONE;)
    {
        const auto DeviceFlag = ExtractLSB(Flags);
        const auto DeviceDir  = GetArchiveDeviceDataFlagString(DeviceFlag);

        for (auto PipelineName : PipelineNames)
        {
            std::string Path{};
            Path.append(TempFolder);
            Path.append(DeviceDir);
            Path.append(PipelineName);
            const auto IsGL = DeviceFlag == ARCHIVE_DEVICE_DATA_FLAG_GL || DeviceFlag == ARCHIVE_DEVICE_DATA_FLAG_GLES;
            const auto Ext  = RenderStatePackager::GetShaderFileExtension(DeviceFlag, IsGL ? SHADER_SOURCE_LANGUAGE_GLSL : SHADER_SOURCE_LANGUAGE_HLSL, !IsGL /* UseBytecode */);
            EXPECT_TRUE(FileSystem::FileExists((Path + Ext).c_str())) << Path + Ext;

            if (DeviceFlag & (ARCHIVE_DEVICE_DATA_FLAG_METAL_MACOS | ARCHIVE_DEVICE_DATA_FLAG_METAL_IOS))
            {
                for (auto MetalExt : {".metal", ".metallib"})
                    EXPECT_TRUE(FileSystem::FileExists((Path + MetalExt).c_str())) << Path + MetalExt;
            }
        }
    }

    FileSystem::DeleteDirectory(TempFolder);
}

TEST(Tools_RenderStatePackager, IgnoredSignatures)
{
    ParsingEnvironmentCreateInfo EnvironmentCI{};
    EnvironmentCI.DeviceFlags     = GetDeviceFlags();
    EnvironmentCI.RenderStateDirs = {"RenderStates/RenderStatePackager"};

    auto pEnvironment = std::make_unique<ParsingEnvironment>(EnvironmentCI);
    ASSERT_TRUE(pEnvironment->Initialize());

    auto  pArchiveFactory = pEnvironment->GetArchiverFactory();
    auto& Packager        = pEnvironment->GetPackager();

    std::vector<std::string> InputFilePaths{"IgnoredSignatures.json"};
    ASSERT_TRUE(Packager.ParseFiles(InputFilePaths));

    const auto* pParser = Packager.GetParser();
    EXPECT_TRUE(pParser->IsSignatureIgnored("IgnoredSignature1"));
    EXPECT_TRUE(pParser->IsSignatureIgnored("IgnoredSignature2"));
    EXPECT_TRUE(pParser->IsSignatureIgnored("IgnoredSignature3"));
    EXPECT_FALSE(pParser->IsSignatureIgnored("Signature"));

    RefCntAutoPtr<IArchiver> pArchiver;
    pArchiveFactory->CreateArchiver(pEnvironment->GetSerializationDevice(), &pArchiver);
    ASSERT_TRUE(Packager.Execute(pArchiver));

    RefCntAutoPtr<IDataBlob> pData;
    ASSERT_TRUE(pArchiver->SerializeToBlob(ContentVersion, &pData));
}

} // namespace
