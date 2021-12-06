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

#include "Shader.h"

namespace Diligent
{

NLOHMANN_JSON_SERIALIZE_ENUM(
    SHADER_SOURCE_LANGUAGE,
    {
        {SHADER_SOURCE_LANGUAGE_DEFAULT, "DEFAULT"},
        {SHADER_SOURCE_LANGUAGE_HLSL, "HLSL"},
        {SHADER_SOURCE_LANGUAGE_GLSL, "GLSL"},
        {SHADER_SOURCE_LANGUAGE_MSL, "MSL"},
        {SHADER_SOURCE_LANGUAGE_GLSL_VERBATIM, "GLSL_VERBATIM"},
    })

NLOHMANN_JSON_SERIALIZE_ENUM(
    SHADER_COMPILER,
    {
        {SHADER_COMPILER_DEFAULT, "DEFAULT"},
        {SHADER_COMPILER_GLSLANG, "GLSLANG"},
        {SHADER_COMPILER_DXC, "DXC"},
        {SHADER_COMPILER_FXC, "FXC"},
        {SHADER_COMPILER_LAST, "LAST"},
    })

NLOHMANN_JSON_SERIALIZE_ENUM(
    SHADER_RESOURCE_TYPE,
    {
        {SHADER_RESOURCE_TYPE_UNKNOWN, "UNKNOWN"},
        {SHADER_RESOURCE_TYPE_CONSTANT_BUFFER, "CONSTANT_BUFFER"},
        {SHADER_RESOURCE_TYPE_TEXTURE_SRV, "TEXTURE_SRV"},
        {SHADER_RESOURCE_TYPE_BUFFER_SRV, "BUFFER_SRV"},
        {SHADER_RESOURCE_TYPE_TEXTURE_UAV, "TEXTURE_UAV"},
        {SHADER_RESOURCE_TYPE_BUFFER_UAV, "BUFFER_UAV"},
        {SHADER_RESOURCE_TYPE_SAMPLER, "SAMPLER"},
        {SHADER_RESOURCE_TYPE_INPUT_ATTACHMENT, "INPUT_ATTACHMENT"},
        {SHADER_RESOURCE_TYPE_ACCEL_STRUCT, "ACCEL_STRUCT"},
        {SHADER_RESOURCE_TYPE_LAST, "LAST"},
    })

inline void Serialize(nlohmann::json& Json, const ShaderDesc& Type, DynamicLinearAllocator& Allocator)
{
    Serialize(Json, static_cast<const DeviceObjectAttribs&>(Type), Allocator);

    if (!(Type.ShaderType == ShaderDesc{}.ShaderType))
        SerializeBitwiseEnum(Json["ShaderType"], Type.ShaderType, Allocator);
}

inline void Deserialize(const nlohmann::json& Json, ShaderDesc& Type, DynamicLinearAllocator& Allocator)
{
    Deserialize(Json, static_cast<DeviceObjectAttribs&>(Type), Allocator);

    if (Json.contains("ShaderType"))
        DeserializeBitwiseEnum(Json["ShaderType"], Type.ShaderType, Allocator);
}

inline void Serialize(nlohmann::json& Json, const ShaderMacro& Type, DynamicLinearAllocator& Allocator)
{
    if (!SafeStrEqual(Type.Name, ShaderMacro{}.Name))
        Serialize(Json["Name"], Type.Name, Allocator);

    if (!SafeStrEqual(Type.Definition, ShaderMacro{}.Definition))
        Serialize(Json["Definition"], Type.Definition, Allocator);
}

inline void Deserialize(const nlohmann::json& Json, ShaderMacro& Type, DynamicLinearAllocator& Allocator)
{
    if (Json.contains("Name"))
        Deserialize(Json["Name"], Type.Name, Allocator);

    if (Json.contains("Definition"))
        Deserialize(Json["Definition"], Type.Definition, Allocator);
}

inline void Serialize(nlohmann::json& Json, const ShaderResourceDesc& Type, DynamicLinearAllocator& Allocator)
{
    if (!SafeStrEqual(Type.Name, ShaderResourceDesc{}.Name))
        Serialize(Json["Name"], Type.Name, Allocator);

    if (!(Type.Type == ShaderResourceDesc{}.Type))
        Serialize(Json["Type"], Type.Type, Allocator);

    if (!(Type.ArraySize == ShaderResourceDesc{}.ArraySize))
        Serialize(Json["ArraySize"], Type.ArraySize, Allocator);
}

inline void Deserialize(const nlohmann::json& Json, ShaderResourceDesc& Type, DynamicLinearAllocator& Allocator)
{
    if (Json.contains("Name"))
        Deserialize(Json["Name"], Type.Name, Allocator);

    if (Json.contains("Type"))
        Deserialize(Json["Type"], Type.Type, Allocator);

    if (Json.contains("ArraySize"))
        Deserialize(Json["ArraySize"], Type.ArraySize, Allocator);
}

} // namespace Diligent
