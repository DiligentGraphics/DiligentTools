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

#pragma once

#include <vector>
#include <memory>

#include "ThreadPool.hpp"
#include "RefCntAutoPtr.hpp"
#include "ArchiverFactory.h"
#include "ArchiverFactoryLoader.h"
#include "RenderStatePackager.hpp"

namespace Diligent
{

struct ParsingEnvironmentCreateInfo
{
    ARCHIVE_DEVICE_DATA_FLAGS DeviceFlags          = {};
    PSO_ARCHIVE_FLAGS         PSOArchiveFlags      = {};
    Uint32                    ThreadCount          = 0;
    Uint32                    ContentVersion       = 0;
    bool                      PrintArchiveContents = false;
    std::vector<std::string>  ShaderDirs           = {};
    std::vector<std::string>  RenderStateDirs      = {};
    std::vector<std::string>  InputFilePaths       = {};
    std::string               OuputFilePath        = {};
    std::string               ConfigFilePath       = {};
    std::string               DumpBytecodeDir      = {};
};

class ParsingEnvironment final
{
public:
    IArchiverFactory* GetArchiverFactory();

    ISerializationDevice* GetSerializationDevice();

    IShaderSourceInputStreamFactory* GetShaderSourceInputStreamFactory();

    IShaderSourceInputStreamFactory* GetParserImportInputStreamFactory();

    RenderStatePackager& GetPackager();

    IThreadPool* GetThreadPool();

    bool Initialize();

    ParsingEnvironment(const ParsingEnvironmentCreateInfo& CI);

    ~ParsingEnvironment();

private:
    RefCntAutoPtr<IArchiverFactory>                m_pArchiveBuilderFactory;
    RefCntAutoPtr<ISerializationDevice>            m_pSerializationDevice;
    RefCntAutoPtr<IShaderSourceInputStreamFactory> m_pShaderStreamFactory;
    RefCntAutoPtr<IShaderSourceInputStreamFactory> m_pRenderStateStreamFactory;
    RefCntAutoPtr<IThreadPool>                     m_pThreadPool;
    std::unique_ptr<RenderStatePackager>           m_pPackager;
    ParsingEnvironmentCreateInfo                   m_CreateInfo;
};

} // namespace Diligent
