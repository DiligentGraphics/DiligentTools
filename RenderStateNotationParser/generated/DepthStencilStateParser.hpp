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

NLOHMANN_JSON_SERIALIZE_ENUM_EX(
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

inline void Serialize(nlohmann::json& Json, const StencilOpDesc& Type, DynamicLinearAllocator& Allocator)
{
    if (!(Type.StencilFailOp == StencilOpDesc{}.StencilFailOp))
        Serialize(Json["StencilFailOp"], Type.StencilFailOp, Allocator);

    if (!(Type.StencilDepthFailOp == StencilOpDesc{}.StencilDepthFailOp))
        Serialize(Json["StencilDepthFailOp"], Type.StencilDepthFailOp, Allocator);

    if (!(Type.StencilPassOp == StencilOpDesc{}.StencilPassOp))
        Serialize(Json["StencilPassOp"], Type.StencilPassOp, Allocator);

    if (!(Type.StencilFunc == StencilOpDesc{}.StencilFunc))
        Serialize(Json["StencilFunc"], Type.StencilFunc, Allocator);
}

inline void Deserialize(const nlohmann::json& Json, StencilOpDesc& Type, DynamicLinearAllocator& Allocator)
{
    if (Json.contains("StencilFailOp"))
        Deserialize(Json["StencilFailOp"], Type.StencilFailOp, Allocator);

    if (Json.contains("StencilDepthFailOp"))
        Deserialize(Json["StencilDepthFailOp"], Type.StencilDepthFailOp, Allocator);

    if (Json.contains("StencilPassOp"))
        Deserialize(Json["StencilPassOp"], Type.StencilPassOp, Allocator);

    if (Json.contains("StencilFunc"))
        Deserialize(Json["StencilFunc"], Type.StencilFunc, Allocator);
}

inline void Serialize(nlohmann::json& Json, const DepthStencilStateDesc& Type, DynamicLinearAllocator& Allocator)
{
    if (!(Type.DepthEnable == DepthStencilStateDesc{}.DepthEnable))
        Serialize(Json["DepthEnable"], Type.DepthEnable, Allocator);

    if (!(Type.DepthWriteEnable == DepthStencilStateDesc{}.DepthWriteEnable))
        Serialize(Json["DepthWriteEnable"], Type.DepthWriteEnable, Allocator);

    if (!(Type.DepthFunc == DepthStencilStateDesc{}.DepthFunc))
        Serialize(Json["DepthFunc"], Type.DepthFunc, Allocator);

    if (!(Type.StencilEnable == DepthStencilStateDesc{}.StencilEnable))
        Serialize(Json["StencilEnable"], Type.StencilEnable, Allocator);

    if (!(Type.StencilReadMask == DepthStencilStateDesc{}.StencilReadMask))
        Serialize(Json["StencilReadMask"], Type.StencilReadMask, Allocator);

    if (!(Type.StencilWriteMask == DepthStencilStateDesc{}.StencilWriteMask))
        Serialize(Json["StencilWriteMask"], Type.StencilWriteMask, Allocator);

    if (!(Type.FrontFace == DepthStencilStateDesc{}.FrontFace))
        Serialize(Json["FrontFace"], Type.FrontFace, Allocator);

    if (!(Type.BackFace == DepthStencilStateDesc{}.BackFace))
        Serialize(Json["BackFace"], Type.BackFace, Allocator);
}

inline void Deserialize(const nlohmann::json& Json, DepthStencilStateDesc& Type, DynamicLinearAllocator& Allocator)
{
    if (Json.contains("DepthEnable"))
        Deserialize(Json["DepthEnable"], Type.DepthEnable, Allocator);

    if (Json.contains("DepthWriteEnable"))
        Deserialize(Json["DepthWriteEnable"], Type.DepthWriteEnable, Allocator);

    if (Json.contains("DepthFunc"))
        Deserialize(Json["DepthFunc"], Type.DepthFunc, Allocator);

    if (Json.contains("StencilEnable"))
        Deserialize(Json["StencilEnable"], Type.StencilEnable, Allocator);

    if (Json.contains("StencilReadMask"))
        Deserialize(Json["StencilReadMask"], Type.StencilReadMask, Allocator);

    if (Json.contains("StencilWriteMask"))
        Deserialize(Json["StencilWriteMask"], Type.StencilWriteMask, Allocator);

    if (Json.contains("FrontFace"))
        Deserialize(Json["FrontFace"], Type.FrontFace, Allocator);

    if (Json.contains("BackFace"))
        Deserialize(Json["BackFace"], Type.BackFace, Allocator);
}

} // namespace Diligent
