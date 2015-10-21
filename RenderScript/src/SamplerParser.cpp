/*     Copyright 2015 Egor Yusov
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

#include "pch.h"
#include "SamplerParser.h"

namespace Diligent
{
    const Char* SamplerParser::SamplerLibName = "Sampler";

    SamplerParser::SamplerParser( IRenderDevice *pRenderDevice, lua_State *L ) :
        EngineObjectParserCommon<ISampler>( pRenderDevice, L, SamplerLibName )
    {
        DEFINE_BUFFERED_STRING_BINDER( m_Bindings, SSamDescWrapper, Name, NameBuffer )

        DEFINE_ENUM_ELEMENT_MAPPING( m_FilterTypeEnumMapping, FILTER_TYPE_POINT );
        DEFINE_ENUM_ELEMENT_MAPPING( m_FilterTypeEnumMapping, FILTER_TYPE_LINEAR );
        DEFINE_ENUM_ELEMENT_MAPPING( m_FilterTypeEnumMapping, FILTER_TYPE_ANISOTROPIC );
        DEFINE_ENUM_ELEMENT_MAPPING( m_FilterTypeEnumMapping, FILTER_TYPE_COMPARISON_POINT );
        DEFINE_ENUM_ELEMENT_MAPPING( m_FilterTypeEnumMapping, FILTER_TYPE_COMPARISON_LINEAR );
        DEFINE_ENUM_ELEMENT_MAPPING( m_FilterTypeEnumMapping, FILTER_TYPE_COMPARISON_ANISOTROPIC );
        VERIFY( m_FilterTypeEnumMapping.m_Str2ValMap.size() == FILTER_TYPE_NUM_FILTERS - 1, "Unexpected map size. Did you update FILTER_TYPE enum?" );
        VERIFY( m_FilterTypeEnumMapping.m_Val2StrMap.size() == FILTER_TYPE_NUM_FILTERS - 1, "Unexpected map size. Did you update FILTER_TYPE enum?" );
        DEFINE_ENUM_BINDER( m_Bindings, SSamDescWrapper, MinFilter, FILTER_TYPE, m_FilterTypeEnumMapping )
        DEFINE_ENUM_BINDER( m_Bindings, SSamDescWrapper, MagFilter, FILTER_TYPE, m_FilterTypeEnumMapping )
        DEFINE_ENUM_BINDER( m_Bindings, SSamDescWrapper, MipFilter, FILTER_TYPE, m_FilterTypeEnumMapping )

        
        DEFINE_ENUM_ELEMENT_MAPPING( m_TexAddrModeEnumMapping, TEXTURE_ADDRESS_WRAP );
        DEFINE_ENUM_ELEMENT_MAPPING( m_TexAddrModeEnumMapping, TEXTURE_ADDRESS_MIRROR );
        DEFINE_ENUM_ELEMENT_MAPPING( m_TexAddrModeEnumMapping, TEXTURE_ADDRESS_CLAMP );
        DEFINE_ENUM_ELEMENT_MAPPING( m_TexAddrModeEnumMapping, TEXTURE_ADDRESS_BORDER );
        DEFINE_ENUM_ELEMENT_MAPPING( m_TexAddrModeEnumMapping, TEXTURE_ADDRESS_MIRROR_ONCE );
        VERIFY( m_TexAddrModeEnumMapping.m_Str2ValMap.size() == TEXTURE_ADDRESS_NUM_MODES - 1, "Unexpected map size. Did you update TEXTURE_ADDRESS_MODE enum?" );
        VERIFY( m_TexAddrModeEnumMapping.m_Val2StrMap.size() == TEXTURE_ADDRESS_NUM_MODES - 1, "Unexpected map size. Did you update TEXTURE_ADDRESS_MODE enum?" );
        DEFINE_ENUM_BINDER( m_Bindings, SSamDescWrapper, AddressU, TEXTURE_ADDRESS_MODE, m_TexAddrModeEnumMapping )
        DEFINE_ENUM_BINDER( m_Bindings, SSamDescWrapper, AddressV, TEXTURE_ADDRESS_MODE, m_TexAddrModeEnumMapping )
        DEFINE_ENUM_BINDER( m_Bindings, SSamDescWrapper, AddressW, TEXTURE_ADDRESS_MODE, m_TexAddrModeEnumMapping )


        Validator<Float32> DummyValidatorF( SkipValidationFunc<float> );
        DEFINE_BINDER( m_Bindings, SSamDescWrapper, MipLODBias, Float32, DummyValidatorF )

        Validator<Uint32> MaxAnisotropyValidator( "Max Anisotropy", 0, 32 );
        DEFINE_BINDER( m_Bindings, SSamDescWrapper, MaxAnisotropy, Uint32, MaxAnisotropyValidator )

        DEFINE_ENUM_BINDER( m_Bindings, SSamDescWrapper, ComparisonFunc, COMPARISON_FUNCTION, m_CmpFuncEnumMapping )

        DEFINE_BINDER( m_Bindings, SSamDescWrapper, BorderColor, RGBALoader, 0 )

        DEFINE_BINDER( m_Bindings, SSamDescWrapper, MinLOD, Float32, DummyValidatorF )
        DEFINE_BINDER( m_Bindings, SSamDescWrapper, MaxLOD, Float32, DummyValidatorF )
    };

    void SamplerParser::CreateObj( lua_State *L )
    {
        INIT_LUA_STACK_TRACKING( L );

        SSamDescWrapper SamplerDesc;
        ParseLuaTable( L, -1, &SamplerDesc, m_Bindings );

        CHECK_LUA_STACK_HEIGHT();

        auto ppSampler = reinterpret_cast<ISampler**>(lua_newuserdata( L, sizeof( ISampler* ) ));
        *ppSampler = nullptr;
        m_pRenderDevice->CreateSampler( SamplerDesc, ppSampler );
        if( *ppSampler == nullptr )
            SCRIPT_PARSING_ERROR(L, "Failed to create a sampler")

        CHECK_LUA_STACK_HEIGHT( +1 );
    }
}
