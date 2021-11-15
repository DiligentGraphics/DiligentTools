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

#include "RasterizerState.h"

namespace Diligent
{

NLOHMANN_JSON_SERIALIZE_ENUM(
    FILL_MODE,
    {
        {FILL_MODE_UNDEFINED, "UNDEFINED"},
        {FILL_MODE_WIREFRAME, "WIREFRAME"},
        {FILL_MODE_SOLID, "SOLID"},
        {FILL_MODE_NUM_MODES, "NUM_MODES"},
    })

NLOHMANN_JSON_SERIALIZE_ENUM(
    CULL_MODE,
    {
        {CULL_MODE_UNDEFINED, "UNDEFINED"},
        {CULL_MODE_NONE, "NONE"},
        {CULL_MODE_FRONT, "FRONT"},
        {CULL_MODE_BACK, "BACK"},
        {CULL_MODE_NUM_MODES, "NUM_MODES"},
    })

inline void to_json(nlohmann::json& Json, const RasterizerStateDesc& Type)
{
    if (!(Type.FillMode == RasterizerStateDesc{}.FillMode))
    {
        Json["FillMode"] = Type.FillMode;
    }

    if (!(Type.CullMode == RasterizerStateDesc{}.CullMode))
    {
        Json["CullMode"] = Type.CullMode;
    }

    if (!(Type.FrontCounterClockwise == RasterizerStateDesc{}.FrontCounterClockwise))
    {
        Json["FrontCounterClockwise"] = Type.FrontCounterClockwise;
    }

    if (!(Type.DepthClipEnable == RasterizerStateDesc{}.DepthClipEnable))
    {
        Json["DepthClipEnable"] = Type.DepthClipEnable;
    }

    if (!(Type.ScissorEnable == RasterizerStateDesc{}.ScissorEnable))
    {
        Json["ScissorEnable"] = Type.ScissorEnable;
    }

    if (!(Type.AntialiasedLineEnable == RasterizerStateDesc{}.AntialiasedLineEnable))
    {
        Json["AntialiasedLineEnable"] = Type.AntialiasedLineEnable;
    }

    if (!(Type.DepthBias == RasterizerStateDesc{}.DepthBias))
    {
        Json["DepthBias"] = Type.DepthBias;
    }

    if (!(Type.DepthBiasClamp == RasterizerStateDesc{}.DepthBiasClamp))
    {
        Json["DepthBiasClamp"] = Type.DepthBiasClamp;
    }

    if (!(Type.SlopeScaledDepthBias == RasterizerStateDesc{}.SlopeScaledDepthBias))
    {
        Json["SlopeScaledDepthBias"] = Type.SlopeScaledDepthBias;
    }
}

inline void from_json(const nlohmann::json& Json, RasterizerStateDesc& Type)
{
    if (Json.contains("FillMode"))
    {
        Json["FillMode"].get_to(Type.FillMode);
    }

    if (Json.contains("CullMode"))
    {
        Json["CullMode"].get_to(Type.CullMode);
    }

    if (Json.contains("FrontCounterClockwise"))
    {
        Json["FrontCounterClockwise"].get_to(Type.FrontCounterClockwise);
    }

    if (Json.contains("DepthClipEnable"))
    {
        Json["DepthClipEnable"].get_to(Type.DepthClipEnable);
    }

    if (Json.contains("ScissorEnable"))
    {
        Json["ScissorEnable"].get_to(Type.ScissorEnable);
    }

    if (Json.contains("AntialiasedLineEnable"))
    {
        Json["AntialiasedLineEnable"].get_to(Type.AntialiasedLineEnable);
    }

    if (Json.contains("DepthBias"))
    {
        Json["DepthBias"].get_to(Type.DepthBias);
    }

    if (Json.contains("DepthBiasClamp"))
    {
        Json["DepthBiasClamp"].get_to(Type.DepthBiasClamp);
    }

    if (Json.contains("SlopeScaledDepthBias"))
    {
        Json["SlopeScaledDepthBias"].get_to(Type.SlopeScaledDepthBias);
    }
}

} // namespace Diligent
