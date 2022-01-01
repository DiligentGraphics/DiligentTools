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

#pragma once

#include "Sampler.h"

namespace Diligent
{

NLOHMANN_JSON_SERIALIZE_ENUM_EX(
    SAMPLER_FLAGS,
    {
        {SAMPLER_FLAG_NONE, "NONE"},
        {SAMPLER_FLAG_SUBSAMPLED, "SUBSAMPLED"},
        {SAMPLER_FLAG_SUBSAMPLED_COARSE_RECONSTRUCTION, "SUBSAMPLED_COARSE_RECONSTRUCTION"},
        {SAMPLER_FLAG_LAST, "LAST"},
    })

inline void Serialize(nlohmann::json& Json, const SamplerDesc& Type, DynamicLinearAllocator& Allocator)
{
    Serialize(Json, static_cast<const DeviceObjectAttribs&>(Type), Allocator);

    if (!(Type.MinFilter == SamplerDesc{}.MinFilter))
        Serialize(Json["MinFilter"], Type.MinFilter, Allocator);

    if (!(Type.MagFilter == SamplerDesc{}.MagFilter))
        Serialize(Json["MagFilter"], Type.MagFilter, Allocator);

    if (!(Type.MipFilter == SamplerDesc{}.MipFilter))
        Serialize(Json["MipFilter"], Type.MipFilter, Allocator);

    if (!(Type.AddressU == SamplerDesc{}.AddressU))
        Serialize(Json["AddressU"], Type.AddressU, Allocator);

    if (!(Type.AddressV == SamplerDesc{}.AddressV))
        Serialize(Json["AddressV"], Type.AddressV, Allocator);

    if (!(Type.AddressW == SamplerDesc{}.AddressW))
        Serialize(Json["AddressW"], Type.AddressW, Allocator);

    if (!(Type.Flags == SamplerDesc{}.Flags))
        SerializeBitwiseEnum(Json["Flags"], Type.Flags, Allocator);

    if (!(Type.MipLODBias == SamplerDesc{}.MipLODBias))
        Serialize(Json["MipLODBias"], Type.MipLODBias, Allocator);

    if (!(Type.MaxAnisotropy == SamplerDesc{}.MaxAnisotropy))
        Serialize(Json["MaxAnisotropy"], Type.MaxAnisotropy, Allocator);

    if (!(Type.ComparisonFunc == SamplerDesc{}.ComparisonFunc))
        Serialize(Json["ComparisonFunc"], Type.ComparisonFunc, Allocator);

    if (!CompareConstArray(Type.BorderColor, SamplerDesc{}.BorderColor))
        SerializeConstArray(Json["BorderColor"], Type.BorderColor, Allocator);

    if (!(Type.MinLOD == SamplerDesc{}.MinLOD))
        Serialize(Json["MinLOD"], Type.MinLOD, Allocator);

    if (!(Type.MaxLOD == SamplerDesc{}.MaxLOD))
        Serialize(Json["MaxLOD"], Type.MaxLOD, Allocator);
}

inline void Deserialize(const nlohmann::json& Json, SamplerDesc& Type, DynamicLinearAllocator& Allocator)
{
    Deserialize(Json, static_cast<DeviceObjectAttribs&>(Type), Allocator);

    if (Json.contains("MinFilter"))
        Deserialize(Json["MinFilter"], Type.MinFilter, Allocator);

    if (Json.contains("MagFilter"))
        Deserialize(Json["MagFilter"], Type.MagFilter, Allocator);

    if (Json.contains("MipFilter"))
        Deserialize(Json["MipFilter"], Type.MipFilter, Allocator);

    if (Json.contains("AddressU"))
        Deserialize(Json["AddressU"], Type.AddressU, Allocator);

    if (Json.contains("AddressV"))
        Deserialize(Json["AddressV"], Type.AddressV, Allocator);

    if (Json.contains("AddressW"))
        Deserialize(Json["AddressW"], Type.AddressW, Allocator);

    if (Json.contains("Flags"))
        DeserializeBitwiseEnum(Json["Flags"], Type.Flags, Allocator);

    if (Json.contains("MipLODBias"))
        Deserialize(Json["MipLODBias"], Type.MipLODBias, Allocator);

    if (Json.contains("MaxAnisotropy"))
        Deserialize(Json["MaxAnisotropy"], Type.MaxAnisotropy, Allocator);

    if (Json.contains("ComparisonFunc"))
        Deserialize(Json["ComparisonFunc"], Type.ComparisonFunc, Allocator);

    if (Json.contains("BorderColor"))
        DeserializeConstArray(Json["BorderColor"], Type.BorderColor, Allocator);

    if (Json.contains("MinLOD"))
        Deserialize(Json["MinLOD"], Type.MinLOD, Allocator);

    if (Json.contains("MaxLOD"))
        Deserialize(Json["MaxLOD"], Type.MaxLOD, Allocator);
}

} // namespace Diligent
