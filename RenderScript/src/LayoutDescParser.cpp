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
#include "LayoutDescParser.h"

namespace std 
{
    DEFINE_ENUM_HASH( Diligent::LayoutElement::FREQUENCY )
}

namespace Diligent
{
    const Char* LayoutDescParser::LayoutDescLibName = "LayoutDesc";

    class LayoutElementLoader; // Used only as a template switch

    template<>
    class MemberBinder<LayoutElementLoader> : public MemberBinderBase
    {
    public:
        MemberBinder( size_t LayoutElementsOffset, size_t ElementsBufferOffset, size_t NumElementsOffset ) :
            MemberBinderBase( ElementsBufferOffset ),
            m_LayoutElementsOffset(LayoutElementsOffset),
            m_NumElementsOffset(NumElementsOffset)
        {
            DEFINE_BINDER( m_Bindings, LayoutElement, InputIndex, Uint32, Validator<Uint32>( "Input Index", 0, 32 ) )
            DEFINE_BINDER( m_Bindings, LayoutElement, BufferSlot, Uint32, Validator<Uint32>( "Buffer Slot", 0, MaxBufferSlots ) )
            DEFINE_BINDER( m_Bindings, LayoutElement, NumComponents, Uint32, Validator<Uint32>( "Num Components", 1, 4 ) )

            DEFINE_ENUM_BINDER( m_Bindings, LayoutElement, ValueType, VALUE_TYPE, m_ValueTypeEnumMapping )

            DEFINE_BINDER( m_Bindings, LayoutElement, IsNormalized, Bool, Validator<Bool>() )
            DEFINE_BINDER( m_Bindings, LayoutElement, RelativeOffset, Uint32, Validator<Uint32>() )


            m_FrequencyEnumMapping.AddMapping( "FREQUENCY_PER_VERTEX", LayoutElement::FREQUENCY_PER_VERTEX );
            m_FrequencyEnumMapping.AddMapping( "FREQUENCY_PER_INSTANCE", LayoutElement::FREQUENCY_PER_INSTANCE );
            VERIFY( m_FrequencyEnumMapping.m_Str2ValMap.size() == LayoutElement::FREQUENCY_NUM_FREQUENCIES - 1,
                    "Unexpected map size. Did you update LayoutElement::FREQUENCY_PER_VERTEX enum?" );
            VERIFY( m_FrequencyEnumMapping.m_Val2StrMap.size() == LayoutElement::FREQUENCY_NUM_FREQUENCIES - 1,
                    "Unexpected map size. Did you update LayoutElement::FREQUENCY_PER_VERTEX enum?" );
            DEFINE_ENUM_BINDER( m_Bindings, LayoutElement, Frequency, LayoutElement::FREQUENCY, m_FrequencyEnumMapping )

            DEFINE_BINDER( m_Bindings, LayoutElement, InstanceDataStepRate, Uint32, Validator<Uint32>() )
        }

        virtual void GetValue( lua_State *L, const void* pBasePointer )
        {
            // Use raw pointer to push the value to Lua because the buffer
            // most likely does not exist
            const auto *Elements = GetMemberByOffest<const LayoutElement*>(pBasePointer, m_LayoutElementsOffset);
            const auto &NumElements = GetMemberByOffest< Uint32 >( pBasePointer, m_NumElementsOffset );
            PushLuaArray( L, Elements, Elements + NumElements, [&]( const LayoutElement &Elem )
            {
                PushLuaTable( L, &Elem, m_Bindings );
            }
            );
        }

        virtual void SetValue( lua_State *L, int Index, void* pBasePointer )
        {
            ParseLuaArray( L, Index, pBasePointer, [&]( void* _pBasePointer, int StackIndex, int NewArrayIndex )
            {
                VERIFY( pBasePointer == _pBasePointer, "Sanity check failed" );
                auto &Elements = GetMemberByOffest<std::vector<LayoutElement> >( pBasePointer, m_MemberOffset );
                auto CurrIndex = Elements.size();
                if( CurrIndex != NewArrayIndex - 1 )
                    SCRIPT_PARSING_ERROR( L, "Explicit array indices are not allowed in layout description.  Provided index ", NewArrayIndex - 1, " conflicts with actual index ", CurrIndex, "." );
                Elements.resize( CurrIndex + 1 );
                ParseLuaTable( L, StackIndex, &(Elements)[CurrIndex], m_Bindings );
                if( Elements[CurrIndex].ValueType == VT_UNDEFINED )
                    SCRIPT_PARSING_ERROR( L, "Valid value type must be specified for layout element #", CurrIndex );
            }
            );

            auto &ElementsBuffer = GetMemberByOffest<std::vector<LayoutElement> >( pBasePointer, m_MemberOffset );
            auto &LayoutElements = GetMemberByOffest< LayoutElement* >( pBasePointer, m_LayoutElementsOffset );
            auto &NumElements = GetMemberByOffest< Uint32 >( pBasePointer, m_NumElementsOffset );
            LayoutElements = ElementsBuffer.data();
            NumElements = static_cast<Uint32>( ElementsBuffer.size() );
        }
    private:
        BindingsMapType m_Bindings;
        ValueTypeEnumMapping m_ValueTypeEnumMapping;
        EnumMapping < LayoutElement::FREQUENCY > m_FrequencyEnumMapping;
        size_t m_LayoutElementsOffset;
        size_t m_NumElementsOffset;
    };

    LayoutDescParser::LayoutDescParser( IRenderDevice *pRenderDevice, lua_State *L ) :
        EngineObjectParserCommon<IVertexDescription>( pRenderDevice, L, LayoutDescLibName ),
        m_SetInputLayoutBinding( this, L, "Context", "SetInputLayout", &LayoutDescParser::SetInputLayout )
    {
        DEFINE_BUFFERED_STRING_BINDER( m_Bindings, LayoutDescWrapper, Name, NameBuffer )

        auto *pLayoutElemBinder = 
            new MemberBinder<LayoutElementLoader>( 
                offsetof(LayoutDescWrapper, LayoutElements), 
                offsetof(LayoutDescWrapper, ElementsBuffer), 
                offsetof(LayoutDescWrapper, NumElements) 
            );
        m_Bindings.insert( std::make_pair( "LayoutElements", std::unique_ptr<MemberBinderBase>(pLayoutElemBinder) ) );
    };

    void LayoutDescParser::CreateObj( lua_State *L )
    {
        INIT_LUA_STACK_TRACKING( L );

        LayoutDescWrapper LayoutDesc;
        ParseLuaTable( L, -2, &LayoutDesc, m_Bindings );
        CHECK_LUA_STACK_HEIGHT();

        auto ppVertexShader = GetUserData<IShader**>( L, -1, "Metatables.Shader" );
        auto ppVertDesc = reinterpret_cast<IVertexDescription**>(lua_newuserdata( L, sizeof( IVertexDescription* ) ));
        *ppVertDesc = nullptr;
        m_pRenderDevice->CreateVertexDescription( LayoutDesc, *ppVertexShader, ppVertDesc );
        if( *ppVertDesc == nullptr )
            SCRIPT_PARSING_ERROR(L, "Failed to create vertex description")

        CHECK_LUA_STACK_HEIGHT( +1 );
    }

    int LayoutDescParser::SetInputLayout( lua_State *L )
    {
        auto pVertDesc = *GetUserData<IVertexDescription**>( L, 1, m_MetatableRegistryName.c_str() );
        auto *pContext = LoadDeviceContextFromRegistry( L );
        pContext->SetVertexDescription( pVertDesc );
        return 0;
    }
}
