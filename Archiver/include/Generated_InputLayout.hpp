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

#include "InputLayout.h"

namespace Diligent
{

NLOHMANN_JSON_SERIALIZE_ENUM(
    INPUT_ELEMENT_FREQUENCY,
    {
        {INPUT_ELEMENT_FREQUENCY_UNDEFINED, "UNDEFINED"},
        {INPUT_ELEMENT_FREQUENCY_PER_VERTEX, "PER_VERTEX"},
        {INPUT_ELEMENT_FREQUENCY_PER_INSTANCE, "PER_INSTANCE"},
        {INPUT_ELEMENT_FREQUENCY_NUM_FREQUENCIES, "NUM_FREQUENCIES"},
    })

inline void to_json(nlohmann::json& Json, const LayoutElement& Type)
{
    if (!CompareStr(Type.HLSLSemantic, LayoutElement{}.HLSLSemantic))
    {
        Json["HLSLSemantic"] = Type.HLSLSemantic;
    }

    if (!(Type.InputIndex == LayoutElement{}.InputIndex))
    {
        Json["InputIndex"] = Type.InputIndex;
    }

    if (!(Type.BufferSlot == LayoutElement{}.BufferSlot))
    {
        Json["BufferSlot"] = Type.BufferSlot;
    }

    if (!(Type.NumComponents == LayoutElement{}.NumComponents))
    {
        Json["NumComponents"] = Type.NumComponents;
    }

    if (!(Type.ValueType == LayoutElement{}.ValueType))
    {
        Json["ValueType"] = Type.ValueType;
    }

    if (!(Type.IsNormalized == LayoutElement{}.IsNormalized))
    {
        Json["IsNormalized"] = Type.IsNormalized;
    }

    if (!(Type.RelativeOffset == LayoutElement{}.RelativeOffset))
    {
        Json["RelativeOffset"] = Type.RelativeOffset;
    }

    if (!(Type.Stride == LayoutElement{}.Stride))
    {
        Json["Stride"] = Type.Stride;
    }

    if (!(Type.Frequency == LayoutElement{}.Frequency))
    {
        Json["Frequency"] = Type.Frequency;
    }

    if (!(Type.InstanceDataStepRate == LayoutElement{}.InstanceDataStepRate))
    {
        Json["InstanceDataStepRate"] = Type.InstanceDataStepRate;
    }
}

inline void from_json(const nlohmann::json& Json, LayoutElement& Type)
{
    if (Json.contains("HLSLSemantic"))
    {
        Type.HLSLSemantic = copy_string(Json["HLSLSemantic"].get<std::string>());
    }

    if (Json.contains("InputIndex"))
    {
        Json["InputIndex"].get_to(Type.InputIndex);
    }

    if (Json.contains("BufferSlot"))
    {
        Json["BufferSlot"].get_to(Type.BufferSlot);
    }

    if (Json.contains("NumComponents"))
    {
        Json["NumComponents"].get_to(Type.NumComponents);
    }

    if (Json.contains("ValueType"))
    {
        Json["ValueType"].get_to(Type.ValueType);
    }

    if (Json.contains("IsNormalized"))
    {
        Json["IsNormalized"].get_to(Type.IsNormalized);
    }

    if (Json.contains("RelativeOffset"))
    {
        Json["RelativeOffset"].get_to(Type.RelativeOffset);
    }

    if (Json.contains("Stride"))
    {
        Json["Stride"].get_to(Type.Stride);
    }

    if (Json.contains("Frequency"))
    {
        Json["Frequency"].get_to(Type.Frequency);
    }

    if (Json.contains("InstanceDataStepRate"))
    {
        Json["InstanceDataStepRate"].get_to(Type.InstanceDataStepRate);
    }
}

inline void to_json(nlohmann::json& Json, const InputLayoutDesc& Type)
{
    if (!(Type.LayoutElements == InputLayoutDesc{}.LayoutElements))
    {
        to_json_ptr(Json["LayoutElements"], Type.LayoutElements, Type.NumElements);
    }

    if (!(Type.NumElements == InputLayoutDesc{}.NumElements))
    {
        Json["NumElements"] = Type.NumElements;
    }
}

inline void from_json(const nlohmann::json& Json, InputLayoutDesc& Type)
{
    if (Json.contains("LayoutElements"))
    {
        from_json_ptr(Json["LayoutElements"], remove_const(&Type.LayoutElements), Json.at("NumElements"));
    }

    if (Json.contains("NumElements"))
    {
        Json["NumElements"].get_to(Type.NumElements);
    }
}

} // namespace Diligent
