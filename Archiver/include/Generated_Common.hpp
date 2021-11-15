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

namespace Diligent
{

template <typename T>
inline T** remove_const(const T** x)
{
    return const_cast<T**>(x);
}

inline const char* copy_string(const std::string& Str)
{
    return EngineEnvironment::GetInstance()->GetDeviceObjectReflection()->CopyString(Str);
}

template <typename Type>
inline void to_json_ptr(nlohmann::json& Json, const Type* pData, size_t Size)
{
    for (size_t i = 0; i < Size; i++)
        Json.push_back(pData[i]);
}

template <typename Type>
inline void from_json_ptr(const nlohmann::json& Json, Type** pObjects, size_t Size)
{
    auto* pData = EngineEnvironment::GetInstance()->GetDeviceObjectReflection()->Allocate<Type>(Size);
    for (size_t i = 0; i < Size; i++)
        pData[i] = Json[i].get<Type>();
    *pObjects = pData;
}

template <typename Type>
inline void to_json_ptr(nlohmann::json& Json, const Type* pObject)
{
    Json = *pObject;
}

template <typename Type>
inline void from_json_ptr(const nlohmann::json& Json, Type** pObject)
{
    auto* pData = EngineEnvironment::GetInstance()->GetDeviceObjectReflection()->Allocate<Type>();
    *pData      = Json.get<Type>();
    *pObject    = pData;
}

template <>
inline void to_json_ptr(nlohmann::json& Json, const void* pData, size_t Size)
{
    std::vector<uint8_t> PackedData(static_cast<const uint8_t*>(pData), static_cast<const uint8_t*>(pData) + Size);
    Json = nlohmann::json::binary(PackedData);
}

template <>
inline void from_json_ptr(const nlohmann::json& Json, void** pObject, size_t Size)
{
    auto* pData = EngineEnvironment::GetInstance()->GetDeviceObjectReflection()->Allocate<Uint8>(Size);
    std::memcpy(pData, Json.get_binary().data(), Size);
    *pObject = pData;
}

template <typename Type>
inline void to_json_interface(nlohmann::json& Json, const Type* pDeviceObject)
{
    EngineEnvironment::GetInstance()->GetDeviceObjectReflection()->Serialize(Json, pDeviceObject);
}

template <typename Type>
inline void from_json_interface(const nlohmann::json& Json, Type** pDeviceObject)
{
    EngineEnvironment::GetInstance()->GetDeviceObjectReflection()->Deserialize(Json, pDeviceObject);
}

template <typename Type>
inline void to_json_interface(nlohmann::json& Json, Type** pDeviceObjects, size_t Size)
{
    for (size_t i = 0; i < Size; i++)
    {
        auto Object = nlohmann::json::object();
        EngineEnvironment::GetInstance()->GetDeviceObjectReflection()->Serialize(Object, pDeviceObjects[i]);
        Json.push_back(Object);
    }
}

template <typename Type>
inline void from_json_interface(const nlohmann::json& Json, Type*** pDeviceObjects, size_t Size)
{
    auto* pData = EngineEnvironment::GetInstance()->GetDeviceObjectReflection()->Allocate<Type*>(Size);
    for (size_t i = 0; i < Size; i++)
        EngineEnvironment::GetInstance()->GetDeviceObjectReflection()->Deserialize(Json[i], &pData[i]);
    *pDeviceObjects = pData;
}

template <typename Type>
inline void to_json_bitwise(nlohmann::json& Json, Type EnumBits)
{
    auto BitArray = nlohmann::json::array();
    for (Uint32 Bits = EnumBits; Bits != 0;)
    {
        const auto Bit = static_cast<const Type>(ExtractLSB(Bits));
        BitArray.push_back(Bit);
    }
    Json = BitArray.size() > 1 ? BitArray : EnumBits;
}

template <typename Type>
inline void from_json_bitwise(const nlohmann::json& Json, Type& EnumBits)
{
    auto ExtractBits = [](auto Array) -> Type {
        Type Bits = {};
        for (auto const& Bit : Array)
            Bits |= Bit.get<Type>();
        return Bits;
    };
    EnumBits = Json.is_array() ? ExtractBits(Json) : Json.get<Type>();
}

} // namespace Diligent
