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

inline void to_json(nlohmann::json& Json, const SamplerDesc& Type)
{
    nlohmann::to_json(Json, static_cast<DeviceObjectAttribs>(Type));

    if (!(Type.MinFilter == SamplerDesc{}.MinFilter))
    {
        Json["MinFilter"] = Type.MinFilter;
    }

    if (!(Type.MagFilter == SamplerDesc{}.MagFilter))
    {
        Json["MagFilter"] = Type.MagFilter;
    }

    if (!(Type.MipFilter == SamplerDesc{}.MipFilter))
    {
        Json["MipFilter"] = Type.MipFilter;
    }

    if (!(Type.AddressU == SamplerDesc{}.AddressU))
    {
        Json["AddressU"] = Type.AddressU;
    }

    if (!(Type.AddressV == SamplerDesc{}.AddressV))
    {
        Json["AddressV"] = Type.AddressV;
    }

    if (!(Type.AddressW == SamplerDesc{}.AddressW))
    {
        Json["AddressW"] = Type.AddressW;
    }

    if (!(Type.Flags == SamplerDesc{}.Flags))
    {
        Json["Flags"] = Type.Flags;
    }

    if (!(Type.MipLODBias == SamplerDesc{}.MipLODBias))
    {
        Json["MipLODBias"] = Type.MipLODBias;
    }

    if (!(Type.MaxAnisotropy == SamplerDesc{}.MaxAnisotropy))
    {
        Json["MaxAnisotropy"] = Type.MaxAnisotropy;
    }

    if (!(Type.ComparisonFunc == SamplerDesc{}.ComparisonFunc))
    {
        Json["ComparisonFunc"] = Type.ComparisonFunc;
    }

    if (!(Type.BorderColor == SamplerDesc{}.BorderColor))
    {
        Json["BorderColor"] = Type.BorderColor;
    }

    if (!(Type.MinLOD == SamplerDesc{}.MinLOD))
    {
        Json["MinLOD"] = Type.MinLOD;
    }

    if (!(Type.MaxLOD == SamplerDesc{}.MaxLOD))
    {
        Json["MaxLOD"] = Type.MaxLOD;
    }
}

inline void from_json(const nlohmann::json& Json, SamplerDesc& Type)
{
    nlohmann::from_json(Json, static_cast<DeviceObjectAttribs&>(Type));

    if (Json.contains("MinFilter"))
    {
        Json["MinFilter"].get_to(Type.MinFilter);
    }

    if (Json.contains("MagFilter"))
    {
        Json["MagFilter"].get_to(Type.MagFilter);
    }

    if (Json.contains("MipFilter"))
    {
        Json["MipFilter"].get_to(Type.MipFilter);
    }

    if (Json.contains("AddressU"))
    {
        Json["AddressU"].get_to(Type.AddressU);
    }

    if (Json.contains("AddressV"))
    {
        Json["AddressV"].get_to(Type.AddressV);
    }

    if (Json.contains("AddressW"))
    {
        Json["AddressW"].get_to(Type.AddressW);
    }

    if (Json.contains("Flags"))
    {
        Json["Flags"].get_to(Type.Flags);
    }

    if (Json.contains("MipLODBias"))
    {
        Json["MipLODBias"].get_to(Type.MipLODBias);
    }

    if (Json.contains("MaxAnisotropy"))
    {
        Json["MaxAnisotropy"].get_to(Type.MaxAnisotropy);
    }

    if (Json.contains("ComparisonFunc"))
    {
        Json["ComparisonFunc"].get_to(Type.ComparisonFunc);
    }

    if (Json.contains("BorderColor"))
    {
        Json["BorderColor"].get_to(Type.BorderColor);
    }

    if (Json.contains("MinLOD"))
    {
        Json["MinLOD"].get_to(Type.MinLOD);
    }

    if (Json.contains("MaxLOD"))
    {
        Json["MaxLOD"].get_to(Type.MaxLOD);
    }
}

} // namespace Diligent
