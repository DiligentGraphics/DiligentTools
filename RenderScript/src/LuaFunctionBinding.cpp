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
#include "LuaFunctionBinding.h"
#include "LuaBindings.h"

namespace Diligent
{
    void LuaFunctionCallerBase::PushFuncStub( lua_State *L, Bool Arg )
    {
        PushValue( L, Arg );
    }

    void LuaFunctionCallerBase::PushFuncStub( lua_State *L, Int32 Arg )
    {
        PushValue<double>( L, Arg );
    }

    void LuaFunctionCallerBase::PushFuncStub( lua_State *L, Uint32 Arg )
    {
        PushValue<double>( L, Arg );
    }

    void LuaFunctionCallerBase::PushFuncStub( lua_State *L, Int16 Arg )
    {
        PushValue<double>( L, Arg );
    }

    void LuaFunctionCallerBase::PushFuncStub( lua_State *L, Uint16 Arg )
    {
        PushValue<double>( L, Arg );
    }

    void LuaFunctionCallerBase::PushFuncStub( lua_State *L, Int8 Arg )
    {
        PushValue<double>( L, Arg );
    }

    void LuaFunctionCallerBase::PushFuncStub( lua_State *L, Uint8 Arg )
    {
        PushValue<double>( L, Arg );
    }

    void LuaFunctionCallerBase::PushFuncStub( lua_State *L, float Arg )
    {
        PushValue<double>( L, Arg );
    }

    void LuaFunctionCallerBase::PushFuncStub( lua_State *L, const Char *Arg )
    {
        PushValue( L, Arg );
    }

    void LuaFunctionCallerBase::PushFuncStub( lua_State *L, const String &Arg )
    {
        PushValue( L, Arg.c_str() );
    }

    void LuaFunctionCallerBase::Run_internal( int NumArgs, const Char *FuncName )
    {
        // http://www.lua.org/pil/25.2.html

        if( FuncName )
        {
            // Push onto the stack the value of the global name.
            lua_getglobal( m_LuaState, FuncName );
            // All arguments are now below the function, while we need to have
            // the function below all the arguments:
            lua_insert( m_LuaState, -NumArgs - 1 );
        }
        else
        {
            // If FuncName == 0, it is assumed that the chunk was previously loaded into the stack
            auto StackTop = lua_gettop( m_LuaState );
            if( StackTop < 1 )
                LOG_ERROR( "Lua stack is empty. Load Lua chunk before calling Run()" );
        }
        // do the call (NumArgs arguments, 0 results)
        // lua_pcall() pops the function and all its arguments from the stack and 
        // pushes all the results (if any)
        // In case of errors, the error message is pushed onto the top of the stack 
        if( lua_pcall( m_LuaState, NumArgs, 0, 0 ) != 0 )
        {
            auto ErrorMsg = lua_tostring( m_LuaState, -1 );
            LOG_ERROR_AND_THROW( "Failed to call function \"", FuncName ? FuncName : "<main chunk>", "\"\n"
                "The following error occured:\n", ErrorMsg );
        }
    }
}
