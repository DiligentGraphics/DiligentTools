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

#pragma once

#include "LuaWrappers.h"
#include "LuaBindings.h"

namespace Diligent
{
    class EngineObjectParserBase
    {
    public:
        EngineObjectParserBase( IRenderDevice *pRenderDevice, lua_State *L, const Char *LibName );
        void PushObject( lua_State *L, const void *pData );
        const String &GetMetatableName(){ return m_MetatableRegistryName; }

        static IDeviceContext* LoadDeviceContextFromRegistry( lua_State *L );

    protected:
        void RegisterTable( lua_State *L );

        static int LuaCreate( lua_State *L );
        virtual void CreateObj( lua_State *L ) = 0;

        static int LuaGC( lua_State *L );
        virtual void DestroyObj( void *pData ) = 0;

        static int LuaIndex( lua_State *L );
        virtual void ReadField( lua_State *L, void *pData, const Char *Field ) = 0;

        static int LuaNewIndex( lua_State *L );
        virtual void UpdateField( lua_State *L, void *pData, const Char *Field );

        virtual void PushExistingObject( lua_State *L, const void *pObject ) = 0;

        Diligent::RefCntAutoPtr<IRenderDevice> m_pRenderDevice;
        BindingsMapType m_Bindings;
        const String m_LibName;
        const String m_MetatableRegistryName;
    };
}
