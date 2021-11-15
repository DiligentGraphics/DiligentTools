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


#include "EngineEnvironment.hpp"

namespace Diligent
{

EngineEnvironment* EngineEnvironment::m_pEnvironment = nullptr;

IArchiverFactory* EngineEnvironment::GetArchiveFactory()
{
    return m_pArchiveBuilderFactory;
}

ISerializationDevice* EngineEnvironment::GetSerializationDevice() {
    return m_pSerializationDevice;
}

IShaderSourceInputStreamFactory* EngineEnvironment::GetShaderSourceInputStreamFactory() {
    return m_pShaderStreamFactory;
}

DeviceObjectReflection* EngineEnvironment::GetDeviceObjectReflection() {
    return nullptr;
}


Uint32 EngineEnvironment::GetDeviceBits() const
{
    //TODO Add option for command line
    Uint32 DeviceBits = 0;
    DeviceBits |= 1 << RENDER_DEVICE_TYPE_D3D12;
    DeviceBits |= 1 << RENDER_DEVICE_TYPE_VULKAN;
    return DeviceBits;
}

EngineEnvironment::EngineEnvironment(const CreateInfo& CI)
{
    auto GetArchiveBuilderFactory = LoadArchiverFactory();
    if (GetArchiveBuilderFactory != nullptr)
    {
        m_pArchiveBuilderFactory = GetArchiveBuilderFactory();
    }
    else
    {
        LOG_FATAL_ERROR("Failed LoadArchiveBuilderFactory");
    }

   // m_pMemoryAllocator = std::make_unique<DynamicLinearAllocator>(DefaultRawMemoryAllocator::GetAllocator());

    SerializationDeviceCreateInfo DeviceCI = {};
    m_pArchiveBuilderFactory->CreateSerializationDevice(DeviceCI , &m_pSerializationDevice);
    m_pArchiveBuilderFactory->CreateDefaultShaderSourceStreamFactory(".", &m_pShaderStreamFactory);
   // VERIFY_EXPR(m_pMemoryAllocator != nullptr);
}

EngineEnvironment::~EngineEnvironment()
{
   // m_pMemoryAllocator->Free();
}

void EngineEnvironment::Initialize(int argc, char** argv)
{
    if (!m_pEnvironment)
    {
        CreateInfo CI  = {};
        m_pEnvironment = new EngineEnvironment{CI};
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
