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

inline void to_json(nlohmann::json& Json, const ImmutableSamplerDesc& Type)
{
    if (!(Type.ShaderStages == ImmutableSamplerDesc{}.ShaderStages))
    {
        to_json_bitwise(Json["ShaderStages"], Type.ShaderStages);
    }

    if (!CompareStr(Type.SamplerOrTextureName, ImmutableSamplerDesc{}.SamplerOrTextureName))
    {
        Json["SamplerOrTextureName"] = Type.SamplerOrTextureName;
    }

    if (!(Type.Desc == ImmutableSamplerDesc{}.Desc))
    {
        Json["Desc"] = Type.Desc;
    }
}

inline void from_json(const nlohmann::json& Json, ImmutableSamplerDesc& Type)
{
    if (Json.contains("ShaderStages"))
    {
        from_json_bitwise(Json["ShaderStages"], Type.ShaderStages);
    }

    if (Json.contains("SamplerOrTextureName"))
    {
        Type.SamplerOrTextureName = copy_string(Json["SamplerOrTextureName"].get<std::string>());
    }

    if (Json.contains("Desc"))
    {
        Json["Desc"].get_to(Type.Desc);
    }
}

inline void to_json(nlohmann::json& Json, const PipelineResourceDesc& Type)
{
    if (!CompareStr(Type.Name, PipelineResourceDesc{}.Name))
    {
        Json["Name"] = Type.Name;
    }

    if (!(Type.ShaderStages == PipelineResourceDesc{}.ShaderStages))
    {
        to_json_bitwise(Json["ShaderStages"], Type.ShaderStages);
    }

    if (!(Type.ArraySize == PipelineResourceDesc{}.ArraySize))
    {
        Json["ArraySize"] = Type.ArraySize;
    }

    if (!(Type.ResourceType == PipelineResourceDesc{}.ResourceType))
    {
        Json["ResourceType"] = Type.ResourceType;
    }

    if (!(Type.VarType == PipelineResourceDesc{}.VarType))
    {
        Json["VarType"] = Type.VarType;
    }

    if (!(Type.Flags == PipelineResourceDesc{}.Flags))
    {
        Json["Flags"] = Type.Flags;
    }
}

inline void from_json(const nlohmann::json& Json, PipelineResourceDesc& Type)
{
    if (Json.contains("Name"))
    {
        Type.Name = copy_string(Json["Name"].get<std::string>());
    }

    if (Json.contains("ShaderStages"))
    {
        from_json_bitwise(Json["ShaderStages"], Type.ShaderStages);
    }

    if (Json.contains("ArraySize"))
    {
        Json["ArraySize"].get_to(Type.ArraySize);
    }

    if (Json.contains("ResourceType"))
    {
        Json["ResourceType"].get_to(Type.ResourceType);
    }

    if (Json.contains("VarType"))
    {
        Json["VarType"].get_to(Type.VarType);
    }

    if (Json.contains("Flags"))
    {
        Json["Flags"].get_to(Type.Flags);
    }
}

inline void to_json(nlohmann::json& Json, const PipelineResourceSignatureDesc& Type)
{
    nlohmann::to_json(Json, static_cast<DeviceObjectAttribs>(Type));

    if (!(Type.Resources == PipelineResourceSignatureDesc{}.Resources))
    {
        to_json_ptr(Json["Resources"], Type.Resources, Type.NumResources);
    }

    if (!(Type.NumResources == PipelineResourceSignatureDesc{}.NumResources))
    {
        Json["NumResources"] = Type.NumResources;
    }

    if (!(Type.ImmutableSamplers == PipelineResourceSignatureDesc{}.ImmutableSamplers))
    {
        to_json_ptr(Json["ImmutableSamplers"], Type.ImmutableSamplers, Type.NumImmutableSamplers);
    }

    if (!(Type.NumImmutableSamplers == PipelineResourceSignatureDesc{}.NumImmutableSamplers))
    {
        Json["NumImmutableSamplers"] = Type.NumImmutableSamplers;
    }

    if (!(Type.BindingIndex == PipelineResourceSignatureDesc{}.BindingIndex))
    {
        Json["BindingIndex"] = Type.BindingIndex;
    }

    if (!(Type.UseCombinedTextureSamplers == PipelineResourceSignatureDesc{}.UseCombinedTextureSamplers))
    {
        Json["UseCombinedTextureSamplers"] = Type.UseCombinedTextureSamplers;
    }

    if (!CompareStr(Type.CombinedSamplerSuffix, PipelineResourceSignatureDesc{}.CombinedSamplerSuffix))
    {
        Json["CombinedSamplerSuffix"] = Type.CombinedSamplerSuffix;
    }

    if (!(Type.SRBAllocationGranularity == PipelineResourceSignatureDesc{}.SRBAllocationGranularity))
    {
        Json["SRBAllocationGranularity"] = Type.SRBAllocationGranularity;
    }
}

inline void from_json(const nlohmann::json& Json, PipelineResourceSignatureDesc& Type)
{
    nlohmann::from_json(Json, static_cast<DeviceObjectAttribs&>(Type));

    if (Json.contains("Resources"))
    {
        from_json_ptr(Json["Resources"], remove_const(&Type.Resources), Json.at("NumResources"));
    }

    if (Json.contains("NumResources"))
    {
        Json["NumResources"].get_to(Type.NumResources);
    }

    if (Json.contains("ImmutableSamplers"))
    {
        from_json_ptr(Json["ImmutableSamplers"], remove_const(&Type.ImmutableSamplers), Json.at("NumImmutableSamplers"));
    }

    if (Json.contains("NumImmutableSamplers"))
    {
        Json["NumImmutableSamplers"].get_to(Type.NumImmutableSamplers);
    }

    if (Json.contains("BindingIndex"))
    {
        Json["BindingIndex"].get_to(Type.BindingIndex);
    }

    if (Json.contains("UseCombinedTextureSamplers"))
    {
        Json["UseCombinedTextureSamplers"].get_to(Type.UseCombinedTextureSamplers);
    }

    if (Json.contains("CombinedSamplerSuffix"))
    {
        Type.CombinedSamplerSuffix = copy_string(Json["CombinedSamplerSuffix"].get<std::string>());
    }

    if (Json.contains("SRBAllocationGranularity"))
    {
        Json["SRBAllocationGranularity"].get_to(Type.SRBAllocationGranularity);
    }
}

} // namespace Diligent
