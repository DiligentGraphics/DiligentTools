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

inline void Serialize(nlohmann::json& Json, const SampleDesc& Type, DynamicLinearAllocator& Allocator)
{
    if (!(Type.Count == SampleDesc{}.Count))
        Serialize(Json["Count"], Type.Count, Allocator);

    if (!(Type.Quality == SampleDesc{}.Quality))
        Serialize(Json["Quality"], Type.Quality, Allocator);
}

inline void Deserialize(const nlohmann::json& Json, SampleDesc& Type, DynamicLinearAllocator& Allocator)
{
    if (Json.contains("Count"))
        Deserialize(Json["Count"], Type.Count, Allocator);

    if (Json.contains("Quality"))
        Deserialize(Json["Quality"], Type.Quality, Allocator);
}

inline void Serialize(nlohmann::json& Json, const ShaderResourceVariableDesc& Type, DynamicLinearAllocator& Allocator)
{
    if (!(Type.ShaderStages == ShaderResourceVariableDesc{}.ShaderStages))
        SerializeBitwiseEnum(Json["ShaderStages"], Type.ShaderStages, Allocator);

    if (!SafeStrEqual(Type.Name, ShaderResourceVariableDesc{}.Name))
        Serialize(Json["Name"], Type.Name, Allocator);

    if (!(Type.Type == ShaderResourceVariableDesc{}.Type))
        Serialize(Json["Type"], Type.Type, Allocator);

    if (!(Type.Flags == ShaderResourceVariableDesc{}.Flags))
        SerializeBitwiseEnum(Json["Flags"], Type.Flags, Allocator);
}

inline void Deserialize(const nlohmann::json& Json, ShaderResourceVariableDesc& Type, DynamicLinearAllocator& Allocator)
{
    if (Json.contains("ShaderStages"))
        DeserializeBitwiseEnum(Json["ShaderStages"], Type.ShaderStages, Allocator);

    if (Json.contains("Name"))
        Deserialize(Json["Name"], Type.Name, Allocator);

    if (Json.contains("Type"))
        Deserialize(Json["Type"], Type.Type, Allocator);

    if (Json.contains("Flags"))
        DeserializeBitwiseEnum(Json["Flags"], Type.Flags, Allocator);
}

inline void Serialize(nlohmann::json& Json, const PipelineResourceLayoutDesc& Type, DynamicLinearAllocator& Allocator)
{
    if (!(Type.DefaultVariableType == PipelineResourceLayoutDesc{}.DefaultVariableType))
        Serialize(Json["DefaultVariableType"], Type.DefaultVariableType, Allocator);

    if (!(Type.DefaultVariableMergeStages == PipelineResourceLayoutDesc{}.DefaultVariableMergeStages))
        SerializeBitwiseEnum(Json["DefaultVariableMergeStages"], Type.DefaultVariableMergeStages, Allocator);

    if (!(Type.Variables == PipelineResourceLayoutDesc{}.Variables))
        Serialize(Json["Variables"], Type.Variables, Type.NumVariables, Allocator);

    if (!(Type.ImmutableSamplers == PipelineResourceLayoutDesc{}.ImmutableSamplers))
        Serialize(Json["ImmutableSamplers"], Type.ImmutableSamplers, Type.NumImmutableSamplers, Allocator);
}

inline void Deserialize(const nlohmann::json& Json, PipelineResourceLayoutDesc& Type, DynamicLinearAllocator& Allocator)
{
    if (Json.contains("DefaultVariableType"))
        Deserialize(Json["DefaultVariableType"], Type.DefaultVariableType, Allocator);

    if (Json.contains("DefaultVariableMergeStages"))
        DeserializeBitwiseEnum(Json["DefaultVariableMergeStages"], Type.DefaultVariableMergeStages, Allocator);

    if (Json.contains("Variables"))
        Deserialize(Json["Variables"], Type.Variables, Type.NumVariables, Allocator);

    if (Json.contains("ImmutableSamplers"))
        Deserialize(Json["ImmutableSamplers"], Type.ImmutableSamplers, Type.NumImmutableSamplers, Allocator);
}

inline void Serialize(nlohmann::json& Json, const GraphicsPipelineDesc& Type, DynamicLinearAllocator& Allocator)
{
    if (!(Type.BlendDesc == GraphicsPipelineDesc{}.BlendDesc))
        Serialize(Json["BlendDesc"], Type.BlendDesc, Allocator);

    if (!(Type.SampleMask == GraphicsPipelineDesc{}.SampleMask))
        Serialize(Json["SampleMask"], Type.SampleMask, Allocator);

    if (!(Type.RasterizerDesc == GraphicsPipelineDesc{}.RasterizerDesc))
        Serialize(Json["RasterizerDesc"], Type.RasterizerDesc, Allocator);

    if (!(Type.DepthStencilDesc == GraphicsPipelineDesc{}.DepthStencilDesc))
        Serialize(Json["DepthStencilDesc"], Type.DepthStencilDesc, Allocator);

    if (!(Type.InputLayout == GraphicsPipelineDesc{}.InputLayout))
        Serialize(Json["InputLayout"], Type.InputLayout, Allocator);

    if (!(Type.PrimitiveTopology == GraphicsPipelineDesc{}.PrimitiveTopology))
        Serialize(Json["PrimitiveTopology"], Type.PrimitiveTopology, Allocator);

    if (!(Type.NumViewports == GraphicsPipelineDesc{}.NumViewports))
        Serialize(Json["NumViewports"], Type.NumViewports, Allocator);

    if (!(Type.NumRenderTargets == GraphicsPipelineDesc{}.NumRenderTargets))
        Serialize(Json["NumRenderTargets"], Type.NumRenderTargets, Allocator);

    if (!(Type.SubpassIndex == GraphicsPipelineDesc{}.SubpassIndex))
        Serialize(Json["SubpassIndex"], Type.SubpassIndex, Allocator);

    if (!(Type.ShadingRateFlags == GraphicsPipelineDesc{}.ShadingRateFlags))
        SerializeBitwiseEnum(Json["ShadingRateFlags"], Type.ShadingRateFlags, Allocator);

    if (!CompareConstArray(Type.RTVFormats, GraphicsPipelineDesc{}.RTVFormats))
        SerializeConstArray(Json["RTVFormats"], Type.RTVFormats, Allocator);

    if (!(Type.DSVFormat == GraphicsPipelineDesc{}.DSVFormat))
        Serialize(Json["DSVFormat"], Type.DSVFormat, Allocator);

    if (!(Type.SmplDesc == GraphicsPipelineDesc{}.SmplDesc))
        Serialize(Json["SmplDesc"], Type.SmplDesc, Allocator);

    if (!(Type.NodeMask == GraphicsPipelineDesc{}.NodeMask))
        Serialize(Json["NodeMask"], Type.NodeMask, Allocator);
}

inline void Deserialize(const nlohmann::json& Json, GraphicsPipelineDesc& Type, DynamicLinearAllocator& Allocator)
{
    if (Json.contains("BlendDesc"))
        Deserialize(Json["BlendDesc"], Type.BlendDesc, Allocator);

    if (Json.contains("SampleMask"))
        Deserialize(Json["SampleMask"], Type.SampleMask, Allocator);

    if (Json.contains("RasterizerDesc"))
        Deserialize(Json["RasterizerDesc"], Type.RasterizerDesc, Allocator);

    if (Json.contains("DepthStencilDesc"))
        Deserialize(Json["DepthStencilDesc"], Type.DepthStencilDesc, Allocator);

    if (Json.contains("InputLayout"))
        Deserialize(Json["InputLayout"], Type.InputLayout, Allocator);

    if (Json.contains("PrimitiveTopology"))
        Deserialize(Json["PrimitiveTopology"], Type.PrimitiveTopology, Allocator);

    if (Json.contains("NumViewports"))
        Deserialize(Json["NumViewports"], Type.NumViewports, Allocator);

    if (Json.contains("NumRenderTargets"))
        Deserialize(Json["NumRenderTargets"], Type.NumRenderTargets, Allocator);

    if (Json.contains("SubpassIndex"))
        Deserialize(Json["SubpassIndex"], Type.SubpassIndex, Allocator);

    if (Json.contains("ShadingRateFlags"))
        DeserializeBitwiseEnum(Json["ShadingRateFlags"], Type.ShadingRateFlags, Allocator);

    if (Json.contains("RTVFormats"))
        DeserializeConstArray(Json["RTVFormats"], Type.RTVFormats, Allocator);

    if (Json.contains("DSVFormat"))
        Deserialize(Json["DSVFormat"], Type.DSVFormat, Allocator);

    if (Json.contains("SmplDesc"))
        Deserialize(Json["SmplDesc"], Type.SmplDesc, Allocator);

    if (Json.contains("NodeMask"))
        Deserialize(Json["NodeMask"], Type.NodeMask, Allocator);
}

inline void Serialize(nlohmann::json& Json, const RayTracingPipelineDesc& Type, DynamicLinearAllocator& Allocator)
{
    if (!(Type.ShaderRecordSize == RayTracingPipelineDesc{}.ShaderRecordSize))
        Serialize(Json["ShaderRecordSize"], Type.ShaderRecordSize, Allocator);

    if (!(Type.MaxRecursionDepth == RayTracingPipelineDesc{}.MaxRecursionDepth))
        Serialize(Json["MaxRecursionDepth"], Type.MaxRecursionDepth, Allocator);
}

inline void Deserialize(const nlohmann::json& Json, RayTracingPipelineDesc& Type, DynamicLinearAllocator& Allocator)
{
    if (Json.contains("ShaderRecordSize"))
        Deserialize(Json["ShaderRecordSize"], Type.ShaderRecordSize, Allocator);

    if (Json.contains("MaxRecursionDepth"))
        Deserialize(Json["MaxRecursionDepth"], Type.MaxRecursionDepth, Allocator);
}

inline void Serialize(nlohmann::json& Json, const PipelineStateDesc& Type, DynamicLinearAllocator& Allocator)
{
    Serialize(Json, static_cast<const DeviceObjectAttribs&>(Type), Allocator);

    if (!(Type.PipelineType == PipelineStateDesc{}.PipelineType))
        Serialize(Json["PipelineType"], Type.PipelineType, Allocator);

    if (!(Type.SRBAllocationGranularity == PipelineStateDesc{}.SRBAllocationGranularity))
        Serialize(Json["SRBAllocationGranularity"], Type.SRBAllocationGranularity, Allocator);

    if (!(Type.ImmediateContextMask == PipelineStateDesc{}.ImmediateContextMask))
        Serialize(Json["ImmediateContextMask"], Type.ImmediateContextMask, Allocator);

    if (!(Type.ResourceLayout == PipelineStateDesc{}.ResourceLayout))
        Serialize(Json["ResourceLayout"], Type.ResourceLayout, Allocator);
}

inline void Deserialize(const nlohmann::json& Json, PipelineStateDesc& Type, DynamicLinearAllocator& Allocator)
{
    Deserialize(Json, static_cast<DeviceObjectAttribs&>(Type), Allocator);

    if (Json.contains("PipelineType"))
        Deserialize(Json["PipelineType"], Type.PipelineType, Allocator);

    if (Json.contains("SRBAllocationGranularity"))
        Deserialize(Json["SRBAllocationGranularity"], Type.SRBAllocationGranularity, Allocator);

    if (Json.contains("ImmediateContextMask"))
        Deserialize(Json["ImmediateContextMask"], Type.ImmediateContextMask, Allocator);

    if (Json.contains("ResourceLayout"))
        Deserialize(Json["ResourceLayout"], Type.ResourceLayout, Allocator);
}

inline void Serialize(nlohmann::json& Json, const TilePipelineDesc& Type, DynamicLinearAllocator& Allocator)
{
    if (!(Type.NumRenderTargets == TilePipelineDesc{}.NumRenderTargets))
        Serialize(Json["NumRenderTargets"], Type.NumRenderTargets, Allocator);

    if (!(Type.SampleCount == TilePipelineDesc{}.SampleCount))
        Serialize(Json["SampleCount"], Type.SampleCount, Allocator);

    if (!CompareConstArray(Type.RTVFormats, TilePipelineDesc{}.RTVFormats))
        SerializeConstArray(Json["RTVFormats"], Type.RTVFormats, Allocator);
}

inline void Deserialize(const nlohmann::json& Json, TilePipelineDesc& Type, DynamicLinearAllocator& Allocator)
{
    if (Json.contains("NumRenderTargets"))
        Deserialize(Json["NumRenderTargets"], Type.NumRenderTargets, Allocator);

    if (Json.contains("SampleCount"))
        Deserialize(Json["SampleCount"], Type.SampleCount, Allocator);

    if (Json.contains("RTVFormats"))
        DeserializeConstArray(Json["RTVFormats"], Type.RTVFormats, Allocator);
}

} // namespace Diligent
