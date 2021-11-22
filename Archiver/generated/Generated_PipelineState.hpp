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

#include "PipelineState.h"

namespace Diligent
{

NLOHMANN_JSON_SERIALIZE_ENUM(
    SHADER_VARIABLE_FLAGS,
    {
        {SHADER_VARIABLE_FLAG_NONE, "NONE"},
        {SHADER_VARIABLE_FLAG_NO_DYNAMIC_BUFFERS, "NO_DYNAMIC_BUFFERS"},
        {SHADER_VARIABLE_FLAG_GENERAL_INPUT_ATTACHMENT, "GENERAL_INPUT_ATTACHMENT"},
        {SHADER_VARIABLE_FLAG_LAST, "LAST"},
    })

NLOHMANN_JSON_SERIALIZE_ENUM(
    PIPELINE_SHADING_RATE_FLAGS,
    {
        {PIPELINE_SHADING_RATE_FLAG_NONE, "NONE"},
        {PIPELINE_SHADING_RATE_FLAG_PER_PRIMITIVE, "PER_PRIMITIVE"},
        {PIPELINE_SHADING_RATE_FLAG_TEXTURE_BASED, "TEXTURE_BASED"},
        {PIPELINE_SHADING_RATE_FLAG_LAST, "LAST"},
    })

NLOHMANN_JSON_SERIALIZE_ENUM(
    PIPELINE_TYPE,
    {
        {PIPELINE_TYPE_GRAPHICS, "GRAPHICS"},
        {PIPELINE_TYPE_COMPUTE, "COMPUTE"},
        {PIPELINE_TYPE_MESH, "MESH"},
        {PIPELINE_TYPE_RAY_TRACING, "RAY_TRACING"},
        {PIPELINE_TYPE_TILE, "TILE"},
        {PIPELINE_TYPE_LAST, "LAST"},
        {PIPELINE_TYPE_INVALID, "INVALID"},
    })

NLOHMANN_JSON_SERIALIZE_ENUM(
    PSO_CREATE_FLAGS,
    {
        {PSO_CREATE_FLAG_NONE, "NONE"},
        {PSO_CREATE_FLAG_IGNORE_MISSING_VARIABLES, "IGNORE_MISSING_VARIABLES"},
        {PSO_CREATE_FLAG_IGNORE_MISSING_IMMUTABLE_SAMPLERS, "IGNORE_MISSING_IMMUTABLE_SAMPLERS"},
        {PSO_CREATE_FLAG_DONT_REMAP_SHADER_RESOURCES, "DONT_REMAP_SHADER_RESOURCES"},
    })

inline void Serialize(nlohmann::json& Json, const SampleDesc& Type, DeviceObjectReflection* pAllocator)
{
    if (!(Type.Count == SampleDesc{}.Count))
        Serialize(Json["Count"], Type.Count, pAllocator);

    if (!(Type.Quality == SampleDesc{}.Quality))
        Serialize(Json["Quality"], Type.Quality, pAllocator);
}

inline void Deserialize(const nlohmann::json& Json, SampleDesc& Type, DeviceObjectReflection* pAllocator)
{
    if (Json.contains("Count"))
        Deserialize(Json["Count"], Type.Count, pAllocator);

    if (Json.contains("Quality"))
        Deserialize(Json["Quality"], Type.Quality, pAllocator);
}

inline void Serialize(nlohmann::json& Json, const ShaderResourceVariableDesc& Type, DeviceObjectReflection* pAllocator)
{
    if (!(Type.ShaderStages == ShaderResourceVariableDesc{}.ShaderStages))
        SerializeBitwiseEnum(Json["ShaderStages"], Type.ShaderStages, pAllocator);

    if (!CompareStr(Type.Name, ShaderResourceVariableDesc{}.Name))
        Serialize(Json["Name"], Type.Name, pAllocator);

    if (!(Type.Type == ShaderResourceVariableDesc{}.Type))
        Serialize(Json["Type"], Type.Type, pAllocator);

    if (!(Type.Flags == ShaderResourceVariableDesc{}.Flags))
        SerializeBitwiseEnum(Json["Flags"], Type.Flags, pAllocator);
}

inline void Deserialize(const nlohmann::json& Json, ShaderResourceVariableDesc& Type, DeviceObjectReflection* pAllocator)
{
    if (Json.contains("ShaderStages"))
        DeserializeBitwiseEnum(Json["ShaderStages"], Type.ShaderStages, pAllocator);

    if (Json.contains("Name"))
        Deserialize(Json["Name"], Type.Name, pAllocator);

    if (Json.contains("Type"))
        Deserialize(Json["Type"], Type.Type, pAllocator);

    if (Json.contains("Flags"))
        DeserializeBitwiseEnum(Json["Flags"], Type.Flags, pAllocator);
}

inline void Serialize(nlohmann::json& Json, const PipelineResourceLayoutDesc& Type, DeviceObjectReflection* pAllocator)
{
    if (!(Type.DefaultVariableType == PipelineResourceLayoutDesc{}.DefaultVariableType))
        Serialize(Json["DefaultVariableType"], Type.DefaultVariableType, pAllocator);

    if (!(Type.DefaultVariableMergeStages == PipelineResourceLayoutDesc{}.DefaultVariableMergeStages))
        SerializeBitwiseEnum(Json["DefaultVariableMergeStages"], Type.DefaultVariableMergeStages, pAllocator);

    if (!(Type.Variables == PipelineResourceLayoutDesc{}.Variables))
        Serialize(Json["Variables"], Type.Variables, Type.NumVariables, pAllocator);

    if (!(Type.ImmutableSamplers == PipelineResourceLayoutDesc{}.ImmutableSamplers))
        Serialize(Json["ImmutableSamplers"], Type.ImmutableSamplers, Type.NumImmutableSamplers, pAllocator);
}

inline void Deserialize(const nlohmann::json& Json, PipelineResourceLayoutDesc& Type, DeviceObjectReflection* pAllocator)
{
    if (Json.contains("DefaultVariableType"))
        Deserialize(Json["DefaultVariableType"], Type.DefaultVariableType, pAllocator);

    if (Json.contains("DefaultVariableMergeStages"))
        DeserializeBitwiseEnum(Json["DefaultVariableMergeStages"], Type.DefaultVariableMergeStages, pAllocator);

    if (Json.contains("Variables"))
        Deserialize(Json["Variables"], Type.Variables, Type.NumVariables, pAllocator);

    if (Json.contains("ImmutableSamplers"))
        Deserialize(Json["ImmutableSamplers"], Type.ImmutableSamplers, Type.NumImmutableSamplers, pAllocator);
}

inline void Serialize(nlohmann::json& Json, const GraphicsPipelineDesc& Type, DeviceObjectReflection* pAllocator)
{
    if (!(Type.BlendDesc == GraphicsPipelineDesc{}.BlendDesc))
        Serialize(Json["BlendDesc"], Type.BlendDesc, pAllocator);

    if (!(Type.SampleMask == GraphicsPipelineDesc{}.SampleMask))
        Serialize(Json["SampleMask"], Type.SampleMask, pAllocator);

    if (!(Type.RasterizerDesc == GraphicsPipelineDesc{}.RasterizerDesc))
        Serialize(Json["RasterizerDesc"], Type.RasterizerDesc, pAllocator);

    if (!(Type.DepthStencilDesc == GraphicsPipelineDesc{}.DepthStencilDesc))
        Serialize(Json["DepthStencilDesc"], Type.DepthStencilDesc, pAllocator);

    if (!(Type.InputLayout == GraphicsPipelineDesc{}.InputLayout))
        Serialize(Json["InputLayout"], Type.InputLayout, pAllocator);

    if (!(Type.PrimitiveTopology == GraphicsPipelineDesc{}.PrimitiveTopology))
        Serialize(Json["PrimitiveTopology"], Type.PrimitiveTopology, pAllocator);

    if (!(Type.NumViewports == GraphicsPipelineDesc{}.NumViewports))
        Serialize(Json["NumViewports"], Type.NumViewports, pAllocator);

    if (!(Type.NumRenderTargets == GraphicsPipelineDesc{}.NumRenderTargets))
        Serialize(Json["NumRenderTargets"], Type.NumRenderTargets, pAllocator);

    if (!(Type.SubpassIndex == GraphicsPipelineDesc{}.SubpassIndex))
        Serialize(Json["SubpassIndex"], Type.SubpassIndex, pAllocator);

    if (!(Type.ShadingRateFlags == GraphicsPipelineDesc{}.ShadingRateFlags))
        SerializeBitwiseEnum(Json["ShadingRateFlags"], Type.ShadingRateFlags, pAllocator);

    if (!CompareConstArray(Type.RTVFormats, GraphicsPipelineDesc{}.RTVFormats))
        SerializeConstArray(Json["RTVFormats"], Type.RTVFormats, pAllocator);

    if (!(Type.DSVFormat == GraphicsPipelineDesc{}.DSVFormat))
        Serialize(Json["DSVFormat"], Type.DSVFormat, pAllocator);

    if (!(Type.SmplDesc == GraphicsPipelineDesc{}.SmplDesc))
        Serialize(Json["SmplDesc"], Type.SmplDesc, pAllocator);

    if (!(Type.pRenderPass == GraphicsPipelineDesc{}.pRenderPass))
        Serialize(Json["pRenderPass"], Type.pRenderPass, pAllocator);

    if (!(Type.NodeMask == GraphicsPipelineDesc{}.NodeMask))
        Serialize(Json["NodeMask"], Type.NodeMask, pAllocator);
}

inline void Deserialize(const nlohmann::json& Json, GraphicsPipelineDesc& Type, DeviceObjectReflection* pAllocator)
{
    if (Json.contains("BlendDesc"))
        Deserialize(Json["BlendDesc"], Type.BlendDesc, pAllocator);

    if (Json.contains("SampleMask"))
        Deserialize(Json["SampleMask"], Type.SampleMask, pAllocator);

    if (Json.contains("RasterizerDesc"))
        Deserialize(Json["RasterizerDesc"], Type.RasterizerDesc, pAllocator);

    if (Json.contains("DepthStencilDesc"))
        Deserialize(Json["DepthStencilDesc"], Type.DepthStencilDesc, pAllocator);

    if (Json.contains("InputLayout"))
        Deserialize(Json["InputLayout"], Type.InputLayout, pAllocator);

    if (Json.contains("PrimitiveTopology"))
        Deserialize(Json["PrimitiveTopology"], Type.PrimitiveTopology, pAllocator);

    if (Json.contains("NumViewports"))
        Deserialize(Json["NumViewports"], Type.NumViewports, pAllocator);

    if (Json.contains("NumRenderTargets"))
        Deserialize(Json["NumRenderTargets"], Type.NumRenderTargets, pAllocator);

    if (Json.contains("SubpassIndex"))
        Deserialize(Json["SubpassIndex"], Type.SubpassIndex, pAllocator);

    if (Json.contains("ShadingRateFlags"))
        DeserializeBitwiseEnum(Json["ShadingRateFlags"], Type.ShadingRateFlags, pAllocator);

    if (Json.contains("RTVFormats"))
        DeserializeConstArray(Json["RTVFormats"], Type.RTVFormats, pAllocator);

    if (Json.contains("DSVFormat"))
        Deserialize(Json["DSVFormat"], Type.DSVFormat, pAllocator);

    if (Json.contains("SmplDesc"))
        Deserialize(Json["SmplDesc"], Type.SmplDesc, pAllocator);

    if (Json.contains("pRenderPass"))
        Deserialize(Json["pRenderPass"], Type.pRenderPass, pAllocator);

    if (Json.contains("NodeMask"))
        Deserialize(Json["NodeMask"], Type.NodeMask, pAllocator);
}

inline void Serialize(nlohmann::json& Json, const RayTracingGeneralShaderGroup& Type, DeviceObjectReflection* pAllocator)
{
    if (!CompareStr(Type.Name, RayTracingGeneralShaderGroup{}.Name))
        Serialize(Json["Name"], Type.Name, pAllocator);

    if (!(Type.pShader == RayTracingGeneralShaderGroup{}.pShader))
        Serialize(Json["pShader"], Type.pShader, pAllocator);
}

inline void Deserialize(const nlohmann::json& Json, RayTracingGeneralShaderGroup& Type, DeviceObjectReflection* pAllocator)
{
    if (Json.contains("Name"))
        Deserialize(Json["Name"], Type.Name, pAllocator);

    if (Json.contains("pShader"))
        Deserialize(Json["pShader"], Type.pShader, pAllocator);
}

inline void Serialize(nlohmann::json& Json, const RayTracingTriangleHitShaderGroup& Type, DeviceObjectReflection* pAllocator)
{
    if (!CompareStr(Type.Name, RayTracingTriangleHitShaderGroup{}.Name))
        Serialize(Json["Name"], Type.Name, pAllocator);

    if (!(Type.pClosestHitShader == RayTracingTriangleHitShaderGroup{}.pClosestHitShader))
        Serialize(Json["pClosestHitShader"], Type.pClosestHitShader, pAllocator);

    if (!(Type.pAnyHitShader == RayTracingTriangleHitShaderGroup{}.pAnyHitShader))
        Serialize(Json["pAnyHitShader"], Type.pAnyHitShader, pAllocator);
}

inline void Deserialize(const nlohmann::json& Json, RayTracingTriangleHitShaderGroup& Type, DeviceObjectReflection* pAllocator)
{
    if (Json.contains("Name"))
        Deserialize(Json["Name"], Type.Name, pAllocator);

    if (Json.contains("pClosestHitShader"))
        Deserialize(Json["pClosestHitShader"], Type.pClosestHitShader, pAllocator);

    if (Json.contains("pAnyHitShader"))
        Deserialize(Json["pAnyHitShader"], Type.pAnyHitShader, pAllocator);
}

inline void Serialize(nlohmann::json& Json, const RayTracingProceduralHitShaderGroup& Type, DeviceObjectReflection* pAllocator)
{
    if (!CompareStr(Type.Name, RayTracingProceduralHitShaderGroup{}.Name))
        Serialize(Json["Name"], Type.Name, pAllocator);

    if (!(Type.pIntersectionShader == RayTracingProceduralHitShaderGroup{}.pIntersectionShader))
        Serialize(Json["pIntersectionShader"], Type.pIntersectionShader, pAllocator);

    if (!(Type.pClosestHitShader == RayTracingProceduralHitShaderGroup{}.pClosestHitShader))
        Serialize(Json["pClosestHitShader"], Type.pClosestHitShader, pAllocator);

    if (!(Type.pAnyHitShader == RayTracingProceduralHitShaderGroup{}.pAnyHitShader))
        Serialize(Json["pAnyHitShader"], Type.pAnyHitShader, pAllocator);
}

inline void Deserialize(const nlohmann::json& Json, RayTracingProceduralHitShaderGroup& Type, DeviceObjectReflection* pAllocator)
{
    if (Json.contains("Name"))
        Deserialize(Json["Name"], Type.Name, pAllocator);

    if (Json.contains("pIntersectionShader"))
        Deserialize(Json["pIntersectionShader"], Type.pIntersectionShader, pAllocator);

    if (Json.contains("pClosestHitShader"))
        Deserialize(Json["pClosestHitShader"], Type.pClosestHitShader, pAllocator);

    if (Json.contains("pAnyHitShader"))
        Deserialize(Json["pAnyHitShader"], Type.pAnyHitShader, pAllocator);
}

inline void Serialize(nlohmann::json& Json, const RayTracingPipelineDesc& Type, DeviceObjectReflection* pAllocator)
{
    if (!(Type.ShaderRecordSize == RayTracingPipelineDesc{}.ShaderRecordSize))
        Serialize(Json["ShaderRecordSize"], Type.ShaderRecordSize, pAllocator);

    if (!(Type.MaxRecursionDepth == RayTracingPipelineDesc{}.MaxRecursionDepth))
        Serialize(Json["MaxRecursionDepth"], Type.MaxRecursionDepth, pAllocator);
}

inline void Deserialize(const nlohmann::json& Json, RayTracingPipelineDesc& Type, DeviceObjectReflection* pAllocator)
{
    if (Json.contains("ShaderRecordSize"))
        Deserialize(Json["ShaderRecordSize"], Type.ShaderRecordSize, pAllocator);

    if (Json.contains("MaxRecursionDepth"))
        Deserialize(Json["MaxRecursionDepth"], Type.MaxRecursionDepth, pAllocator);
}

inline void Serialize(nlohmann::json& Json, const PipelineStateDesc& Type, DeviceObjectReflection* pAllocator)
{
    Serialize(Json, static_cast<DeviceObjectAttribs>(Type), pAllocator);

    if (!(Type.PipelineType == PipelineStateDesc{}.PipelineType))
        Serialize(Json["PipelineType"], Type.PipelineType, pAllocator);

    if (!(Type.SRBAllocationGranularity == PipelineStateDesc{}.SRBAllocationGranularity))
        Serialize(Json["SRBAllocationGranularity"], Type.SRBAllocationGranularity, pAllocator);

    if (!(Type.ImmediateContextMask == PipelineStateDesc{}.ImmediateContextMask))
        Serialize(Json["ImmediateContextMask"], Type.ImmediateContextMask, pAllocator);

    if (!(Type.ResourceLayout == PipelineStateDesc{}.ResourceLayout))
        Serialize(Json["ResourceLayout"], Type.ResourceLayout, pAllocator);
}

inline void Deserialize(const nlohmann::json& Json, PipelineStateDesc& Type, DeviceObjectReflection* pAllocator)
{
    Deserialize(Json, static_cast<DeviceObjectAttribs&>(Type), pAllocator);

    if (Json.contains("PipelineType"))
        Deserialize(Json["PipelineType"], Type.PipelineType, pAllocator);

    if (Json.contains("SRBAllocationGranularity"))
        Deserialize(Json["SRBAllocationGranularity"], Type.SRBAllocationGranularity, pAllocator);

    if (Json.contains("ImmediateContextMask"))
        Deserialize(Json["ImmediateContextMask"], Type.ImmediateContextMask, pAllocator);

    if (Json.contains("ResourceLayout"))
        Deserialize(Json["ResourceLayout"], Type.ResourceLayout, pAllocator);
}

inline void Serialize(nlohmann::json& Json, const PipelineStateCreateInfo& Type, DeviceObjectReflection* pAllocator)
{
    if (!(Type.PSODesc == PipelineStateCreateInfo{}.PSODesc))
        Serialize(Json["PSODesc"], Type.PSODesc, pAllocator);

    if (!(Type.Flags == PipelineStateCreateInfo{}.Flags))
        SerializeBitwiseEnum(Json["Flags"], Type.Flags, pAllocator);

    if (!(Type.ppResourceSignatures == PipelineStateCreateInfo{}.ppResourceSignatures))
        Serialize(Json["ppResourceSignatures"], Type.ppResourceSignatures, Type.ResourceSignaturesCount, pAllocator);
}

inline void Deserialize(const nlohmann::json& Json, PipelineStateCreateInfo& Type, DeviceObjectReflection* pAllocator)
{
    if (Json.contains("PSODesc"))
        Deserialize(Json["PSODesc"], Type.PSODesc, pAllocator);

    if (Json.contains("Flags"))
        DeserializeBitwiseEnum(Json["Flags"], Type.Flags, pAllocator);

    if (Json.contains("ppResourceSignatures"))
        Deserialize(Json["ppResourceSignatures"], Type.ppResourceSignatures, Type.ResourceSignaturesCount, pAllocator);
}

inline void Serialize(nlohmann::json& Json, const GraphicsPipelineStateCreateInfo& Type, DeviceObjectReflection* pAllocator)
{
    Serialize(Json, static_cast<PipelineStateCreateInfo>(Type), pAllocator);

    if (!(Type.GraphicsPipeline == GraphicsPipelineStateCreateInfo{}.GraphicsPipeline))
        Serialize(Json["GraphicsPipeline"], Type.GraphicsPipeline, pAllocator);

    if (!(Type.pVS == GraphicsPipelineStateCreateInfo{}.pVS))
        Serialize(Json["pVS"], Type.pVS, pAllocator);

    if (!(Type.pPS == GraphicsPipelineStateCreateInfo{}.pPS))
        Serialize(Json["pPS"], Type.pPS, pAllocator);

    if (!(Type.pDS == GraphicsPipelineStateCreateInfo{}.pDS))
        Serialize(Json["pDS"], Type.pDS, pAllocator);

    if (!(Type.pHS == GraphicsPipelineStateCreateInfo{}.pHS))
        Serialize(Json["pHS"], Type.pHS, pAllocator);

    if (!(Type.pGS == GraphicsPipelineStateCreateInfo{}.pGS))
        Serialize(Json["pGS"], Type.pGS, pAllocator);

    if (!(Type.pAS == GraphicsPipelineStateCreateInfo{}.pAS))
        Serialize(Json["pAS"], Type.pAS, pAllocator);

    if (!(Type.pMS == GraphicsPipelineStateCreateInfo{}.pMS))
        Serialize(Json["pMS"], Type.pMS, pAllocator);
}

inline void Deserialize(const nlohmann::json& Json, GraphicsPipelineStateCreateInfo& Type, DeviceObjectReflection* pAllocator)
{
    Deserialize(Json, static_cast<PipelineStateCreateInfo&>(Type), pAllocator);

    if (Json.contains("GraphicsPipeline"))
        Deserialize(Json["GraphicsPipeline"], Type.GraphicsPipeline, pAllocator);

    if (Json.contains("pVS"))
        Deserialize(Json["pVS"], Type.pVS, pAllocator);

    if (Json.contains("pPS"))
        Deserialize(Json["pPS"], Type.pPS, pAllocator);

    if (Json.contains("pDS"))
        Deserialize(Json["pDS"], Type.pDS, pAllocator);

    if (Json.contains("pHS"))
        Deserialize(Json["pHS"], Type.pHS, pAllocator);

    if (Json.contains("pGS"))
        Deserialize(Json["pGS"], Type.pGS, pAllocator);

    if (Json.contains("pAS"))
        Deserialize(Json["pAS"], Type.pAS, pAllocator);

    if (Json.contains("pMS"))
        Deserialize(Json["pMS"], Type.pMS, pAllocator);
}

inline void Serialize(nlohmann::json& Json, const ComputePipelineStateCreateInfo& Type, DeviceObjectReflection* pAllocator)
{
    Serialize(Json, static_cast<PipelineStateCreateInfo>(Type), pAllocator);

    if (!(Type.pCS == ComputePipelineStateCreateInfo{}.pCS))
        Serialize(Json["pCS"], Type.pCS, pAllocator);
}

inline void Deserialize(const nlohmann::json& Json, ComputePipelineStateCreateInfo& Type, DeviceObjectReflection* pAllocator)
{
    Deserialize(Json, static_cast<PipelineStateCreateInfo&>(Type), pAllocator);

    if (Json.contains("pCS"))
        Deserialize(Json["pCS"], Type.pCS, pAllocator);
}

inline void Serialize(nlohmann::json& Json, const RayTracingPipelineStateCreateInfo& Type, DeviceObjectReflection* pAllocator)
{
    Serialize(Json, static_cast<PipelineStateCreateInfo>(Type), pAllocator);

    if (!(Type.RayTracingPipeline == RayTracingPipelineStateCreateInfo{}.RayTracingPipeline))
        Serialize(Json["RayTracingPipeline"], Type.RayTracingPipeline, pAllocator);

    if (!(Type.pGeneralShaders == RayTracingPipelineStateCreateInfo{}.pGeneralShaders))
        Serialize(Json["pGeneralShaders"], Type.pGeneralShaders, Type.GeneralShaderCount, pAllocator);

    if (!(Type.pTriangleHitShaders == RayTracingPipelineStateCreateInfo{}.pTriangleHitShaders))
        Serialize(Json["pTriangleHitShaders"], Type.pTriangleHitShaders, Type.TriangleHitShaderCount, pAllocator);

    if (!(Type.pProceduralHitShaders == RayTracingPipelineStateCreateInfo{}.pProceduralHitShaders))
        Serialize(Json["pProceduralHitShaders"], Type.pProceduralHitShaders, Type.ProceduralHitShaderCount, pAllocator);

    if (!CompareStr(Type.pShaderRecordName, RayTracingPipelineStateCreateInfo{}.pShaderRecordName))
        Serialize(Json["pShaderRecordName"], Type.pShaderRecordName, pAllocator);

    if (!(Type.MaxAttributeSize == RayTracingPipelineStateCreateInfo{}.MaxAttributeSize))
        Serialize(Json["MaxAttributeSize"], Type.MaxAttributeSize, pAllocator);

    if (!(Type.MaxPayloadSize == RayTracingPipelineStateCreateInfo{}.MaxPayloadSize))
        Serialize(Json["MaxPayloadSize"], Type.MaxPayloadSize, pAllocator);
}

inline void Deserialize(const nlohmann::json& Json, RayTracingPipelineStateCreateInfo& Type, DeviceObjectReflection* pAllocator)
{
    Deserialize(Json, static_cast<PipelineStateCreateInfo&>(Type), pAllocator);

    if (Json.contains("RayTracingPipeline"))
        Deserialize(Json["RayTracingPipeline"], Type.RayTracingPipeline, pAllocator);

    if (Json.contains("pGeneralShaders"))
        Deserialize(Json["pGeneralShaders"], Type.pGeneralShaders, Type.GeneralShaderCount, pAllocator);

    if (Json.contains("pTriangleHitShaders"))
        Deserialize(Json["pTriangleHitShaders"], Type.pTriangleHitShaders, Type.TriangleHitShaderCount, pAllocator);

    if (Json.contains("pProceduralHitShaders"))
        Deserialize(Json["pProceduralHitShaders"], Type.pProceduralHitShaders, Type.ProceduralHitShaderCount, pAllocator);

    if (Json.contains("pShaderRecordName"))
        Deserialize(Json["pShaderRecordName"], Type.pShaderRecordName, pAllocator);

    if (Json.contains("MaxAttributeSize"))
        Deserialize(Json["MaxAttributeSize"], Type.MaxAttributeSize, pAllocator);

    if (Json.contains("MaxPayloadSize"))
        Deserialize(Json["MaxPayloadSize"], Type.MaxPayloadSize, pAllocator);
}

inline void Serialize(nlohmann::json& Json, const TilePipelineDesc& Type, DeviceObjectReflection* pAllocator)
{
    if (!(Type.NumRenderTargets == TilePipelineDesc{}.NumRenderTargets))
        Serialize(Json["NumRenderTargets"], Type.NumRenderTargets, pAllocator);

    if (!(Type.SampleCount == TilePipelineDesc{}.SampleCount))
        Serialize(Json["SampleCount"], Type.SampleCount, pAllocator);

    if (!CompareConstArray(Type.RTVFormats, TilePipelineDesc{}.RTVFormats))
        SerializeConstArray(Json["RTVFormats"], Type.RTVFormats, pAllocator);
}

inline void Deserialize(const nlohmann::json& Json, TilePipelineDesc& Type, DeviceObjectReflection* pAllocator)
{
    if (Json.contains("NumRenderTargets"))
        Deserialize(Json["NumRenderTargets"], Type.NumRenderTargets, pAllocator);

    if (Json.contains("SampleCount"))
        Deserialize(Json["SampleCount"], Type.SampleCount, pAllocator);

    if (Json.contains("RTVFormats"))
        DeserializeConstArray(Json["RTVFormats"], Type.RTVFormats, pAllocator);
}

inline void Serialize(nlohmann::json& Json, const TilePipelineStateCreateInfo& Type, DeviceObjectReflection* pAllocator)
{
    Serialize(Json, static_cast<PipelineStateCreateInfo>(Type), pAllocator);

    if (!(Type.TilePipeline == TilePipelineStateCreateInfo{}.TilePipeline))
        Serialize(Json["TilePipeline"], Type.TilePipeline, pAllocator);

    if (!(Type.pTS == TilePipelineStateCreateInfo{}.pTS))
        Serialize(Json["pTS"], Type.pTS, pAllocator);
}

inline void Deserialize(const nlohmann::json& Json, TilePipelineStateCreateInfo& Type, DeviceObjectReflection* pAllocator)
{
    Deserialize(Json, static_cast<PipelineStateCreateInfo&>(Type), pAllocator);

    if (Json.contains("TilePipeline"))
        Deserialize(Json["TilePipeline"], Type.TilePipeline, pAllocator);

    if (Json.contains("pTS"))
        Deserialize(Json["pTS"], Type.pTS, pAllocator);
}

} // namespace Diligent
