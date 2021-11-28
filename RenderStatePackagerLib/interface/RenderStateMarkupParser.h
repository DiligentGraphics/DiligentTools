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
 /// Defines Diligent::IDeviceObjectDescriptorParser interface
#include "../../../DiligentCore/Graphics/GraphicsEngine/interface/RenderDevice.h"

DILIGENT_BEGIN_NAMESPACE(Diligent)


struct GraphicsPipelineMarkup
{
    const Char* pRenderPass DEFAULT_INITIALIZER(nullptr);

#if DILIGENT_CPP_INTERFACE
    constexpr bool operator == (const GraphicsPipelineMarkup& RHS) const
    {   
        //TODO
        return false;
    }
#endif

};
typedef struct GraphicsPipelineMarkup GraphicsPipelineMarkup;

struct PipelineStateCreateMarkup
{
    const Char** ppResourceSignatures DEFAULT_INITIALIZER(nullptr);

    Uint32 ResourceSignaturesCount    DEFAULT_INITIALIZER(0);  

#if DILIGENT_CPP_INTERFACE
    constexpr bool operator == (const PipelineStateCreateMarkup& RHS) const
    {
        //TODO
        return false;
    }
#endif  
};
typedef struct PipelineStateCreateMarkup PipelineStateCreateMarkup;

struct GraphicsPipelineStateCreateMarkup DILIGENT_DERIVE(PipelineStateCreateMarkup)
    GraphicsPipelineMarkup GraphicsPipeline DEFAULT_INITIALIZER({});

    const Char* pVS                         DEFAULT_INITIALIZER(nullptr);
                                              
    const Char* pPS                         DEFAULT_INITIALIZER(nullptr);
                                              
    const Char* pDS                         DEFAULT_INITIALIZER(nullptr);
                                              
    const Char* pHS                         DEFAULT_INITIALIZER(nullptr);
                                              
    const Char* pGS                         DEFAULT_INITIALIZER(nullptr);
                                              
    const Char* pAS                         DEFAULT_INITIALIZER(nullptr);
                                              
    const Char* pMS                         DEFAULT_INITIALIZER(nullptr);

#if DILIGENT_CPP_INTERFACE
    constexpr bool operator == (const GraphicsPipelineStateCreateMarkup& RHS) const
    {
        //TODO
        return false;
    }
#endif
};
typedef struct GraphicsPipelineStateCreateMarkup GraphicsPipelineStateCreateMarkup;

struct ComputePipelineStateCreateMarkup DILIGENT_DERIVE(PipelineStateCreateMarkup)
    const Char* pCS DEFAULT_INITIALIZER(nullptr);

#if DILIGENT_CPP_INTERFACE
    constexpr bool operator == (const ComputePipelineStateCreateMarkup& RHS) const
    {
        //TODO
        return false;
    }
#endif
};
typedef struct ComputePipelineStateCreateMarkup ComputePipelineStateCreateMarkup;

struct TilePipelineStateCreateMarkup DILIGENT_DERIVE(PipelineStateCreateMarkup)
    const Char* pTS DEFAULT_INITIALIZER(nullptr);
    
#if DILIGENT_CPP_INTERFACE
    constexpr bool operator == (const TilePipelineStateCreateMarkup& RHS) const
    {
        //TODO
        return false;
    }
#endif
};
typedef struct TilePipelineStateCreateMarkup TilePipelineStateCreateMarkup;

struct RayTracingGeneralShaderGroupMarkup
{
    const Char* pShader  DEFAULT_INITIALIZER(nullptr);

#if DILIGENT_CPP_INTERFACE
    constexpr bool operator == (const TilePipelineStateCreateMarkup& RHS) const
    {
        //TODO
        return false;
    }
#endif
};
typedef struct TilePipelineStateCreateMarkup TilePipelineStateCreateMarkup;


struct RayTracingTriangleHitShaderGroupMarkup
{
    const Char* pClosestHitShader DEFAULT_INITIALIZER(nullptr);

    const Char* pAnyHitShader     DEFAULT_INITIALIZER(nullptr);

#if DILIGENT_CPP_INTERFACE
    constexpr bool operator == (const RayTracingTriangleHitShaderGroupMarkup& RHS) const
    {
        //TODO
        return false;
    }
#endif
};
typedef struct RayTracingTriangleHitShaderGroupMarkup RayTracingTriangleHitShaderGroupMarkup;


struct RayTracingProceduralHitShaderGroupMarkup
{
    const Char* pIntersectionShader DEFAULT_INITIALIZER(nullptr);

    const Char* pClosestHitShader   DEFAULT_INITIALIZER(nullptr);

    const Char* pAnyHitShader       DEFAULT_INITIALIZER(nullptr);

#if DILIGENT_CPP_INTERFACE
    constexpr bool operator == (const RayTracingProceduralHitShaderGroupMarkup& RHS) const
    {
        //TODO
        return false;
    }
#endif
};
typedef struct RayTracingProceduralHitShaderGroupMarkup RayTracingProceduralHitShaderGroupMarkup;

struct RayTracingPipelineStateCreateMarkup DILIGENT_DERIVE(PipelineStateCreateMarkup)
    const RayTracingGeneralShaderGroupMarkup*       pGeneralShaders          DEFAULT_INITIALIZER(nullptr);

    Uint32                                          GeneralShaderCount       DEFAULT_INITIALIZER(0);

    const RayTracingTriangleHitShaderGroupMarkup*   pTriangleHitShaders      DEFAULT_INITIALIZER(nullptr);

    Uint32                                          TriangleHitShaderCount   DEFAULT_INITIALIZER(0);

    const RayTracingProceduralHitShaderGroupMarkup* pProceduralHitShaders    DEFAULT_INITIALIZER(nullptr);

    Uint32                                          ProceduralHitShaderCount DEFAULT_INITIALIZER(0);

#if DILIGENT_CPP_INTERFACE
    constexpr bool operator == (const RayTracingPipelineStateCreateMarkup& RHS) const
    {
        //TODO
        return false;
    }
#endif
};
typedef struct RayTracingPipelineStateCreateMarkup RayTracingPipelineStateCreateMarkup;

// {355AC9f7-5D9D-423D-AE35-80E0028DE17E}
static const INTERFACE_ID IID_DeviceObjectDescriptorParser = {0x355AC9F7, 0x5D9D, 0x423D, {0xAE, 0x35, 0x80, 0xE0, 0x02, 0x8D, 0xE1, 0x7E}};

#define DILIGENT_INTERFACE_NAME IRenderStateMarkupParser
#include "../../../DiligentCore/Primitives/interface/DefineInterfaceHelperMacros.h"

#define DeviceObjectDescriptionParserInclusiveMethods \
    IObjectInclusiveMethods;                          \
    IRenderStateMarkupParser RenderStateMarkupParser


DILIGENT_BEGIN_INTERFACE(IRenderStateMarkupParser, IObject)
{
    VIRTUAL Bool METHOD(GetGraphicsPipelineStateByName)(THIS_
                                                        const Char* Name,
                                                        GraphicsPipelineStateCreateInfo REF CreateInfo,
                                                        GraphicsPipelineStateCreateMarkup REF CreateMarkup) CONST PURE;

    VIRTUAL Bool METHOD(GetComputePipelineStateByName)(THIS_
                                                       const Char* Name,
                                                       ComputePipelineStateCreateInfo REF CreateInfo,
                                                       ComputePipelineStateCreateMarkup REF CreateMarkup) CONST PURE;

    VIRTUAL Bool METHOD(GetRayTracingPipelineStateByName)(THIS_
                                                          const Char* Name,
                                                          RayTracingPipelineStateCreateInfo REF CreateInfo,
                                                          RayTracingPipelineStateCreateMarkup REF CreateMarkup) CONST PURE;

    VIRTUAL Bool METHOD(GetTilePipelineStateByName)(THIS_
                                                    const Char* Name,
                                                    TilePipelineStateCreateInfo REF CreateInfo,
                                                    TilePipelineStateCreateMarkup REF CreateMarkup) CONST PURE;

    VIRTUAL Bool METHOD(GetResourceSignatureByName)(THIS_
                                                    const Char* Name,
                                                    PipelineResourceSignatureDesc REF CreateInfo) CONST PURE;

    VIRTUAL Bool METHOD(GetShaderByName)(THIS_
                                         const Char* Name,
                                         ShaderCreateInfo REF CreateInfo) CONST PURE;

    VIRTUAL Bool METHOD(GetRenderPassByName)(THIS_
                                             const Char* Name,
                                             RenderPassDesc REF CreateInfo) CONST PURE;

    VIRTUAL Bool METHOD(GetGraphicsPipelineStateByIndex)(THIS_
                                                         Uint32 Index,
                                                         GraphicsPipelineStateCreateInfo REF CreateInfo,
                                                         GraphicsPipelineStateCreateMarkup REF CreateMarkup) CONST PURE;

    VIRTUAL Bool METHOD(GetComputePipelineStateByIndex)(THIS_
                                                        Uint32 Index,
                                                        ComputePipelineStateCreateInfo REF CreateInfo,
                                                        ComputePipelineStateCreateMarkup REF CreateMarkup) CONST PURE;

    VIRTUAL Bool METHOD(GetRayTracingPipelineStateByIndex)(THIS_
                                                           Uint32 Index,
                                                           RayTracingPipelineStateCreateInfo REF CreateInfo,
                                                           RayTracingPipelineStateCreateMarkup REF CreateMarkup) CONST PURE;

    VIRTUAL Bool METHOD(GetTilePipelineStateByIndex)(THIS_
                                                     Uint32 Index,
                                                     TilePipelineStateCreateInfo REF CreateInfo,
                                                     TilePipelineStateCreateMarkup REF CreateMarkup) CONST PURE;

    VIRTUAL Bool METHOD(GetResourceSignatureByIndex)(THIS_
                                                     Uint32 Index,
                                                     PipelineResourceSignatureDesc REF CreateInfo) CONST PURE;

    VIRTUAL Bool METHOD(GetShaderByIndex)(THIS_
                                          Uint32 Index,
                                          ShaderCreateInfo REF CreateInfo) CONST PURE;

    VIRTUAL Bool METHOD(GetRenderPassByIndex)(THIS_
                                              Uint32 Index,
                                              RenderPassDesc REF CreateInfo) CONST PURE;

    VIRTUAL Uint32 METHOD(GetGraphicsPipelineStateCount)(THIS_) CONST PURE;

    VIRTUAL Uint32 METHOD(GetComputePipelineStateCount)(THIS_) CONST PURE;
    
    VIRTUAL Uint32 METHOD(GetRayTracingPipelineStateCount)(THIS_) CONST PURE;

    VIRTUAL Uint32 METHOD(GetTilePipelineStateCount)(THIS_) CONST PURE;

    VIRTUAL Uint32 METHOD(GetResourceSignatureCount)(THIS_) CONST PURE;

    VIRTUAL Uint32 METHOD(GetShaderCount)(THIS_) CONST PURE;

    VIRTUAL Uint32 METHOD(GetRenderPassCount)(THIS_) CONST PURE;
};
DILIGENT_END_INTERFACE

#include "../../../DiligentCore/Primitives/interface/UndefInterfaceHelperMacros.h"

#if DILIGENT_C_INTERFACE

#    define IRenderStateMarkupParser_GetGraphicsPipelineStateByName(This, ...)    CALL_IFACE_METHOD(RenderStateMarkupParser, GetGraphicsPipelineStateByName,    This, __VA_ARGS__)
#    define IRenderStateMarkupParser_GetComputePipelineStateByName(This, ...)     CALL_IFACE_METHOD(RenderStateMarkupParser, GetComputePipelineStateByName,     This, __VA_ARGS__)
#    define IRenderStateMarkupParser_GetRayTracingPipelineStateByName(This, ...)  CALL_IFACE_METHOD(RenderStateMarkupParser, GetRayTracingPipelineStateByName,  This, __VA_ARGS__)
#    define IRenderStateMarkupParser_GetTilePipelineStateByName(This, ...)        CALL_IFACE_METHOD(RenderStateMarkupParser, GetTilePipelineStateByName,        This, __VA_ARGS__)
#    define IRenderStateMarkupParser_GetResourceSignatureByName(This, ...)        CALL_IFACE_METHOD(RenderStateMarkupParser, GetResourceSignatureByName,        This, __VA_ARGS__)
#    define IRenderStateMarkupParser_GetShaderByName(This, ...)                   CALL_IFACE_METHOD(RenderStateMarkupParser, GetShaderByName,                   This, __VA_ARGS__)
#    define IRenderStateMarkupParser_GetRenderPassByName(This, ...)               CALL_IFACE_METHOD(RenderStateMarkupParser, GetRenderPassByName,               This, __VA_ARGS__)

#    define IRenderStateMarkupParser_GetGraphicsPipelineStateByIndex(This, ...)   CALL_IFACE_METHOD(RenderStateMarkupParser, GetGraphicsPipelineStateByIndex,   This, __VA_ARGS__)
#    define IRenderStateMarkupParser_GetComputePipelineStateByIndex(This, ...)    CALL_IFACE_METHOD(RenderStateMarkupParser, GetComputePipelineStateByIndex,    This, __VA_ARGS__)
#    define IRenderStateMarkupParser_GetRayTracingPipelineStateByIndex(This, ...) CALL_IFACE_METHOD(RenderStateMarkupParser, GetRayTracingPipelineStateByIndex, This, __VA_ARGS__)
#    define IRenderStateMarkupParser_GetTilePipelineStateByIndex(This, ...)       CALL_IFACE_METHOD(RenderStateMarkupParser, GetTilePipelineStateByIndex,       This, __VA_ARGS__)
#    define IRenderStateMarkupParser_GetResourceSignatureByIndex(This, ...)       CALL_IFACE_METHOD(RenderStateMarkupParser, GetResourceSignatureByIndex,       This, __VA_ARGS__)
#    define IRenderStateMarkupParser_GetShaderByIndex(This, ...)                  CALL_IFACE_METHOD(RenderStateMarkupParser, GetShaderByIndex,                  This, __VA_ARGS__)
#    define IRenderStateMarkupParser_GetRenderPassByIndex(This, ...)              CALL_IFACE_METHOD(RenderStateMarkupParser, GetRenderPassByIndex,              This, __VA_ARGS__)

#    define IRenderStateMarkupParser_GetGraphicsPipelineStateCount(This, ...)     CALL_IFACE_METHOD(RenderStateMarkupParser, GetGraphicsPipelineStateCount,     This, __VA_ARGS__)
#    define IRenderStateMarkupParser_GetComputePipelineStateCount(This, ...)      CALL_IFACE_METHOD(RenderStateMarkupParser, GetComputePipelineStateCount,      This, __VA_ARGS__)
#    define IRenderStateMarkupParser_GetRayTracingPipelineStateCount(This, ...)   CALL_IFACE_METHOD(RenderStateMarkupParser, GetTilePipelineStateCount,         This, __VA_ARGS__)
#    define IRenderStateMarkupParser_GetTilePipelineStateCount(This, ...)         CALL_IFACE_METHOD(RenderStateMarkupParser, GetTilePipelineStateCount,         This, __VA_ARGS__)
#    define IRenderStateMarkupParser_GetResourceSignatureCount(This, ...)         CALL_IFACE_METHOD(RenderStateMarkupParser, GetResourceSignatureCount,         This, __VA_ARGS__)
#    define IRenderStateMarkupParser_GetShaderCount(This, ...)                    CALL_IFACE_METHOD(RenderStateMarkupParser, GetShaderCount,                    This, __VA_ARGS__)
#    define IRenderStateMarkupParser_GetRenderPassCount(This, ...)                CALL_IFACE_METHOD(RenderStateMarkupParser, GetRenderPassCount,                This, __VA_ARGS__)

#endif

#include "../../../DiligentCore/Primitives/interface/DefineGlobalFuncHelperMacros.h"

void DILIGENT_GLOBAL_FUNCTION(CreateRenderStateMarkupParserFromFile)(const Char* FilePath,
                                                                     IRenderStateMarkupParser** ppParser);

void DILIGENT_GLOBAL_FUNCTION(CreateRenderStateMarkupParserFromString)(const Char* StrData,
                                                                       IRenderStateMarkupParser** ppParser);

#include "../../../DiligentCore/Primitives/interface/UndefGlobalFuncHelperMacros.h"

DILIGENT_END_NAMESPACE // namespace Diligent
