/*     Copyright 2015-2019 Egor Yusov
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
#include "DepthStencilStateDescParser.h"

namespace Diligent
{
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

            DEFINE_ENUM_BINDER( m_Bindings, StencilOpDesc, StencilFailOp,      m_StencilOpEnumMapping );
            DEFINE_ENUM_BINDER( m_Bindings, StencilOpDesc, StencilDepthFailOp, m_StencilOpEnumMapping );
            DEFINE_ENUM_BINDER( m_Bindings, StencilOpDesc, StencilPassOp,      m_StencilOpEnumMapping );
            DEFINE_ENUM_BINDER( m_Bindings, StencilOpDesc, StencilFunc,        m_CmpFuncEnumMapping );
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

    MemberBinder<DepthStencilStateDesc>::MemberBinder( size_t MemberOffset, size_t Dummy ) :
            MemberBinderBase( MemberOffset )
    {
        DEFINE_BINDER( m_Bindings, DepthStencilStateDesc, DepthEnable );
        DEFINE_BINDER( m_Bindings, DepthStencilStateDesc, DepthWriteEnable );
        DEFINE_ENUM_BINDER( m_Bindings, DepthStencilStateDesc, DepthFunc, m_CmpFuncEnumMapping );

        DEFINE_BINDER( m_Bindings, DepthStencilStateDesc, StencilEnable );
        DEFINE_BINDER( m_Bindings, DepthStencilStateDesc, StencilReadMask );
        DEFINE_BINDER( m_Bindings, DepthStencilStateDesc, StencilWriteMask );
        DEFINE_BINDER_EX( m_Bindings, DepthStencilStateDesc, FrontFace, StencilOpDesc, 0 );
        DEFINE_BINDER_EX( m_Bindings, DepthStencilStateDesc, BackFace, StencilOpDesc, 0 );
    };

    void MemberBinder<DepthStencilStateDesc> ::GetValue(lua_State *L, const void* pBasePointer)
    {
        const auto &DSSDesc = GetMemberByOffest< DepthStencilStateDesc >(pBasePointer, m_MemberOffset);
        PushLuaTable(L, &DSSDesc, m_Bindings);
    }

    void MemberBinder<DepthStencilStateDesc> ::SetValue(lua_State *L, int Index, void* pBasePointer)
    {
        auto &DSSDesc = GetMemberByOffest< DepthStencilStateDesc>( pBasePointer, m_MemberOffset );
        ParseLuaTable( L, Index, &DSSDesc, m_Bindings );
    }
}
