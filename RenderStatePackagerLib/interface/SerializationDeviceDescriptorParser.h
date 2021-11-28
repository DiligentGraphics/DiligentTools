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

 /// \file
 /// Defines Diligent::ISerializationDeviceDescriptorParser interface
#include "../../../DiligentCore/Graphics/Archiver/interface/SerializationDevice.h"


DILIGENT_BEGIN_NAMESPACE(Diligent)

// {355AC9f7-5D9D-423D-AE35-80E0028DE17E}
static const INTERFACE_ID IID_SerializationDeviceDescriptorParser = {0x355AC9F7, 0x5D9D, 0x423D, {0xAE, 0x35, 0x80, 0xE0, 0x02, 0x8D, 0xE1, 0x7E}};

#define DILIGENT_INTERFACE_NAME ISerializationDeviceDescriptorParser
#include "../../../DiligentCore/Primitives/interface/DefineInterfaceHelperMacros.h"

#define SerializationDeviceDescriptorParserInclusiveMethods \
    IObjectInclusiveMethods;                          \
    ISerializationDeviceDescriptorParser SerializationDeviceDescriptorParser


DILIGENT_BEGIN_INTERFACE(ISerializationDeviceDescriptorParser, IObject) {
    
    VIRTUAL Bool METHOD(GetDeviceState)(THIS_                                       
                                        SerializationDeviceCreateInfo REF CreateInfo) CONST PURE;

};
DILIGENT_END_INTERFACE

#include "../../../DiligentCore/Primitives/interface/UndefInterfaceHelperMacros.h"

#if DILIGENT_C_INTERFACE

#    define ISerializationDeviceDescriptorParser_GetDeviceState(This, ...) CALL_IFACE_METHOD(SerializationDeviceDescriptorParser, GetDeviceState, This, __VA_ARGS__)

#endif

#include "../../../DiligentCore/Primitives/interface/DefineGlobalFuncHelperMacros.h"

void DILIGENT_GLOBAL_FUNCTION(CreateSerializationDeviceDescriptorParserFromFile)(const Char* FilePath,
                                                                                 ISerializationDeviceDescriptorParser** ppParser);

void DILIGENT_GLOBAL_FUNCTION(CreateSerializationDeviceDescriptorParserFromString)(const Char* StrData,
                                                                                   ISerializationDeviceDescriptorParser** ppParser);

#include "../../../DiligentCore/Primitives/interface/UndefGlobalFuncHelperMacros.h"

DILIGENT_END_NAMESPACE // namespace Diligent
