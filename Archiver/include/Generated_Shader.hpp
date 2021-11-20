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

inline void Serialize(nlohmann::json& Json, const ShaderDesc& Type, DeviceObjectReflection* pAllocator)
{
    Serialize(Json, static_cast<DeviceObjectAttribs>(Type), pAllocator);

    if (!(Type.ShaderType == ShaderDesc{}.ShaderType))
        SerializeBitwiseEnum(Json["ShaderType"], Type.ShaderType, pAllocator);
}

inline void Deserialize(const nlohmann::json& Json, ShaderDesc& Type, DeviceObjectReflection* pAllocator)
{
    Deserialize(Json, static_cast<DeviceObjectAttribs&>(Type), pAllocator);

    if (Json.contains("ShaderType"))
        DeserializeBitwiseEnum(Json["ShaderType"], Type.ShaderType, pAllocator);
}

inline void Serialize(nlohmann::json& Json, const ShaderMacro& Type, DeviceObjectReflection* pAllocator)
{
    if (!CompareStr(Type.Name, ShaderMacro{}.Name))
        Serialize(Json["Name"], Type.Name, pAllocator);

    if (!CompareStr(Type.Definition, ShaderMacro{}.Definition))
        Serialize(Json["Definition"], Type.Definition, pAllocator);
}

inline void Deserialize(const nlohmann::json& Json, ShaderMacro& Type, DeviceObjectReflection* pAllocator)
{
    if (Json.contains("Name"))
        Deserialize(Json["Name"], Type.Name, pAllocator);

    if (Json.contains("Definition"))
        Deserialize(Json["Definition"], Type.Definition, pAllocator);
}

inline void Serialize(nlohmann::json& Json, const ShaderCreateInfo& Type, DeviceObjectReflection* pAllocator)
{
    if (!CompareStr(Type.FilePath, ShaderCreateInfo{}.FilePath))
        Serialize(Json["FilePath"], Type.FilePath, pAllocator);

    if (!CompareStr(Type.Source, ShaderCreateInfo{}.Source))
        Serialize(Json["Source"], Type.Source, pAllocator);

    if (!(Type.ByteCode == ShaderCreateInfo{}.ByteCode))
        Serialize(Json["ByteCode"], Type.ByteCode, Type.ByteCodeSize, pAllocator);

    if (!(Type.SourceLength == ShaderCreateInfo{}.SourceLength))
        Serialize(Json["SourceLength"], Type.SourceLength, pAllocator);

    if (!(Type.ByteCodeSize == ShaderCreateInfo{}.ByteCodeSize))
        Serialize(Json["ByteCodeSize"], Type.ByteCodeSize, pAllocator);

    if (!CompareStr(Type.EntryPoint, ShaderCreateInfo{}.EntryPoint))
        Serialize(Json["EntryPoint"], Type.EntryPoint, pAllocator);

    if (!(Type.Macros == ShaderCreateInfo{}.Macros))
        Serialize(Json["Macros"], Type.Macros, pAllocator);

    if (!(Type.UseCombinedTextureSamplers == ShaderCreateInfo{}.UseCombinedTextureSamplers))
        Serialize(Json["UseCombinedTextureSamplers"], Type.UseCombinedTextureSamplers, pAllocator);

    if (!CompareStr(Type.CombinedSamplerSuffix, ShaderCreateInfo{}.CombinedSamplerSuffix))
        Serialize(Json["CombinedSamplerSuffix"], Type.CombinedSamplerSuffix, pAllocator);

    if (!(Type.Desc == ShaderCreateInfo{}.Desc))
        Serialize(Json["Desc"], Type.Desc, pAllocator);

    if (!(Type.SourceLanguage == ShaderCreateInfo{}.SourceLanguage))
        Serialize(Json["SourceLanguage"], Type.SourceLanguage, pAllocator);

    if (!(Type.ShaderCompiler == ShaderCreateInfo{}.ShaderCompiler))
        Serialize(Json["ShaderCompiler"], Type.ShaderCompiler, pAllocator);

    if (!(Type.HLSLVersion == ShaderCreateInfo{}.HLSLVersion))
        Serialize(Json["HLSLVersion"], Type.HLSLVersion, pAllocator);

    if (!(Type.GLSLVersion == ShaderCreateInfo{}.GLSLVersion))
        Serialize(Json["GLSLVersion"], Type.GLSLVersion, pAllocator);

    if (!(Type.GLESSLVersion == ShaderCreateInfo{}.GLESSLVersion))
        Serialize(Json["GLESSLVersion"], Type.GLESSLVersion, pAllocator);

    if (!(Type.CompileFlags == ShaderCreateInfo{}.CompileFlags))
        Serialize(Json["CompileFlags"], Type.CompileFlags, pAllocator);
}

inline void Deserialize(const nlohmann::json& Json, ShaderCreateInfo& Type, DeviceObjectReflection* pAllocator)
{
    if (Json.contains("FilePath"))
        Deserialize(Json["FilePath"], Type.FilePath, pAllocator);

    if (Json.contains("Source"))
        Deserialize(Json["Source"], Type.Source, pAllocator);

    if (Json.contains("ByteCode"))
        Deserialize(Json["ByteCode"], Type.ByteCode, Type.ByteCodeSize, pAllocator);

    if (Json.contains("SourceLength"))
        Deserialize(Json["SourceLength"], Type.SourceLength, pAllocator);

    if (Json.contains("ByteCodeSize"))
        Deserialize(Json["ByteCodeSize"], Type.ByteCodeSize, pAllocator);

    if (Json.contains("EntryPoint"))
        Deserialize(Json["EntryPoint"], Type.EntryPoint, pAllocator);

    if (Json.contains("Macros"))
        Deserialize(Json["Macros"], Type.Macros, pAllocator);

    if (Json.contains("UseCombinedTextureSamplers"))
        Deserialize(Json["UseCombinedTextureSamplers"], Type.UseCombinedTextureSamplers, pAllocator);

    if (Json.contains("CombinedSamplerSuffix"))
        Deserialize(Json["CombinedSamplerSuffix"], Type.CombinedSamplerSuffix, pAllocator);

    if (Json.contains("Desc"))
        Deserialize(Json["Desc"], Type.Desc, pAllocator);

    if (Json.contains("SourceLanguage"))
        Deserialize(Json["SourceLanguage"], Type.SourceLanguage, pAllocator);

    if (Json.contains("ShaderCompiler"))
        Deserialize(Json["ShaderCompiler"], Type.ShaderCompiler, pAllocator);

    if (Json.contains("HLSLVersion"))
        Deserialize(Json["HLSLVersion"], Type.HLSLVersion, pAllocator);

    if (Json.contains("GLSLVersion"))
        Deserialize(Json["GLSLVersion"], Type.GLSLVersion, pAllocator);

    if (Json.contains("GLESSLVersion"))
        Deserialize(Json["GLESSLVersion"], Type.GLESSLVersion, pAllocator);

    if (Json.contains("CompileFlags"))
        Deserialize(Json["CompileFlags"], Type.CompileFlags, pAllocator);
}

inline void Serialize(nlohmann::json& Json, const ShaderResourceDesc& Type, DeviceObjectReflection* pAllocator)
{
    if (!CompareStr(Type.Name, ShaderResourceDesc{}.Name))
        Serialize(Json["Name"], Type.Name, pAllocator);

    if (!(Type.Type == ShaderResourceDesc{}.Type))
        Serialize(Json["Type"], Type.Type, pAllocator);

    if (!(Type.ArraySize == ShaderResourceDesc{}.ArraySize))
        Serialize(Json["ArraySize"], Type.ArraySize, pAllocator);
}

inline void Deserialize(const nlohmann::json& Json, ShaderResourceDesc& Type, DeviceObjectReflection* pAllocator)
{
    if (Json.contains("Name"))
        Deserialize(Json["Name"], Type.Name, pAllocator);

    if (Json.contains("Type"))
        Deserialize(Json["Type"], Type.Type, pAllocator);

    if (Json.contains("ArraySize"))
        Deserialize(Json["ArraySize"], Type.ArraySize, pAllocator);
}

} // namespace Diligent
