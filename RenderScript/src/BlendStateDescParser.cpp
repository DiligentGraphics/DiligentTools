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

#include "pch.h"
#include "BlendStateDescParser.h"
#include "LuaWrappers.h"
#include "ClassMethodBinding.h"


namespace std 
{
    DEFINE_ENUM_HASH( Diligent::BLEND_FACTOR )
    DEFINE_ENUM_HASH( Diligent::BLEND_OPERATION )
    DEFINE_ENUM_HASH( Diligent::COLOR_MASK )
}

namespace Diligent
{
    class RenderTargetBlendDescArrayParser;

    template<>
    class MemberBinder<RenderTargetBlendDescArrayParser> : public MemberBinderBase
    {
    public:
        MemberBinder( size_t MemberOffset, size_t Dummy ) :
            MemberBinderBase( MemberOffset )
        {
            DEFINE_BINDER( m_Bindings, RenderTargetBlendDesc, BlendEnable );
            
            DEFINE_ENUM_ELEMENT_MAPPING( m_BlendFactorEnumMapping, BLEND_FACTOR_ZERO);
            DEFINE_ENUM_ELEMENT_MAPPING( m_BlendFactorEnumMapping, BLEND_FACTOR_ONE);
            DEFINE_ENUM_ELEMENT_MAPPING( m_BlendFactorEnumMapping, BLEND_FACTOR_SRC_COLOR);
            DEFINE_ENUM_ELEMENT_MAPPING( m_BlendFactorEnumMapping, BLEND_FACTOR_INV_SRC_COLOR);
            DEFINE_ENUM_ELEMENT_MAPPING( m_BlendFactorEnumMapping, BLEND_FACTOR_SRC_ALPHA);
            DEFINE_ENUM_ELEMENT_MAPPING( m_BlendFactorEnumMapping, BLEND_FACTOR_INV_SRC_ALPHA);
            DEFINE_ENUM_ELEMENT_MAPPING( m_BlendFactorEnumMapping, BLEND_FACTOR_DEST_ALPHA);
            DEFINE_ENUM_ELEMENT_MAPPING( m_BlendFactorEnumMapping, BLEND_FACTOR_INV_DEST_ALPHA);
            DEFINE_ENUM_ELEMENT_MAPPING( m_BlendFactorEnumMapping, BLEND_FACTOR_DEST_COLOR);
            DEFINE_ENUM_ELEMENT_MAPPING( m_BlendFactorEnumMapping, BLEND_FACTOR_INV_DEST_COLOR);
            DEFINE_ENUM_ELEMENT_MAPPING( m_BlendFactorEnumMapping, BLEND_FACTOR_SRC_ALPHA_SAT);
            DEFINE_ENUM_ELEMENT_MAPPING( m_BlendFactorEnumMapping, BLEND_FACTOR_BLEND_FACTOR);
            DEFINE_ENUM_ELEMENT_MAPPING( m_BlendFactorEnumMapping, BLEND_FACTOR_INV_BLEND_FACTOR);
            DEFINE_ENUM_ELEMENT_MAPPING( m_BlendFactorEnumMapping, BLEND_FACTOR_SRC1_COLOR);
            DEFINE_ENUM_ELEMENT_MAPPING( m_BlendFactorEnumMapping, BLEND_FACTOR_INV_SRC1_COLOR);
            DEFINE_ENUM_ELEMENT_MAPPING( m_BlendFactorEnumMapping, BLEND_FACTOR_SRC1_ALPHA);
            DEFINE_ENUM_ELEMENT_MAPPING( m_BlendFactorEnumMapping, BLEND_FACTOR_INV_SRC1_ALPHA);
            VERIFY( m_BlendFactorEnumMapping.m_Str2ValMap.size() == BLEND_FACTOR_NUM_FACTORS - 1,
                    "Unexpected map size. Did you update BLEND_FACTOR enum?" );
            VERIFY( m_BlendFactorEnumMapping.m_Val2StrMap.size() == BLEND_FACTOR_NUM_FACTORS - 1,
                    "Unexpected map size. Did you update BLEND_FACTOR enum?" );

            DEFINE_ENUM_BINDER( m_Bindings, RenderTargetBlendDesc, SrcBlend,       m_BlendFactorEnumMapping );
            DEFINE_ENUM_BINDER( m_Bindings, RenderTargetBlendDesc, DestBlend,      m_BlendFactorEnumMapping );
            DEFINE_ENUM_BINDER( m_Bindings, RenderTargetBlendDesc, SrcBlendAlpha,  m_BlendFactorEnumMapping );
            DEFINE_ENUM_BINDER( m_Bindings, RenderTargetBlendDesc, DestBlendAlpha, m_BlendFactorEnumMapping );


            DEFINE_ENUM_ELEMENT_MAPPING( m_BlendOpEnumMapping, BLEND_OPERATION_ADD );
            DEFINE_ENUM_ELEMENT_MAPPING( m_BlendOpEnumMapping, BLEND_OPERATION_SUBTRACT );
            DEFINE_ENUM_ELEMENT_MAPPING( m_BlendOpEnumMapping, BLEND_OPERATION_REV_SUBTRACT );
            DEFINE_ENUM_ELEMENT_MAPPING( m_BlendOpEnumMapping, BLEND_OPERATION_MIN );
            DEFINE_ENUM_ELEMENT_MAPPING( m_BlendOpEnumMapping, BLEND_OPERATION_MAX );
            VERIFY( m_BlendOpEnumMapping.m_Str2ValMap.size() == BLEND_OPERATION_NUM_OPERATIONS - 1,
                    "Unexpected map size. Did you update BLEND_OPERATION enum?" );
            VERIFY( m_BlendOpEnumMapping.m_Val2StrMap.size() == BLEND_OPERATION_NUM_OPERATIONS - 1,
                    "Unexpected map size. Did you update BLEND_OPERATION enum?" );

            DEFINE_ENUM_BINDER( m_Bindings, RenderTargetBlendDesc, BlendOp,      m_BlendOpEnumMapping );
            DEFINE_ENUM_BINDER( m_Bindings, RenderTargetBlendDesc, BlendOpAlpha, m_BlendOpEnumMapping );


            DEFINE_ENUM_ELEMENT_MAPPING( m_ColorMaskEnumMapping, COLOR_MASK_RED );
            DEFINE_ENUM_ELEMENT_MAPPING( m_ColorMaskEnumMapping, COLOR_MASK_GREEN );
            DEFINE_ENUM_ELEMENT_MAPPING( m_ColorMaskEnumMapping, COLOR_MASK_BLUE );
            DEFINE_ENUM_ELEMENT_MAPPING( m_ColorMaskEnumMapping, COLOR_MASK_ALPHA );
            DEFINE_ENUM_ELEMENT_MAPPING( m_ColorMaskEnumMapping, COLOR_MASK_ALL );
            DEFINE_FLAGS_BINDER( m_Bindings, RenderTargetBlendDesc, RenderTargetWriteMask, COLOR_MASK, m_ColorMaskEnumMapping );
        }

        virtual void GetValue( lua_State *L, const void* pBasePointer )
        {
            const auto &RTs = GetMemberByOffest< const RenderTargetBlendDesc >(pBasePointer, m_MemberOffset);
            PushLuaArray( L, &RTs, &RTs+BlendStateDesc::MaxRenderTargets, [&](const RenderTargetBlendDesc &Elem)
            {
                PushLuaTable( L, &Elem, m_Bindings );
            } 
            );
        }

        virtual void SetValue( lua_State *L, int Index, void* pBasePointer )
        {
            ParseLuaArray( L, Index, pBasePointer, [&]( void* _pBasePointer, int StackIndex, int NewArrayIndex )
            {
                VERIFY( pBasePointer == _pBasePointer, "Sanity check" );
                auto &RenderTargets = GetMemberByOffest<RenderTargetBlendDesc>( pBasePointer, m_MemberOffset );
                
                auto MaxRTs = BlendStateDesc::MaxRenderTargets;
                if( !(NewArrayIndex >= 1 && NewArrayIndex <= MaxRTs) )
                    SCRIPT_PARSING_ERROR( L, "Incorrect render target index ",  NewArrayIndex, ". Only 1..", MaxRTs, " are allowed" );

                ParseLuaTable( L, StackIndex, &RenderTargets + (NewArrayIndex-1), m_Bindings );
            }
            );
        }
    private:
        BindingsMapType m_Bindings;

        EnumMapping < BLEND_FACTOR > m_BlendFactorEnumMapping;
        EnumMapping < BLEND_OPERATION > m_BlendOpEnumMapping;
        EnumMapping < COLOR_MASK > m_ColorMaskEnumMapping;
    };
    

    MemberBinder<BlendStateDesc> :: MemberBinder( size_t MemberOffset, size_t Dummy ) :
        MemberBinderBase( MemberOffset )
    {
        DEFINE_BINDER( m_Bindings, BlendStateDesc, AlphaToCoverageEnable );
        DEFINE_BINDER( m_Bindings, BlendStateDesc, IndependentBlendEnable );

        DEFINE_BINDER_EX( m_Bindings, BlendStateDesc, RenderTargets, RenderTargetBlendDescArrayParser, 0 );
    }

    void MemberBinder<BlendStateDesc> ::GetValue(lua_State *L, const void* pBasePointer)
    {
        const auto &BlendDesc = GetMemberByOffest< BlendStateDesc >(pBasePointer, m_MemberOffset);
        PushLuaTable(L, &BlendDesc, m_Bindings);
    }

    void MemberBinder<BlendStateDesc> ::SetValue(lua_State *L, int Index, void* pBasePointer)
    {
        auto &BlendDesc = GetMemberByOffest< BlendStateDesc>( pBasePointer, m_MemberOffset );
        ParseLuaTable( L, Index, &BlendDesc, m_Bindings );
    }
}
