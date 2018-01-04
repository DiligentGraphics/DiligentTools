/*     Copyright 2015-2018 Egor Yusov
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
#include "PSODescParser.h"
#include "BlendStateDescParser.h"
#include "RasterizerStateDescParser.h"
#include "DepthStencilStateDescParser.h"
#include "InputLayoutDescParser.h"

namespace std 
{
    DEFINE_ENUM_HASH( Diligent::PRIMITIVE_TOPOLOGY_TYPE )
}

namespace Diligent
{
    class RTVFormatsParser;
    
    template<>
    class MemberBinder<RTVFormatsParser> : public MemberBinderBase
    {
    public:
        MemberBinder( size_t /*MemberOffset*/, size_t /*Dummy*/ ) :
            // We will use a pointer to the GraphicsPipelineDesc structure itself
            MemberBinderBase( 0 ),
            m_TexFmtLoader(0, "RTVFormats", m_TexFmtEnumMapping)
        {
        }

        virtual void GetValue( lua_State *L, const void* pBasePointer )override
        {
            const auto &GraphicsPipeline = GetMemberByOffest< GraphicsPipelineDesc >(pBasePointer, m_MemberOffset);
            PushLuaArray( L, GraphicsPipeline.RTVFormats, GraphicsPipeline.RTVFormats + GraphicsPipeline.NumRenderTargets, [&]( const TEXTURE_FORMAT &Fmt )
            {
                m_TexFmtLoader.GetValue(L, &Fmt);
            });
        }

        virtual void SetValue( lua_State *L, int Index, void* pBasePointer )override
        {
            auto &GraphicsPipeline = GetMemberByOffest< GraphicsPipelineDesc >( pBasePointer, m_MemberOffset );
            if(lua_type(L,Index) == LUA_TTABLE)
            {
                ParseLuaArray( L, Index, pBasePointer, [&]( void* _pBasePointer, int StackIndex, int NewArrayIndex )
                {
                    // Lua array indices are 1-based
                    NewArrayIndex -= 1;
                    VERIFY( pBasePointer == _pBasePointer, "Sanity check failed" );
                    if( NewArrayIndex < 0 || NewArrayIndex >= MaxRenderTargets )
                        SCRIPT_PARSING_ERROR( L, "Render target array index ", NewArrayIndex," is out of allowed range [", 0, ' ', MaxRenderTargets-1, ']' );

                    m_TexFmtLoader.SetValue(L, StackIndex, &GraphicsPipeline.RTVFormats[NewArrayIndex]);
                    GraphicsPipeline.NumRenderTargets = std::max(GraphicsPipeline.NumRenderTargets, static_cast<Uint32>(NewArrayIndex+1));
                }
                );
            }
            else if(lua_type(L,Index) == LUA_TSTRING)
            {
                m_TexFmtLoader.SetValue(L, Index, &GraphicsPipeline.RTVFormats[0]);
                GraphicsPipeline.NumRenderTargets = 1;
            }
            else
            {
                SCRIPT_PARSING_ERROR( L, "Unexpected type ", lua_typename(L, Index), ". Table of strings or a string are expected" );
            }
        }
    private:
        TextureFormatEnumMapping m_TexFmtEnumMapping;
        EnumMemberBinder<TEXTURE_FORMAT> m_TexFmtLoader;
    };


    template<>
    class MemberBinder<SampleDesc> : public MemberBinderBase
    {
    public:
        MemberBinder( size_t MemberOffset, size_t Dummy ) :
            MemberBinderBase( MemberOffset )
        {
            DEFINE_BINDER( m_Bindings, SampleDesc, Count, Uint32, Validator<Uint32>("Count", 1,32) );
            DEFINE_BINDER( m_Bindings, SampleDesc, Quality, Uint32, Validator<Uint32>("Quality", 0,std::numeric_limits<Uint32>::max()) );
        }

        virtual void GetValue( lua_State *L, const void* pBasePointer )override
        {
            const auto &SmplDesc = GetMemberByOffest< SampleDesc >(pBasePointer, m_MemberOffset);
            PushLuaTable(L, &SmplDesc, m_Bindings);
        }

        virtual void SetValue( lua_State *L, int Index, void* pBasePointer )override
        {
            auto &SmplDesc = GetMemberByOffest< SampleDesc >( pBasePointer, m_MemberOffset );
            ParseLuaTable( L, Index, &SmplDesc, m_Bindings );
        }

    private:
        BindingsMapType m_Bindings;
    };

    template<>
    class MemberBinder<GraphicsPipelineDesc> : public MemberBinderBase
    {
    public:
        MemberBinder( size_t MemberOffset, size_t Dummy ) :
            MemberBinderBase( MemberOffset )
        {
            std::vector<String> AllowedMetatable = { "Metatables.Shader" };

            DEFINE_BINDER( m_Bindings, GraphicsPipelineDesc, pVS, EngineObjectPtrLoader<IShader>, AllowedMetatable );
            DEFINE_BINDER( m_Bindings, GraphicsPipelineDesc, pPS, EngineObjectPtrLoader<IShader>, AllowedMetatable );
            DEFINE_BINDER( m_Bindings, GraphicsPipelineDesc, pDS, EngineObjectPtrLoader<IShader>, AllowedMetatable );
            DEFINE_BINDER( m_Bindings, GraphicsPipelineDesc, pHS, EngineObjectPtrLoader<IShader>, AllowedMetatable );
            DEFINE_BINDER( m_Bindings, GraphicsPipelineDesc, pGS, EngineObjectPtrLoader<IShader>, AllowedMetatable );

            //D3D12_STREAM_OUTPUT_DESC StreamOutput;

            DEFINE_BINDER( m_Bindings, GraphicsPipelineDesc, BlendDesc, BlendStateDesc, 0 )
            DEFINE_BINDER( m_Bindings, GraphicsPipelineDesc, SampleMask, Uint32, Validator<Uint32>() )
            DEFINE_BINDER( m_Bindings, GraphicsPipelineDesc, RasterizerDesc, RasterizerStateDesc, 0 )
            DEFINE_BINDER( m_Bindings, GraphicsPipelineDesc, DepthStencilDesc, DepthStencilStateDesc, 0 )

            auto *pLayoutElemBinder = 
                new MemberBinder<InputLayoutDesc>( 
                    offsetof(GraphicsPipelineDesc, InputLayout), 
                    offsetof(PSODescParser::PSODescWrapper, LayoutElementsBuffer) - offsetof(PSODescParser::PSODescWrapper, GraphicsPipeline)
                );
            m_Bindings.insert( std::make_pair( "InputLayout", std::unique_ptr<MemberBinderBase>(pLayoutElemBinder) ) );

            //D3D12_INDEX_BUFFER_STRIP_CUT_VALUE IBStripCutValue;

            DEFINE_ENUM_ELEMENT_MAPPING( m_PrimTopologyTypeEnumMapping, PRIMITIVE_TOPOLOGY_TYPE_UNDEFINED );
            DEFINE_ENUM_ELEMENT_MAPPING( m_PrimTopologyTypeEnumMapping, PRIMITIVE_TOPOLOGY_TYPE_POINT );
            DEFINE_ENUM_ELEMENT_MAPPING( m_PrimTopologyTypeEnumMapping, PRIMITIVE_TOPOLOGY_TYPE_LINE );
            DEFINE_ENUM_ELEMENT_MAPPING( m_PrimTopologyTypeEnumMapping, PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE );
            DEFINE_ENUM_ELEMENT_MAPPING( m_PrimTopologyTypeEnumMapping, PRIMITIVE_TOPOLOGY_TYPE_PATCH );
            VERIFY( m_PrimTopologyTypeEnumMapping.m_Str2ValMap.size() == PRIMITIVE_TOPOLOGY_TYPE_NUM_TYPES,
                    "Unexpected map size. Did you update PRIMITIVE_TOPOLOGY_TYPE enum?" );
            VERIFY( m_PrimTopologyTypeEnumMapping.m_Val2StrMap.size() == PRIMITIVE_TOPOLOGY_TYPE_NUM_TYPES,
                    "Unexpected map size. Did you update PRIMITIVE_TOPOLOGY_TYPE enum?" );
            DEFINE_ENUM_BINDER( m_Bindings, GraphicsPipelineDesc, PrimitiveTopologyType, PRIMITIVE_TOPOLOGY_TYPE, m_PrimTopologyTypeEnumMapping );

            DEFINE_BINDER( m_Bindings, GraphicsPipelineDesc, RTVFormats, RTVFormatsParser, 0 );
            DEFINE_ENUM_BINDER( m_Bindings, GraphicsPipelineDesc, DSVFormat, TEXTURE_FORMAT, m_TexFmtEnumMapping );
            
            DEFINE_BINDER( m_Bindings, GraphicsPipelineDesc, SmplDesc, SampleDesc, 0 );

            //Uint32 NodeMask;
        }

        virtual void GetValue( lua_State *L, const void* pBasePointer )override
        {
            const auto &GraphicsPipeline = GetMemberByOffest< GraphicsPipelineDesc >(pBasePointer, m_MemberOffset);
            PushLuaTable(L, &GraphicsPipeline, m_Bindings);
        }

        virtual void SetValue( lua_State *L, int Index, void* pBasePointer )override
        {
            auto &GraphicsPipeline = GetMemberByOffest< GraphicsPipelineDesc >( pBasePointer, m_MemberOffset );
            ParseLuaTable( L, Index, &GraphicsPipeline, m_Bindings );
        }
    private:
        TextureFormatEnumMapping m_TexFmtEnumMapping;
        EnumMapping < PRIMITIVE_TOPOLOGY_TYPE > m_PrimTopologyTypeEnumMapping;
        BindingsMapType m_Bindings;
    };


    template<>
    class MemberBinder<ComputePipelineDesc> : public MemberBinderBase
    {
    public:
        MemberBinder( size_t MemberOffset, size_t Dummy ) :
            MemberBinderBase( MemberOffset )
        {
            std::vector<String> AllowedMetatable = { "Metatables.Shader" };
            DEFINE_BINDER( m_Bindings, ComputePipelineDesc, pCS, EngineObjectPtrLoader<IShader>, AllowedMetatable );
        }

        virtual void GetValue( lua_State *L, const void* pBasePointer )override
        {
            const auto &ComputePipeline = GetMemberByOffest< ComputePipelineDesc >(pBasePointer, m_MemberOffset);
            PushLuaTable(L, &ComputePipeline, m_Bindings);
        }

        virtual void SetValue( lua_State *L, int Index, void* pBasePointer )override
        {
            auto &ComputePipeline = GetMemberByOffest< ComputePipelineDesc >( pBasePointer, m_MemberOffset );
            ParseLuaTable( L, Index, &ComputePipeline, m_Bindings );
        }
    private:
        BindingsMapType m_Bindings;
    };


    const Char* PSODescParser::PSODescLibName = "PipelineState";
    PSODescParser::PSODescParser( IRenderDevice *pRenderDevice, lua_State *L ) :
        EngineObjectParserCommon<IPipelineState>( pRenderDevice, L, PSODescLibName ),
        m_SetPSOBinding( this, L, "Context", "SetPipelineState", &PSODescParser::SetPSO )
    {
        DEFINE_BUFFERED_STRING_BINDER( m_Bindings, PSODescWrapper, Name, NameBuffer )

        DEFINE_BINDER( m_Bindings, PSODescWrapper, IsComputePipeline, Bool, Validator<Bool>() )
        DEFINE_BINDER( m_Bindings, PSODescWrapper, SRBAllocationGranularity, Uint32, Validator<Uint32>("SRBAllocationGranularity", 1, 65536) )

        DEFINE_BINDER( m_Bindings, PSODescWrapper, GraphicsPipeline, GraphicsPipelineDesc, 0 )
        DEFINE_BINDER( m_Bindings, PSODescWrapper, ComputePipeline, ComputePipelineDesc, 0 )
    };

    void PSODescParser::CreateObj( lua_State *L )
    {
        INIT_LUA_STACK_TRACKING( L );

        PSODescWrapper PSODesc;
        ParseLuaTable( L, 1, &PSODesc, m_Bindings );

        CHECK_LUA_STACK_HEIGHT();

        auto ppPSO = reinterpret_cast<IPipelineState**>(lua_newuserdata( L, sizeof( IPipelineState* ) ));
        *ppPSO = nullptr;
        m_pRenderDevice->CreatePipelineState( PSODesc, ppPSO );
        if( *ppPSO == nullptr )
            SCRIPT_PARSING_ERROR( L, "Failed to create Pipeline State Object state object" )

        CHECK_LUA_STACK_HEIGHT( +1 );
    }

    int PSODescParser::SetPSO( lua_State *L )
    {
        auto *pPSO = *GetUserData<IPipelineState**>( L, 1, m_MetatableRegistryName.c_str() );
        
        auto *pContext = LoadDeviceContextFromRegistry( L );
        pContext->SetPipelineState( pPSO );

        return 0;
    }
}
