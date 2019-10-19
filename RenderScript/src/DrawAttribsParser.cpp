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
#include "DrawAttribsParser.h"
#include "BufferParser.h"
#include "ScriptParser.h"

namespace Diligent
{
    using CombinedDrawAttribs = ScriptParser::CombinedDrawAttribs;

    const Char* DrawAttribsParser::DrawAttribsLibName = "DrawAttribs";

    DrawAttribsParser::DrawAttribsParser( BufferParser *pBuffParser, IRenderDevice *pRenderDevice, lua_State *L ) :
        EngineObjectParserBase( pRenderDevice, L, DrawAttribsLibName ),
        m_DrawBinding( this, L, "Context", "Draw", &DrawAttribsParser::Draw ),
        m_DispatchComputeBinding( this, L, "Context", "DispatchCompute", &DrawAttribsParser::DispatchCompute ),
        m_BufferMetatableName(pBuffParser->GetMetatableName())
    {
        //  NumVertices and NumIndices are in Union
        DEFINE_BINDER( m_Bindings, CombinedDrawAttribs, NumVertices );
        DEFINE_BINDER( m_Bindings, CombinedDrawAttribs, NumIndices );

        DEFINE_ENUM_ELEMENT_MAPPING( m_DrawFlagsEnumMapping, DRAW_FLAG_NONE );
        DEFINE_ENUM_ELEMENT_MAPPING( m_DrawFlagsEnumMapping, DRAW_FLAG_VERIFY_STATES );
        DEFINE_ENUM_ELEMENT_MAPPING( m_DrawFlagsEnumMapping, DRAW_FLAG_VERIFY_DRAW_ATTRIBS );
        DEFINE_ENUM_ELEMENT_MAPPING( m_DrawFlagsEnumMapping, DRAW_FLAG_VERIFY_RENDER_TARGETS );
        DEFINE_ENUM_ELEMENT_MAPPING( m_DrawFlagsEnumMapping, DRAW_FLAG_VERIFY_ALL );
        
        DEFINE_FLAGS_BINDER( m_Bindings, CombinedDrawAttribs, Flags, DRAW_FLAGS, m_DrawFlagsEnumMapping );
        DEFINE_ENUM_BINDER( m_Bindings, CombinedDrawAttribs, IndirectAttribsBufferStateTransitionMode, m_StateTransitionModeEnumMapping);

        DEFINE_ENUM_ELEMENT_MAPPING( m_ValueTypeEnumMapping, VT_UINT16 );
        DEFINE_ENUM_ELEMENT_MAPPING( m_ValueTypeEnumMapping, VT_UINT32 );
        DEFINE_ENUM_BINDER( m_Bindings, CombinedDrawAttribs, IndexType, m_ValueTypeEnumMapping );
        
        DEFINE_BINDER( m_Bindings, CombinedDrawAttribs, NumInstances );
        
        DEFINE_BINDER( m_Bindings, CombinedDrawAttribs, BaseVertex );
        DEFINE_BINDER( m_Bindings, CombinedDrawAttribs, IndirectDrawArgsOffset );

        // StartVertexLocation and FirstIndexLocation are in union
        DEFINE_BINDER( m_Bindings, CombinedDrawAttribs, StartVertexLocation );
        DEFINE_BINDER( m_Bindings, CombinedDrawAttribs, FirstIndexLocation );

        DEFINE_BINDER( m_Bindings, CombinedDrawAttribs, FirstInstanceLocation );

        std::vector<String> AllowedMetatable = { "Metatables.Buffer" };
        DEFINE_BINDER_EX( m_Bindings, CombinedDrawAttribs, pIndirectDrawAttribs, EngineObjectPtrLoader<IBuffer>, AllowedMetatable );
    };

    void DrawAttribsParser::CreateObj( lua_State *L )
    {
        INIT_LUA_STACK_TRACKING(L);

        CombinedDrawAttribs DrawAttrs;
        ParseLuaTable( L, 1, &DrawAttrs, m_Bindings );

        CHECK_LUA_STACK_HEIGHT();

        auto pDrawAttribs = reinterpret_cast<CombinedDrawAttribs*>(lua_newuserdata( L, sizeof( CombinedDrawAttribs ) ));
        memcpy(pDrawAttribs, &DrawAttrs, sizeof(CombinedDrawAttribs));

        CHECK_LUA_STACK_HEIGHT( +1 );
    }

    void DrawAttribsParser::DestroyObj( void *pData )
    {
        // We do not need to do anything, because the whole object is 
        // created as full user data and thus managed by Lua
    }

    void DrawAttribsParser::ReadField( lua_State *L, void *pData, const Char *Field )
    {
        auto pDrawAttribs = reinterpret_cast<CombinedDrawAttribs*>(pData);
        PushField( L, pDrawAttribs, Field, m_Bindings );
    }

    void DrawAttribsParser::UpdateField( lua_State *L, void *pData, const Char *Field )
    {
        auto pDrawAttribs = reinterpret_cast<CombinedDrawAttribs*>(pData);
        Diligent::UpdateField( L, -1, pDrawAttribs, Field, m_Bindings );
    }

    void DrawAttribsParser::PushExistingObject( lua_State *L, const void *pObject )
    {
        auto pDrawAttribs = reinterpret_cast<CombinedDrawAttribs*>(lua_newuserdata( L, sizeof( CombinedDrawAttribs ) ));
        memcpy( pDrawAttribs, pObject, sizeof( CombinedDrawAttribs ) );
    }

    int DrawAttribsParser::Draw( lua_State *L )
    {
        auto pDrawAttribs = GetUserData<CombinedDrawAttribs*>( L, 1, m_MetatableRegistryName.c_str() );
        auto *pContext = LoadDeviceContextFromRegistry( L );
        if (pDrawAttribs->pIndirectDrawAttribs != nullptr)
        {
            if (pDrawAttribs->IndexType != VT_UNDEFINED)
            {
                DrawIndexedIndirectAttribs Attribs;
                Attribs.Flags = pDrawAttribs->Flags;
                Attribs.IndexType = pDrawAttribs->IndexType;
                Attribs.IndirectAttribsBufferStateTransitionMode = pDrawAttribs->IndirectAttribsBufferStateTransitionMode;
                Attribs.IndirectDrawArgsOffset = pDrawAttribs->IndirectDrawArgsOffset;
                pContext->DrawIndexedIndirect(Attribs, pDrawAttribs->pIndirectDrawAttribs);
            }
            else
            {
                DrawIndirectAttribs Attribs;
                Attribs.Flags = pDrawAttribs->Flags;
                Attribs.IndirectAttribsBufferStateTransitionMode = pDrawAttribs->IndirectAttribsBufferStateTransitionMode;
                Attribs.IndirectDrawArgsOffset = pDrawAttribs->IndirectDrawArgsOffset;
                pContext->DrawIndirect(Attribs, pDrawAttribs->pIndirectDrawAttribs);
            }
        }
        else
        {
            if (pDrawAttribs->IndexType != VT_UNDEFINED)
            {
                DrawIndexedAttribs Attribs;
                Attribs.BaseVertex = pDrawAttribs->BaseVertex;
                Attribs.FirstIndexLocation = pDrawAttribs->FirstIndexLocation;
                Attribs.FirstInstanceLocation = pDrawAttribs->FirstInstanceLocation;
                Attribs.Flags = pDrawAttribs->Flags;
                Attribs.IndexType = pDrawAttribs->IndexType;
                Attribs.NumIndices = pDrawAttribs->NumIndices;
                Attribs.NumInstances = pDrawAttribs->NumInstances;
                pContext->DrawIndexed(Attribs);
            }
            else
            {
                DrawAttribs Attribs;
                Attribs.FirstInstanceLocation = pDrawAttribs->FirstInstanceLocation;
                Attribs.Flags = pDrawAttribs->Flags;
                Attribs.NumInstances = pDrawAttribs->NumInstances;
                Attribs.NumVertices = pDrawAttribs->NumVertices;
                Attribs.StartVertexLocation = pDrawAttribs->StartVertexLocation;
                pContext->Draw(Attribs);
            }
        }

        return 0;
    }

    int DrawAttribsParser::DispatchCompute( lua_State *L )
    {
        if( lua_type( L, 1 ) == LUA_TUSERDATA )
        {
            DispatchComputeIndirectAttribs DispatchAttrs;
            auto* pIndirectDispatchAttribs = *GetUserData<IBuffer**>( L, 1, m_BufferMetatableName.c_str() );
            int CurrArgInd = 2;
            if( CurrArgInd <= lua_gettop( L ) && lua_isnumber( L, CurrArgInd ) )
            {
                DispatchAttrs.DispatchArgsByteOffset = ReadValueFromLua<Uint32>( L, CurrArgInd );
                ++CurrArgInd;
            }
            if( CurrArgInd <= lua_gettop( L ) && lua_type( L, CurrArgInd ) == LUA_TSTRING )
            {
                EnumMemberBinder<RESOURCE_STATE_TRANSITION_MODE> StateTransitionModeLoader(0, "StateTransitionMode", m_StateTransitionModeEnumMapping);
                StateTransitionModeLoader.SetValue( L, CurrArgInd, &DispatchAttrs.IndirectAttribsBufferStateTransitionMode );
            }
            auto *pContext = LoadDeviceContextFromRegistry( L );
            pContext->DispatchComputeIndirect( DispatchAttrs, pIndirectDispatchAttribs );
        }
        else
        {
            DispatchComputeAttribs DispatchAttrs;
            if( lua_gettop( L ) >= 1 )
                DispatchAttrs.ThreadGroupCountX = ReadValueFromLua<Uint32>( L, 1 );
            if( lua_gettop( L ) >= 2 )
                DispatchAttrs.ThreadGroupCountY = ReadValueFromLua<Uint32>( L, 2 );
            if( lua_gettop( L ) >= 3 )
                DispatchAttrs.ThreadGroupCountY = ReadValueFromLua<Uint32>( L, 3 );
            auto *pContext = LoadDeviceContextFromRegistry( L );
            pContext->DispatchCompute( DispatchAttrs );
        }


        return 0;
    }
}
