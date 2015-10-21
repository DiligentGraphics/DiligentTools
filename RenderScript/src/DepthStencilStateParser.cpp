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
#include "DepthStencilStateParser.h"

namespace std 
{
    DEFINE_ENUM_HASH( Diligent::STENCIL_OP )
}

namespace Diligent
{
    const Char* DepthStencilStateParser::DepthStencilStateLibName = "DepthStencilState";

    template<>
    class MemberBinder<StencilOpDesc> : public MemberBinderBase
    {
    public:
        MemberBinder( size_t MemberOffset, size_t Dummy ) :
            MemberBinderBase( MemberOffset )
        {
            DEFINE_ENUM_ELEMENT_MAPPING( m_StencilOpEnumMapping, STENCIL_OP_KEEP );
            DEFINE_ENUM_ELEMENT_MAPPING( m_StencilOpEnumMapping, STENCIL_OP_ZERO );
            DEFINE_ENUM_ELEMENT_MAPPING( m_StencilOpEnumMapping, STENCIL_OP_REPLACE );
            DEFINE_ENUM_ELEMENT_MAPPING( m_StencilOpEnumMapping, STENCIL_OP_INCR_SAT );
            DEFINE_ENUM_ELEMENT_MAPPING( m_StencilOpEnumMapping, STENCIL_OP_DECR_SAT );
            DEFINE_ENUM_ELEMENT_MAPPING( m_StencilOpEnumMapping, STENCIL_OP_INVERT );
            DEFINE_ENUM_ELEMENT_MAPPING( m_StencilOpEnumMapping, STENCIL_OP_INCR_WRAP );
            DEFINE_ENUM_ELEMENT_MAPPING( m_StencilOpEnumMapping, STENCIL_OP_DECR_WRAP );
            VERIFY( m_StencilOpEnumMapping.m_Str2ValMap.size() == STENCIL_OP_NUM_OPS - 1,
                    "Unexpected map size. Did you update STENCIL_OP enum?" );
            VERIFY( m_StencilOpEnumMapping.m_Val2StrMap.size() == STENCIL_OP_NUM_OPS - 1,
                    "Unexpected map size. Did you update STENCIL_OP enum?" );

            DEFINE_ENUM_BINDER( m_Bindings, StencilOpDesc, StencilFailOp, STENCIL_OP, m_StencilOpEnumMapping )
            DEFINE_ENUM_BINDER( m_Bindings, StencilOpDesc, StencilDepthFailOp, STENCIL_OP, m_StencilOpEnumMapping )
            DEFINE_ENUM_BINDER( m_Bindings, StencilOpDesc, StencilPassOp, STENCIL_OP, m_StencilOpEnumMapping )
            DEFINE_ENUM_BINDER( m_Bindings, StencilOpDesc, StencilFunc, COMPARISON_FUNCTION, m_CmpFuncEnumMapping )
        }

        virtual void GetValue( lua_State *L, const void* pBasePointer )
        {
            const auto &StOpDesc = GetMemberByOffest< StencilOpDesc >(pBasePointer, m_MemberOffset);
            PushLuaTable( L, &StOpDesc, m_Bindings );
        }

        virtual void SetValue( lua_State *L, int Index, void* pBasePointer )
        {
            auto &StOpDesc = GetMemberByOffest< StencilOpDesc >( pBasePointer, m_MemberOffset );
            ParseLuaTable( L, Index, &StOpDesc, m_Bindings );
        }

    private:
        BindingsMapType m_Bindings;
        ComparisonFuncEnumMapping  m_CmpFuncEnumMapping;
        EnumMapping < STENCIL_OP > m_StencilOpEnumMapping;
    };

    DepthStencilStateParser::DepthStencilStateParser( IRenderDevice *pRenderDevice, lua_State *L ) :
        EngineObjectParserCommon<IDepthStencilState>( pRenderDevice, L, DepthStencilStateLibName ),
        m_SetDepthStencilBinding( this, L, "Context", "SetDepthStencilState", &DepthStencilStateParser::SetDepthStencilState )
    {
        DEFINE_BUFFERED_STRING_BINDER( m_Bindings, SDSSDescWrapper, Name, NameBuffer )

        DEFINE_BINDER( m_Bindings, SDSSDescWrapper, DepthEnable, Bool, Validator<Bool>() )
        DEFINE_BINDER( m_Bindings, SDSSDescWrapper, DepthWriteEnable, Bool, Validator<Bool>() )
        DEFINE_ENUM_BINDER( m_Bindings, SDSSDescWrapper, DepthFunc, COMPARISON_FUNCTION, m_CmpFuncEnumMapping )

        DEFINE_BINDER( m_Bindings, SDSSDescWrapper, StencilEnable, Bool, Validator<Bool>() )
        DEFINE_BINDER( m_Bindings, SDSSDescWrapper, StencilReadMask, Uint8, Validator<Uint8>() )
        DEFINE_BINDER( m_Bindings, SDSSDescWrapper, StencilWriteMask, Uint8, Validator<Uint8>() )
        DEFINE_BINDER( m_Bindings, SDSSDescWrapper, FrontFace, StencilOpDesc, 0 )
        DEFINE_BINDER( m_Bindings, SDSSDescWrapper, BackFace, StencilOpDesc, 0 )
    };

    void DepthStencilStateParser::CreateObj( lua_State *L )
    {
        INIT_LUA_STACK_TRACKING( L );

        SDSSDescWrapper DepthStencilDesc;
        ParseLuaTable( L, 1, &DepthStencilDesc, m_Bindings );

        CHECK_LUA_STACK_HEIGHT();

        auto ppDepthStencilState = reinterpret_cast<IDepthStencilState**>(lua_newuserdata( L, sizeof( IDepthStencilState* ) ));
        *ppDepthStencilState = nullptr;
        m_pRenderDevice->CreateDepthStencilState( DepthStencilDesc, ppDepthStencilState );
        if( *ppDepthStencilState == nullptr )
            SCRIPT_PARSING_ERROR(L, "Failed to create depth stencil state")

        CHECK_LUA_STACK_HEIGHT( +1 );
    }

    int DepthStencilStateParser::SetDepthStencilState( lua_State *L )
    {
        auto pVertDesc = *GetUserData<IDepthStencilState**>( L, 1, m_MetatableRegistryName.c_str() );
        Uint8 StencilRef = 0;
        if( lua_gettop( L ) > 1 )
        {
            StencilRef = ReadValueFromLua<Uint8>( L, 2 );
        }
        auto *pContext = LoadDeviceContextFromRegistry( L );
        pContext->SetDepthStencilState( pVertDesc, StencilRef );
        return 0;
    }
}
