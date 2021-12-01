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
 /// Defines Diligent::IRenderStateNotationParser interface
#include "../../../DiligentCore/Graphics/GraphicsEngine/interface/RenderDevice.h"

DILIGENT_BEGIN_NAMESPACE(Diligent)

struct PipelineStateNotation 
{
    PipelineStateDesc PSODesc;
                                               
    PSO_CREATE_FLAGS  Flags                       DEFAULT_INITIALIZER(PSO_CREATE_FLAG_NONE);

    const Char**      ppResourceSignatureNames    DEFAULT_INITIALIZER(nullptr);

    Uint32            ResourceSignaturesNameCount DEFAULT_INITIALIZER(0);
};
typedef struct PipelineStateNotation PipelineStateNotation;


struct GraphicsPipelineNotation DILIGENT_DERIVE(PipelineStateNotation)

    GraphicsPipelineDesc Desc;

    const Char*         pRenderPassName  DEFAULT_INITIALIZER(nullptr);

    const Char*         pVSName          DEFAULT_INITIALIZER(nullptr);
                        
    const Char*         pPSName          DEFAULT_INITIALIZER(nullptr);
                        
    const Char*         pDSName          DEFAULT_INITIALIZER(nullptr);
                        
    const Char*         pHSName          DEFAULT_INITIALIZER(nullptr);
                        
    const Char*         pGSName          DEFAULT_INITIALIZER(nullptr);
                        
    const Char*         pASName          DEFAULT_INITIALIZER(nullptr);
                        
    const Char*         pMSName          DEFAULT_INITIALIZER(nullptr);
};
typedef struct GraphicsPipelineNotation GraphicsPipelineNotation;


struct ComputePipelineNotation DILIGENT_DERIVE(PipelineStateNotation)

    const Char* pCSName DEFAULT_INITIALIZER(nullptr);
};
typedef struct ComputePipelineNotation ComputePipelineNotation;


struct TilePipelineNotation DILIGENT_DERIVE(PipelineStateNotation)

    const Char* pTSName DEFAULT_INITIALIZER(nullptr);
};
typedef struct TilePipelineNotation TilePipelineNotation;


struct RTGeneralShaderGroupNotation 
{
    const Char* Name         DEFAULT_INITIALIZER(nullptr);

    const Char* pShaderName  DEFAULT_INITIALIZER(nullptr);
};
typedef struct RTGeneralShaderGroupNotation RTGeneralShaderGroupNotation;


struct RTTriangleHitShaderGroupNotation 
{ 
    const Char* Name                  DEFAULT_INITIALIZER(nullptr);

    const Char* pClosestHitShaderName DEFAULT_INITIALIZER(nullptr);

    const Char* pAnyHitShaderName     DEFAULT_INITIALIZER(nullptr);
};
typedef struct RTTriangleHitShaderGroupNotation RTTriangleHitShaderGroupNotation;


struct RTProceduralHitShaderGroupNotation 
{
    const Char* Name                    DEFAULT_INITIALIZER(nullptr);

    const Char* pIntersectionShaderName DEFAULT_INITIALIZER(nullptr);

    const Char* pClosestHitShaderName   DEFAULT_INITIALIZER(nullptr);

    const Char* pAnyHitShaderName       DEFAULT_INITIALIZER(nullptr);
};
typedef struct RTProceduralHitShaderGroupNotation RTProceduralHitShaderGroupNotation;


struct RayTracingPipelineNotation DILIGENT_DERIVE(PipelineStateNotation)

    RayTracingPipelineDesc                       Desc;

    const RTGeneralShaderGroupNotation*          pGeneralShaders          DEFAULT_INITIALIZER(nullptr);

    Uint32                                       GeneralShaderCount       DEFAULT_INITIALIZER(0);
    
    const RTTriangleHitShaderGroupNotation*      pTriangleHitShaders      DEFAULT_INITIALIZER(nullptr);
    
    Uint32                                       TriangleHitShaderCount   DEFAULT_INITIALIZER(0);
    
    const RTProceduralHitShaderGroupNotation*    pProceduralHitShaders    DEFAULT_INITIALIZER(nullptr);
    
    Uint32                                       ProceduralHitShaderCount DEFAULT_INITIALIZER(0);

    const char*                                  pShaderRecordName        DEFAULT_INITIALIZER(nullptr);

    Uint32                                       MaxAttributeSize         DEFAULT_INITIALIZER(0);

    Uint32                                       MaxPayloadSize           DEFAULT_INITIALIZER(0);
};
typedef struct RayTracingPipelineNotation RayTracingPipelineNotation;


struct RenderStateNotationParserInfo 
{
    Uint32 ResourceSignatureCount       DEFAULT_INITIALIZER(0);

    Uint32 ShaderCount                  DEFAULT_INITIALIZER(0);

    Uint32 RenderPassCount              DEFAULT_INITIALIZER(0);

    Uint32 GraphicsPipelineStateCount   DEFAULT_INITIALIZER(0);

    Uint32 ComputePipelineStateCount    DEFAULT_INITIALIZER(0);

    Uint32 RayTracingPipelineStateCount DEFAULT_INITIALIZER(0);

    Uint32 TilePipelineStateCount       DEFAULT_INITIALIZER(0);
};
typedef struct RenderStateNotationParserInfo RenderStateNotationParserInfo;


// {355AC9f7-5D9D-423D-AE35-80E0028DE17E}
static const INTERFACE_ID IID_RenderStateNotationParser = {0x355AC9F7, 0x5D9D, 0x423D, {0xAE, 0x35, 0x80, 0xE0, 0x02, 0x8D, 0xE1, 0x7E}};

#define DILIGENT_INTERFACE_NAME IRenderStateNotationParser
#include "../../../DiligentCore/Primitives/interface/DefineInterfaceHelperMacros.h"

#define RenderStateNotationParserrInclusiveMethods \
    IObjectInclusiveMethods;                          \
    IRenderStateNotationParser RenderStateNotationParser


DILIGENT_BEGIN_INTERFACE(IRenderStateNotationParser, IObject)
{
    VIRTUAL CONST GraphicsPipelineNotation* METHOD(GetGraphicsPipelineStateByName)(THIS_
                                                                                   const Char* Name) CONST PURE;

    VIRTUAL CONST ComputePipelineNotation* METHOD(GetComputePipelineStateByName)(THIS_
                                                                                 const Char* Name) CONST PURE;

    VIRTUAL CONST RayTracingPipelineNotation* METHOD(GetRayTracingPipelineStateByName)(THIS_
                                                                                       const Char* Name) CONST PURE;

    VIRTUAL CONST TilePipelineNotation* METHOD(GetTilePipelineStateByName)(THIS_
                                                                           const Char* Name) CONST PURE;

    VIRTUAL CONST PipelineResourceSignatureDesc* METHOD(GetResourceSignatureByName)(THIS_
                                                                                   const Char* Name) CONST PURE;

    VIRTUAL CONST ShaderCreateInfo* METHOD(GetShaderByName)(THIS_
                                                            const Char* Name) CONST PURE;

    VIRTUAL CONST RenderPassDesc*  METHOD(GetRenderPassByName)(THIS_
                                                               const Char* Name) CONST PURE;

    VIRTUAL CONST GraphicsPipelineNotation* METHOD(GetGraphicsPipelineStateByIndex)(THIS_
                                                                                    Uint32 Index) CONST PURE;

    VIRTUAL CONST ComputePipelineNotation* METHOD(GetComputePipelineStateByIndex)(THIS_
                                                                                  Uint32 Index) CONST PURE;

    VIRTUAL CONST RayTracingPipelineNotation* METHOD(GetRayTracingPipelineStateByIndex)(THIS_
                                                                                        Uint32 Index) CONST PURE;

    VIRTUAL CONST TilePipelineNotation* METHOD(GetTilePipelineStateByIndex)(THIS_
                                                                            Uint32 Index) CONST PURE;

    VIRTUAL CONST PipelineResourceSignatureDesc* METHOD(GetResourceSignatureByIndex)(THIS_
                                                                                     Uint32 Index) CONST PURE;

    VIRTUAL CONST ShaderCreateInfo* METHOD(GetShaderByIndex)(THIS_
                                                             Uint32 Index) CONST PURE;

    VIRTUAL CONST RenderPassDesc* METHOD(GetRenderPassByIndex)(THIS_
                                                               Uint32 Index) CONST PURE;

    VIRTUAL CONST RenderStateNotationParserInfo REF METHOD(GetInfo)(THIS_) CONST PURE;

};
DILIGENT_END_INTERFACE

#include "../../../DiligentCore/Primitives/interface/UndefInterfaceHelperMacros.h"

#if DILIGENT_C_INTERFACE

#    define IRenderStateNotationParser_GetGraphicsPipelineStateByName(This, ...)    CALL_IFACE_METHOD(RenderStateNotationParser, GetGraphicsPipelineStateByName,    This, __VA_ARGS__)
#    define IRenderStateNotationParser_GetComputePipelineStateByName(This, ...)     CALL_IFACE_METHOD(RenderStateNotationParser, GetComputePipelineStateByName,     This, __VA_ARGS__)
#    define IRenderStateNotationParser_GetRayTracingPipelineStateByName(This, ...)  CALL_IFACE_METHOD(RenderStateNotationParser, GetRayTracingPipelineStateByName,  This, __VA_ARGS__)
#    define IRenderStateNotationParser_GetTilePipelineStateByName(This, ...)        CALL_IFACE_METHOD(RenderStateNotationParser, GetTilePipelineStateByName,        This, __VA_ARGS__)
#    define IRenderStateNotationParser_GetResourceSignatureByName(This, ...)        CALL_IFACE_METHOD(RenderStateNotationParser, GetResourceSignatureByName,        This, __VA_ARGS__)
#    define IRenderStateNotationParser_GetShaderByName(This, ...)                   CALL_IFACE_METHOD(RenderStateNotationParser, GetShaderByName,                   This, __VA_ARGS__)
#    define IRenderStateNotationParser_GetRenderPassByName(This, ...)               CALL_IFACE_METHOD(RenderStateNotationParser, GetRenderPassByName,               This, __VA_ARGS__)

#    define IRenderStateNotationParser_GetGraphicsPipelineStateByIndex(This, ...)   CALL_IFACE_METHOD(RenderStateNotationParser, GetGraphicsPipelineStateByIndex,   This, __VA_ARGS__)
#    define IRenderStateNotationParser_GetComputePipelineStateByIndex(This, ...)    CALL_IFACE_METHOD(RenderStateNotationParser, GetComputePipelineStateByIndex,    This, __VA_ARGS__)
#    define IRenderStateNotationParser_GetRayTracingPipelineStateByIndex(This, ...) CALL_IFACE_METHOD(RenderStateNotationParser, GetRayTracingPipelineStateByIndex, This, __VA_ARGS__)
#    define IRenderStateNotationParser_GetTilePipelineStateByIndex(This, ...)       CALL_IFACE_METHOD(RenderStateNotationParser, GetTilePipelineStateByIndex,       This, __VA_ARGS__)
#    define IRenderStateNotationParser_GetResourceSignatureByIndex(This, ...)       CALL_IFACE_METHOD(RenderStateNotationParser, GetResourceSignatureByIndex,       This, __VA_ARGS__)
#    define IRenderStateNotationParser_GetShaderByIndex(This, ...)                  CALL_IFACE_METHOD(RenderStateNotationParser, GetShaderByIndex,                  This, __VA_ARGS__)
#    define IRenderStateNotationParser_GetRenderPassByIndex(This, ...)              CALL_IFACE_METHOD(RenderStateNotationParser, GetRenderPassByIndex,              This, __VA_ARGS__)

#    define IRenderStateNotationParser_GetRenderPassByIndex(This, ...)              CALL_IFACE_METHOD(RenderStateNotationParser, GetInfo,                           This)

#endif

#include "../../../DiligentCore/Primitives/interface/DefineGlobalFuncHelperMacros.h"

void DILIGENT_GLOBAL_FUNCTION(CreateRenderStateNotationParserFromFile)(const Char* FilePath,
                                                                      IRenderStateNotationParser** ppParser);

void DILIGENT_GLOBAL_FUNCTION(CreateRenderStateNotationParserFromString)(const Char* StrData,
                                                                         IRenderStateNotationParser** ppParser);

#include "../../../DiligentCore/Primitives/interface/UndefGlobalFuncHelperMacros.h"

DILIGENT_END_NAMESPACE // namespace Diligent
