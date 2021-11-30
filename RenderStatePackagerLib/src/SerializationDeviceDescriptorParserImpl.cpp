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
#include "SerializationDeviceDescriptorParserImpl.hpp"
#include "FileWrapper.hpp"
#include "DataBlobImpl.hpp"
#include "DefaultRawMemoryAllocator.hpp"

namespace Diligent
{

SerializationDeviceDescriptorParserImpl::SerializationDeviceDescriptorParserImpl(IReferenceCounters* pRefCounters, const char* StrData) :
    TBase{pRefCounters}
{
    m_pAllocator        = std::make_unique<DynamicLinearAllocator>(DefaultRawMemoryAllocator::GetAllocator());
    nlohmann::json Json = nlohmann::json::parse(StrData);
    Deserialize(Json, m_SerializationDeviceCI, *m_pAllocator);
}

Bool SerializationDeviceDescriptorParserImpl::GetDeviceState(SerializationDeviceCreateInfo& CreateInfo) const
{
    CreateInfo = m_SerializationDeviceCI;
    return true;
}

void CreateSerializationDeviceDescriptorParserFromFile(const Char*                            FilePath,
                                                       ISerializationDeviceDescriptorParser** ppParser)
{
    try
    {
        FileWrapper File{FilePath, EFileAccessMode::Read};
        if (!File)
            LOG_ERROR_AND_THROW("Failed to open file '", FilePath, "'.");

        RefCntAutoPtr<DataBlobImpl> pFileData{MakeNewRCObj<DataBlobImpl>()(0)};
        File->Read(pFileData);

        String Source(reinterpret_cast<const char*>(pFileData->GetConstDataPtr()), pFileData->GetSize());

        RefCntAutoPtr<ISerializationDeviceDescriptorParser> pParser{MakeNewRCObj<SerializationDeviceDescriptorParserImpl>()(Source.c_str())};
        if (pParser)
            pParser->QueryInterface(IID_SerializationDeviceDescriptorParser, reinterpret_cast<IObject**>(ppParser));
    }
    catch (std::runtime_error& err)
    {
        LOG_ERROR("Failed to create descriptor parser from file: ", err.what());
    }
}

void CreateISerializationDeviceDescriptorParserFromString(const Char*                            pData,
                                                          ISerializationDeviceDescriptorParser** ppParser)
{
    VERIFY_EXPR(pData != nullptr);
    try
    {
        RefCntAutoPtr<ISerializationDeviceDescriptorParser> pParser{MakeNewRCObj<SerializationDeviceDescriptorParserImpl>()(pData)};
        if (pParser)
            pParser->QueryInterface(IID_SerializationDeviceDescriptorParser, reinterpret_cast<IObject**>(ppParser));
    }
    catch (std::runtime_error& err)
    {
        LOG_ERROR("Failed to create descriptor parser from string: ", err.what());
    }
}

} // namespace Diligent

extern "C"
{
    void Diligent_CreateSerializationDeviceDescriptorParserFromFile(const char*                                      FilePath,
                                                                    Diligent::ISerializationDeviceDescriptorParser** ppLoader)
    {
        Diligent::CreateSerializationDeviceDescriptorParserFromFile(FilePath, ppLoader);
    }

    void Diligent_CreateSerializationDeviceDescriptorParserFromString(const char*                                      pData,
                                                                      Diligent::ISerializationDeviceDescriptorParser** ppLoader)
    {
        Diligent::CreateISerializationDeviceDescriptorParserFromString(pData, ppLoader);
    }
}
