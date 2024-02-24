/*
 *  Copyright 2019-2022 Diligent Graphics LLC
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

#include "gtest/gtest.h"
#include "DRSNLoader.hpp"
#include "GraphicsTypesOutputInserters.hpp"

using namespace Diligent;

namespace
{

TEST(Tools_RenderStateNotationParser, ParseGraphicsTypesEnums)
{
    DynamicLinearAllocator Allocator{DefaultRawMemoryAllocator::GetAllocator()};

    ASSERT_TRUE(TestEnum<VALUE_TYPE>(Allocator, VT_UNDEFINED, VT_NUM_TYPES));

    ASSERT_TRUE(TestEnum<TEXTURE_FORMAT>(Allocator, TEX_FORMAT_UNKNOWN, TEX_FORMAT_NUM_FORMATS));

    ASSERT_TRUE(TestEnum<FILTER_TYPE>(Allocator, FILTER_TYPE_UNKNOWN, FILTER_TYPE_NUM_FILTERS));

    ASSERT_TRUE(TestEnum<TEXTURE_ADDRESS_MODE>(Allocator, TEXTURE_ADDRESS_UNKNOWN, TEXTURE_ADDRESS_NUM_MODES));

    ASSERT_TRUE(TestEnum<COMPARISON_FUNCTION>(Allocator, COMPARISON_FUNC_UNKNOWN, COMPARISON_FUNC_NUM_FUNCTIONS));

    ASSERT_TRUE(TestEnum<PRIMITIVE_TOPOLOGY>(Allocator, PRIMITIVE_TOPOLOGY_UNDEFINED, PRIMITIVE_TOPOLOGY_NUM_TOPOLOGIES));

    ASSERT_TRUE(TestEnum<RENDER_DEVICE_TYPE>(Allocator, RENDER_DEVICE_TYPE_UNDEFINED, RENDER_DEVICE_TYPE_COUNT));

    ASSERT_TRUE(TestEnum<ADAPTER_TYPE>(Allocator, ADAPTER_TYPE_UNKNOWN, ADAPTER_TYPE_DISCRETE));

    ASSERT_TRUE(TestEnum<DEVICE_FEATURE_STATE>(Allocator, DEVICE_FEATURE_STATE_DISABLED, DEVICE_FEATURE_STATE_OPTIONAL));

    ASSERT_TRUE(TestBitwiseEnum<SAMPLE_COUNT>(Allocator, SAMPLE_COUNT_MAX));

    ASSERT_TRUE(TestBitwiseEnum<RESOURCE_STATE>(Allocator, RESOURCE_STATE_MAX_BIT));
}

TEST(Tools_RenderStateNotationParser, ParseVersion)
{
    CHECK_STRUCT_SIZE(Version, 8);

    DynamicLinearAllocator Allocator{DefaultRawMemoryAllocator::GetAllocator()};

    nlohmann::json JsonReference = LoadDRSNFromFile("RenderStates/GraphicsTypes/Version.json");

    Version DescReference{};
    DescReference.Major = 1;
    DescReference.Minor = 2;

    Version Desc{};
    ParseRSN(JsonReference, Desc, Allocator);
    ASSERT_EQ(Desc, DescReference);
}

TEST(Tools_RenderStateNotationParser, ParseDeviceFeatures)
{
    DynamicLinearAllocator Allocator{DefaultRawMemoryAllocator::GetAllocator()};

    nlohmann::json JsonReference = LoadDRSNFromFile("RenderStates/GraphicsTypes/DeviceFeatures.json");

    DeviceFeatures DescReference{};
    DescReference.SeparablePrograms                 = DEVICE_FEATURE_STATE_ENABLED;
    DescReference.ShaderResourceQueries             = DEVICE_FEATURE_STATE_ENABLED;
    DescReference.WireframeFill                     = DEVICE_FEATURE_STATE_ENABLED;
    DescReference.MultithreadedResourceCreation     = DEVICE_FEATURE_STATE_ENABLED;
    DescReference.ComputeShaders                    = DEVICE_FEATURE_STATE_ENABLED;
    DescReference.Tessellation                      = DEVICE_FEATURE_STATE_ENABLED;
    DescReference.MeshShaders                       = DEVICE_FEATURE_STATE_ENABLED;
    DescReference.RayTracing                        = DEVICE_FEATURE_STATE_ENABLED;
    DescReference.BindlessResources                 = DEVICE_FEATURE_STATE_ENABLED;
    DescReference.OcclusionQueries                  = DEVICE_FEATURE_STATE_ENABLED;
    DescReference.BinaryOcclusionQueries            = DEVICE_FEATURE_STATE_ENABLED;
    DescReference.TimestampQueries                  = DEVICE_FEATURE_STATE_ENABLED;
    DescReference.PipelineStatisticsQueries         = DEVICE_FEATURE_STATE_ENABLED;
    DescReference.DepthBiasClamp                    = DEVICE_FEATURE_STATE_ENABLED;
    DescReference.DepthClamp                        = DEVICE_FEATURE_STATE_ENABLED;
    DescReference.IndependentBlend                  = DEVICE_FEATURE_STATE_ENABLED;
    DescReference.DualSourceBlend                   = DEVICE_FEATURE_STATE_ENABLED;
    DescReference.MultiViewport                     = DEVICE_FEATURE_STATE_ENABLED;
    DescReference.TextureCompressionBC              = DEVICE_FEATURE_STATE_ENABLED;
    DescReference.VertexPipelineUAVWritesAndAtomics = DEVICE_FEATURE_STATE_ENABLED;
    DescReference.PixelUAVWritesAndAtomics          = DEVICE_FEATURE_STATE_ENABLED;
    DescReference.TextureUAVExtendedFormats         = DEVICE_FEATURE_STATE_ENABLED;
    DescReference.ShaderFloat16                     = DEVICE_FEATURE_STATE_ENABLED;
    DescReference.ResourceBuffer16BitAccess         = DEVICE_FEATURE_STATE_ENABLED;
    DescReference.UniformBuffer16BitAccess          = DEVICE_FEATURE_STATE_ENABLED;
    DescReference.ShaderInputOutput16               = DEVICE_FEATURE_STATE_ENABLED;
    DescReference.ShaderInt8                        = DEVICE_FEATURE_STATE_ENABLED;
    DescReference.ResourceBuffer8BitAccess          = DEVICE_FEATURE_STATE_ENABLED;
    DescReference.UniformBuffer8BitAccess           = DEVICE_FEATURE_STATE_ENABLED;
    DescReference.ShaderResourceRuntimeArray        = DEVICE_FEATURE_STATE_ENABLED;
    DescReference.WaveOp                            = DEVICE_FEATURE_STATE_ENABLED;
    DescReference.InstanceDataStepRate              = DEVICE_FEATURE_STATE_ENABLED;
    DescReference.NativeFence                       = DEVICE_FEATURE_STATE_ENABLED;
    DescReference.TileShaders                       = DEVICE_FEATURE_STATE_ENABLED;
    DescReference.TransferQueueTimestampQueries     = DEVICE_FEATURE_STATE_ENABLED;
    DescReference.VariableRateShading               = DEVICE_FEATURE_STATE_ENABLED;
    DescReference.SparseResources                   = DEVICE_FEATURE_STATE_ENABLED;
    DescReference.SubpassFramebufferFetch           = DEVICE_FEATURE_STATE_OPTIONAL;
    DescReference.TextureComponentSwizzle           = DEVICE_FEATURE_STATE_OPTIONAL;
    DescReference.TextureSubresourceViews           = DEVICE_FEATURE_STATE_ENABLED;
    DescReference.NativeMultiDraw                   = DEVICE_FEATURE_STATE_OPTIONAL;
    static_assert(sizeof(DescReference) == 43, "Did you add a new feature? Please add it to the test");

    DeviceFeatures Desc{};
    ParseRSN(JsonReference, Desc, Allocator);
    ASSERT_EQ(Desc, DescReference);
}

TEST(Tools_RenderStateNotationParser, ParseTextureProperties)
{
    CHECK_STRUCT_SIZE(TextureProperties, 32);

    DynamicLinearAllocator Allocator{DefaultRawMemoryAllocator::GetAllocator()};

    nlohmann::json JsonReference = LoadDRSNFromFile("RenderStates/GraphicsTypes/TextureProperties.json");

    TextureProperties DescReference{};
    DescReference.MaxTexture1DDimension      = 2048;
    DescReference.MaxTexture1DArraySlices    = 512;
    DescReference.MaxTexture2DDimension      = 512;
    DescReference.MaxTexture2DArraySlices    = 8;
    DescReference.MaxTexture3DDimension      = 64;
    DescReference.MaxTextureCubeDimension    = 8;
    DescReference.Texture2DMSSupported       = true;
    DescReference.Texture2DMSArraySupported  = true;
    DescReference.TextureViewSupported       = true;
    DescReference.CubemapArraysSupported     = true;
    DescReference.TextureView2DOn3DSupported = true;

    TextureProperties Desc{};
    ParseRSN(JsonReference, Desc, Allocator);
    ASSERT_EQ(Desc, DescReference);
}

TEST(Tools_RenderStateNotationParser, ParseSamplerProperties)
{
    CHECK_STRUCT_SIZE(SamplerProperties, 3);

    DynamicLinearAllocator Allocator{DefaultRawMemoryAllocator::GetAllocator()};

    nlohmann::json JsonReference = LoadDRSNFromFile("RenderStates/GraphicsTypes/SamplerProperties.json");

    SamplerProperties DescReference{};
    DescReference.MaxAnisotropy               = 16;
    DescReference.BorderSamplingModeSupported = true;
    DescReference.LODBiasSupported            = true;

    SamplerProperties Desc{};
    ParseRSN(JsonReference, Desc, Allocator);
    ASSERT_EQ(Desc, DescReference);
}

TEST(Tools_RenderStateNotationParser, ParseWaveOpProperties)
{
    CHECK_STRUCT_SIZE(WaveOpProperties, 16);

    DynamicLinearAllocator Allocator{DefaultRawMemoryAllocator::GetAllocator()};

    nlohmann::json JsonReference = LoadDRSNFromFile("RenderStates/GraphicsTypes/WaveOpProperties.json");

    WaveOpProperties DescReference{};
    DescReference.MinSize         = 1;
    DescReference.MaxSize         = 64;
    DescReference.Features        = WAVE_FEATURE_BALLOUT | WAVE_FEATURE_QUAD;
    DescReference.SupportedStages = SHADER_TYPE_VERTEX | SHADER_TYPE_PIXEL;

    WaveOpProperties Desc{};
    ParseRSN(JsonReference, Desc, Allocator);
    ASSERT_EQ(Desc, DescReference);
}

TEST(Tools_RenderStateNotationParser, ParseBufferPropertiess)
{
    CHECK_STRUCT_SIZE(BufferProperties, 8);

    DynamicLinearAllocator Allocator{DefaultRawMemoryAllocator::GetAllocator()};

    nlohmann::json JsonReference = LoadDRSNFromFile("RenderStates/GraphicsTypes/BufferProperties.json");

    BufferProperties DescReference{};
    DescReference.ConstantBufferOffsetAlignment   = 64;
    DescReference.StructuredBufferOffsetAlignment = 128;

    BufferProperties Desc{};
    ParseRSN(JsonReference, Desc, Allocator);
    ASSERT_EQ(Desc, DescReference);
}

TEST(Tools_RenderStateNotationParser, ParseRayTracingProperties)
{
    CHECK_STRUCT_SIZE(RayTracingProperties, 60);

    DynamicLinearAllocator Allocator{DefaultRawMemoryAllocator::GetAllocator()};

    nlohmann::json JsonReference = LoadDRSNFromFile("RenderStates/GraphicsTypes/RayTracingProperties.json");

    RayTracingProperties DescReference{};
    DescReference.IndexBufferAlignment     = 4;
    DescReference.InstanceBufferAlignment  = 8;
    DescReference.VertexBufferAlignment    = 16;
    DescReference.BoxBufferAlignment       = 32;
    DescReference.ScratchBufferAlignment   = 64;
    DescReference.ShaderGroupBaseAlignment = 128;
    DescReference.TransformBufferAlignment = 256;

    DescReference.MaxInstancesPerTLAS  = 512;
    DescReference.MaxPrimitivesPerBLAS = 1024;
    DescReference.MaxGeometriesPerBLAS = 2048;

    DescReference.MaxRayGenThreads      = 64;
    DescReference.MaxRecursionDepth     = 16;
    DescReference.MaxShaderRecordStride = 32;
    DescReference.ShaderGroupHandleSize = 4;

    DescReference.CapFlags = RAY_TRACING_CAP_FLAG_INLINE_RAY_TRACING | RAY_TRACING_CAP_FLAG_INDIRECT_RAY_TRACING;

    RayTracingProperties Desc{};
    ParseRSN(JsonReference, Desc, Allocator);
    ASSERT_EQ(Desc, DescReference);
}

TEST(Tools_RenderStateNotationParser, ParseMeshShaderProperties)
{
    CHECK_STRUCT_SIZE(MeshShaderProperties, 16);

    DynamicLinearAllocator Allocator{DefaultRawMemoryAllocator::GetAllocator()};

    nlohmann::json JsonReference = LoadDRSNFromFile("RenderStates/GraphicsTypes/MeshShaderProperties.json");

    MeshShaderProperties DescReference{};
    DescReference.MaxThreadGroupCountX     = 4;
    DescReference.MaxThreadGroupCountY     = 8;
    DescReference.MaxThreadGroupCountZ     = 12;
    DescReference.MaxThreadGroupTotalCount = 32;

    MeshShaderProperties Desc{};
    ParseRSN(JsonReference, Desc, Allocator);
    ASSERT_EQ(Desc, DescReference);
}

TEST(Tools_RenderStateNotationParser, ParseComputeShaderProperties)
{
    CHECK_STRUCT_SIZE(ComputeShaderProperties, 32);

    DynamicLinearAllocator Allocator{DefaultRawMemoryAllocator::GetAllocator()};

    nlohmann::json JsonReference = LoadDRSNFromFile("RenderStates/GraphicsTypes/ComputeShaderProperties.json");

    ComputeShaderProperties DescReference{};
    DescReference.SharedMemorySize    = 1024;
    DescReference.MaxThreadGroupSizeX = 4;
    DescReference.MaxThreadGroupSizeY = 8;
    DescReference.MaxThreadGroupSizeZ = 16;

    DescReference.MaxThreadGroupCountX = 256;
    DescReference.MaxThreadGroupCountY = 512;
    DescReference.MaxThreadGroupCountZ = 64;

    ComputeShaderProperties Desc{};
    ParseRSN(JsonReference, Desc, Allocator);
    ASSERT_EQ(Desc, DescReference);
}

TEST(Tools_RenderStateNotationParser, ParseNDCAttribs)
{
    CHECK_STRUCT_SIZE(NDCAttribs, 12);

    DynamicLinearAllocator Allocator{DefaultRawMemoryAllocator::GetAllocator()};

    nlohmann::json JsonReference = LoadDRSNFromFile("RenderStates/GraphicsTypes/NDCAttribs.json");

    NDCAttribs DescReference{};
    DescReference.MinZ          = 0.5f;
    DescReference.YtoVScale     = 1.0f;
    DescReference.ZtoDepthScale = 0.25f;

    NDCAttribs Desc{};
    ParseRSN(JsonReference, Desc, Allocator);
    ASSERT_EQ(Desc, DescReference);
}

TEST(Tools_RenderStateNotationParser, ParseRenderDeviceInfo)
{
    CHECK_STRUCT_SIZE(RenderDeviceInfo, 100);

    DynamicLinearAllocator Allocator{DefaultRawMemoryAllocator::GetAllocator()};

    nlohmann::json JsonReference = LoadDRSNFromFile("RenderStates/GraphicsTypes/RenderDeviceInfo.json");

    RenderDeviceInfo DescReference{};
    DescReference.APIVersion                      = Version{1, 2};
    DescReference.NDC.MinZ                        = -1.0f;
    DescReference.Type                            = RENDER_DEVICE_TYPE_VULKAN;
    DescReference.Features.BinaryOcclusionQueries = DEVICE_FEATURE_STATE_ENABLED;
    DescReference.MaxShaderVersion.HLSL           = {3, 4};
    DescReference.MaxShaderVersion.GLSL           = {5, 6};
    DescReference.MaxShaderVersion.GLESSL         = {7, 8};
    DescReference.MaxShaderVersion.MSL            = {9, 10};

    RenderDeviceInfo Desc{};
    ParseRSN(JsonReference, Desc, Allocator);
    ASSERT_EQ(Desc, DescReference);
}

TEST(Tools_RenderStateNotationParser, ParseAdapterMemoryInfo)
{
    CHECK_STRUCT_SIZE(AdapterMemoryInfo, 40);

    DynamicLinearAllocator Allocator{DefaultRawMemoryAllocator::GetAllocator()};

    nlohmann::json JsonReference = LoadDRSNFromFile("RenderStates/GraphicsTypes/AdapterMemoryInfo.json");

    AdapterMemoryInfo DescReference{};
    DescReference.LocalMemory         = 8192;
    DescReference.HostVisibleMemory   = 256;
    DescReference.UnifiedMemory       = 16364;
    DescReference.MaxMemoryAllocation = 1024;

    DescReference.UnifiedMemoryCPUAccess     = CPU_ACCESS_READ | CPU_ACCESS_WRITE;
    DescReference.MemorylessTextureBindFlags = BIND_SHADER_RESOURCE | BIND_RENDER_TARGET;

    AdapterMemoryInfo Desc{};
    ParseRSN(JsonReference, Desc, Allocator);
    ASSERT_EQ(Desc, DescReference);
}

TEST(Tools_RenderStateNotationParser, ParseShadingRateMode)
{
    CHECK_STRUCT_SIZE(ShadingRateMode, 2);

    DynamicLinearAllocator Allocator{DefaultRawMemoryAllocator::GetAllocator()};

    nlohmann::json JsonReference = LoadDRSNFromFile("RenderStates/GraphicsTypes/ShadingRateMode.json");

    ShadingRateMode DescReference{};
    DescReference.Rate       = SHADING_RATE_2X4;
    DescReference.SampleBits = SAMPLE_COUNT_4 | SAMPLE_COUNT_16;

    ShadingRateMode Desc{};
    ParseRSN(JsonReference, Desc, Allocator);
    ASSERT_EQ(Desc, DescReference);
}

TEST(Tools_RenderStateNotationParser, ParseShadingRateProperties)
{
    CHECK_STRUCT_SIZE(ShadingRateProperties, 52);

    DynamicLinearAllocator Allocator{DefaultRawMemoryAllocator::GetAllocator()};

    nlohmann::json JsonReference = LoadDRSNFromFile("RenderStates/GraphicsTypes/ShadingRateProperties.json");

    ShadingRateProperties DescReference{};
    DescReference.BindFlags = BIND_UNORDERED_ACCESS | BIND_SHADER_RESOURCE;
    DescReference.CapFlags  = SHADING_RATE_CAP_FLAG_PER_DRAW | SHADING_RATE_CAP_FLAG_PER_PRIMITIVE;
    DescReference.Combiners = SHADING_RATE_COMBINER_MIN | SHADING_RATE_COMBINER_MUL;
    DescReference.Format    = SHADING_RATE_FORMAT_COL_ROW_FP32;

    DescReference.NumShadingRates          = 8;
    DescReference.MaxSabsampledArraySlices = 4;
    DescReference.ShadingRateTextureAccess = SHADING_RATE_TEXTURE_ACCESS_ON_SET_RTV;

    DescReference.MinTileSize[0] = 2;
    DescReference.MinTileSize[1] = 4;
    DescReference.MaxTileSize[0] = 8;
    DescReference.MaxTileSize[1] = 16;

    ShadingRateProperties Desc{};
    ParseRSN(JsonReference, Desc, Allocator);
    ASSERT_EQ(Desc, DescReference);
}

TEST(Tools_RenderStateNotationParser, ParseDrawCommandProperties)
{
    CHECK_STRUCT_SIZE(DrawCommandProperties, 12);

    DynamicLinearAllocator Allocator{DefaultRawMemoryAllocator::GetAllocator()};

    nlohmann::json JsonReference = LoadDRSNFromFile("RenderStates/GraphicsTypes/DrawCommandProperties.json");

    DrawCommandProperties DescReference{};
    DescReference.CapFlags             = DRAW_COMMAND_CAP_FLAG_DRAW_INDIRECT | DRAW_COMMAND_CAP_FLAG_NATIVE_MULTI_DRAW_INDIRECT;
    DescReference.MaxDrawIndirectCount = 2048;
    DescReference.MaxIndexValue        = 1024;

    DrawCommandProperties Desc{};
    ParseRSN(JsonReference, Desc, Allocator);
    ASSERT_EQ(Desc, DescReference);
}

TEST(Tools_RenderStateNotationParser, ParseSparseResourceProperties)
{
    CHECK_STRUCT_SIZE(SparseResourceProperties, 32);

    DynamicLinearAllocator Allocator{DefaultRawMemoryAllocator::GetAllocator()};

    nlohmann::json JsonReference = LoadDRSNFromFile("RenderStates/GraphicsTypes/SparseResourceProperties.json");

    SparseResourceProperties DescReference{};
    DescReference.AddressSpaceSize  = 2048;
    DescReference.BufferBindFlags   = BIND_UNORDERED_ACCESS | BIND_SHADER_RESOURCE;
    DescReference.CapFlags          = SPARSE_RESOURCE_CAP_FLAG_ALIASED | SPARSE_RESOURCE_CAP_FLAG_BUFFER;
    DescReference.ResourceSpaceSize = 1024;
    DescReference.StandardBlockSize = 64;

    SparseResourceProperties Desc{};
    ParseRSN(JsonReference, Desc, Allocator);
    ASSERT_EQ(Desc, DescReference);
}

TEST(Tools_RenderStateNotationParser, ParseCommandQueueInfo)
{
    CHECK_STRUCT_SIZE(CommandQueueInfo, 20);

    DynamicLinearAllocator Allocator{DefaultRawMemoryAllocator::GetAllocator()};

    nlohmann::json JsonReference = LoadDRSNFromFile("RenderStates/GraphicsTypes/CommandQueueInfo.json");

    CommandQueueInfo DescReference{};
    DescReference.QueueType         = COMMAND_QUEUE_TYPE_GRAPHICS;
    DescReference.MaxDeviceContexts = 16;

    DescReference.TextureCopyGranularity[0] = 4;
    DescReference.TextureCopyGranularity[1] = 8;
    DescReference.TextureCopyGranularity[2] = 16;

    CommandQueueInfo Desc = {};
    ParseRSN(JsonReference, Desc, Allocator);
    ASSERT_EQ(Desc, DescReference);
}

TEST(Tools_RenderStateNotationParser, ParseGraphicsAdapterInfo)
{
    CHECK_STRUCT_SIZE(GraphicsAdapterInfo, 816);

    DynamicLinearAllocator Allocator{DefaultRawMemoryAllocator::GetAllocator()};

    nlohmann::json JsonReference = LoadDRSNFromFile("RenderStates/GraphicsTypes/GraphicsAdapterInfo.json");

    GraphicsAdapterInfo DescReference{};
    DescReference.Type                                 = ADAPTER_TYPE_DISCRETE;
    DescReference.Vendor                               = ADAPTER_VENDOR_NVIDIA;
    DescReference.VendorId                             = 8;
    DescReference.DeviceId                             = 128;
    DescReference.NumOutputs                           = 1;
    DescReference.Memory.LocalMemory                   = 256;
    DescReference.RayTracing.BoxBufferAlignment        = 64;
    DescReference.WaveOp.MinSize                       = 1;
    DescReference.Buffer.ConstantBufferOffsetAlignment = 64;
    DescReference.Texture.CubemapArraysSupported       = true;
    DescReference.Sampler.MaxAnisotropy                = 8;
    DescReference.MeshShader.MaxThreadGroupCountX      = 10;
    DescReference.MeshShader.MaxThreadGroupCountY      = 20;
    DescReference.MeshShader.MaxThreadGroupCountZ      = 30;
    DescReference.MeshShader.MaxThreadGroupTotalCount  = 100;
    DescReference.ShadingRate.Combiners                = SHADING_RATE_COMBINER_OVERRIDE;
    DescReference.ComputeShader.SharedMemorySize       = 1024;
    DescReference.DrawCommand.MaxDrawIndirectCount     = 4;
    DescReference.SparseResources.AddressSpaceSize     = 64;
    DescReference.Features.GeometryShaders             = DEVICE_FEATURE_STATE_ENABLED;
    DescReference.NumQueues                            = 2;
    DescReference.Queues[0].QueueType                  = COMMAND_QUEUE_TYPE_COMPUTE;
    DescReference.Queues[1].QueueType                  = COMMAND_QUEUE_TYPE_GRAPHICS;

    String Name = "NVIDIA: RTX 2080";
    memcpy(DescReference.Description, Name.c_str(), Name.size());

    GraphicsAdapterInfo Desc = {};
    ParseRSN(JsonReference, Desc, Allocator);
    ASSERT_EQ(Desc, DescReference);
}

} // namespace
