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

#include "DepthStencilState.h"

namespace Diligent
{

NLOHMANN_JSON_SERIALIZE_ENUM(
    STENCIL_OP,
    {
        {STENCIL_OP_UNDEFINED, "UNDEFINED"},
        {STENCIL_OP_KEEP, "KEEP"},
        {STENCIL_OP_ZERO, "ZERO"},
        {STENCIL_OP_REPLACE, "REPLACE"},
        {STENCIL_OP_INCR_SAT, "INCR_SAT"},
        {STENCIL_OP_DECR_SAT, "DECR_SAT"},
        {STENCIL_OP_INVERT, "INVERT"},
        {STENCIL_OP_INCR_WRAP, "INCR_WRAP"},
        {STENCIL_OP_DECR_WRAP, "DECR_WRAP"},
        {STENCIL_OP_NUM_OPS, "NUM_OPS"},
    })

inline void to_json(nlohmann::json& Json, const StencilOpDesc& Type)
{
    if (!(Type.StencilFailOp == StencilOpDesc{}.StencilFailOp))
    {
        Json["StencilFailOp"] = Type.StencilFailOp;
    }

    if (!(Type.StencilDepthFailOp == StencilOpDesc{}.StencilDepthFailOp))
    {
        Json["StencilDepthFailOp"] = Type.StencilDepthFailOp;
    }

    if (!(Type.StencilPassOp == StencilOpDesc{}.StencilPassOp))
    {
        Json["StencilPassOp"] = Type.StencilPassOp;
    }

    if (!(Type.StencilFunc == StencilOpDesc{}.StencilFunc))
    {
        Json["StencilFunc"] = Type.StencilFunc;
    }
}

inline void from_json(const nlohmann::json& Json, StencilOpDesc& Type)
{
    if (Json.contains("StencilFailOp"))
    {
        Json["StencilFailOp"].get_to(Type.StencilFailOp);
    }

    if (Json.contains("StencilDepthFailOp"))
    {
        Json["StencilDepthFailOp"].get_to(Type.StencilDepthFailOp);
    }

    if (Json.contains("StencilPassOp"))
    {
        Json["StencilPassOp"].get_to(Type.StencilPassOp);
    }

    if (Json.contains("StencilFunc"))
    {
        Json["StencilFunc"].get_to(Type.StencilFunc);
    }
}

inline void to_json(nlohmann::json& Json, const DepthStencilStateDesc& Type)
{
    if (!(Type.DepthEnable == DepthStencilStateDesc{}.DepthEnable))
    {
        Json["DepthEnable"] = Type.DepthEnable;
    }

    if (!(Type.DepthWriteEnable == DepthStencilStateDesc{}.DepthWriteEnable))
    {
        Json["DepthWriteEnable"] = Type.DepthWriteEnable;
    }

    if (!(Type.DepthFunc == DepthStencilStateDesc{}.DepthFunc))
    {
        Json["DepthFunc"] = Type.DepthFunc;
    }

    if (!(Type.StencilEnable == DepthStencilStateDesc{}.StencilEnable))
    {
        Json["StencilEnable"] = Type.StencilEnable;
    }

    if (!(Type.StencilReadMask == DepthStencilStateDesc{}.StencilReadMask))
    {
        Json["StencilReadMask"] = Type.StencilReadMask;
    }

    if (!(Type.StencilWriteMask == DepthStencilStateDesc{}.StencilWriteMask))
    {
        Json["StencilWriteMask"] = Type.StencilWriteMask;
    }

    if (!(Type.FrontFace == DepthStencilStateDesc{}.FrontFace))
    {
        Json["FrontFace"] = Type.FrontFace;
    }

    if (!(Type.BackFace == DepthStencilStateDesc{}.BackFace))
    {
        Json["BackFace"] = Type.BackFace;
    }
}

inline void from_json(const nlohmann::json& Json, DepthStencilStateDesc& Type)
{
    if (Json.contains("DepthEnable"))
    {
        Json["DepthEnable"].get_to(Type.DepthEnable);
    }

    if (Json.contains("DepthWriteEnable"))
    {
        Json["DepthWriteEnable"].get_to(Type.DepthWriteEnable);
    }

    if (Json.contains("DepthFunc"))
    {
        Json["DepthFunc"].get_to(Type.DepthFunc);
    }

    if (Json.contains("StencilEnable"))
    {
        Json["StencilEnable"].get_to(Type.StencilEnable);
    }

    if (Json.contains("StencilReadMask"))
    {
        Json["StencilReadMask"].get_to(Type.StencilReadMask);
    }

    if (Json.contains("StencilWriteMask"))
    {
        Json["StencilWriteMask"].get_to(Type.StencilWriteMask);
    }

    if (Json.contains("FrontFace"))
    {
        Json["FrontFace"].get_to(Type.FrontFace);
    }

    if (Json.contains("BackFace"))
    {
        Json["BackFace"].get_to(Type.BackFace);
    }
}

} // namespace Diligent
