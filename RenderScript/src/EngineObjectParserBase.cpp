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
#include "EngineObjectParserBase.h"
#include "ScriptParser.h"

using namespace Diligent;

namespace Diligent
{
    EngineObjectParserBase::EngineObjectParserBase( IRenderDevice *pRenderDevice, lua_State *L, const Char *LibName ) :
        m_pRenderDevice( pRenderDevice ),
        m_LibName( LibName ),
        m_MetatableRegistryName( String( "Metatables." ) + m_LibName )
    {
        // TODO: remove this function, move everything to ctor
        RegisterTable( L );
    }

    int EngineObjectParserBase::LuaCreate( lua_State *L )
    {
        INIT_LUA_STACK_TRACKING( L );

        EngineObjectParserBase *This = static_cast<EngineObjectParserBase *>(lua_touserdata( L, lua_upvalueindex( 1 ) ));
        // This pointer cannot be null because we stored it in an up value when created the library
        VERIFY( This, "This pointer is null" );
        if( !This )return 0;

        try
        {
            This->CreateObj( L );
        }
        catch( const std::runtime_error &err )
        {
            SCRIPT_PARSING_ERROR( L, "Failed to create ", This->m_LibName, " object: \n", err.what() );
        }

        // Push onto the stack the metatable associated with name given in the registry
        luaL_getmetatable( L, This->m_MetatableRegistryName.c_str() );  // -0 | +1 -> +1
        // Pop a table from the top of the stack and set it as the new metatable 
        // for the value at the given index (which is where the new user datum is)
        lua_setmetatable( L, -2 );                                      // -1 | +0 -> -1

        CHECK_LUA_STACK_HEIGHT( +1 );

        // Return number of return arguments
        // New userdatum is on the top of the stack
        return 1;
    }

    int EngineObjectParserBase::LuaGC( lua_State *L )
    {
        INIT_LUA_STACK_TRACKING( L );

        EngineObjectParserBase *This = static_cast<EngineObjectParserBase *>(lua_touserdata( L, lua_upvalueindex( 1 ) ));
        // This pointer cannot be null because we stored it in an up value when created the library
        VERIFY( This, "This pointer is null" );
        if( !This )return 0;

        // Do not throw exception as this function can be called from dtor!
        //auto pData = GetUserData<void*>( L, 1, This->m_MetatableRegistryName.c_str() );
        auto pData = reinterpret_cast<void*>(luaL_testudata( L, 1, This->m_MetatableRegistryName.c_str() ));
        if( pData )
        {
            This->DestroyObj( pData );
        }

        CHECK_LUA_STACK_HEIGHT();

        return 0;
    }

    int EngineObjectParserBase::LuaIndex( lua_State *L )
    {
        INIT_LUA_STACK_TRACKING( L );

        // Whenever Lua calls C, the called function gets a new stack, which is independent 
        // of previous stacks and of stacks of C functions that are still active. This stack 
        // initially contains any arguments to the C function and it is where the C function 
        // pushes its results to be returned to the calle


        // Note that the syntax var.Name is just syntactic sugar for var["Name"]
        // The first parameter to the __index() function is the object and the second is 
        // the index

        EngineObjectParserBase *This = static_cast<EngineObjectParserBase *>(lua_touserdata( L, lua_upvalueindex( 1 ) ));
        // This pointer cannot be null because we stored it in an up value when created the library
        VERIFY( This, "This pointer is null" );
        if( !This )return 0;

        auto pData = GetUserData<void*>( L, 1, This->m_MetatableRegistryName.c_str() );
        auto Field = ReadValueFromLua<const Char*>( L, 2 );

        // First try to find the field in the metatable
        // Push metatable onto the stack
        luaL_getmetatable( L, This->m_MetatableRegistryName.c_str() ); // -0 | +1 -> +1
        // Duplicate key on the top of the stack
        lua_pushvalue( L, -2 );                     // -0 | +1 -> +1
        // Use rawget to avoid calling __index
        lua_rawget( L, -2 );                        // -1 | +1 -> +0
        // Remove metatable from the stack
        lua_remove( L, -2 );                        // -1 | +0 -> -1
        if( lua_type( L, -1 ) == LUA_TNIL )
        {
            // Pop nil from the stack
            lua_pop( L, 1 );                        // -1 | +0 -> -1
            This->ReadField( L, pData, Field );
        }
        else
        {
            // Value is on top of the stack
        }

        CHECK_LUA_STACK_HEIGHT( +1 );

        return 1;
    }

    void EngineObjectParserBase::UpdateField( lua_State *L, void *pData, const Char *Field )
    {
        SCRIPT_PARSING_ERROR( L, "Attempting to update \"", Field, "\" field of a read-only object \"", m_LibName.c_str(), '\"' );
    }

    int EngineObjectParserBase::LuaNewIndex( lua_State *L )
    {
        INIT_LUA_STACK_TRACKING( L );

        EngineObjectParserBase *This = static_cast<EngineObjectParserBase *>(lua_touserdata( L, lua_upvalueindex( 1 ) ));
        // This pointer cannot be null because we stored it in an up value when created the library
        VERIFY( This, "This pointer is null" );
        if( !This )return 0;

        auto pData = GetUserData<void*>( L, 1, This->m_MetatableRegistryName.c_str() );
        auto Field = ReadValueFromLua<const Char*>( L, 2 );
        This->UpdateField( L, pData, Field );

        CHECK_LUA_STACK_HEIGHT();

        return 0;
    }

    void EngineObjectParserBase::RegisterTable( lua_State *L )
    {
        INIT_LUA_STACK_TRACKING( L );

        // If the registry already has the key with the same name, luaL_newmetatable() returns 0. 
        // Otherwise, it creates a new table to be used as a metatable for userdata, adds it to the 
        // registry with key tname, and returns 1. In both cases it pushes onto the stack the final 
        // value associated with tname in the registry.
        auto Created = luaL_newmetatable( L, m_MetatableRegistryName.c_str() );     // -0 | +1 -> +1
        VERIFY( Created, "Metatble with the same name already registered!" ); (void)Created;

        // http://lua-users.org/wiki/MetatableEvents
        luaL_Reg MetaMethods[] = {
            // An object is marked for finalization when its metatable is set and the metatable 
            // has a field indexed by the string "__gc". NOTE: if a metatable without a __gc field 
            // is set and that field is only later added to the metatable, the object will NOT be marked 
            // for finalization.
            { "__gc", LuaGC },
            { "__index", LuaIndex },
            { "__newindex", LuaNewIndex },
            { NULL, NULL }
        };

        lua_pushlightuserdata( L, this );   // -0 | +1 -> +1

        // Register all functions in the array into the table on the top of the stack
        // When nup (last parameter) is not zero, all functions are created sharing nup upvalues,
        // which must be previously pushed on the stack on top of the library table. 
        // These values are popped from the stack after the registration.
        luaL_setfuncs( L, MetaMethods, 1 ); // -1 | +0 -> -1

        // luaL_setfuncs() does the following for every function in the list:
        //lua_pushstring( L, FuncName );
        //lua_pushlightuserdata( L, this );
        //lua_pushcclosure( L, Function, 1 );
        //lua_settable( L, -3 );

        // Protect metatable from tampering in the script
        // If __metatable field is set, then getmetatable() function returns
        // what is stored in this field instead of the actual metatable
        // and setmetatable() is not allowed to access it
        lua_pushliteral( L, "__metatable" );                    // -0 | +1 -> +1
        lua_pushliteral( L, "Metatable is not accessible!" );   // -0 | +1 -> +1
        // Note that lua_settable() may trigger a metamethod 
        // for the "newindex" event  
        lua_settable( L, -3 );                                  // -2 | +0 -> -2

        // Pop metatable
        lua_pop( L, 1 );                                        // -1 | +0 -> -1

        CHECK_LUA_STACK_HEIGHT();

        luaL_Reg Methods[] = {
            { "Create", LuaCreate },
            { NULL, NULL }
        };

        // Create a new table with a size optimized to store all entries in the array Methods
        // (but does not actually store them).
        luaL_newlibtable( L, Methods );         // -0 | +1 -> +1

        // Push pointer onto the stack
        lua_pushlightuserdata( L, this );       // -0 | +1 -> +1

        // Register all functions in the array into the table on the top of the stack
        // When nup (last parameter) is not zero, all functions are created sharing nup upvalues,
        // which must be previously pushed on the stack on top of the library table. 
        // These values are popped from the stack after the registration.
        luaL_setfuncs( L, Methods, 1 );         // -1 | +0 -> -1

        // Pop a value from the stack and set it as the new value of global name.
        lua_setglobal( L, m_LibName.c_str() );  // -1 | +0 -> -1

        CHECK_LUA_STACK_HEIGHT();
    }

    void EngineObjectParserBase::PushObject( lua_State *L, const void *pData )
    {
        INIT_LUA_STACK_TRACKING( L );

        if( pData )
            PushExistingObject( L, pData );
        else
            lua_pushnil( L );

        luaL_getmetatable( L, m_MetatableRegistryName.c_str() );  // -0 | +1 -> +1
        lua_setmetatable( L, -2 );                                // -1 | +0 -> -1

        CHECK_LUA_STACK_HEIGHT( +1 );
    }

    IDeviceContext* EngineObjectParserBase::LoadDeviceContextFromRegistry( lua_State *L )
    {
        INIT_LUA_STACK_TRACKING( L );

        lua_pushstring( L, ScriptParser::DeviceContextRegistryKey );  // -0 | +1 -> +1
        lua_gettable(L, LUA_REGISTRYINDEX );                           // -1 | +1 -> +0
        CheckType( L, -1, LUA_TLIGHTUSERDATA );
        IDeviceContext* pContext = reinterpret_cast<IDeviceContext*>( lua_touserdata(L, -1) );
        lua_pop( L, 1 );                                               // -1 | +0 -> -1
        
        VERIFY( pContext != nullptr, "Device context is null" );

        CHECK_LUA_STACK_HEIGHT();

        return pContext;
    }
}
