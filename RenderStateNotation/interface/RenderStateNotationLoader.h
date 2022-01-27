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

// clang-format off

/// \file
/// Defines Diligent::IRenderStateNotationLoader interface
#include "RenderStateNotationParser.h"

DILIGENT_BEGIN_NAMESPACE(Diligent)

#if DILIGENT_C_INTERFACE
#    define REF *
#else
#    define REF &
#endif

struct RenderStateNotationLoaderCreateInfo
{
    IRenderDevice*                   pDevice        DEFAULT_INITIALIZER(nullptr);

    IRenderStateNotationParser*      pParser        DEFAULT_INITIALIZER(nullptr);

    IShaderSourceInputStreamFactory* pStreamFactory DEFAULT_INITIALIZER(nullptr);
};
typedef struct RenderStateNotationLoaderCreateInfo RenderStateNotationLoaderCreateInfo;

struct LoadResourceSignatureInfo
{
    const Char* Name                                         DEFAULT_INITIALIZER(nullptr);

    bool AddToCache                                          DEFAULT_INITIALIZER(true);

    void (*Modify)(PipelineResourceSignatureDesc REF, void*) DEFAULT_INITIALIZER(nullptr);

    void* pUserData                                          DEFAULT_INITIALIZER(nullptr);
};
typedef struct LoadResourceSignatureInfo LoadResourceSignatureInfo;

struct LoadRenderPassInfo
{
    const Char* Name                          DEFAULT_INITIALIZER(nullptr);

    bool AddToCache                           DEFAULT_INITIALIZER(true);

    void (*Modify)(RenderPassDesc REF, void*) DEFAULT_INITIALIZER(nullptr);

    void* pUserData                           DEFAULT_INITIALIZER(nullptr);
};
typedef struct LoadRenderPassInfo LoadRenderPassInfo;

struct LoadShaderInfo
{
    const Char* Name                            DEFAULT_INITIALIZER(nullptr);

    bool AddToCache                             DEFAULT_INITIALIZER(true);

    void (*Modify)(ShaderCreateInfo REF, void*) DEFAULT_INITIALIZER(nullptr);

    void* pUserData                             DEFAULT_INITIALIZER(nullptr);
};
typedef struct LoadShaderInfo LoadShaderInfo;

struct LoadPipelineStateInfo
{
    const Char* Name                                                                    DEFAULT_INITIALIZER(nullptr);

    PIPELINE_TYPE PipelineType                                                          DEFAULT_INITIALIZER(PIPELINE_TYPE_INVALID);

    bool AddToCache                                                                     DEFAULT_INITIALIZER(true);

    void (*ModifyPipeline)(PipelineStateCreateInfo REF, void*)                          DEFAULT_INITIALIZER(nullptr);

    void* pModifyPipelineData                                                           DEFAULT_INITIALIZER(nullptr);

    void (*ModifyShader)(ShaderCreateInfo REF, SHADER_TYPE, bool REF, void*)            DEFAULT_INITIALIZER(nullptr);

    void* pModifyShaderData                                                             DEFAULT_INITIALIZER(nullptr);

    void (*ModifyResourceSignature)(PipelineResourceSignatureDesc REF, bool REF, void*) DEFAULT_INITIALIZER(nullptr);

    void* pModifyResourceSignatureData                                                  DEFAULT_INITIALIZER(nullptr);

    void (*ModifyRenderPass)(RenderPassDesc REF, bool REF, void*)                       DEFAULT_INITIALIZER(nullptr);

    void* pModifyRenderPassData                                                         DEFAULT_INITIALIZER(nullptr);
};
typedef struct LoadPipelineStateInfo LoadPipelineStateInfo;

// clang-format on

// {FD9B12C5-3BC5-4729-A2B4-924DF374B3D3}
static const INTERFACE_ID IID_RenderStateNotationLoader = {0xFD9B12C5, 0x3BC5, 0x4729, {0xA2, 0xB4, 0x92, 0x4D, 0xF3, 0x74, 0xB3, 0xD3}};

#define DILIGENT_INTERFACE_NAME IRenderStateNotationLoader
#include "../../../DiligentCore/Primitives/interface/DefineInterfaceHelperMacros.h"

#define IRenderStateNotationLoaderInclusiveMethods \
    IObjectInclusiveMethods;                       \
    IRenderStateNotationLoader RenderStateNotationLoader

// clang-format off

DILIGENT_BEGIN_INTERFACE(IRenderStateNotationLoader, IObject) 
{
    VIRTUAL void METHOD(LoadPipelineState)(THIS_
                                           const LoadPipelineStateInfo REF LoadInfo, 
                                           IPipelineState**                ppPSO) PURE;

    VIRTUAL void METHOD(LoadResourceSignature)(THIS_
                                               const LoadResourceSignatureInfo REF LoadInfo,
                                               IPipelineResourceSignature**        ppSignature) PURE;

    VIRTUAL void METHOD(LoadRenderPass)(THIS_
                                        const LoadRenderPassInfo REF LoadInfo,
                                        IRenderPass**                ppRenderPass) PURE;

    VIRTUAL void METHOD(LoadShader)(THIS_
                                    const LoadShaderInfo REF LoadInfo,
                                    IShader**                ppShader) PURE;
};
DILIGENT_END_INTERFACE

#include "../../../DiligentCore/Primitives/interface/UndefInterfaceHelperMacros.h"

#if DILIGENT_C_INTERFACE

// clang-format off
#    define IRenderStateNotationLoader_LoadPipelineState(This, ...)     CALL_IFACE_METHOD(RenderStateNotationLoader, LoadPipelineState,     This, __VA_ARGS__)
#    define IRenderStateNotationLoader_LoadResourceSignature(This, ...) CALL_IFACE_METHOD(RenderStateNotationLoader, LoadResourceSignature, This, __VA_ARGS__)
#    define IRenderStateNotationLoader_LoadRenderPass(This, ...)        CALL_IFACE_METHOD(RenderStateNotationLoader, LoadRenderPass,        This, __VA_ARGS__)
#    define IRenderStateNotationLoader_LoadShader(This, ...)            CALL_IFACE_METHOD(RenderStateNotationLoader, LoadShader,            This, __VA_ARGS__)
// clang-format on

#endif

#include "../../../DiligentCore/Primitives/interface/DefineGlobalFuncHelperMacros.h"

void DILIGENT_GLOBAL_FUNCTION(CreateRenderStateNotationLoader)(const RenderStateNotationLoaderCreateInfo REF CreateInfo,
                                                               IRenderStateNotationLoader**                  ppLoader);

#include "../../../DiligentCore/Primitives/interface/UndefGlobalFuncHelperMacros.h"

DILIGENT_END_NAMESPACE // namespace Diligent
