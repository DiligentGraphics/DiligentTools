/*     Copyright 2015-2016 Egor Yusov
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
#include "InputLayoutDescParser.h"

namespace Diligent
{
    MemberBinder<InputLayoutDesc> :: MemberBinder( size_t InputLayoutOffset, size_t ElementsBufferOffset ) :
        MemberBinderBase( InputLayoutOffset ),
        m_LayoutElementsBufferOffset(ElementsBufferOffset)
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

    void MemberBinder<InputLayoutDesc> :: GetValue( lua_State *L, const void* pBasePointer )
    {
        // Use raw pointer to push the value to Lua because the buffer
        // most likely does not exist
        const auto &InputLayout = GetMemberByOffest<const InputLayoutDesc>(pBasePointer, m_MemberOffset);
        PushLuaArray( L, InputLayout.LayoutElements, InputLayout.LayoutElements + InputLayout.NumElements, [&]( const LayoutElement &Elem )
        {
            PushLuaTable( L, &Elem, m_Bindings );
        }
        );
    }

    void MemberBinder<InputLayoutDesc> :: SetValue( lua_State *L, int Index, void* pBasePointer )
    {
        ParseLuaArray( L, Index, pBasePointer, [&]( void* _pBasePointer, int StackIndex, int NewArrayIndex )
        {
            VERIFY( pBasePointer == _pBasePointer, "Sanity check failed" );
            auto &Elements = GetMemberByOffest<std::vector<LayoutElement> >( pBasePointer, m_LayoutElementsBufferOffset);
            auto CurrIndex = Elements.size();
            if( CurrIndex != NewArrayIndex - 1 )
                SCRIPT_PARSING_ERROR( L, "Explicit array indices are not allowed in layout description.  Provided index ", NewArrayIndex - 1, " conflicts with actual index ", CurrIndex, "." );
            Elements.resize( CurrIndex + 1 );
            ParseLuaTable( L, StackIndex, &(Elements)[CurrIndex], m_Bindings );
            if( Elements[CurrIndex].ValueType == VT_UNDEFINED )
                SCRIPT_PARSING_ERROR( L, "Valid value type must be specified for layout element #", CurrIndex );
        }
        );

        auto &ElementsBuffer = GetMemberByOffest<std::vector<LayoutElement> >( pBasePointer, m_LayoutElementsBufferOffset );
        auto &InputLayout = GetMemberByOffest< InputLayoutDesc >( pBasePointer, m_MemberOffset );
        InputLayout.LayoutElements = ElementsBuffer.data();
        InputLayout.NumElements = static_cast<Uint32>( ElementsBuffer.size() );
    }
}
