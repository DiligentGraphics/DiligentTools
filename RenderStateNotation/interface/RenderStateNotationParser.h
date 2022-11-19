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
/// Defines Diligent::IRenderStateNotationParser interface
#include "../../../DiligentCore/Graphics/GraphicsEngine/interface/RenderDevice.h"

DILIGENT_BEGIN_NAMESPACE(Diligent)

/// Pipeline state notation.

/// \note
///     This structure mirrors the PipelineStateCreateInfo struct, but
///     uses names to identify resource signatures used by the pipeline.
struct PipelineStateNotation 
{
    /// Pipeline state description.
    PipelineStateDesc PSODesc;

    /// Pipeline state creation flags, see Diligent::PSO_CREATE_FLAGS.
    PSO_CREATE_FLAGS  Flags                       DEFAULT_INITIALIZER(PSO_CREATE_FLAG_NONE);

    /// A pointer to an array of resource signature names.
    const Char**      ppResourceSignatureNames    DEFAULT_INITIALIZER(nullptr);

    /// The number of resource signature names in ppResourceSignatureNames array.
    Uint32            ResourceSignaturesNameCount DEFAULT_INITIALIZER(0);

    // Required to ensure correct memory layout for inherited structs on clang/gcc.
    Uint32            _Padding                    DEFAULT_INITIALIZER(~0u);

#if DILIGENT_CPP_INTERFACE
    /// Comparison operator tests if two structures are equivalent
    bool operator == (const PipelineStateNotation& RHS) const
    {
        if (!(PSODesc == RHS.PSODesc) || !(Flags == RHS.Flags) || !(ResourceSignaturesNameCount == RHS.ResourceSignaturesNameCount))
            return false;

        for (Uint32 SignatureID = 0; SignatureID < ResourceSignaturesNameCount; SignatureID++)
            if (!SafeStrEqual(ppResourceSignatureNames[SignatureID], RHS.ppResourceSignatureNames[SignatureID]))
                return false;

        return true;
    }
    bool operator != (const PipelineStateNotation& RHS) const
    {
        return !(*this == RHS);
    }
#endif
};
typedef struct PipelineStateNotation PipelineStateNotation;

/// Graphics pipeline state notation.

/// \note
///     This structure mirrors the GraphicsPipelineStateCreateInfo struct, but
///     uses names to identify render pass and shaders used by the pipeline.
struct GraphicsPipelineNotation DILIGENT_DERIVE(PipelineStateNotation)

    /// Graphics pipeline state description.
    GraphicsPipelineDesc Desc;

    /// Render pass name.
    const Char*         pRenderPassName  DEFAULT_INITIALIZER(nullptr);

    /// Vertex shader name.
    const Char*         pVSName          DEFAULT_INITIALIZER(nullptr);

    /// Pixel shader name.
    const Char*         pPSName          DEFAULT_INITIALIZER(nullptr);

    /// Domain shader name.
    const Char*         pDSName          DEFAULT_INITIALIZER(nullptr);

    /// Hull shader name.
    const Char*         pHSName          DEFAULT_INITIALIZER(nullptr);

    /// Geometry shader name.
    const Char*         pGSName          DEFAULT_INITIALIZER(nullptr);

    /// Amplification shader name.
    const Char*         pASName          DEFAULT_INITIALIZER(nullptr);

    /// Mesh shader name.
    const Char*         pMSName          DEFAULT_INITIALIZER(nullptr);

#if DILIGENT_CPP_INTERFACE
    /// Comparison operator tests if two structures are equivalent
    bool operator == (const GraphicsPipelineNotation& RHS) const 
    {
        if (!(static_cast<const PipelineStateNotation&>(*this) == static_cast<const PipelineStateNotation&>(RHS)))
            return false;

        return Desc == RHS.Desc &&
               SafeStrEqual(pRenderPassName, RHS.pRenderPassName) &&
               SafeStrEqual(pVSName, RHS.pVSName) &&
               SafeStrEqual(pPSName, RHS.pPSName) &&
               SafeStrEqual(pDSName, RHS.pDSName) &&
               SafeStrEqual(pHSName, RHS.pHSName) &&
               SafeStrEqual(pGSName, RHS.pGSName) &&
               SafeStrEqual(pASName, RHS.pASName) &&
               SafeStrEqual(pMSName, RHS.pMSName);
    }
    bool operator != (const GraphicsPipelineNotation& RHS) const
    {
        return !(*this == RHS);
    }
#endif
};
typedef struct GraphicsPipelineNotation GraphicsPipelineNotation;

/// Compute pipeline state notation.

/// \note
///     This structure mirrors the ComputePipelineStateCreateInfo struct, but
///     uses a name to identify the compute shader used by the pipeline.
struct ComputePipelineNotation DILIGENT_DERIVE(PipelineStateNotation)

    /// Compute shader name.
    const Char* pCSName DEFAULT_INITIALIZER(nullptr);

#if DILIGENT_CPP_INTERFACE
    /// Comparison operator tests if two structures are equivalent
    bool operator == (const ComputePipelineNotation& RHS) const 
    {
        if (!(static_cast<const PipelineStateNotation &>(*this) == static_cast<const PipelineStateNotation&>(RHS)))
            return false;

        return SafeStrEqual(pCSName, RHS.pCSName);           
    }
    bool operator != (const ComputePipelineNotation& RHS) const
    {
        return !(*this == RHS);
    }
#endif
};
typedef struct ComputePipelineNotation ComputePipelineNotation;

/// Tile pipeline state notation.

/// \note
///     This structure mirrors the TilePipelineStateCreateInfo struct, but
///     uses a name to identify the tile shader used by the pipeline.
struct TilePipelineNotation DILIGENT_DERIVE(PipelineStateNotation)

    /// Tile shader name.
    const Char* pTSName DEFAULT_INITIALIZER(nullptr);

#if DILIGENT_CPP_INTERFACE
    /// Comparison operator tests if two structures are equivalent
    bool operator == (const TilePipelineNotation& RHS) const 
    {
        if (!(static_cast<const PipelineStateNotation&>(*this) == static_cast<const PipelineStateNotation&>(RHS)))
            return false;

        return SafeStrEqual(pTSName, RHS.pTSName);
    }
    bool operator != (const TilePipelineNotation& RHS) const
    {
        return !(*this == RHS);
    }
#endif
};
typedef struct TilePipelineNotation TilePipelineNotation;

/// Ray tracing general shader group notation.

/// \note
///     This structure mirrors the RayTracingGeneralShaderGroup struct, but
///     uses a name to identify the shader used by the group.
struct RTGeneralShaderGroupNotation 
{
    /// Unique group name.
    const Char* Name         DEFAULT_INITIALIZER(nullptr);

    // Shader name.
    const Char* pShaderName  DEFAULT_INITIALIZER(nullptr);

#if DILIGENT_CPP_INTERFACE
    /// Comparison operator tests if two structures are equivalent
    bool operator == (const RTGeneralShaderGroupNotation& RHS) const
    {
        return SafeStrEqual(Name, RHS.Name) &&
               SafeStrEqual(pShaderName, RHS.pShaderName);
    }
    bool operator != (const RTGeneralShaderGroupNotation& RHS) const
    {
        return !(*this == RHS);
    }
#endif
};
typedef struct RTGeneralShaderGroupNotation RTGeneralShaderGroupNotation;

/// Ray tracing triangle hit shader group notation.

/// \note
///     This structure mirrors the RayTracingTriangleHitShaderGroup struct, but
///     uses names to identify the shaders used by the group.
struct RTTriangleHitShaderGroupNotation 
{
    /// Unique group name.
    const Char* Name                  DEFAULT_INITIALIZER(nullptr);

    /// Closest hit shader name.
    const Char* pClosestHitShaderName DEFAULT_INITIALIZER(nullptr);

    /// Any-hit shader name.
    const Char* pAnyHitShaderName     DEFAULT_INITIALIZER(nullptr);

#if DILIGENT_CPP_INTERFACE
    /// Comparison operator tests if two structures are equivalent
    bool operator == (const RTTriangleHitShaderGroupNotation& RHS) const 
    {
        return SafeStrEqual(Name, RHS.Name) &&
               SafeStrEqual(pClosestHitShaderName, RHS.pClosestHitShaderName) &&
               SafeStrEqual(pAnyHitShaderName, RHS.pAnyHitShaderName);
    }
    bool operator != (const RTTriangleHitShaderGroupNotation& RHS) const
    {
        return !(*this == RHS);
    }
#endif
};
typedef struct RTTriangleHitShaderGroupNotation RTTriangleHitShaderGroupNotation;

/// Ray tracing procedural hit shader group notation.

/// \note
///     This structure mirrors the RayTracingTriangleHitShaderGroup struct, but
///     uses names to identify the shaders used by the group.
struct RTProceduralHitShaderGroupNotation 
{
    /// Unique group name.
    const Char* Name                    DEFAULT_INITIALIZER(nullptr);

    /// Intersection shader name.
    const Char* pIntersectionShaderName DEFAULT_INITIALIZER(nullptr);

    /// Closest hit shader name.
    const Char* pClosestHitShaderName   DEFAULT_INITIALIZER(nullptr);

    /// Any-hit shader name.
    const Char* pAnyHitShaderName       DEFAULT_INITIALIZER(nullptr);

#if DILIGENT_CPP_INTERFACE
    /// Comparison operator tests if two structures are equivalent
    bool operator == (const RTProceduralHitShaderGroupNotation& RHS) const 
    {
        return SafeStrEqual(Name, RHS.Name) &&
               SafeStrEqual(pIntersectionShaderName, RHS.pIntersectionShaderName) &&
               SafeStrEqual(pClosestHitShaderName, RHS.pClosestHitShaderName) &&
               SafeStrEqual(pAnyHitShaderName, RHS.pAnyHitShaderName);
    }
    bool operator != (const RTProceduralHitShaderGroupNotation& RHS) const
    {
        return !(*this == RHS);
    }
#endif
};
typedef struct RTProceduralHitShaderGroupNotation RTProceduralHitShaderGroupNotation;

/// Ray tracing pipeline state notation.

/// \note
///     This structure mirrors the RayTracingPipelineStateCreateInfo struct, but
///     uses group notations to identify ray tracing groups.
struct RayTracingPipelineNotation DILIGENT_DERIVE(PipelineStateNotation)

    /// Ray tracing pipeline description.
    RayTracingPipelineDesc                       RayTracingPipeline;

    /// A pointer to an array of GeneralShaderCount RTGeneralShaderGroupNotation structures that contain shader group description.
    const RTGeneralShaderGroupNotation*          pGeneralShaders          DEFAULT_INITIALIZER(nullptr);

    /// The number of general shader groups in pGeneralShaders array.
    Uint32                                       GeneralShaderCount       DEFAULT_INITIALIZER(0);

    /// A pointer to an array of TriangleHitShaderCount RTTriangleHitShaderGroupNotation structures that contain shader group description.
    const RTTriangleHitShaderGroupNotation*      pTriangleHitShaders      DEFAULT_INITIALIZER(nullptr);

    /// The number of triangle hit shader groups in pTriangleHitShaders array.
    Uint32                                       TriangleHitShaderCount   DEFAULT_INITIALIZER(0);

    /// A pointer to an array of ProceduralHitShaderCount RTProceduralHitShaderGroupNotation structures that contain shader group description.
    const RTProceduralHitShaderGroupNotation*    pProceduralHitShaders    DEFAULT_INITIALIZER(nullptr);

    /// The number of procedural shader groups in pProceduralHitShaders array.
    Uint32                                       ProceduralHitShaderCount DEFAULT_INITIALIZER(0);

    /// The name of the constant buffer that will be used by the local root signature.
    const char*                                  pShaderRecordName        DEFAULT_INITIALIZER(nullptr);

    /// The maximum hit shader attribute size in bytes.
    Uint32                                       MaxAttributeSize         DEFAULT_INITIALIZER(0);

    /// The maximum payload size in bytes.
    Uint32                                       MaxPayloadSize           DEFAULT_INITIALIZER(0);

#if DILIGENT_CPP_INTERFACE
    /// Comparison operator tests if two structures are equivalent
    bool operator == (const RayTracingPipelineNotation& RHS) const
    {
        if (!(static_cast<const PipelineStateNotation&>(*this) == static_cast<const PipelineStateNotation&>(RHS)))
            return false;

        if (!(RayTracingPipeline == RHS.RayTracingPipeline) ||
            !(GeneralShaderCount == RHS.GeneralShaderCount) || 
            !(TriangleHitShaderCount == RHS.TriangleHitShaderCount) ||
            !(ProceduralHitShaderCount == RHS.ProceduralHitShaderCount) ||
            !(MaxAttributeSize == RHS.MaxAttributeSize) ||
            !(MaxPayloadSize == RHS.MaxPayloadSize) || 
            !SafeStrEqual(pShaderRecordName, RHS.pShaderRecordName))
            return false;

        for (Uint32 GroupID = 0; GroupID < GeneralShaderCount; GroupID++)
            if (!(pGeneralShaders[GroupID] == RHS.pGeneralShaders[GroupID]))
                return false;

        for (Uint32 GroupID = 0; GroupID < TriangleHitShaderCount; GroupID++)
            if (!(pTriangleHitShaders[GroupID] == RHS.pTriangleHitShaders[GroupID]))
                return false;

        for (Uint32 GroupID = 0; GroupID < ProceduralHitShaderCount; GroupID++)
            if (!(pProceduralHitShaders[GroupID] == RHS.pProceduralHitShaders[GroupID]))
                return false;

        return true;
    }
    bool operator != (const RayTracingPipelineNotation& RHS) const
    {
        return !(*this == RHS);
    }
#endif
};
typedef struct RayTracingPipelineNotation RayTracingPipelineNotation;

/// Render state notation parser info.
struct RenderStateNotationParserInfo 
{
    /// The number of parsed resource signatures.
    Uint32 ResourceSignatureCount DEFAULT_INITIALIZER(0);

    /// The number of parsed shaders.
    Uint32 ShaderCount            DEFAULT_INITIALIZER(0);

    /// The number of parsed render passes.
    Uint32 RenderPassCount        DEFAULT_INITIALIZER(0);

    /// The number of parsed pipeline states.
    Uint32 PipelineStateCount     DEFAULT_INITIALIZER(0);
};
typedef struct RenderStateNotationParserInfo RenderStateNotationParserInfo;

/// Render state notation parser initialization information.
struct RenderStateNotationParserCreateInfo 
{
    /// Whether to enable state reloading with IRenderStateNotationParser::Reload() method.
    bool EnableReload DEFAULT_INITIALIZER(false);
};
typedef struct RenderStateNotationParserCreateInfo RenderStateNotationParserCreateInfo;

// clang-format on

// {355AC9f7-5D9D-423D-AE35-80E0028DE17E}
static const INTERFACE_ID IID_RenderStateNotationParser = {0x355AC9F7, 0x5D9D, 0x423D, {0xAE, 0x35, 0x80, 0xE0, 0x02, 0x8D, 0xE1, 0x7E}};

#define DILIGENT_INTERFACE_NAME IRenderStateNotationParser
#include "../../../DiligentCore/Primitives/interface/DefineInterfaceHelperMacros.h"

#define IRenderStateNotationParserInclusiveMethods \
    IObjectInclusiveMethods;                       \
    IRenderStateNotationParser RenderStateNotationParser

// clang-format off

/// Render state notation parser interface.
DILIGENT_BEGIN_INTERFACE(IRenderStateNotationParser, IObject)
{
    /// Parses a render state notation file.

    /// \param [in] FilePath       - Render state notation file path.
    /// \param [in] pStreamFactory - The factory that is used to load the source file if FilePath is not null.
    ///                              It is also used to create additional input streams for import files.
    /// \param [in] pReloadFactory - An optional factory to use for state reloading.
    ///                              If null, pStreamFactory will be used when Reload() method is called.
    ///
    /// \return
    /// - True if the file was parsed successfully.
    /// - False otherwise.
    ///
    /// \remarks This method must be externally synchronized.
    VIRTUAL Bool METHOD(ParseFile)(THIS_
                                   const Char*                      FilePath,
                                   IShaderSourceInputStreamFactory* pStreamFactory,
                                   IShaderSourceInputStreamFactory* pReloadFactory DEFAULT_VALUE(nullptr)) PURE;

    /// Parses a render state notation string.

    /// \param [in] Source         - A pointer to the render state notation string to parse.
    /// \param [in] Length         - Length of the source data string.
    ///                              When Source is not a null-terminated string, this member
    ///                              should be used to specify the length of the source data.
    ///                              If Length is zero, the source data is assumed to be null-terminated.
    ///
    /// \param [in] pStreamFactory - The factory that is used to load the source file if FilePath is not null.
    ///                              It is also used to create additional input streams for import files.
    /// \param [in] pReloadFactory - An optional factory to use for state reloading.
    ///                              If null, pStreamFactory will be used when Reload() method is called.
    ///
    /// \return
    /// - True if the string was parsed successfully.
    /// - False otherwise.
    /// \remarks This method must be externally synchronized.
    VIRTUAL Bool METHOD(ParseString)(THIS_
                                     const Char*                      Source,
                                     Uint32                           Length,
                                     IShaderSourceInputStreamFactory* pStreamFactory,
                                     IShaderSourceInputStreamFactory* pReloadFactory DEFAULT_VALUE(nullptr)) PURE;

    /// Returns the pipeline state notation by its name. If the resource is not found, returns nullptr.

    /// \param [in] Name         - Name of the PSO.
    /// \param [in] PipelineType - Pipeline state type.
    /// \return Const pointer to the PipelineStateNotation structure, see Diligent::PipelineStateNotation.
    ///
    /// \remarks This method must be externally synchronized.
    VIRTUAL CONST PipelineStateNotation* METHOD(GetPipelineStateByName)(THIS_
                                                                        const Char*   Name,
                                                                        PIPELINE_TYPE PipelineType DEFAULT_VALUE(PIPELINE_TYPE_INVALID)) CONST PURE;

    /// Returns the resource signature notation by its name. If the resource is not found, returns nullptr.

    /// \param [in] Name - Name of the resource signature.
    /// \return Const pointer to the PipelineResourceSignatureDesc structure, see Diligent::PipelineResourceSignatureDesc.
    ///
    /// \remarks This method must be externally synchronized.
    VIRTUAL CONST PipelineResourceSignatureDesc* METHOD(GetResourceSignatureByName)(THIS_
                                                                                    const Char* Name) CONST PURE;

    /// Returns the shader create info by its name. If the resource is not found, returns nullptr.

    /// \param [in] Name - Name of the shader.
    /// \return Const pointer to the ShaderCreateInfo structure, see Diligent::ShaderCreateInfo.
    ///
    /// \remarks This method must be externally synchronized.
    VIRTUAL CONST ShaderCreateInfo* METHOD(GetShaderByName)(THIS_
                                                            const Char* Name) CONST PURE;

    /// Returns the render pass description by its name. If the resource is not found, returns nullptr.

    /// \param [in] Name - Name of the render pass.
    /// \return Const pointer to the RenderPassDesc structure, see Diligent::RenderPassDesc.
    ///
    /// \remarks This method must be externally synchronized.
    VIRTUAL CONST RenderPassDesc*  METHOD(GetRenderPassByName)(THIS_
                                                               const Char* Name) CONST PURE;

    /// Returns the pipeline state notation by its index.

    /// \param [in] Index - Pipeline state notation index. The index must be between 0 and the total number
    ///                     of pipeline state notations stored in this parser as returned by
    ///                     IRenderStateNotationParser::GetInfo().PipelineStateCount.
    /// \return Const pointer to the PipelineStateNotation structure.
    ///
    /// \remarks This method must be externally synchronized.
    VIRTUAL CONST PipelineStateNotation* METHOD(GetPipelineStateByIndex)(THIS_
                                                                         Uint32 Index) CONST PURE;

    /// Returns the pipeline resource signature description by its index.

    /// \param [in] Index - Pipeline resource signature descriptor index. The index must be between 0 and the total number
    ///                     of pipeline resource signature descriptors stored in this parser as returned by
    ///                     IRenderStateNotationParser::GetInfo().ResourceSignatureCount.
    /// \return Const pointer to the PipelineResourceSignatureDesc structure.
    ///
    /// \remarks This method must be externally synchronized.
    VIRTUAL CONST PipelineResourceSignatureDesc* METHOD(GetResourceSignatureByIndex)(THIS_
                                                                                     Uint32 Index) CONST PURE;

    /// Returns the shader create info by its index.

    /// \param [in] Index - Shader create info index. The index must be between 0 and the total number
    ///                     of shader create infos in this parser as returned by
    ///                     IRenderStateNotationParser::GetInfo().ShaderCount.
    /// \return Const pointer to the ShaderCreateInfo structure.
    ///
    /// \remarks This method must be externally synchronized.
    VIRTUAL CONST ShaderCreateInfo* METHOD(GetShaderByIndex)(THIS_
                                                             Uint32 Index) CONST PURE;

    /// Returns the render pass description by its index.

    /// \param [in] Index - Render pass index. The index must be between 0 and the total number
    ///                     of render pass descriptors stored in this parser as returned by
    ///                     IRenderStateNotationParser::GetInfo().RenderPassCount.
    /// \return Const pointer to the RenderPassDesc structure.
    ///
    /// \remarks This method must be externally synchronized.
    VIRTUAL CONST RenderPassDesc* METHOD(GetRenderPassByIndex)(THIS_
                                                               Uint32 Index) CONST PURE;

    /// Checks if the given signature is in the ignored list.
    /// 
    /// \param [in] Name - Name of the signature.
    /// \return true if the given signature is ignored, and false otherwise.
    ///
    /// \remarks This method must be externally synchronized.
    VIRTUAL Bool METHOD(IsSignatureIgnored)(THIS_
                                            const Char* Name) CONST PURE;

    /// Returns the render state notation parser info.

    /// \return Const reference to the RenderStateNotationParserInfo structure.
    ///
    /// \remarks This method must be externally synchronized.
    VIRTUAL CONST RenderStateNotationParserInfo REF METHOD(GetInfo)(THIS) CONST PURE;

    /// Resets the parser to default state.
    VIRTUAL void METHOD(Reset)(THIS) PURE;

    /// Reload all states. 
    ///
    /// \return true if all states were reloaded successfully, and false otherwise.
    ///
    /// \note   This method is only allowed if the EnableReload member of RenderStateNotationParserCreateInfo
    ///         struct was set to true when the parser was created.
    VIRTUAL bool METHOD(Reload)(THIS) PURE;
};
DILIGENT_END_INTERFACE

#include "../../../DiligentCore/Primitives/interface/UndefInterfaceHelperMacros.h"

#if DILIGENT_C_INTERFACE

// clang-format off
#    define IRenderStateNotationParser_ParseFile(This, ...)                   CALL_IFACE_METHOD(RenderStateNotationParser, ParseFile,                   This, __VA_ARGS__)
#    define IRenderStateNotationParser_ParseString(This, ...)                 CALL_IFACE_METHOD(RenderStateNotationParser, ParseString,                 This, __VA_ARGS__)
#    define IRenderStateNotationParser_GetPipelineStateByName(This, ...)      CALL_IFACE_METHOD(RenderStateNotationParser, GetPipelineStateByName,      This, __VA_ARGS__)
#    define IRenderStateNotationParser_GetResourceSignatureByName(This, ...)  CALL_IFACE_METHOD(RenderStateNotationParser, GetResourceSignatureByName,  This, __VA_ARGS__)
#    define IRenderStateNotationParser_GetShaderByName(This, ...)             CALL_IFACE_METHOD(RenderStateNotationParser, GetShaderByName,             This, __VA_ARGS__)
#    define IRenderStateNotationParser_GetRenderPassByName(This, ...)         CALL_IFACE_METHOD(RenderStateNotationParser, GetRenderPassByName,         This, __VA_ARGS__)
#    define IRenderStateNotationParser_GetPipelineStateByIndex(This, ...)     CALL_IFACE_METHOD(RenderStateNotationParser, GetPipelineStateByIndex,     This, __VA_ARGS__)
#    define IRenderStateNotationParser_GetResourceSignatureByIndex(This, ...) CALL_IFACE_METHOD(RenderStateNotationParser, GetResourceSignatureByIndex, This, __VA_ARGS__)
#    define IRenderStateNotationParser_GetShaderByIndex(This, ...)            CALL_IFACE_METHOD(RenderStateNotationParser, GetShaderByIndex,            This, __VA_ARGS__)
#    define IRenderStateNotationParser_GetRenderPassByIndex(This, ...)        CALL_IFACE_METHOD(RenderStateNotationParser, GetRenderPassByIndex,        This, __VA_ARGS__)
#    define IRenderStateNotationParser_GetInfo(This, ...)                     CALL_IFACE_METHOD(RenderStateNotationParser, GetInfo,                     This)
#    define IRenderStateNotationParser_Reset(This)                            CALL_IFACE_METHOD(RenderStateNotationParser, Reset,                       This)
#    define IRenderStateNotationParser_Reload(This)                           CALL_IFACE_METHOD(RenderStateNotationParser, Reload,                      This)
// clang-format on

#endif

#include "../../../DiligentCore/Primitives/interface/DefineGlobalFuncHelperMacros.h"

void DILIGENT_GLOBAL_FUNCTION(CreateRenderStateNotationParser)(const RenderStateNotationParserCreateInfo REF CreateInfo,
                                                               IRenderStateNotationParser**                  pParser);


#include "../../../DiligentCore/Primitives/interface/UndefGlobalFuncHelperMacros.h"

DILIGENT_END_NAMESPACE // namespace Diligent
