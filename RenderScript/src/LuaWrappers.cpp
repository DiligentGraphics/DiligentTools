/*     Copyright 2019 Diligent Graphics LLC
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
#include "LuaWrappers.h"

namespace Diligent
{
    LuaState::LuaState( Uint32 OpenLibFlags )
    {
        m_pLuaState = luaL_newstate();
        //luaL_openlibs( m_pLuaState );

        INIT_LUA_STACK_TRACKING( m_pLuaState );
        struct LuaStdLibInfo
        {
            const char *name;
            lua_CFunction func;
            Uint32 Flag;
        }LuaStdLibs[] = 
        {
                { "_G",             luaopen_base,       LUA_LIB_BASE      },
                { LUA_LOADLIBNAME,  luaopen_package,    LUA_LIB_PACKAGE   },
                { LUA_COLIBNAME,    luaopen_coroutine,  LUA_LIB_COROUTINE },
                { LUA_TABLIBNAME,   luaopen_table,      LUA_LIB_TABLE     },
                { LUA_IOLIBNAME,    luaopen_io,         LUA_LIB_IO        },
                { LUA_OSLIBNAME,    luaopen_os,         LUA_LIB_OS        },
                { LUA_STRLIBNAME,   luaopen_string,     LUA_LIB_STRING    },
                { LUA_BITLIBNAME,   luaopen_bit32,      LUA_LIB_BIT32     },
                { LUA_MATHLIBNAME,  luaopen_math,       LUA_LIB_MATH      },
                { LUA_DBLIBNAME,    luaopen_debug,      LUA_LIB_DEBUG     },
                { nullptr,          nullptr,            0                 }
        };

        for( auto lib = LuaStdLibs; lib->func; lib++ )
        {
            if( OpenLibFlags & lib->Flag )
            {
                luaL_requiref( m_pLuaState, lib->name, lib->func, 1 );  // -0 | +1 -> +1
                lua_pop( m_pLuaState, 1 );                              // -1 | +0 -> -1
            }
        }

        CHECK_LUA_STACK_HEIGHT();
    }

    LuaState::~LuaState()
    {
        Close();
    }
    
    void LuaState::Close()
    {
        if( m_pLuaState )
            lua_close( m_pLuaState );
        m_pLuaState = nullptr;
    }

    LuaState::operator lua_State *()
    {
        return m_pLuaState;
    }
}
