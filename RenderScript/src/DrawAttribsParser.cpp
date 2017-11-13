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
#include "DrawAttribsParser.h"
#include "BufferParser.h"

namespace Diligent
{
    const Char* DrawAttribsParser::DrawAttribsLibName = "DrawAttribs";

    DrawAttribsParser::DrawAttribsParser( BufferParser *pBuffParser, IRenderDevice *pRenderDevice, lua_State *L ) :
        EngineObjectParserBase( pRenderDevice, L, DrawAttribsLibName ),
        m_DrawBinding( this, L, "Context", "Draw", &DrawAttribsParser::Draw ),
        m_DispatchComputeBinding( this, L, "Context", "DispatchCompute", &DrawAttribsParser::DispatchCompute ),
        m_BufferMetatableName(pBuffParser->GetMetatableName())
    {
        DEFINE_ENUM_ELEMENT_MAPPING( m_PrimTopologyEnumMapping, PRIMITIVE_TOPOLOGY_TRIANGLE_LIST );
        DEFINE_ENUM_ELEMENT_MAPPING( m_PrimTopologyEnumMapping, PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP );
        DEFINE_ENUM_ELEMENT_MAPPING( m_PrimTopologyEnumMapping, PRIMITIVE_TOPOLOGY_POINT_LIST );
        DEFINE_ENUM_ELEMENT_MAPPING( m_PrimTopologyEnumMapping, PRIMITIVE_TOPOLOGY_LINE_LIST );
        DEFINE_ENUM_ELEMENT_MAPPING( m_PrimTopologyEnumMapping, PRIMITIVE_TOPOLOGY_1_CONTROL_POINT_PATCHLIST );
        DEFINE_ENUM_ELEMENT_MAPPING( m_PrimTopologyEnumMapping, PRIMITIVE_TOPOLOGY_2_CONTROL_POINT_PATCHLIST );
        DEFINE_ENUM_ELEMENT_MAPPING( m_PrimTopologyEnumMapping, PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST );
        DEFINE_ENUM_ELEMENT_MAPPING( m_PrimTopologyEnumMapping, PRIMITIVE_TOPOLOGY_4_CONTROL_POINT_PATCHLIST );
        DEFINE_ENUM_ELEMENT_MAPPING( m_PrimTopologyEnumMapping, PRIMITIVE_TOPOLOGY_5_CONTROL_POINT_PATCHLIST );
        DEFINE_ENUM_ELEMENT_MAPPING( m_PrimTopologyEnumMapping, PRIMITIVE_TOPOLOGY_6_CONTROL_POINT_PATCHLIST );
        DEFINE_ENUM_ELEMENT_MAPPING( m_PrimTopologyEnumMapping, PRIMITIVE_TOPOLOGY_7_CONTROL_POINT_PATCHLIST );
        DEFINE_ENUM_ELEMENT_MAPPING( m_PrimTopologyEnumMapping, PRIMITIVE_TOPOLOGY_8_CONTROL_POINT_PATCHLIST );
        DEFINE_ENUM_ELEMENT_MAPPING( m_PrimTopologyEnumMapping, PRIMITIVE_TOPOLOGY_9_CONTROL_POINT_PATCHLIST );
        DEFINE_ENUM_ELEMENT_MAPPING( m_PrimTopologyEnumMapping, PRIMITIVE_TOPOLOGY_10_CONTROL_POINT_PATCHLIST );
        DEFINE_ENUM_ELEMENT_MAPPING( m_PrimTopologyEnumMapping, PRIMITIVE_TOPOLOGY_11_CONTROL_POINT_PATCHLIST );
        DEFINE_ENUM_ELEMENT_MAPPING( m_PrimTopologyEnumMapping, PRIMITIVE_TOPOLOGY_12_CONTROL_POINT_PATCHLIST );
        DEFINE_ENUM_ELEMENT_MAPPING( m_PrimTopologyEnumMapping, PRIMITIVE_TOPOLOGY_13_CONTROL_POINT_PATCHLIST );
        DEFINE_ENUM_ELEMENT_MAPPING( m_PrimTopologyEnumMapping, PRIMITIVE_TOPOLOGY_14_CONTROL_POINT_PATCHLIST );
        DEFINE_ENUM_ELEMENT_MAPPING( m_PrimTopologyEnumMapping, PRIMITIVE_TOPOLOGY_15_CONTROL_POINT_PATCHLIST );
        DEFINE_ENUM_ELEMENT_MAPPING( m_PrimTopologyEnumMapping, PRIMITIVE_TOPOLOGY_16_CONTROL_POINT_PATCHLIST );
        DEFINE_ENUM_ELEMENT_MAPPING( m_PrimTopologyEnumMapping, PRIMITIVE_TOPOLOGY_17_CONTROL_POINT_PATCHLIST );
        DEFINE_ENUM_ELEMENT_MAPPING( m_PrimTopologyEnumMapping, PRIMITIVE_TOPOLOGY_18_CONTROL_POINT_PATCHLIST );
        DEFINE_ENUM_ELEMENT_MAPPING( m_PrimTopologyEnumMapping, PRIMITIVE_TOPOLOGY_19_CONTROL_POINT_PATCHLIST );
        DEFINE_ENUM_ELEMENT_MAPPING( m_PrimTopologyEnumMapping, PRIMITIVE_TOPOLOGY_20_CONTROL_POINT_PATCHLIST );
        DEFINE_ENUM_ELEMENT_MAPPING( m_PrimTopologyEnumMapping, PRIMITIVE_TOPOLOGY_21_CONTROL_POINT_PATCHLIST );
        DEFINE_ENUM_ELEMENT_MAPPING( m_PrimTopologyEnumMapping, PRIMITIVE_TOPOLOGY_22_CONTROL_POINT_PATCHLIST );
        DEFINE_ENUM_ELEMENT_MAPPING( m_PrimTopologyEnumMapping, PRIMITIVE_TOPOLOGY_23_CONTROL_POINT_PATCHLIST );
        DEFINE_ENUM_ELEMENT_MAPPING( m_PrimTopologyEnumMapping, PRIMITIVE_TOPOLOGY_24_CONTROL_POINT_PATCHLIST );
        DEFINE_ENUM_ELEMENT_MAPPING( m_PrimTopologyEnumMapping, PRIMITIVE_TOPOLOGY_25_CONTROL_POINT_PATCHLIST );
        DEFINE_ENUM_ELEMENT_MAPPING( m_PrimTopologyEnumMapping, PRIMITIVE_TOPOLOGY_26_CONTROL_POINT_PATCHLIST );
        DEFINE_ENUM_ELEMENT_MAPPING( m_PrimTopologyEnumMapping, PRIMITIVE_TOPOLOGY_27_CONTROL_POINT_PATCHLIST );
        DEFINE_ENUM_ELEMENT_MAPPING( m_PrimTopologyEnumMapping, PRIMITIVE_TOPOLOGY_28_CONTROL_POINT_PATCHLIST );
        DEFINE_ENUM_ELEMENT_MAPPING( m_PrimTopologyEnumMapping, PRIMITIVE_TOPOLOGY_29_CONTROL_POINT_PATCHLIST );
        DEFINE_ENUM_ELEMENT_MAPPING( m_PrimTopologyEnumMapping, PRIMITIVE_TOPOLOGY_30_CONTROL_POINT_PATCHLIST );
        DEFINE_ENUM_ELEMENT_MAPPING( m_PrimTopologyEnumMapping, PRIMITIVE_TOPOLOGY_31_CONTROL_POINT_PATCHLIST );
        DEFINE_ENUM_ELEMENT_MAPPING( m_PrimTopologyEnumMapping, PRIMITIVE_TOPOLOGY_32_CONTROL_POINT_PATCHLIST );
        VERIFY( m_PrimTopologyEnumMapping.m_Str2ValMap.size() == PRIMITIVE_TOPOLOGY_NUM_TOPOLOGIES - 1,
                "Unexpected map size. Did you update PRIMITIVE_TOPOLOGY enum?" );
        VERIFY( m_PrimTopologyEnumMapping.m_Val2StrMap.size() == PRIMITIVE_TOPOLOGY_NUM_TOPOLOGIES - 1,
                "Unexpected map size. Did you update PRIMITIVE_TOPOLOGY enum?" );
        DEFINE_ENUM_BINDER( m_Bindings, DrawAttribs, Topology, PRIMITIVE_TOPOLOGY, m_PrimTopologyEnumMapping );

        //  NumVertices and NumIndices are in Union
        DEFINE_BINDER( m_Bindings, DrawAttribs, NumVertices, Uint32, Validator<Uint32>() );
        DEFINE_BINDER( m_Bindings, DrawAttribs, NumIndices, Uint32, Validator<Uint32>() );

        DEFINE_ENUM_ELEMENT_MAPPING( m_ValueTypeEnumMapping, VT_UINT16 );
        DEFINE_ENUM_ELEMENT_MAPPING( m_ValueTypeEnumMapping, VT_UINT32 );
        DEFINE_ENUM_BINDER( m_Bindings, DrawAttribs, IndexType, VALUE_TYPE, m_ValueTypeEnumMapping );
        
        DEFINE_BINDER( m_Bindings, DrawAttribs, IsIndexed, Bool, Validator<Bool>() );
        DEFINE_BINDER( m_Bindings, DrawAttribs, NumInstances, Uint32, Validator<Uint32>() );
        DEFINE_BINDER( m_Bindings, DrawAttribs, IsIndirect, Bool, Validator<Bool>() );
        DEFINE_BINDER( m_Bindings, DrawAttribs, BaseVertex, Uint32, Validator<Uint32>() );
        DEFINE_BINDER( m_Bindings, DrawAttribs, IndirectDrawArgsOffset, Uint32, Validator<Uint32>() );

        // StartVertexLocation and FirstIndexLocation are in union
        DEFINE_BINDER( m_Bindings, DrawAttribs, StartVertexLocation, Uint32, Validator<Uint32>() );
        DEFINE_BINDER( m_Bindings, DrawAttribs, FirstIndexLocation, Uint32, Validator<Uint32>() );

        DEFINE_BINDER( m_Bindings, DrawAttribs, FirstInstanceLocation, Uint32, Validator<Uint32>() );

        std::vector<String> AllowedMetatable = { "Metatables.Buffer" };
        DEFINE_BINDER( m_Bindings, DrawAttribs, pIndirectDrawAttribs, EngineObjectPtrLoader<IBuffer>, AllowedMetatable );
    };

    void DrawAttribsParser::CreateObj( lua_State *L )
    {
        INIT_LUA_STACK_TRACKING(L);

        DrawAttribs DrawAttrs;
        ParseLuaTable( L, 1, &DrawAttrs, m_Bindings );

        CHECK_LUA_STACK_HEIGHT();

        auto pDrawAttribs = reinterpret_cast<DrawAttribs*>(lua_newuserdata( L, sizeof( DrawAttribs ) ));
        memcpy(pDrawAttribs, &DrawAttrs, sizeof(DrawAttribs));

        CHECK_LUA_STACK_HEIGHT( +1 );
    }

    void DrawAttribsParser::DestroyObj( void *pData )
    {
        // We do not need to do anything, because the whole object is 
        // created as full user data and thus managed by Lua
    }

    void DrawAttribsParser::ReadField( lua_State *L, void *pData, const Char *Field )
    {
        auto pDrawAttribs = reinterpret_cast<DrawAttribs*>(pData);
        PushField( L, pDrawAttribs, Field, m_Bindings );
    }

    void DrawAttribsParser::UpdateField( lua_State *L, void *pData, const Char *Field )
    {
        auto pDrawAttribs = reinterpret_cast<DrawAttribs*>(pData);
        Diligent::UpdateField( L, -1, pDrawAttribs, Field, m_Bindings );
    }

    void DrawAttribsParser::PushExistingObject( lua_State *L, const void *pObject )
    {
        auto pDrawAttribs = reinterpret_cast<DrawAttribs*>(lua_newuserdata( L, sizeof( DrawAttribs ) ));
        memcpy( pDrawAttribs, pObject, sizeof( DrawAttribs ) );
    }

    int DrawAttribsParser::Draw( lua_State *L )
    {
        auto pDrawAttribs = GetUserData<DrawAttribs*>( L, 1, m_MetatableRegistryName.c_str() );
        auto *pContext = LoadDeviceContextFromRegistry( L );
        pContext->Draw( *pDrawAttribs );

        return 0;
    }

    int DrawAttribsParser::DispatchCompute( lua_State *L )
    {
        DispatchComputeAttribs DispatchAttrs;
        if( lua_type( L, 1 ) == LUA_TUSERDATA )
        {
            DispatchAttrs.pIndirectDispatchAttribs = *GetUserData<IBuffer**>( L, 1, m_BufferMetatableName.c_str() );
            if( lua_gettop( L ) > 1 )
            {
                DispatchAttrs.DispatchArgsByteOffset = ReadValueFromLua<Uint32>( L, 2 );
            }
        }
        else
        {
            if( lua_gettop( L ) >= 1 )
                DispatchAttrs.ThreadGroupCountX = ReadValueFromLua<Uint32>( L, 1 );
            if( lua_gettop( L ) >= 2 )
                DispatchAttrs.ThreadGroupCountY = ReadValueFromLua<Uint32>( L, 2 );
            if( lua_gettop( L ) >= 3 )
                DispatchAttrs.ThreadGroupCountY = ReadValueFromLua<Uint32>( L, 3 );
        }

        auto *pContext = LoadDeviceContextFromRegistry( L );
        pContext->DispatchCompute( DispatchAttrs );

        return 0;
    }
}
