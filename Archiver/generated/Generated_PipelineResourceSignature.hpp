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

inline void Serialize(nlohmann::json& Json, const ImmutableSamplerDesc& Type, DeviceObjectReflection* pAllocator)
{
    if (!(Type.ShaderStages == ImmutableSamplerDesc{}.ShaderStages))
        SerializeBitwiseEnum(Json["ShaderStages"], Type.ShaderStages, pAllocator);

    if (!CompareStr(Type.SamplerOrTextureName, ImmutableSamplerDesc{}.SamplerOrTextureName))
        Serialize(Json["SamplerOrTextureName"], Type.SamplerOrTextureName, pAllocator);

    if (!(Type.Desc == ImmutableSamplerDesc{}.Desc))
        Serialize(Json["Desc"], Type.Desc, pAllocator);
}

inline void Deserialize(const nlohmann::json& Json, ImmutableSamplerDesc& Type, DeviceObjectReflection* pAllocator)
{
    if (Json.contains("ShaderStages"))
        DeserializeBitwiseEnum(Json["ShaderStages"], Type.ShaderStages, pAllocator);

    if (Json.contains("SamplerOrTextureName"))
        Deserialize(Json["SamplerOrTextureName"], Type.SamplerOrTextureName, pAllocator);

    if (Json.contains("Desc"))
        Deserialize(Json["Desc"], Type.Desc, pAllocator);
}

inline void Serialize(nlohmann::json& Json, const PipelineResourceDesc& Type, DeviceObjectReflection* pAllocator)
{
    if (!CompareStr(Type.Name, PipelineResourceDesc{}.Name))
        Serialize(Json["Name"], Type.Name, pAllocator);

    if (!(Type.ShaderStages == PipelineResourceDesc{}.ShaderStages))
        SerializeBitwiseEnum(Json["ShaderStages"], Type.ShaderStages, pAllocator);

    if (!(Type.ArraySize == PipelineResourceDesc{}.ArraySize))
        Serialize(Json["ArraySize"], Type.ArraySize, pAllocator);

    if (!(Type.ResourceType == PipelineResourceDesc{}.ResourceType))
        Serialize(Json["ResourceType"], Type.ResourceType, pAllocator);

    if (!(Type.VarType == PipelineResourceDesc{}.VarType))
        Serialize(Json["VarType"], Type.VarType, pAllocator);

    if (!(Type.Flags == PipelineResourceDesc{}.Flags))
        SerializeBitwiseEnum(Json["Flags"], Type.Flags, pAllocator);
}

inline void Deserialize(const nlohmann::json& Json, PipelineResourceDesc& Type, DeviceObjectReflection* pAllocator)
{
    if (Json.contains("Name"))
        Deserialize(Json["Name"], Type.Name, pAllocator);

    if (Json.contains("ShaderStages"))
        DeserializeBitwiseEnum(Json["ShaderStages"], Type.ShaderStages, pAllocator);

    if (Json.contains("ArraySize"))
        Deserialize(Json["ArraySize"], Type.ArraySize, pAllocator);

    if (Json.contains("ResourceType"))
        Deserialize(Json["ResourceType"], Type.ResourceType, pAllocator);

    if (Json.contains("VarType"))
        Deserialize(Json["VarType"], Type.VarType, pAllocator);

    if (Json.contains("Flags"))
        DeserializeBitwiseEnum(Json["Flags"], Type.Flags, pAllocator);
}

inline void Serialize(nlohmann::json& Json, const PipelineResourceSignatureDesc& Type, DeviceObjectReflection* pAllocator)
{
    Serialize(Json, static_cast<DeviceObjectAttribs>(Type), pAllocator);

    if (!(Type.Resources == PipelineResourceSignatureDesc{}.Resources))
        Serialize(Json["Resources"], Type.Resources, Type.NumResources, pAllocator);

    if (!(Type.ImmutableSamplers == PipelineResourceSignatureDesc{}.ImmutableSamplers))
        Serialize(Json["ImmutableSamplers"], Type.ImmutableSamplers, Type.NumImmutableSamplers, pAllocator);

    if (!(Type.BindingIndex == PipelineResourceSignatureDesc{}.BindingIndex))
        Serialize(Json["BindingIndex"], Type.BindingIndex, pAllocator);

    if (!(Type.UseCombinedTextureSamplers == PipelineResourceSignatureDesc{}.UseCombinedTextureSamplers))
        Serialize(Json["UseCombinedTextureSamplers"], Type.UseCombinedTextureSamplers, pAllocator);

    if (!CompareStr(Type.CombinedSamplerSuffix, PipelineResourceSignatureDesc{}.CombinedSamplerSuffix))
        Serialize(Json["CombinedSamplerSuffix"], Type.CombinedSamplerSuffix, pAllocator);

    if (!(Type.SRBAllocationGranularity == PipelineResourceSignatureDesc{}.SRBAllocationGranularity))
        Serialize(Json["SRBAllocationGranularity"], Type.SRBAllocationGranularity, pAllocator);
}

inline void Deserialize(const nlohmann::json& Json, PipelineResourceSignatureDesc& Type, DeviceObjectReflection* pAllocator)
{
    Deserialize(Json, static_cast<DeviceObjectAttribs&>(Type), pAllocator);

    if (Json.contains("Resources"))
        Deserialize(Json["Resources"], Type.Resources, Type.NumResources, pAllocator);

    if (Json.contains("ImmutableSamplers"))
        Deserialize(Json["ImmutableSamplers"], Type.ImmutableSamplers, Type.NumImmutableSamplers, pAllocator);

    if (Json.contains("BindingIndex"))
        Deserialize(Json["BindingIndex"], Type.BindingIndex, pAllocator);

    if (Json.contains("UseCombinedTextureSamplers"))
        Deserialize(Json["UseCombinedTextureSamplers"], Type.UseCombinedTextureSamplers, pAllocator);

    if (Json.contains("CombinedSamplerSuffix"))
        Deserialize(Json["CombinedSamplerSuffix"], Type.CombinedSamplerSuffix, pAllocator);

    if (Json.contains("SRBAllocationGranularity"))
        Deserialize(Json["SRBAllocationGranularity"], Type.SRBAllocationGranularity, pAllocator);
}

} // namespace Diligent
