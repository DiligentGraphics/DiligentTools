/*     Copyright 2015-2017 Egor Yusov
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

#include "LuaWrappers.h"
#include "LuaBindings.h"
#include "EngineObjectParserBase.h"
#include "ClassMethodBinding.h"
#include "EngineObjectParserCommon.h"

namespace std
{
    DEFINE_ENUM_HASH( Diligent::FILTER_TYPE )
    DEFINE_ENUM_HASH( Diligent::TEXTURE_ADDRESS_MODE )
}

namespace Diligent
{
    template<typename StructType>
    void InitSamplerParserBindings(BindingsMapType &Bindings, 
                                   EnumMapping<FILTER_TYPE> &FilterTypeEnumMapping,
                                   EnumMapping<TEXTURE_ADDRESS_MODE> &TexAddrModeEnumMapping,
                                   ComparisonFuncEnumMapping &CmpFuncEnumMapping)
    {
        DEFINE_ENUM_ELEMENT_MAPPING( FilterTypeEnumMapping, FILTER_TYPE_POINT );
        DEFINE_ENUM_ELEMENT_MAPPING( FilterTypeEnumMapping, FILTER_TYPE_LINEAR );
        DEFINE_ENUM_ELEMENT_MAPPING( FilterTypeEnumMapping, FILTER_TYPE_ANISOTROPIC );
        DEFINE_ENUM_ELEMENT_MAPPING( FilterTypeEnumMapping, FILTER_TYPE_COMPARISON_POINT );
        DEFINE_ENUM_ELEMENT_MAPPING( FilterTypeEnumMapping, FILTER_TYPE_COMPARISON_LINEAR );
        DEFINE_ENUM_ELEMENT_MAPPING( FilterTypeEnumMapping, FILTER_TYPE_COMPARISON_ANISOTROPIC );
        DEFINE_ENUM_ELEMENT_MAPPING( FilterTypeEnumMapping, FILTER_TYPE_MINIMUM_POINT );
        DEFINE_ENUM_ELEMENT_MAPPING( FilterTypeEnumMapping, FILTER_TYPE_MINIMUM_LINEAR );
        DEFINE_ENUM_ELEMENT_MAPPING( FilterTypeEnumMapping, FILTER_TYPE_MINIMUM_ANISOTROPIC );
        DEFINE_ENUM_ELEMENT_MAPPING( FilterTypeEnumMapping, FILTER_TYPE_MAXIMUM_POINT );
        DEFINE_ENUM_ELEMENT_MAPPING( FilterTypeEnumMapping, FILTER_TYPE_MAXIMUM_LINEAR );
        DEFINE_ENUM_ELEMENT_MAPPING( FilterTypeEnumMapping, FILTER_TYPE_MAXIMUM_ANISOTROPIC );
        VERIFY( FilterTypeEnumMapping.m_Str2ValMap.size() == FILTER_TYPE_NUM_FILTERS - 1, "Unexpected map size. Did you update FILTER_TYPE enum?" );
        VERIFY( FilterTypeEnumMapping.m_Val2StrMap.size() == FILTER_TYPE_NUM_FILTERS - 1, "Unexpected map size. Did you update FILTER_TYPE enum?" );
        DEFINE_ENUM_BINDER( Bindings, StructType, MinFilter, FILTER_TYPE, FilterTypeEnumMapping )
        DEFINE_ENUM_BINDER( Bindings, StructType, MagFilter, FILTER_TYPE, FilterTypeEnumMapping )
        DEFINE_ENUM_BINDER( Bindings, StructType, MipFilter, FILTER_TYPE, FilterTypeEnumMapping )

        
        DEFINE_ENUM_ELEMENT_MAPPING( TexAddrModeEnumMapping, TEXTURE_ADDRESS_WRAP );
        DEFINE_ENUM_ELEMENT_MAPPING( TexAddrModeEnumMapping, TEXTURE_ADDRESS_MIRROR );
        DEFINE_ENUM_ELEMENT_MAPPING( TexAddrModeEnumMapping, TEXTURE_ADDRESS_CLAMP );
        DEFINE_ENUM_ELEMENT_MAPPING( TexAddrModeEnumMapping, TEXTURE_ADDRESS_BORDER );
        DEFINE_ENUM_ELEMENT_MAPPING( TexAddrModeEnumMapping, TEXTURE_ADDRESS_MIRROR_ONCE );
        VERIFY( TexAddrModeEnumMapping.m_Str2ValMap.size() == TEXTURE_ADDRESS_NUM_MODES - 1, "Unexpected map size. Did you update TEXTURE_ADDRESS_MODE enum?" );
        VERIFY( TexAddrModeEnumMapping.m_Val2StrMap.size() == TEXTURE_ADDRESS_NUM_MODES - 1, "Unexpected map size. Did you update TEXTURE_ADDRESS_MODE enum?" );
        DEFINE_ENUM_BINDER( Bindings, StructType, AddressU, TEXTURE_ADDRESS_MODE, TexAddrModeEnumMapping )
        DEFINE_ENUM_BINDER( Bindings, StructType, AddressV, TEXTURE_ADDRESS_MODE, TexAddrModeEnumMapping )
        DEFINE_ENUM_BINDER( Bindings, StructType, AddressW, TEXTURE_ADDRESS_MODE, TexAddrModeEnumMapping )


        Validator<Float32> DummyValidatorF( SkipValidationFunc<float> );
        DEFINE_BINDER( Bindings, StructType, MipLODBias, Float32, DummyValidatorF )

        Validator<Uint32> MaxAnisotropyValidator( "Max Anisotropy", 0, 32 );
        DEFINE_BINDER( Bindings, StructType, MaxAnisotropy, Uint32, MaxAnisotropyValidator )

        DEFINE_ENUM_BINDER( Bindings, StructType, ComparisonFunc, COMPARISON_FUNCTION, CmpFuncEnumMapping )

        DEFINE_BINDER( Bindings, StructType, BorderColor, RGBALoader, 0 )

        DEFINE_BINDER( Bindings, StructType, MinLOD, Float32, DummyValidatorF )
        DEFINE_BINDER( Bindings, StructType, MaxLOD, Float32, DummyValidatorF )
    }

    class SamplerParser : public EngineObjectParserCommon<ISampler>
    {
    public:
        SamplerParser( IRenderDevice *pRenderDevice, lua_State *L );
        static const Char* SamplerLibName;

    protected:
        virtual void CreateObj( lua_State *L );

    private:
        // SamplerDesc structure does not provide storage for the Name field.
        // We need to use ObjectDescWrapper<> to be able to store the field.
        typedef ObjectDescWrapper<SamplerDesc> SSamDescWrapper;

        EnumMapping<FILTER_TYPE>           m_FilterTypeEnumMapping;
        EnumMapping<TEXTURE_ADDRESS_MODE>  m_TexAddrModeEnumMapping;
        ComparisonFuncEnumMapping          m_CmpFuncEnumMapping;
    };
}
