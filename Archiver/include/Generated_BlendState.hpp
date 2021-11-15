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

#include "BlendState.h"

namespace Diligent
{

NLOHMANN_JSON_SERIALIZE_ENUM(
    BLEND_FACTOR,
    {
        {BLEND_FACTOR_UNDEFINED, "UNDEFINED"},
        {BLEND_FACTOR_ZERO, "ZERO"},
        {BLEND_FACTOR_ONE, "ONE"},
        {BLEND_FACTOR_SRC_COLOR, "SRC_COLOR"},
        {BLEND_FACTOR_INV_SRC_COLOR, "INV_SRC_COLOR"},
        {BLEND_FACTOR_SRC_ALPHA, "SRC_ALPHA"},
        {BLEND_FACTOR_INV_SRC_ALPHA, "INV_SRC_ALPHA"},
        {BLEND_FACTOR_DEST_ALPHA, "DEST_ALPHA"},
        {BLEND_FACTOR_INV_DEST_ALPHA, "INV_DEST_ALPHA"},
        {BLEND_FACTOR_DEST_COLOR, "DEST_COLOR"},
        {BLEND_FACTOR_INV_DEST_COLOR, "INV_DEST_COLOR"},
        {BLEND_FACTOR_SRC_ALPHA_SAT, "SRC_ALPHA_SAT"},
        {BLEND_FACTOR_BLEND_FACTOR, "BLEND_FACTOR"},
        {BLEND_FACTOR_INV_BLEND_FACTOR, "INV_BLEND_FACTOR"},
        {BLEND_FACTOR_SRC1_COLOR, "SRC1_COLOR"},
        {BLEND_FACTOR_INV_SRC1_COLOR, "INV_SRC1_COLOR"},
        {BLEND_FACTOR_SRC1_ALPHA, "SRC1_ALPHA"},
        {BLEND_FACTOR_INV_SRC1_ALPHA, "INV_SRC1_ALPHA"},
        {BLEND_FACTOR_NUM_FACTORS, "NUM_FACTORS"},
    })

NLOHMANN_JSON_SERIALIZE_ENUM(
    BLEND_OPERATION,
    {
        {BLEND_OPERATION_UNDEFINED, "UNDEFINED"},
        {BLEND_OPERATION_ADD, "ADD"},
        {BLEND_OPERATION_SUBTRACT, "SUBTRACT"},
        {BLEND_OPERATION_REV_SUBTRACT, "REV_SUBTRACT"},
        {BLEND_OPERATION_MIN, "MIN"},
        {BLEND_OPERATION_MAX, "MAX"},
        {BLEND_OPERATION_NUM_OPERATIONS, "NUM_OPERATIONS"},
    })

NLOHMANN_JSON_SERIALIZE_ENUM(
    COLOR_MASK,
    {
        {COLOR_MASK_NONE, "NONE"},
        {COLOR_MASK_RED, "RED"},
        {COLOR_MASK_GREEN, "GREEN"},
        {COLOR_MASK_BLUE, "BLUE"},
        {COLOR_MASK_ALPHA, "ALPHA"},
        {COLOR_MASK_ALL, "ALL"},
    })

NLOHMANN_JSON_SERIALIZE_ENUM(
    LOGIC_OPERATION,
    {
        {LOGIC_OP_CLEAR, "CLEAR"},
        {LOGIC_OP_SET, "SET"},
        {LOGIC_OP_COPY, "COPY"},
        {LOGIC_OP_COPY_INVERTED, "COPY_INVERTED"},
        {LOGIC_OP_NOOP, "NOOP"},
        {LOGIC_OP_INVERT, "INVERT"},
        {LOGIC_OP_AND, "AND"},
        {LOGIC_OP_NAND, "NAND"},
        {LOGIC_OP_OR, "OR"},
        {LOGIC_OP_NOR, "NOR"},
        {LOGIC_OP_XOR, "XOR"},
        {LOGIC_OP_EQUIV, "EQUIV"},
        {LOGIC_OP_AND_REVERSE, "AND_REVERSE"},
        {LOGIC_OP_AND_INVERTED, "AND_INVERTED"},
        {LOGIC_OP_OR_REVERSE, "OR_REVERSE"},
        {LOGIC_OP_OR_INVERTED, "OR_INVERTED"},
        {LOGIC_OP_NUM_OPERATIONS, "NUM_OPERATIONS"},
    })

inline void to_json(nlohmann::json& Json, const RenderTargetBlendDesc& Type)
{
    if (!(Type.BlendEnable == RenderTargetBlendDesc{}.BlendEnable))
    {
        Json["BlendEnable"] = Type.BlendEnable;
    }

    if (!(Type.LogicOperationEnable == RenderTargetBlendDesc{}.LogicOperationEnable))
    {
        Json["LogicOperationEnable"] = Type.LogicOperationEnable;
    }

    if (!(Type.SrcBlend == RenderTargetBlendDesc{}.SrcBlend))
    {
        Json["SrcBlend"] = Type.SrcBlend;
    }

    if (!(Type.DestBlend == RenderTargetBlendDesc{}.DestBlend))
    {
        Json["DestBlend"] = Type.DestBlend;
    }

    if (!(Type.BlendOp == RenderTargetBlendDesc{}.BlendOp))
    {
        Json["BlendOp"] = Type.BlendOp;
    }

    if (!(Type.SrcBlendAlpha == RenderTargetBlendDesc{}.SrcBlendAlpha))
    {
        Json["SrcBlendAlpha"] = Type.SrcBlendAlpha;
    }

    if (!(Type.DestBlendAlpha == RenderTargetBlendDesc{}.DestBlendAlpha))
    {
        Json["DestBlendAlpha"] = Type.DestBlendAlpha;
    }

    if (!(Type.BlendOpAlpha == RenderTargetBlendDesc{}.BlendOpAlpha))
    {
        Json["BlendOpAlpha"] = Type.BlendOpAlpha;
    }

    if (!(Type.LogicOp == RenderTargetBlendDesc{}.LogicOp))
    {
        Json["LogicOp"] = Type.LogicOp;
    }

    if (!(Type.RenderTargetWriteMask == RenderTargetBlendDesc{}.RenderTargetWriteMask))
    {
        Json["RenderTargetWriteMask"] = Type.RenderTargetWriteMask;
    }
}

inline void from_json(const nlohmann::json& Json, RenderTargetBlendDesc& Type)
{
    if (Json.contains("BlendEnable"))
    {
        Json["BlendEnable"].get_to(Type.BlendEnable);
    }

    if (Json.contains("LogicOperationEnable"))
    {
        Json["LogicOperationEnable"].get_to(Type.LogicOperationEnable);
    }

    if (Json.contains("SrcBlend"))
    {
        Json["SrcBlend"].get_to(Type.SrcBlend);
    }

    if (Json.contains("DestBlend"))
    {
        Json["DestBlend"].get_to(Type.DestBlend);
    }

    if (Json.contains("BlendOp"))
    {
        Json["BlendOp"].get_to(Type.BlendOp);
    }

    if (Json.contains("SrcBlendAlpha"))
    {
        Json["SrcBlendAlpha"].get_to(Type.SrcBlendAlpha);
    }

    if (Json.contains("DestBlendAlpha"))
    {
        Json["DestBlendAlpha"].get_to(Type.DestBlendAlpha);
    }

    if (Json.contains("BlendOpAlpha"))
    {
        Json["BlendOpAlpha"].get_to(Type.BlendOpAlpha);
    }

    if (Json.contains("LogicOp"))
    {
        Json["LogicOp"].get_to(Type.LogicOp);
    }

    if (Json.contains("RenderTargetWriteMask"))
    {
        Json["RenderTargetWriteMask"].get_to(Type.RenderTargetWriteMask);
    }
}

inline void to_json(nlohmann::json& Json, const BlendStateDesc& Type)
{
    if (!(Type.AlphaToCoverageEnable == BlendStateDesc{}.AlphaToCoverageEnable))
    {
        Json["AlphaToCoverageEnable"] = Type.AlphaToCoverageEnable;
    }

    if (!(Type.IndependentBlendEnable == BlendStateDesc{}.IndependentBlendEnable))
    {
        Json["IndependentBlendEnable"] = Type.IndependentBlendEnable;
    }

    if (!(Type.RenderTargets == BlendStateDesc{}.RenderTargets))
    {
        Json["RenderTargets"] = Type.RenderTargets;
    }
}

inline void from_json(const nlohmann::json& Json, BlendStateDesc& Type)
{
    if (Json.contains("AlphaToCoverageEnable"))
    {
        Json["AlphaToCoverageEnable"].get_to(Type.AlphaToCoverageEnable);
    }

    if (Json.contains("IndependentBlendEnable"))
    {
        Json["IndependentBlendEnable"].get_to(Type.IndependentBlendEnable);
    }

    if (Json.contains("RenderTargets"))
    {
        Json["RenderTargets"].get_to(Type.RenderTargets);
    }
}

} // namespace Diligent
