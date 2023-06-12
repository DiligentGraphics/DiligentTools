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

#include "gtest/gtest.h"
#include "RefCntAutoPtr.hpp"
#include "DefaultShaderSourceStreamFactory.h"
#include "RenderStatePackager.hpp"
#include "ParsingEnvironment.hpp"
#include "GPUTestingEnvironment.hpp"

using namespace Diligent;
using namespace Diligent::Testing;

namespace
{

static constexpr Uint32 ContentVersion = 246;

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

TEST(Tools_RenderStatePackager, Pipeline)
{
    auto* pTestingEnv = GPUTestingEnvironment::GetInstance();
    auto* pDevice     = pTestingEnv->GetDevice();

    ParsingEnvironmentCreateInfo EnvironmentCI{};
    EnvironmentCI.DeviceFlags     = GetDeviceFlags();
    EnvironmentCI.RenderStateDirs = {"RenderStates"};
    EnvironmentCI.ShaderDirs      = {"Shaders"};
    EnvironmentCI.ThreadCount     = 1;

    auto pParsingEnv = std::make_unique<ParsingEnvironment>(EnvironmentCI);
    ASSERT_TRUE(pParsingEnv->Initialize());

    auto  pArchiverFactory = pParsingEnv->GetArchiverFactory();
    auto& Packager         = pParsingEnv->GetPackager();
    ASSERT_TRUE(Packager.ParseFiles({"PSO.json"}));

    RefCntAutoPtr<IArchiver> pArchiver;
    pArchiverFactory->CreateArchiver(pParsingEnv->GetSerializationDevice(), &pArchiver);
    ASSERT_TRUE(Packager.Execute(pArchiver));

    RefCntAutoPtr<IDataBlob> pArchive;
    pArchiver->SerializeToBlob(ContentVersion, &pArchive);
    ASSERT_NE(pArchive, nullptr);

    RefCntAutoPtr<IDearchiver> pDearchiver;
    DearchiverCreateInfo       DearchiverCI{};
    pDevice->GetEngineFactory()->CreateDearchiver(DearchiverCI, &pDearchiver);

    pDearchiver->LoadArchive(pArchive);

    RefCntAutoPtr<IPipelineState> pPSO;
    PipelineStateUnpackInfo       PsoUnpackInfo;
    PsoUnpackInfo.pDevice      = pDevice;
    PsoUnpackInfo.PipelineType = PIPELINE_TYPE_GRAPHICS;
    PsoUnpackInfo.Name         = "GeometryOpaque";
    pDearchiver->UnpackPipelineState(PsoUnpackInfo, &pPSO);
    EXPECT_NE(pPSO, nullptr);
}

void TestSignaturePacking(bool UseSplitArchive)
{
    auto* pTestingEnv = GPUTestingEnvironment::GetInstance();
    auto* pDevice     = pTestingEnv->GetDevice();

    ParsingEnvironmentCreateInfo EnvironmentCI{};
    EnvironmentCI.DeviceFlags     = GetDeviceFlags();
    EnvironmentCI.PSOArchiveFlags = PSO_ARCHIVE_FLAG_DO_NOT_PACK_SIGNATURES;
    EnvironmentCI.ThreadCount     = 1;
    EnvironmentCI.RenderStateDirs = {"RenderStates"};
    EnvironmentCI.ShaderDirs      = {"Shaders"};

    auto pParsingEnv = std::make_unique<ParsingEnvironment>(EnvironmentCI);
    ASSERT_TRUE(pParsingEnv->Initialize());

    auto  pArchiverFactory = pParsingEnv->GetArchiverFactory();
    auto& Packager         = pParsingEnv->GetPackager();

    RefCntAutoPtr<IArchiver> pArchiver;
    pArchiverFactory->CreateArchiver(pParsingEnv->GetSerializationDevice(), &pArchiver);

    RefCntAutoPtr<IDataBlob> pPSOArchive, pSignArchive;
    if (UseSplitArchive)
    {
        ASSERT_TRUE(Packager.ParseFiles({"Signature.json"}));
        ASSERT_TRUE(Packager.Execute(pArchiver));
        pArchiver->SerializeToBlob(ContentVersion, &pSignArchive);
        ASSERT_NE(pSignArchive, nullptr);

        ASSERT_TRUE(Packager.ParseFiles({"PSO_Sign.json", "IgnoreSignature.json"}));
        pArchiver->Reset();
        ASSERT_TRUE(Packager.Execute(pArchiver));
        pArchiver->SerializeToBlob(ContentVersion, &pPSOArchive);
        ASSERT_NE(pPSOArchive, nullptr);
    }
    else
    {
        ASSERT_TRUE(Packager.ParseFiles({"PSO_Sign.json", "Signature.json"}));
        ASSERT_TRUE(Packager.Execute(pArchiver));
        pArchiver->SerializeToBlob(ContentVersion, &pPSOArchive);
        ASSERT_NE(pPSOArchive, nullptr);
        pSignArchive = pPSOArchive;
    }

    RefCntAutoPtr<IDearchiver> pDearchiver;
    DearchiverCreateInfo       DearchiverCI{};
    pDevice->GetEngineFactory()->CreateDearchiver(DearchiverCI, &pDearchiver);

    pDearchiver->LoadArchive(pSignArchive);

    {
        RefCntAutoPtr<IPipelineResourceSignature> pPRS;
        ResourceSignatureUnpackInfo               PRSUnpackInfo;
        PRSUnpackInfo.pDevice = pDevice;
        PRSUnpackInfo.Name    = "TestSignature";
        pDearchiver->UnpackResourceSignature(PRSUnpackInfo, &pPRS);
        EXPECT_NE(pPRS, nullptr);
    }

    pDearchiver->LoadArchive(pPSOArchive);

    {
        RefCntAutoPtr<IRenderPass> pRP;
        RenderPassUnpackInfo       RPUnpackInfo;
        RPUnpackInfo.pDevice = pDevice;
        RPUnpackInfo.Name    = "TestRenderPass";
        pDearchiver->UnpackRenderPass(RPUnpackInfo, &pRP);
        EXPECT_NE(pRP, nullptr);
    }

    {
        RefCntAutoPtr<IPipelineState> pPSO;
        PipelineStateUnpackInfo       PsoUnpackInfo;
        PsoUnpackInfo.pDevice      = pDevice;
        PsoUnpackInfo.PipelineType = PIPELINE_TYPE_GRAPHICS;
        PsoUnpackInfo.Name         = "GeometryOpaque";
        pDearchiver->UnpackPipelineState(PsoUnpackInfo, &pPSO);
        EXPECT_NE(pPSO, nullptr);
    }
}

TEST(Tools_RenderStatePackager, Signature)
{
    constexpr bool UseSplitArchive = false;
    TestSignaturePacking(UseSplitArchive);
}

TEST(Tools_RenderStatePackager, Signature_SplitArchive)
{
    constexpr bool UseSplitArchive = true;
    TestSignaturePacking(UseSplitArchive);
}

} // namespace
