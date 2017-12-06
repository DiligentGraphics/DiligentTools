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

#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
#include "EnumMappings.h"

namespace Diligent
{
    template< typename ValueType>
    ValueType ReadValueFromLua( lua_State *L, int Index )
    {
        UNSUPPORTED( "Type is not supported" );
        return static_cast<ValueType>(0);
    }
    template<>int ReadValueFromLua<int>( lua_State *L, int Index );
    template<>double ReadValueFromLua<double>( lua_State *L, int Index );
    template<>float ReadValueFromLua<float>( lua_State *L, int Index );
    template<>String ReadValueFromLua<String>( lua_State *L, int Index );
    template<>const Char* ReadValueFromLua<const Char*>( lua_State *L, int Index );
    template<>Bool ReadValueFromLua<Bool>( lua_State *L, int Index );
    template<>Uint32 ReadValueFromLua<Uint32>( lua_State *L, int Index );
    template<>Uint8 ReadValueFromLua<Uint8>( lua_State *L, int Index );


    inline void CheckType( lua_State *L, int Index, int ExpectedType )
    {
        auto Type = lua_type( L, Index );
        if( Type != ExpectedType )
        {
            auto TypeName = lua_typename( L, Type );
            auto ExpectedTypeName = lua_typename( L, ExpectedType );
            auto Param = lua_tostring( L, Index );
            SCRIPT_PARSING_ERROR( L, "Incorrect argument: \"", Param ? Param : "<Unknown>", "\". \"", ExpectedTypeName, "\" is expected, while \"", TypeName, "\" is provided." );
        }
    }

    // Special version of luaL_testudata() which takes an array of allowed metatables
    void *luaL_testudata( lua_State *L, int ud, const std::vector<String> &MetatableNames );

    template<typename DataType, typename MetatableNameType>
    DataType GetUserData( lua_State *L, int Index, MetatableNameType MetatableName )
    {
        CheckType( L, Index, LUA_TUSERDATA );
        auto pUserData = luaL_testudata( L, Index, MetatableName );
        if( pUserData == nullptr )
        {
            auto Type = lua_type( L, Index );
            auto TypeName = lua_typename( L, Type );
            SCRIPT_PARSING_ERROR( L, "Bad argument #", Index, ". User data with metatable \"", MetatableName, "\" is expected. \"", TypeName, "\" is provided." );
        }
        return reinterpret_cast<DataType>(pUserData);
    }

    template<typename DataType>
    DataType GetGlobalObject( lua_State *L, const Char* ObjectName, const Char* MetatableName )
    {
        INIT_LUA_STACK_TRACKING( L );
        // Pushes onto the stack the value of the given global name
        lua_getglobal( L, ObjectName );                             // -0 | +1 -> +1
        auto pData = GetUserData<DataType>( L, -1, MetatableName ); // -0 | +0 -> 0
        lua_pop( L, 1 );                                            // -1 | +0 -> -1
        CHECK_LUA_STACK_HEIGHT();
        return pData;
    }



    template<typename Type>
    void PushValue( lua_State *L, Type )
    {
        UNSUPPORTED( "Type is not supported" );
    }
    // Forward declarations of template specializations
    template<> void PushValue<double>( lua_State *L, double Val );
    template<> void PushValue<const float&>( lua_State *L, const float& Val );
    template<> void PushValue<const Int32&>( lua_State *L, const Int32& Val );
    template<> void PushValue<const Uint32&>( lua_State *L, const Uint32& Val );
    template<> void PushValue<const Uint8&>( lua_State *L, const Uint8& Val );
    template<> void PushValue<const Char*>( lua_State *L, const Char* Val );
    template<> void PushValue<const Char* const&>( lua_State *L, const Char* const& Val );
    template<> void PushValue<const String&>( lua_State *L, const String& Val );
    template<> void PushValue<bool>( lua_State *L, bool Val );
    template<> void PushValue<const bool &>( lua_State *L, const bool &Val );


    class MemberBinderBase
    {
    public:
        MemberBinderBase( size_t MemberOffset ) :
            m_MemberOffset( MemberOffset )
        {}
        virtual ~MemberBinderBase(){}
        virtual void GetValue( lua_State *L, const void* pBasePointer ) = 0;
        virtual void SetValue( lua_State *L, int Index, void* pBasePointer ) = 0;

    protected:
        const size_t m_MemberOffset;
    };
    typedef std::unordered_map<Diligent::HashMapStringKey, std::unique_ptr<MemberBinderBase> > BindingsMapType;

#define DEFINE_BINDER(BindingsMap, Struct, Member, type, ValidationFunc) \
    {\
        auto *pNewBinder = new MemberBinder<type>( offsetof( Struct, Member ), ValidationFunc ); \
        /* No need to make a copy of #Member since it is constant string.  */                     \
        /* HashMapStringKey will simply keep pointer to it                 */                     \
        BindingsMap.insert( std::make_pair( #Member, std::unique_ptr<MemberBinderBase>(pNewBinder) ) ); \
    }

    template<typename ValueType>
    void SkipValidationFunc( const ValueType & )
    {
        // Do nothing
    }

    template<typename ValueType>
    class Validator
    {
    public:
        typedef void( *ValidationFuncType )(const ValueType &);
        Validator( const char *pParameterName, ValueType MinValue, ValueType MaxValue ) :
            m_pParameterName( pParameterName ),
            m_MinValue( MinValue ),
            m_MaxValue( MaxValue ),
            m_ValidationFunc( nullptr )
        {}

        Validator( ValidationFuncType ValidationFunc = SkipValidationFunc<ValueType> ) :
            m_pParameterName( nullptr ),
            m_MinValue( ValueType() ),
            m_MaxValue( ValueType() ),
            m_ValidationFunc( ValidationFunc )
        {}

        void SetParameterName( const char *NewName ){ m_pParameterName = NewName; }

        void operator()( lua_State *L, const ValueType &Value )const
        {
            if( m_ValidationFunc )
            {
                m_ValidationFunc( Value );
            }
            else
            {
                if( Value < m_MinValue || Value > m_MaxValue )
                {
                    SCRIPT_PARSING_ERROR( L, "Parameter '", m_pParameterName, "' (", Value, ") is out of range [", m_MinValue, ",", m_MaxValue, "]\n" );
                }
            }
        }

    private:
        const char *m_pParameterName;
        ValueType m_MinValue, m_MaxValue;
        ValidationFuncType m_ValidationFunc;
    };

    template<>
    class Validator < Bool >
    {
    public:
        void operator()( lua_State *L, const Bool & )const
        {
            // Do nothing
        }
    };
    
    template<typename MemberType>
    MemberType& GetMemberByOffest( void* pBasePointer, size_t Offset )
    {
        return *(reinterpret_cast<MemberType*>(reinterpret_cast<char*>(pBasePointer)+Offset));
    }

    template<typename MemberType>
    const MemberType& GetMemberByOffest( const void* pBasePointer, size_t Offset )
    {
        return *(reinterpret_cast<const MemberType*>(reinterpret_cast<const char*>(pBasePointer)+Offset));
    }

    template<typename MemberType>
    class MemberBinder : public MemberBinderBase
    {
    public:
        MemberBinder( size_t MemberOffset, Validator<MemberType> Validator ) :
            MemberBinderBase( MemberOffset ),
            m_Validator( Validator )
        {}

        virtual void GetValue( lua_State *L, const void* pBasePointer )
        {
            const auto &Value = GetMemberByOffest<MemberType>(pBasePointer, m_MemberOffset);
            PushValue<const MemberType &>( L, Value );
        }

        virtual void SetValue( lua_State *L, int Index, void* pBasePointer )
        {
            auto Value = ReadValueFromLua<MemberType>( L, Index );
            m_Validator( L, Value );
            GetMemberByOffest<MemberType>( pBasePointer, m_MemberOffset ) = Value;
        }
    protected:
        Validator<MemberType> m_Validator;
    };


    template<typename TableElementParser>
    inline void ParseLuaTable( lua_State *L, int Index, void* pBasePointer, TableElementParser ElemParser )
    {
        CheckType( L, Index, LUA_TTABLE );

        lua_pushnil( L );  // first key which will be popped out in the first call to lua_next()
        // lua_next() pops a key from the stack, and pushes a key–value pair from the table at the given index 
        // (the "next" pair after the given key). If there are no more elements in the table, then lua_next returns 
        // 0 (and pushes nothing).
        if( Index < 0 )
            --Index;
        while( lua_next( L, Index ) != 0 )
        {
            //  Key is now at index -2 and Value is at index -1
            auto IsString = lua_isstring( L, -2 );
            if( !IsString )
            {
                SCRIPT_PARSING_ERROR( L, "Table key value must be string")
            }
            // NOTE: the lua_tostring function returns a pointer to an internal copy of the string. 
            // The string is always zero-terminated and Lua ensures that this pointer is valid as long 
            // as the corresponding value is in the stack. 
            auto Key = lua_tostring( L, -2 );
            ElemParser(-1, pBasePointer, Key);

            // Pop value from the stack, but KEEP Key for the next iteration
            lua_pop( L, 1 );
        }
    }

    inline void ParseLuaTable( lua_State *L, int Index, void* pBasePointer, BindingsMapType &Bindings )
    {
        ParseLuaTable( L, Index, pBasePointer, 
                       [&](int Index, void* pBasePointer, const char *Key)
                        {
                            auto Binding = Bindings.find( Key );
                            if( Binding != Bindings.end() )
                            {
                                Binding->second->SetValue( L, Index, pBasePointer );
                            }
                            else
                            {
                                SCRIPT_PARSING_ERROR( L, "Unknown Member \"", Key, '\"' );
                            }
                        }
                     );
    }

    template<class ArrayElemParser>
    inline void ParseLuaArray( lua_State *L, int Index, void* pBasePointer, ArrayElemParser ElemParser )
    {
        CheckType( L, Index, LUA_TTABLE );

        lua_pushnil( L );  // first key which will be popped out in the first call to lua_next()
        // lua_next() pops a key from the stack, and pushes a key–value pair from the table at the given index 
        // (the "next" pair after the given key). If there are no more elements in the table, then lua_next returns 
        // 0 (and pushes nothing).
        if( Index < 0 )
            --Index;
        while( lua_next( L, Index ) != 0 )
        {
            //  Key is now at index -2 and Value is at index -1
            CheckType( L, -2, LUA_TNUMBER );

            auto NewArrayIndex = static_cast<int>( lua_tointeger( L, -2 ) );
            ElemParser( pBasePointer, -1, NewArrayIndex );

            // Pop value from the stack, but KEEP Key for the next iteration
            lua_pop( L, 1 );
        }
    }

    inline void PushLuaTable( lua_State *L, const void* pBasePointer, BindingsMapType &Bindings )
    {
        lua_newtable( L );
        for( auto it = Bindings.begin(); it != Bindings.end(); ++it )
        {
            const auto &Field = it->first.GetStr();
            lua_pushstring( L, Field );
            it->second->GetValue( L, pBasePointer );
            lua_settable( L, -3 ); // Stack: 0
        }
    }

    template<typename ItType, class ElemPushAlg>
    void PushLuaArray( lua_State *L, ItType Begin, ItType End, ElemPushAlg PushAlg )
    {
        lua_newtable( L );
        int ArrayInd = 1; // Lua arrays are 1-based
        for( auto it = Begin; it != End; ++it, ++ArrayInd )
        {
            lua_pushnumber( L, ArrayInd );  // -0 | +1 -> +1
            PushAlg( *it );                 // -0 | +1 -> +1
            lua_settable( L, -3 );          // -2 | +0 -> -2
        }
    }
    
    template<typename ArrayType, class ElemPushAlg>
    void PushLuaArray( lua_State *L, const ArrayType& Arr, ElemPushAlg PushAlg )
    {
        PushLuaArray( L, Arr.begin(), Arr.end(), PushAlg );
    }

    inline void PushField( lua_State *L, const void* pBasePointer, const Char *Field, BindingsMapType &Bindings )
    {
        auto It = Bindings.find( Field );
        if( It != Bindings.end() )
        {
            It->second->GetValue( L, pBasePointer );
        }
        else
        {
            SCRIPT_PARSING_ERROR( L, "Unknown Member \"", Field, '\"' );
        }
    }

    inline void UpdateField( lua_State *L, int Index, void* pBasePointer, const Char *Field, BindingsMapType &Bindings )
    {
        auto It = Bindings.find( Field );
        if( It != Bindings.end() )
        {
            It->second->SetValue( L, Index, pBasePointer );
        }
        else
        {
            SCRIPT_PARSING_ERROR( L, "Unknown Member \"", Field, '\"' );
        }
    }

    class RGBALoader; // Used only as a template switch
    template<>
    class MemberBinder<RGBALoader> : public MemberBinderBase
    {
    public:
        MemberBinder( size_t MemberOffset, size_t Dummy ) :
            MemberBinderBase( MemberOffset )
        {
            const char* Members[] = { "r", "g", "b", "a" };
            for( int c = 0; c < 4; ++c )
            {
                auto *pNewBinder = new MemberBinder<Float32>( MemberOffset + sizeof( Float32 )*c, Validator<float>() );
                // No need to make a copy of Members[c] since it is constant string.
                // HashMapStringKey will simply keep pointer to it
                Bindings.insert( std::make_pair( Members[c], std::unique_ptr<MemberBinderBase>( pNewBinder ) ) );
            }
        }
        virtual void GetValue( lua_State *L, const void* pBasePointer )
        {
            PushLuaTable( L, pBasePointer, Bindings );
        }

        virtual void SetValue( lua_State *L, int Index, void* pBasePointer )
        {
            ParseLuaTable( L, Index, pBasePointer, Bindings );
        }
    private:
        BindingsMapType Bindings;
    };


    template< typename EnumType >
    String GetEnumMappingsString( const EnumMapping<EnumType> &EnumMapping )
    {
        String Values;
        bool bFirst = true;
        for( auto it = EnumMapping.m_Str2ValMap.begin(); it != EnumMapping.m_Str2ValMap.end(); ++it )
        {
            if( !bFirst )
                Values += ", ";
            Values += '\"';
            Values.append( it->first.GetStr() );
            Values += '\"';
            bFirst = false;
        }
        return Values;
    }

    template<typename EnumType>
    class EnumMemberBinder : public MemberBinderBase
    {
    public:
        EnumMemberBinder( size_t MemberOffset, const Char* MemberName, const EnumMapping<EnumType> &EnumMapping ) :
            MemberBinderBase( MemberOffset ),
            m_MemberName( MemberName ),
            m_EnumMapping( EnumMapping )
        {
        }

        virtual void GetValue( lua_State *L, const void* pBasePointer )
        {
            const auto &Val = GetMemberByOffest<EnumType>( pBasePointer, m_MemberOffset );
            auto It = m_EnumMapping.m_Val2StrMap.find( Val );
            if( It != m_EnumMapping.m_Val2StrMap.end() )
            {
                const String& StrVal = It->second;
                PushValue<const String&>( L, StrVal );
            }
            else
            {
                UNEXPECTED( "Enum value (", Val, ") not found in the map" );
                SCRIPT_PARSING_ERROR( L, "Enum value (", Val, ") not found in the map" );
            }
        }

        virtual void SetValue( lua_State *L, int Index, void* pBasePointer )
        {
            auto Str = ReadValueFromLua<String>( L, Index );
            auto It = m_EnumMapping.m_Str2ValMap.find( Str.c_str() );
            if( It != m_EnumMapping.m_Str2ValMap.end() )
            {
                auto Val = It->second;
                GetMemberByOffest<EnumType>(pBasePointer, m_MemberOffset) = Val;
            }
            else
            {
                String AllowableValues = GetEnumMappingsString<EnumType>( m_EnumMapping );
                SCRIPT_PARSING_ERROR( L, "Unknown value (\"", Str, "\") provided for parameter ", m_MemberName, ". Only the following values are allowed:\n", AllowableValues );
            }
        }

    private:
        const Char *m_MemberName;
        const EnumMapping<EnumType> &m_EnumMapping;
    };

#define DEFINE_ENUM_BINDER(BindingsMap, Struct, Member, type, EnumMapping ) \
    {\
        auto *pNewBinder = new EnumMemberBinder<type>( offsetof( Struct, Member ), #Member, EnumMapping ); \
        /* No need to make a copy of #Member since it is constant string.  */                               \
        /* HashMapStringKey will simply keep pointer to it                 */                               \
        BindingsMap.insert( std::make_pair( #Member, std::unique_ptr<MemberBinderBase>(pNewBinder) ) ); \
    }

    template< typename EnumType, typename FlagsType = Uint32 >
    class FlagsLoader : public MemberBinderBase
    {
    public:
        FlagsLoader( size_t MemberOffset, const Char* MemberName, const EnumMapping<EnumType> &EnumMapping ) :
            MemberBinderBase( MemberOffset ),
            m_MemberName( MemberName ),
            m_EnumMapping( EnumMapping )
        {
        }
        virtual void GetValue( lua_State *L, const void* pBasePointer )
        {
            auto Flags = GetMemberByOffest<FlagsType>( pBasePointer, m_MemberOffset );
            lua_newtable( L );
            int ArrayInd = 1;
            for( auto it = m_EnumMapping.m_Val2StrMap.begin(); it != m_EnumMapping.m_Val2StrMap.end(); ++it )
            {
                if( static_cast<EnumType>(Flags & it->first) == it->first )
                {
                    lua_pushnumber( L, ArrayInd );              // -0 | +1 -> +1
                    PushValue<const String&>( L, it->second );  // -0 | +1 -> +1
                    lua_settable( L, -3 );                      // -2 | +0 -> -2
                    ++ArrayInd;
                }
            }
        }

        virtual void SetValue( lua_State *L, int Index, void* pBasePointer )
        {
            FlagsType Flags = 0;
            if( lua_isnumber( L, Index ) )
            {
                Flags = static_cast<FlagsType>( ReadValueFromLua<Uint32>( L, Index ) );
            }
            else if( lua_isstring( L, Index ) )
            {
                Flags = ReadFlag( L, Index );
            }
            else if( lua_istable( L, Index ) )
            {
                ParseLuaArray( L, Index, pBasePointer, [ &]( void* _pBasePointer, int StackIndex, int NewArrayIndex )
                {
                    auto CurrFlag = ReadFlag( L, StackIndex );
                    Flags |= CurrFlag;
                }
                );
            }
            else
            {
                SCRIPT_PARSING_ERROR( L, m_MemberName, "must be specified as a single string or an array of strings." );
            }
            GetMemberByOffest<FlagsType>( pBasePointer, m_MemberOffset ) = Flags;
        }

    private:
        FlagsType ReadFlag( lua_State *L, int StackIndex )
        {
            auto CurrFlagName = ReadValueFromLua<const Char *>( L, StackIndex );
            auto It = m_EnumMapping.m_Str2ValMap.find( CurrFlagName );
            if( It != m_EnumMapping.m_Str2ValMap.end() )
            {
                return It->second;
            }
            else
            {
                String AllowableValues = GetEnumMappingsString<EnumType>( m_EnumMapping );
                SCRIPT_PARSING_ERROR( L, "Unknown flag (\"", CurrFlagName, "\") provided for parameter ", m_MemberName, ". Only the following flags are allowed:\n", AllowableValues );
                return 0;
            }
        }

        const Char *m_MemberName;
        const EnumMapping<EnumType> &m_EnumMapping;
    };
#define DEFINE_FLAGS_BINDER(BindingsMap, Struct, Member, type, EnumMapping ) \
    {\
        auto *pNewBinder = new FlagsLoader<type>( offsetof( Struct, Member ), #Member, EnumMapping ); \
        /* No need to make a copy of #Member since it is constant string.  */                          \
        /* HashMapStringKey will simply keep pointer to it                 */                          \
        BindingsMap.insert( std::make_pair( #Member, std::unique_ptr<MemberBinderBase>(pNewBinder) ) ); \
    }


    template<typename FieldType>
    void SetTableField( lua_State *L, const char *FieldName, int TableStackIndex, FieldType Value )
    {
        INIT_LUA_STACK_TRACKING( L );

        lua_pushstring( L, FieldName );   // -0 | +1 -> +1
        PushValue<FieldType>( L, Value ); // -0 | +1 -> +1

        // lua_settable() does the equivalent to t[k] = v, where t is the value at the given index, 
        // v is the value at the top of the stack (-1), and k is the value just below the top (-2)
        // The function pops both the key and the value from the stack. As in Lua, this function may 
        // trigger a metamethod for the "newindex" event
        lua_settable( L, TableStackIndex - 2 ); // -2 | +0 -> -2

        CHECK_LUA_STACK_HEIGHT();
    }

    class NumericArrayLoader
    {
        public:
            NumericArrayLoader();
            void LoadArray( lua_State *L, int StackIndex, std::vector< Uint8 >& RawData );
        private:
            EnumMapping<VALUE_TYPE> m_ValueTypeEnumMapping;
            EnumMemberBinder<VALUE_TYPE> m_ValueTypeBinder;
            typedef void (*ParseNumericArrayFuncType)(lua_State *L, int StackIndex, std::vector< Uint8 >& RawData);
            std::unordered_map< VALUE_TYPE, ParseNumericArrayFuncType > m_ParseFuncJumpTbl;
    };


    template<typename EngineObjectType>
    class EngineObjectPtrLoader
    {
    public:
        typedef EngineObjectType ObjectType;
    };

    template<typename EngineObjectType>
    class MemberBinder< EngineObjectPtrLoader<EngineObjectType> > : public MemberBinderBase
    {
    public:
        typedef typename EngineObjectPtrLoader<EngineObjectType>::ObjectType ObjectType;

        MemberBinder( size_t MemberOffset, const std::vector<String> &_Metatables ) :
            MemberBinderBase( MemberOffset ),
            Metatables( _Metatables )
        {
        }
        
        virtual void GetValue( lua_State *L, const void* pBasePointer )
        {
            VERIFY( Metatables.size() == 1, "Ambiguous metatable" );
            const auto& MetatableName = *Metatables.begin();
            auto pObject = GetMemberByOffest<ObjectType*>( pBasePointer, m_MemberOffset );
            if( pObject )
            {
                auto ppNewObject = reinterpret_cast<ObjectType**>(lua_newuserdata( L, sizeof( ObjectType* ) ));
                *ppNewObject = pObject;
                pObject->AddRef();
                // Push onto the stack the metatable associated with name given in the registry
                luaL_getmetatable( L, MetatableName.c_str() );                  // -0 | +1 -> +1
                // Pop a table from the top of the stack and set it as the new metatable 
                // for the value at the given index (which is where the new user datum is)
                lua_setmetatable( L, -2 );                                      // -1 | +0 -> -1
            }
            else
            {
                lua_pushnil(L);
            }
        }

        virtual void SetValue( lua_State *L, int Index, void* pBasePointer )
        {
            ObjectType *pObject = nullptr;
            pObject = *GetUserData<ObjectType**>( L, -1, Metatables );
            GetMemberByOffest<ObjectType*>( pBasePointer, m_MemberOffset ) = pObject;
        }
    private:
        BindingsMapType Bindings;
        std::vector<String> Metatables;
    };

    // None of the graphics engine structures (such as BufferDesc or TextureDesc)
    // provide storage for strings, but only contain const Char* pointers.
    // The script wraps every such structure to provide the storage
    // This binder handles such strings
    class BufferedStringBinder : public MemberBinderBase
    {
    public:
        BufferedStringBinder( 
                size_t StringPtrOffset, // This is the offset of the const Char* pointer
                size_t StringBuffOffset // This is the offset of the buffer that contains string data
            ) :
            MemberBinderBase( StringBuffOffset ),
            m_StringPtrOffset( StringPtrOffset ) 
        {
        }

        virtual void GetValue( lua_State *L, const void* pBasePointer )
        {
            // Always use const Char* pointer to push value to Lua as there 
            // might be no valid buffer
            auto *StringPtr = GetMemberByOffest<const Char*>( pBasePointer, m_StringPtrOffset );
            PushValue<const Char*>( L, StringPtr );
        }

        virtual void SetValue( lua_State *L, int Index, void* pBasePointer )
        {
            // When reading the string from Lua, we need to make
            // sure that the string pointer contains the right data
            auto SrcString = ReadValueFromLua<String>( L, Index );
            // Set the const Char* pointer
            auto &DstStrBuff = GetMemberByOffest<String>( pBasePointer, m_MemberOffset );
            auto &DstStringPtr = GetMemberByOffest<const Char*>( pBasePointer, m_StringPtrOffset );
            DstStrBuff = SrcString;
            DstStringPtr = DstStrBuff.data();
        }
    protected:
        size_t m_StringPtrOffset;
    };

#define DEFINE_BUFFERED_STRING_BINDER(BindingsMap, Struct, StringPtr, StringBuffer) \
    {\
        auto *pNewBinder = new BufferedStringBinder( offsetof( Struct, StringPtr ), offsetof( Struct, StringBuffer ) ); \
        /* No need to make a copy of #Member since it is constant string.  */                               \
        /* HashMapStringKey will simply keep pointer to it                 */                               \
        BindingsMap.insert( std::make_pair( #StringPtr, std::unique_ptr<MemberBinderBase>(pNewBinder) ) ); \
    }

    // Object description wrapper that provides storage for
    // the Name field
    template<typename ObjectDescType>
    struct ObjectDescWrapper : ObjectDescType
    {
        String NameBuffer;
    };
}
