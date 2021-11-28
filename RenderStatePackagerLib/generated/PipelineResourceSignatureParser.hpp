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

#include "PipelineResourceSignature.h"

namespace Diligent
{

NLOHMANN_JSON_SERIALIZE_ENUM(
    PIPELINE_RESOURCE_FLAGS,
    {
        {PIPELINE_RESOURCE_FLAG_NONE, "NONE"},
        {PIPELINE_RESOURCE_FLAG_NO_DYNAMIC_BUFFERS, "NO_DYNAMIC_BUFFERS"},
        {PIPELINE_RESOURCE_FLAG_COMBINED_SAMPLER, "COMBINED_SAMPLER"},
        {PIPELINE_RESOURCE_FLAG_FORMATTED_BUFFER, "FORMATTED_BUFFER"},
        {PIPELINE_RESOURCE_FLAG_RUNTIME_ARRAY, "RUNTIME_ARRAY"},
        {PIPELINE_RESOURCE_FLAG_GENERAL_INPUT_ATTACHMENT, "GENERAL_INPUT_ATTACHMENT"},
        {PIPELINE_RESOURCE_FLAG_LAST, "LAST"},
    })

inline void Serialize(nlohmann::json& Json, const ImmutableSamplerDesc& Type, DynamicLinearAllocator& Allocator)
{
    if (!(Type.ShaderStages == ImmutableSamplerDesc{}.ShaderStages))
        SerializeBitwiseEnum(Json["ShaderStages"], Type.ShaderStages, Allocator);

    if (!CompareStr(Type.SamplerOrTextureName, ImmutableSamplerDesc{}.SamplerOrTextureName))
        Serialize(Json["SamplerOrTextureName"], Type.SamplerOrTextureName, Allocator);

    if (!(Type.Desc == ImmutableSamplerDesc{}.Desc))
        Serialize(Json["Desc"], Type.Desc, Allocator);
}

inline void Deserialize(const nlohmann::json& Json, ImmutableSamplerDesc& Type, DynamicLinearAllocator& Allocator)
{
    if (Json.contains("ShaderStages"))
        DeserializeBitwiseEnum(Json["ShaderStages"], Type.ShaderStages, Allocator);

    if (Json.contains("SamplerOrTextureName"))
        Deserialize(Json["SamplerOrTextureName"], Type.SamplerOrTextureName, Allocator);

    if (Json.contains("Desc"))
        Deserialize(Json["Desc"], Type.Desc, Allocator);
}

inline void Serialize(nlohmann::json& Json, const PipelineResourceDesc& Type, DynamicLinearAllocator& Allocator)
{
    if (!CompareStr(Type.Name, PipelineResourceDesc{}.Name))
        Serialize(Json["Name"], Type.Name, Allocator);

    if (!(Type.ShaderStages == PipelineResourceDesc{}.ShaderStages))
        SerializeBitwiseEnum(Json["ShaderStages"], Type.ShaderStages, Allocator);

    if (!(Type.ArraySize == PipelineResourceDesc{}.ArraySize))
        Serialize(Json["ArraySize"], Type.ArraySize, Allocator);

    if (!(Type.ResourceType == PipelineResourceDesc{}.ResourceType))
        Serialize(Json["ResourceType"], Type.ResourceType, Allocator);

    if (!(Type.VarType == PipelineResourceDesc{}.VarType))
        Serialize(Json["VarType"], Type.VarType, Allocator);

    if (!(Type.Flags == PipelineResourceDesc{}.Flags))
        SerializeBitwiseEnum(Json["Flags"], Type.Flags, Allocator);
}

inline void Deserialize(const nlohmann::json& Json, PipelineResourceDesc& Type, DynamicLinearAllocator& Allocator)
{
    if (Json.contains("Name"))
        Deserialize(Json["Name"], Type.Name, Allocator);

    if (Json.contains("ShaderStages"))
        DeserializeBitwiseEnum(Json["ShaderStages"], Type.ShaderStages, Allocator);

    if (Json.contains("ArraySize"))
        Deserialize(Json["ArraySize"], Type.ArraySize, Allocator);

    if (Json.contains("ResourceType"))
        Deserialize(Json["ResourceType"], Type.ResourceType, Allocator);

    if (Json.contains("VarType"))
        Deserialize(Json["VarType"], Type.VarType, Allocator);

    if (Json.contains("Flags"))
        DeserializeBitwiseEnum(Json["Flags"], Type.Flags, Allocator);
}

inline void Serialize(nlohmann::json& Json, const PipelineResourceSignatureDesc& Type, DynamicLinearAllocator& Allocator)
{
    Serialize(Json, static_cast<const DeviceObjectAttribs&>(Type), Allocator);

    if (!(Type.Resources == PipelineResourceSignatureDesc{}.Resources))
        Serialize(Json["Resources"], Type.Resources, Type.NumResources, Allocator);

    if (!(Type.ImmutableSamplers == PipelineResourceSignatureDesc{}.ImmutableSamplers))
        Serialize(Json["ImmutableSamplers"], Type.ImmutableSamplers, Type.NumImmutableSamplers, Allocator);

    if (!(Type.BindingIndex == PipelineResourceSignatureDesc{}.BindingIndex))
        Serialize(Json["BindingIndex"], Type.BindingIndex, Allocator);

    if (!(Type.UseCombinedTextureSamplers == PipelineResourceSignatureDesc{}.UseCombinedTextureSamplers))
        Serialize(Json["UseCombinedTextureSamplers"], Type.UseCombinedTextureSamplers, Allocator);

    if (!CompareStr(Type.CombinedSamplerSuffix, PipelineResourceSignatureDesc{}.CombinedSamplerSuffix))
        Serialize(Json["CombinedSamplerSuffix"], Type.CombinedSamplerSuffix, Allocator);

    if (!(Type.SRBAllocationGranularity == PipelineResourceSignatureDesc{}.SRBAllocationGranularity))
        Serialize(Json["SRBAllocationGranularity"], Type.SRBAllocationGranularity, Allocator);
}

inline void Deserialize(const nlohmann::json& Json, PipelineResourceSignatureDesc& Type, DynamicLinearAllocator& Allocator)
{
    Deserialize(Json, static_cast<DeviceObjectAttribs&>(Type), Allocator);

    if (Json.contains("Resources"))
        Deserialize(Json["Resources"], Type.Resources, Type.NumResources, Allocator);

    if (Json.contains("ImmutableSamplers"))
        Deserialize(Json["ImmutableSamplers"], Type.ImmutableSamplers, Type.NumImmutableSamplers, Allocator);

    if (Json.contains("BindingIndex"))
        Deserialize(Json["BindingIndex"], Type.BindingIndex, Allocator);

    if (Json.contains("UseCombinedTextureSamplers"))
        Deserialize(Json["UseCombinedTextureSamplers"], Type.UseCombinedTextureSamplers, Allocator);

    if (Json.contains("CombinedSamplerSuffix"))
        Deserialize(Json["CombinedSamplerSuffix"], Type.CombinedSamplerSuffix, Allocator);

    if (Json.contains("SRBAllocationGranularity"))
        Deserialize(Json["SRBAllocationGranularity"], Type.SRBAllocationGranularity, Allocator);
}

} // namespace Diligent
