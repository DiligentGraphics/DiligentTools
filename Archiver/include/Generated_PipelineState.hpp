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

inline void to_json(nlohmann::json& Json, const SampleDesc& Type)
{
    if (!(Type.Count == SampleDesc{}.Count))
    {
        Json["Count"] = Type.Count;
    }

    if (!(Type.Quality == SampleDesc{}.Quality))
    {
        Json["Quality"] = Type.Quality;
    }
}

inline void from_json(const nlohmann::json& Json, SampleDesc& Type)
{
    if (Json.contains("Count"))
    {
        Json["Count"].get_to(Type.Count);
    }

    if (Json.contains("Quality"))
    {
        Json["Quality"].get_to(Type.Quality);
    }
}

inline void to_json(nlohmann::json& Json, const ShaderResourceVariableDesc& Type)
{
    if (!(Type.ShaderStages == ShaderResourceVariableDesc{}.ShaderStages))
    {
        to_json_bitwise(Json["ShaderStages"], Type.ShaderStages);
    }

    if (!CompareStr(Type.Name, ShaderResourceVariableDesc{}.Name))
    {
        Json["Name"] = Type.Name;
    }

    if (!(Type.Type == ShaderResourceVariableDesc{}.Type))
    {
        Json["Type"] = Type.Type;
    }

    if (!(Type.Flags == ShaderResourceVariableDesc{}.Flags))
    {
        to_json_bitwise(Json["Flags"], Type.Flags);
    }
}

inline void from_json(const nlohmann::json& Json, ShaderResourceVariableDesc& Type)
{
    if (Json.contains("ShaderStages"))
    {
        from_json_bitwise(Json["ShaderStages"], Type.ShaderStages);
    }

    if (Json.contains("Name"))
    {
        Type.Name = copy_string(Json["Name"].get<std::string>());
    }

    if (Json.contains("Type"))
    {
        Json["Type"].get_to(Type.Type);
    }

    if (Json.contains("Flags"))
    {
        from_json_bitwise(Json["Flags"], Type.Flags);
    }
}

inline void to_json(nlohmann::json& Json, const PipelineResourceLayoutDesc& Type)
{
    if (!(Type.DefaultVariableType == PipelineResourceLayoutDesc{}.DefaultVariableType))
    {
        Json["DefaultVariableType"] = Type.DefaultVariableType;
    }

    if (!(Type.DefaultVariableMergeStages == PipelineResourceLayoutDesc{}.DefaultVariableMergeStages))
    {
        to_json_bitwise(Json["DefaultVariableMergeStages"], Type.DefaultVariableMergeStages);
    }

    if (!(Type.NumVariables == PipelineResourceLayoutDesc{}.NumVariables))
    {
        Json["NumVariables"] = Type.NumVariables;
    }

    if (!(Type.Variables == PipelineResourceLayoutDesc{}.Variables))
    {
        to_json_ptr(Json["Variables"], Type.Variables, Type.NumVariables);
    }

    if (!(Type.NumImmutableSamplers == PipelineResourceLayoutDesc{}.NumImmutableSamplers))
    {
        Json["NumImmutableSamplers"] = Type.NumImmutableSamplers;
    }

    if (!(Type.ImmutableSamplers == PipelineResourceLayoutDesc{}.ImmutableSamplers))
    {
        to_json_ptr(Json["ImmutableSamplers"], Type.ImmutableSamplers, Type.NumImmutableSamplers);
    }
}

inline void from_json(const nlohmann::json& Json, PipelineResourceLayoutDesc& Type)
{
    if (Json.contains("DefaultVariableType"))
    {
        Json["DefaultVariableType"].get_to(Type.DefaultVariableType);
    }

    if (Json.contains("DefaultVariableMergeStages"))
    {
        from_json_bitwise(Json["DefaultVariableMergeStages"], Type.DefaultVariableMergeStages);
    }

    if (Json.contains("NumVariables"))
    {
        Json["NumVariables"].get_to(Type.NumVariables);
    }

    if (Json.contains("Variables"))
    {
        from_json_ptr(Json["Variables"], remove_const(&Type.Variables), Json.at("NumVariables"));
    }

    if (Json.contains("NumImmutableSamplers"))
    {
        Json["NumImmutableSamplers"].get_to(Type.NumImmutableSamplers);
    }

    if (Json.contains("ImmutableSamplers"))
    {
        from_json_ptr(Json["ImmutableSamplers"], remove_const(&Type.ImmutableSamplers), Json.at("NumImmutableSamplers"));
    }
}

inline void to_json(nlohmann::json& Json, const GraphicsPipelineDesc& Type)
{
    if (!(Type.BlendDesc == GraphicsPipelineDesc{}.BlendDesc))
    {
        Json["BlendDesc"] = Type.BlendDesc;
    }

    if (!(Type.SampleMask == GraphicsPipelineDesc{}.SampleMask))
    {
        Json["SampleMask"] = Type.SampleMask;
    }

    if (!(Type.RasterizerDesc == GraphicsPipelineDesc{}.RasterizerDesc))
    {
        Json["RasterizerDesc"] = Type.RasterizerDesc;
    }

    if (!(Type.DepthStencilDesc == GraphicsPipelineDesc{}.DepthStencilDesc))
    {
        Json["DepthStencilDesc"] = Type.DepthStencilDesc;
    }

    if (!(Type.InputLayout == GraphicsPipelineDesc{}.InputLayout))
    {
        Json["InputLayout"] = Type.InputLayout;
    }

    if (!(Type.PrimitiveTopology == GraphicsPipelineDesc{}.PrimitiveTopology))
    {
        Json["PrimitiveTopology"] = Type.PrimitiveTopology;
    }

    if (!(Type.NumViewports == GraphicsPipelineDesc{}.NumViewports))
    {
        Json["NumViewports"] = Type.NumViewports;
    }

    if (!(Type.NumRenderTargets == GraphicsPipelineDesc{}.NumRenderTargets))
    {
        Json["NumRenderTargets"] = Type.NumRenderTargets;
    }

    if (!(Type.SubpassIndex == GraphicsPipelineDesc{}.SubpassIndex))
    {
        Json["SubpassIndex"] = Type.SubpassIndex;
    }

    if (!(Type.ShadingRateFlags == GraphicsPipelineDesc{}.ShadingRateFlags))
    {
        to_json_bitwise(Json["ShadingRateFlags"], Type.ShadingRateFlags);
    }

    if (!(Type.RTVFormats == GraphicsPipelineDesc{}.RTVFormats))
    {
        Json["RTVFormats"] = Type.RTVFormats;
    }

    if (!(Type.DSVFormat == GraphicsPipelineDesc{}.DSVFormat))
    {
        Json["DSVFormat"] = Type.DSVFormat;
    }

    if (!(Type.SmplDesc == GraphicsPipelineDesc{}.SmplDesc))
    {
        Json["SmplDesc"] = Type.SmplDesc;
    }

    if (!(Type.pRenderPass == GraphicsPipelineDesc{}.pRenderPass))
    {
        to_json_interface(Json["pRenderPass"], Type.pRenderPass);
    }

    if (!(Type.NodeMask == GraphicsPipelineDesc{}.NodeMask))
    {
        Json["NodeMask"] = Type.NodeMask;
    }
}

inline void from_json(const nlohmann::json& Json, GraphicsPipelineDesc& Type)
{
    if (Json.contains("BlendDesc"))
    {
        Json["BlendDesc"].get_to(Type.BlendDesc);
    }

    if (Json.contains("SampleMask"))
    {
        Json["SampleMask"].get_to(Type.SampleMask);
    }

    if (Json.contains("RasterizerDesc"))
    {
        Json["RasterizerDesc"].get_to(Type.RasterizerDesc);
    }

    if (Json.contains("DepthStencilDesc"))
    {
        Json["DepthStencilDesc"].get_to(Type.DepthStencilDesc);
    }

    if (Json.contains("InputLayout"))
    {
        Json["InputLayout"].get_to(Type.InputLayout);
    }

    if (Json.contains("PrimitiveTopology"))
    {
        Json["PrimitiveTopology"].get_to(Type.PrimitiveTopology);
    }

    if (Json.contains("NumViewports"))
    {
        Json["NumViewports"].get_to(Type.NumViewports);
    }

    if (Json.contains("NumRenderTargets"))
    {
        Json["NumRenderTargets"].get_to(Type.NumRenderTargets);
    }

    if (Json.contains("SubpassIndex"))
    {
        Json["SubpassIndex"].get_to(Type.SubpassIndex);
    }

    if (Json.contains("ShadingRateFlags"))
    {
        from_json_bitwise(Json["ShadingRateFlags"], Type.ShadingRateFlags);
    }

    if (Json.contains("RTVFormats"))
    {
        Json["RTVFormats"].get_to(Type.RTVFormats);
    }

    if (Json.contains("DSVFormat"))
    {
        Json["DSVFormat"].get_to(Type.DSVFormat);
    }

    if (Json.contains("SmplDesc"))
    {
        Json["SmplDesc"].get_to(Type.SmplDesc);
    }

    if (Json.contains("pRenderPass"))
    {
        from_json_interface(Json["pRenderPass"], &Type.pRenderPass);
    }

    if (Json.contains("NodeMask"))
    {
        Json["NodeMask"].get_to(Type.NodeMask);
    }
}

inline void to_json(nlohmann::json& Json, const PipelineStateDesc& Type)
{
    nlohmann::to_json(Json, static_cast<DeviceObjectAttribs>(Type));

    if (!(Type.PipelineType == PipelineStateDesc{}.PipelineType))
    {
        Json["PipelineType"] = Type.PipelineType;
    }

    if (!(Type.SRBAllocationGranularity == PipelineStateDesc{}.SRBAllocationGranularity))
    {
        Json["SRBAllocationGranularity"] = Type.SRBAllocationGranularity;
    }

    if (!(Type.ImmediateContextMask == PipelineStateDesc{}.ImmediateContextMask))
    {
        Json["ImmediateContextMask"] = Type.ImmediateContextMask;
    }

    if (!(Type.ResourceLayout == PipelineStateDesc{}.ResourceLayout))
    {
        Json["ResourceLayout"] = Type.ResourceLayout;
    }
}

inline void from_json(const nlohmann::json& Json, PipelineStateDesc& Type)
{
    nlohmann::from_json(Json, static_cast<DeviceObjectAttribs&>(Type));

    if (Json.contains("PipelineType"))
    {
        Json["PipelineType"].get_to(Type.PipelineType);
    }

    if (Json.contains("SRBAllocationGranularity"))
    {
        Json["SRBAllocationGranularity"].get_to(Type.SRBAllocationGranularity);
    }

    if (Json.contains("ImmediateContextMask"))
    {
        Json["ImmediateContextMask"].get_to(Type.ImmediateContextMask);
    }

    if (Json.contains("ResourceLayout"))
    {
        Json["ResourceLayout"].get_to(Type.ResourceLayout);
    }
}

inline void to_json(nlohmann::json& Json, const PipelineStateCreateInfo& Type)
{
    if (!(Type.PSODesc == PipelineStateCreateInfo{}.PSODesc))
    {
        Json["PSODesc"] = Type.PSODesc;
    }

    if (!(Type.Flags == PipelineStateCreateInfo{}.Flags))
    {
        to_json_bitwise(Json["Flags"], Type.Flags);
    }

    if (!(Type.ppResourceSignatures == PipelineStateCreateInfo{}.ppResourceSignatures))
    {
        to_json_interface(Json["ppResourceSignatures"], Type.ppResourceSignatures, Type.ResourceSignaturesCount);
    }

    if (!(Type.ResourceSignaturesCount == PipelineStateCreateInfo{}.ResourceSignaturesCount))
    {
        Json["ResourceSignaturesCount"] = Type.ResourceSignaturesCount;
    }
}

inline void from_json(const nlohmann::json& Json, PipelineStateCreateInfo& Type)
{
    if (Json.contains("PSODesc"))
    {
        Json["PSODesc"].get_to(Type.PSODesc);
    }

    if (Json.contains("Flags"))
    {
        from_json_bitwise(Json["Flags"], Type.Flags);
    }

    if (Json.contains("ppResourceSignatures"))
    {
        from_json_interface(Json["ppResourceSignatures"], &Type.ppResourceSignatures, Json.at("ResourceSignaturesCount"));
    }

    if (Json.contains("ResourceSignaturesCount"))
    {
        Json["ResourceSignaturesCount"].get_to(Type.ResourceSignaturesCount);
    }
}

inline void to_json(nlohmann::json& Json, const GraphicsPipelineStateCreateInfo& Type)
{
    nlohmann::to_json(Json, static_cast<PipelineStateCreateInfo>(Type));

    if (!(Type.GraphicsPipeline == GraphicsPipelineStateCreateInfo{}.GraphicsPipeline))
    {
        Json["GraphicsPipeline"] = Type.GraphicsPipeline;
    }

    if (!(Type.pVS == GraphicsPipelineStateCreateInfo{}.pVS))
    {
        to_json_interface(Json["pVS"], Type.pVS);
    }

    if (!(Type.pPS == GraphicsPipelineStateCreateInfo{}.pPS))
    {
        to_json_interface(Json["pPS"], Type.pPS);
    }

    if (!(Type.pDS == GraphicsPipelineStateCreateInfo{}.pDS))
    {
        to_json_interface(Json["pDS"], Type.pDS);
    }

    if (!(Type.pHS == GraphicsPipelineStateCreateInfo{}.pHS))
    {
        to_json_interface(Json["pHS"], Type.pHS);
    }

    if (!(Type.pGS == GraphicsPipelineStateCreateInfo{}.pGS))
    {
        to_json_interface(Json["pGS"], Type.pGS);
    }

    if (!(Type.pAS == GraphicsPipelineStateCreateInfo{}.pAS))
    {
        to_json_interface(Json["pAS"], Type.pAS);
    }

    if (!(Type.pMS == GraphicsPipelineStateCreateInfo{}.pMS))
    {
        to_json_interface(Json["pMS"], Type.pMS);
    }
}

inline void from_json(const nlohmann::json& Json, GraphicsPipelineStateCreateInfo& Type)
{
    nlohmann::from_json(Json, static_cast<PipelineStateCreateInfo&>(Type));

    if (Json.contains("GraphicsPipeline"))
    {
        Json["GraphicsPipeline"].get_to(Type.GraphicsPipeline);
    }

    if (Json.contains("pVS"))
    {
        from_json_interface(Json["pVS"], &Type.pVS);
    }

    if (Json.contains("pPS"))
    {
        from_json_interface(Json["pPS"], &Type.pPS);
    }

    if (Json.contains("pDS"))
    {
        from_json_interface(Json["pDS"], &Type.pDS);
    }

    if (Json.contains("pHS"))
    {
        from_json_interface(Json["pHS"], &Type.pHS);
    }

    if (Json.contains("pGS"))
    {
        from_json_interface(Json["pGS"], &Type.pGS);
    }

    if (Json.contains("pAS"))
    {
        from_json_interface(Json["pAS"], &Type.pAS);
    }

    if (Json.contains("pMS"))
    {
        from_json_interface(Json["pMS"], &Type.pMS);
    }
}

inline void to_json(nlohmann::json& Json, const TilePipelineDesc& Type)
{
    if (!(Type.NumRenderTargets == TilePipelineDesc{}.NumRenderTargets))
    {
        Json["NumRenderTargets"] = Type.NumRenderTargets;
    }

    if (!(Type.SampleCount == TilePipelineDesc{}.SampleCount))
    {
        Json["SampleCount"] = Type.SampleCount;
    }

    if (!(Type.RTVFormats == TilePipelineDesc{}.RTVFormats))
    {
        Json["RTVFormats"] = Type.RTVFormats;
    }
}

inline void from_json(const nlohmann::json& Json, TilePipelineDesc& Type)
{
    if (Json.contains("NumRenderTargets"))
    {
        Json["NumRenderTargets"].get_to(Type.NumRenderTargets);
    }

    if (Json.contains("SampleCount"))
    {
        Json["SampleCount"].get_to(Type.SampleCount);
    }

    if (Json.contains("RTVFormats"))
    {
        Json["RTVFormats"].get_to(Type.RTVFormats);
    }
}

inline void to_json(nlohmann::json& Json, const TilePipelineStateCreateInfo& Type)
{
    nlohmann::to_json(Json, static_cast<PipelineStateCreateInfo>(Type));

    if (!(Type.TilePipeline == TilePipelineStateCreateInfo{}.TilePipeline))
    {
        Json["TilePipeline"] = Type.TilePipeline;
    }

    if (!(Type.pTS == TilePipelineStateCreateInfo{}.pTS))
    {
        to_json_interface(Json["pTS"], Type.pTS);
    }
}

inline void from_json(const nlohmann::json& Json, TilePipelineStateCreateInfo& Type)
{
    nlohmann::from_json(Json, static_cast<PipelineStateCreateInfo&>(Type));

    if (Json.contains("TilePipeline"))
    {
        Json["TilePipeline"].get_to(Type.TilePipeline);
    }

    if (Json.contains("pTS"))
    {
        from_json_interface(Json["pTS"], &Type.pTS);
    }
}

} // namespace Diligent
