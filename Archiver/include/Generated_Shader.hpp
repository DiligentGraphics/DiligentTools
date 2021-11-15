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

inline void to_json(nlohmann::json& Json, const ShaderDesc& Type)
{
    nlohmann::to_json(Json, static_cast<DeviceObjectAttribs>(Type));

    if (!(Type.ShaderType == ShaderDesc{}.ShaderType))
    {
        to_json_bitwise(Json["ShaderType"], Type.ShaderType);
    }
}

inline void from_json(const nlohmann::json& Json, ShaderDesc& Type)
{
    nlohmann::from_json(Json, static_cast<DeviceObjectAttribs&>(Type));

    if (Json.contains("ShaderType"))
    {
        from_json_bitwise(Json["ShaderType"], Type.ShaderType);
    }
}

inline void to_json(nlohmann::json& Json, const ShaderMacro& Type)
{
    if (!CompareStr(Type.Name, ShaderMacro{}.Name))
    {
        Json["Name"] = Type.Name;
    }

    if (!CompareStr(Type.Definition, ShaderMacro{}.Definition))
    {
        Json["Definition"] = Type.Definition;
    }
}

inline void from_json(const nlohmann::json& Json, ShaderMacro& Type)
{
    if (Json.contains("Name"))
    {
        Type.Name = copy_string(Json["Name"].get<std::string>());
    }

    if (Json.contains("Definition"))
    {
        Type.Definition = copy_string(Json["Definition"].get<std::string>());
    }
}

inline void to_json(nlohmann::json& Json, const ShaderCreateInfo& Type)
{
    if (!CompareStr(Type.FilePath, ShaderCreateInfo{}.FilePath))
    {
        Json["FilePath"] = Type.FilePath;
    }

    if (!CompareStr(Type.Source, ShaderCreateInfo{}.Source))
    {
        Json["Source"] = Type.Source;
    }

    if (!(Type.ByteCode == ShaderCreateInfo{}.ByteCode))
    {
        to_json_ptr(Json["ByteCode"], Type.ByteCode, Type.ByteCodeSize);
    }

    if (!(Type.SourceLength == ShaderCreateInfo{}.SourceLength))
    {
        Json["SourceLength"] = Type.SourceLength;
    }

    if (!(Type.ByteCodeSize == ShaderCreateInfo{}.ByteCodeSize))
    {
        Json["ByteCodeSize"] = Type.ByteCodeSize;
    }

    if (!CompareStr(Type.EntryPoint, ShaderCreateInfo{}.EntryPoint))
    {
        Json["EntryPoint"] = Type.EntryPoint;
    }

    if (!(Type.Macros == ShaderCreateInfo{}.Macros))
    {
        to_json_ptr(Json["Macros"], Type.Macros);
    }

    if (!(Type.UseCombinedTextureSamplers == ShaderCreateInfo{}.UseCombinedTextureSamplers))
    {
        Json["UseCombinedTextureSamplers"] = Type.UseCombinedTextureSamplers;
    }

    if (!CompareStr(Type.CombinedSamplerSuffix, ShaderCreateInfo{}.CombinedSamplerSuffix))
    {
        Json["CombinedSamplerSuffix"] = Type.CombinedSamplerSuffix;
    }

    if (!(Type.Desc == ShaderCreateInfo{}.Desc))
    {
        Json["Desc"] = Type.Desc;
    }

    if (!(Type.SourceLanguage == ShaderCreateInfo{}.SourceLanguage))
    {
        Json["SourceLanguage"] = Type.SourceLanguage;
    }

    if (!(Type.ShaderCompiler == ShaderCreateInfo{}.ShaderCompiler))
    {
        Json["ShaderCompiler"] = Type.ShaderCompiler;
    }

    if (!(Type.HLSLVersion == ShaderCreateInfo{}.HLSLVersion))
    {
        Json["HLSLVersion"] = Type.HLSLVersion;
    }

    if (!(Type.GLSLVersion == ShaderCreateInfo{}.GLSLVersion))
    {
        Json["GLSLVersion"] = Type.GLSLVersion;
    }

    if (!(Type.GLESSLVersion == ShaderCreateInfo{}.GLESSLVersion))
    {
        Json["GLESSLVersion"] = Type.GLESSLVersion;
    }

    if (!(Type.CompileFlags == ShaderCreateInfo{}.CompileFlags))
    {
        Json["CompileFlags"] = Type.CompileFlags;
    }
}

inline void from_json(const nlohmann::json& Json, ShaderCreateInfo& Type)
{
    if (Json.contains("FilePath"))
    {
        Type.FilePath = copy_string(Json["FilePath"].get<std::string>());
    }

    if (Json.contains("Source"))
    {
        Type.Source = copy_string(Json["Source"].get<std::string>());
    }

    if (Json.contains("ByteCode"))
    {
        from_json_ptr(Json["ByteCode"], remove_const(&Type.ByteCode), Json.at("ByteCodeSize"));
    }

    if (Json.contains("SourceLength"))
    {
        Json["SourceLength"].get_to(Type.SourceLength);
    }

    if (Json.contains("ByteCodeSize"))
    {
        Json["ByteCodeSize"].get_to(Type.ByteCodeSize);
    }

    if (Json.contains("EntryPoint"))
    {
        Type.EntryPoint = copy_string(Json["EntryPoint"].get<std::string>());
    }

    if (Json.contains("Macros"))
    {
        from_json_ptr(Json["Macros"], remove_const(&Type.Macros));
    }

    if (Json.contains("UseCombinedTextureSamplers"))
    {
        Json["UseCombinedTextureSamplers"].get_to(Type.UseCombinedTextureSamplers);
    }

    if (Json.contains("CombinedSamplerSuffix"))
    {
        Type.CombinedSamplerSuffix = copy_string(Json["CombinedSamplerSuffix"].get<std::string>());
    }

    if (Json.contains("Desc"))
    {
        Json["Desc"].get_to(Type.Desc);
    }

    if (Json.contains("SourceLanguage"))
    {
        Json["SourceLanguage"].get_to(Type.SourceLanguage);
    }

    if (Json.contains("ShaderCompiler"))
    {
        Json["ShaderCompiler"].get_to(Type.ShaderCompiler);
    }

    if (Json.contains("HLSLVersion"))
    {
        Json["HLSLVersion"].get_to(Type.HLSLVersion);
    }

    if (Json.contains("GLSLVersion"))
    {
        Json["GLSLVersion"].get_to(Type.GLSLVersion);
    }

    if (Json.contains("GLESSLVersion"))
    {
        Json["GLESSLVersion"].get_to(Type.GLESSLVersion);
    }

    if (Json.contains("CompileFlags"))
    {
        Json["CompileFlags"].get_to(Type.CompileFlags);
    }
}

inline void to_json(nlohmann::json& Json, const ShaderResourceDesc& Type)
{
    if (!CompareStr(Type.Name, ShaderResourceDesc{}.Name))
    {
        Json["Name"] = Type.Name;
    }

    if (!(Type.Type == ShaderResourceDesc{}.Type))
    {
        Json["Type"] = Type.Type;
    }

    if (!(Type.ArraySize == ShaderResourceDesc{}.ArraySize))
    {
        Json["ArraySize"] = Type.ArraySize;
    }
}

inline void from_json(const nlohmann::json& Json, ShaderResourceDesc& Type)
{
    if (Json.contains("Name"))
    {
        Type.Name = copy_string(Json["Name"].get<std::string>());
    }

    if (Json.contains("Type"))
    {
        Json["Type"].get_to(Type.Type);
    }

    if (Json.contains("ArraySize"))
    {
        Json["ArraySize"].get_to(Type.ArraySize);
    }
}

} // namespace Diligent
