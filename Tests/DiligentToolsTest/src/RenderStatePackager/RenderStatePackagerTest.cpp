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

#include <memory>
#include <vector>
#include <string>

#include "gtest/gtest.h"
#include "RenderStatePackager.hpp"
#include "ParsingEnvironment.hpp"

using namespace Diligent;

namespace
{

static constexpr ARCHIVE_DEVICE_DATA_FLAGS GetDeviceBits()
{
    ARCHIVE_DEVICE_DATA_FLAGS DeviceBits = ARCHIVE_DEVICE_DATA_FLAG_NONE;
#if D3D11_SUPPORTED
    DeviceBits = DeviceBits | ARCHIVE_DEVICE_DATA_FLAG_D3D11;
#endif
#if D3D12_SUPPORTED
    DeviceBits = DeviceBits | ARCHIVE_DEVICE_DATA_FLAG_D3D12;
#endif
#if VULKAN_SUPPORTED
    DeviceBits = DeviceBits | ARCHIVE_DEVICE_DATA_FLAG_VULKAN;
#endif
#if METAL_SUPPORTED
    DeviceBits = DeviceBits | ARCHIVE_DEVICE_DATA_FLAG_METAL_MACOS;
    DeviceBits = DeviceBits | ARCHIVE_DEVICE_DATA_FLAG_METAL_IOS;
#endif
    return DeviceBits;
}

TEST(Tools_RenderStatePackager, PackagerBasicTest)
{
    ParsingEnvironmentCreateInfo EnvironmentCI{};
    EnvironmentCI.DeviceBits      = GetDeviceBits();
    EnvironmentCI.RenderStateDirs = {"RenderStates/RenderStatePackager"};
    EnvironmentCI.ShaderDirs      = {"Shaders"};

    auto pEnvironment = std::make_unique<ParsingEnvironment>(EnvironmentCI);
    ASSERT_TRUE(pEnvironment->Initilize());

    auto pArchiveFactory = pEnvironment->GetArchiveFactory();
    auto pConverter      = pEnvironment->GetDeviceObjectConverter();

    std::vector<std::string> InputFilePaths{"RenderStatesLibrary.json"};
    ASSERT_TRUE(pConverter->ParseFiles(InputFilePaths));

    RefCntAutoPtr<IArchiver> pArchive;
    pArchiveFactory->CreateArchiver(pEnvironment->GetSerializationDevice(), &pArchive);
    ASSERT_TRUE(pConverter->Execute(pArchive));

    RefCntAutoPtr<IDataBlob> pData;
    ASSERT_TRUE(pArchive->SerializeToBlob(&pData));
}

TEST(Tools_RenderStatePackager, PackagerIncorrectShaderPathTest)
{
    ParsingEnvironmentCreateInfo EnvironmentCI{};
    EnvironmentCI.DeviceBits      = GetDeviceBits();
    EnvironmentCI.RenderStateDirs = {"RenderStates/RenderStatePackager"};
    EnvironmentCI.ShaderDirs      = {""};

    auto pEnvironment = std::make_unique<ParsingEnvironment>(EnvironmentCI);
    ASSERT_TRUE(pEnvironment->Initilize());

    auto pArchiveFactory = pEnvironment->GetArchiveFactory();
    auto pConverter      = pEnvironment->GetDeviceObjectConverter();

    std::vector<std::string> InputFilePaths{"RenderStatesLibrary.json"};
    ASSERT_TRUE(pConverter->ParseFiles(InputFilePaths));

    RefCntAutoPtr<IArchiver> pArchive;
    pArchiveFactory->CreateArchiver(pEnvironment->GetSerializationDevice(), &pArchive);

    ASSERT_FALSE(pConverter->Execute(pArchive));
}

TEST(Tools_RenderStatePackager, PackagerIncorrectRenderStatePath)
{
    ParsingEnvironmentCreateInfo EnvironmentCI{};
    EnvironmentCI.DeviceBits      = GetDeviceBits();
    EnvironmentCI.RenderStateDirs = {""};
    EnvironmentCI.ShaderDirs      = {"Shaders"};

    auto pEnvironment = std::make_unique<ParsingEnvironment>(EnvironmentCI);
    ASSERT_TRUE(pEnvironment->Initilize());

    auto pConverter = pEnvironment->GetDeviceObjectConverter();

    std::vector<std::string> InputFilePaths{"RenderStatesLibrary.json"};
    ASSERT_FALSE(pConverter->ParseFiles(InputFilePaths));
}

TEST(Tools_RenderStatePackager, PackagerMissingObjectsTest)
{
    ParsingEnvironmentCreateInfo EnvironmentCI{};
    EnvironmentCI.DeviceBits      = GetDeviceBits();
    EnvironmentCI.RenderStateDirs = {"RenderStates/RenderStatePackager"};
    EnvironmentCI.ShaderDirs      = {"Shaders"};

    auto pEnvironment = std::make_unique<ParsingEnvironment>(EnvironmentCI);
    ASSERT_TRUE(pEnvironment->Initilize());

    auto pArchiveFactory = pEnvironment->GetArchiveFactory();

    auto pConverter = pEnvironment->GetDeviceObjectConverter();

    {
        std::vector<std::string> InputFilePaths{"MissingShader.json"};
        ASSERT_TRUE(pConverter->ParseFiles(InputFilePaths));

        RefCntAutoPtr<IArchiver> pArchive;
        pArchiveFactory->CreateArchiver(pEnvironment->GetSerializationDevice(), &pArchive);

        ASSERT_FALSE(pConverter->Execute(pArchive));
        pConverter->Reset();
    }

    {
        std::vector<std::string> InputFilePaths{"MissingRenderPass.json"};
        ASSERT_TRUE(pConverter->ParseFiles(InputFilePaths));

        RefCntAutoPtr<IArchiver> pArchive;
        pArchiveFactory->CreateArchiver(pEnvironment->GetSerializationDevice(), &pArchive);

        ASSERT_FALSE(pConverter->Execute(pArchive));
        pConverter->Reset();
    }

    {
        std::vector<std::string> InputFilePaths{"MissingResourceSignature.json"};
        ASSERT_TRUE(pConverter->ParseFiles(InputFilePaths));

        RefCntAutoPtr<IArchiver> pArchive;
        pArchiveFactory->CreateArchiver(pEnvironment->GetSerializationDevice(), &pArchive);

        ASSERT_FALSE(pConverter->Execute(pArchive));
        pConverter->Reset();
    }
}

} // namespace
