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
#include "PSODescParser.h"
#include "BlendStateDescParser.h"
#include "RasterizerStateDescParser.h"
#include "DepthStencilStateDescParser.h"
#include "InputLayoutDescParser.h"

namespace std 
{
    DEFINE_ENUM_HASH( Diligent::PRIMITIVE_TOPOLOGY )
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
                    GraphicsPipeline.NumRenderTargets = std::max(GraphicsPipeline.NumRenderTargets, static_cast<Uint8>(NewArrayIndex+1));
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
            using CountType = decltype(SampleDesc::Count);
            DEFINE_BINDER_EX( m_Bindings, SampleDesc, Count, CountType, Validator<CountType>("Count", 1,32) );
            using QualityType = decltype(SampleDesc::Quality);
            DEFINE_BINDER_EX( m_Bindings, SampleDesc, Quality, QualityType, Validator<QualityType>("Quality", 0,std::numeric_limits<QualityType>::max()) );
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

            DEFINE_BINDER_EX( m_Bindings, GraphicsPipelineDesc, pVS, EngineObjectPtrLoader<IShader>, AllowedMetatable );
            DEFINE_BINDER_EX( m_Bindings, GraphicsPipelineDesc, pPS, EngineObjectPtrLoader<IShader>, AllowedMetatable );
            DEFINE_BINDER_EX( m_Bindings, GraphicsPipelineDesc, pDS, EngineObjectPtrLoader<IShader>, AllowedMetatable );
            DEFINE_BINDER_EX( m_Bindings, GraphicsPipelineDesc, pHS, EngineObjectPtrLoader<IShader>, AllowedMetatable );
            DEFINE_BINDER_EX( m_Bindings, GraphicsPipelineDesc, pGS, EngineObjectPtrLoader<IShader>, AllowedMetatable );

            //D3D12_STREAM_OUTPUT_DESC StreamOutput;

            DEFINE_BINDER_EX( m_Bindings, GraphicsPipelineDesc, BlendDesc, BlendStateDesc, 0 );
            DEFINE_BINDER( m_Bindings, GraphicsPipelineDesc, SampleMask );
            DEFINE_BINDER_EX( m_Bindings, GraphicsPipelineDesc, RasterizerDesc, RasterizerStateDesc, 0 );
            DEFINE_BINDER_EX( m_Bindings, GraphicsPipelineDesc, DepthStencilDesc, DepthStencilStateDesc, 0 );

            auto *pLayoutElemBinder = 
                new MemberBinder<InputLayoutDesc>( 
                    offsetof(GraphicsPipelineDesc, InputLayout), 
                    offsetof(PSODescParser::PSODescWrapper, LayoutElementsBuffer) - offsetof(PSODescParser::PSODescWrapper, GraphicsPipeline)
                );
            m_Bindings.insert( std::make_pair( "InputLayout", std::unique_ptr<MemberBinderBase>(pLayoutElemBinder) ) );

            //D3D12_INDEX_BUFFER_STRIP_CUT_VALUE IBStripCutValue;

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
            DEFINE_ENUM_BINDER( m_Bindings, GraphicsPipelineDesc, PrimitiveTopology, m_PrimTopologyEnumMapping);

            DEFINE_BINDER_EX( m_Bindings, GraphicsPipelineDesc, RTVFormats, RTVFormatsParser, 0 );
            DEFINE_ENUM_BINDER( m_Bindings, GraphicsPipelineDesc, DSVFormat, m_TexFmtEnumMapping );
            
            DEFINE_BINDER_EX( m_Bindings, GraphicsPipelineDesc, SmplDesc, SampleDesc, 0 );

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
        EnumMapping < PRIMITIVE_TOPOLOGY > m_PrimTopologyEnumMapping;
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
            DEFINE_BINDER_EX( m_Bindings, ComputePipelineDesc, pCS, EngineObjectPtrLoader<IShader>, AllowedMetatable );
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
        m_SetPSOBinding( this, L, "Context", "SetPipelineState", &PSODescParser::SetPSO ),
        m_IsCompatibleWithBinding(this, L, m_MetatableRegistryName.c_str(), "IsCompatibleWith", &PSODescParser::IsCompatibleWith)
    {
        DEFINE_BUFFERED_STRING_BINDER( m_Bindings, PSODescWrapper, Name, NameBuffer )

        DEFINE_BINDER( m_Bindings, PSODescWrapper, IsComputePipeline );
        using SRBAllocationGranularityType  = decltype(PSODescWrapper::SRBAllocationGranularity);
        Validator<SRBAllocationGranularityType> SRBValidator("SRBAllocationGranularity", 1, 65536);
        DEFINE_BINDER_EX( m_Bindings, PSODescWrapper, SRBAllocationGranularity, SRBAllocationGranularityType, SRBValidator);

        DEFINE_BINDER_EX( m_Bindings, PSODescWrapper, GraphicsPipeline, GraphicsPipelineDesc, 0 );
        DEFINE_BINDER_EX( m_Bindings, PSODescWrapper, ComputePipeline, ComputePipelineDesc, 0 );
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

    int PSODescParser::IsCompatibleWith(lua_State *L)
    {
        INIT_LUA_STACK_TRACKING(L);

        auto *pThisPSO = *GetUserData<IPipelineState**>(L, 1, m_MetatableRegistryName.c_str());
        
        // Buffer should be the first argument
        auto *pPSO = *GetUserData<IPipelineState**>(L, 2, m_MetatableRegistryName.c_str());

        auto IsCompatible = pThisPSO->IsCompatibleWith(pPSO);

        // Push existing object
        PushValue(L, IsCompatible);

        // Returning one value to Lua
        return 1;
    }
}
