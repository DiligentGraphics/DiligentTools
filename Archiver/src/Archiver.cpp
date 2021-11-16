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
#include <fstream>
using namespace Diligent;


int main(int argc, char* argv[])
{     
    EngineEnvironment::Initialize(argc, argv);

    auto pEnvironment     = EngineEnvironment::GetInstance();
    auto pArchiveFactory  = pEnvironment->GetArchiveFactory();
    auto pMemoryAllocator = pEnvironment->GetDeviceObjectReflection();

    RefCntAutoPtr<IArchiver> pBuilder;
    pArchiveFactory->CreateArchiver(pEnvironment->GetSerializationDevice(), &pBuilder);

    auto const& OutputFilePath = pEnvironment->GetDesc().OuputFilePath;
    auto const& InputFilePaths = pEnvironment->GetDesc().InputFilePaths;

    for (auto const& e : InputFilePaths)
    {
        std::ifstream  stream(e);
        nlohmann::json Json = nlohmann::json::parse(stream);

        GraphicsPipelineStateCreateInfo PSOCreateInfo = {};
        nlohmann::from_json(Json, PSOCreateInfo);

        PipelineStateArchiveInfo ArchiveInfo = {};
        ArchiveInfo.DeviceBits               = pEnvironment->GetDesc().DeviceBits;
        if (!pBuilder->AddGraphicsPipelineState(PSOCreateInfo, ArchiveInfo))
            LOG_FATAL_ERROR("Failed to ArchiveGraphicsPipelineState -> '", PSOCreateInfo.PSODesc.Name, "'.");
    }

    RefCntAutoPtr<IDataBlob> pData;
    pBuilder->SerializeToBlob(&pData);

    std::ofstream stream(OutputFilePath, std::ios::binary);
    stream.write(static_cast<const char*>(pData->GetConstDataPtr()), pData->GetSize());

    EngineEnvironment::Shutdown();
}
