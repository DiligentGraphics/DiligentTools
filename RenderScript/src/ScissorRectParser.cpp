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
#include "ScissorRectParser.h"

namespace Diligent
{
    const Char* ScissorRectParser::ScissorRectLibName = "ScissorRect";

    ScissorRectParser::ScissorRectParser( IRenderDevice *pRenderDevice, lua_State *L ) :
        EngineObjectParserBase( pRenderDevice, L, ScissorRectLibName ),
        m_SetScissorRectsBinding( this, L, "Context", "SetScissorRects", &ScissorRectParser::SetScissorRects )
    {
        m_ScissorRects.reserve( 8 );

        //  NumVertices and NumIndices are in Union
        DEFINE_BINDER( m_Bindings, Rect, left   );
        DEFINE_BINDER( m_Bindings, Rect, top    );
        DEFINE_BINDER( m_Bindings, Rect, right  );
        DEFINE_BINDER( m_Bindings, Rect, bottom );
    };

    void ScissorRectParser::CreateObj( lua_State *L )
    {
        INIT_LUA_STACK_TRACKING(L);

        Rect ScissorRect;
        ParseLuaTable( L, 1, &ScissorRect, m_Bindings );
        if( ScissorRect.left > ScissorRect.right )
            SCRIPT_PARSING_ERROR(L, "Scissor rect left and right boundaries (", ScissorRect.left, ", ", ScissorRect.right, ") are incorrect")
        if( ScissorRect.top > ScissorRect.bottom )
            SCRIPT_PARSING_ERROR(L, "Scissor rect top and bottom boundaries (", ScissorRect.top, ", ", ScissorRect.bottom, ") are incorrect")

        CHECK_LUA_STACK_HEIGHT();

        auto pScissorRect = reinterpret_cast<Rect*>(lua_newuserdata( L, sizeof( Rect ) ));
        memcpy(pScissorRect, &ScissorRect, sizeof(ScissorRect));

        CHECK_LUA_STACK_HEIGHT( +1 );
    }

    void ScissorRectParser::DestroyObj( void *pData )
    {
        // We do not need to do anything, because the whole object is 
        // created as full user data and thus managed by Lua
    }

    void ScissorRectParser::ReadField( lua_State *L, void *pData, const Char *Field )
    {
        auto pScissorRect = reinterpret_cast<Rect*>(pData);
        PushField( L, pScissorRect, Field, m_Bindings );
    }

    void ScissorRectParser::UpdateField( lua_State *L, void *pData, const Char *Field )
    {
        auto pScissorRect = reinterpret_cast<Rect*>(pData);
        Diligent::UpdateField( L, -1, pScissorRect, Field, m_Bindings );
    }

    void ScissorRectParser::PushExistingObject( lua_State *L, const void *pObject )
    {
        auto pScissorRect = reinterpret_cast<Rect*>(lua_newuserdata( L, sizeof( Rect ) ));
        memcpy( pScissorRect, pObject, sizeof( Rect ) );
    }

    int ScissorRectParser::SetScissorRects( lua_State *L )
    {
        auto NumArgs = lua_gettop( L );
        Uint32 RTWidth  = 0;
        Uint32 RTHeight = 0;
        m_ScissorRects.clear();
        for( int Arg = 1; Arg <= NumArgs; ++Arg )
        {
            if( lua_type( L, Arg ) == LUA_TUSERDATA )
            {
                auto pScissorRect = GetUserData<Rect*>( L, Arg, m_MetatableRegistryName.c_str() );
                m_ScissorRects.emplace_back( *pScissorRect );
            }
            else
            {
                if( RTWidth == 0 )
                    RTWidth = ReadValueFromLua<Uint32>( L, Arg );
                else if( RTHeight == 0 )
                    RTHeight = ReadValueFromLua<Uint32>( L, Arg );
                else
                    SCRIPT_PARSING_ERROR( L, "Render target size already specified (", RTWidth, "x", RTHeight, ")." );
            }
        }

        if( (RTWidth == 0 && RTHeight != 0) || (RTWidth != 0 && RTHeight == 0) )
            SCRIPT_PARSING_ERROR( L, "Render target size is incomplete (", RTWidth, "x", RTHeight, "). Use either 0x0 or fully specified size" );
            
        Uint32 NumScissorRects = static_cast<Uint32>( m_ScissorRects.size() );
        
        if( NumScissorRects == 0 )
            SCRIPT_PARSING_ERROR( L, "At least one viewport must be specified" );

        auto *pContext = LoadDeviceContextFromRegistry( L );
        pContext->SetScissorRects( NumScissorRects, m_ScissorRects.data(), RTWidth, RTHeight );

        return 0;
    }
}
