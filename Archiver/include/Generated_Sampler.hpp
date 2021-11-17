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

#include "Sampler.h"

namespace Diligent
{

inline void Serialize(nlohmann::json& Json, const SamplerDesc& Type, DeviceObjectReflection* pAllocator)
{
    Serialize(Json, static_cast<DeviceObjectAttribs>(Type), pAllocator);

    if (!(Type.MinFilter == SamplerDesc{}.MinFilter))
        Serialize(Json["MinFilter"], Type.MinFilter, pAllocator);

    if (!(Type.MagFilter == SamplerDesc{}.MagFilter))
        Serialize(Json["MagFilter"], Type.MagFilter, pAllocator);

    if (!(Type.MipFilter == SamplerDesc{}.MipFilter))
        Serialize(Json["MipFilter"], Type.MipFilter, pAllocator);

    if (!(Type.AddressU == SamplerDesc{}.AddressU))
        Serialize(Json["AddressU"], Type.AddressU, pAllocator);

    if (!(Type.AddressV == SamplerDesc{}.AddressV))
        Serialize(Json["AddressV"], Type.AddressV, pAllocator);

    if (!(Type.AddressW == SamplerDesc{}.AddressW))
        Serialize(Json["AddressW"], Type.AddressW, pAllocator);

    if (!(Type.Flags == SamplerDesc{}.Flags))
        Serialize(Json["Flags"], Type.Flags, pAllocator);

    if (!(Type.MipLODBias == SamplerDesc{}.MipLODBias))
        Serialize(Json["MipLODBias"], Type.MipLODBias, pAllocator);

    if (!(Type.MaxAnisotropy == SamplerDesc{}.MaxAnisotropy))
        Serialize(Json["MaxAnisotropy"], Type.MaxAnisotropy, pAllocator);

    if (!(Type.ComparisonFunc == SamplerDesc{}.ComparisonFunc))
        Serialize(Json["ComparisonFunc"], Type.ComparisonFunc, pAllocator);

    if (!CompareConstArray(Type.BorderColor, SamplerDesc{}.BorderColor, _countof(Type.BorderColor)))
        SerializeConstArray(Json["BorderColor"], Type.BorderColor, _countof(Type.BorderColor), pAllocator);

    if (!(Type.MinLOD == SamplerDesc{}.MinLOD))
        Serialize(Json["MinLOD"], Type.MinLOD, pAllocator);

    if (!(Type.MaxLOD == SamplerDesc{}.MaxLOD))
        Serialize(Json["MaxLOD"], Type.MaxLOD, pAllocator);
}

inline void Deserialize(const nlohmann::json& Json, SamplerDesc& Type, DeviceObjectReflection* pAllocator)
{
    Deserialize(Json, static_cast<DeviceObjectAttribs&>(Type), pAllocator);

    if (Json.contains("MinFilter"))
        Deserialize(Json["MinFilter"], Type.MinFilter, pAllocator);

    if (Json.contains("MagFilter"))
        Deserialize(Json["MagFilter"], Type.MagFilter, pAllocator);

    if (Json.contains("MipFilter"))
        Deserialize(Json["MipFilter"], Type.MipFilter, pAllocator);

    if (Json.contains("AddressU"))
        Deserialize(Json["AddressU"], Type.AddressU, pAllocator);

    if (Json.contains("AddressV"))
        Deserialize(Json["AddressV"], Type.AddressV, pAllocator);

    if (Json.contains("AddressW"))
        Deserialize(Json["AddressW"], Type.AddressW, pAllocator);

    if (Json.contains("Flags"))
        Deserialize(Json["Flags"], Type.Flags, pAllocator);

    if (Json.contains("MipLODBias"))
        Deserialize(Json["MipLODBias"], Type.MipLODBias, pAllocator);

    if (Json.contains("MaxAnisotropy"))
        Deserialize(Json["MaxAnisotropy"], Type.MaxAnisotropy, pAllocator);

    if (Json.contains("ComparisonFunc"))
        Deserialize(Json["ComparisonFunc"], Type.ComparisonFunc, pAllocator);

    if (Json.contains("BorderColor"))
        DeserializeConstArray(Json["BorderColor"], Type.BorderColor, _countof(Type.BorderColor), pAllocator);

    if (Json.contains("MinLOD"))
        Deserialize(Json["MinLOD"], Type.MinLOD, pAllocator);

    if (Json.contains("MaxLOD"))
        Deserialize(Json["MaxLOD"], Type.MaxLOD, pAllocator);
}

} // namespace Diligent
