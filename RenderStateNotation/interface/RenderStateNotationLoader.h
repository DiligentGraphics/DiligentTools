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
#include "../../../DiligentCore/Graphics/GraphicsTools/interface/RenderStateCache.h"


DILIGENT_BEGIN_NAMESPACE(Diligent)

#if DILIGENT_C_INTERFACE
#    define REF *
#else
#    define REF &
#endif

/// Render state notation loader initialization info.
struct RenderStateNotationLoaderCreateInfo
{
    /// A pointer to the render device that will be used to create objects.
    IRenderDevice*                   pDevice        DEFAULT_INITIALIZER(nullptr);

    /// A pointer to the render state notation parser.
    IRenderStateNotationParser*      pParser        DEFAULT_INITIALIZER(nullptr);

    /// The factory that is used to load the shader source files.
    IShaderSourceInputStreamFactory* pStreamFactory DEFAULT_INITIALIZER(nullptr);

    /// A pointer to an optional render state cache.
    IRenderStateCache*               pStateCache    DEFAULT_INITIALIZER(nullptr);
};
typedef struct RenderStateNotationLoaderCreateInfo RenderStateNotationLoaderCreateInfo;

/// Resource signature load info.
struct LoadResourceSignatureInfo
{
    /// Name of the resource signature to load.
    const Char* Name                                         DEFAULT_INITIALIZER(nullptr);

    /// Flag indicating whether to add the resource to the internal cache.
    bool AddToCache                                          DEFAULT_INITIALIZER(true);

    /// An optional function to be called by the render state notation loader
    /// to let the application modify the pipeline resource signature descriptor.
    void (*Modify)(PipelineResourceSignatureDesc REF, void*) DEFAULT_INITIALIZER(nullptr);

    /// A pointer to the user data to pass to the Modify function.
    void* pUserData                                          DEFAULT_INITIALIZER(nullptr);
};
typedef struct LoadResourceSignatureInfo LoadResourceSignatureInfo;

/// Render pass load info.
struct LoadRenderPassInfo
{
    /// Name of the render pass to load.
    const Char* Name                          DEFAULT_INITIALIZER(nullptr);

    /// Flag indicating whether to add the resource to the internal cache.
    bool AddToCache                           DEFAULT_INITIALIZER(true);

    /// An optional function to be called by the render state notation loader
    /// to let the application modify the render pass descriptor.
    void (*Modify)(RenderPassDesc REF, void*) DEFAULT_INITIALIZER(nullptr);

    /// A pointer to the user data to pass to the Modify function.
    void* pUserData                           DEFAULT_INITIALIZER(nullptr);
};
typedef struct LoadRenderPassInfo LoadRenderPassInfo;

/// Shader load info.
struct LoadShaderInfo
{
    /// Name of the shader to load.
    const Char* Name                            DEFAULT_INITIALIZER(nullptr);

    /// Flag indicating whether to add the resource to the internal cache.
    bool AddToCache                             DEFAULT_INITIALIZER(true);

    /// An optional function to be called by the render state notation loader
    /// to let the application modify the shader create info.
    void (*Modify)(ShaderCreateInfo REF, void*) DEFAULT_INITIALIZER(nullptr);

    /// A pointer to the user data to pass to the Modify function.
    void* pUserData                             DEFAULT_INITIALIZER(nullptr);
};
typedef struct LoadShaderInfo LoadShaderInfo;

/// Pipeline state load info.
struct LoadPipelineStateInfo
{
    /// Name of the PSO to load.
    const Char* Name                                                                    DEFAULT_INITIALIZER(nullptr);

    /// The type of the pipeline state to load, see Diligent::PIPELINE_TYPE.
    PIPELINE_TYPE PipelineType                                                          DEFAULT_INITIALIZER(PIPELINE_TYPE_INVALID);

    /// Flag indicating whether to add the resource to the internal cache.
    bool AddToCache                                                                     DEFAULT_INITIALIZER(true);

    /// An optional function to be called by the render state notation loader
    /// to let the application modify the pipeline state create info.
    ///
    /// \remarks    An application should check the pipeline type (PipelineCI.Desc.PipelineType) and cast
    ///             the reference to the appropriate PSO create info struct, e.g. for PIPELINE_TYPE_GRAPHICS:
    ///
    ///                 auto& GraphicsPipelineCI = static_cast<GraphicsPipelineStateCreateInfo>(PipelineCI);
    ///
    ///             Modifying graphics pipeline states (e.g. rasterizer, depth-stencil, blend, render
    ///             target formats, etc.) is the most expected usage of the callback.
    ///
    ///             The following members of the structure must not be modified:
    ///             - PipelineCI.PSODesc.PipelineType
    ///
    ///             An application may modify shader pointers, resource signature pointers and render pass pointer,
    ///             but it must ensure that all objects are compatible.
    //
    ///             The callbacks are executed in the following order:
    ///              - ModifyResourceSignature
    ///              - ModifyRenderPass
    ///              - ModifyShader
    ///              - ModifyPipeline
    void (*ModifyPipeline)(PipelineStateCreateInfo REF, void*)                          DEFAULT_INITIALIZER(nullptr);

    /// A pointer to the user data to pass to the ModifyPipeline function.
    void* pModifyPipelineData                                                           DEFAULT_INITIALIZER(nullptr);

    /// An optional function to be called by the render state notation loader
    /// to let the application modify the shader create info.
    ///
    /// \remarks    An application should choose shader stage to modify.
    ///
    ///                 switch (ShaderType) {
    ///                    case SHADER_TYPE_VERTEX:
    ///                        ShaderCI.Macros = MacrosList;
    ///                        break;
    ///                    case ...
    ///                 }
    ///
    ///             The following members of the structure must not be modified:
    ///             - ShaderCI.Desc.ShaderType
    ///
    ///             The third (bool) parameter indicates whether the modified shader object
    ///             should be added to the internal cache and should be set by the callee.
    void (*ModifyShader)(ShaderCreateInfo REF, SHADER_TYPE, bool REF, void*)            DEFAULT_INITIALIZER(nullptr);

    /// A pointer to the user data to pass to the ModifyShader function.
    void* pModifyShaderData                                                             DEFAULT_INITIALIZER(nullptr);

    /// An optional function to be called by the render state notation loader
    /// to let the application modify the pipeline resource signature descriptor.
    ///
    /// \remarks
    ///             The third (bool) parameter indicates whether the modified resource signature object
    ///             should be added to the internal cache and should be set by the callee.
    void (*ModifyResourceSignature)(PipelineResourceSignatureDesc REF, bool REF, void*) DEFAULT_INITIALIZER(nullptr);

    /// A pointer to the user data to pass to the ModifyResourceSignature function.
    void* pModifyResourceSignatureData                                                  DEFAULT_INITIALIZER(nullptr);

    /// An optional function to be called by the render state notation loader
    /// to let the application modify the pipeline render pass descriptor.
    ///
    /// \remarks
    ///             The third (bool) parameter indicates whether the modified render pass object
    ///             should be added to the internal cache and should be set by the callee.
    void (*ModifyRenderPass)(RenderPassDesc REF, bool REF, void*)                       DEFAULT_INITIALIZER(nullptr);

    /// A pointer to the user data to pass to the ModifyRenderPass function.
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

// Render state notation loader interface.
DILIGENT_BEGIN_INTERFACE(IRenderStateNotationLoader, IObject) 
{
    /// Loads a pipeline state from the render state notation parser.

    /// \param [in]  LoadInfo - Pipeline state load info, see Diligent::LoadPipelineStateInfo.
    /// \param [out] ppPSO    - Address of the memory location where a pointer to the pipeline state object will be stored.
    ///
    /// \remarks This method must be externally synchronized.
    VIRTUAL void METHOD(LoadPipelineState)(THIS_
                                           const LoadPipelineStateInfo REF LoadInfo, 
                                           IPipelineState**                ppPSO) PURE;

    /// Loads a resource signature from the render state notation parser.

    /// \param [in]  LoadInfo    - Render pass load info, see Diligent::LoadResourceSignatureInfo.
    /// \param [out] ppSignature - Address of the memory location where a pointer to the pipeline resource signature object will be stored.
    ///
    /// \remarks This method must be externally synchronized.
    VIRTUAL void METHOD(LoadResourceSignature)(THIS_
                                               const LoadResourceSignatureInfo REF LoadInfo,
                                               IPipelineResourceSignature**        ppSignature) PURE;

    /// Loads a render pass from the render state notation parser.

    /// \param [in]  LoadInfo     - Render pass load info, see Diligent::LoadRenderPassInfo.
    /// \param [out] ppRenderPass - Address of the memory location where a pointer to the loaded render pass object will be stored.
    ///
    /// \remarks This method must be externally synchronized.
    VIRTUAL void METHOD(LoadRenderPass)(THIS_
                                        const LoadRenderPassInfo REF LoadInfo,
                                        IRenderPass**                ppRenderPass) PURE;

    /// Loads a shader from the render state notation parser.

    /// \param [in]  LoadInfo - Shader load info, see Diligent::LoadShaderInfo.
    /// \param [out] ppShader - Address of the memory location where a pointer to the loaded shader object will be stored.
    ///
    /// \remarks This method must be externally synchronized.
    VIRTUAL void METHOD(LoadShader)(THIS_
                                    const LoadShaderInfo REF LoadInfo,
                                    IShader**                ppShader) PURE;

    /// Reloads all states.
    ///
    /// \return true if the states were reloaded successfully, and false otherwise.
    ///
    /// \note   This method requires that both render state notation parser
    ///         as well as the render state cache (if not null) support
    ///         state reloading.
    ///
    /// \remarks    Most of the states in the render state notation can be reloaded with the following
    ///             exceptions:
    ///             - Pipeline resource layouts and signatures can't be modified
    ///             - Shaders can be reloaded, but can't be replaced (e.g. a PSO can't use another shader after the reload)
    VIRTUAL bool METHOD(Reload)(THIS) PURE;
};
DILIGENT_END_INTERFACE

#include "../../../DiligentCore/Primitives/interface/UndefInterfaceHelperMacros.h"

#if DILIGENT_C_INTERFACE

// clang-format off
#    define IRenderStateNotationLoader_LoadPipelineState(This, ...)     CALL_IFACE_METHOD(RenderStateNotationLoader, LoadPipelineState,     This, __VA_ARGS__)
#    define IRenderStateNotationLoader_LoadResourceSignature(This, ...) CALL_IFACE_METHOD(RenderStateNotationLoader, LoadResourceSignature, This, __VA_ARGS__)
#    define IRenderStateNotationLoader_LoadRenderPass(This, ...)        CALL_IFACE_METHOD(RenderStateNotationLoader, LoadRenderPass,        This, __VA_ARGS__)
#    define IRenderStateNotationLoader_LoadShader(This, ...)            CALL_IFACE_METHOD(RenderStateNotationLoader, LoadShader,            This, __VA_ARGS__)
#    define IRenderStateNotationLoader_Reload(This)                     CALL_IFACE_METHOD(RenderStateNotationLoader, Reload,                This)
// clang-format on

#endif

#include "../../../DiligentCore/Primitives/interface/DefineGlobalFuncHelperMacros.h"

void DILIGENT_GLOBAL_FUNCTION(CreateRenderStateNotationLoader)(const RenderStateNotationLoaderCreateInfo REF CreateInfo,
                                                               IRenderStateNotationLoader**                  ppLoader);

#include "../../../DiligentCore/Primitives/interface/UndefGlobalFuncHelperMacros.h"

DILIGENT_END_NAMESPACE // namespace Diligent
