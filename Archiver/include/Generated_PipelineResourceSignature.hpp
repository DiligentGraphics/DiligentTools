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

inline void Serialize(nlohmann::json& Json, const ImmutableSamplerDesc& Type, DeviceObjectReflection* pAllocator)
{
    if (!(Type.ShaderStages == ImmutableSamplerDesc{}.ShaderStages))
        SerializeBitwiseEnum(Json["ShaderStages"], Type.ShaderStages, pAllocator);

    if (!CompareStr(Type.SamplerOrTextureName, ImmutableSamplerDesc{}.SamplerOrTextureName))
        Json["SamplerOrTextureName"] = Type.SamplerOrTextureName;

    if (!(Type.Desc == ImmutableSamplerDesc{}.Desc))
        Serialize(Json["Desc"], Type.Desc, pAllocator);
}

inline void Deserialize(const nlohmann::json& Json, ImmutableSamplerDesc& Type, DeviceObjectReflection* pAllocator)
{
    if (Json.contains("ShaderStages"))
        DeserializeBitwiseEnum(Json["ShaderStages"], Type.ShaderStages, pAllocator);

    if (Json.contains("SamplerOrTextureName"))
        Type.SamplerOrTextureName = CopyString(Json["SamplerOrTextureName"].get<std::string>(), pAllocator);

    if (Json.contains("Desc"))
        Deserialize(Json["Desc"], Type.Desc, pAllocator);
}

inline void Serialize(nlohmann::json& Json, const PipelineResourceDesc& Type, DeviceObjectReflection* pAllocator)
{
    if (!CompareStr(Type.Name, PipelineResourceDesc{}.Name))
        Json["Name"] = Type.Name;

    if (!(Type.ShaderStages == PipelineResourceDesc{}.ShaderStages))
        SerializeBitwiseEnum(Json["ShaderStages"], Type.ShaderStages, pAllocator);

    if (!(Type.ArraySize == PipelineResourceDesc{}.ArraySize))
        Serialize(Json["ArraySize"], Type.ArraySize, pAllocator);

    if (!(Type.ResourceType == PipelineResourceDesc{}.ResourceType))
        Serialize(Json["ResourceType"], Type.ResourceType, pAllocator);

    if (!(Type.VarType == PipelineResourceDesc{}.VarType))
        Serialize(Json["VarType"], Type.VarType, pAllocator);

    if (!(Type.Flags == PipelineResourceDesc{}.Flags))
        Serialize(Json["Flags"], Type.Flags, pAllocator);
}

inline void Deserialize(const nlohmann::json& Json, PipelineResourceDesc& Type, DeviceObjectReflection* pAllocator)
{
    if (Json.contains("Name"))
        Type.Name = CopyString(Json["Name"].get<std::string>(), pAllocator);

    if (Json.contains("ShaderStages"))
        DeserializeBitwiseEnum(Json["ShaderStages"], Type.ShaderStages, pAllocator);

    if (Json.contains("ArraySize"))
        Deserialize(Json["ArraySize"], Type.ArraySize, pAllocator);

    if (Json.contains("ResourceType"))
        Deserialize(Json["ResourceType"], Type.ResourceType, pAllocator);

    if (Json.contains("VarType"))
        Deserialize(Json["VarType"], Type.VarType, pAllocator);

    if (Json.contains("Flags"))
        Deserialize(Json["Flags"], Type.Flags, pAllocator);
}

inline void Serialize(nlohmann::json& Json, const PipelineResourceSignatureDesc& Type, DeviceObjectReflection* pAllocator)
{
    Serialize(Json, static_cast<DeviceObjectAttribs>(Type), pAllocator);

    if (!(Type.Resources == PipelineResourceSignatureDesc{}.Resources))
        SerializePtr(Json["Resources"], Type.Resources, Type.NumResources, pAllocator);

    if (!(Type.NumResources == PipelineResourceSignatureDesc{}.NumResources))
        Serialize(Json["NumResources"], Type.NumResources, pAllocator);

    if (!(Type.ImmutableSamplers == PipelineResourceSignatureDesc{}.ImmutableSamplers))
        SerializePtr(Json["ImmutableSamplers"], Type.ImmutableSamplers, Type.NumImmutableSamplers, pAllocator);

    if (!(Type.NumImmutableSamplers == PipelineResourceSignatureDesc{}.NumImmutableSamplers))
        Serialize(Json["NumImmutableSamplers"], Type.NumImmutableSamplers, pAllocator);

    if (!(Type.BindingIndex == PipelineResourceSignatureDesc{}.BindingIndex))
        Serialize(Json["BindingIndex"], Type.BindingIndex, pAllocator);

    if (!(Type.UseCombinedTextureSamplers == PipelineResourceSignatureDesc{}.UseCombinedTextureSamplers))
        Serialize(Json["UseCombinedTextureSamplers"], Type.UseCombinedTextureSamplers, pAllocator);

    if (!CompareStr(Type.CombinedSamplerSuffix, PipelineResourceSignatureDesc{}.CombinedSamplerSuffix))
        Json["CombinedSamplerSuffix"] = Type.CombinedSamplerSuffix;

    if (!(Type.SRBAllocationGranularity == PipelineResourceSignatureDesc{}.SRBAllocationGranularity))
        Serialize(Json["SRBAllocationGranularity"], Type.SRBAllocationGranularity, pAllocator);
}

inline void Deserialize(const nlohmann::json& Json, PipelineResourceSignatureDesc& Type, DeviceObjectReflection* pAllocator)
{
    Deserialize(Json, static_cast<DeviceObjectAttribs&>(Type), pAllocator);

    if (Json.contains("Resources"))
        DeserializePtr(Json["Resources"], RemoveConst(&Type.Resources), Json.at("NumResources"), pAllocator);

    if (Json.contains("NumResources"))
        Deserialize(Json["NumResources"], Type.NumResources, pAllocator);

    if (Json.contains("ImmutableSamplers"))
        DeserializePtr(Json["ImmutableSamplers"], RemoveConst(&Type.ImmutableSamplers), Json.at("NumImmutableSamplers"), pAllocator);

    if (Json.contains("NumImmutableSamplers"))
        Deserialize(Json["NumImmutableSamplers"], Type.NumImmutableSamplers, pAllocator);

    if (Json.contains("BindingIndex"))
        Deserialize(Json["BindingIndex"], Type.BindingIndex, pAllocator);

    if (Json.contains("UseCombinedTextureSamplers"))
        Deserialize(Json["UseCombinedTextureSamplers"], Type.UseCombinedTextureSamplers, pAllocator);

    if (Json.contains("CombinedSamplerSuffix"))
        Type.CombinedSamplerSuffix = CopyString(Json["CombinedSamplerSuffix"].get<std::string>(), pAllocator);

    if (Json.contains("SRBAllocationGranularity"))
        Deserialize(Json["SRBAllocationGranularity"], Type.SRBAllocationGranularity, pAllocator);
}

} // namespace Diligent
