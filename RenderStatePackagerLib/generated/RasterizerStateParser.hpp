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

inline void Serialize(nlohmann::json& Json, const RasterizerStateDesc& Type, DynamicLinearAllocator& Allocator)
{
    if (!(Type.FillMode == RasterizerStateDesc{}.FillMode))
        Serialize(Json["FillMode"], Type.FillMode, Allocator);

    if (!(Type.CullMode == RasterizerStateDesc{}.CullMode))
        Serialize(Json["CullMode"], Type.CullMode, Allocator);

    if (!(Type.FrontCounterClockwise == RasterizerStateDesc{}.FrontCounterClockwise))
        Serialize(Json["FrontCounterClockwise"], Type.FrontCounterClockwise, Allocator);

    if (!(Type.DepthClipEnable == RasterizerStateDesc{}.DepthClipEnable))
        Serialize(Json["DepthClipEnable"], Type.DepthClipEnable, Allocator);

    if (!(Type.ScissorEnable == RasterizerStateDesc{}.ScissorEnable))
        Serialize(Json["ScissorEnable"], Type.ScissorEnable, Allocator);

    if (!(Type.AntialiasedLineEnable == RasterizerStateDesc{}.AntialiasedLineEnable))
        Serialize(Json["AntialiasedLineEnable"], Type.AntialiasedLineEnable, Allocator);

    if (!(Type.DepthBias == RasterizerStateDesc{}.DepthBias))
        Serialize(Json["DepthBias"], Type.DepthBias, Allocator);

    if (!(Type.DepthBiasClamp == RasterizerStateDesc{}.DepthBiasClamp))
        Serialize(Json["DepthBiasClamp"], Type.DepthBiasClamp, Allocator);

    if (!(Type.SlopeScaledDepthBias == RasterizerStateDesc{}.SlopeScaledDepthBias))
        Serialize(Json["SlopeScaledDepthBias"], Type.SlopeScaledDepthBias, Allocator);
}

inline void Deserialize(const nlohmann::json& Json, RasterizerStateDesc& Type, DynamicLinearAllocator& Allocator)
{
    if (Json.contains("FillMode"))
        Deserialize(Json["FillMode"], Type.FillMode, Allocator);

    if (Json.contains("CullMode"))
        Deserialize(Json["CullMode"], Type.CullMode, Allocator);

    if (Json.contains("FrontCounterClockwise"))
        Deserialize(Json["FrontCounterClockwise"], Type.FrontCounterClockwise, Allocator);

    if (Json.contains("DepthClipEnable"))
        Deserialize(Json["DepthClipEnable"], Type.DepthClipEnable, Allocator);

    if (Json.contains("ScissorEnable"))
        Deserialize(Json["ScissorEnable"], Type.ScissorEnable, Allocator);

    if (Json.contains("AntialiasedLineEnable"))
        Deserialize(Json["AntialiasedLineEnable"], Type.AntialiasedLineEnable, Allocator);

    if (Json.contains("DepthBias"))
        Deserialize(Json["DepthBias"], Type.DepthBias, Allocator);

    if (Json.contains("DepthBiasClamp"))
        Deserialize(Json["DepthBiasClamp"], Type.DepthBiasClamp, Allocator);

    if (Json.contains("SlopeScaledDepthBias"))
        Deserialize(Json["SlopeScaledDepthBias"], Type.SlopeScaledDepthBias, Allocator);
}

} // namespace Diligent
