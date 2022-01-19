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
#include <unordered_map>

#include "Archiver.h"
#include "ThreadPool.hpp"
#include "RefCntAutoPtr.hpp"
#include "SerializationDevice.h"
#include "RenderStateNotationParser.h"
#include "HashUtils.hpp"

namespace Diligent
{

class RenderStatePackager final
{
public:
    RenderStatePackager(RefCntAutoPtr<ISerializationDevice>            pDevice,
                        RefCntAutoPtr<IShaderSourceInputStreamFactory> pShaderStreamFactory,
                        RefCntAutoPtr<IShaderSourceInputStreamFactory> pRenderStateStreamFactory,
                        RefCntAutoPtr<IThreadPool>                     pThreadPool,
                        ARCHIVE_DEVICE_DATA_FLAGS                      DeviceFlags,
                        PSO_ARCHIVE_FLAGS                              PSOArchiveFlags);

    bool ParseFiles(std::vector<std::string> const& DRSNPaths);

    bool Execute(RefCntAutoPtr<IArchiver> pArchive);

    void Reset();

private:
    RefCntAutoPtr<ISerializationDevice>            m_pDevice;
    RefCntAutoPtr<IShaderSourceInputStreamFactory> m_pShaderStreamFactory;
    RefCntAutoPtr<IShaderSourceInputStreamFactory> m_pRenderStateStreamFactory;
    RefCntAutoPtr<IThreadPool>                     m_pThreadPool;

    template <typename T>
    using TNamedObjectHashMap = std::unordered_map<HashMapStringKey, RefCntAutoPtr<T>, HashMapStringKey::Hasher>;

    TNamedObjectHashMap<IShader>                           m_Shaders;
    TNamedObjectHashMap<IRenderPass>                       m_RenderPasses;
    TNamedObjectHashMap<IPipelineResourceSignature>        m_ResourceSignatures;
    std::vector<RefCntAutoPtr<IRenderStateNotationParser>> m_RSNParsers;

    const ARCHIVE_DEVICE_DATA_FLAGS m_DeviceFlags;
    const PSO_ARCHIVE_FLAGS         m_PSOArchiveFlags;
};

} // namespace Diligent
