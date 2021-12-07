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
inline void Serialize(nlohmann::json& Json, const ShaderMacro& Type, DynamicLinearAllocator& Allocator);

inline void Deserialize(const nlohmann::json& Json, ShaderMacro& Type, DynamicLinearAllocator& Allocator);

template <typename Type, std::enable_if_t<!std::is_pointer<Type>::value, bool> = true>
inline void Serialize(nlohmann::json& Json, const Type& Object, DynamicLinearAllocator& Allocator)
{
    Json = Object;
}

template <typename Type, std::enable_if_t<!std::is_pointer<Type>::value, bool> = true>
inline void Deserialize(const nlohmann::json& Json, Type& pObject, DynamicLinearAllocator& Allocator)
{
    Json.get_to(pObject);
}

template <typename Type>
inline void Serialize(nlohmann::json& Json, const Type* const pObject, DynamicLinearAllocator& Allocator)
{
    Serialize(Json, *pObject, Allocator);
}

template <typename Type>
inline void Deserialize(const nlohmann::json& Json, const Type*& pObject, DynamicLinearAllocator& Allocator)
{
    auto* pData = Allocator.Construct<Type>();
    Deserialize(Json, *pData, Allocator);
    pObject = pData;
}

template <typename Type>
inline void Deserialize(const nlohmann::json& Json, Type*& pObject, DynamicLinearAllocator& Allocator)
{
    auto* pData = Allocator.Construct<Type>();
    Deserialize(Json, *pData, Allocator);
    pObject = pData;
}

template <typename Type, typename TypeSize>
inline void Serialize(nlohmann::json& Json, const Type* const pData, TypeSize NumElements, DynamicLinearAllocator& Allocator)
{
    for (size_t i = 0; i < NumElements; i++)
    {
        auto Object = nlohmann::json::object();
        Serialize(Object, pData[i], Allocator);
        Json.push_back(Object);
    }
}

template <typename Type, typename TypeSize>
inline void Deserialize(const nlohmann::json& Json, const Type*& pObjects, TypeSize& NumElements, DynamicLinearAllocator& Allocator)
{
    auto* pData = Allocator.ConstructArray<Type>(Json.size());
    for (size_t i = 0; i < Json.size(); i++)
        Deserialize(Json[i], pData[i], Allocator);

    pObjects    = pData;
    NumElements = static_cast<TypeSize>(Json.size());
}

template <>
inline void Serialize(nlohmann::json& Json, const void* pData, size_t Size, DynamicLinearAllocator& Allocator)
{
    std::vector<uint8_t> PackedData(static_cast<const uint8_t*>(pData), static_cast<const uint8_t*>(pData) + Size);
    Json = nlohmann::json::binary(PackedData);
}

template <>
inline void Deserialize(const nlohmann::json& Json, const void*& pObject, size_t& Size, DynamicLinearAllocator& Allocator)
{
    auto* pData = Allocator.ConstructArray<Uint8>(Json.get_binary().size());
    std::memcpy(pData, Json.get_binary().data(), Json.get_binary().size());
    pObject = pData;
    Size    = Json.get_binary().size();
}

template <>
inline void Serialize(nlohmann::json& Json, const char* const Str, DynamicLinearAllocator& Allocator)
{
    if (Str != nullptr)
        Json = Str;
}

template <>
inline void Deserialize(const nlohmann::json& Json, const char*& Str, DynamicLinearAllocator& Allocator)
{
    Str = Allocator.CopyString(Json.get<std::string>());
}

template <>
inline void Serialize(nlohmann::json& Json, const ShaderMacro* const pMacros, DynamicLinearAllocator& Allocator)
{
    for (size_t i = 0; pMacros[i].Name != nullptr || pMacros[i].Definition != nullptr; i++)
    {
        auto Object = nlohmann::json::object();
        Serialize(Object, pMacros[i], Allocator);
        Json.push_back(Object);
    }
}

template <>
inline void Deserialize(const nlohmann::json& Json, const ShaderMacro*& pMacros, DynamicLinearAllocator& Allocator)
{
    auto* pData = Allocator.ConstructArray<ShaderMacro>(Json.size() + 1);
    for (size_t i = 0; i < Json.size(); i++)
        Deserialize(Json[i], pData[i], Allocator);

    pMacros = pData;
}

inline void Deserialize(const nlohmann::json& Json, const Char**& pObjects, Uint32& NumElements, DynamicLinearAllocator& Allocator)
{
    auto* pData = Allocator.ConstructArray<const char*>(Json.size());
    for (size_t i = 0; i < Json.size(); i++)
        pData[i] = Allocator.CopyString(Json[i].get<std::string>());

    pObjects    = pData;
    NumElements = static_cast<Uint32>(Json.size());
}

template <typename Type>
inline void SerializeBitwiseEnum(nlohmann::json& Json, Type EnumBits, DynamicLinearAllocator& Allocator)
{
    auto BitArray = nlohmann::json::array();
    for (Uint32 Bits = EnumBits; Bits != 0;)
        BitArray.push_back(static_cast<const Type>(ExtractLSB(Bits)));

    if (BitArray.size() > 1)
        Json = BitArray;
    else
        Json = EnumBits;
}

template <typename Type>
inline void DeserializeBitwiseEnum(const nlohmann::json& Json, Type& EnumBits, DynamicLinearAllocator& Allocator)
{
    auto ExtractBits = [](const nlohmann::json& Array) -> Type {
        Type Bits = {};
        for (auto const& Bit : Array)
            Bits |= static_cast<Type>(Bit.get<Type>());
        return Bits;
    };

    if (Json.is_array())
        EnumBits = ExtractBits(Json);
    else
        EnumBits = Json.get<Type>();
}

template <typename Type, size_t NumElements, std::enable_if_t<std::is_arithmetic<Type>::value, bool> = true>
inline void SerializeConstArray(nlohmann::json& Json, const Type (&pObjects)[NumElements], DynamicLinearAllocator& Allocator)
{
    for (size_t i = 0; i < NumElements; i++)
        Json.push_back(pObjects[i]);
}

template <typename Type, size_t NumElements, std::enable_if_t<!std::is_arithmetic<Type>::value, bool> = true>
inline void SerializeConstArray(nlohmann::json& Json, const Type (&pObjects)[NumElements], DynamicLinearAllocator& Allocator)
{
    for (size_t i = 0; i < NumElements; i++)
        if (!(pObjects[i] == Type{}))
            Serialize(Json[std::to_string(i)], pObjects[i], Allocator);
}

template <size_t NumElements>
inline void SerializeConstArray(nlohmann::json& Json, const char (&pData)[NumElements], DynamicLinearAllocator& Allocator)
{
     Json = std::string{pData};
}

template <typename Type, size_t NumElements, std::enable_if_t<std::is_arithmetic<Type>::value, bool> = true>
inline void DeserializeConstArray(const nlohmann::json& Json, Type (&pObjects)[NumElements], DynamicLinearAllocator& Allocator)
{
    for (size_t i = 0; i < NumElements; i++)
        pObjects[i] = Json[i].get<Type>();
}

template <typename Type, size_t NumElements, std::enable_if_t<!std::is_arithmetic<Type>::value, bool> = true>
inline void DeserializeConstArray(const nlohmann::json& Json, Type (&pObjects)[NumElements], DynamicLinearAllocator& Allocator)
{
    for (size_t i = 0; i < NumElements; i++)
        if (Json.contains(std::to_string(i)))
            Deserialize(Json[std::to_string(i)], pObjects[i], Allocator);
}

template <size_t NumElements>
inline void DeserializeConstArray(const nlohmann::json& Json, char (&pData)[NumElements], DynamicLinearAllocator& Allocator)
{
    auto Str = Json.get<std::string>();
    memcpy(pData, Str.c_str(), Str.size());
}

template <typename Type, size_t NumElements>
inline bool CompareConstArray(const Type (&Lhs)[NumElements], const Type (&Rhs)[NumElements])
{
    for (size_t i = 0; i < NumElements; i++)
        if (!(Lhs[i] == Rhs[i]))
            return false;
    return true;
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
{%- macro Serialize(type, fields, inheritance, fields_size, fields_size_inv) -%}
inline void Serialize(nlohmann::json& Json, const {{ type }}& Type, DynamicLinearAllocator& Allocator) 
{
{%- for base_type in inheritance %}
	Serialize(Json, static_cast<const {{base_type}}&>(Type), Allocator);
{% endfor -%}
{%- for field in fields %}
	{%- if field['meta'] == 'string' and field['name'] not in fields_size %}
	if (!SafeStrEqual(Type.{{ field['name'] }}, {{ type }}{}.{{ field['name'] }}))
	{% elif field['meta'] == 'const_array' %} 	
	if (!CompareConstArray(Type.{{ field['name'] }}, {{ type }}{}.{{ field['name'] }}))
    {% elif field['name'] in fields_size_inv -%}
	{% else %}
	if (!(Type.{{ field['name'] }} == {{ type }}{}.{{ field['name'] }}))
	{% endif -%}
    {%- if field['meta'] == 'const_array' -%}
		SerializeConstArray(Json["{{ field['name'] }}"], Type.{{ field['name'] }}, Allocator);
    {% elif field['meta'] == 'bitwise' -%}
        SerializeBitwiseEnum(Json["{{ field['name'] }}"], Type.{{ field['name'] }}, Allocator);	
	{% else %}
		{%- if field['name'] in fields_size -%}
			Serialize(Json["{{ field['name'] }}"], Type.{{ field['name'] }}, Type.{{ fields_size[field['name']] }}, Allocator);
        {% elif field['name'] in fields_size_inv -%}
		{% else -%}
			Serialize(Json["{{ field['name'] }}"], Type.{{ field['name'] }}, Allocator);
		{% endif -%}	
	{% endif -%}
{% endfor -%}
}
{% endmacro -%}

{%- macro Deserialize(type, fields, inheritance, fields_size, fields_size_inv) -%}
inline void Deserialize(const nlohmann::json& Json, {{ type }}& Type, DynamicLinearAllocator& Allocator)
{
{%- for base_type in inheritance %}
	Deserialize(Json, static_cast<{{base_type}}&>(Type), Allocator);
{% endfor -%}
{%- for field in fields %}
    {%- if field['name'] in fields_size_inv -%}
    {% else %}
    if (Json.contains("{{ field['name'] }}")) 
    {% endif -%}
	{%- if field['meta'] == 'const_array' -%}
		DeserializeConstArray(Json["{{ field['name'] }}"], Type.{{ field['name'] }}, Allocator);
    {% elif field['meta'] == 'bitwise' -%}
        DeserializeBitwiseEnum(Json["{{ field['name'] }}"], Type.{{ field['name'] }}, Allocator);
    {% elif field['name'] in fields_size_inv -%}
	{% else %}
		{%- if field['name'] in fields_size -%}
            Deserialize(Json["{{ field['name'] }}"], Type.{{ field['name'] }}, Type.{{ fields_size[field['name']] }}, Allocator);
        {% elif field['name'] in fields_size_inv -%}
        {% else -%}
            Deserialize(Json["{{ field['name'] }}"], Type.{{ field['name'] }}, Allocator);
        {% endif -%}
	{% endif -%}
{% endfor -%}
}
{%- endmacro -%}

{%- for type, info in structs %}
{{ Serialize(type, info['fields'], info['inheritance'], field_size[type], field_size_inv[type]) }}
{{ Deserialize(type, info['fields'], info['inheritance'], field_size[type], field_size_inv[type])}}
{% endfor %}
''')
