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
    ADAPTER_TYPE,
    {
        {ADAPTER_TYPE_UNKNOWN, "UNKNOWN"},
        {ADAPTER_TYPE_SOFTWARE, "SOFTWARE"},
        {ADAPTER_TYPE_INTEGRATED, "INTEGRATED"},
        {ADAPTER_TYPE_DISCRETE, "DISCRETE"},
    })

NLOHMANN_JSON_SERIALIZE_ENUM(
    RENDER_DEVICE_TYPE,
    {
        {RENDER_DEVICE_TYPE_UNDEFINED, "UNDEFINED"},
        {RENDER_DEVICE_TYPE_D3D11, "D3D11"},
        {RENDER_DEVICE_TYPE_D3D12, "D3D12"},
        {RENDER_DEVICE_TYPE_GL, "GL"},
        {RENDER_DEVICE_TYPE_GLES, "GLES"},
        {RENDER_DEVICE_TYPE_VULKAN, "VULKAN"},
        {RENDER_DEVICE_TYPE_METAL, "METAL"},
        {RENDER_DEVICE_TYPE_COUNT, "COUNT"},
    })

NLOHMANN_JSON_SERIALIZE_ENUM(
    DEVICE_FEATURE_STATE,
    {
        {DEVICE_FEATURE_STATE_DISABLED, "DISABLED"},
        {DEVICE_FEATURE_STATE_ENABLED, "ENABLED"},
        {DEVICE_FEATURE_STATE_OPTIONAL, "OPTIONAL"},
    })

NLOHMANN_JSON_SERIALIZE_ENUM(
    ADAPTER_VENDOR,
    {
        {ADAPTER_VENDOR_UNKNOWN, "UNKNOWN"},
        {ADAPTER_VENDOR_NVIDIA, "NVIDIA"},
        {ADAPTER_VENDOR_AMD, "AMD"},
        {ADAPTER_VENDOR_INTEL, "INTEL"},
        {ADAPTER_VENDOR_ARM, "ARM"},
        {ADAPTER_VENDOR_QUALCOMM, "QUALCOMM"},
        {ADAPTER_VENDOR_IMGTECH, "IMGTECH"},
        {ADAPTER_VENDOR_MSFT, "MSFT"},
        {ADAPTER_VENDOR_APPLE, "APPLE"},
        {ADAPTER_VENDOR_MESA, "MESA"},
        {ADAPTER_VENDOR_BROADCOM, "BROADCOM"},
        {ADAPTER_VENDOR_LAST, "LAST"},
    })

NLOHMANN_JSON_SERIALIZE_ENUM(
    SHADING_RATE,
    {
        {SHADING_RATE_1X1, "1X1"},
        {SHADING_RATE_1X2, "1X2"},
        {SHADING_RATE_1X4, "1X4"},
        {SHADING_RATE_2X1, "2X1"},
        {SHADING_RATE_2X2, "2X2"},
        {SHADING_RATE_2X4, "2X4"},
        {SHADING_RATE_4X1, "4X1"},
        {SHADING_RATE_4X2, "4X2"},
        {SHADING_RATE_4X4, "4X4"},
        {SHADING_RATE_MAX, "MAX"},
    })

NLOHMANN_JSON_SERIALIZE_ENUM(
    SAMPLE_COUNT,
    {
        {SAMPLE_COUNT_NONE, "NONE"},
        {SAMPLE_COUNT_1, "1"},
        {SAMPLE_COUNT_2, "2"},
        {SAMPLE_COUNT_4, "4"},
        {SAMPLE_COUNT_8, "8"},
        {SAMPLE_COUNT_16, "16"},
        {SAMPLE_COUNT_32, "32"},
        {SAMPLE_COUNT_64, "64"},
        {SAMPLE_COUNT_ALL, "ALL"},
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
        Serialize(Json["Name"], Type.Name, pAllocator);
}

inline void Deserialize(const nlohmann::json& Json, DeviceObjectAttribs& Type, DeviceObjectReflection* pAllocator)
{
    if (Json.contains("Name"))
        Deserialize(Json["Name"], Type.Name, pAllocator);
}

inline void Serialize(nlohmann::json& Json, const DeviceFeatures& Type, DeviceObjectReflection* pAllocator)
{
    if (!(Type.SeparablePrograms == DeviceFeatures{}.SeparablePrograms))
        Serialize(Json["SeparablePrograms"], Type.SeparablePrograms, pAllocator);

    if (!(Type.ShaderResourceQueries == DeviceFeatures{}.ShaderResourceQueries))
        Serialize(Json["ShaderResourceQueries"], Type.ShaderResourceQueries, pAllocator);

    if (!(Type.WireframeFill == DeviceFeatures{}.WireframeFill))
        Serialize(Json["WireframeFill"], Type.WireframeFill, pAllocator);

    if (!(Type.MultithreadedResourceCreation == DeviceFeatures{}.MultithreadedResourceCreation))
        Serialize(Json["MultithreadedResourceCreation"], Type.MultithreadedResourceCreation, pAllocator);

    if (!(Type.ComputeShaders == DeviceFeatures{}.ComputeShaders))
        Serialize(Json["ComputeShaders"], Type.ComputeShaders, pAllocator);

    if (!(Type.GeometryShaders == DeviceFeatures{}.GeometryShaders))
        Serialize(Json["GeometryShaders"], Type.GeometryShaders, pAllocator);

    if (!(Type.Tessellation == DeviceFeatures{}.Tessellation))
        Serialize(Json["Tessellation"], Type.Tessellation, pAllocator);

    if (!(Type.MeshShaders == DeviceFeatures{}.MeshShaders))
        Serialize(Json["MeshShaders"], Type.MeshShaders, pAllocator);

    if (!(Type.RayTracing == DeviceFeatures{}.RayTracing))
        Serialize(Json["RayTracing"], Type.RayTracing, pAllocator);

    if (!(Type.BindlessResources == DeviceFeatures{}.BindlessResources))
        Serialize(Json["BindlessResources"], Type.BindlessResources, pAllocator);

    if (!(Type.OcclusionQueries == DeviceFeatures{}.OcclusionQueries))
        Serialize(Json["OcclusionQueries"], Type.OcclusionQueries, pAllocator);

    if (!(Type.BinaryOcclusionQueries == DeviceFeatures{}.BinaryOcclusionQueries))
        Serialize(Json["BinaryOcclusionQueries"], Type.BinaryOcclusionQueries, pAllocator);

    if (!(Type.TimestampQueries == DeviceFeatures{}.TimestampQueries))
        Serialize(Json["TimestampQueries"], Type.TimestampQueries, pAllocator);

    if (!(Type.PipelineStatisticsQueries == DeviceFeatures{}.PipelineStatisticsQueries))
        Serialize(Json["PipelineStatisticsQueries"], Type.PipelineStatisticsQueries, pAllocator);

    if (!(Type.DurationQueries == DeviceFeatures{}.DurationQueries))
        Serialize(Json["DurationQueries"], Type.DurationQueries, pAllocator);

    if (!(Type.DepthBiasClamp == DeviceFeatures{}.DepthBiasClamp))
        Serialize(Json["DepthBiasClamp"], Type.DepthBiasClamp, pAllocator);

    if (!(Type.DepthClamp == DeviceFeatures{}.DepthClamp))
        Serialize(Json["DepthClamp"], Type.DepthClamp, pAllocator);

    if (!(Type.IndependentBlend == DeviceFeatures{}.IndependentBlend))
        Serialize(Json["IndependentBlend"], Type.IndependentBlend, pAllocator);

    if (!(Type.DualSourceBlend == DeviceFeatures{}.DualSourceBlend))
        Serialize(Json["DualSourceBlend"], Type.DualSourceBlend, pAllocator);

    if (!(Type.MultiViewport == DeviceFeatures{}.MultiViewport))
        Serialize(Json["MultiViewport"], Type.MultiViewport, pAllocator);

    if (!(Type.TextureCompressionBC == DeviceFeatures{}.TextureCompressionBC))
        Serialize(Json["TextureCompressionBC"], Type.TextureCompressionBC, pAllocator);

    if (!(Type.VertexPipelineUAVWritesAndAtomics == DeviceFeatures{}.VertexPipelineUAVWritesAndAtomics))
        Serialize(Json["VertexPipelineUAVWritesAndAtomics"], Type.VertexPipelineUAVWritesAndAtomics, pAllocator);

    if (!(Type.PixelUAVWritesAndAtomics == DeviceFeatures{}.PixelUAVWritesAndAtomics))
        Serialize(Json["PixelUAVWritesAndAtomics"], Type.PixelUAVWritesAndAtomics, pAllocator);

    if (!(Type.TextureUAVExtendedFormats == DeviceFeatures{}.TextureUAVExtendedFormats))
        Serialize(Json["TextureUAVExtendedFormats"], Type.TextureUAVExtendedFormats, pAllocator);

    if (!(Type.ShaderFloat16 == DeviceFeatures{}.ShaderFloat16))
        Serialize(Json["ShaderFloat16"], Type.ShaderFloat16, pAllocator);

    if (!(Type.ResourceBuffer16BitAccess == DeviceFeatures{}.ResourceBuffer16BitAccess))
        Serialize(Json["ResourceBuffer16BitAccess"], Type.ResourceBuffer16BitAccess, pAllocator);

    if (!(Type.UniformBuffer16BitAccess == DeviceFeatures{}.UniformBuffer16BitAccess))
        Serialize(Json["UniformBuffer16BitAccess"], Type.UniformBuffer16BitAccess, pAllocator);

    if (!(Type.ShaderInputOutput16 == DeviceFeatures{}.ShaderInputOutput16))
        Serialize(Json["ShaderInputOutput16"], Type.ShaderInputOutput16, pAllocator);

    if (!(Type.ShaderInt8 == DeviceFeatures{}.ShaderInt8))
        Serialize(Json["ShaderInt8"], Type.ShaderInt8, pAllocator);

    if (!(Type.ResourceBuffer8BitAccess == DeviceFeatures{}.ResourceBuffer8BitAccess))
        Serialize(Json["ResourceBuffer8BitAccess"], Type.ResourceBuffer8BitAccess, pAllocator);

    if (!(Type.UniformBuffer8BitAccess == DeviceFeatures{}.UniformBuffer8BitAccess))
        Serialize(Json["UniformBuffer8BitAccess"], Type.UniformBuffer8BitAccess, pAllocator);

    if (!(Type.ShaderResourceRuntimeArray == DeviceFeatures{}.ShaderResourceRuntimeArray))
        Serialize(Json["ShaderResourceRuntimeArray"], Type.ShaderResourceRuntimeArray, pAllocator);

    if (!(Type.WaveOp == DeviceFeatures{}.WaveOp))
        Serialize(Json["WaveOp"], Type.WaveOp, pAllocator);

    if (!(Type.InstanceDataStepRate == DeviceFeatures{}.InstanceDataStepRate))
        Serialize(Json["InstanceDataStepRate"], Type.InstanceDataStepRate, pAllocator);

    if (!(Type.NativeFence == DeviceFeatures{}.NativeFence))
        Serialize(Json["NativeFence"], Type.NativeFence, pAllocator);

    if (!(Type.TileShaders == DeviceFeatures{}.TileShaders))
        Serialize(Json["TileShaders"], Type.TileShaders, pAllocator);

    if (!(Type.TransferQueueTimestampQueries == DeviceFeatures{}.TransferQueueTimestampQueries))
        Serialize(Json["TransferQueueTimestampQueries"], Type.TransferQueueTimestampQueries, pAllocator);

    if (!(Type.VariableRateShading == DeviceFeatures{}.VariableRateShading))
        Serialize(Json["VariableRateShading"], Type.VariableRateShading, pAllocator);

    if (!(Type.SparseResources == DeviceFeatures{}.SparseResources))
        Serialize(Json["SparseResources"], Type.SparseResources, pAllocator);
}

inline void Deserialize(const nlohmann::json& Json, DeviceFeatures& Type, DeviceObjectReflection* pAllocator)
{
    if (Json.contains("SeparablePrograms"))
        Deserialize(Json["SeparablePrograms"], Type.SeparablePrograms, pAllocator);

    if (Json.contains("ShaderResourceQueries"))
        Deserialize(Json["ShaderResourceQueries"], Type.ShaderResourceQueries, pAllocator);

    if (Json.contains("WireframeFill"))
        Deserialize(Json["WireframeFill"], Type.WireframeFill, pAllocator);

    if (Json.contains("MultithreadedResourceCreation"))
        Deserialize(Json["MultithreadedResourceCreation"], Type.MultithreadedResourceCreation, pAllocator);

    if (Json.contains("ComputeShaders"))
        Deserialize(Json["ComputeShaders"], Type.ComputeShaders, pAllocator);

    if (Json.contains("GeometryShaders"))
        Deserialize(Json["GeometryShaders"], Type.GeometryShaders, pAllocator);

    if (Json.contains("Tessellation"))
        Deserialize(Json["Tessellation"], Type.Tessellation, pAllocator);

    if (Json.contains("MeshShaders"))
        Deserialize(Json["MeshShaders"], Type.MeshShaders, pAllocator);

    if (Json.contains("RayTracing"))
        Deserialize(Json["RayTracing"], Type.RayTracing, pAllocator);

    if (Json.contains("BindlessResources"))
        Deserialize(Json["BindlessResources"], Type.BindlessResources, pAllocator);

    if (Json.contains("OcclusionQueries"))
        Deserialize(Json["OcclusionQueries"], Type.OcclusionQueries, pAllocator);

    if (Json.contains("BinaryOcclusionQueries"))
        Deserialize(Json["BinaryOcclusionQueries"], Type.BinaryOcclusionQueries, pAllocator);

    if (Json.contains("TimestampQueries"))
        Deserialize(Json["TimestampQueries"], Type.TimestampQueries, pAllocator);

    if (Json.contains("PipelineStatisticsQueries"))
        Deserialize(Json["PipelineStatisticsQueries"], Type.PipelineStatisticsQueries, pAllocator);

    if (Json.contains("DurationQueries"))
        Deserialize(Json["DurationQueries"], Type.DurationQueries, pAllocator);

    if (Json.contains("DepthBiasClamp"))
        Deserialize(Json["DepthBiasClamp"], Type.DepthBiasClamp, pAllocator);

    if (Json.contains("DepthClamp"))
        Deserialize(Json["DepthClamp"], Type.DepthClamp, pAllocator);

    if (Json.contains("IndependentBlend"))
        Deserialize(Json["IndependentBlend"], Type.IndependentBlend, pAllocator);

    if (Json.contains("DualSourceBlend"))
        Deserialize(Json["DualSourceBlend"], Type.DualSourceBlend, pAllocator);

    if (Json.contains("MultiViewport"))
        Deserialize(Json["MultiViewport"], Type.MultiViewport, pAllocator);

    if (Json.contains("TextureCompressionBC"))
        Deserialize(Json["TextureCompressionBC"], Type.TextureCompressionBC, pAllocator);

    if (Json.contains("VertexPipelineUAVWritesAndAtomics"))
        Deserialize(Json["VertexPipelineUAVWritesAndAtomics"], Type.VertexPipelineUAVWritesAndAtomics, pAllocator);

    if (Json.contains("PixelUAVWritesAndAtomics"))
        Deserialize(Json["PixelUAVWritesAndAtomics"], Type.PixelUAVWritesAndAtomics, pAllocator);

    if (Json.contains("TextureUAVExtendedFormats"))
        Deserialize(Json["TextureUAVExtendedFormats"], Type.TextureUAVExtendedFormats, pAllocator);

    if (Json.contains("ShaderFloat16"))
        Deserialize(Json["ShaderFloat16"], Type.ShaderFloat16, pAllocator);

    if (Json.contains("ResourceBuffer16BitAccess"))
        Deserialize(Json["ResourceBuffer16BitAccess"], Type.ResourceBuffer16BitAccess, pAllocator);

    if (Json.contains("UniformBuffer16BitAccess"))
        Deserialize(Json["UniformBuffer16BitAccess"], Type.UniformBuffer16BitAccess, pAllocator);

    if (Json.contains("ShaderInputOutput16"))
        Deserialize(Json["ShaderInputOutput16"], Type.ShaderInputOutput16, pAllocator);

    if (Json.contains("ShaderInt8"))
        Deserialize(Json["ShaderInt8"], Type.ShaderInt8, pAllocator);

    if (Json.contains("ResourceBuffer8BitAccess"))
        Deserialize(Json["ResourceBuffer8BitAccess"], Type.ResourceBuffer8BitAccess, pAllocator);

    if (Json.contains("UniformBuffer8BitAccess"))
        Deserialize(Json["UniformBuffer8BitAccess"], Type.UniformBuffer8BitAccess, pAllocator);

    if (Json.contains("ShaderResourceRuntimeArray"))
        Deserialize(Json["ShaderResourceRuntimeArray"], Type.ShaderResourceRuntimeArray, pAllocator);

    if (Json.contains("WaveOp"))
        Deserialize(Json["WaveOp"], Type.WaveOp, pAllocator);

    if (Json.contains("InstanceDataStepRate"))
        Deserialize(Json["InstanceDataStepRate"], Type.InstanceDataStepRate, pAllocator);

    if (Json.contains("NativeFence"))
        Deserialize(Json["NativeFence"], Type.NativeFence, pAllocator);

    if (Json.contains("TileShaders"))
        Deserialize(Json["TileShaders"], Type.TileShaders, pAllocator);

    if (Json.contains("TransferQueueTimestampQueries"))
        Deserialize(Json["TransferQueueTimestampQueries"], Type.TransferQueueTimestampQueries, pAllocator);

    if (Json.contains("VariableRateShading"))
        Deserialize(Json["VariableRateShading"], Type.VariableRateShading, pAllocator);

    if (Json.contains("SparseResources"))
        Deserialize(Json["SparseResources"], Type.SparseResources, pAllocator);
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

inline void Serialize(nlohmann::json& Json, const TextureProperties& Type, DeviceObjectReflection* pAllocator)
{
    if (!(Type.MaxTexture1DDimension == TextureProperties{}.MaxTexture1DDimension))
        Serialize(Json["MaxTexture1DDimension"], Type.MaxTexture1DDimension, pAllocator);

    if (!(Type.MaxTexture1DArraySlices == TextureProperties{}.MaxTexture1DArraySlices))
        Serialize(Json["MaxTexture1DArraySlices"], Type.MaxTexture1DArraySlices, pAllocator);

    if (!(Type.MaxTexture2DDimension == TextureProperties{}.MaxTexture2DDimension))
        Serialize(Json["MaxTexture2DDimension"], Type.MaxTexture2DDimension, pAllocator);

    if (!(Type.MaxTexture2DArraySlices == TextureProperties{}.MaxTexture2DArraySlices))
        Serialize(Json["MaxTexture2DArraySlices"], Type.MaxTexture2DArraySlices, pAllocator);

    if (!(Type.MaxTexture3DDimension == TextureProperties{}.MaxTexture3DDimension))
        Serialize(Json["MaxTexture3DDimension"], Type.MaxTexture3DDimension, pAllocator);

    if (!(Type.MaxTextureCubeDimension == TextureProperties{}.MaxTextureCubeDimension))
        Serialize(Json["MaxTextureCubeDimension"], Type.MaxTextureCubeDimension, pAllocator);

    if (!(Type.Texture2DMSSupported == TextureProperties{}.Texture2DMSSupported))
        Serialize(Json["Texture2DMSSupported"], Type.Texture2DMSSupported, pAllocator);

    if (!(Type.Texture2DMSArraySupported == TextureProperties{}.Texture2DMSArraySupported))
        Serialize(Json["Texture2DMSArraySupported"], Type.Texture2DMSArraySupported, pAllocator);

    if (!(Type.TextureViewSupported == TextureProperties{}.TextureViewSupported))
        Serialize(Json["TextureViewSupported"], Type.TextureViewSupported, pAllocator);

    if (!(Type.CubemapArraysSupported == TextureProperties{}.CubemapArraysSupported))
        Serialize(Json["CubemapArraysSupported"], Type.CubemapArraysSupported, pAllocator);

    if (!(Type.TextureView2DOn3DSupported == TextureProperties{}.TextureView2DOn3DSupported))
        Serialize(Json["TextureView2DOn3DSupported"], Type.TextureView2DOn3DSupported, pAllocator);
}

inline void Deserialize(const nlohmann::json& Json, TextureProperties& Type, DeviceObjectReflection* pAllocator)
{
    if (Json.contains("MaxTexture1DDimension"))
        Deserialize(Json["MaxTexture1DDimension"], Type.MaxTexture1DDimension, pAllocator);

    if (Json.contains("MaxTexture1DArraySlices"))
        Deserialize(Json["MaxTexture1DArraySlices"], Type.MaxTexture1DArraySlices, pAllocator);

    if (Json.contains("MaxTexture2DDimension"))
        Deserialize(Json["MaxTexture2DDimension"], Type.MaxTexture2DDimension, pAllocator);

    if (Json.contains("MaxTexture2DArraySlices"))
        Deserialize(Json["MaxTexture2DArraySlices"], Type.MaxTexture2DArraySlices, pAllocator);

    if (Json.contains("MaxTexture3DDimension"))
        Deserialize(Json["MaxTexture3DDimension"], Type.MaxTexture3DDimension, pAllocator);

    if (Json.contains("MaxTextureCubeDimension"))
        Deserialize(Json["MaxTextureCubeDimension"], Type.MaxTextureCubeDimension, pAllocator);

    if (Json.contains("Texture2DMSSupported"))
        Deserialize(Json["Texture2DMSSupported"], Type.Texture2DMSSupported, pAllocator);

    if (Json.contains("Texture2DMSArraySupported"))
        Deserialize(Json["Texture2DMSArraySupported"], Type.Texture2DMSArraySupported, pAllocator);

    if (Json.contains("TextureViewSupported"))
        Deserialize(Json["TextureViewSupported"], Type.TextureViewSupported, pAllocator);

    if (Json.contains("CubemapArraysSupported"))
        Deserialize(Json["CubemapArraysSupported"], Type.CubemapArraysSupported, pAllocator);

    if (Json.contains("TextureView2DOn3DSupported"))
        Deserialize(Json["TextureView2DOn3DSupported"], Type.TextureView2DOn3DSupported, pAllocator);
}

inline void Serialize(nlohmann::json& Json, const SamplerProperties& Type, DeviceObjectReflection* pAllocator)
{
    if (!(Type.BorderSamplingModeSupported == SamplerProperties{}.BorderSamplingModeSupported))
        Serialize(Json["BorderSamplingModeSupported"], Type.BorderSamplingModeSupported, pAllocator);

    if (!(Type.AnisotropicFilteringSupported == SamplerProperties{}.AnisotropicFilteringSupported))
        Serialize(Json["AnisotropicFilteringSupported"], Type.AnisotropicFilteringSupported, pAllocator);

    if (!(Type.LODBiasSupported == SamplerProperties{}.LODBiasSupported))
        Serialize(Json["LODBiasSupported"], Type.LODBiasSupported, pAllocator);
}

inline void Deserialize(const nlohmann::json& Json, SamplerProperties& Type, DeviceObjectReflection* pAllocator)
{
    if (Json.contains("BorderSamplingModeSupported"))
        Deserialize(Json["BorderSamplingModeSupported"], Type.BorderSamplingModeSupported, pAllocator);

    if (Json.contains("AnisotropicFilteringSupported"))
        Deserialize(Json["AnisotropicFilteringSupported"], Type.AnisotropicFilteringSupported, pAllocator);

    if (Json.contains("LODBiasSupported"))
        Deserialize(Json["LODBiasSupported"], Type.LODBiasSupported, pAllocator);
}

inline void Serialize(nlohmann::json& Json, const WaveOpProperties& Type, DeviceObjectReflection* pAllocator)
{
    if (!(Type.MinSize == WaveOpProperties{}.MinSize))
        Serialize(Json["MinSize"], Type.MinSize, pAllocator);

    if (!(Type.MaxSize == WaveOpProperties{}.MaxSize))
        Serialize(Json["MaxSize"], Type.MaxSize, pAllocator);

    if (!(Type.SupportedStages == WaveOpProperties{}.SupportedStages))
        SerializeBitwiseEnum(Json["SupportedStages"], Type.SupportedStages, pAllocator);

    if (!(Type.Features == WaveOpProperties{}.Features))
        Serialize(Json["Features"], Type.Features, pAllocator);
}

inline void Deserialize(const nlohmann::json& Json, WaveOpProperties& Type, DeviceObjectReflection* pAllocator)
{
    if (Json.contains("MinSize"))
        Deserialize(Json["MinSize"], Type.MinSize, pAllocator);

    if (Json.contains("MaxSize"))
        Deserialize(Json["MaxSize"], Type.MaxSize, pAllocator);

    if (Json.contains("SupportedStages"))
        DeserializeBitwiseEnum(Json["SupportedStages"], Type.SupportedStages, pAllocator);

    if (Json.contains("Features"))
        Deserialize(Json["Features"], Type.Features, pAllocator);
}

inline void Serialize(nlohmann::json& Json, const BufferProperties& Type, DeviceObjectReflection* pAllocator)
{
    if (!(Type.ConstantBufferOffsetAlignment == BufferProperties{}.ConstantBufferOffsetAlignment))
        Serialize(Json["ConstantBufferOffsetAlignment"], Type.ConstantBufferOffsetAlignment, pAllocator);

    if (!(Type.StructuredBufferOffsetAlignment == BufferProperties{}.StructuredBufferOffsetAlignment))
        Serialize(Json["StructuredBufferOffsetAlignment"], Type.StructuredBufferOffsetAlignment, pAllocator);
}

inline void Deserialize(const nlohmann::json& Json, BufferProperties& Type, DeviceObjectReflection* pAllocator)
{
    if (Json.contains("ConstantBufferOffsetAlignment"))
        Deserialize(Json["ConstantBufferOffsetAlignment"], Type.ConstantBufferOffsetAlignment, pAllocator);

    if (Json.contains("StructuredBufferOffsetAlignment"))
        Deserialize(Json["StructuredBufferOffsetAlignment"], Type.StructuredBufferOffsetAlignment, pAllocator);
}

inline void Serialize(nlohmann::json& Json, const RayTracingProperties& Type, DeviceObjectReflection* pAllocator)
{
    if (!(Type.MaxRecursionDepth == RayTracingProperties{}.MaxRecursionDepth))
        Serialize(Json["MaxRecursionDepth"], Type.MaxRecursionDepth, pAllocator);

    if (!(Type.ShaderGroupHandleSize == RayTracingProperties{}.ShaderGroupHandleSize))
        Serialize(Json["ShaderGroupHandleSize"], Type.ShaderGroupHandleSize, pAllocator);

    if (!(Type.MaxShaderRecordStride == RayTracingProperties{}.MaxShaderRecordStride))
        Serialize(Json["MaxShaderRecordStride"], Type.MaxShaderRecordStride, pAllocator);

    if (!(Type.ShaderGroupBaseAlignment == RayTracingProperties{}.ShaderGroupBaseAlignment))
        Serialize(Json["ShaderGroupBaseAlignment"], Type.ShaderGroupBaseAlignment, pAllocator);

    if (!(Type.MaxRayGenThreads == RayTracingProperties{}.MaxRayGenThreads))
        Serialize(Json["MaxRayGenThreads"], Type.MaxRayGenThreads, pAllocator);

    if (!(Type.MaxInstancesPerTLAS == RayTracingProperties{}.MaxInstancesPerTLAS))
        Serialize(Json["MaxInstancesPerTLAS"], Type.MaxInstancesPerTLAS, pAllocator);

    if (!(Type.MaxPrimitivesPerBLAS == RayTracingProperties{}.MaxPrimitivesPerBLAS))
        Serialize(Json["MaxPrimitivesPerBLAS"], Type.MaxPrimitivesPerBLAS, pAllocator);

    if (!(Type.MaxGeometriesPerBLAS == RayTracingProperties{}.MaxGeometriesPerBLAS))
        Serialize(Json["MaxGeometriesPerBLAS"], Type.MaxGeometriesPerBLAS, pAllocator);

    if (!(Type.VertexBufferAlignment == RayTracingProperties{}.VertexBufferAlignment))
        Serialize(Json["VertexBufferAlignment"], Type.VertexBufferAlignment, pAllocator);

    if (!(Type.IndexBufferAlignment == RayTracingProperties{}.IndexBufferAlignment))
        Serialize(Json["IndexBufferAlignment"], Type.IndexBufferAlignment, pAllocator);

    if (!(Type.TransformBufferAlignment == RayTracingProperties{}.TransformBufferAlignment))
        Serialize(Json["TransformBufferAlignment"], Type.TransformBufferAlignment, pAllocator);

    if (!(Type.BoxBufferAlignment == RayTracingProperties{}.BoxBufferAlignment))
        Serialize(Json["BoxBufferAlignment"], Type.BoxBufferAlignment, pAllocator);

    if (!(Type.ScratchBufferAlignment == RayTracingProperties{}.ScratchBufferAlignment))
        Serialize(Json["ScratchBufferAlignment"], Type.ScratchBufferAlignment, pAllocator);

    if (!(Type.InstanceBufferAlignment == RayTracingProperties{}.InstanceBufferAlignment))
        Serialize(Json["InstanceBufferAlignment"], Type.InstanceBufferAlignment, pAllocator);

    if (!(Type.CapFlags == RayTracingProperties{}.CapFlags))
        Serialize(Json["CapFlags"], Type.CapFlags, pAllocator);
}

inline void Deserialize(const nlohmann::json& Json, RayTracingProperties& Type, DeviceObjectReflection* pAllocator)
{
    if (Json.contains("MaxRecursionDepth"))
        Deserialize(Json["MaxRecursionDepth"], Type.MaxRecursionDepth, pAllocator);

    if (Json.contains("ShaderGroupHandleSize"))
        Deserialize(Json["ShaderGroupHandleSize"], Type.ShaderGroupHandleSize, pAllocator);

    if (Json.contains("MaxShaderRecordStride"))
        Deserialize(Json["MaxShaderRecordStride"], Type.MaxShaderRecordStride, pAllocator);

    if (Json.contains("ShaderGroupBaseAlignment"))
        Deserialize(Json["ShaderGroupBaseAlignment"], Type.ShaderGroupBaseAlignment, pAllocator);

    if (Json.contains("MaxRayGenThreads"))
        Deserialize(Json["MaxRayGenThreads"], Type.MaxRayGenThreads, pAllocator);

    if (Json.contains("MaxInstancesPerTLAS"))
        Deserialize(Json["MaxInstancesPerTLAS"], Type.MaxInstancesPerTLAS, pAllocator);

    if (Json.contains("MaxPrimitivesPerBLAS"))
        Deserialize(Json["MaxPrimitivesPerBLAS"], Type.MaxPrimitivesPerBLAS, pAllocator);

    if (Json.contains("MaxGeometriesPerBLAS"))
        Deserialize(Json["MaxGeometriesPerBLAS"], Type.MaxGeometriesPerBLAS, pAllocator);

    if (Json.contains("VertexBufferAlignment"))
        Deserialize(Json["VertexBufferAlignment"], Type.VertexBufferAlignment, pAllocator);

    if (Json.contains("IndexBufferAlignment"))
        Deserialize(Json["IndexBufferAlignment"], Type.IndexBufferAlignment, pAllocator);

    if (Json.contains("TransformBufferAlignment"))
        Deserialize(Json["TransformBufferAlignment"], Type.TransformBufferAlignment, pAllocator);

    if (Json.contains("BoxBufferAlignment"))
        Deserialize(Json["BoxBufferAlignment"], Type.BoxBufferAlignment, pAllocator);

    if (Json.contains("ScratchBufferAlignment"))
        Deserialize(Json["ScratchBufferAlignment"], Type.ScratchBufferAlignment, pAllocator);

    if (Json.contains("InstanceBufferAlignment"))
        Deserialize(Json["InstanceBufferAlignment"], Type.InstanceBufferAlignment, pAllocator);

    if (Json.contains("CapFlags"))
        Deserialize(Json["CapFlags"], Type.CapFlags, pAllocator);
}

inline void Serialize(nlohmann::json& Json, const MeshShaderProperties& Type, DeviceObjectReflection* pAllocator)
{
    if (!(Type.MaxTaskCount == MeshShaderProperties{}.MaxTaskCount))
        Serialize(Json["MaxTaskCount"], Type.MaxTaskCount, pAllocator);
}

inline void Deserialize(const nlohmann::json& Json, MeshShaderProperties& Type, DeviceObjectReflection* pAllocator)
{
    if (Json.contains("MaxTaskCount"))
        Deserialize(Json["MaxTaskCount"], Type.MaxTaskCount, pAllocator);
}

inline void Serialize(nlohmann::json& Json, const ComputeShaderProperties& Type, DeviceObjectReflection* pAllocator)
{
    if (!(Type.SharedMemorySize == ComputeShaderProperties{}.SharedMemorySize))
        Serialize(Json["SharedMemorySize"], Type.SharedMemorySize, pAllocator);

    if (!(Type.MaxThreadGroupInvocations == ComputeShaderProperties{}.MaxThreadGroupInvocations))
        Serialize(Json["MaxThreadGroupInvocations"], Type.MaxThreadGroupInvocations, pAllocator);

    if (!(Type.MaxThreadGroupSizeX == ComputeShaderProperties{}.MaxThreadGroupSizeX))
        Serialize(Json["MaxThreadGroupSizeX"], Type.MaxThreadGroupSizeX, pAllocator);

    if (!(Type.MaxThreadGroupSizeY == ComputeShaderProperties{}.MaxThreadGroupSizeY))
        Serialize(Json["MaxThreadGroupSizeY"], Type.MaxThreadGroupSizeY, pAllocator);

    if (!(Type.MaxThreadGroupSizeZ == ComputeShaderProperties{}.MaxThreadGroupSizeZ))
        Serialize(Json["MaxThreadGroupSizeZ"], Type.MaxThreadGroupSizeZ, pAllocator);

    if (!(Type.MaxThreadGroupCountX == ComputeShaderProperties{}.MaxThreadGroupCountX))
        Serialize(Json["MaxThreadGroupCountX"], Type.MaxThreadGroupCountX, pAllocator);

    if (!(Type.MaxThreadGroupCountY == ComputeShaderProperties{}.MaxThreadGroupCountY))
        Serialize(Json["MaxThreadGroupCountY"], Type.MaxThreadGroupCountY, pAllocator);

    if (!(Type.MaxThreadGroupCountZ == ComputeShaderProperties{}.MaxThreadGroupCountZ))
        Serialize(Json["MaxThreadGroupCountZ"], Type.MaxThreadGroupCountZ, pAllocator);
}

inline void Deserialize(const nlohmann::json& Json, ComputeShaderProperties& Type, DeviceObjectReflection* pAllocator)
{
    if (Json.contains("SharedMemorySize"))
        Deserialize(Json["SharedMemorySize"], Type.SharedMemorySize, pAllocator);

    if (Json.contains("MaxThreadGroupInvocations"))
        Deserialize(Json["MaxThreadGroupInvocations"], Type.MaxThreadGroupInvocations, pAllocator);

    if (Json.contains("MaxThreadGroupSizeX"))
        Deserialize(Json["MaxThreadGroupSizeX"], Type.MaxThreadGroupSizeX, pAllocator);

    if (Json.contains("MaxThreadGroupSizeY"))
        Deserialize(Json["MaxThreadGroupSizeY"], Type.MaxThreadGroupSizeY, pAllocator);

    if (Json.contains("MaxThreadGroupSizeZ"))
        Deserialize(Json["MaxThreadGroupSizeZ"], Type.MaxThreadGroupSizeZ, pAllocator);

    if (Json.contains("MaxThreadGroupCountX"))
        Deserialize(Json["MaxThreadGroupCountX"], Type.MaxThreadGroupCountX, pAllocator);

    if (Json.contains("MaxThreadGroupCountY"))
        Deserialize(Json["MaxThreadGroupCountY"], Type.MaxThreadGroupCountY, pAllocator);

    if (Json.contains("MaxThreadGroupCountZ"))
        Deserialize(Json["MaxThreadGroupCountZ"], Type.MaxThreadGroupCountZ, pAllocator);
}

inline void Serialize(nlohmann::json& Json, const NDCAttribs& Type, DeviceObjectReflection* pAllocator)
{
    if (!(Type.MinZ == NDCAttribs{}.MinZ))
        Serialize(Json["MinZ"], Type.MinZ, pAllocator);

    if (!(Type.ZtoDepthScale == NDCAttribs{}.ZtoDepthScale))
        Serialize(Json["ZtoDepthScale"], Type.ZtoDepthScale, pAllocator);

    if (!(Type.YtoVScale == NDCAttribs{}.YtoVScale))
        Serialize(Json["YtoVScale"], Type.YtoVScale, pAllocator);
}

inline void Deserialize(const nlohmann::json& Json, NDCAttribs& Type, DeviceObjectReflection* pAllocator)
{
    if (Json.contains("MinZ"))
        Deserialize(Json["MinZ"], Type.MinZ, pAllocator);

    if (Json.contains("ZtoDepthScale"))
        Deserialize(Json["ZtoDepthScale"], Type.ZtoDepthScale, pAllocator);

    if (Json.contains("YtoVScale"))
        Deserialize(Json["YtoVScale"], Type.YtoVScale, pAllocator);
}

inline void Serialize(nlohmann::json& Json, const RenderDeviceInfo& Type, DeviceObjectReflection* pAllocator)
{
    if (!(Type.Type == RenderDeviceInfo{}.Type))
        Serialize(Json["Type"], Type.Type, pAllocator);

    if (!(Type.APIVersion == RenderDeviceInfo{}.APIVersion))
        Serialize(Json["APIVersion"], Type.APIVersion, pAllocator);

    if (!(Type.Features == RenderDeviceInfo{}.Features))
        Serialize(Json["Features"], Type.Features, pAllocator);

    if (!(Type.NDC == RenderDeviceInfo{}.NDC))
        Serialize(Json["NDC"], Type.NDC, pAllocator);
}

inline void Deserialize(const nlohmann::json& Json, RenderDeviceInfo& Type, DeviceObjectReflection* pAllocator)
{
    if (Json.contains("Type"))
        Deserialize(Json["Type"], Type.Type, pAllocator);

    if (Json.contains("APIVersion"))
        Deserialize(Json["APIVersion"], Type.APIVersion, pAllocator);

    if (Json.contains("Features"))
        Deserialize(Json["Features"], Type.Features, pAllocator);

    if (Json.contains("NDC"))
        Deserialize(Json["NDC"], Type.NDC, pAllocator);
}

inline void Serialize(nlohmann::json& Json, const AdapterMemoryInfo& Type, DeviceObjectReflection* pAllocator)
{
    if (!(Type.LocalMemory == AdapterMemoryInfo{}.LocalMemory))
        Serialize(Json["LocalMemory"], Type.LocalMemory, pAllocator);

    if (!(Type.HostVisibleMemory == AdapterMemoryInfo{}.HostVisibleMemory))
        Serialize(Json["HostVisibleMemory"], Type.HostVisibleMemory, pAllocator);

    if (!(Type.UnifiedMemory == AdapterMemoryInfo{}.UnifiedMemory))
        Serialize(Json["UnifiedMemory"], Type.UnifiedMemory, pAllocator);

    if (!(Type.MaxMemoryAllocation == AdapterMemoryInfo{}.MaxMemoryAllocation))
        Serialize(Json["MaxMemoryAllocation"], Type.MaxMemoryAllocation, pAllocator);

    if (!(Type.UnifiedMemoryCPUAccess == AdapterMemoryInfo{}.UnifiedMemoryCPUAccess))
        Serialize(Json["UnifiedMemoryCPUAccess"], Type.UnifiedMemoryCPUAccess, pAllocator);

    if (!(Type.MemorylessTextureBindFlags == AdapterMemoryInfo{}.MemorylessTextureBindFlags))
        Serialize(Json["MemorylessTextureBindFlags"], Type.MemorylessTextureBindFlags, pAllocator);
}

inline void Deserialize(const nlohmann::json& Json, AdapterMemoryInfo& Type, DeviceObjectReflection* pAllocator)
{
    if (Json.contains("LocalMemory"))
        Deserialize(Json["LocalMemory"], Type.LocalMemory, pAllocator);

    if (Json.contains("HostVisibleMemory"))
        Deserialize(Json["HostVisibleMemory"], Type.HostVisibleMemory, pAllocator);

    if (Json.contains("UnifiedMemory"))
        Deserialize(Json["UnifiedMemory"], Type.UnifiedMemory, pAllocator);

    if (Json.contains("MaxMemoryAllocation"))
        Deserialize(Json["MaxMemoryAllocation"], Type.MaxMemoryAllocation, pAllocator);

    if (Json.contains("UnifiedMemoryCPUAccess"))
        Deserialize(Json["UnifiedMemoryCPUAccess"], Type.UnifiedMemoryCPUAccess, pAllocator);

    if (Json.contains("MemorylessTextureBindFlags"))
        Deserialize(Json["MemorylessTextureBindFlags"], Type.MemorylessTextureBindFlags, pAllocator);
}

inline void Serialize(nlohmann::json& Json, const ShadingRateMode& Type, DeviceObjectReflection* pAllocator)
{
    if (!(Type.Rate == ShadingRateMode{}.Rate))
        Serialize(Json["Rate"], Type.Rate, pAllocator);

    if (!(Type.SampleBits == ShadingRateMode{}.SampleBits))
        SerializeBitwiseEnum(Json["SampleBits"], Type.SampleBits, pAllocator);
}

inline void Deserialize(const nlohmann::json& Json, ShadingRateMode& Type, DeviceObjectReflection* pAllocator)
{
    if (Json.contains("Rate"))
        Deserialize(Json["Rate"], Type.Rate, pAllocator);

    if (Json.contains("SampleBits"))
        DeserializeBitwiseEnum(Json["SampleBits"], Type.SampleBits, pAllocator);
}

inline void Serialize(nlohmann::json& Json, const ShadingRateProperties& Type, DeviceObjectReflection* pAllocator)
{
    if (!CompareConstArray(Type.ShadingRates, ShadingRateProperties{}.ShadingRates))
        SerializeConstArray(Json["ShadingRates"], Type.ShadingRates, pAllocator);

    if (!(Type.NumShadingRates == ShadingRateProperties{}.NumShadingRates))
        Serialize(Json["NumShadingRates"], Type.NumShadingRates, pAllocator);

    if (!(Type.CapFlags == ShadingRateProperties{}.CapFlags))
        Serialize(Json["CapFlags"], Type.CapFlags, pAllocator);

    if (!(Type.Combiners == ShadingRateProperties{}.Combiners))
        Serialize(Json["Combiners"], Type.Combiners, pAllocator);

    if (!(Type.Format == ShadingRateProperties{}.Format))
        Serialize(Json["Format"], Type.Format, pAllocator);

    if (!(Type.ShadingRateTextureAccess == ShadingRateProperties{}.ShadingRateTextureAccess))
        Serialize(Json["ShadingRateTextureAccess"], Type.ShadingRateTextureAccess, pAllocator);

    if (!(Type.BindFlags == ShadingRateProperties{}.BindFlags))
        Serialize(Json["BindFlags"], Type.BindFlags, pAllocator);

    if (!CompareConstArray(Type.MinTileSize, ShadingRateProperties{}.MinTileSize))
        SerializeConstArray(Json["MinTileSize"], Type.MinTileSize, pAllocator);

    if (!CompareConstArray(Type.MaxTileSize, ShadingRateProperties{}.MaxTileSize))
        SerializeConstArray(Json["MaxTileSize"], Type.MaxTileSize, pAllocator);

    if (!(Type.MaxSabsampledArraySlices == ShadingRateProperties{}.MaxSabsampledArraySlices))
        Serialize(Json["MaxSabsampledArraySlices"], Type.MaxSabsampledArraySlices, pAllocator);
}

inline void Deserialize(const nlohmann::json& Json, ShadingRateProperties& Type, DeviceObjectReflection* pAllocator)
{
    if (Json.contains("ShadingRates"))
        DeserializeConstArray(Json["ShadingRates"], Type.ShadingRates, pAllocator);

    if (Json.contains("NumShadingRates"))
        Deserialize(Json["NumShadingRates"], Type.NumShadingRates, pAllocator);

    if (Json.contains("CapFlags"))
        Deserialize(Json["CapFlags"], Type.CapFlags, pAllocator);

    if (Json.contains("Combiners"))
        Deserialize(Json["Combiners"], Type.Combiners, pAllocator);

    if (Json.contains("Format"))
        Deserialize(Json["Format"], Type.Format, pAllocator);

    if (Json.contains("ShadingRateTextureAccess"))
        Deserialize(Json["ShadingRateTextureAccess"], Type.ShadingRateTextureAccess, pAllocator);

    if (Json.contains("BindFlags"))
        Deserialize(Json["BindFlags"], Type.BindFlags, pAllocator);

    if (Json.contains("MinTileSize"))
        DeserializeConstArray(Json["MinTileSize"], Type.MinTileSize, pAllocator);

    if (Json.contains("MaxTileSize"))
        DeserializeConstArray(Json["MaxTileSize"], Type.MaxTileSize, pAllocator);

    if (Json.contains("MaxSabsampledArraySlices"))
        Deserialize(Json["MaxSabsampledArraySlices"], Type.MaxSabsampledArraySlices, pAllocator);
}

inline void Serialize(nlohmann::json& Json, const DrawCommandProperties& Type, DeviceObjectReflection* pAllocator)
{
    if (!(Type.CapFlags == DrawCommandProperties{}.CapFlags))
        Serialize(Json["CapFlags"], Type.CapFlags, pAllocator);

    if (!(Type.MaxIndexValue == DrawCommandProperties{}.MaxIndexValue))
        Serialize(Json["MaxIndexValue"], Type.MaxIndexValue, pAllocator);

    if (!(Type.MaxDrawIndirectCount == DrawCommandProperties{}.MaxDrawIndirectCount))
        Serialize(Json["MaxDrawIndirectCount"], Type.MaxDrawIndirectCount, pAllocator);
}

inline void Deserialize(const nlohmann::json& Json, DrawCommandProperties& Type, DeviceObjectReflection* pAllocator)
{
    if (Json.contains("CapFlags"))
        Deserialize(Json["CapFlags"], Type.CapFlags, pAllocator);

    if (Json.contains("MaxIndexValue"))
        Deserialize(Json["MaxIndexValue"], Type.MaxIndexValue, pAllocator);

    if (Json.contains("MaxDrawIndirectCount"))
        Deserialize(Json["MaxDrawIndirectCount"], Type.MaxDrawIndirectCount, pAllocator);
}

inline void Serialize(nlohmann::json& Json, const SparseResourceProperties& Type, DeviceObjectReflection* pAllocator)
{
    if (!(Type.AddressSpaceSize == SparseResourceProperties{}.AddressSpaceSize))
        Serialize(Json["AddressSpaceSize"], Type.AddressSpaceSize, pAllocator);

    if (!(Type.ResourceSpaceSize == SparseResourceProperties{}.ResourceSpaceSize))
        Serialize(Json["ResourceSpaceSize"], Type.ResourceSpaceSize, pAllocator);

    if (!(Type.CapFlags == SparseResourceProperties{}.CapFlags))
        Serialize(Json["CapFlags"], Type.CapFlags, pAllocator);

    if (!(Type.StandardBlockSize == SparseResourceProperties{}.StandardBlockSize))
        Serialize(Json["StandardBlockSize"], Type.StandardBlockSize, pAllocator);

    if (!(Type.BufferBindFlags == SparseResourceProperties{}.BufferBindFlags))
        Serialize(Json["BufferBindFlags"], Type.BufferBindFlags, pAllocator);

    if (!(Type._Padding == SparseResourceProperties{}._Padding))
        Serialize(Json["_Padding"], Type._Padding, pAllocator);
}

inline void Deserialize(const nlohmann::json& Json, SparseResourceProperties& Type, DeviceObjectReflection* pAllocator)
{
    if (Json.contains("AddressSpaceSize"))
        Deserialize(Json["AddressSpaceSize"], Type.AddressSpaceSize, pAllocator);

    if (Json.contains("ResourceSpaceSize"))
        Deserialize(Json["ResourceSpaceSize"], Type.ResourceSpaceSize, pAllocator);

    if (Json.contains("CapFlags"))
        Deserialize(Json["CapFlags"], Type.CapFlags, pAllocator);

    if (Json.contains("StandardBlockSize"))
        Deserialize(Json["StandardBlockSize"], Type.StandardBlockSize, pAllocator);

    if (Json.contains("BufferBindFlags"))
        Deserialize(Json["BufferBindFlags"], Type.BufferBindFlags, pAllocator);

    if (Json.contains("_Padding"))
        Deserialize(Json["_Padding"], Type._Padding, pAllocator);
}

inline void Serialize(nlohmann::json& Json, const CommandQueueInfo& Type, DeviceObjectReflection* pAllocator)
{
    if (!(Type.QueueType == CommandQueueInfo{}.QueueType))
        Serialize(Json["QueueType"], Type.QueueType, pAllocator);

    if (!(Type.MaxDeviceContexts == CommandQueueInfo{}.MaxDeviceContexts))
        Serialize(Json["MaxDeviceContexts"], Type.MaxDeviceContexts, pAllocator);

    if (!CompareConstArray(Type.TextureCopyGranularity, CommandQueueInfo{}.TextureCopyGranularity))
        SerializeConstArray(Json["TextureCopyGranularity"], Type.TextureCopyGranularity, pAllocator);
}

inline void Deserialize(const nlohmann::json& Json, CommandQueueInfo& Type, DeviceObjectReflection* pAllocator)
{
    if (Json.contains("QueueType"))
        Deserialize(Json["QueueType"], Type.QueueType, pAllocator);

    if (Json.contains("MaxDeviceContexts"))
        Deserialize(Json["MaxDeviceContexts"], Type.MaxDeviceContexts, pAllocator);

    if (Json.contains("TextureCopyGranularity"))
        DeserializeConstArray(Json["TextureCopyGranularity"], Type.TextureCopyGranularity, pAllocator);
}

inline void Serialize(nlohmann::json& Json, const GraphicsAdapterInfo& Type, DeviceObjectReflection* pAllocator)
{
    if (!CompareConstArray(Type.Description, GraphicsAdapterInfo{}.Description))
        SerializeConstArray(Json["Description"], Type.Description, pAllocator);

    if (!(Type.Type == GraphicsAdapterInfo{}.Type))
        Serialize(Json["Type"], Type.Type, pAllocator);

    if (!(Type.Vendor == GraphicsAdapterInfo{}.Vendor))
        Serialize(Json["Vendor"], Type.Vendor, pAllocator);

    if (!(Type.VendorId == GraphicsAdapterInfo{}.VendorId))
        Serialize(Json["VendorId"], Type.VendorId, pAllocator);

    if (!(Type.DeviceId == GraphicsAdapterInfo{}.DeviceId))
        Serialize(Json["DeviceId"], Type.DeviceId, pAllocator);

    if (!(Type.NumOutputs == GraphicsAdapterInfo{}.NumOutputs))
        Serialize(Json["NumOutputs"], Type.NumOutputs, pAllocator);

    if (!(Type.Memory == GraphicsAdapterInfo{}.Memory))
        Serialize(Json["Memory"], Type.Memory, pAllocator);

    if (!(Type.RayTracing == GraphicsAdapterInfo{}.RayTracing))
        Serialize(Json["RayTracing"], Type.RayTracing, pAllocator);

    if (!(Type.WaveOp == GraphicsAdapterInfo{}.WaveOp))
        Serialize(Json["WaveOp"], Type.WaveOp, pAllocator);

    if (!(Type.Buffer == GraphicsAdapterInfo{}.Buffer))
        Serialize(Json["Buffer"], Type.Buffer, pAllocator);

    if (!(Type.Texture == GraphicsAdapterInfo{}.Texture))
        Serialize(Json["Texture"], Type.Texture, pAllocator);

    if (!(Type.Sampler == GraphicsAdapterInfo{}.Sampler))
        Serialize(Json["Sampler"], Type.Sampler, pAllocator);

    if (!(Type.MeshShader == GraphicsAdapterInfo{}.MeshShader))
        Serialize(Json["MeshShader"], Type.MeshShader, pAllocator);

    if (!(Type.ShadingRate == GraphicsAdapterInfo{}.ShadingRate))
        Serialize(Json["ShadingRate"], Type.ShadingRate, pAllocator);

    if (!(Type.ComputeShader == GraphicsAdapterInfo{}.ComputeShader))
        Serialize(Json["ComputeShader"], Type.ComputeShader, pAllocator);

    if (!(Type.DrawCommand == GraphicsAdapterInfo{}.DrawCommand))
        Serialize(Json["DrawCommand"], Type.DrawCommand, pAllocator);

    if (!(Type.SparseResources == GraphicsAdapterInfo{}.SparseResources))
        Serialize(Json["SparseResources"], Type.SparseResources, pAllocator);

    if (!(Type.Features == GraphicsAdapterInfo{}.Features))
        Serialize(Json["Features"], Type.Features, pAllocator);

    if (!CompareConstArray(Type.Queues, GraphicsAdapterInfo{}.Queues))
        SerializeConstArray(Json["Queues"], Type.Queues, pAllocator);

    if (!(Type.NumQueues == GraphicsAdapterInfo{}.NumQueues))
        Serialize(Json["NumQueues"], Type.NumQueues, pAllocator);
}

inline void Deserialize(const nlohmann::json& Json, GraphicsAdapterInfo& Type, DeviceObjectReflection* pAllocator)
{
    if (Json.contains("Description"))
        DeserializeConstArray(Json["Description"], Type.Description, pAllocator);

    if (Json.contains("Type"))
        Deserialize(Json["Type"], Type.Type, pAllocator);

    if (Json.contains("Vendor"))
        Deserialize(Json["Vendor"], Type.Vendor, pAllocator);

    if (Json.contains("VendorId"))
        Deserialize(Json["VendorId"], Type.VendorId, pAllocator);

    if (Json.contains("DeviceId"))
        Deserialize(Json["DeviceId"], Type.DeviceId, pAllocator);

    if (Json.contains("NumOutputs"))
        Deserialize(Json["NumOutputs"], Type.NumOutputs, pAllocator);

    if (Json.contains("Memory"))
        Deserialize(Json["Memory"], Type.Memory, pAllocator);

    if (Json.contains("RayTracing"))
        Deserialize(Json["RayTracing"], Type.RayTracing, pAllocator);

    if (Json.contains("WaveOp"))
        Deserialize(Json["WaveOp"], Type.WaveOp, pAllocator);

    if (Json.contains("Buffer"))
        Deserialize(Json["Buffer"], Type.Buffer, pAllocator);

    if (Json.contains("Texture"))
        Deserialize(Json["Texture"], Type.Texture, pAllocator);

    if (Json.contains("Sampler"))
        Deserialize(Json["Sampler"], Type.Sampler, pAllocator);

    if (Json.contains("MeshShader"))
        Deserialize(Json["MeshShader"], Type.MeshShader, pAllocator);

    if (Json.contains("ShadingRate"))
        Deserialize(Json["ShadingRate"], Type.ShadingRate, pAllocator);

    if (Json.contains("ComputeShader"))
        Deserialize(Json["ComputeShader"], Type.ComputeShader, pAllocator);

    if (Json.contains("DrawCommand"))
        Deserialize(Json["DrawCommand"], Type.DrawCommand, pAllocator);

    if (Json.contains("SparseResources"))
        Deserialize(Json["SparseResources"], Type.SparseResources, pAllocator);

    if (Json.contains("Features"))
        Deserialize(Json["Features"], Type.Features, pAllocator);

    if (Json.contains("Queues"))
        DeserializeConstArray(Json["Queues"], Type.Queues, pAllocator);

    if (Json.contains("NumQueues"))
        Deserialize(Json["NumQueues"], Type.NumQueues, pAllocator);
}

} // namespace Diligent
