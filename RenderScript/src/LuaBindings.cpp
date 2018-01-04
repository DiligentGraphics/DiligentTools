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
#include "LuaBindings.h"
#include "GraphicsAccessories.h"

using namespace std;

namespace Diligent
{
    template<typename Type>
    Type ReadIntValueFromLua( lua_State *L, int Index )
    {
        CheckType( L, Index, LUA_TNUMBER );

        int isnum;
        auto Val = lua_tonumberx( L, Index, &isnum );
        if( !isnum )
        {
            auto Str = lua_tostring( L, Index );
            SCRIPT_PARSING_ERROR( L, "Failed to convert parameter ", Str, " to int" );
        }
        if( static_cast<Type>(Val) != Val )
        {
            SCRIPT_PARSING_ERROR( L, "Parameter value (", Val, ") is not integer. Truncating to int" );
        }
        return static_cast<Type>(Val);
    }

    template<typename Type>
    Type ReadFPValueFromLua( lua_State *L, int Index )
    {
        CheckType( L, Index, LUA_TNUMBER );

        int isnum;
        auto Val = lua_tonumberx( L, Index, &isnum );
        if( !isnum )
        {
            auto Str = lua_tostring( L, Index );
            SCRIPT_PARSING_ERROR( L, "Failed to convert parameter ", Str, " to floating point" );
        }
        return static_cast<Type>(Val);
    }

    template<>
    int ReadValueFromLua<int>( lua_State *L, int Index )
    {
        return ReadIntValueFromLua<int>( L, Index );
    }

    template<>
    double ReadValueFromLua<double>( lua_State *L, int Index )
    {
        return ReadFPValueFromLua<double>( L, Index );
    }

    template<>
    float ReadValueFromLua<float>( lua_State *L, int Index )
    {
        return ReadFPValueFromLua<float>( L, Index );
    }


    template<>
    String ReadValueFromLua<String>( lua_State *L, int Index )
    {
        CheckType( L, Index, LUA_TSTRING );

        auto Str = lua_tostring( L, Index );
        return String( Str );
    }

    template<>
    const Char* ReadValueFromLua<const Char*>( lua_State *L, int Index )
    {
        CheckType( L, Index, LUA_TSTRING );

        auto Str = lua_tostring( L, Index );
        return Str;
    }

    template<>
    Bool ReadValueFromLua<Bool>( lua_State *L, int Index )
    {
        CheckType( L, Index, LUA_TBOOLEAN );
        auto Val = lua_toboolean( L, Index );
        return Val ? True : False;
    }

    template<>
    Uint32 ReadValueFromLua<Uint32>( lua_State *L, int Index )
    {
        return ReadIntValueFromLua<Uint32>( L, Index );
    }

    template<>
    Uint8 ReadValueFromLua<Uint8>( lua_State *L, int Index )
    {
        return ReadIntValueFromLua<Uint8>( L, Index );
    }



    template<>
    void PushValue<double>( lua_State *L, double Val )
    {
        lua_pushnumber( L, Val );
    }

    template<>
    void PushValue<const float&>( lua_State *L, const float& Val )
    {
        lua_pushnumber( L, Val );
    }

    template<>
    void PushValue<const Int32&>( lua_State *L, const Int32& Val )
    {
        lua_pushnumber( L, Val );
    }

    template<>
    void PushValue<const Uint8&>( lua_State *L, const Uint8& Val )
    {
        lua_pushnumber( L, Val );
    }

    template<>
    void PushValue<const Uint32&>( lua_State *L, const Uint32& Val )
    {
        lua_pushnumber( L, Val );
    }

    template<>
    void PushValue<const Char*>( lua_State *L, const Char* Val )
    {
        lua_pushstring( L, Val );
    }
    
    template<> void PushValue<const Char* const&>( lua_State *L, const Char* const& Val )
    {
        lua_pushstring( L, Val );
    }

    template<>
    void PushValue<const String&>( lua_State *L, const String& Val )
    {
        lua_pushstring( L, Val.c_str() );
    }

    template<>
    void PushValue<bool>( lua_State *L, bool Val )
    {
        lua_pushboolean( L, Val );
    }

    template<>
    void PushValue<const bool &>( lua_State *L, const bool &Val )
    {
        lua_pushboolean( L, Val );
    }



    template<VALUE_TYPE VTType>
    void ParseNumericArray( lua_State *L, int StackIndex, std::vector< Uint8 >& RawData )
    {
        typedef typename VALUE_TYPE2CType<VTType>::CType ElemType;
        CheckType( L, StackIndex, LUA_TTABLE );
        auto ArraySize = lua_rawlen( L, StackIndex );

        auto ElemSize = sizeof( ElemType );
        RawData.reserve( ArraySize * ElemSize );

        ParseLuaArray( L, StackIndex, &RawData, [ &]( void* pBasePointer, int StackIndex, int NewArrayIndex )
        {
            VERIFY( pBasePointer == &RawData, "Sanity check failed" );
            auto CurrIndex = RawData.size() / ElemSize;
            if(static_cast<int>(CurrIndex) != NewArrayIndex - 1 )
                SCRIPT_PARSING_ERROR( L, "Explicit array indices are not allowed in array initialization. Provided index ", NewArrayIndex - 1, " conflicts with actual index ", CurrIndex, "." );
            RawData.resize( (CurrIndex + 1) * ElemSize );
            auto CurrValue = ReadValueFromLua<double>( L, StackIndex );
            reinterpret_cast<ElemType&>(RawData[CurrIndex * ElemSize]) = static_cast<ElemType>(CurrValue);
        } );
    }

    NumericArrayLoader::NumericArrayLoader() :
        m_ValueTypeBinder( 0, "VALUE_TYPE", m_ValueTypeEnumMapping )
    {
        DEFINE_ENUM_ELEMENT_MAPPING( m_ValueTypeEnumMapping, VT_INT8 );
        DEFINE_ENUM_ELEMENT_MAPPING( m_ValueTypeEnumMapping, VT_INT16 );
        DEFINE_ENUM_ELEMENT_MAPPING( m_ValueTypeEnumMapping, VT_INT32 );
        DEFINE_ENUM_ELEMENT_MAPPING( m_ValueTypeEnumMapping, VT_UINT8 );
        DEFINE_ENUM_ELEMENT_MAPPING( m_ValueTypeEnumMapping, VT_UINT16 );
        DEFINE_ENUM_ELEMENT_MAPPING( m_ValueTypeEnumMapping, VT_UINT32 );
        DEFINE_ENUM_ELEMENT_MAPPING( m_ValueTypeEnumMapping, VT_FLOAT16 );
        DEFINE_ENUM_ELEMENT_MAPPING( m_ValueTypeEnumMapping, VT_FLOAT32 );
        VERIFY( m_ValueTypeEnumMapping.m_Str2ValMap.size() == VT_NUM_TYPES - 1,
                "Unexpected map size. Did you update VALUE_TYPE enum?" );
        VERIFY( m_ValueTypeEnumMapping.m_Val2StrMap.size() == VT_NUM_TYPES - 1,
                "Unexpected map size. Did you update VALUE_TYPE enum?" );

        m_ParseFuncJumpTbl.insert( make_pair( VT_INT8,    ParseNumericArray<VT_INT8> ) );
        m_ParseFuncJumpTbl.insert( make_pair( VT_INT16,   ParseNumericArray<VT_INT16> ) );
        m_ParseFuncJumpTbl.insert( make_pair( VT_INT32,   ParseNumericArray<VT_INT32> ) );
        m_ParseFuncJumpTbl.insert( make_pair( VT_UINT8,   ParseNumericArray<VT_UINT8> ) );
        m_ParseFuncJumpTbl.insert( make_pair( VT_UINT16,  ParseNumericArray<VT_UINT16> ) );
        m_ParseFuncJumpTbl.insert( make_pair( VT_UINT32,  ParseNumericArray<VT_UINT32> ) );
        //m_ParseFuncJumpTbl.insert( make_pair( VT_FLOAT16, ParseNumericArray<VT_FLOAT16> ) );
        m_ParseFuncJumpTbl.insert( make_pair( VT_FLOAT32, ParseNumericArray<VT_FLOAT32> ) );
    };


    void NumericArrayLoader::LoadArray( lua_State *L, int StackIndex, std::vector< Uint8 >& RawData )
    {
        VALUE_TYPE ValueType;
        m_ValueTypeBinder.SetValue( L, StackIndex-1, &ValueType );

        auto it = m_ParseFuncJumpTbl.find( ValueType );
        if( it != m_ParseFuncJumpTbl.end() )
        {
            it->second( L, StackIndex, RawData );
        }
        else
        {
            SCRIPT_PARSING_ERROR( L, "No method to parse array of value VALUE_TYPE==", ValueType);
        }
    }

    // Special version of luaL_testudata() which takes an array of allowed metatables
    void *luaL_testudata( lua_State *L, int ud, const std::vector<String> &MetatableNames )
    {
        void *p = lua_touserdata( L, ud );
        if( p != nullptr )
        {
            // value is a userdata?
            if( lua_getmetatable( L, ud ) )                         // -0 | +(0|1) -> +(0|1)
            {
                // does it have a metatable?
                bool bMatchingMTFound = false;
                for( auto mtname = MetatableNames.begin(); mtname != MetatableNames.end() && !bMatchingMTFound; ++mtname )
                {
                    // get correct metatable
                    luaL_getmetatable( L, mtname->c_str() );        // -0 | +1 -> +1
                    if( lua_rawequal( L, -1, -2 ) )  // are the same?
                        bMatchingMTFound = true;

                    // pop correct metatable
                    lua_pop( L, 1 );                                // -1 | +0 -> -1
                }
                // pop user metatable
                lua_pop( L, 1 );                                    // -1 | +0 -> -1
                return bMatchingMTFound ? p : nullptr;
            }
        }
        return nullptr;  // value is not a userdata with a metatable
    }
}
