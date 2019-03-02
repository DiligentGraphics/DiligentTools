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
#include "ScriptParser.h"
#include "LuaBindings.h"
#include "SamplerParser.h"
#include "ShaderParser.h"
#include "BufferParser.h"
#include "TextureParser.h"
#include "DrawAttribsParser.h"
#include "FileSystem.h"
#include "ResourceMappingParser.h"
#include "TextureViewParser.h"
#include "BufferViewParser.h"
#include "PSODescParser.h"
#include "DeviceContextFuncBindings.h"
#include "ViewportParser.h"
#include "ScissorRectParser.h"
#include "ShaderVariableParser.h"
#include "ShaderResourceBindingParser.h"

namespace Diligent
{
    const Char* ScriptParser::DeviceContextRegistryKey = "DeviceContext";

#define IMPLEMENT_PUSH_FUNC_STUBS(ObjectType, ParserName) \
    void ScriptParser::SpecialPushFuncs::PushFuncStub( lua_State *L, const ObjectType* pObject)   \
    {                                                                                               \
        m_pScriptParser->ParserName->PushObject( L, pObject );                                      \
    }                                                                                               \
                                                                                                    \
    void ScriptParser::SpecialPushFuncs::PushFuncStub( lua_State *L, const RefCntAutoPtr<ObjectType> &pObject )   \
    {                                                                                               \
        PushFuncStub( L, pObject.RawPtr() );                                                        \
    }

    IMPLEMENT_PUSH_FUNC_STUBS( ISampler, m_pSamplerParser)
    IMPLEMENT_PUSH_FUNC_STUBS( IShader, m_pShaderParser )
    IMPLEMENT_PUSH_FUNC_STUBS( IBuffer, m_pBufferParser )
    IMPLEMENT_PUSH_FUNC_STUBS( ITexture, m_pTextureParser )
    IMPLEMENT_PUSH_FUNC_STUBS( IResourceMapping, m_pResourceMappingParser )
    IMPLEMENT_PUSH_FUNC_STUBS( ITextureView, m_pTextureViewParser )
    IMPLEMENT_PUSH_FUNC_STUBS( IBufferView, m_pBufferViewParser )
    IMPLEMENT_PUSH_FUNC_STUBS( IPipelineState, m_pPSOParser )
    IMPLEMENT_PUSH_FUNC_STUBS( IShaderResourceVariable, m_pShaderVariableParser )
    IMPLEMENT_PUSH_FUNC_STUBS( IShaderResourceBinding, m_pShaderResBindingParser )


    void ScriptParser::SpecialPushFuncs::PushFuncStub( lua_State *L, const DrawAttribs &DrawAttribs )
    {
        m_pScriptParser->m_pDrawAttribsParser->PushObject( L, &DrawAttribs );
    }

    void ScriptParser::SpecialPushFuncs::PushFuncStub( lua_State *L, const Viewport &Viewport )
    {
        m_pScriptParser->m_pViewportParser->PushObject( L, &Viewport );
    }

    void ScriptParser::SpecialPushFuncs::PushFuncStub( lua_State *L, const Rect &Rect )
    {
        m_pScriptParser->m_pScissorRectParser->PushObject( L, &Rect );
    }
    
    ScriptParser::ScriptParser( IReferenceCounters *pRefCounters, IRenderDevice *pRenderDevice ) :
        TBase(pRefCounters),
        m_pRenderDevice( pRenderDevice ),
        m_LuaState( LuaState::LUA_LIB_BASE | LuaState::LUA_LIB_COROUTINE | LuaState::LUA_LIB_TABLE | 
                    LuaState::LUA_LIB_STRING | LuaState::LUA_LIB_BIT32 | LuaState::LUA_LIB_MATH )
    {
        // Run's ctor is called BEFORE the m_LuaState's ctor
        // We need to explicitly init the object:
        m_RunFunctionCaller.SetLuaState( m_LuaState );
        m_RunFunctionCaller.SetScriptParser( this );

        // We need to define global constants before initializing parsers
        DefineGlobalConstants(m_LuaState);

        m_pSamplerParser.reset( new SamplerParser( pRenderDevice, m_LuaState ) );
        m_pBufferParser.reset( new BufferParser( pRenderDevice, m_LuaState ) );
        m_pTextureParser.reset( new TextureParser( pRenderDevice, m_LuaState ) );
        m_pDrawAttribsParser.reset( new DrawAttribsParser( m_pBufferParser.get(), pRenderDevice, m_LuaState ) );
        // Texture view parser must be create AFTER texture parser, because it 
        // registers CreateView function in Metatables.Texture table
        m_pTextureViewParser.reset( new TextureViewParser( m_pTextureParser.get(), m_pSamplerParser.get(), pRenderDevice, m_LuaState ) );
        m_pBufferViewParser.reset( new BufferViewParser( m_pBufferParser.get(), pRenderDevice, m_LuaState ) );
        m_pResourceMappingParser.reset( new ResourceMappingParser( pRenderDevice, m_LuaState, m_pTextureViewParser.get(), m_pBufferParser.get(), m_pBufferViewParser.get() ) );
        m_pShaderParser.reset( new ShaderParser( pRenderDevice, m_LuaState ) );
        m_pPSOParser.reset( new PSODescParser( pRenderDevice, m_LuaState, m_pResourceMappingParser->GetMetatableName() ) );
        m_pViewportParser.reset( new ViewportParser( pRenderDevice, m_LuaState ) );
        m_pScissorRectParser.reset( new ScissorRectParser( pRenderDevice, m_LuaState ) );
        m_pShaderVariableParser.reset( new ShaderVariableParser( pRenderDevice, m_LuaState, m_pPSOParser->GetMetatableName(), m_pBufferParser->GetMetatableName(), m_pBufferViewParser->GetMetatableName(), m_pTextureViewParser->GetMetatableName() ) );
        m_pShaderResBindingParser.reset( new ShaderResourceBindingParser( pRenderDevice, m_LuaState, m_pPSOParser->GetMetatableName(), m_pResourceMappingParser->GetMetatableName(), m_pShaderVariableParser->GetMetatableName() ) );
        m_pDeviceCtxFuncBindings.reset( new DeviceContextFuncBindings( pRenderDevice, m_LuaState, m_pTextureViewParser.get(), m_pShaderResBindingParser.get(), m_pPSOParser.get() ) );
    }

    ScriptParser::~ScriptParser()
    {
        // It is essentially important to close Lua first, because we need to release
        // all user data
        m_LuaState.Close();
    }

    void ScriptParser::DefineGlobalConstants( lua_State *L )
    {
        INIT_LUA_STACK_TRACKING( L );

        // Create a new empty table and push it onto the stack
        lua_newtable( L ); // -0 | +1 -> +1

        const auto &DeviceCaps = m_pRenderDevice->GetDeviceCaps();
        const Char *pDeviceStr = nullptr;
        switch(DeviceCaps.DevType)
        {
            case DeviceType::OpenGL:
                pDeviceStr = "OpenGL";
            break;
            
            case DeviceType::OpenGLES:
                pDeviceStr = "OpenGLES";
            break;

            case DeviceType::D3D11:
                pDeviceStr = "D3D11";
            break;

            case DeviceType::D3D12:
                pDeviceStr = "D3D12";
            break;

            case DeviceType::Vulkan:
                pDeviceStr = "Vulkan";
                break;

            default:
                UNEXPECTED( "Unknown device type" );
        }
        SetTableField( L, "DeviceType", -1, pDeviceStr ); // -0 | +0 -> 0

        // lua_setglobal() pops a value from the stack and sets it as the new value of global name.
        lua_setglobal( L, "Constants" ); // -1 | +0 -> -1

        CHECK_LUA_STACK_HEIGHT();

        lua_newtable( L );               // -0 | +1 -> +1
        lua_setglobal( L, "Context" );   // -1 | +0 -> -1

        CHECK_LUA_STACK_HEIGHT();
    }

    // Every C-function called by Lua sees only its own private stack, with its first argument at index 1
    // Return the number of results pushed onto the stack

    void ScriptParser::Parse( const Char *pScript )
    {
        // Load the chunck and push it onto the top of the stack.
        // In case of errors, the error message is pushed onto the top of the stack 
        if( luaL_loadstring( m_LuaState, pScript ) )
        {
            // Get the error message from the top of the stack

            // NOTE: the lua_tostring function returns a pointer to an internal copy of the string. 
            // The string is always zero-terminated and Lua ensures that this pointer is valid as long 
            // as the corresponding value is in the stack. 
            auto ErrorMsg = lua_tostring( m_LuaState, -1 );
            lua_pop(m_LuaState, 1);  // pop error message from the stack

            if( ErrorMsg )
            {
                LOG_ERROR_AND_THROW( "Failed to parse the script file:\n", ErrorMsg );
            }
            else
            {
                LOG_ERROR_AND_THROW( "Failed to parse the script file." );
            }
        }
    }

    void ScriptParser::GetSamplerByName( const Char *SamplerName, ISampler** ppSampler )
    {
        m_pSamplerParser->GetObjectByName( m_LuaState, SamplerName, ppSampler );
    }

    void ScriptParser::GetShaderByName( const Char *ShaderName, IShader** ppShader )
    {
        m_pShaderParser->GetObjectByName( m_LuaState, ShaderName, ppShader );
    }

    void ScriptParser::GetBufferByName( const Char *BufferName, IBuffer** ppBuffer )
    {
        m_pBufferParser->GetObjectByName( m_LuaState, BufferName, ppBuffer );
    }

    void ScriptParser::GetTextureByName( const Char *TextureName, ITexture** ppTexture )
    {
        m_pTextureParser->GetObjectByName( m_LuaState, TextureName, ppTexture );
    }

    void ScriptParser::GetResourceMappingByName( const Char *ResourceMappingName, IResourceMapping** ppResourceMapping )
    {
        m_pResourceMappingParser->GetObjectByName( m_LuaState, ResourceMappingName, ppResourceMapping );
    }

    void ScriptParser::GetTextureViewByName( const Char *TextureViewName, ITextureView** ppTextureView )
    {
        m_pTextureViewParser->GetObjectByName( m_LuaState, TextureViewName, ppTextureView );
    }

    void ScriptParser::GetBufferViewByName( const Char *BufferViewName, IBufferView** ppBufferView )
    {
        m_pBufferViewParser->GetObjectByName( m_LuaState, BufferViewName, ppBufferView );
    }

    void ScriptParser::GetPipelineStateByName( const Char *PSOName, IPipelineState** ppPSO )
    {
        m_pPSOParser->GetObjectByName( m_LuaState, PSOName, ppPSO );
    }

    void ScriptParser::GetShaderVariableByName( const Char *ShaderVarName, IShaderResourceVariable** ppShaderVar )
    {
        m_pShaderVariableParser->GetObjectByName( m_LuaState, ShaderVarName, ppShaderVar );
    }

    void ScriptParser::GetShaderResourceBindingByName( const Char *SRBName, IShaderResourceBinding** ppSRB )
    {
        m_pShaderResBindingParser->GetObjectByName( m_LuaState, SRBName, ppSRB );
    }

    void ScriptParser::QueryInterface(const INTERFACE_ID& IID, IObject** ppInterface )
    {
        UNSUPPORTED( "Not implemented" );
    }
}
