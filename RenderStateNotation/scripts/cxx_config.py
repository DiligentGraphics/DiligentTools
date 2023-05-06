# ----------------------------------------------------------------------------
# Copyright 2019-2022 Diligent Graphics LLC
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
# In no event and under no legal theory, whether in tort (including negligence),
# contract, or otherwise, unless required by applicable law (such as deliberate
# and grossly negligent acts) or agreed to in writing, shall any Contributor be
# liable for any damages, including any direct, indirect, special, incidental,
# or consequential damages of any character arising as a result of this License or
# out of the use or inability to use the software (including but not limited to damages
# for loss of goodwill, work stoppage, computer failure or malfunction, or any and
# all other commercial damages or losses), even if such Contributor has been advised
# of the possibility of such damages.
# ----------------------------------------------------------------------------

CXX_REGISTERED_STRUCT = {
    "Version",
    "RenderTargetBlendDesc",
    "BlendStateDesc",

    "StencilOpDesc",
    "DepthStencilStateDesc",
    "RasterizerStateDesc",

    "InputLayoutDesc",
    "LayoutElement",

    "SampleDesc",
    "ShaderResourceVariableDesc",
    "PipelineResourceDesc",
    "PipelineResourceSignatureDesc",

    "SamplerDesc",
    "ImmutableSamplerDesc",

    "PipelineResourceLayoutDesc",
    "PipelineStateDesc",  

    "GraphicsPipelineDesc",
    "RayTracingPipelineDesc",
    "TilePipelineDesc",

    "RenderPassAttachmentDesc",
    "AttachmentReference",
    "ShadingRateAttachment",
    "SubpassDesc",
    "SubpassDependencyDesc",
    "RenderPassDesc",

    "ShaderDesc",
    "ShaderMacro",
    "ShaderResourceDesc",
    "ShaderCreateInfo",

    "RenderDeviceShaderVersionInfo",
    "RenderDeviceInfo",
    "GraphicsAdapterInfo",
    "DeviceFeatures",
    "AdapterMemoryInfo",
    "RayTracingProperties",
    "WaveOpProperties",
    "BufferProperties",
    "TextureProperties",
    "SamplerProperties",
    "MeshShaderProperties",
    "ShadingRateProperties",
    "ComputeShaderProperties",
    "DrawCommandProperties",
    "SparseResourceProperties",
    "ShadingRateMode",
    "CommandQueueInfo",
    "NDCAttribs",

    "SerializationDeviceD3D11Info",
    "SerializationDeviceD3D12Info",
    "SerializationDeviceVkInfo",
    "SerializationDeviceMtlInfo",
    "SerializationDeviceCreateInfo",
}

CXX_REGISTERD_BASE_STRUCT = {
    "DeviceObjectAttribs" : {"name": "Name", 'type': "const char *", "meta": "string"}
}

CXX_REGISTERED_ENUM = {
    "BLEND_FACTOR",
    "BLEND_OPERATION",
    "COLOR_MASK",
    "LOGIC_OPERATION",
    "COLOR_MASK",

    "STENCIL_OP",
    "COMPARISON_FUNCTION",

    "FILL_MODE",
    "CULL_MODE",

    "INPUT_ELEMENT_FREQUENCY",
    "VALUE_TYPE",

    "TEXTURE_FORMAT",
    "PRIMITIVE_TOPOLOGY",

    "RESOURCE_STATE",
    "ACCESS_FLAGS",
    "ATTACHMENT_LOAD_OP",
    "ATTACHMENT_STORE_OP",
  
    "PIPELINE_TYPE",
    "PIPELINE_STAGE_FLAGS",
    "PIPELINE_SHADING_RATE_FLAGS",
    "PIPELINE_RESOURCE_FLAGS",
    "PSO_CREATE_FLAGS",

    "SAMPLER_FLAGS",
    "FILTER_TYPE",
    "TEXTURE_ADDRESS_MODE",

    "SHADER_TYPE",
    "SHADER_SOURCE_LANGUAGE",
    "SHADER_COMPILER",
    "SHADER_RESOURCE_TYPE",
    "SHADER_RESOURCE_VARIABLE_TYPE",
    "SHADER_RESOURCE_VARIABLE_TYPE_FLAGS",
    "SHADER_VARIABLE_FLAGS",

    "ADAPTER_TYPE",
    "ADAPTER_VENDOR",
    "BIND_FLAGS",
    "CPU_ACCESS_FLAGS",

    "WAVE_FEATURE",
    "RAY_TRACING_CAP_FLAGS",
    "COMMAND_QUEUE_TYPE",
    "SPARSE_RESOURCE_CAP_FLAGS",
    "DRAW_COMMAND_CAP_FLAGS",
    "SHADING_RATE_CAP_FLAGS",
    "SHADING_RATE_COMBINER",
    "SHADING_RATE_TEXTURE_ACCESS",
    "SHADING_RATE_FORMAT",

    "RENDER_DEVICE_TYPE",
    "DEVICE_FEATURE_STATE",
    "SHADING_RATE",
    "SAMPLE_COUNT"
}

CXX_SUFFIX_FILE = "Parser"

CXX_EXTENSION_FILE = "hpp"

CXX_LICENCE = '''/*
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
'''
