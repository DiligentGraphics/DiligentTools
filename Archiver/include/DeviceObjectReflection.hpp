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

#pragma once

#include "json.hpp"
#include "DynamicLinearAllocator.hpp"
#include "Shader.h"
#include "RenderPass.h"

namespace Diligent
{
class DeviceObjectReflection final
{
public:
    DeviceObjectReflection(RefCntAutoPtr<ISerializationDevice> pDevice, RefCntAutoPtr<IShaderSourceInputStreamFactory> pStreamFactory, Uint32 DeviceBits);

    template <typename T>
    T* Allocate(size_t Count = 1)
    {
        auto pData = m_pMemoryAllocator->Allocate<T>(Count);
        for (size_t i = 0; i < Count; i++)
            pData[i] = {};
        return pData;
    }

    Char* CopyString(const String Str)
    {
        return m_pMemoryAllocator->CopyString(Str);
    }

    void Serialize(nlohmann::json& Json, const IRenderPass* pDeviceObject);

    void Deserialize(const nlohmann::json& Json, IRenderPass** pDeviceObject);

    void Serialize(nlohmann::json& Json, const IShader* pDeviceObject);

    void Deserialize(const nlohmann::json& Json, IShader** pDeviceObject);

    void Serialize(nlohmann::json& Json, const IPipelineResourceSignature* pDeviceObject);

    void Deserialize(const nlohmann::json& Json, IPipelineResourceSignature** pDeviceObject);

    void Flush();

private:
    RefCntAutoPtr<ISerializationDevice>            m_pDevice;
    RefCntAutoPtr<IShaderSourceInputStreamFactory> m_pStreamFactory;
    std::unique_ptr<DynamicLinearAllocator>        m_pMemoryAllocator;

    std::vector<RefCntAutoPtr<IRenderPass>>                m_RenderPasses;
    std::vector<RefCntAutoPtr<IShader>>                    m_Shaders;
    std::vector<RefCntAutoPtr<IPipelineResourceSignature>> m_ResourceSignatures;

    Uint32 m_DeviceBits;
};

} // namespace Diligent
