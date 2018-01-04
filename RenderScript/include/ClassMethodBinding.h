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

#pragma once

namespace Diligent
{
    template<typename OwnerClassType>
    class ClassMethodCaller
    {
    public:
        typedef int(OwnerClassType::*MemberFunctionType)(lua_State *LuaState);

        ClassMethodCaller( OwnerClassType *pOwner,
                            lua_State *L,
                            const Char *LuaTableName,
                            const Char *LuaFunctionName,
                            MemberFunctionType MemberFunction) :
            m_MemberFunction( MemberFunction )
        {
            INIT_LUA_STACK_TRACKING( L );

            lua_getglobal( L, LuaTableName );       // -0 | +1 -> +1
            if( lua_type( L, -1 ) == LUA_TNIL )
            {
                // Global name is not found, try to find metatable with that name
                // Pop nil from the stack
                lua_pop( L, 1 );                        // -1 | +0 -> -1
                luaL_getmetatable( L, LuaTableName );   // -0 | +1 -> +1
            }
            CheckType( L, -1, LUA_TTABLE );
            lua_pushstring( L, LuaFunctionName );   // -0 | +1 -> +1
            lua_pushlightuserdata( L, pOwner );     // -0 | +1 -> +1
            lua_pushlightuserdata( L, this );       // -0 | +1 -> +1
            lua_pushcclosure( L, LuaEntry, 2 );     // -2 | +0 -> -2
            lua_settable( L, -3 );                  // -1 | +0 -> -1
            // Pop table
            lua_pop( L, 1 );                        // -1 | +0 -> -1

            CHECK_LUA_STACK_HEIGHT();
        };

    private:
        MemberFunctionType m_MemberFunction;

        static int LuaEntry( lua_State *L )
        {
            auto pOwner = static_cast<OwnerClassType*>(lua_touserdata( L, lua_upvalueindex( 1 ) ));
            VERIFY( pOwner, "Owner pointer is null" );

            auto pThis = static_cast<ClassMethodCaller*>(lua_touserdata( L, lua_upvalueindex( 2 ) ));
            VERIFY( pThis, "This pointer is null" );

            if( pOwner && pThis )
            {
                auto MemberFunction = pThis->m_MemberFunction;

                // A C function returns an integer with the number of values it is returning in Lua. 
                // Therefore, the function does not need to clear the stack before pushing its results. 
                // After it returns, Lua automatically removes whatever is in the stack below the results.
                return (pOwner->*MemberFunction)(L);
            }
            else
                return 0;
        }
    };
}
