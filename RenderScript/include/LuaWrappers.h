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

#pragma once

#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
#include "BasicTypes.h"

namespace Diligent
{
    class LuaState
    {
    public:
        enum
        {
            LUA_LIB_BASE        = 0x001,
            LUA_LIB_PACKAGE     = 0x002,
            LUA_LIB_COROUTINE   = 0x004,
            LUA_LIB_TABLE       = 0x008,
            LUA_LIB_IO          = 0x010,
            LUA_LIB_OS          = 0x020,
            LUA_LIB_STRING      = 0x040,
            LUA_LIB_BIT32       = 0x080,
            LUA_LIB_MATH        = 0x100,
            LUA_LIB_DEBUG       = 0x200
        };
        LuaState( Uint32 OpenLibFlags = static_cast<Uint32>(-1) );
        ~LuaState();
        void Close();

        operator lua_State *();

    private:
        lua_State *m_pLuaState;
    };

}