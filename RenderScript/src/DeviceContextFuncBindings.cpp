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
#include "DeviceContextFuncBindings.h"
#include "GraphicsUtilities.h"
#include "TextureViewParser.h"
#include "EngineObjectParserBase.h"

namespace Diligent
{
    DeviceContextFuncBindings::DeviceContextFuncBindings( IRenderDevice *pRenderDevice, lua_State *L, TextureViewParser *pTexViewPasrser ) :
        m_SetRenderTargetsBinding(this, L, "Context", "SetRenderTargets", &DeviceContextFuncBindings::SetRenderTargets),
        m_ClearRenderTargetBinding(this, L, "Context", "ClearRenderTarget", &DeviceContextFuncBindings::ClearRenderTarget),
        m_ClearDepthStencilBinding(this, L, "Context", "ClearDepthStencil", &DeviceContextFuncBindings::ClearDepthStencil),
        m_TexViewMetatableName( pTexViewPasrser->GetMetatableName() )
    {
    };

    int DeviceContextFuncBindings::SetRenderTargets( lua_State *L )
    {
        auto NumArgs = lua_gettop( L );
        ITextureView *pRTVs[MaxRenderTargets] = {};
        ITextureView *pDSV = nullptr;
        Uint32 NumRTs = 0;
        for( int CurrArg = 1; CurrArg <= NumArgs; ++CurrArg )
        {
            auto *pView = *GetUserData<ITextureView**>( L, CurrArg, m_TexViewMetatableName.c_str() );
            auto ViewType = pView->GetDesc().ViewType;
            if( ViewType == TEXTURE_VIEW_RENDER_TARGET )
            {
                if( NumRTs < MaxRenderTargets )
                {
                    pRTVs[NumRTs] = pView;
                    ++NumRTs;
                }
                else
                {
                    SCRIPT_PARSING_ERROR( L, "Too many render targets are being set. ", MaxRenderTargets, " at most are allowed." );
                }
            }
            else if( ViewType == TEXTURE_VIEW_DEPTH_STENCIL )
            {
                if( pDSV != nullptr )
                    SCRIPT_PARSING_ERROR( L, "Respecifying depth stencil view. Only one is allowed" );
                pDSV = pView;
            }
            else
            { 
                SCRIPT_PARSING_ERROR( L, "Unexpected view type. Only render target and depth stencil are allowed" );
            }
        }

        auto *pContext = EngineObjectParserBase::LoadDeviceContextFromRegistry( L );
        pContext->SetRenderTargets( NumRTs, pRTVs, pDSV );

        // Return no values to Lua
        return 0;
    }

    int DeviceContextFuncBindings::ClearRenderTarget( lua_State *L )
    {
        auto NumArgs = lua_gettop( L );

        ITextureView *pView = nullptr;
        Float32 RGBA[4] = {};

        int CurrArg = 1;
        if( CurrArg <= NumArgs )
        {
            if( lua_type( L, CurrArg ) == LUA_TUSERDATA )
            {
                pView = *GetUserData<ITextureView**>( L, CurrArg, m_TexViewMetatableName.c_str() );
                ++CurrArg;
            }
        }
        for( int c = 0; c < 4 && CurrArg <= NumArgs; ++c, ++CurrArg )
        {
            RGBA[c] = ReadValueFromLua<Float32>( L, CurrArg );
        }
        auto *pContext = EngineObjectParserBase::LoadDeviceContextFromRegistry( L );
        pContext->ClearRenderTarget( pView, RGBA );

        return 0;
    }

    int DeviceContextFuncBindings::ClearDepthStencil( lua_State *L )
    {
        auto NumArgs = lua_gettop( L );

        ITextureView *pView = nullptr;
        Float32 fDepth = 1.f;
        Uint8 Stencil = 0;
        Uint32 ClearFlags = 0;

        int CurrArg = 1;
        if( CurrArg <= NumArgs )
        {
            if( lua_type( L, CurrArg ) == LUA_TUSERDATA )
            {
                pView = *GetUserData<ITextureView**>( L, CurrArg, m_TexViewMetatableName.c_str() );
                ++CurrArg;
            }
        }

        if( CurrArg <= NumArgs )
        {
            fDepth = ReadValueFromLua<Float32>( L, CurrArg );
            ClearFlags |= CLEAR_DEPTH_FLAG;
            ++CurrArg;
        }

        if( CurrArg <= NumArgs )
        {
            Stencil = ReadValueFromLua<Uint8>( L, CurrArg );
            ClearFlags |= CLEAR_STENCIL_FLAG;
            ++CurrArg;
        }

        auto *pContext = EngineObjectParserBase::LoadDeviceContextFromRegistry( L );
        pContext->ClearDepthStencil( pView, ClearFlags, fDepth, Stencil );

        return 0;
    }
}
