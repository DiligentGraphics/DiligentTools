/*     Copyright 2015-2018 Egor Yusov
 *  
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 * 
 *     http://www.apache.org/licenses/LICENSE-2.0
 * 
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT OF ANY PROPRIETARY RIGHTS.
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

#include "HashUtils.h"

#define DEFINE_ENUM_HASH(Type)\
template<>struct hash<Type>                     \
{                                               \
    size_t operator()( const Type &x ) const    \
    {                                           \
        return hash<int>()(x);                  \
    }                                           \
};

namespace std
{
    DEFINE_ENUM_HASH( Diligent::RESOURCE_DIMENSION )
    DEFINE_ENUM_HASH( Diligent::TEXTURE_FORMAT )
    DEFINE_ENUM_HASH( Diligent::VALUE_TYPE )
    DEFINE_ENUM_HASH( Diligent::USAGE )
    // Explicit namespace declaraion is necesseary to avoid 
    // name conflicts when building for windows store
    DEFINE_ENUM_HASH( Diligent::BIND_FLAGS )
    DEFINE_ENUM_HASH( Diligent::CPU_ACCESS_FLAGS )
    DEFINE_ENUM_HASH( Diligent::COMPARISON_FUNCTION )
    DEFINE_ENUM_HASH( Diligent::BIND_SHADER_RESOURCES_FLAGS )
    DEFINE_ENUM_HASH( Diligent::SHADER_TYPE )
    DEFINE_ENUM_HASH( Diligent::RESOURCE_STATE_TRANSITION_MODE )
}

namespace Diligent
{
    template<typename EnumType>
    struct EnumMapping
    {
        void AddMapping( const Char *Str, EnumType Val )
        {
            m_Str2ValMap.insert( std::make_pair( Diligent::HashMapStringKey(Str, true), Val ) );
            m_Val2StrMap.insert( std::make_pair( Val, Str ) );
        }

        std::unordered_map< Diligent::HashMapStringKey, EnumType> m_Str2ValMap;
        std::unordered_map<EnumType, String> m_Val2StrMap;
    };
#define DEFINE_ENUM_ELEMENT_MAPPING(EnumMapping, Elem) EnumMapping.AddMapping(#Elem, Elem)

    class CpuAccessFlagEnumMapping : public EnumMapping < Diligent::CPU_ACCESS_FLAGS >
    {
    public:
        CpuAccessFlagEnumMapping();
    };
    
    class UsageEnumMapping : public EnumMapping < Diligent::USAGE >
    {
    public:
        UsageEnumMapping();
    };

    class TextureFormatEnumMapping : public EnumMapping < Diligent::TEXTURE_FORMAT >
    {
    public:
        TextureFormatEnumMapping();
    };

    class ResourceDimEnumMapping : public EnumMapping < Diligent::RESOURCE_DIMENSION >
    {
    public:
        ResourceDimEnumMapping();
    };

    class ValueTypeEnumMapping : public EnumMapping < Diligent::VALUE_TYPE >
    {
    public:
        ValueTypeEnumMapping();
    };

    class ComparisonFuncEnumMapping : public EnumMapping < Diligent::COMPARISON_FUNCTION >
    {
    public:
        ComparisonFuncEnumMapping();
    };

    class BindShaderResourcesFlagEnumMapping : public EnumMapping < Diligent::BIND_SHADER_RESOURCES_FLAGS >
    {
    public:
        BindShaderResourcesFlagEnumMapping();
    };

    class ShaderTypeEnumMapping : public EnumMapping<SHADER_TYPE>
    {
    public:
        ShaderTypeEnumMapping();
    };

    class StateTransitionModeEnumMapping : public EnumMapping<RESOURCE_STATE_TRANSITION_MODE>
    {
    public:
        StateTransitionModeEnumMapping();
    };
}
