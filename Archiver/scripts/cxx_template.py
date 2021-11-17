# ----------------------------------------------------------------------------
# Copyright 2019-2021 Diligent Graphics LLC
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
# In no event and under no legal theory, whether in tort (including negligence),
# contract, or otherwise, unless required by applicable law (such as deliberate
# and grossly negligent acts) or agreed to in writing, shall any Contributor be
# liable for any damages, including any direct, indirect, special, incidental,
# or consequential damages of any character arising as a result of this License or
# out of the use or inability to use the software (including but not limited to damages
# for loss of goodwill, work stoppage, computer failure or malfunction, or any and
# all other commercial damages or losses), even if such Contributor has been advised
# of the possibility of such damages.
# ----------------------------------------------------------------------------

from jinja2 import Template

CXX_COMMON_SERIALIZE_TEMPLATE = Template('''
template <typename T>
inline T** RemoveConst(const T** x)
{
    return const_cast<T**>(x);
}

inline const char* CopyString(const std::string& Str, DeviceObjectReflection* pAllocator)
{
    return pAllocator->CopyString(Str);
}

template <typename Type>
inline void Serialize(nlohmann::json& Json, const Type& Object, DeviceObjectReflection* pAllocator)
{
    Json = Object;
}

template <typename Type>
inline void Deserialize(const nlohmann::json& Json, Type& pObject, DeviceObjectReflection* pAllocator)
{
    Json.get_to(pObject);
}

template <typename Type>
inline void SerializePtr(nlohmann::json& Json, const Type* pData, size_t Size, DeviceObjectReflection* pAllocator)
{
    for (size_t i = 0; i < Size; i++)
    {
        auto Object = nlohmann::json::object();
        Serialize(Object, pData[i], pAllocator);
        Json.push_back(Object);
    }
}

template <typename Type>
inline void DeserializePtr(const nlohmann::json& Json, Type** pObjects, size_t Size, DeviceObjectReflection* pAllocator)
{
    auto* pData = pAllocator->Allocate<Type>(Size);
    for (size_t i = 0; i < Size; i++)
        Deserialize(Json[i], pData[i], pAllocator);
    *pObjects = pData;
}

template <typename Type>
inline void SerializePtr(nlohmann::json& Json, const Type* pObject, DeviceObjectReflection* pAllocator)
{
    Serialize(Json, *pObject, pAllocator);
}

template <typename Type>
inline void DeserializePtr(const nlohmann::json& Json, Type** pObject, DeviceObjectReflection* pAllocator)
{
    auto* pData = pAllocator->Allocate<Type>();
    Deserialize(Json, *pData, pAllocator);
    *pObject = pData;
}

template <>
inline void SerializePtr(nlohmann::json& Json, const void* pData, size_t Size, DeviceObjectReflection* pAllocator)
{
    std::vector<uint8_t> PackedData(static_cast<const uint8_t*>(pData), static_cast<const uint8_t*>(pData) + Size);
    Json = nlohmann::json::binary(PackedData);
}

template <>
inline void DeserializePtr(const nlohmann::json& Json, void** pObject, size_t Size, DeviceObjectReflection* pAllocator)
{
    auto* pData = pAllocator->Allocate<Uint8>(Size);
    std::memcpy(pData, Json.get_binary().data(), Size);
    *pObject = pData;
}

template <typename Type>
inline void SerializeInterface(nlohmann::json& Json, const Type* pDeviceObject, DeviceObjectReflection* pAllocator)
{
    pAllocator->Serialize(Json, pDeviceObject);
}

template <typename Type>
inline void DeserializeInterface(const nlohmann::json& Json, Type** pDeviceObject, DeviceObjectReflection* pAllocator)
{
    pAllocator->Deserialize(Json, pDeviceObject);
}

template <typename Type>
inline void SerializeInterface(nlohmann::json& Json, Type** pDeviceObjects, size_t Size, DeviceObjectReflection* pAllocator)
{
    for (size_t i = 0; i < Size; i++)
    {
        auto Object = nlohmann::json::object();
        pAllocator->Serialize(Object, pDeviceObjects[i]);
        Json.push_back(Object);
    }
}

template <typename Type>
inline void DeserializeInterface(const nlohmann::json& Json, Type*** pDeviceObjects, size_t Size, DeviceObjectReflection* pAllocator)
{
    auto* pData = pAllocator->Allocate<Type*>(Size);
    for (size_t i = 0; i < Size; i++)
        pAllocator->Deserialize(Json[i], &pData[i]);
    *pDeviceObjects = pData;
}

template <typename Type>
inline void SerializeBitwiseEnum(nlohmann::json& Json, Type EnumBits, DeviceObjectReflection* pAllocator)
{
    auto BitArray = nlohmann::json::array();
    for (Uint32 Bits = EnumBits; Bits != 0;)
        BitArray.push_back(static_cast<const Type>(ExtractLSB(Bits)));

    Json = BitArray.size() > 1 ? BitArray : EnumBits;
}

template <typename Type>
inline void DeserializeBitwiseEnum(const nlohmann::json& Json, Type& EnumBits, DeviceObjectReflection* pAllocator)
{
    auto ExtractBits = [](auto Array) -> Type {
        Type Bits = {};
        for (auto const& Bit : Array)
            Bits |= Bit.get<Type>();
        return Bits;
    };
    EnumBits = Json.is_array() ? ExtractBits(Json) : Json.get<Type>();
}

template <typename Type>
inline void SerializeConstArray(nlohmann::json& Json, const Type* pObjects, size_t Size, DeviceObjectReflection* pAllocator)
{
    for (size_t i = 0; i < Size; i++)
        if (!(pObjects[i] == Type{}))
            Serialize(Json[std::to_string(i)], pObjects[i], pAllocator);
}

template <typename Type>
inline void DeserializeConstArray(const nlohmann::json& Json, Type* pObjects, size_t Size, DeviceObjectReflection* pAllocator)
{
    for (size_t i = 0; i < Size; i++)
        if (Json.contains(std::to_string(i)))
            Deserialize(Json[std::to_string(i)], pObjects[i], pAllocator);
}

''')

CXX_ENUM_SERIALIZE_TEMPLATE = Template(''' 
{%- macro serialize_enum(type, xitems) -%}
NLOHMANN_JSON_SERIALIZE_ENUM(
{{type}}, {
{%- for item in xitems %}
{ {{ item['value'] }}, "{{ item['name'] }}" },
{%- endfor %}
})
{% endmacro -%}
{%- for type, xitems in enums %}
{{serialize_enum(type, xitems)}}
{%- endfor -%}
''')

CXX_STRUCT_SERIALIZE_TEMPLATE = Template('''
{%- macro Serialize(type, fields, inheritance, fields_size) -%}
inline void Serialize(nlohmann::json& Json, const {{ type }}& Type, DeviceObjectReflection* pAllocator) 
{
{%- for base_type in inheritance %}
	Serialize(Json, static_cast<{{base_type}}>(Type), pAllocator);
{% endfor -%}
{%- for field in fields %}
	{%- if field['meta'] == 'string' %}
	if (!CompareStr(Type.{{ field['name'] }}, {{ type }}{}.{{ field['name'] }}))
	{%- elif field['meta'] == 'const_array' %} 	
	if (!CompareConstArray(Type.{{ field['name'] }}, {{ type }}{}.{{ field['name'] }}, _countof(Type.{{ field['name'] }})))
	{%- else %}
	if (!(Type.{{ field['name'] }} == {{ type }}{}.{{ field['name'] }}))
	{%- endif -%}
    {%- if field['meta'] == 'string' %}
		Json["{{ field['name'] }}"] = Type.{{ field['name'] }};
	{% elif field['meta'] == 'const_array' %}
		SerializeConstArray(Json["{{ field['name'] }}"], Type.{{ field['name'] }}, _countof(Type.{{ field['name'] }}), pAllocator);
    {% elif field['meta'] == 'bitwise' %}
        SerializeBitwiseEnum(Json["{{ field['name'] }}"], Type.{{ field['name'] }}, pAllocator);
	{% elif field['meta'] == 'interface' %}
		{%- if field['name'] in fields_size %}
			SerializeInterface(Json["{{ field['name'] }}"], Type.{{ field['name'] }}, Type.{{ fields_size[field['name']] }}, pAllocator);
		{% else %}
			SerializeInterface(Json["{{ field['name'] }}"], Type.{{ field['name'] }}, pAllocator);
		{% endif -%}	
	{% elif field['meta'] == 'pointer' %}
		{%- if field['name'] in fields_size %}
			SerializePtr(Json["{{ field['name'] }}"], Type.{{ field['name'] }}, Type.{{ fields_size[field['name']] }}, pAllocator);
		{% else %}
			SerializePtr(Json["{{ field['name'] }}"], Type.{{ field['name'] }}, pAllocator);
		{% endif -%}	
	{% else %}
		Serialize(Json["{{ field['name'] }}"], Type.{{ field['name'] }}, pAllocator);
	{% endif -%}
{% endfor -%}
}
{% endmacro -%}

{%- macro Deserialize(type, fields, inheritance, fields_size) -%}
inline void Deserialize(const nlohmann::json& Json, {{ type }}& Type, DeviceObjectReflection* pAllocator)
{
{%- for base_type in inheritance %}
	Deserialize(Json, static_cast<{{base_type}}&>(Type), pAllocator);
{% endfor -%}
{%- for field in fields %}
	if (Json.contains("{{ field['name'] }}")) 
	{%- if field['meta'] == 'string' %}
		Type.{{ field['name'] }} = CopyString(Json["{{ field['name']}}"].get<std::string>(), pAllocator);
	{% elif field['meta'] == 'const_array' %}
		DeserializeConstArray(Json["{{ field['name'] }}"], Type.{{ field['name'] }}, _countof(Type.{{ field['name'] }}), pAllocator);
    {% elif field['meta'] == 'bitwise' %}
        DeserializeBitwiseEnum(Json["{{ field['name'] }}"], Type.{{ field['name'] }}, pAllocator);
	{% elif field['meta'] == 'interface' %}
		{%- if field['name'] in fields_size %}
			DeserializeInterface(Json["{{ field['name'] }}"], &Type.{{ field['name'] }}, Json.at("{{ fields_size[field['name']] }}"), pAllocator);
		{% else %}
			DeserializeInterface(Json["{{ field['name'] }}"], &Type.{{ field['name'] }}, pAllocator);
		{% endif -%}
	{% elif field['meta'] == 'pointer' %}
		{%- if field['name'] in fields_size %}
			DeserializePtr(Json["{{ field['name'] }}"], RemoveConst(&Type.{{ field['name'] }}), Json.at("{{ fields_size[field['name']] }}"), pAllocator);
		{% else %}
			DeserializePtr(Json["{{ field['name'] }}"], RemoveConst(&Type.{{ field['name'] }}), pAllocator);
		{% endif -%}
	{% else %}
		Deserialize(Json["{{ field['name'] }}"], Type.{{ field['name'] }}, pAllocator);
	{% endif -%}
{% endfor -%}
}
{%- endmacro -%}

{%- for type, info in structs %}
{{ Serialize(type, info['fields'], info['inheritance'], field_size[type]) }}
{{ Deserialize(type, info['fields'], info['inheritance'], field_size[type])}}
{% endfor %}
''')
