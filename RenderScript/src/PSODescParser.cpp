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
#include "PSODescParser.h"
#include "BlendStateDescParser.h"
#include "RasterizerStateDescParser.h"
#include "DepthStencilStateDescParser.h"
#include "InputLayoutDescParser.h"
#include "SamplerParser.h"

namespace Diligent
{
    class ShaderVariableTypeEnumMapping : public EnumMapping < Diligent::SHADER_RESOURCE_VARIABLE_TYPE >
    {
    public:
        ShaderVariableTypeEnumMapping()
        {
            DEFINE_ENUM_ELEMENT_MAPPING( (*this), SHADER_RESOURCE_VARIABLE_TYPE_STATIC );
            DEFINE_ENUM_ELEMENT_MAPPING( (*this), SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE );
            DEFINE_ENUM_ELEMENT_MAPPING( (*this), SHADER_RESOURCE_VARIABLE_TYPE_DYNAMIC );
        }
    };

    class ShaderResourceVariableDescArrayParser;
    template<>
    class MemberBinder<ShaderResourceVariableDescArrayParser> : public MemberBinderBase
    {
    public:
        MemberBinder( size_t VariableDescOffset, 
                      size_t NumVariablesOffset,
                      size_t VarDescBufferOffset,
                      size_t VarNamesBufferOffset ) :
            MemberBinderBase( VariableDescOffset ),
            m_NumVariablesOffset(NumVariablesOffset),
            m_VarDescBufferOffset(VarDescBufferOffset),
            m_VarNamesBufferOffset(VarNamesBufferOffset)
        {
            DEFINE_ENUM_BINDER( m_Bindings, ShaderResourceVariableDesc, Type, m_ShaderVarTypeEnumMapping );
            DEFINE_FLAGS_BINDER( m_Bindings, ShaderResourceVariableDesc, ShaderStages, SHADER_TYPE, m_ShaderTypeEnumMapping );
        }

        virtual void GetValue( lua_State *L, const void* pBasePointer )override
        {
            // Use raw pointer to push the value to Lua because the buffer
            // most likely does not exist
            const auto &VarDesc = GetMemberByOffest<ShaderResourceVariableDesc*>( pBasePointer, m_MemberOffset);
            const auto &NumVars = GetMemberByOffest<Uint32>( pBasePointer, m_NumVariablesOffset);

            PushLuaArray( L, VarDesc, VarDesc + NumVars, [&]( const ShaderResourceVariableDesc &VarDesc )
            {
                // Push variable type. The function will leave the new table on top
                // of the stack
                PushLuaTable( L, &VarDesc, m_Bindings ); // Stack: +1
                
                // Push name into the same table
                lua_pushstring( L, "Name" ); // Stack: +2
                lua_pushstring( L, VarDesc.Name); // Stack: +3
                lua_settable( L, -3 ); // Stack: +1
            }
            );
        }

        virtual void SetValue( lua_State *L, int Index, void* pBasePointer )override
        {
            auto &ShaderVarDescBuffer = GetMemberByOffest<std::vector<ShaderResourceVariableDesc>>( pBasePointer, m_VarDescBufferOffset);
            auto &ShaderNamesBuffer = GetMemberByOffest<std::vector<String>>( pBasePointer, m_VarNamesBufferOffset);

            ParseLuaArray( L, Index, pBasePointer, 
                           [&]( void* _pBasePointer, int StackIndex, int NewArrayIndex )
                           {
                               VERIFY_EXPR( pBasePointer == _pBasePointer );
                           
                               auto CurrIndex = ShaderVarDescBuffer.size();
                               if( static_cast<int>(CurrIndex) != NewArrayIndex - 1 )
                                   SCRIPT_PARSING_ERROR( L, "Explicit array indices are not allowed in shader name description.  Provided index ", NewArrayIndex - 1, " conflicts with actual index ", CurrIndex, "." );
                               ShaderVarDescBuffer.resize( CurrIndex + 1 );
                               ShaderNamesBuffer.resize( CurrIndex + 1 );
                               ParseLuaTable( L, StackIndex, &(ShaderVarDescBuffer)[CurrIndex], 
                                              [&](int TblStackInd, void* __pBasePointer, const char *Key) 
                                              {
                                                  auto Binding = m_Bindings.find( Key );
                                                  if( Binding != m_Bindings.end() )
                                                  {
                                                      Binding->second->SetValue( L, TblStackInd, __pBasePointer );
                                                  }
                                                  else if (strcmp(Key, "Name") == 0)
                                                  {
                                                      auto Name = ReadValueFromLua<const Char*>(L, TblStackInd);
                                                      ShaderNamesBuffer[CurrIndex] = Name;
                                                  }
                                                  else
                                                  {
                                                      SCRIPT_PARSING_ERROR( L, "Unknown Member \"", Key, '\"' );
                                                  }
                                              }
                                );

                            if( ShaderNamesBuffer[CurrIndex].length() == 0 )
                                SCRIPT_PARSING_ERROR(L, "Missing shader variable name")
                        }
            );

            for(size_t v=0; v < ShaderVarDescBuffer.size(); ++v)
            {
                ShaderVarDescBuffer[v].Name = ShaderNamesBuffer[v].c_str();
            }

            auto &VarDesc = GetMemberByOffest<ShaderResourceVariableDesc*>( pBasePointer, m_MemberOffset);
            auto &NumVars = GetMemberByOffest<Uint32>( pBasePointer, m_NumVariablesOffset);
            NumVars = static_cast<Uint32>(ShaderVarDescBuffer.size());
            VarDesc = NumVars ? ShaderVarDescBuffer.data() : nullptr;
        }    
    private:
        BindingsMapType m_Bindings;
        ShaderVariableTypeEnumMapping m_ShaderVarTypeEnumMapping;
        ShaderTypeEnumMapping m_ShaderTypeEnumMapping;
        size_t m_NumVariablesOffset;
        size_t m_VarDescBufferOffset;
        size_t m_VarNamesBufferOffset;
    };


    class StaticSamplerDescArrayParser;
    template<>
    class MemberBinder<StaticSamplerDescArrayParser> : public MemberBinderBase
    {
    public:
        MemberBinder( size_t StaticSamplerDescOffset, 
                      size_t NumStaticSamplersOffset,
                      size_t StaticSamplersBufferOffset,
                      size_t StaticSamplerTexNamesBufferOffset ) :
            MemberBinderBase( StaticSamplerDescOffset ),
            m_NumStaticSamplersOffset(NumStaticSamplersOffset),
            m_StaticSamplersBufferOffset(StaticSamplersBufferOffset),
            m_StaticSamplerTexNamesBufferOffset(StaticSamplerTexNamesBufferOffset)
        {
            DEFINE_FLAGS_BINDER( m_Bindings, StaticSamplerDesc, ShaderStages, SHADER_TYPE, m_ShaderTypeEnumMapping );
            InitSamplerParserBindings<SamplerDesc>(m_SamDescBindings, m_FilterTypeEnumMapping, m_TexAddrModeEnumMapping, m_CmpFuncEnumMapping);
        }

        virtual void GetValue( lua_State *L, const void* pBasePointer )override
        {
            // Use raw pointer to push the value to Lua because the buffer
            // most likely does not exist
            const auto &StaticSamplers = GetMemberByOffest<StaticSamplerDesc*>( pBasePointer, m_MemberOffset);
            const auto &NumStaticSamplers = GetMemberByOffest<Uint32>( pBasePointer, m_NumStaticSamplersOffset);

            PushLuaArray( L, StaticSamplers, StaticSamplers + NumStaticSamplers, [&]( const StaticSamplerDesc &SamDesc )
            {
                // Push variable type. The function will leave the new table on top
                // of the stack
                PushLuaTable( L, &SamDesc, m_Bindings ); // Stack: +1

                // Push "Desc" field
                lua_pushstring( L, "Desc" ); // Stack: +2
                // Push members of StaticSamplerDesc::Desc. The function will leave new table on top
                // of the stack
                PushLuaTable( L, &SamDesc.Desc, m_SamDescBindings ); // Stack: +3
                // Push the table from the top into the parent table
                lua_settable( L, -3 ); // Stack: +1

                // Push "SamplerOrTextureName" field
                lua_pushstring( L, "SamplerOrTextureName" ); // Stack: +2
                lua_pushstring( L, SamDesc.SamplerOrTextureName); // Stack: +3
                lua_settable( L, -3 ); // Stack: +1
            }
            );
        }

        virtual void SetValue( lua_State *L, int Index, void* pBasePointer )override
        {
            auto &StaticSamplersBuffer = GetMemberByOffest<std::vector<StaticSamplerDesc>>( pBasePointer, m_StaticSamplersBufferOffset);
            auto &StaticSamplerTexNamesBuffer = GetMemberByOffest<std::vector<String>>( pBasePointer, m_StaticSamplerTexNamesBufferOffset);

            ParseLuaArray( L, Index, pBasePointer, 
                           [&]( void* _pBasePointer, int StackIndex, int NewArrayIndex )
                           {
                               VERIFY_EXPR( pBasePointer == _pBasePointer );
                           
                               auto CurrIndex = StaticSamplersBuffer.size();
                               if(static_cast<int>(CurrIndex) != NewArrayIndex - 1 )
                                   SCRIPT_PARSING_ERROR( L, "Explicit array indices are not allowed in static sampler description.  Provided index ", NewArrayIndex - 1, " conflicts with actual index ", CurrIndex, "." );
                               StaticSamplersBuffer.resize( CurrIndex + 1 );
                               StaticSamplerTexNamesBuffer.resize( CurrIndex + 1 );
                               ParseLuaTable( L, StackIndex, &(StaticSamplersBuffer)[CurrIndex], 
                                              [&](int TblStackInd, void* __pBasePointer, const char *Key) 
                                              {
                                                  auto Binding = m_Bindings.find( Key );
                                                  if( Binding != m_Bindings.end() )
                                                  {
                                                      Binding->second->SetValue( L, TblStackInd, __pBasePointer );
                                                  }
                                                  else if (strcmp(Key, "Desc") == 0)
                                                  {
                                                       ParseLuaTable( L, StackIndex, &(StaticSamplersBuffer)[CurrIndex].Desc, 
                                                                      [&](int TblStackInd, void* __pBasePointer, const char *Key) 
                                                                      {    
                                                                           if (strcmp(Key, "Name") == 0)
                                                                           {
                                                                               UNSUPPORTED("Parsing of the static sampler name is not implemented");
                                                                           }
                                                                           else
                                                                           {
                                                                               auto Binding = m_SamDescBindings.find( Key );
                                                                               if (Binding != m_SamDescBindings.end())
                                                                               {
                                                                                   Binding->second->SetValue( L, TblStackInd, __pBasePointer );
                                                                               }
                                                                               else
                                                                               {
                                                                                   SCRIPT_PARSING_ERROR( L, "Unknown Member \"", Key, '\"' );
                                                                               }
                                                                           }
                                                                       }
                                                                     );
                                                  }
                                                  else if (strcmp(Key, "SamplerOrTextureName") == 0)
                                                  {
                                                      auto Name = ReadValueFromLua<const Char*>(L, TblStackInd);
                                                      StaticSamplerTexNamesBuffer[CurrIndex] = Name;
                                                  }
                                                  else
                                                  {
                                                      SCRIPT_PARSING_ERROR( L, "Unknown Member \"", Key, '\"' );
                                                  }
                                              }
                                );

                            if( StaticSamplerTexNamesBuffer[CurrIndex].length() == 0 )
                                SCRIPT_PARSING_ERROR(L, "Missing static sampler texture name")
                        }
            );

            for(size_t v=0; v < StaticSamplersBuffer.size(); ++v)
            {
                StaticSamplersBuffer[v].SamplerOrTextureName = StaticSamplerTexNamesBuffer[v].c_str();
            }

            auto &StaticSamplers = GetMemberByOffest<StaticSamplerDesc*>( pBasePointer, m_MemberOffset);
            auto &NumStaticSamplers = GetMemberByOffest<Uint32>( pBasePointer, m_NumStaticSamplersOffset);
            NumStaticSamplers = static_cast<Uint32>(StaticSamplersBuffer.size());
            StaticSamplers = NumStaticSamplers ? StaticSamplersBuffer.data() : nullptr;
        }    
    private:
        BindingsMapType m_Bindings;
        BindingsMapType m_SamDescBindings;
        EnumMapping<FILTER_TYPE>           m_FilterTypeEnumMapping;
        EnumMapping<TEXTURE_ADDRESS_MODE>  m_TexAddrModeEnumMapping;
        ComparisonFuncEnumMapping          m_CmpFuncEnumMapping;
        ShaderTypeEnumMapping              m_ShaderTypeEnumMapping;

        size_t m_NumStaticSamplersOffset;
        size_t m_StaticSamplersBufferOffset;
        size_t m_StaticSamplerTexNamesBufferOffset;
    };


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
    class MemberBinder<PipelineResourceLayoutDesc> : public MemberBinderBase
    {
    public:
        MemberBinder( size_t MemberOffset, size_t VarDescBufferOffset, size_t VarNamesBufferOffset, size_t StaticSamplersBufferOffset, size_t StaticSamplerTexNamesBufferOffset ) :
            MemberBinderBase( MemberOffset )
        {
            DEFINE_ENUM_BINDER( m_Bindings, PipelineResourceLayoutDesc, DefaultVariableType, m_ShaderVarTypeEnumMapping );

            auto *pShaderDescBinder = 
                new MemberBinder<ShaderResourceVariableDescArrayParser>( 
                    offsetof(PipelineResourceLayoutDesc, Variables), 
                    offsetof(PipelineResourceLayoutDesc, NumVariables), 
                    VarDescBufferOffset, 
                    VarNamesBufferOffset
                );
            m_Bindings.insert( std::make_pair( "Variables", std::unique_ptr<MemberBinderBase>(pShaderDescBinder) ) );

            auto *pStaticSamplerDescBinder = 
                new MemberBinder<StaticSamplerDescArrayParser>( 
                    offsetof(PipelineResourceLayoutDesc, StaticSamplers), 
                    offsetof(PipelineResourceLayoutDesc, NumStaticSamplers), 
                    StaticSamplersBufferOffset, 
                    StaticSamplerTexNamesBufferOffset
                );
            m_Bindings.insert( std::make_pair( "StaticSamplers", std::unique_ptr<MemberBinderBase>(pStaticSamplerDescBinder) ) );
        }

        virtual void GetValue( lua_State *L, const void* pBasePointer )
        {
            const auto *pShaderDesc = &GetMemberByOffest<ShaderDesc>( pBasePointer, m_MemberOffset );
            PushLuaTable( L, pShaderDesc, m_Bindings );
        }

        virtual void SetValue( lua_State *L, int Index, void* pBasePointer )
        {
            auto *pShaderDesc = &GetMemberByOffest<ShaderDesc>(pBasePointer, m_MemberOffset);
            ParseLuaTable( L, Index, pShaderDesc, m_Bindings );
        }

    private:
        BindingsMapType m_Bindings;
        ShaderVariableTypeEnumMapping m_ShaderVarTypeEnumMapping;
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
    PSODescParser::PSODescParser( IRenderDevice *pRenderDevice, lua_State *L, const String& ResMappingMetatableName  ) :
        EngineObjectParserCommon<IPipelineState>( pRenderDevice, L, PSODescLibName ),
        m_SetPSOBinding( this, L, "Context", "SetPipelineState", &PSODescParser::SetPSO ),
        m_IsCompatibleWithBinding(this, L, m_MetatableRegistryName.c_str(), "IsCompatibleWith", &PSODescParser::IsCompatibleWith),
        m_ResMappingMetatableName(ResMappingMetatableName),
        m_BindStaticResourcesBinding( this, L, m_MetatableRegistryName.c_str(), "BindStaticResources", &PSODescParser::BindStaticResources )
    {
        DEFINE_BUFFERED_STRING_BINDER( m_Bindings, PSODescWrapper, Name, NameBuffer )

        DEFINE_BINDER( m_Bindings, PSODescWrapper, IsComputePipeline );
        using SRBAllocationGranularityType  = decltype(PSODescWrapper::SRBAllocationGranularity);
        Validator<SRBAllocationGranularityType> SRBValidator("SRBAllocationGranularity", 1, 65536);
        DEFINE_BINDER_EX( m_Bindings, PSODescWrapper, SRBAllocationGranularity, SRBAllocationGranularityType, SRBValidator);

        auto *pShaderDescBinder = 
            new MemberBinder<PipelineResourceLayoutDesc>( 
                offsetof(PSODescWrapper, ResourceLayout), 
                offsetof(PSODescWrapper, m_VarDescBuffer) - offsetof(PSODescWrapper, ResourceLayout), 
                offsetof(PSODescWrapper, m_VarNamesBuffer) - offsetof(PSODescWrapper, ResourceLayout),
                offsetof(PSODescWrapper, m_StaticSamplersBuffer) - offsetof(PSODescWrapper, ResourceLayout),
                offsetof(PSODescWrapper, m_StaticSamplerTexNamesBuffer) - offsetof(PSODescWrapper, ResourceLayout)
            );
        m_Bindings.insert( std::make_pair( "ResourceLayout", std::unique_ptr<MemberBinderBase>(pShaderDescBinder) ) );

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

    int PSODescParser::BindStaticResources( lua_State *L )
    {
        // In order to communicate properly with Lua, a C function must use the following protocol, 
        // which defines the way parameters and results are passed : a C function receives its arguments 
        // from Lua in its PRIVATE stack in direct order( the first argument is pushed first ).So, when the 
        // function starts, lua_gettop( L ) returns the number of arguments received by the function.The first 
        // argument( if any ) is at index 1 and its last argument is at index lua_gettop( L ).
        // To return values to Lua, a C function just pushes them onto the stack, in direct order 
        // ( the first result is pushed first ), and returns the number of results. 
        // Any other value in the stack below the results will be properly discarded by Lua. 
        // Like a Lua function, a C function called by Lua can also return many results.

        try
        {
            auto NumArgs = lua_gettop( L );
            if( NumArgs < 3 )
            {
                SCRIPT_PARSING_ERROR( L, "At least 2 arguments (shader flags and resource mapping) are expected" );
            }

            int ArgStackInd = 1;

            auto *pPSO = *GetUserData<IPipelineState**>( L, ArgStackInd, m_MetatableRegistryName.c_str() );
            VERIFY( pPSO, "PSO pointer is null" );
            if( !pPSO )return 0;

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


            pPSO->BindStaticResources( ShaderFlags, pResourceMapping, Flags );
        }
        catch( const std::runtime_error& )
        {

        }

        return 0;
    }
}
