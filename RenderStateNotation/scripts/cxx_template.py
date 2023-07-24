# ----------------------------------------------------------------------------
# Copyright 2019-2022 Diligent Graphics LLC
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
// https://json.nlohmann.me/home/exceptions/#jsonexceptiontype_error302
constexpr Uint32 JsonTypeError = 302;

// https://json.nlohmann.me/home/exceptions/#jsonexceptionother_error501
constexpr Uint32 JsonInvalidEnum = 501;

// https://json.nlohmann.me/home/exceptions/#jsonexceptionother_error501
constexpr Uint32 JsonUnexpectedKey = 501;

#define NLOHMANN_JSON_SERIALIZE_ENUM_EX(ENUM_TYPE, ...)                                                                                                                                                      \
    inline void to_json(nlohmann::json& j, const ENUM_TYPE& e)                                                                                                                                               \
    {                                                                                                                                                                                                        \
        static_assert(std::is_enum<ENUM_TYPE>::value, #ENUM_TYPE " must be an enum!");                                                                                                                       \
        static const std::pair<ENUM_TYPE, nlohmann::json> m[] = __VA_ARGS__;                                                                                                                                 \
        auto                                              it  = std::find_if(std::begin(m), std::end(m), [e](const std::pair<ENUM_TYPE, nlohmann::json>& ej_pair) -> bool { return ej_pair.first == e; });   \
        if (it == std::end(m)) throw nlohmann::json::other_error::create(JsonInvalidEnum, std::string("invalid enum value for " #ENUM_TYPE ""), j);                                                          \
        j = it->second;                                                                                                                                                                                      \
    }                                                                                                                                                                                                        \
    inline void from_json(const nlohmann::json& j, ENUM_TYPE& e)                                                                                                                                             \
    {                                                                                                                                                                                                        \
        static_assert(std::is_enum<ENUM_TYPE>::value, #ENUM_TYPE " must be an enum!");                                                                                                                       \
        static const std::pair<ENUM_TYPE, nlohmann::json> m[] = __VA_ARGS__;                                                                                                                                 \
        auto                                              it  = std::find_if(std::begin(m), std::end(m), [&j](const std::pair<ENUM_TYPE, nlohmann::json>& ej_pair) -> bool { return ej_pair.second == j; }); \
        if (it == std::end(m)) throw nlohmann::json::other_error::create(JsonInvalidEnum, std::string("invalid enum value for " #ENUM_TYPE ": ") + j.get<std::string>(), j);                                 \
        e = it->first;                                                                                                                                                                                       \
    }

#define NLOHMANN_JSON_VALIDATE_KEYS(JSON, ...)                                                                                                          \
    do                                                                                                                                                  \
    {                                                                                                                                                   \
        static const char* m[] = __VA_ARGS__;                                                                                                           \
        for (auto it = JSON.begin(); it != JSON.end(); ++it)                                                                                            \
        {                                                                                                                                               \
            auto result = std::find_if(std::begin(m), std::end(m), [&it](const char* key) -> bool { return std::strcmp(it.key().c_str(), key) == 0; }); \
            if (result == std::end(m)) throw nlohmann::json::other_error::create(JsonUnexpectedKey, std::string("unexpected key: ") + it.key(), JSON);  \
        }                                                                                                                                               \
    } while (false)

void WriteRSN(nlohmann::json& Json, const ShaderMacro& Type, DynamicLinearAllocator& Allocator);

void ParseRSN(const nlohmann::json& Json, ShaderMacro& Type, DynamicLinearAllocator& Allocator);

template <typename Type, std::enable_if_t<!std::is_pointer<Type>::value, bool> = true>
inline void WriteRSN(nlohmann::json& Json, const Type& Object, DynamicLinearAllocator& Allocator)
{
    Json = Object;
}

template <typename Type, std::enable_if_t<!std::is_pointer<Type>::value, bool> = true>
inline void ParseRSN(const nlohmann::json& Json, Type& pObject, DynamicLinearAllocator& Allocator)
{
    Json.get_to(pObject);
}

template <typename Type>
inline void WriteRSN(nlohmann::json& Json, const Type* const pObject, DynamicLinearAllocator& Allocator)
{
    WriteRSN(Json, *pObject, Allocator);
}

template <typename Type>
inline void ParseRSN(const nlohmann::json& Json, const Type*& pObject, DynamicLinearAllocator& Allocator)
{
    auto* pData = Allocator.Construct<Type>();
    ParseRSN(Json, *pData, Allocator);
    pObject = pData;
}

template <typename Type>
inline void ParseRSN(const nlohmann::json& Json, Type*& pObject, DynamicLinearAllocator& Allocator)
{
    auto* pData = Allocator.Construct<Type>();
    ParseRSN(Json, *pData, Allocator);
    pObject = pData;
}

template <typename Type, typename TypeSize>
inline void WriteRSN(nlohmann::json& Json, const Type* const pData, TypeSize NumElements, DynamicLinearAllocator& Allocator)
{
    for (size_t i = 0; i < NumElements; i++)
    {
        auto Object = nlohmann::json::object();
        WriteRSN(Object, pData[i], Allocator);
        Json.push_back(Object);
    }
}

template <typename Type, typename TypeSize>
inline void ParseRSN(const nlohmann::json& Json, const Type*& pObjects, TypeSize& NumElements, DynamicLinearAllocator& Allocator)
{
    if (!Json.is_array())
        throw nlohmann::json::type_error::create(JsonTypeError, std::string("type must be array, but is ") + Json.type_name(), Json);

    auto* pData = Allocator.ConstructArray<Type>(Json.size());
    for (size_t i = 0; i < Json.size(); i++)
        ParseRSN(Json[i], pData[i], Allocator);

    pObjects    = pData;
    NumElements = static_cast<TypeSize>(Json.size());
}

template <>
inline void WriteRSN(nlohmann::json& Json, const void* pData, size_t Size, DynamicLinearAllocator& Allocator)
{
    std::vector<uint8_t> PackedData(static_cast<const uint8_t*>(pData), static_cast<const uint8_t*>(pData) + Size);
    Json = nlohmann::json::binary(PackedData);
}

template <>
inline void ParseRSN(const nlohmann::json& Json, const void*& pObject, size_t& Size, DynamicLinearAllocator& Allocator)
{
    auto* pData = Allocator.ConstructArray<Uint8>(Json.get_binary().size());
    std::memcpy(pData, Json.get_binary().data(), Json.get_binary().size());
    pObject = pData;
    Size    = Json.get_binary().size();
}

template <>
inline void WriteRSN(nlohmann::json& Json, const char* const Str, DynamicLinearAllocator& Allocator)
{
    if (Str != nullptr)
        Json = Str;
}

template <>
inline void ParseRSN(const nlohmann::json& Json, const char*& Str, DynamicLinearAllocator& Allocator)
{
    Str = Allocator.CopyString(Json.get<std::string>());
}

inline void WriteRSN(nlohmann::json& Json, const ShaderMacroArray& Macros, DynamicLinearAllocator& Allocator)
{
    for (size_t i = 0; Macros.Count; i++)
    {
        auto Object = nlohmann::json::object();
        WriteRSN(Object, Macros.Elements[i], Allocator);
        Json.push_back(Object);
    }
}

inline void ParseRSN(const nlohmann::json& Json, ShaderMacroArray& Macros, DynamicLinearAllocator& Allocator)
{
    if (!Json.is_array())
        throw nlohmann::json::type_error::create(JsonTypeError, std::string("type must be array, but is ") + Json.type_name(), Json);

    auto* pData = Allocator.ConstructArray<ShaderMacro>(Json.size());
    for (size_t i = 0; i < Json.size(); i++)
        ParseRSN(Json[i], pData[i], Allocator);

    Macros.Elements = pData;
    Macros.Count    = static_cast<Uint32>(Json.size());
}

inline void ParseRSN(const nlohmann::json& Json, const Char**& pObjects, Uint32& NumElements, DynamicLinearAllocator& Allocator)
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
    {
        EnumBits = ExtractBits(Json);
    }
    else if(Json.is_string())
    {
        EnumBits = Json.get<Type>();
    }
    else
    {
        throw nlohmann::json::type_error::create(JsonTypeError, std::string("type must be array or string, but is ") + Json.type_name(), Json);  
    }
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
            WriteRSN(Json[std::to_string(i)], pObjects[i], Allocator);
}

template <size_t NumElements>
inline void SerializeConstArray(nlohmann::json& Json, const char (&pData)[NumElements], DynamicLinearAllocator& Allocator)
{
    Json = std::string{pData};
}

template <typename Type, size_t NumElements, std::enable_if_t<std::is_arithmetic<Type>::value, bool> = true>
inline void DeserializeConstArray(const nlohmann::json& Json, Type (&pObjects)[NumElements], DynamicLinearAllocator& Allocator)
{
    if (!Json.is_array())
        throw nlohmann::json::type_error::create(JsonTypeError, std::string("type must be array, but is ") + Json.type_name(), Json);

    for (size_t i = 0; i < NumElements; i++)
        pObjects[i] = Json[i].get<Type>();
}

template <typename Type, size_t NumElements, std::enable_if_t<!std::is_arithmetic<Type>::value, bool> = true>
inline void DeserializeConstArray(const nlohmann::json& Json, Type (&pObjects)[NumElements], DynamicLinearAllocator& Allocator)
{
    if (!Json.is_object())
        throw nlohmann::json::type_error::create(JsonTypeError, std::string("type must be object, but is ") + Json.type_name(), Json);

    for (size_t i = 0; i < NumElements; i++)
        if (Json.contains(std::to_string(i)))
            ParseRSN(Json[std::to_string(i)], pObjects[i], Allocator);
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
NLOHMANN_JSON_SERIALIZE_ENUM_EX(
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
{%- macro WriteRSN(type, fields, inheritance, fields_size, fields_size_inv) -%}
inline void WriteRSN(nlohmann::json& Json, const {{ type }}& Type, DynamicLinearAllocator& Allocator) 
{
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
			WriteRSN(Json["{{ field['name'] }}"], Type.{{ field['name'] }}, Type.{{ fields_size[field['name']] }}, Allocator);
        {% elif field['name'] in fields_size_inv -%}
		{% else -%}
			WriteRSN(Json["{{ field['name'] }}"], Type.{{ field['name'] }}, Allocator);
		{% endif -%}	
	{% endif -%}
{% endfor -%}
}
{% endmacro -%}

{%- macro ParseRSN(type, fields, inheritance, fields_size, fields_size_inv) -%}
inline void ParseRSN(const nlohmann::json& Json, {{ type }}& Type, DynamicLinearAllocator& Allocator)
{
NLOHMANN_JSON_VALIDATE_KEYS(Json, {
{%- for field in fields %}
    "{{ field['name'] }}",
{%- endfor %}
{%- if type == "GraphicsPipelineDesc" -%}
    "pRenderPass",
{%- endif -%}
});
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
            ParseRSN(Json["{{ field['name'] }}"], Type.{{ field['name'] }}, Type.{{ fields_size[field['name']] }}, Allocator);
        {% elif field['name'] in fields_size_inv -%}
        {% else -%}
            ParseRSN(Json["{{ field['name'] }}"], Type.{{ field['name'] }}, Allocator);
        {% endif -%}
	{% endif -%}
{% endfor -%}
}
{%- endmacro -%}

{%- for type, info in structs %}
{{ WriteRSN(type, info['fields'], info['inheritance'], field_size[type], field_size_inv[type]) }}
{{ ParseRSN(type, info['fields'], info['inheritance'], field_size[type], field_size_inv[type])}}
{% endfor %}
''')
