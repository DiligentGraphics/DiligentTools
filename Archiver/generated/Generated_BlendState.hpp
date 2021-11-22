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

inline void Serialize(nlohmann::json& Json, const RenderTargetBlendDesc& Type, DeviceObjectReflection* pAllocator)
{
    if (!(Type.BlendEnable == RenderTargetBlendDesc{}.BlendEnable))
        Serialize(Json["BlendEnable"], Type.BlendEnable, pAllocator);

    if (!(Type.LogicOperationEnable == RenderTargetBlendDesc{}.LogicOperationEnable))
        Serialize(Json["LogicOperationEnable"], Type.LogicOperationEnable, pAllocator);

    if (!(Type.SrcBlend == RenderTargetBlendDesc{}.SrcBlend))
        Serialize(Json["SrcBlend"], Type.SrcBlend, pAllocator);

    if (!(Type.DestBlend == RenderTargetBlendDesc{}.DestBlend))
        Serialize(Json["DestBlend"], Type.DestBlend, pAllocator);

    if (!(Type.BlendOp == RenderTargetBlendDesc{}.BlendOp))
        Serialize(Json["BlendOp"], Type.BlendOp, pAllocator);

    if (!(Type.SrcBlendAlpha == RenderTargetBlendDesc{}.SrcBlendAlpha))
        Serialize(Json["SrcBlendAlpha"], Type.SrcBlendAlpha, pAllocator);

    if (!(Type.DestBlendAlpha == RenderTargetBlendDesc{}.DestBlendAlpha))
        Serialize(Json["DestBlendAlpha"], Type.DestBlendAlpha, pAllocator);

    if (!(Type.BlendOpAlpha == RenderTargetBlendDesc{}.BlendOpAlpha))
        Serialize(Json["BlendOpAlpha"], Type.BlendOpAlpha, pAllocator);

    if (!(Type.LogicOp == RenderTargetBlendDesc{}.LogicOp))
        Serialize(Json["LogicOp"], Type.LogicOp, pAllocator);

    if (!(Type.RenderTargetWriteMask == RenderTargetBlendDesc{}.RenderTargetWriteMask))
        Serialize(Json["RenderTargetWriteMask"], Type.RenderTargetWriteMask, pAllocator);
}

inline void Deserialize(const nlohmann::json& Json, RenderTargetBlendDesc& Type, DeviceObjectReflection* pAllocator)
{
    if (Json.contains("BlendEnable"))
        Deserialize(Json["BlendEnable"], Type.BlendEnable, pAllocator);

    if (Json.contains("LogicOperationEnable"))
        Deserialize(Json["LogicOperationEnable"], Type.LogicOperationEnable, pAllocator);

    if (Json.contains("SrcBlend"))
        Deserialize(Json["SrcBlend"], Type.SrcBlend, pAllocator);

    if (Json.contains("DestBlend"))
        Deserialize(Json["DestBlend"], Type.DestBlend, pAllocator);

    if (Json.contains("BlendOp"))
        Deserialize(Json["BlendOp"], Type.BlendOp, pAllocator);

    if (Json.contains("SrcBlendAlpha"))
        Deserialize(Json["SrcBlendAlpha"], Type.SrcBlendAlpha, pAllocator);

    if (Json.contains("DestBlendAlpha"))
        Deserialize(Json["DestBlendAlpha"], Type.DestBlendAlpha, pAllocator);

    if (Json.contains("BlendOpAlpha"))
        Deserialize(Json["BlendOpAlpha"], Type.BlendOpAlpha, pAllocator);

    if (Json.contains("LogicOp"))
        Deserialize(Json["LogicOp"], Type.LogicOp, pAllocator);

    if (Json.contains("RenderTargetWriteMask"))
        Deserialize(Json["RenderTargetWriteMask"], Type.RenderTargetWriteMask, pAllocator);
}

inline void Serialize(nlohmann::json& Json, const BlendStateDesc& Type, DeviceObjectReflection* pAllocator)
{
    if (!(Type.AlphaToCoverageEnable == BlendStateDesc{}.AlphaToCoverageEnable))
        Serialize(Json["AlphaToCoverageEnable"], Type.AlphaToCoverageEnable, pAllocator);

    if (!(Type.IndependentBlendEnable == BlendStateDesc{}.IndependentBlendEnable))
        Serialize(Json["IndependentBlendEnable"], Type.IndependentBlendEnable, pAllocator);

    if (!CompareConstArray(Type.RenderTargets, BlendStateDesc{}.RenderTargets))
        SerializeConstArray(Json["RenderTargets"], Type.RenderTargets, pAllocator);
}

inline void Deserialize(const nlohmann::json& Json, BlendStateDesc& Type, DeviceObjectReflection* pAllocator)
{
    if (Json.contains("AlphaToCoverageEnable"))
        Deserialize(Json["AlphaToCoverageEnable"], Type.AlphaToCoverageEnable, pAllocator);

    if (Json.contains("IndependentBlendEnable"))
        Deserialize(Json["IndependentBlendEnable"], Type.IndependentBlendEnable, pAllocator);

    if (Json.contains("RenderTargets"))
        DeserializeConstArray(Json["RenderTargets"], Type.RenderTargets, pAllocator);
}

} // namespace Diligent
