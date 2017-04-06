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

#include "pch.h"
#include "ShaderVariableParser.h"

namespace Diligent
{
    const Char* ShaderVariableParser::ShaderVariableLibName = "ShaderVariable";

    ShaderVariableParser::ShaderVariableParser( IRenderDevice *pRenderDevice, lua_State *L, 
                                                  const String &ShaderLibMetatableName,
                                                  const String &BufferLibMetatableName,
                                                  const String &BufferViewLibMetatableName,
                                                  const String &TexViewMetatableName ) :
        EngineObjectParserBase( pRenderDevice, L, ShaderVariableLibName ),
        m_ShaderLibMetatableName(ShaderLibMetatableName),
        m_BufferLibMetatableName(BufferLibMetatableName),
        m_BufferViewLibMetatableName(BufferViewLibMetatableName),
        m_TexViewMetatableName(TexViewMetatableName),
        m_SetBinding( this, L, m_MetatableRegistryName.c_str(), "Set", &ShaderVariableParser::Set ),
        m_GetShaderVariableBinding( this, L, m_ShaderLibMetatableName.c_str(), "GetShaderVariable", &ShaderVariableParser::GetShaderVariable )
    {
    };

    void ShaderVariableParser::CreateObj( lua_State *L )
    {
        INIT_LUA_STACK_TRACKING(L);

        // Shader should be the first argument
        auto *pShader = *GetUserData<IShader**>( L, 1, m_ShaderLibMetatableName.c_str() );
        
        // Variable name should be the second argument
        auto VarName = ReadValueFromLua<String>( L, 2 );

        auto pVar = pShader->GetShaderVariable( VarName.c_str() );

        auto pNewShaderVarLuaObj = reinterpret_cast<IShaderVariable**>(lua_newuserdata( L, sizeof( IShaderVariable* ) ));
        *pNewShaderVarLuaObj = pVar;
        pVar->AddRef();

        CHECK_LUA_STACK_HEIGHT( +1 );
    }

    void ShaderVariableParser::DestroyObj( void *pData )
    {
        if( pData != nullptr )
        {
            auto ppShaderVar = reinterpret_cast<IShaderVariable**>(pData);
            if( *ppShaderVar != nullptr )
                (*ppShaderVar)->Release();
        }
    }

    void ShaderVariableParser::ReadField( lua_State *L, void *pData, const Char *Field )
    {
        SCRIPT_PARSING_ERROR(L, "Shader variables have no fields that can be read")
    }

    void ShaderVariableParser::UpdateField( lua_State *L, void *pData, const Char *Field )
    {
        SCRIPT_PARSING_ERROR(L, "Shader variables have no fields that can be updated")
    }

    void ShaderVariableParser::PushExistingObject( lua_State *L, const void *pObject )
    {
        auto pShaderVariable = reinterpret_cast<IShaderVariable**>(lua_newuserdata( L, sizeof( IShaderVariable* ) ));
        *pShaderVariable = reinterpret_cast<IShaderVariable*>( const_cast<void*>(pObject) );
        (*pShaderVariable)->AddRef();
    }

    int ShaderVariableParser::Set( lua_State *L )
    {
        // Shader variable is the first argument
        auto *pShaderVar = *GetUserData<IShaderVariable**>( L, 1, m_MetatableRegistryName.c_str() );

        IDeviceObject *pObject = nullptr;
        auto ArgType = lua_type( L, 2 );
        if( ArgType == LUA_TUSERDATA )
        {
            auto ppObject = reinterpret_cast<IDeviceObject**>( luaL_testudata( L, 2, m_BufferLibMetatableName.c_str() ) );
            if( !ppObject )
                ppObject = reinterpret_cast<IDeviceObject**>( luaL_testudata( L, 2, m_BufferViewLibMetatableName.c_str() ) );
            if( !ppObject )
                ppObject = reinterpret_cast<IDeviceObject**>( luaL_testudata( L, 2, m_TexViewMetatableName.c_str() ) );
            if( ppObject )
            {
                pObject = *ppObject;
            }
            else
            {
                SCRIPT_PARSING_ERROR(L, "Set() function expects buffer, buffer view or texture view as an argument")
            }
        }
        else if( ArgType == LUA_TNIL )
        {
            pObject = nullptr;
        }
        else
        {
            SCRIPT_PARSING_ERROR(L, "Set() function expects user data or nil")
        }

        pShaderVar->Set( pObject );

        return 0;
    }

    int ShaderVariableParser::GetShaderVariable( lua_State *L )
    {
        return LuaCreate(L);
    }

    void ShaderVariableParser::GetObjectByName( lua_State *L, const Char *ShaderName, IShaderVariable** ppObject )
    {
        auto pObject = *GetGlobalObject<IShaderVariable**>( L, ShaderName, m_MetatableRegistryName.c_str() );
        *ppObject = pObject;
        pObject->AddRef();
    }
}
