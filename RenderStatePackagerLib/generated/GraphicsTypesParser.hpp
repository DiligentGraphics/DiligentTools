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

inline void Serialize(nlohmann::json& Json, const DeviceObjectAttribs& Type, DynamicLinearAllocator& Allocator)
{
    if (!CompareStr(Type.Name, DeviceObjectAttribs{}.Name))
        Serialize(Json["Name"], Type.Name, Allocator);
}

inline void Deserialize(const nlohmann::json& Json, DeviceObjectAttribs& Type, DynamicLinearAllocator& Allocator)
{
    if (Json.contains("Name"))
        Deserialize(Json["Name"], Type.Name, Allocator);
}

inline void Serialize(nlohmann::json& Json, const DeviceFeatures& Type, DynamicLinearAllocator& Allocator)
{
    if (!(Type.SeparablePrograms == DeviceFeatures{}.SeparablePrograms))
        Serialize(Json["SeparablePrograms"], Type.SeparablePrograms, Allocator);

    if (!(Type.ShaderResourceQueries == DeviceFeatures{}.ShaderResourceQueries))
        Serialize(Json["ShaderResourceQueries"], Type.ShaderResourceQueries, Allocator);

    if (!(Type.WireframeFill == DeviceFeatures{}.WireframeFill))
        Serialize(Json["WireframeFill"], Type.WireframeFill, Allocator);

    if (!(Type.MultithreadedResourceCreation == DeviceFeatures{}.MultithreadedResourceCreation))
        Serialize(Json["MultithreadedResourceCreation"], Type.MultithreadedResourceCreation, Allocator);

    if (!(Type.ComputeShaders == DeviceFeatures{}.ComputeShaders))
        Serialize(Json["ComputeShaders"], Type.ComputeShaders, Allocator);

    if (!(Type.GeometryShaders == DeviceFeatures{}.GeometryShaders))
        Serialize(Json["GeometryShaders"], Type.GeometryShaders, Allocator);

    if (!(Type.Tessellation == DeviceFeatures{}.Tessellation))
        Serialize(Json["Tessellation"], Type.Tessellation, Allocator);

    if (!(Type.MeshShaders == DeviceFeatures{}.MeshShaders))
        Serialize(Json["MeshShaders"], Type.MeshShaders, Allocator);

    if (!(Type.RayTracing == DeviceFeatures{}.RayTracing))
        Serialize(Json["RayTracing"], Type.RayTracing, Allocator);

    if (!(Type.BindlessResources == DeviceFeatures{}.BindlessResources))
        Serialize(Json["BindlessResources"], Type.BindlessResources, Allocator);

    if (!(Type.OcclusionQueries == DeviceFeatures{}.OcclusionQueries))
        Serialize(Json["OcclusionQueries"], Type.OcclusionQueries, Allocator);

    if (!(Type.BinaryOcclusionQueries == DeviceFeatures{}.BinaryOcclusionQueries))
        Serialize(Json["BinaryOcclusionQueries"], Type.BinaryOcclusionQueries, Allocator);

    if (!(Type.TimestampQueries == DeviceFeatures{}.TimestampQueries))
        Serialize(Json["TimestampQueries"], Type.TimestampQueries, Allocator);

    if (!(Type.PipelineStatisticsQueries == DeviceFeatures{}.PipelineStatisticsQueries))
        Serialize(Json["PipelineStatisticsQueries"], Type.PipelineStatisticsQueries, Allocator);

    if (!(Type.DurationQueries == DeviceFeatures{}.DurationQueries))
        Serialize(Json["DurationQueries"], Type.DurationQueries, Allocator);

    if (!(Type.DepthBiasClamp == DeviceFeatures{}.DepthBiasClamp))
        Serialize(Json["DepthBiasClamp"], Type.DepthBiasClamp, Allocator);

    if (!(Type.DepthClamp == DeviceFeatures{}.DepthClamp))
        Serialize(Json["DepthClamp"], Type.DepthClamp, Allocator);

    if (!(Type.IndependentBlend == DeviceFeatures{}.IndependentBlend))
        Serialize(Json["IndependentBlend"], Type.IndependentBlend, Allocator);

    if (!(Type.DualSourceBlend == DeviceFeatures{}.DualSourceBlend))
        Serialize(Json["DualSourceBlend"], Type.DualSourceBlend, Allocator);

    if (!(Type.MultiViewport == DeviceFeatures{}.MultiViewport))
        Serialize(Json["MultiViewport"], Type.MultiViewport, Allocator);

    if (!(Type.TextureCompressionBC == DeviceFeatures{}.TextureCompressionBC))
        Serialize(Json["TextureCompressionBC"], Type.TextureCompressionBC, Allocator);

    if (!(Type.VertexPipelineUAVWritesAndAtomics == DeviceFeatures{}.VertexPipelineUAVWritesAndAtomics))
        Serialize(Json["VertexPipelineUAVWritesAndAtomics"], Type.VertexPipelineUAVWritesAndAtomics, Allocator);

    if (!(Type.PixelUAVWritesAndAtomics == DeviceFeatures{}.PixelUAVWritesAndAtomics))
        Serialize(Json["PixelUAVWritesAndAtomics"], Type.PixelUAVWritesAndAtomics, Allocator);

    if (!(Type.TextureUAVExtendedFormats == DeviceFeatures{}.TextureUAVExtendedFormats))
        Serialize(Json["TextureUAVExtendedFormats"], Type.TextureUAVExtendedFormats, Allocator);

    if (!(Type.ShaderFloat16 == DeviceFeatures{}.ShaderFloat16))
        Serialize(Json["ShaderFloat16"], Type.ShaderFloat16, Allocator);

    if (!(Type.ResourceBuffer16BitAccess == DeviceFeatures{}.ResourceBuffer16BitAccess))
        Serialize(Json["ResourceBuffer16BitAccess"], Type.ResourceBuffer16BitAccess, Allocator);

    if (!(Type.UniformBuffer16BitAccess == DeviceFeatures{}.UniformBuffer16BitAccess))
        Serialize(Json["UniformBuffer16BitAccess"], Type.UniformBuffer16BitAccess, Allocator);

    if (!(Type.ShaderInputOutput16 == DeviceFeatures{}.ShaderInputOutput16))
        Serialize(Json["ShaderInputOutput16"], Type.ShaderInputOutput16, Allocator);

    if (!(Type.ShaderInt8 == DeviceFeatures{}.ShaderInt8))
        Serialize(Json["ShaderInt8"], Type.ShaderInt8, Allocator);

    if (!(Type.ResourceBuffer8BitAccess == DeviceFeatures{}.ResourceBuffer8BitAccess))
        Serialize(Json["ResourceBuffer8BitAccess"], Type.ResourceBuffer8BitAccess, Allocator);

    if (!(Type.UniformBuffer8BitAccess == DeviceFeatures{}.UniformBuffer8BitAccess))
        Serialize(Json["UniformBuffer8BitAccess"], Type.UniformBuffer8BitAccess, Allocator);

    if (!(Type.ShaderResourceRuntimeArray == DeviceFeatures{}.ShaderResourceRuntimeArray))
        Serialize(Json["ShaderResourceRuntimeArray"], Type.ShaderResourceRuntimeArray, Allocator);

    if (!(Type.WaveOp == DeviceFeatures{}.WaveOp))
        Serialize(Json["WaveOp"], Type.WaveOp, Allocator);

    if (!(Type.InstanceDataStepRate == DeviceFeatures{}.InstanceDataStepRate))
        Serialize(Json["InstanceDataStepRate"], Type.InstanceDataStepRate, Allocator);

    if (!(Type.NativeFence == DeviceFeatures{}.NativeFence))
        Serialize(Json["NativeFence"], Type.NativeFence, Allocator);

    if (!(Type.TileShaders == DeviceFeatures{}.TileShaders))
        Serialize(Json["TileShaders"], Type.TileShaders, Allocator);

    if (!(Type.TransferQueueTimestampQueries == DeviceFeatures{}.TransferQueueTimestampQueries))
        Serialize(Json["TransferQueueTimestampQueries"], Type.TransferQueueTimestampQueries, Allocator);

    if (!(Type.VariableRateShading == DeviceFeatures{}.VariableRateShading))
        Serialize(Json["VariableRateShading"], Type.VariableRateShading, Allocator);

    if (!(Type.SparseResources == DeviceFeatures{}.SparseResources))
        Serialize(Json["SparseResources"], Type.SparseResources, Allocator);
}

inline void Deserialize(const nlohmann::json& Json, DeviceFeatures& Type, DynamicLinearAllocator& Allocator)
{
    if (Json.contains("SeparablePrograms"))
        Deserialize(Json["SeparablePrograms"], Type.SeparablePrograms, Allocator);

    if (Json.contains("ShaderResourceQueries"))
        Deserialize(Json["ShaderResourceQueries"], Type.ShaderResourceQueries, Allocator);

    if (Json.contains("WireframeFill"))
        Deserialize(Json["WireframeFill"], Type.WireframeFill, Allocator);

    if (Json.contains("MultithreadedResourceCreation"))
        Deserialize(Json["MultithreadedResourceCreation"], Type.MultithreadedResourceCreation, Allocator);

    if (Json.contains("ComputeShaders"))
        Deserialize(Json["ComputeShaders"], Type.ComputeShaders, Allocator);

    if (Json.contains("GeometryShaders"))
        Deserialize(Json["GeometryShaders"], Type.GeometryShaders, Allocator);

    if (Json.contains("Tessellation"))
        Deserialize(Json["Tessellation"], Type.Tessellation, Allocator);

    if (Json.contains("MeshShaders"))
        Deserialize(Json["MeshShaders"], Type.MeshShaders, Allocator);

    if (Json.contains("RayTracing"))
        Deserialize(Json["RayTracing"], Type.RayTracing, Allocator);

    if (Json.contains("BindlessResources"))
        Deserialize(Json["BindlessResources"], Type.BindlessResources, Allocator);

    if (Json.contains("OcclusionQueries"))
        Deserialize(Json["OcclusionQueries"], Type.OcclusionQueries, Allocator);

    if (Json.contains("BinaryOcclusionQueries"))
        Deserialize(Json["BinaryOcclusionQueries"], Type.BinaryOcclusionQueries, Allocator);

    if (Json.contains("TimestampQueries"))
        Deserialize(Json["TimestampQueries"], Type.TimestampQueries, Allocator);

    if (Json.contains("PipelineStatisticsQueries"))
        Deserialize(Json["PipelineStatisticsQueries"], Type.PipelineStatisticsQueries, Allocator);

    if (Json.contains("DurationQueries"))
        Deserialize(Json["DurationQueries"], Type.DurationQueries, Allocator);

    if (Json.contains("DepthBiasClamp"))
        Deserialize(Json["DepthBiasClamp"], Type.DepthBiasClamp, Allocator);

    if (Json.contains("DepthClamp"))
        Deserialize(Json["DepthClamp"], Type.DepthClamp, Allocator);

    if (Json.contains("IndependentBlend"))
        Deserialize(Json["IndependentBlend"], Type.IndependentBlend, Allocator);

    if (Json.contains("DualSourceBlend"))
        Deserialize(Json["DualSourceBlend"], Type.DualSourceBlend, Allocator);

    if (Json.contains("MultiViewport"))
        Deserialize(Json["MultiViewport"], Type.MultiViewport, Allocator);

    if (Json.contains("TextureCompressionBC"))
        Deserialize(Json["TextureCompressionBC"], Type.TextureCompressionBC, Allocator);

    if (Json.contains("VertexPipelineUAVWritesAndAtomics"))
        Deserialize(Json["VertexPipelineUAVWritesAndAtomics"], Type.VertexPipelineUAVWritesAndAtomics, Allocator);

    if (Json.contains("PixelUAVWritesAndAtomics"))
        Deserialize(Json["PixelUAVWritesAndAtomics"], Type.PixelUAVWritesAndAtomics, Allocator);

    if (Json.contains("TextureUAVExtendedFormats"))
        Deserialize(Json["TextureUAVExtendedFormats"], Type.TextureUAVExtendedFormats, Allocator);

    if (Json.contains("ShaderFloat16"))
        Deserialize(Json["ShaderFloat16"], Type.ShaderFloat16, Allocator);

    if (Json.contains("ResourceBuffer16BitAccess"))
        Deserialize(Json["ResourceBuffer16BitAccess"], Type.ResourceBuffer16BitAccess, Allocator);

    if (Json.contains("UniformBuffer16BitAccess"))
        Deserialize(Json["UniformBuffer16BitAccess"], Type.UniformBuffer16BitAccess, Allocator);

    if (Json.contains("ShaderInputOutput16"))
        Deserialize(Json["ShaderInputOutput16"], Type.ShaderInputOutput16, Allocator);

    if (Json.contains("ShaderInt8"))
        Deserialize(Json["ShaderInt8"], Type.ShaderInt8, Allocator);

    if (Json.contains("ResourceBuffer8BitAccess"))
        Deserialize(Json["ResourceBuffer8BitAccess"], Type.ResourceBuffer8BitAccess, Allocator);

    if (Json.contains("UniformBuffer8BitAccess"))
        Deserialize(Json["UniformBuffer8BitAccess"], Type.UniformBuffer8BitAccess, Allocator);

    if (Json.contains("ShaderResourceRuntimeArray"))
        Deserialize(Json["ShaderResourceRuntimeArray"], Type.ShaderResourceRuntimeArray, Allocator);

    if (Json.contains("WaveOp"))
        Deserialize(Json["WaveOp"], Type.WaveOp, Allocator);

    if (Json.contains("InstanceDataStepRate"))
        Deserialize(Json["InstanceDataStepRate"], Type.InstanceDataStepRate, Allocator);

    if (Json.contains("NativeFence"))
        Deserialize(Json["NativeFence"], Type.NativeFence, Allocator);

    if (Json.contains("TileShaders"))
        Deserialize(Json["TileShaders"], Type.TileShaders, Allocator);

    if (Json.contains("TransferQueueTimestampQueries"))
        Deserialize(Json["TransferQueueTimestampQueries"], Type.TransferQueueTimestampQueries, Allocator);

    if (Json.contains("VariableRateShading"))
        Deserialize(Json["VariableRateShading"], Type.VariableRateShading, Allocator);

    if (Json.contains("SparseResources"))
        Deserialize(Json["SparseResources"], Type.SparseResources, Allocator);
}

inline void Serialize(nlohmann::json& Json, const Version& Type, DynamicLinearAllocator& Allocator)
{
    if (!(Type.Major == Version{}.Major))
        Serialize(Json["Major"], Type.Major, Allocator);

    if (!(Type.Minor == Version{}.Minor))
        Serialize(Json["Minor"], Type.Minor, Allocator);
}

inline void Deserialize(const nlohmann::json& Json, Version& Type, DynamicLinearAllocator& Allocator)
{
    if (Json.contains("Major"))
        Deserialize(Json["Major"], Type.Major, Allocator);

    if (Json.contains("Minor"))
        Deserialize(Json["Minor"], Type.Minor, Allocator);
}

inline void Serialize(nlohmann::json& Json, const TextureProperties& Type, DynamicLinearAllocator& Allocator)
{
    if (!(Type.MaxTexture1DDimension == TextureProperties{}.MaxTexture1DDimension))
        Serialize(Json["MaxTexture1DDimension"], Type.MaxTexture1DDimension, Allocator);

    if (!(Type.MaxTexture1DArraySlices == TextureProperties{}.MaxTexture1DArraySlices))
        Serialize(Json["MaxTexture1DArraySlices"], Type.MaxTexture1DArraySlices, Allocator);

    if (!(Type.MaxTexture2DDimension == TextureProperties{}.MaxTexture2DDimension))
        Serialize(Json["MaxTexture2DDimension"], Type.MaxTexture2DDimension, Allocator);

    if (!(Type.MaxTexture2DArraySlices == TextureProperties{}.MaxTexture2DArraySlices))
        Serialize(Json["MaxTexture2DArraySlices"], Type.MaxTexture2DArraySlices, Allocator);

    if (!(Type.MaxTexture3DDimension == TextureProperties{}.MaxTexture3DDimension))
        Serialize(Json["MaxTexture3DDimension"], Type.MaxTexture3DDimension, Allocator);

    if (!(Type.MaxTextureCubeDimension == TextureProperties{}.MaxTextureCubeDimension))
        Serialize(Json["MaxTextureCubeDimension"], Type.MaxTextureCubeDimension, Allocator);

    if (!(Type.Texture2DMSSupported == TextureProperties{}.Texture2DMSSupported))
        Serialize(Json["Texture2DMSSupported"], Type.Texture2DMSSupported, Allocator);

    if (!(Type.Texture2DMSArraySupported == TextureProperties{}.Texture2DMSArraySupported))
        Serialize(Json["Texture2DMSArraySupported"], Type.Texture2DMSArraySupported, Allocator);

    if (!(Type.TextureViewSupported == TextureProperties{}.TextureViewSupported))
        Serialize(Json["TextureViewSupported"], Type.TextureViewSupported, Allocator);

    if (!(Type.CubemapArraysSupported == TextureProperties{}.CubemapArraysSupported))
        Serialize(Json["CubemapArraysSupported"], Type.CubemapArraysSupported, Allocator);

    if (!(Type.TextureView2DOn3DSupported == TextureProperties{}.TextureView2DOn3DSupported))
        Serialize(Json["TextureView2DOn3DSupported"], Type.TextureView2DOn3DSupported, Allocator);
}

inline void Deserialize(const nlohmann::json& Json, TextureProperties& Type, DynamicLinearAllocator& Allocator)
{
    if (Json.contains("MaxTexture1DDimension"))
        Deserialize(Json["MaxTexture1DDimension"], Type.MaxTexture1DDimension, Allocator);

    if (Json.contains("MaxTexture1DArraySlices"))
        Deserialize(Json["MaxTexture1DArraySlices"], Type.MaxTexture1DArraySlices, Allocator);

    if (Json.contains("MaxTexture2DDimension"))
        Deserialize(Json["MaxTexture2DDimension"], Type.MaxTexture2DDimension, Allocator);

    if (Json.contains("MaxTexture2DArraySlices"))
        Deserialize(Json["MaxTexture2DArraySlices"], Type.MaxTexture2DArraySlices, Allocator);

    if (Json.contains("MaxTexture3DDimension"))
        Deserialize(Json["MaxTexture3DDimension"], Type.MaxTexture3DDimension, Allocator);

    if (Json.contains("MaxTextureCubeDimension"))
        Deserialize(Json["MaxTextureCubeDimension"], Type.MaxTextureCubeDimension, Allocator);

    if (Json.contains("Texture2DMSSupported"))
        Deserialize(Json["Texture2DMSSupported"], Type.Texture2DMSSupported, Allocator);

    if (Json.contains("Texture2DMSArraySupported"))
        Deserialize(Json["Texture2DMSArraySupported"], Type.Texture2DMSArraySupported, Allocator);

    if (Json.contains("TextureViewSupported"))
        Deserialize(Json["TextureViewSupported"], Type.TextureViewSupported, Allocator);

    if (Json.contains("CubemapArraysSupported"))
        Deserialize(Json["CubemapArraysSupported"], Type.CubemapArraysSupported, Allocator);

    if (Json.contains("TextureView2DOn3DSupported"))
        Deserialize(Json["TextureView2DOn3DSupported"], Type.TextureView2DOn3DSupported, Allocator);
}

inline void Serialize(nlohmann::json& Json, const SamplerProperties& Type, DynamicLinearAllocator& Allocator)
{
    if (!(Type.BorderSamplingModeSupported == SamplerProperties{}.BorderSamplingModeSupported))
        Serialize(Json["BorderSamplingModeSupported"], Type.BorderSamplingModeSupported, Allocator);

    if (!(Type.AnisotropicFilteringSupported == SamplerProperties{}.AnisotropicFilteringSupported))
        Serialize(Json["AnisotropicFilteringSupported"], Type.AnisotropicFilteringSupported, Allocator);

    if (!(Type.LODBiasSupported == SamplerProperties{}.LODBiasSupported))
        Serialize(Json["LODBiasSupported"], Type.LODBiasSupported, Allocator);
}

inline void Deserialize(const nlohmann::json& Json, SamplerProperties& Type, DynamicLinearAllocator& Allocator)
{
    if (Json.contains("BorderSamplingModeSupported"))
        Deserialize(Json["BorderSamplingModeSupported"], Type.BorderSamplingModeSupported, Allocator);

    if (Json.contains("AnisotropicFilteringSupported"))
        Deserialize(Json["AnisotropicFilteringSupported"], Type.AnisotropicFilteringSupported, Allocator);

    if (Json.contains("LODBiasSupported"))
        Deserialize(Json["LODBiasSupported"], Type.LODBiasSupported, Allocator);
}

inline void Serialize(nlohmann::json& Json, const WaveOpProperties& Type, DynamicLinearAllocator& Allocator)
{
    if (!(Type.MinSize == WaveOpProperties{}.MinSize))
        Serialize(Json["MinSize"], Type.MinSize, Allocator);

    if (!(Type.MaxSize == WaveOpProperties{}.MaxSize))
        Serialize(Json["MaxSize"], Type.MaxSize, Allocator);

    if (!(Type.SupportedStages == WaveOpProperties{}.SupportedStages))
        SerializeBitwiseEnum(Json["SupportedStages"], Type.SupportedStages, Allocator);

    if (!(Type.Features == WaveOpProperties{}.Features))
        Serialize(Json["Features"], Type.Features, Allocator);
}

inline void Deserialize(const nlohmann::json& Json, WaveOpProperties& Type, DynamicLinearAllocator& Allocator)
{
    if (Json.contains("MinSize"))
        Deserialize(Json["MinSize"], Type.MinSize, Allocator);

    if (Json.contains("MaxSize"))
        Deserialize(Json["MaxSize"], Type.MaxSize, Allocator);

    if (Json.contains("SupportedStages"))
        DeserializeBitwiseEnum(Json["SupportedStages"], Type.SupportedStages, Allocator);

    if (Json.contains("Features"))
        Deserialize(Json["Features"], Type.Features, Allocator);
}

inline void Serialize(nlohmann::json& Json, const BufferProperties& Type, DynamicLinearAllocator& Allocator)
{
    if (!(Type.ConstantBufferOffsetAlignment == BufferProperties{}.ConstantBufferOffsetAlignment))
        Serialize(Json["ConstantBufferOffsetAlignment"], Type.ConstantBufferOffsetAlignment, Allocator);

    if (!(Type.StructuredBufferOffsetAlignment == BufferProperties{}.StructuredBufferOffsetAlignment))
        Serialize(Json["StructuredBufferOffsetAlignment"], Type.StructuredBufferOffsetAlignment, Allocator);
}

inline void Deserialize(const nlohmann::json& Json, BufferProperties& Type, DynamicLinearAllocator& Allocator)
{
    if (Json.contains("ConstantBufferOffsetAlignment"))
        Deserialize(Json["ConstantBufferOffsetAlignment"], Type.ConstantBufferOffsetAlignment, Allocator);

    if (Json.contains("StructuredBufferOffsetAlignment"))
        Deserialize(Json["StructuredBufferOffsetAlignment"], Type.StructuredBufferOffsetAlignment, Allocator);
}

inline void Serialize(nlohmann::json& Json, const RayTracingProperties& Type, DynamicLinearAllocator& Allocator)
{
    if (!(Type.MaxRecursionDepth == RayTracingProperties{}.MaxRecursionDepth))
        Serialize(Json["MaxRecursionDepth"], Type.MaxRecursionDepth, Allocator);

    if (!(Type.ShaderGroupHandleSize == RayTracingProperties{}.ShaderGroupHandleSize))
        Serialize(Json["ShaderGroupHandleSize"], Type.ShaderGroupHandleSize, Allocator);

    if (!(Type.MaxShaderRecordStride == RayTracingProperties{}.MaxShaderRecordStride))
        Serialize(Json["MaxShaderRecordStride"], Type.MaxShaderRecordStride, Allocator);

    if (!(Type.ShaderGroupBaseAlignment == RayTracingProperties{}.ShaderGroupBaseAlignment))
        Serialize(Json["ShaderGroupBaseAlignment"], Type.ShaderGroupBaseAlignment, Allocator);

    if (!(Type.MaxRayGenThreads == RayTracingProperties{}.MaxRayGenThreads))
        Serialize(Json["MaxRayGenThreads"], Type.MaxRayGenThreads, Allocator);

    if (!(Type.MaxInstancesPerTLAS == RayTracingProperties{}.MaxInstancesPerTLAS))
        Serialize(Json["MaxInstancesPerTLAS"], Type.MaxInstancesPerTLAS, Allocator);

    if (!(Type.MaxPrimitivesPerBLAS == RayTracingProperties{}.MaxPrimitivesPerBLAS))
        Serialize(Json["MaxPrimitivesPerBLAS"], Type.MaxPrimitivesPerBLAS, Allocator);

    if (!(Type.MaxGeometriesPerBLAS == RayTracingProperties{}.MaxGeometriesPerBLAS))
        Serialize(Json["MaxGeometriesPerBLAS"], Type.MaxGeometriesPerBLAS, Allocator);

    if (!(Type.VertexBufferAlignment == RayTracingProperties{}.VertexBufferAlignment))
        Serialize(Json["VertexBufferAlignment"], Type.VertexBufferAlignment, Allocator);

    if (!(Type.IndexBufferAlignment == RayTracingProperties{}.IndexBufferAlignment))
        Serialize(Json["IndexBufferAlignment"], Type.IndexBufferAlignment, Allocator);

    if (!(Type.TransformBufferAlignment == RayTracingProperties{}.TransformBufferAlignment))
        Serialize(Json["TransformBufferAlignment"], Type.TransformBufferAlignment, Allocator);

    if (!(Type.BoxBufferAlignment == RayTracingProperties{}.BoxBufferAlignment))
        Serialize(Json["BoxBufferAlignment"], Type.BoxBufferAlignment, Allocator);

    if (!(Type.ScratchBufferAlignment == RayTracingProperties{}.ScratchBufferAlignment))
        Serialize(Json["ScratchBufferAlignment"], Type.ScratchBufferAlignment, Allocator);

    if (!(Type.InstanceBufferAlignment == RayTracingProperties{}.InstanceBufferAlignment))
        Serialize(Json["InstanceBufferAlignment"], Type.InstanceBufferAlignment, Allocator);

    if (!(Type.CapFlags == RayTracingProperties{}.CapFlags))
        Serialize(Json["CapFlags"], Type.CapFlags, Allocator);
}

inline void Deserialize(const nlohmann::json& Json, RayTracingProperties& Type, DynamicLinearAllocator& Allocator)
{
    if (Json.contains("MaxRecursionDepth"))
        Deserialize(Json["MaxRecursionDepth"], Type.MaxRecursionDepth, Allocator);

    if (Json.contains("ShaderGroupHandleSize"))
        Deserialize(Json["ShaderGroupHandleSize"], Type.ShaderGroupHandleSize, Allocator);

    if (Json.contains("MaxShaderRecordStride"))
        Deserialize(Json["MaxShaderRecordStride"], Type.MaxShaderRecordStride, Allocator);

    if (Json.contains("ShaderGroupBaseAlignment"))
        Deserialize(Json["ShaderGroupBaseAlignment"], Type.ShaderGroupBaseAlignment, Allocator);

    if (Json.contains("MaxRayGenThreads"))
        Deserialize(Json["MaxRayGenThreads"], Type.MaxRayGenThreads, Allocator);

    if (Json.contains("MaxInstancesPerTLAS"))
        Deserialize(Json["MaxInstancesPerTLAS"], Type.MaxInstancesPerTLAS, Allocator);

    if (Json.contains("MaxPrimitivesPerBLAS"))
        Deserialize(Json["MaxPrimitivesPerBLAS"], Type.MaxPrimitivesPerBLAS, Allocator);

    if (Json.contains("MaxGeometriesPerBLAS"))
        Deserialize(Json["MaxGeometriesPerBLAS"], Type.MaxGeometriesPerBLAS, Allocator);

    if (Json.contains("VertexBufferAlignment"))
        Deserialize(Json["VertexBufferAlignment"], Type.VertexBufferAlignment, Allocator);

    if (Json.contains("IndexBufferAlignment"))
        Deserialize(Json["IndexBufferAlignment"], Type.IndexBufferAlignment, Allocator);

    if (Json.contains("TransformBufferAlignment"))
        Deserialize(Json["TransformBufferAlignment"], Type.TransformBufferAlignment, Allocator);

    if (Json.contains("BoxBufferAlignment"))
        Deserialize(Json["BoxBufferAlignment"], Type.BoxBufferAlignment, Allocator);

    if (Json.contains("ScratchBufferAlignment"))
        Deserialize(Json["ScratchBufferAlignment"], Type.ScratchBufferAlignment, Allocator);

    if (Json.contains("InstanceBufferAlignment"))
        Deserialize(Json["InstanceBufferAlignment"], Type.InstanceBufferAlignment, Allocator);

    if (Json.contains("CapFlags"))
        Deserialize(Json["CapFlags"], Type.CapFlags, Allocator);
}

inline void Serialize(nlohmann::json& Json, const MeshShaderProperties& Type, DynamicLinearAllocator& Allocator)
{
    if (!(Type.MaxTaskCount == MeshShaderProperties{}.MaxTaskCount))
        Serialize(Json["MaxTaskCount"], Type.MaxTaskCount, Allocator);
}

inline void Deserialize(const nlohmann::json& Json, MeshShaderProperties& Type, DynamicLinearAllocator& Allocator)
{
    if (Json.contains("MaxTaskCount"))
        Deserialize(Json["MaxTaskCount"], Type.MaxTaskCount, Allocator);
}

inline void Serialize(nlohmann::json& Json, const ComputeShaderProperties& Type, DynamicLinearAllocator& Allocator)
{
    if (!(Type.SharedMemorySize == ComputeShaderProperties{}.SharedMemorySize))
        Serialize(Json["SharedMemorySize"], Type.SharedMemorySize, Allocator);

    if (!(Type.MaxThreadGroupInvocations == ComputeShaderProperties{}.MaxThreadGroupInvocations))
        Serialize(Json["MaxThreadGroupInvocations"], Type.MaxThreadGroupInvocations, Allocator);

    if (!(Type.MaxThreadGroupSizeX == ComputeShaderProperties{}.MaxThreadGroupSizeX))
        Serialize(Json["MaxThreadGroupSizeX"], Type.MaxThreadGroupSizeX, Allocator);

    if (!(Type.MaxThreadGroupSizeY == ComputeShaderProperties{}.MaxThreadGroupSizeY))
        Serialize(Json["MaxThreadGroupSizeY"], Type.MaxThreadGroupSizeY, Allocator);

    if (!(Type.MaxThreadGroupSizeZ == ComputeShaderProperties{}.MaxThreadGroupSizeZ))
        Serialize(Json["MaxThreadGroupSizeZ"], Type.MaxThreadGroupSizeZ, Allocator);

    if (!(Type.MaxThreadGroupCountX == ComputeShaderProperties{}.MaxThreadGroupCountX))
        Serialize(Json["MaxThreadGroupCountX"], Type.MaxThreadGroupCountX, Allocator);

    if (!(Type.MaxThreadGroupCountY == ComputeShaderProperties{}.MaxThreadGroupCountY))
        Serialize(Json["MaxThreadGroupCountY"], Type.MaxThreadGroupCountY, Allocator);

    if (!(Type.MaxThreadGroupCountZ == ComputeShaderProperties{}.MaxThreadGroupCountZ))
        Serialize(Json["MaxThreadGroupCountZ"], Type.MaxThreadGroupCountZ, Allocator);
}

inline void Deserialize(const nlohmann::json& Json, ComputeShaderProperties& Type, DynamicLinearAllocator& Allocator)
{
    if (Json.contains("SharedMemorySize"))
        Deserialize(Json["SharedMemorySize"], Type.SharedMemorySize, Allocator);

    if (Json.contains("MaxThreadGroupInvocations"))
        Deserialize(Json["MaxThreadGroupInvocations"], Type.MaxThreadGroupInvocations, Allocator);

    if (Json.contains("MaxThreadGroupSizeX"))
        Deserialize(Json["MaxThreadGroupSizeX"], Type.MaxThreadGroupSizeX, Allocator);

    if (Json.contains("MaxThreadGroupSizeY"))
        Deserialize(Json["MaxThreadGroupSizeY"], Type.MaxThreadGroupSizeY, Allocator);

    if (Json.contains("MaxThreadGroupSizeZ"))
        Deserialize(Json["MaxThreadGroupSizeZ"], Type.MaxThreadGroupSizeZ, Allocator);

    if (Json.contains("MaxThreadGroupCountX"))
        Deserialize(Json["MaxThreadGroupCountX"], Type.MaxThreadGroupCountX, Allocator);

    if (Json.contains("MaxThreadGroupCountY"))
        Deserialize(Json["MaxThreadGroupCountY"], Type.MaxThreadGroupCountY, Allocator);

    if (Json.contains("MaxThreadGroupCountZ"))
        Deserialize(Json["MaxThreadGroupCountZ"], Type.MaxThreadGroupCountZ, Allocator);
}

inline void Serialize(nlohmann::json& Json, const NDCAttribs& Type, DynamicLinearAllocator& Allocator)
{
    if (!(Type.MinZ == NDCAttribs{}.MinZ))
        Serialize(Json["MinZ"], Type.MinZ, Allocator);

    if (!(Type.ZtoDepthScale == NDCAttribs{}.ZtoDepthScale))
        Serialize(Json["ZtoDepthScale"], Type.ZtoDepthScale, Allocator);

    if (!(Type.YtoVScale == NDCAttribs{}.YtoVScale))
        Serialize(Json["YtoVScale"], Type.YtoVScale, Allocator);
}

inline void Deserialize(const nlohmann::json& Json, NDCAttribs& Type, DynamicLinearAllocator& Allocator)
{
    if (Json.contains("MinZ"))
        Deserialize(Json["MinZ"], Type.MinZ, Allocator);

    if (Json.contains("ZtoDepthScale"))
        Deserialize(Json["ZtoDepthScale"], Type.ZtoDepthScale, Allocator);

    if (Json.contains("YtoVScale"))
        Deserialize(Json["YtoVScale"], Type.YtoVScale, Allocator);
}

inline void Serialize(nlohmann::json& Json, const RenderDeviceInfo& Type, DynamicLinearAllocator& Allocator)
{
    if (!(Type.Type == RenderDeviceInfo{}.Type))
        Serialize(Json["Type"], Type.Type, Allocator);

    if (!(Type.APIVersion == RenderDeviceInfo{}.APIVersion))
        Serialize(Json["APIVersion"], Type.APIVersion, Allocator);

    if (!(Type.Features == RenderDeviceInfo{}.Features))
        Serialize(Json["Features"], Type.Features, Allocator);

    if (!(Type.NDC == RenderDeviceInfo{}.NDC))
        Serialize(Json["NDC"], Type.NDC, Allocator);
}

inline void Deserialize(const nlohmann::json& Json, RenderDeviceInfo& Type, DynamicLinearAllocator& Allocator)
{
    if (Json.contains("Type"))
        Deserialize(Json["Type"], Type.Type, Allocator);

    if (Json.contains("APIVersion"))
        Deserialize(Json["APIVersion"], Type.APIVersion, Allocator);

    if (Json.contains("Features"))
        Deserialize(Json["Features"], Type.Features, Allocator);

    if (Json.contains("NDC"))
        Deserialize(Json["NDC"], Type.NDC, Allocator);
}

inline void Serialize(nlohmann::json& Json, const AdapterMemoryInfo& Type, DynamicLinearAllocator& Allocator)
{
    if (!(Type.LocalMemory == AdapterMemoryInfo{}.LocalMemory))
        Serialize(Json["LocalMemory"], Type.LocalMemory, Allocator);

    if (!(Type.HostVisibleMemory == AdapterMemoryInfo{}.HostVisibleMemory))
        Serialize(Json["HostVisibleMemory"], Type.HostVisibleMemory, Allocator);

    if (!(Type.UnifiedMemory == AdapterMemoryInfo{}.UnifiedMemory))
        Serialize(Json["UnifiedMemory"], Type.UnifiedMemory, Allocator);

    if (!(Type.MaxMemoryAllocation == AdapterMemoryInfo{}.MaxMemoryAllocation))
        Serialize(Json["MaxMemoryAllocation"], Type.MaxMemoryAllocation, Allocator);

    if (!(Type.UnifiedMemoryCPUAccess == AdapterMemoryInfo{}.UnifiedMemoryCPUAccess))
        Serialize(Json["UnifiedMemoryCPUAccess"], Type.UnifiedMemoryCPUAccess, Allocator);

    if (!(Type.MemorylessTextureBindFlags == AdapterMemoryInfo{}.MemorylessTextureBindFlags))
        Serialize(Json["MemorylessTextureBindFlags"], Type.MemorylessTextureBindFlags, Allocator);
}

inline void Deserialize(const nlohmann::json& Json, AdapterMemoryInfo& Type, DynamicLinearAllocator& Allocator)
{
    if (Json.contains("LocalMemory"))
        Deserialize(Json["LocalMemory"], Type.LocalMemory, Allocator);

    if (Json.contains("HostVisibleMemory"))
        Deserialize(Json["HostVisibleMemory"], Type.HostVisibleMemory, Allocator);

    if (Json.contains("UnifiedMemory"))
        Deserialize(Json["UnifiedMemory"], Type.UnifiedMemory, Allocator);

    if (Json.contains("MaxMemoryAllocation"))
        Deserialize(Json["MaxMemoryAllocation"], Type.MaxMemoryAllocation, Allocator);

    if (Json.contains("UnifiedMemoryCPUAccess"))
        Deserialize(Json["UnifiedMemoryCPUAccess"], Type.UnifiedMemoryCPUAccess, Allocator);

    if (Json.contains("MemorylessTextureBindFlags"))
        Deserialize(Json["MemorylessTextureBindFlags"], Type.MemorylessTextureBindFlags, Allocator);
}

inline void Serialize(nlohmann::json& Json, const ShadingRateMode& Type, DynamicLinearAllocator& Allocator)
{
    if (!(Type.Rate == ShadingRateMode{}.Rate))
        Serialize(Json["Rate"], Type.Rate, Allocator);

    if (!(Type.SampleBits == ShadingRateMode{}.SampleBits))
        SerializeBitwiseEnum(Json["SampleBits"], Type.SampleBits, Allocator);
}

inline void Deserialize(const nlohmann::json& Json, ShadingRateMode& Type, DynamicLinearAllocator& Allocator)
{
    if (Json.contains("Rate"))
        Deserialize(Json["Rate"], Type.Rate, Allocator);

    if (Json.contains("SampleBits"))
        DeserializeBitwiseEnum(Json["SampleBits"], Type.SampleBits, Allocator);
}

inline void Serialize(nlohmann::json& Json, const ShadingRateProperties& Type, DynamicLinearAllocator& Allocator)
{
    if (!CompareConstArray(Type.ShadingRates, ShadingRateProperties{}.ShadingRates))
        SerializeConstArray(Json["ShadingRates"], Type.ShadingRates, Allocator);

    if (!(Type.NumShadingRates == ShadingRateProperties{}.NumShadingRates))
        Serialize(Json["NumShadingRates"], Type.NumShadingRates, Allocator);

    if (!(Type.CapFlags == ShadingRateProperties{}.CapFlags))
        Serialize(Json["CapFlags"], Type.CapFlags, Allocator);

    if (!(Type.Combiners == ShadingRateProperties{}.Combiners))
        Serialize(Json["Combiners"], Type.Combiners, Allocator);

    if (!(Type.Format == ShadingRateProperties{}.Format))
        Serialize(Json["Format"], Type.Format, Allocator);

    if (!(Type.ShadingRateTextureAccess == ShadingRateProperties{}.ShadingRateTextureAccess))
        Serialize(Json["ShadingRateTextureAccess"], Type.ShadingRateTextureAccess, Allocator);

    if (!(Type.BindFlags == ShadingRateProperties{}.BindFlags))
        Serialize(Json["BindFlags"], Type.BindFlags, Allocator);

    if (!CompareConstArray(Type.MinTileSize, ShadingRateProperties{}.MinTileSize))
        SerializeConstArray(Json["MinTileSize"], Type.MinTileSize, Allocator);

    if (!CompareConstArray(Type.MaxTileSize, ShadingRateProperties{}.MaxTileSize))
        SerializeConstArray(Json["MaxTileSize"], Type.MaxTileSize, Allocator);

    if (!(Type.MaxSabsampledArraySlices == ShadingRateProperties{}.MaxSabsampledArraySlices))
        Serialize(Json["MaxSabsampledArraySlices"], Type.MaxSabsampledArraySlices, Allocator);
}

inline void Deserialize(const nlohmann::json& Json, ShadingRateProperties& Type, DynamicLinearAllocator& Allocator)
{
    if (Json.contains("ShadingRates"))
        DeserializeConstArray(Json["ShadingRates"], Type.ShadingRates, Allocator);

    if (Json.contains("NumShadingRates"))
        Deserialize(Json["NumShadingRates"], Type.NumShadingRates, Allocator);

    if (Json.contains("CapFlags"))
        Deserialize(Json["CapFlags"], Type.CapFlags, Allocator);

    if (Json.contains("Combiners"))
        Deserialize(Json["Combiners"], Type.Combiners, Allocator);

    if (Json.contains("Format"))
        Deserialize(Json["Format"], Type.Format, Allocator);

    if (Json.contains("ShadingRateTextureAccess"))
        Deserialize(Json["ShadingRateTextureAccess"], Type.ShadingRateTextureAccess, Allocator);

    if (Json.contains("BindFlags"))
        Deserialize(Json["BindFlags"], Type.BindFlags, Allocator);

    if (Json.contains("MinTileSize"))
        DeserializeConstArray(Json["MinTileSize"], Type.MinTileSize, Allocator);

    if (Json.contains("MaxTileSize"))
        DeserializeConstArray(Json["MaxTileSize"], Type.MaxTileSize, Allocator);

    if (Json.contains("MaxSabsampledArraySlices"))
        Deserialize(Json["MaxSabsampledArraySlices"], Type.MaxSabsampledArraySlices, Allocator);
}

inline void Serialize(nlohmann::json& Json, const DrawCommandProperties& Type, DynamicLinearAllocator& Allocator)
{
    if (!(Type.CapFlags == DrawCommandProperties{}.CapFlags))
        Serialize(Json["CapFlags"], Type.CapFlags, Allocator);

    if (!(Type.MaxIndexValue == DrawCommandProperties{}.MaxIndexValue))
        Serialize(Json["MaxIndexValue"], Type.MaxIndexValue, Allocator);

    if (!(Type.MaxDrawIndirectCount == DrawCommandProperties{}.MaxDrawIndirectCount))
        Serialize(Json["MaxDrawIndirectCount"], Type.MaxDrawIndirectCount, Allocator);
}

inline void Deserialize(const nlohmann::json& Json, DrawCommandProperties& Type, DynamicLinearAllocator& Allocator)
{
    if (Json.contains("CapFlags"))
        Deserialize(Json["CapFlags"], Type.CapFlags, Allocator);

    if (Json.contains("MaxIndexValue"))
        Deserialize(Json["MaxIndexValue"], Type.MaxIndexValue, Allocator);

    if (Json.contains("MaxDrawIndirectCount"))
        Deserialize(Json["MaxDrawIndirectCount"], Type.MaxDrawIndirectCount, Allocator);
}

inline void Serialize(nlohmann::json& Json, const SparseResourceProperties& Type, DynamicLinearAllocator& Allocator)
{
    if (!(Type.AddressSpaceSize == SparseResourceProperties{}.AddressSpaceSize))
        Serialize(Json["AddressSpaceSize"], Type.AddressSpaceSize, Allocator);

    if (!(Type.ResourceSpaceSize == SparseResourceProperties{}.ResourceSpaceSize))
        Serialize(Json["ResourceSpaceSize"], Type.ResourceSpaceSize, Allocator);

    if (!(Type.CapFlags == SparseResourceProperties{}.CapFlags))
        Serialize(Json["CapFlags"], Type.CapFlags, Allocator);

    if (!(Type.StandardBlockSize == SparseResourceProperties{}.StandardBlockSize))
        Serialize(Json["StandardBlockSize"], Type.StandardBlockSize, Allocator);

    if (!(Type.BufferBindFlags == SparseResourceProperties{}.BufferBindFlags))
        Serialize(Json["BufferBindFlags"], Type.BufferBindFlags, Allocator);

    if (!(Type._Padding == SparseResourceProperties{}._Padding))
        Serialize(Json["_Padding"], Type._Padding, Allocator);
}

inline void Deserialize(const nlohmann::json& Json, SparseResourceProperties& Type, DynamicLinearAllocator& Allocator)
{
    if (Json.contains("AddressSpaceSize"))
        Deserialize(Json["AddressSpaceSize"], Type.AddressSpaceSize, Allocator);

    if (Json.contains("ResourceSpaceSize"))
        Deserialize(Json["ResourceSpaceSize"], Type.ResourceSpaceSize, Allocator);

    if (Json.contains("CapFlags"))
        Deserialize(Json["CapFlags"], Type.CapFlags, Allocator);

    if (Json.contains("StandardBlockSize"))
        Deserialize(Json["StandardBlockSize"], Type.StandardBlockSize, Allocator);

    if (Json.contains("BufferBindFlags"))
        Deserialize(Json["BufferBindFlags"], Type.BufferBindFlags, Allocator);

    if (Json.contains("_Padding"))
        Deserialize(Json["_Padding"], Type._Padding, Allocator);
}

inline void Serialize(nlohmann::json& Json, const CommandQueueInfo& Type, DynamicLinearAllocator& Allocator)
{
    if (!(Type.QueueType == CommandQueueInfo{}.QueueType))
        Serialize(Json["QueueType"], Type.QueueType, Allocator);

    if (!(Type.MaxDeviceContexts == CommandQueueInfo{}.MaxDeviceContexts))
        Serialize(Json["MaxDeviceContexts"], Type.MaxDeviceContexts, Allocator);

    if (!CompareConstArray(Type.TextureCopyGranularity, CommandQueueInfo{}.TextureCopyGranularity))
        SerializeConstArray(Json["TextureCopyGranularity"], Type.TextureCopyGranularity, Allocator);
}

inline void Deserialize(const nlohmann::json& Json, CommandQueueInfo& Type, DynamicLinearAllocator& Allocator)
{
    if (Json.contains("QueueType"))
        Deserialize(Json["QueueType"], Type.QueueType, Allocator);

    if (Json.contains("MaxDeviceContexts"))
        Deserialize(Json["MaxDeviceContexts"], Type.MaxDeviceContexts, Allocator);

    if (Json.contains("TextureCopyGranularity"))
        DeserializeConstArray(Json["TextureCopyGranularity"], Type.TextureCopyGranularity, Allocator);
}

inline void Serialize(nlohmann::json& Json, const GraphicsAdapterInfo& Type, DynamicLinearAllocator& Allocator)
{
    if (!CompareConstArray(Type.Description, GraphicsAdapterInfo{}.Description))
        SerializeConstArray(Json["Description"], Type.Description, Allocator);

    if (!(Type.Type == GraphicsAdapterInfo{}.Type))
        Serialize(Json["Type"], Type.Type, Allocator);

    if (!(Type.Vendor == GraphicsAdapterInfo{}.Vendor))
        Serialize(Json["Vendor"], Type.Vendor, Allocator);

    if (!(Type.VendorId == GraphicsAdapterInfo{}.VendorId))
        Serialize(Json["VendorId"], Type.VendorId, Allocator);

    if (!(Type.DeviceId == GraphicsAdapterInfo{}.DeviceId))
        Serialize(Json["DeviceId"], Type.DeviceId, Allocator);

    if (!(Type.NumOutputs == GraphicsAdapterInfo{}.NumOutputs))
        Serialize(Json["NumOutputs"], Type.NumOutputs, Allocator);

    if (!(Type.Memory == GraphicsAdapterInfo{}.Memory))
        Serialize(Json["Memory"], Type.Memory, Allocator);

    if (!(Type.RayTracing == GraphicsAdapterInfo{}.RayTracing))
        Serialize(Json["RayTracing"], Type.RayTracing, Allocator);

    if (!(Type.WaveOp == GraphicsAdapterInfo{}.WaveOp))
        Serialize(Json["WaveOp"], Type.WaveOp, Allocator);

    if (!(Type.Buffer == GraphicsAdapterInfo{}.Buffer))
        Serialize(Json["Buffer"], Type.Buffer, Allocator);

    if (!(Type.Texture == GraphicsAdapterInfo{}.Texture))
        Serialize(Json["Texture"], Type.Texture, Allocator);

    if (!(Type.Sampler == GraphicsAdapterInfo{}.Sampler))
        Serialize(Json["Sampler"], Type.Sampler, Allocator);

    if (!(Type.MeshShader == GraphicsAdapterInfo{}.MeshShader))
        Serialize(Json["MeshShader"], Type.MeshShader, Allocator);

    if (!(Type.ShadingRate == GraphicsAdapterInfo{}.ShadingRate))
        Serialize(Json["ShadingRate"], Type.ShadingRate, Allocator);

    if (!(Type.ComputeShader == GraphicsAdapterInfo{}.ComputeShader))
        Serialize(Json["ComputeShader"], Type.ComputeShader, Allocator);

    if (!(Type.DrawCommand == GraphicsAdapterInfo{}.DrawCommand))
        Serialize(Json["DrawCommand"], Type.DrawCommand, Allocator);

    if (!(Type.SparseResources == GraphicsAdapterInfo{}.SparseResources))
        Serialize(Json["SparseResources"], Type.SparseResources, Allocator);

    if (!(Type.Features == GraphicsAdapterInfo{}.Features))
        Serialize(Json["Features"], Type.Features, Allocator);

    if (!CompareConstArray(Type.Queues, GraphicsAdapterInfo{}.Queues))
        SerializeConstArray(Json["Queues"], Type.Queues, Allocator);

    if (!(Type.NumQueues == GraphicsAdapterInfo{}.NumQueues))
        Serialize(Json["NumQueues"], Type.NumQueues, Allocator);
}

inline void Deserialize(const nlohmann::json& Json, GraphicsAdapterInfo& Type, DynamicLinearAllocator& Allocator)
{
    if (Json.contains("Description"))
        DeserializeConstArray(Json["Description"], Type.Description, Allocator);

    if (Json.contains("Type"))
        Deserialize(Json["Type"], Type.Type, Allocator);

    if (Json.contains("Vendor"))
        Deserialize(Json["Vendor"], Type.Vendor, Allocator);

    if (Json.contains("VendorId"))
        Deserialize(Json["VendorId"], Type.VendorId, Allocator);

    if (Json.contains("DeviceId"))
        Deserialize(Json["DeviceId"], Type.DeviceId, Allocator);

    if (Json.contains("NumOutputs"))
        Deserialize(Json["NumOutputs"], Type.NumOutputs, Allocator);

    if (Json.contains("Memory"))
        Deserialize(Json["Memory"], Type.Memory, Allocator);

    if (Json.contains("RayTracing"))
        Deserialize(Json["RayTracing"], Type.RayTracing, Allocator);

    if (Json.contains("WaveOp"))
        Deserialize(Json["WaveOp"], Type.WaveOp, Allocator);

    if (Json.contains("Buffer"))
        Deserialize(Json["Buffer"], Type.Buffer, Allocator);

    if (Json.contains("Texture"))
        Deserialize(Json["Texture"], Type.Texture, Allocator);

    if (Json.contains("Sampler"))
        Deserialize(Json["Sampler"], Type.Sampler, Allocator);

    if (Json.contains("MeshShader"))
        Deserialize(Json["MeshShader"], Type.MeshShader, Allocator);

    if (Json.contains("ShadingRate"))
        Deserialize(Json["ShadingRate"], Type.ShadingRate, Allocator);

    if (Json.contains("ComputeShader"))
        Deserialize(Json["ComputeShader"], Type.ComputeShader, Allocator);

    if (Json.contains("DrawCommand"))
        Deserialize(Json["DrawCommand"], Type.DrawCommand, Allocator);

    if (Json.contains("SparseResources"))
        Deserialize(Json["SparseResources"], Type.SparseResources, Allocator);

    if (Json.contains("Features"))
        Deserialize(Json["Features"], Type.Features, Allocator);

    if (Json.contains("Queues"))
        DeserializeConstArray(Json["Queues"], Type.Queues, Allocator);

    if (Json.contains("NumQueues"))
        Deserialize(Json["NumQueues"], Type.NumQueues, Allocator);
}

} // namespace Diligent
