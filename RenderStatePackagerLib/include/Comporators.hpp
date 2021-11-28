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

namespace Diligent
{

inline bool CompareStr(const char* const Lhs, const char* const Rhs)
{
    if (Lhs == Rhs)
        return true;

    return (Lhs != nullptr && Rhs != nullptr) ? std::strcmp(Lhs, Rhs) == 0 : false;
}

template <typename Type, size_t NumElements>
inline bool CompareConstArray(const Type (&Lhs)[NumElements], const Type (&Rhs)[NumElements])
{
    for (size_t i = 0; i < NumElements; i++)
        if (!(Lhs[i] == Rhs[i]))
            return false;
    return true;
}

inline bool operator==(const SampleDesc& Lhs, const SampleDesc& Rhs)
{
    //TODO
    return false;
}

inline bool operator==(const PipelineResourceLayoutDesc& Lhs, const PipelineResourceLayoutDesc& Rhs)
{
    //TODO
    return false;
}

inline bool operator==(const PipelineStateDesc& Lhs, const PipelineStateDesc& Rhs)
{
    //TODO
    return false;
}

inline bool operator==(const TilePipelineDesc& Lhs, const TilePipelineDesc& Rhs)
{
    //TODO
    return false;
}

inline bool operator==(const RayTracingPipelineDesc& Lhs, const RayTracingPipelineDesc& Rhs)
{
    //TODO
    return false;
}

inline bool operator==(const ShaderDesc& Lhs, const ShaderDesc& Rhs)
{
    //TODO
    return false;
}

inline bool operator==(const RenderDeviceInfo& Lhs, const RenderDeviceInfo& Rhs)
{
    //TODO
    return false;
}

inline bool operator==(const DeviceFeatures& Lhs, const DeviceFeatures& Rhs)
{
    //TODO
    return false;
}

inline bool operator==(const ComputeShaderProperties& Lhs, const ComputeShaderProperties& Rhs)
{
    //TODO
    return false;
}

inline bool operator==(const CommandQueueInfo& Lhs, const CommandQueueInfo& Rhs)
{
    //TODO
    return false;
}

inline bool operator==(const DrawCommandProperties& Lhs, const DrawCommandProperties& Rhs)
{
    //TODO
    return false;
}

inline bool operator==(const SparseResourceProperties& Lhs, const SparseResourceProperties& Rhs)
{
    //TODO
    return false;
}

inline bool operator==(const MeshShaderProperties& Lhs, const MeshShaderProperties& Rhs)
{
    //TODO
    return false;
}

inline bool operator==(const ShadingRateProperties& Lhs, const ShadingRateProperties& Rhs)
{
    //TODO
    return false;
}

inline bool operator==(const RayTracingProperties& Lhs, const RayTracingProperties& Rhs)
{
    //TODO
    return false;
}

inline bool operator==(const AdapterMemoryInfo& Lhs, const AdapterMemoryInfo& Rhs)
{
    //TODO
    return false;
}

inline bool operator==(const BufferProperties& Lhs, const BufferProperties& Rhs)
{
    //TODO
    return false;
}

inline bool operator==(const TextureProperties& Lhs, const TextureProperties& Rhs)
{
    //TODO
    return false;
}

inline bool operator==(const SamplerProperties& Lhs, const SamplerProperties& Rhs)
{
    //TODO
    return false;
}

inline bool operator==(const WaveOpProperties& Lhs, const WaveOpProperties& Rhs)
{
    //TODO
    return false;
}

inline bool operator==(const NDCAttribs& Lhs, const NDCAttribs& Rhs)
{
    //TODO
    return false;
}

inline bool operator==(const ShadingRateMode& Lhs, const ShadingRateMode& Rhs)
{
    //TODO
    return false;
}

inline bool operator==(const GraphicsAdapterInfo& Lhs, const GraphicsAdapterInfo& Rhs)
{
    //TODO
    return false;
}

inline bool operator==(const SerializationDeviceD3D11Info& Lhs, const SerializationDeviceD3D11Info& Rhs)
{
    //TODO
    return false;
}

inline bool operator==(const SerializationDeviceD3D12Info& Lhs, const SerializationDeviceD3D12Info& Rhs)
{
    //TODO
    return false;
}

inline bool operator==(const SerializationDeviceVkInfo& Lhs, const SerializationDeviceVkInfo& Rhs)
{
    //TODO
    return false;
}

inline bool operator==(const SerializationDeviceMtlInfo& Lhs, const SerializationDeviceMtlInfo& Rhs)
{
    //TODO
    return false;
}

} // namespace Diligent
