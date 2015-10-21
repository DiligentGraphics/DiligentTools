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

#include "EngineObjectParserBase.h"

namespace Diligent
{
    template<typename ObjectType>
    class EngineObjectParserCommon  : public EngineObjectParserBase
    {
    public:
        EngineObjectParserCommon( IRenderDevice *pRenderDevice, lua_State *L, const Char *LibName ) :
            EngineObjectParserBase( pRenderDevice, L, LibName )
        {}

        virtual void GetObjectByName( lua_State *L, const Char *ShaderName, ObjectType** ppObject )
        {
            auto pObject = *GetGlobalObject<ObjectType**>( L, ShaderName, m_MetatableRegistryName.c_str() );
            *ppObject = pObject;
            pObject->AddRef();
        }

    protected:
        virtual void PushExistingObject( lua_State *L, const void *pObject )
        {
            auto ppObject = reinterpret_cast<ObjectType**>(lua_newuserdata( L, sizeof( ObjectType* ) ));
            *ppObject = reinterpret_cast<ObjectType*>(const_cast<void*>(pObject));
            (*ppObject)->AddRef();
        }

        virtual void DestroyObj( void *pData )
        {
            if( pData != nullptr )
            {
                auto ppObject = reinterpret_cast<ObjectType**>(pData);
                if( *ppObject != nullptr )
                 (*ppObject)->Release();
            }
        }

        virtual void ReadField( lua_State *L, void *pData, const Char *Field )
        {
            auto pObject = *reinterpret_cast<ObjectType**>(pData);
            const auto &Desc = pObject->GetDesc();
            PushField( L, &Desc, Field, m_Bindings );
        }
    };
}
