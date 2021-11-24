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
    return Lhs.Count == Rhs.Count && Lhs.Quality == Rhs.Quality;
}

inline bool operator==(const PipelineResourceLayoutDesc& Lhs, const PipelineResourceLayoutDesc& Rhs)
{
    return Lhs.DefaultVariableType == Rhs.DefaultVariableType &&
        Lhs.DefaultVariableMergeStages == Rhs.DefaultVariableMergeStages &&
        Lhs.NumVariables == Rhs.NumVariables &&
        Lhs.Variables == Rhs.Variables &&
        Lhs.NumImmutableSamplers == Rhs.NumVariables &&
        Lhs.ImmutableSamplers == Rhs.ImmutableSamplers;
}

inline bool operator==(const PipelineStateDesc& Lhs, const PipelineStateDesc& Rhs)
{
    return CompareStr(Lhs.Name, Rhs.Name) &&
        Lhs.PipelineType == Rhs.PipelineType &&
        Lhs.SRBAllocationGranularity == Rhs.SRBAllocationGranularity &&
        Lhs.ImmediateContextMask == Rhs.ImmediateContextMask &&
        Lhs.ResourceLayout == Rhs.ResourceLayout;
}

inline bool operator==(const TilePipelineDesc& Lhs, const TilePipelineDesc& Rhs)
{
    return false;
}

inline bool operator==(const RayTracingPipelineDesc& Lhs, const RayTracingPipelineDesc& Rhs)
{
    return false;
}

inline bool operator==(const ShaderDesc& Lhs, const ShaderDesc& Rhs)
{
    return CompareStr(Lhs.Name, Rhs.Name) && Lhs.ShaderType == Rhs.ShaderType;
}

inline bool operator==(const RenderDeviceInfo& Lhs, const RenderDeviceInfo& Rhs)
{
    return false;
}

inline bool operator==(const DeviceFeatures& Lhs, const DeviceFeatures& Rhs)
{
    return false;
}

inline bool operator==(const ComputeShaderProperties& Lhs, const ComputeShaderProperties& Rhs)
{
    return false;
}

inline bool operator==(const CommandQueueInfo& Lhs, const CommandQueueInfo& Rhs) {
    return false;
}

inline bool operator==(const DrawCommandProperties& Lhs, const DrawCommandProperties& Rhs)
{
    return false;
}

inline bool operator==(const SparseResourceProperties& Lhs, const SparseResourceProperties& Rhs)
{
    return false;
}

inline bool operator==(const MeshShaderProperties& Lhs, const MeshShaderProperties& Rhs)
{
    return false;
}

inline bool operator==(const ShadingRateProperties& Lhs, const ShadingRateProperties& Rhs)
{
    return false;
}

inline bool operator==(const RayTracingProperties& Lhs, const RayTracingProperties& Rhs)
{
    return false;
}

inline bool operator==(const AdapterMemoryInfo& Lhs, const AdapterMemoryInfo& Rhs)
{
    return false;
}

inline bool operator==(const BufferProperties& Lhs, const BufferProperties& Rhs)
{
    return false;
}

inline bool operator==(const TextureProperties& Lhs, const TextureProperties& Rhs)
{
    return false;
}

inline bool operator==(const SamplerProperties& Lhs, const SamplerProperties& Rhs)
{
    return false;
}

inline bool operator==(const WaveOpProperties& Lhs, const WaveOpProperties& Rhs)
{
    return false;
}

inline bool operator==(const NDCAttribs& Lhs, const NDCAttribs& Rhs)
{
    return false;
}

inline bool operator==(const ShadingRateMode& Lhs, const ShadingRateMode& Rhs) {
    return false;
}

inline bool operator==(const GraphicsAdapterInfo& Lhs, const GraphicsAdapterInfo& Rhs) {
    return false;
}

inline bool operator==(const SerializationDeviceD3D11Info& Lhs, const SerializationDeviceD3D11Info& Rhs) {
    return false;
}

inline bool operator==(const SerializationDeviceD3D12Info& Lhs, const SerializationDeviceD3D12Info& Rhs) {
    return false;
}


inline bool operator==(const SerializationDeviceVkInfo& Lhs, const SerializationDeviceVkInfo& Rhs) {
    return false;
}

inline bool operator==(const SerializationDeviceMtlInfo& Lhs, const SerializationDeviceMtlInfo& Rhs) {
    return false;
}



} // namespace Diligent
