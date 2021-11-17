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

#include "GraphicsTypes.h"

namespace Diligent
{

NLOHMANN_JSON_SERIALIZE_ENUM(
    VALUE_TYPE,
    {
        {VT_UNDEFINED, "UNDEFINED"},
        {VT_INT8, "INT8"},
        {VT_INT16, "INT16"},
        {VT_INT32, "INT32"},
        {VT_UINT8, "UINT8"},
        {VT_UINT16, "UINT16"},
        {VT_UINT32, "UINT32"},
        {VT_FLOAT16, "FLOAT16"},
        {VT_FLOAT32, "FLOAT32"},
        {VT_NUM_TYPES, "NUM_TYPES"},
    })

NLOHMANN_JSON_SERIALIZE_ENUM(
    SHADER_TYPE,
    {
        {SHADER_TYPE_UNKNOWN, "UNKNOWN"},
        {SHADER_TYPE_VERTEX, "VERTEX"},
        {SHADER_TYPE_PIXEL, "PIXEL"},
        {SHADER_TYPE_GEOMETRY, "GEOMETRY"},
        {SHADER_TYPE_HULL, "HULL"},
        {SHADER_TYPE_DOMAIN, "DOMAIN"},
        {SHADER_TYPE_COMPUTE, "COMPUTE"},
        {SHADER_TYPE_AMPLIFICATION, "AMPLIFICATION"},
        {SHADER_TYPE_MESH, "MESH"},
        {SHADER_TYPE_RAY_GEN, "RAY_GEN"},
        {SHADER_TYPE_RAY_MISS, "RAY_MISS"},
        {SHADER_TYPE_RAY_CLOSEST_HIT, "RAY_CLOSEST_HIT"},
        {SHADER_TYPE_RAY_ANY_HIT, "RAY_ANY_HIT"},
        {SHADER_TYPE_RAY_INTERSECTION, "RAY_INTERSECTION"},
        {SHADER_TYPE_CALLABLE, "CALLABLE"},
        {SHADER_TYPE_TILE, "TILE"},
        {SHADER_TYPE_LAST, "LAST"},
        {SHADER_TYPE_ALL_GRAPHICS, "ALL_GRAPHICS"},
        {SHADER_TYPE_ALL_MESH, "ALL_MESH"},
        {SHADER_TYPE_ALL_RAY_TRACING, "ALL_RAY_TRACING"},
    })

NLOHMANN_JSON_SERIALIZE_ENUM(
    TEXTURE_FORMAT,
    {
        {TEX_FORMAT_UNKNOWN, "UNKNOWN"},
        {TEX_FORMAT_RGBA32_TYPELESS, "RGBA32_TYPELESS"},
        {TEX_FORMAT_RGBA32_FLOAT, "RGBA32_FLOAT"},
        {TEX_FORMAT_RGBA32_UINT, "RGBA32_UINT"},
        {TEX_FORMAT_RGBA32_SINT, "RGBA32_SINT"},
        {TEX_FORMAT_RGB32_TYPELESS, "RGB32_TYPELESS"},
        {TEX_FORMAT_RGB32_FLOAT, "RGB32_FLOAT"},
        {TEX_FORMAT_RGB32_UINT, "RGB32_UINT"},
        {TEX_FORMAT_RGB32_SINT, "RGB32_SINT"},
        {TEX_FORMAT_RGBA16_TYPELESS, "RGBA16_TYPELESS"},
        {TEX_FORMAT_RGBA16_FLOAT, "RGBA16_FLOAT"},
        {TEX_FORMAT_RGBA16_UNORM, "RGBA16_UNORM"},
        {TEX_FORMAT_RGBA16_UINT, "RGBA16_UINT"},
        {TEX_FORMAT_RGBA16_SNORM, "RGBA16_SNORM"},
        {TEX_FORMAT_RGBA16_SINT, "RGBA16_SINT"},
        {TEX_FORMAT_RG32_TYPELESS, "RG32_TYPELESS"},
        {TEX_FORMAT_RG32_FLOAT, "RG32_FLOAT"},
        {TEX_FORMAT_RG32_UINT, "RG32_UINT"},
        {TEX_FORMAT_RG32_SINT, "RG32_SINT"},
        {TEX_FORMAT_R32G8X24_TYPELESS, "R32G8X24_TYPELESS"},
        {TEX_FORMAT_D32_FLOAT_S8X24_UINT, "D32_FLOAT_S8X24_UINT"},
        {TEX_FORMAT_R32_FLOAT_X8X24_TYPELESS, "R32_FLOAT_X8X24_TYPELESS"},
        {TEX_FORMAT_X32_TYPELESS_G8X24_UINT, "X32_TYPELESS_G8X24_UINT"},
        {TEX_FORMAT_RGB10A2_TYPELESS, "RGB10A2_TYPELESS"},
        {TEX_FORMAT_RGB10A2_UNORM, "RGB10A2_UNORM"},
        {TEX_FORMAT_RGB10A2_UINT, "RGB10A2_UINT"},
        {TEX_FORMAT_R11G11B10_FLOAT, "R11G11B10_FLOAT"},
        {TEX_FORMAT_RGBA8_TYPELESS, "RGBA8_TYPELESS"},
        {TEX_FORMAT_RGBA8_UNORM, "RGBA8_UNORM"},
        {TEX_FORMAT_RGBA8_UNORM_SRGB, "RGBA8_UNORM_SRGB"},
        {TEX_FORMAT_RGBA8_UINT, "RGBA8_UINT"},
        {TEX_FORMAT_RGBA8_SNORM, "RGBA8_SNORM"},
        {TEX_FORMAT_RGBA8_SINT, "RGBA8_SINT"},
        {TEX_FORMAT_RG16_TYPELESS, "RG16_TYPELESS"},
        {TEX_FORMAT_RG16_FLOAT, "RG16_FLOAT"},
        {TEX_FORMAT_RG16_UNORM, "RG16_UNORM"},
        {TEX_FORMAT_RG16_UINT, "RG16_UINT"},
        {TEX_FORMAT_RG16_SNORM, "RG16_SNORM"},
        {TEX_FORMAT_RG16_SINT, "RG16_SINT"},
        {TEX_FORMAT_R32_TYPELESS, "R32_TYPELESS"},
        {TEX_FORMAT_D32_FLOAT, "D32_FLOAT"},
        {TEX_FORMAT_R32_FLOAT, "R32_FLOAT"},
        {TEX_FORMAT_R32_UINT, "R32_UINT"},
        {TEX_FORMAT_R32_SINT, "R32_SINT"},
        {TEX_FORMAT_R24G8_TYPELESS, "R24G8_TYPELESS"},
        {TEX_FORMAT_D24_UNORM_S8_UINT, "D24_UNORM_S8_UINT"},
        {TEX_FORMAT_R24_UNORM_X8_TYPELESS, "R24_UNORM_X8_TYPELESS"},
        {TEX_FORMAT_X24_TYPELESS_G8_UINT, "X24_TYPELESS_G8_UINT"},
        {TEX_FORMAT_RG8_TYPELESS, "RG8_TYPELESS"},
        {TEX_FORMAT_RG8_UNORM, "RG8_UNORM"},
        {TEX_FORMAT_RG8_UINT, "RG8_UINT"},
        {TEX_FORMAT_RG8_SNORM, "RG8_SNORM"},
        {TEX_FORMAT_RG8_SINT, "RG8_SINT"},
        {TEX_FORMAT_R16_TYPELESS, "R16_TYPELESS"},
        {TEX_FORMAT_R16_FLOAT, "R16_FLOAT"},
        {TEX_FORMAT_D16_UNORM, "D16_UNORM"},
        {TEX_FORMAT_R16_UNORM, "R16_UNORM"},
        {TEX_FORMAT_R16_UINT, "R16_UINT"},
        {TEX_FORMAT_R16_SNORM, "R16_SNORM"},
        {TEX_FORMAT_R16_SINT, "R16_SINT"},
        {TEX_FORMAT_R8_TYPELESS, "R8_TYPELESS"},
        {TEX_FORMAT_R8_UNORM, "R8_UNORM"},
        {TEX_FORMAT_R8_UINT, "R8_UINT"},
        {TEX_FORMAT_R8_SNORM, "R8_SNORM"},
        {TEX_FORMAT_R8_SINT, "R8_SINT"},
        {TEX_FORMAT_A8_UNORM, "A8_UNORM"},
        {TEX_FORMAT_R1_UNORM, "R1_UNORM"},
        {TEX_FORMAT_RGB9E5_SHAREDEXP, "RGB9E5_SHAREDEXP"},
        {TEX_FORMAT_RG8_B8G8_UNORM, "RG8_B8G8_UNORM"},
        {TEX_FORMAT_G8R8_G8B8_UNORM, "G8R8_G8B8_UNORM"},
        {TEX_FORMAT_BC1_TYPELESS, "BC1_TYPELESS"},
        {TEX_FORMAT_BC1_UNORM, "BC1_UNORM"},
        {TEX_FORMAT_BC1_UNORM_SRGB, "BC1_UNORM_SRGB"},
        {TEX_FORMAT_BC2_TYPELESS, "BC2_TYPELESS"},
        {TEX_FORMAT_BC2_UNORM, "BC2_UNORM"},
        {TEX_FORMAT_BC2_UNORM_SRGB, "BC2_UNORM_SRGB"},
        {TEX_FORMAT_BC3_TYPELESS, "BC3_TYPELESS"},
        {TEX_FORMAT_BC3_UNORM, "BC3_UNORM"},
        {TEX_FORMAT_BC3_UNORM_SRGB, "BC3_UNORM_SRGB"},
        {TEX_FORMAT_BC4_TYPELESS, "BC4_TYPELESS"},
        {TEX_FORMAT_BC4_UNORM, "BC4_UNORM"},
        {TEX_FORMAT_BC4_SNORM, "BC4_SNORM"},
        {TEX_FORMAT_BC5_TYPELESS, "BC5_TYPELESS"},
        {TEX_FORMAT_BC5_UNORM, "BC5_UNORM"},
        {TEX_FORMAT_BC5_SNORM, "BC5_SNORM"},
        {TEX_FORMAT_B5G6R5_UNORM, "B5G6R5_UNORM"},
        {TEX_FORMAT_B5G5R5A1_UNORM, "B5G5R5A1_UNORM"},
        {TEX_FORMAT_BGRA8_UNORM, "BGRA8_UNORM"},
        {TEX_FORMAT_BGRX8_UNORM, "BGRX8_UNORM"},
        {TEX_FORMAT_R10G10B10_XR_BIAS_A2_UNORM, "R10G10B10_XR_BIAS_A2_UNORM"},
        {TEX_FORMAT_BGRA8_TYPELESS, "BGRA8_TYPELESS"},
        {TEX_FORMAT_BGRA8_UNORM_SRGB, "BGRA8_UNORM_SRGB"},
        {TEX_FORMAT_BGRX8_TYPELESS, "BGRX8_TYPELESS"},
        {TEX_FORMAT_BGRX8_UNORM_SRGB, "BGRX8_UNORM_SRGB"},
        {TEX_FORMAT_BC6H_TYPELESS, "BC6H_TYPELESS"},
        {TEX_FORMAT_BC6H_UF16, "BC6H_UF16"},
        {TEX_FORMAT_BC6H_SF16, "BC6H_SF16"},
        {TEX_FORMAT_BC7_TYPELESS, "BC7_TYPELESS"},
        {TEX_FORMAT_BC7_UNORM, "BC7_UNORM"},
        {TEX_FORMAT_BC7_UNORM_SRGB, "BC7_UNORM_SRGB"},
        {TEX_FORMAT_NUM_FORMATS, "NUM_FORMATS"},
    })

NLOHMANN_JSON_SERIALIZE_ENUM(
    COMPARISON_FUNCTION,
    {
        {COMPARISON_FUNC_UNKNOWN, "UNKNOWN"},
        {COMPARISON_FUNC_NEVER, "NEVER"},
        {COMPARISON_FUNC_LESS, "LESS"},
        {COMPARISON_FUNC_EQUAL, "EQUAL"},
        {COMPARISON_FUNC_LESS_EQUAL, "LESS_EQUAL"},
        {COMPARISON_FUNC_GREATER, "GREATER"},
        {COMPARISON_FUNC_NOT_EQUAL, "NOT_EQUAL"},
        {COMPARISON_FUNC_GREATER_EQUAL, "GREATER_EQUAL"},
        {COMPARISON_FUNC_ALWAYS, "ALWAYS"},
        {COMPARISON_FUNC_NUM_FUNCTIONS, "NUM_FUNCTIONS"},
    })

NLOHMANN_JSON_SERIALIZE_ENUM(
    PRIMITIVE_TOPOLOGY,
    {
        {PRIMITIVE_TOPOLOGY_UNDEFINED, "UNDEFINED"},
        {PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, "TRIANGLE_LIST"},
        {PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP, "TRIANGLE_STRIP"},
        {PRIMITIVE_TOPOLOGY_POINT_LIST, "POINT_LIST"},
        {PRIMITIVE_TOPOLOGY_LINE_LIST, "LINE_LIST"},
        {PRIMITIVE_TOPOLOGY_LINE_STRIP, "LINE_STRIP"},
        {PRIMITIVE_TOPOLOGY_1_CONTROL_POINT_PATCHLIST, "1_CONTROL_POINT_PATCHLIST"},
        {PRIMITIVE_TOPOLOGY_2_CONTROL_POINT_PATCHLIST, "2_CONTROL_POINT_PATCHLIST"},
        {PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST, "3_CONTROL_POINT_PATCHLIST"},
        {PRIMITIVE_TOPOLOGY_4_CONTROL_POINT_PATCHLIST, "4_CONTROL_POINT_PATCHLIST"},
        {PRIMITIVE_TOPOLOGY_5_CONTROL_POINT_PATCHLIST, "5_CONTROL_POINT_PATCHLIST"},
        {PRIMITIVE_TOPOLOGY_6_CONTROL_POINT_PATCHLIST, "6_CONTROL_POINT_PATCHLIST"},
        {PRIMITIVE_TOPOLOGY_7_CONTROL_POINT_PATCHLIST, "7_CONTROL_POINT_PATCHLIST"},
        {PRIMITIVE_TOPOLOGY_8_CONTROL_POINT_PATCHLIST, "8_CONTROL_POINT_PATCHLIST"},
        {PRIMITIVE_TOPOLOGY_9_CONTROL_POINT_PATCHLIST, "9_CONTROL_POINT_PATCHLIST"},
        {PRIMITIVE_TOPOLOGY_10_CONTROL_POINT_PATCHLIST, "10_CONTROL_POINT_PATCHLIST"},
        {PRIMITIVE_TOPOLOGY_11_CONTROL_POINT_PATCHLIST, "11_CONTROL_POINT_PATCHLIST"},
        {PRIMITIVE_TOPOLOGY_12_CONTROL_POINT_PATCHLIST, "12_CONTROL_POINT_PATCHLIST"},
        {PRIMITIVE_TOPOLOGY_13_CONTROL_POINT_PATCHLIST, "13_CONTROL_POINT_PATCHLIST"},
        {PRIMITIVE_TOPOLOGY_14_CONTROL_POINT_PATCHLIST, "14_CONTROL_POINT_PATCHLIST"},
        {PRIMITIVE_TOPOLOGY_15_CONTROL_POINT_PATCHLIST, "15_CONTROL_POINT_PATCHLIST"},
        {PRIMITIVE_TOPOLOGY_16_CONTROL_POINT_PATCHLIST, "16_CONTROL_POINT_PATCHLIST"},
        {PRIMITIVE_TOPOLOGY_17_CONTROL_POINT_PATCHLIST, "17_CONTROL_POINT_PATCHLIST"},
        {PRIMITIVE_TOPOLOGY_18_CONTROL_POINT_PATCHLIST, "18_CONTROL_POINT_PATCHLIST"},
        {PRIMITIVE_TOPOLOGY_19_CONTROL_POINT_PATCHLIST, "19_CONTROL_POINT_PATCHLIST"},
        {PRIMITIVE_TOPOLOGY_20_CONTROL_POINT_PATCHLIST, "20_CONTROL_POINT_PATCHLIST"},
        {PRIMITIVE_TOPOLOGY_21_CONTROL_POINT_PATCHLIST, "21_CONTROL_POINT_PATCHLIST"},
        {PRIMITIVE_TOPOLOGY_22_CONTROL_POINT_PATCHLIST, "22_CONTROL_POINT_PATCHLIST"},
        {PRIMITIVE_TOPOLOGY_23_CONTROL_POINT_PATCHLIST, "23_CONTROL_POINT_PATCHLIST"},
        {PRIMITIVE_TOPOLOGY_24_CONTROL_POINT_PATCHLIST, "24_CONTROL_POINT_PATCHLIST"},
        {PRIMITIVE_TOPOLOGY_25_CONTROL_POINT_PATCHLIST, "25_CONTROL_POINT_PATCHLIST"},
        {PRIMITIVE_TOPOLOGY_26_CONTROL_POINT_PATCHLIST, "26_CONTROL_POINT_PATCHLIST"},
        {PRIMITIVE_TOPOLOGY_27_CONTROL_POINT_PATCHLIST, "27_CONTROL_POINT_PATCHLIST"},
        {PRIMITIVE_TOPOLOGY_28_CONTROL_POINT_PATCHLIST, "28_CONTROL_POINT_PATCHLIST"},
        {PRIMITIVE_TOPOLOGY_29_CONTROL_POINT_PATCHLIST, "29_CONTROL_POINT_PATCHLIST"},
        {PRIMITIVE_TOPOLOGY_30_CONTROL_POINT_PATCHLIST, "30_CONTROL_POINT_PATCHLIST"},
        {PRIMITIVE_TOPOLOGY_31_CONTROL_POINT_PATCHLIST, "31_CONTROL_POINT_PATCHLIST"},
        {PRIMITIVE_TOPOLOGY_32_CONTROL_POINT_PATCHLIST, "32_CONTROL_POINT_PATCHLIST"},
        {PRIMITIVE_TOPOLOGY_NUM_TOPOLOGIES, "NUM_TOPOLOGIES"},
    })

NLOHMANN_JSON_SERIALIZE_ENUM(
    PIPELINE_STAGE_FLAGS,
    {
        {PIPELINE_STAGE_FLAG_UNDEFINED, "UNDEFINED"},
        {PIPELINE_STAGE_FLAG_TOP_OF_PIPE, "TOP_OF_PIPE"},
        {PIPELINE_STAGE_FLAG_DRAW_INDIRECT, "DRAW_INDIRECT"},
        {PIPELINE_STAGE_FLAG_VERTEX_INPUT, "VERTEX_INPUT"},
        {PIPELINE_STAGE_FLAG_VERTEX_SHADER, "VERTEX_SHADER"},
        {PIPELINE_STAGE_FLAG_HULL_SHADER, "HULL_SHADER"},
        {PIPELINE_STAGE_FLAG_DOMAIN_SHADER, "DOMAIN_SHADER"},
        {PIPELINE_STAGE_FLAG_GEOMETRY_SHADER, "GEOMETRY_SHADER"},
        {PIPELINE_STAGE_FLAG_PIXEL_SHADER, "PIXEL_SHADER"},
        {PIPELINE_STAGE_FLAG_EARLY_FRAGMENT_TESTS, "EARLY_FRAGMENT_TESTS"},
        {PIPELINE_STAGE_FLAG_LATE_FRAGMENT_TESTS, "LATE_FRAGMENT_TESTS"},
        {PIPELINE_STAGE_FLAG_RENDER_TARGET, "RENDER_TARGET"},
        {PIPELINE_STAGE_FLAG_COMPUTE_SHADER, "COMPUTE_SHADER"},
        {PIPELINE_STAGE_FLAG_TRANSFER, "TRANSFER"},
        {PIPELINE_STAGE_FLAG_BOTTOM_OF_PIPE, "BOTTOM_OF_PIPE"},
        {PIPELINE_STAGE_FLAG_HOST, "HOST"},
        {PIPELINE_STAGE_FLAG_CONDITIONAL_RENDERING, "CONDITIONAL_RENDERING"},
        {PIPELINE_STAGE_FLAG_SHADING_RATE_TEXTURE, "SHADING_RATE_TEXTURE"},
        {PIPELINE_STAGE_FLAG_RAY_TRACING_SHADER, "RAY_TRACING_SHADER"},
        {PIPELINE_STAGE_FLAG_ACCELERATION_STRUCTURE_BUILD, "ACCELERATION_STRUCTURE_BUILD"},
        {PIPELINE_STAGE_FLAG_TASK_SHADER, "TASK_SHADER"},
        {PIPELINE_STAGE_FLAG_MESH_SHADER, "MESH_SHADER"},
        {PIPELINE_STAGE_FLAG_FRAGMENT_DENSITY_PROCESS, "FRAGMENT_DENSITY_PROCESS"},
        {PIPELINE_STAGE_FLAG_DEFAULT, "DEFAULT"},
    })

NLOHMANN_JSON_SERIALIZE_ENUM(
    ACCESS_FLAGS,
    {
        {ACCESS_FLAG_NONE, "NONE"},
        {ACCESS_FLAG_INDIRECT_COMMAND_READ, "INDIRECT_COMMAND_READ"},
        {ACCESS_FLAG_INDEX_READ, "INDEX_READ"},
        {ACCESS_FLAG_VERTEX_READ, "VERTEX_READ"},
        {ACCESS_FLAG_UNIFORM_READ, "UNIFORM_READ"},
        {ACCESS_FLAG_INPUT_ATTACHMENT_READ, "INPUT_ATTACHMENT_READ"},
        {ACCESS_FLAG_SHADER_READ, "SHADER_READ"},
        {ACCESS_FLAG_SHADER_WRITE, "SHADER_WRITE"},
        {ACCESS_FLAG_RENDER_TARGET_READ, "RENDER_TARGET_READ"},
        {ACCESS_FLAG_RENDER_TARGET_WRITE, "RENDER_TARGET_WRITE"},
        {ACCESS_FLAG_DEPTH_STENCIL_READ, "DEPTH_STENCIL_READ"},
        {ACCESS_FLAG_DEPTH_STENCIL_WRITE, "DEPTH_STENCIL_WRITE"},
        {ACCESS_FLAG_COPY_SRC, "COPY_SRC"},
        {ACCESS_FLAG_COPY_DST, "COPY_DST"},
        {ACCESS_FLAG_HOST_READ, "HOST_READ"},
        {ACCESS_FLAG_HOST_WRITE, "HOST_WRITE"},
        {ACCESS_FLAG_MEMORY_READ, "MEMORY_READ"},
        {ACCESS_FLAG_MEMORY_WRITE, "MEMORY_WRITE"},
        {ACCESS_FLAG_CONDITIONAL_RENDERING_READ, "CONDITIONAL_RENDERING_READ"},
        {ACCESS_FLAG_SHADING_RATE_TEXTURE_READ, "SHADING_RATE_TEXTURE_READ"},
        {ACCESS_FLAG_ACCELERATION_STRUCTURE_READ, "ACCELERATION_STRUCTURE_READ"},
        {ACCESS_FLAG_ACCELERATION_STRUCTURE_WRITE, "ACCELERATION_STRUCTURE_WRITE"},
        {ACCESS_FLAG_FRAGMENT_DENSITY_MAP_READ, "FRAGMENT_DENSITY_MAP_READ"},
        {ACCESS_FLAG_DEFAULT, "DEFAULT"},
    })

NLOHMANN_JSON_SERIALIZE_ENUM(
    RESOURCE_STATE,
    {
        {RESOURCE_STATE_UNKNOWN, "UNKNOWN"},
        {RESOURCE_STATE_UNDEFINED, "UNDEFINED"},
        {RESOURCE_STATE_VERTEX_BUFFER, "VERTEX_BUFFER"},
        {RESOURCE_STATE_CONSTANT_BUFFER, "CONSTANT_BUFFER"},
        {RESOURCE_STATE_INDEX_BUFFER, "INDEX_BUFFER"},
        {RESOURCE_STATE_RENDER_TARGET, "RENDER_TARGET"},
        {RESOURCE_STATE_UNORDERED_ACCESS, "UNORDERED_ACCESS"},
        {RESOURCE_STATE_DEPTH_WRITE, "DEPTH_WRITE"},
        {RESOURCE_STATE_DEPTH_READ, "DEPTH_READ"},
        {RESOURCE_STATE_SHADER_RESOURCE, "SHADER_RESOURCE"},
        {RESOURCE_STATE_STREAM_OUT, "STREAM_OUT"},
        {RESOURCE_STATE_INDIRECT_ARGUMENT, "INDIRECT_ARGUMENT"},
        {RESOURCE_STATE_COPY_DEST, "COPY_DEST"},
        {RESOURCE_STATE_COPY_SOURCE, "COPY_SOURCE"},
        {RESOURCE_STATE_RESOLVE_DEST, "RESOLVE_DEST"},
        {RESOURCE_STATE_RESOLVE_SOURCE, "RESOLVE_SOURCE"},
        {RESOURCE_STATE_INPUT_ATTACHMENT, "INPUT_ATTACHMENT"},
        {RESOURCE_STATE_PRESENT, "PRESENT"},
        {RESOURCE_STATE_BUILD_AS_READ, "BUILD_AS_READ"},
        {RESOURCE_STATE_BUILD_AS_WRITE, "BUILD_AS_WRITE"},
        {RESOURCE_STATE_RAY_TRACING, "RAY_TRACING"},
        {RESOURCE_STATE_COMMON, "COMMON"},
        {RESOURCE_STATE_SHADING_RATE, "SHADING_RATE"},
        {RESOURCE_STATE_MAX_BIT, "MAX_BIT"},
        {RESOURCE_STATE_GENERIC_READ, "GENERIC_READ"},
    })

inline void Serialize(nlohmann::json& Json, const DeviceObjectAttribs& Type, DeviceObjectReflection* pAllocator)
{
    if (!CompareStr(Type.Name, DeviceObjectAttribs{}.Name))
        Json["Name"] = Type.Name;
}

inline void Deserialize(const nlohmann::json& Json, DeviceObjectAttribs& Type, DeviceObjectReflection* pAllocator)
{
    if (Json.contains("Name"))
        Type.Name = CopyString(Json["Name"].get<std::string>(), pAllocator);
}

inline void Serialize(nlohmann::json& Json, const Version& Type, DeviceObjectReflection* pAllocator)
{
    if (!(Type.Major == Version{}.Major))
        Serialize(Json["Major"], Type.Major, pAllocator);

    if (!(Type.Minor == Version{}.Minor))
        Serialize(Json["Minor"], Type.Minor, pAllocator);
}

inline void Deserialize(const nlohmann::json& Json, Version& Type, DeviceObjectReflection* pAllocator)
{
    if (Json.contains("Major"))
        Deserialize(Json["Major"], Type.Major, pAllocator);

    if (Json.contains("Minor"))
        Deserialize(Json["Minor"], Type.Minor, pAllocator);
}

} // namespace Diligent
