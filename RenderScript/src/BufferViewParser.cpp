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

#include "pch.h"
#include "BufferViewParser.h"
#include "BufferParser.h"
#include "SamplerParser.h"

namespace Diligent
{
    const Char* BufferViewParser::BufferViewLibName = "BufferView";

    BufferViewParser::BufferViewParser( BufferParser *pBufParser, 
                                          IRenderDevice *pRenderDevice, 
                                          lua_State *L ) :
        EngineObjectParserCommon<IBufferView>( pRenderDevice, L, BufferViewLibName ),
        m_BufferLibMetatableName(pBufParser->GetMetatableName()),
        m_CreateViewBinding( this, L, m_BufferLibMetatableName.c_str(), "CreateView", &BufferViewParser::CreateView ),
        m_GetDefaultViewBinding( this, L, m_BufferLibMetatableName.c_str(), "GetDefaultView", &BufferViewParser::GetDefaultView ),
        m_ViewTypeParser( 0, "ViewType", m_ViewTypeEnumMapping )
    {
        DEFINE_BUFFERED_STRING_BINDER( m_Bindings, SBuffViewDescWrapper, Name, NameBuffer );

        DEFINE_ENUM_ELEMENT_MAPPING( m_ViewTypeEnumMapping, BUFFER_VIEW_SHADER_RESOURCE );
        DEFINE_ENUM_ELEMENT_MAPPING( m_ViewTypeEnumMapping, BUFFER_VIEW_UNORDERED_ACCESS );
        VERIFY( m_ViewTypeEnumMapping.m_Str2ValMap.size() == BUFFER_VIEW_NUM_VIEWS - 1,
                "Unexpected map size. Did you update BUFFER_VIEW_TYPE enum?" );
        VERIFY( m_ViewTypeEnumMapping.m_Val2StrMap.size() == BUFFER_VIEW_NUM_VIEWS - 1,
                "Unexpected map size. Did you update BUFFER_VIEW_TYPE enum?" );
        DEFINE_ENUM_BINDER( m_Bindings, SBuffViewDescWrapper, ViewType, BUFFER_VIEW_TYPE, m_ViewTypeEnumMapping );

        DEFINE_BINDER( m_Bindings, SBuffViewDescWrapper, ByteOffset, Uint32, Validator<Uint32>() );
        DEFINE_BINDER( m_Bindings, SBuffViewDescWrapper, ByteWidth, Uint32, Validator<Uint32>() );
    };

    void BufferViewParser::CreateObj( lua_State *L )
    {
        INIT_LUA_STACK_TRACKING(L);
        
        auto *pBuffer = *GetUserData<IBuffer**>( L, 1, m_BufferLibMetatableName.c_str() );

        SBuffViewDescWrapper BufferViewDesc;
        ParseLuaTable( L, 2, &BufferViewDesc, m_Bindings );
        CHECK_LUA_STACK_HEIGHT();

        auto ppBufferView = reinterpret_cast<IBufferView**>(lua_newuserdata( L, sizeof( IBufferView* ) ));
        *ppBufferView = nullptr;
        pBuffer->CreateView( BufferViewDesc, ppBufferView );
        if( *ppBufferView == nullptr )
            SCRIPT_PARSING_ERROR(L, "Failed to create buffer view")

        CHECK_LUA_STACK_HEIGHT( +1 );
    }

    int BufferViewParser::CreateView( lua_State *L )
    {
        // Note that LuaCreate is a static function.
        // However, this pointer to the BufferViewParser object
        // is stored in the upvalue. So LuaCreate will call 
        // BufferViewParser::CreateObj()
        return LuaCreate(L);
    }

    int BufferViewParser::GetDefaultView( lua_State *L )
    {
        INIT_LUA_STACK_TRACKING( L );

        // Buffer should be the first argument
        auto *pBuffer = *GetUserData<IBuffer**>( L, 1, m_BufferLibMetatableName.c_str() );
        
        // View type should be the second argument
        BUFFER_VIEW_TYPE ViewType;
        m_ViewTypeParser.SetValue( L, 2, &ViewType );

        auto pView = pBuffer->GetDefaultView( ViewType );
        
        // Push existing object
        PushObject(L, pView);

        CHECK_LUA_STACK_HEIGHT( +1 );

        // Returning one value to Lua
        return 1;
    }
}
