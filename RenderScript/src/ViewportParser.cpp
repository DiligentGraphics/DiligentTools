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
#include "ViewportParser.h"

namespace Diligent
{
    const Char* ViewportParser::ViewportLibName = "Viewport";

    ViewportParser::ViewportParser( IRenderDevice *pRenderDevice, lua_State *L ) :
        EngineObjectParserBase( pRenderDevice, L, ViewportLibName ),
        m_SetViewportsBinding( this, L, "Context", "SetViewports", &ViewportParser::SetViewports )
    {
        m_Viewports.reserve( 8 );

        //  NumVertices and NumIndices are in Union
        DEFINE_BINDER( m_Bindings, Viewport, TopLeftX );
        DEFINE_BINDER( m_Bindings, Viewport, TopLeftY );
        DEFINE_BINDER( m_Bindings, Viewport, Width    );
        DEFINE_BINDER( m_Bindings, Viewport, Height   );
        DEFINE_BINDER( m_Bindings, Viewport, MinDepth );
        DEFINE_BINDER( m_Bindings, Viewport, MaxDepth );
    };

    void ViewportParser::CreateObj( lua_State *L )
    {
        INIT_LUA_STACK_TRACKING(L);

        Viewport VP;
        ParseLuaTable( L, 1, &VP, m_Bindings );
        if( VP.Width < 0 )
            SCRIPT_PARSING_ERROR( L, "VP width (", VP.Width, ") cannot be negative" );
        if( VP.Height < 0 )
            SCRIPT_PARSING_ERROR( L, "VP height (", VP.Height, ") cannot be negative" );
        if( VP.MinDepth > VP.MaxDepth )
            SCRIPT_PARSING_ERROR( L, "VP depth range (", VP.MinDepth, ", ", VP.MaxDepth, ") is incorrect" );

        CHECK_LUA_STACK_HEIGHT();

        auto pViewport = reinterpret_cast<Viewport*>(lua_newuserdata( L, sizeof( Viewport ) ));
        memcpy(pViewport, &VP, sizeof(Viewport));

        CHECK_LUA_STACK_HEIGHT( +1 );
    }

    void ViewportParser::DestroyObj( void *pData )
    {
        // We do not need to do anything, because the whole object is 
        // created as full user data and thus managed by Lua
    }

    void ViewportParser::ReadField( lua_State *L, void *pData, const Char *Field )
    {
        auto pViewport = reinterpret_cast<Viewport*>(pData);
        PushField( L, pViewport, Field, m_Bindings );
    }

    void ViewportParser::UpdateField( lua_State *L, void *pData, const Char *Field )
    {
        auto pViewport = reinterpret_cast<Viewport*>(pData);
        Diligent::UpdateField( L, -1, pViewport, Field, m_Bindings );
    }

    void ViewportParser::PushExistingObject( lua_State *L, const void *pObject )
    {
        auto pViewport = reinterpret_cast<Viewport*>(lua_newuserdata( L, sizeof( Viewport ) ));
        memcpy( pViewport, pObject, sizeof( Viewport ) );
    }

    int ViewportParser::SetViewports( lua_State *L )
    {
        auto *pContext = LoadDeviceContextFromRegistry( L );
        auto NumArgs = lua_gettop( L );
        if( NumArgs == 0 )
            pContext->SetViewports( 1, nullptr, 0, 0 );
        else
        {
            Uint32 RTWidth  = 0;
            Uint32 RTHeight = 0;
            m_Viewports.clear();
            for( int Arg = 1; Arg <= NumArgs; ++Arg )
            {
                if( lua_type( L, Arg ) == LUA_TUSERDATA )
                {
                    auto pViewport = GetUserData<Viewport*>( L, Arg, m_MetatableRegistryName.c_str() );
                    m_Viewports.emplace_back( *pViewport );
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

            Uint32 NumViewports = static_cast<Uint32>( m_Viewports.size() );

            if( NumViewports == 0 )
                SCRIPT_PARSING_ERROR( L, "At least one viewport must be specified" );

            pContext->SetViewports( NumViewports, m_Viewports.data(), RTWidth, RTHeight );
        }

        return 0;
    }
}
