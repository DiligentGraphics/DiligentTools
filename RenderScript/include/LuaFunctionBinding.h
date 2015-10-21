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

#pragma once

namespace Diligent
{
    class LuaFunctionCallerBase
    {
    public:
        LuaFunctionCallerBase( lua_State *LuaState = nullptr ) :
            m_LuaState( LuaState )
        {
        };

        void SetLuaState( lua_State *LuaState )
        {
            m_LuaState = LuaState;
        }
        
        // We need these stubs to hide implementation details
        void PushFuncStub( lua_State *L, Bool Arg );
        void PushFuncStub( lua_State *L, Int32 Arg );
        void PushFuncStub( lua_State *L, Uint32 Arg );
        void PushFuncStub( lua_State *L, Int16 Arg );
        void PushFuncStub( lua_State *L, Uint16 Arg );
        void PushFuncStub( lua_State *L, Int8 Arg );
        void PushFuncStub( lua_State *L, Uint8 Arg );
        void PushFuncStub( lua_State *L, float Arg );
        void PushFuncStub( lua_State *L, const Char *Arg );
        void PushFuncStub( lua_State *L, const String &Arg );

    protected:
        void Run_internal( int NumArgs, const Char *FuncName );

        lua_State *m_LuaState;
    };

    class DummyPushFuncs
    {
    public:
        void PushFuncStub();
    };

    template<typename AdditionalPushFuncs = DummyPushFuncs>
    class LuaFunctionCaller : public LuaFunctionCallerBase, public AdditionalPushFuncs
    {
    public:
        void operator()()
        {
            operator()( nullptr );
        }

        template<typename... ArgsType>
        void operator()( const Char *FuncName, const ArgsType&... Args )
        {
            Run_internal( 0, FuncName, Args... );
        }

        // We need to make visible these parent-class definitions here
        using LuaFunctionCallerBase::PushFuncStub;
        using LuaFunctionCallerBase::Run_internal;
        using AdditionalPushFuncs::PushFuncStub;

    protected:
        template<typename ArgType>
        void Run_internal( int NumArgs, const Char *FuncName, const ArgType &Arg )
        {
            PushFuncStub( m_LuaState, Arg );
            Run_internal( NumArgs + 1, FuncName );
        }

        template<typename ArgType, typename... RestArgsType>
        void Run_internal( int NumArgs, const Char *FuncName, const ArgType &Arg, const RestArgsType&... RestArgs )
        {
            PushFuncStub( m_LuaState, Arg );
            Run_internal( NumArgs + 1, FuncName, RestArgs... ); // recursive call using pack expansion syntax
        }
    };
}
