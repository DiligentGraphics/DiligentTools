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


#include "pch.h"
#include "DefaultRawMemoryAllocator.hpp"

namespace Diligent
{

DeviceObjectReflection::DeviceObjectReflection(RefCntAutoPtr<ISerializationDevice> pDevice, RefCntAutoPtr<IShaderSourceInputStreamFactory> pStreamFactory, Uint32 DeviceBits) :
    m_pDevice(pDevice), m_pStreamFactory(pStreamFactory), m_DeviceBits(DeviceBits)
{
    m_pMemoryAllocator = std::make_unique<DynamicLinearAllocator>(DefaultRawMemoryAllocator::GetAllocator());
}

void DeviceObjectReflection::Serialize(nlohmann::json& Json, const IRenderPass* pDeviceObject)
{
    auto const& ResourceDesc = pDeviceObject->GetDesc();
    VERIFY_EXPR(ResourceDesc.Name != nullptr);
    Diligent::Serialize(Json, ResourceDesc, this);
}

void DeviceObjectReflection::Deserialize(const nlohmann::json& Json, IRenderPass** pDeviceObject)
{
    RenderPassDesc ResourceDesc = {};
    Diligent::Deserialize(Json, ResourceDesc, this);
    VERIFY_EXPR(ResourceDesc.Name != nullptr);

    RefCntAutoPtr<IRenderPass> pShader;
    m_pDevice->CreateRenderPass(ResourceDesc, &pShader);
    m_RenderPasses.push_back(pShader);

    if (!pShader)
        LOG_ERROR_AND_THROW("Failed to create RenderPass -> '", ResourceDesc.Name, "'.");
    
    *pDeviceObject = pShader;
}

void DeviceObjectReflection::Serialize(nlohmann::json& Json, const IShader* pDeviceObject)
{
    auto const& ResourceDesc = pDeviceObject->GetDesc();
    VERIFY_EXPR(ResourceDesc.Name != nullptr);

    Diligent::Serialize(Json, ResourceDesc, this);
}

void DeviceObjectReflection::Deserialize(const nlohmann::json& Json, IShader** pDeviceObject)
{

    ShaderCreateInfo ResourceDesc = {};
    Diligent::Deserialize(Json, ResourceDesc, this);
    VERIFY_EXPR(ResourceDesc.Desc.Name != nullptr);
    ResourceDesc.pShaderSourceStreamFactory = m_pStreamFactory;

    RefCntAutoPtr<IShader> pShader;
    m_pDevice->CreateShader(ResourceDesc, m_DeviceBits, &pShader);
    m_Shaders.push_back(pShader);

    if (!pShader)
        LOG_ERROR_AND_THROW("Failed to create Shader -> '", ResourceDesc.Desc.Name, "'.");
    
    *pDeviceObject = pShader;
}

void DeviceObjectReflection::Serialize(nlohmann::json& Json, const IPipelineResourceSignature* pDeviceObject)
{
    auto const& ResourceDesc = pDeviceObject->GetDesc();
    VERIFY_EXPR(ResourceDesc.Name != nullptr);

    Diligent::Serialize(Json, ResourceDesc, this);
}

void DeviceObjectReflection::Deserialize(const nlohmann::json& Json, IPipelineResourceSignature** pDeviceObject)
{
    PipelineResourceSignatureDesc ResourceDesc = {};
    Diligent::Deserialize(Json, ResourceDesc, this);

    RefCntAutoPtr<IPipelineResourceSignature> pResourceSignature;
    m_pDevice->CreatePipelineResourceSignature(ResourceDesc, m_DeviceBits, &pResourceSignature);
    m_ResourceSignatures.push_back(pResourceSignature);
   
    if (!pResourceSignature)
        LOG_ERROR_AND_THROW("Failed to create Resource Signature -> '", ResourceDesc.Name, "'.");

    *pDeviceObject = pResourceSignature;
}

void DeviceObjectReflection::Flush()
{
    m_RenderPasses.clear();
    m_Shaders.clear();
    m_ResourceSignatures.clear();
    m_pMemoryAllocator->Free();
}

} // namespace Diligent
