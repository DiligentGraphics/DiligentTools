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
 /// Defines Diligent::IDeviceObjectDescriptionParser interface

DILIGENT_BEGIN_NAMESPACE(Diligent)

struct PipelineStateCreateInfoJSON 
{
    PipelineStateDesc PSODesc;
    
};

struct GraphicsPipelineStateDescJSON DILIGENT_DERIVE(PipelineStateCreateInfoJSON) 
{
    GraphicsPipelineDesc GraphicsPipeline;

    const char* VS DEFAULT_INITIALIZER(nullptr);

    const char* PS DEFAULT_INITIALIZER(nullptr);

    const char* DS DEFAULT_INITIALIZER(nullptr);

    const char* HS DEFAULT_INITIALIZER(nullptr);

    const char* GS DEFAULT_INITIALIZER(nullptr);

    const char* AS DEFAULT_INITIALIZER(nullptr);

    const char* MS DEFAULT_INITIALIZER(nullptr);
};

struct ComputePipelineStateDescJSON DILIGENT_DERIVE(PipelineStateCreateInfoJSON) 
{
    const char* CS DEFAULT_INITIALIZER(nullptr);
};

struct ResourceSignatureDescJSON 
{

};

struct ShaderDescJSON 
{

};

struct RenderPassDescJSON 
{

};

// {355AC9f7-5D9D-423D-AE35-80E0028DE17E}
static const INTERFACE_ID IID_DeviceObjectDescriptionParser = {0x355AC9F7, 0x5D9D, 0x423D, {0xAE, 0x35, 0x80, 0xE0, 0x02, 0x8D, 0xE1, 0x7E}};

DILIGENT_BEGIN_INTERFACE(IDeviceObjectDescriptionParser, IObject) 
{
    VIRTUAL Bool METHOD(GetGraphicsPipelineStateDesc)(THIS_
                                                      const char* Name,
                                                      GraphicsPipelineStateDescJSON REF) PURE;

    VIRTUAL Bool METHOD(GetComputePipelineStateDesc)(THIS_ 
                                                     const char* Name,
                                                     ComputePipelineStateDescJSON REF) PURE;

    VIRTUAL Bool METHOD(GetResourceSignatureDesc)(THIS_
                                                  const char* Name,
                                                  ResourceSignatureDescJSON REF) PURE;

    VIRTUAL Bool METHOD(GetShaderDesc)(THIS_
                                       const char* Name,
                                       ShaderDescJSON REF) PURE;

    VIRTUAL Bool METHOD(GetRenderPassDesc)(THIS_
                                           const char* Name,
                                           RenderPassDescJSON REF) PURE;
};
DILIGENT_END_INTERFACE

#include "../../../Primitives/interface/UndefInterfaceHelperMacros.h"

#if DILIGENT_C_INTERFACE

#    define IDeviceObjectDescriptionParser_GetGraphicsPipelineStateDesc(This, ...) CALL_IFACE_METHOD(DeviceObjectDescriptionParser, GetGraphicsPipelineStateDesc, This, __VA_ARGS__)
#    define IDeviceObjectDescriptionParser_GetComputePipelineStateDesc(This, ...)  CALL_IFACE_METHOD(DeviceObjectDescriptionParser, GetComputePipelineStateDesc,  This, __VA_ARGS__)
#    define IDeviceObjectDescriptionParser_GetResourceSignatureDesc(This, ...)     CALL_IFACE_METHOD(DeviceObjectDescriptionParser, GetResourceSignatureDesc,     This, __VA_ARGS__)
#    define IDeviceObjectDescriptionParser_GetShaderDesc(This, ...)                CALL_IFACE_METHOD(DeviceObjectDescriptionParser, GetShaderDesc,                This, __VA_ARGS__)
#    define IDeviceObjectDescriptionParser_GetRenderPassDesc(This, ...)            CALL_IFACE_METHOD(DeviceObjectDescriptionParser, GetRenderPassDesc,            This, __VA_ARGS__)


#endif

DILIGENT_END_NAMESPACE // namespace Diligent
