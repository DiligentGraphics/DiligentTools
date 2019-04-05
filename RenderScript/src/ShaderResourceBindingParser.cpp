/*     Copyright 2015-2019 Egor Yusov
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
#include "ShaderResourceBindingParser.h"

namespace Diligent
{
    const Char* ShaderResourceBindingParser::ShaderResourceBindingLibName = "ShaderResourceBinding";

    ShaderResourceBindingParser::ShaderResourceBindingParser( IRenderDevice *pRenderDevice, lua_State *L,
                                                              const String &PSOLibMetatableName,
                                                              const String &ResMappingMetatableName,
                                                              const String &ShaderVarMetatableRegistryName ) :
        EngineObjectParserBase( pRenderDevice, L, ShaderResourceBindingLibName ),
        m_PSOLibMetatableName(PSOLibMetatableName),
        m_ResMappingMetatableName(ResMappingMetatableName),
        m_ShaderVarMetatableRegistryName(ShaderVarMetatableRegistryName),
        m_BindResourcesBinding( this, L, m_MetatableRegistryName.c_str(), "BindResources", &ShaderResourceBindingParser::BindResources ),
        m_GetVariableByNameBinding( this, L, m_MetatableRegistryName.c_str(), "GetVariableByName", &ShaderResourceBindingParser::GetVariable ),
        m_GetVariableByIndexBinding( this, L, m_MetatableRegistryName.c_str(), "GetVariableByIndex", &ShaderResourceBindingParser::GetVariable ),
        m_CreateShaderResourceBinding( this, L, PSOLibMetatableName.c_str(), "CreateShaderResourceBinding", &ShaderResourceBindingParser::CreateShaderResourceBinding ),
        m_InitializeStaticResourcesBinding( this, L, m_MetatableRegistryName.c_str(), "InitializeStaticResources", &ShaderResourceBindingParser::InitializeStaticResources )
    {

    }


    void ShaderResourceBindingParser::CreateObj( lua_State *L )
    {
        INIT_LUA_STACK_TRACKING(L);

        // PSO should be the first argument
        auto *pPSO = *GetUserData<IPipelineState**>( L, 1, m_PSOLibMetatableName.c_str() );
        
        bool InitStaticResources = false;
        auto NumArgs = lua_gettop( L );
        if( NumArgs >= 2 )
        {
            InitStaticResources = ReadValueFromLua<Bool>( L, 2 );
        }

        auto pNewShaderResBndngLuaObj = reinterpret_cast<IShaderResourceBinding**>(lua_newuserdata( L, sizeof( IShaderResourceBinding* ) ));
        pPSO->CreateShaderResourceBinding(pNewShaderResBndngLuaObj, InitStaticResources);

        CHECK_LUA_STACK_HEIGHT( +1 );
    }

    void ShaderResourceBindingParser::DestroyObj( void *pData )
    {
        if( pData != nullptr )
        {
            auto ppShaderResBinding = reinterpret_cast<IShaderResourceBinding**>(pData);
            if( *ppShaderResBinding != nullptr )
                (*ppShaderResBinding)->Release();
        }
    }

    void ShaderResourceBindingParser::ReadField( lua_State *L, void *pData, const Char *Field )
    {
        SCRIPT_PARSING_ERROR(L, "Shader resource binding have no fields that can be read")
    }

    void ShaderResourceBindingParser::UpdateField( lua_State *L, void *pData, const Char *Field )
    {
        SCRIPT_PARSING_ERROR(L, "Shader resource binding have no fields that can be updated")
    }


    void ShaderResourceBindingParser::PushExistingObject( lua_State *L, const void *pObject )
    {
        auto pShaderResourceBinding = reinterpret_cast<IShaderResourceBinding**>(lua_newuserdata( L, sizeof( IShaderResourceBinding* ) ));
        *pShaderResourceBinding = reinterpret_cast<IShaderResourceBinding*>( const_cast<void*>(pObject) );
        (*pShaderResourceBinding)->AddRef();
    }


    int ShaderResourceBindingParser::CreateShaderResourceBinding( lua_State *L )
    {
        return LuaCreate(L);
    }

    int ShaderResourceBindingParser::BindResources( lua_State *L )
    {
        try
        {
            auto NumArgs = lua_gettop( L );
            if( NumArgs < 3 )
            {
                SCRIPT_PARSING_ERROR( L, "At least 2 arguments (shader flags and resource mapping) are expected" );
            }
            
            
            int ArgStackInd = 1;

            auto *pResBinding = *GetUserData<IShaderResourceBinding**>( L, ArgStackInd, m_MetatableRegistryName.c_str() );
            VERIFY( pResBinding, "Resource mapping pointer is null" );
            if( !pResBinding )return 0;

            ++ArgStackInd;
            Uint32 ShaderFlags = 0;
            {
                FlagsLoader<SHADER_TYPE> FlagsLoader(0, "BindShaderResourceFlags", m_ShaderTypeEnumMapping);
                FlagsLoader.SetValue( L, ArgStackInd, &ShaderFlags );
            }

            ++ArgStackInd;
            auto *pResourceMapping = *GetUserData<IResourceMapping**>( L, ArgStackInd, m_ResMappingMetatableName.c_str() );
            if( !pResourceMapping )
            {
                SCRIPT_PARSING_ERROR( L, "Incorrect 2nd argument type: resource mapping is expected" );
            }

            ++ArgStackInd;
            Uint32 Flags = 0;
            // The last argument may be flags
            if( NumArgs >= ArgStackInd &&
                (lua_type( L, ArgStackInd ) == LUA_TSTRING || lua_type( L, ArgStackInd ) == LUA_TTABLE ) )
            {
                FlagsLoader<BIND_SHADER_RESOURCES_FLAGS> FlagsLoader(0, "BindShaderResourceFlags", m_BindShaderResFlagEnumMapping);
                FlagsLoader.SetValue( L, ArgStackInd, &Flags );
            }

            pResBinding->BindResources( ShaderFlags, pResourceMapping, Flags );
        }
        catch( const std::runtime_error& )
        {

        }
        return 0;
    }

    int ShaderResourceBindingParser::GetVariable( lua_State *L )
    {
        auto NumArgs = lua_gettop( L );
        if( NumArgs < 3 )
        {
            SCRIPT_PARSING_ERROR( L, "2 arguments (shader type and variable name) are expected" );
        }

        INIT_LUA_STACK_TRACKING(L);

        int ArgStackInd = 1;

        // The object itself goes first
        auto *pShaderResBinding = *GetUserData<IShaderResourceBinding**>( L, ArgStackInd, m_MetatableRegistryName.c_str() );
        
        // Shader type should be the first argument
        ++ArgStackInd;
        SHADER_TYPE ShaderType = SHADER_TYPE_UNKNOWN;
        EnumMemberBinder<SHADER_TYPE> ShaderTypeParser(0, "ShaderType", m_ShaderTypeEnumMapping);
        ShaderTypeParser.SetValue(L, ArgStackInd, &ShaderType);

        ++ArgStackInd;
        IShaderResourceVariable* pVar = nullptr;
        if (lua_type(L,ArgStackInd) == LUA_TSTRING)
        {
            // Variable name should be the second argument
            auto VarName = ReadValueFromLua<String>( L, ArgStackInd );
            pVar = pShaderResBinding->GetVariableByName(ShaderType, VarName.c_str());
        }
        else
        {
            // Variable name should be the second argument
            auto VarIndex = ReadValueFromLua<int>( L, ArgStackInd );
            pVar = pShaderResBinding->GetVariableByIndex(ShaderType, VarIndex);
        }

        auto pNewShaderVarLuaObj = reinterpret_cast<IShaderResourceVariable**>(lua_newuserdata( L, sizeof( IShaderResourceVariable* ) ));
        *pNewShaderVarLuaObj = pVar;
        pVar->AddRef();

        // Push onto the stack the metatable associated with name given in the registry
        luaL_getmetatable( L, m_ShaderVarMetatableRegistryName.c_str() );   // -0 | +1 -> +1
        // Pop a table from the top of the stack and set it as the new metatable 
        // for the value at the given index (which is where the new user datum is)
        lua_setmetatable( L, -2 );                                          // -1 | +0 -> -1

        CHECK_LUA_STACK_HEIGHT( +1 );

        return 1;
    }

    int ShaderResourceBindingParser::InitializeStaticResources( lua_State *L )
    {
        auto NumArgs = lua_gettop( L );
        // The object itself goes first
        auto *pShaderResBinding = *GetUserData<IShaderResourceBinding**>( L, 1, m_MetatableRegistryName.c_str() );
        IPipelineState* pPSO = nullptr;

        if (NumArgs >= 2)
        {
            pPSO = *GetUserData<IPipelineState**>( L, 2, m_PSOLibMetatableName.c_str() );
        }

        pShaderResBinding->InitializeStaticResources(pPSO);
        return 0;
    }

    void ShaderResourceBindingParser::GetObjectByName( lua_State *L, const Char *ShaderName, IShaderResourceBinding** ppObject )
    {
        auto pObject = *GetGlobalObject<IShaderResourceBinding**>( L, ShaderName, m_MetatableRegistryName.c_str() );
        *ppObject = pObject;
        pObject->AddRef();
    }
}
