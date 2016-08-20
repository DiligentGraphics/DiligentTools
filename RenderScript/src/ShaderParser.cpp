/*     Copyright 2015-2016 Egor Yusov
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
#include "ShaderParser.h"
#include "BasicShaderSourceStreamFactory.h"
#include "SamplerParser.h"


namespace std
{
    DEFINE_ENUM_HASH( Diligent::SHADER_VARIABLE_TYPE )
}

namespace Diligent
{
    const Char* ShaderParser::ShaderLibName = "Shader";


    class ShaderVariableTypeEnumMapping : public EnumMapping < Diligent::SHADER_VARIABLE_TYPE >
    {
    public:
        ShaderVariableTypeEnumMapping()
        {
            DEFINE_ENUM_ELEMENT_MAPPING( (*this), SHADER_VARIABLE_TYPE_STATIC );
            DEFINE_ENUM_ELEMENT_MAPPING( (*this), SHADER_VARIABLE_TYPE_MUTABLE );
            DEFINE_ENUM_ELEMENT_MAPPING( (*this), SHADER_VARIABLE_TYPE_DYNAMIC );
        }
    };

    class ShaderVariableDescArrayParser;
    template<>
    class MemberBinder<ShaderVariableDescArrayParser> : public MemberBinderBase
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
            DEFINE_ENUM_BINDER( m_Bindings, ShaderVariableDesc, Type, SHADER_VARIABLE_TYPE, m_ShaderVarTypeEnumMapping )
        }

        virtual void GetValue( lua_State *L, const void* pBasePointer )override
        {
            // Use raw pointer to push the value to Lua because the buffer
            // most likely does not exist
            const auto &VarDesc = GetMemberByOffest<ShaderVariableDesc*>( pBasePointer, m_MemberOffset);
            const auto &NumVars = GetMemberByOffest<Uint32>( pBasePointer, m_NumVariablesOffset);

            PushLuaArray( L, VarDesc, VarDesc + NumVars, [&]( const ShaderVariableDesc &VarDesc )
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

        virtual void SetValue( lua_State *L, int Index, void* pBasePointer )
        {
            auto &ShaderVarDescBuffer = GetMemberByOffest<std::vector<ShaderVariableDesc>>( pBasePointer, m_VarDescBufferOffset);
            auto &ShaderNamesBuffer = GetMemberByOffest<std::vector<String>>( pBasePointer, m_VarNamesBufferOffset);

            ParseLuaArray( L, Index, pBasePointer, 
                           [&]( void* _pBasePointer, int StackIndex, int NewArrayIndex )
                           {
                               VERIFY_EXPR( pBasePointer == _pBasePointer );
                           
                               auto CurrIndex = ShaderVarDescBuffer.size();
                               if( CurrIndex != NewArrayIndex - 1 )
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

            auto &VarDesc = GetMemberByOffest<ShaderVariableDesc*>( pBasePointer, m_MemberOffset);
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
            InitSamplerParserBindings<SamplerDesc>(m_Bindings, m_FilterTypeEnumMapping, m_TexAddrModeEnumMapping, m_CmpFuncEnumMapping);
        }

        virtual void GetValue( lua_State *L, const void* pBasePointer )override
        {
            // Use raw pointer to push the value to Lua because the buffer
            // most likely does not exist
            const auto &StaticSamplers = GetMemberByOffest<StaticSamplerDesc*>( pBasePointer, m_MemberOffset);
            const auto &NumStaticSamplers = GetMemberByOffest<Uint32>( pBasePointer, m_NumStaticSamplersOffset);

            PushLuaArray( L, StaticSamplers, StaticSamplers + NumStaticSamplers, [&]( const StaticSamplerDesc &SamDesc )
            {
                // Push new table to hold variables of StaticSamplerDesc struct
                lua_newtable(L); // Stack: +1

                // Push "Desc" field
                lua_pushstring( L, "Desc" ); // Stack: +2
                // Push members of StaticSamplerDesc::Desc. The function will leave new table on top
                // of the stack
                PushLuaTable( L, &SamDesc.Desc, m_Bindings ); // Stack: +3
                // Push the table from the top into the parent table
                lua_settable( L, -3 ); // Stack: +1

                // Push "TextureName" field
                lua_pushstring( L, "TextureName" ); // Stack: +2
                lua_pushstring( L, SamDesc.TextureName); // Stack: +3
                lua_settable( L, -3 ); // Stack: +1
            }
            );
        }

        virtual void SetValue( lua_State *L, int Index, void* pBasePointer )
        {
            auto &StaticSamplersBuffer = GetMemberByOffest<std::vector<StaticSamplerDesc>>( pBasePointer, m_StaticSamplersBufferOffset);
            auto &StaticSamplerTexNamesBuffer = GetMemberByOffest<std::vector<String>>( pBasePointer, m_StaticSamplerTexNamesBufferOffset);

            ParseLuaArray( L, Index, pBasePointer, 
                           [&]( void* _pBasePointer, int StackIndex, int NewArrayIndex )
                           {
                               VERIFY_EXPR( pBasePointer == _pBasePointer );
                           
                               auto CurrIndex = StaticSamplersBuffer.size();
                               if( CurrIndex != NewArrayIndex - 1 )
                                   SCRIPT_PARSING_ERROR( L, "Explicit array indices are not allowed in static sampler description.  Provided index ", NewArrayIndex - 1, " conflicts with actual index ", CurrIndex, "." );
                               StaticSamplersBuffer.resize( CurrIndex + 1 );
                               StaticSamplerTexNamesBuffer.resize( CurrIndex + 1 );
                               ParseLuaTable( L, StackIndex, &(StaticSamplersBuffer)[CurrIndex], 
                                              [&](int TblStackInd, void* __pBasePointer, const char *Key) 
                                              {
                                                  if (strcmp(Key, "Desc") == 0)
                                                  {
                                                       ParseLuaTable( L, StackIndex, &(StaticSamplersBuffer)[CurrIndex].Desc, 
                                                                      [&](int TblStackInd, void* __pBasePointer, const char *Key) 
                                                                      {    
                                                                           if (strcmp(Key, "Name") == 0)
                                                                           {
                                                                               UNSUPPORTED("Parsing of the static sampler name is not implemented")
                                                                           }
                                                                           else
                                                                           {
                                                                               auto Binding = m_Bindings.find( Key );
                                                                               if (Binding != m_Bindings.end())
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
                                                  else if (strcmp(Key, "TextureName") == 0)
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
                StaticSamplersBuffer[v].TextureName = StaticSamplerTexNamesBuffer[v].c_str();
            }

            auto &StaticSamplers = GetMemberByOffest<StaticSamplerDesc*>( pBasePointer, m_MemberOffset);
            auto &NumStaticSamplers = GetMemberByOffest<Uint32>( pBasePointer, m_NumStaticSamplersOffset);
            NumStaticSamplers = static_cast<Uint32>(StaticSamplersBuffer.size());
            StaticSamplers = NumStaticSamplers ? StaticSamplersBuffer.data() : nullptr;
        }    
    private:
        BindingsMapType m_Bindings;
        EnumMapping<FILTER_TYPE>           m_FilterTypeEnumMapping;
        EnumMapping<TEXTURE_ADDRESS_MODE>  m_TexAddrModeEnumMapping;
        ComparisonFuncEnumMapping          m_CmpFuncEnumMapping;

        size_t m_NumStaticSamplersOffset;
        size_t m_StaticSamplersBufferOffset;
        size_t m_StaticSamplerTexNamesBufferOffset;
    };


    class ShaderDescLoader;

    template<>
    class MemberBinder<ShaderDescLoader> : public MemberBinderBase
    {
    public:
        MemberBinder( size_t MemberOffset, size_t VarDescBufferOffset, size_t VarNamesBufferOffset, size_t StaticSamplersBufferOffset, size_t StaticSamplerTexNamesBufferOffset ) :
            MemberBinderBase( MemberOffset )
        {
            auto *pNameBinder = new BufferedStringBinder( offsetof( ShaderDesc, Name ), 
                // This is a small hack: we need to compute an offset from the beginning of ShaderDesc structure to
                // the NameBuffer member of ShaderCreationAttribsWrapper:
                offsetof( ShaderParser::ShaderCreationAttribsWrapper, NameBuffer ) - offsetof( ShaderParser::ShaderCreationAttribsWrapper, Desc.Name ) );
            // No need to make a copy of Name since it is constant and HashMapStringKey will simply
            // keep pointer to it
            m_Bindings.insert( std::make_pair( "Name", std::unique_ptr<MemberBinderBase>(pNameBinder) ) );

            DEFINE_ENUM_BINDER( m_Bindings, ShaderDesc, ShaderType, SHADER_TYPE, m_ShaderTypeEnumMapping )

            DEFINE_ENUM_BINDER( m_Bindings, ShaderDesc, DefaultVariableType, SHADER_VARIABLE_TYPE, m_ShaderVarTypeEnumMapping )

            auto *pShaderDescBinder = 
                new MemberBinder<ShaderVariableDescArrayParser>( 
                    offsetof(ShaderDesc, VariableDesc), 
                    offsetof(ShaderDesc, NumVariables), 
                    VarDescBufferOffset, 
                    VarNamesBufferOffset
                );
            m_Bindings.insert( std::make_pair( "VariableDesc", std::unique_ptr<MemberBinderBase>(pShaderDescBinder) ) );

            auto *pStaticSamplerDescBinder = 
                new MemberBinder<StaticSamplerDescArrayParser>( 
                    offsetof(ShaderDesc, StaticSamplers), 
                    offsetof(ShaderDesc, NumStaticSamplers), 
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
        ShaderTypeEnumMapping m_ShaderTypeEnumMapping;
        ShaderVariableTypeEnumMapping m_ShaderVarTypeEnumMapping;
    };

    ShaderParser::ShaderParser( IRenderDevice *pRenderDevice, lua_State *L, const String& ResMappingMetatableName ) :
        EngineObjectParserCommon<IShader>( pRenderDevice, L, ShaderLibName ),
        m_BindResourcesBinding( this, L, m_MetatableRegistryName.c_str(), "BindResources", &ShaderParser::BindResources ),
        m_ResMappingMetatableName(ResMappingMetatableName)
    {
        DEFINE_BUFFERED_STRING_BINDER( m_Bindings, ShaderCreationAttribsWrapper, FilePath, FilePathBuffer )
        DEFINE_BUFFERED_STRING_BINDER( m_Bindings, ShaderCreationAttribsWrapper, Source, SourceBuffer )
        DEFINE_BUFFERED_STRING_BINDER( m_Bindings, ShaderCreationAttribsWrapper, EntryPoint, EntryPointBuffer )
        DEFINE_BUFFERED_STRING_BINDER( m_Bindings, ShaderCreationAttribsWrapper, SearchDirectories, SearchDirectoriesBuffer )

        DEFINE_ENUM_ELEMENT_MAPPING( m_ShaderSourceLangEnumMapping, SHADER_SOURCE_LANGUAGE_DEFAULT );
        DEFINE_ENUM_ELEMENT_MAPPING( m_ShaderSourceLangEnumMapping, SHADER_SOURCE_LANGUAGE_HLSL );
        DEFINE_ENUM_ELEMENT_MAPPING( m_ShaderSourceLangEnumMapping, SHADER_SOURCE_LANGUAGE_GLSL );
        DEFINE_ENUM_BINDER( m_Bindings, ShaderCreationAttribsWrapper, SourceLanguage, SHADER_SOURCE_LANGUAGE, m_ShaderSourceLangEnumMapping );

        auto *pShaderDescBinder = 
            new MemberBinder<ShaderDescLoader>( 
                offsetof(ShaderCreationAttribsWrapper, Desc), 
                offsetof(ShaderCreationAttribsWrapper, m_VarDescBuffer) - offsetof(ShaderCreationAttribsWrapper, Desc), 
                offsetof(ShaderCreationAttribsWrapper, m_VarNamesBuffer) - offsetof(ShaderCreationAttribsWrapper, Desc),
                offsetof(ShaderCreationAttribsWrapper, m_StaticSamplersBuffer) - offsetof(ShaderCreationAttribsWrapper, Desc),
                offsetof(ShaderCreationAttribsWrapper, m_StaticSamplerTexNamesBuffer) - offsetof(ShaderCreationAttribsWrapper, Desc)
            );
        m_Bindings.insert( std::make_pair( "Desc", std::unique_ptr<MemberBinderBase>(pShaderDescBinder) ) );
    };

    void ShaderParser::CreateObj( lua_State *L )
    {
        INIT_LUA_STACK_TRACKING( L );

        ShaderCreationAttribsWrapper ShaderCreationAttrs;
        ParseLuaTable( L, -1, &ShaderCreationAttrs, m_Bindings );

        BasicShaderSourceStreamFactory BasicSSSFactory(ShaderCreationAttrs.SearchDirectories);
        ShaderCreationAttrs.pShaderSourceStreamFactory = &BasicSSSFactory;

        CHECK_LUA_STACK_HEIGHT();

        IShader **ppShader = reinterpret_cast<IShader**>( lua_newuserdata( L, sizeof( IShader* ) ) );
        *ppShader = nullptr;
        m_pRenderDevice->CreateShader( ShaderCreationAttrs, ppShader );
        if( *ppShader == nullptr )
            SCRIPT_PARSING_ERROR(L, "Failed to create a shader")

        CHECK_LUA_STACK_HEIGHT( +1 );
    }

    void ShaderParser::ReadField( lua_State *L, void *pData, const Char *Field )
    {
        ShaderCreationAttribsWrapper ShaderCreationAttrs;
        auto *ppShader = reinterpret_cast<IShader**>(pData);
        ShaderCreationAttrs.Desc = (*ppShader)->GetDesc();
        PushField( L, &ShaderCreationAttrs, Field, m_Bindings );
    }
    
    int ShaderParser::BindResources( lua_State *L )
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
            if( NumArgs < 2 )
            {
                SCRIPT_PARSING_ERROR( L, "At least 1 argument (resource mapping) is expected" );
            }

            auto *pShader = *GetUserData<IShader**>( L, 1, m_MetatableRegistryName.c_str() );
            VERIFY( pShader, "Shader pointer is null" );
            if( !pShader )return 0;

            auto *pResourceMapping = *GetUserData<IResourceMapping**>( L, 2, m_ResMappingMetatableName.c_str() );
            if( !pResourceMapping )
            {
                SCRIPT_PARSING_ERROR( L, "Incorrect 1st argument type: resource mapping is expected" );
            }

            Uint32 Flags = 0;
            // The last argument may be flags
            const int FlagsArgInd = 3;
            if( NumArgs >= FlagsArgInd &&
                (lua_type( L, FlagsArgInd ) == LUA_TSTRING || lua_type( L, FlagsArgInd ) == LUA_TTABLE ) )
            {
                FlagsLoader<BIND_SHADER_RESOURCES_FLAGS> FlagsLoader(0, "BindShaderResourceFlags", m_BindShaderResFlagEnumMapping);
                FlagsLoader.SetValue( L, FlagsArgInd, &Flags );
            }


            pShader->BindResources( pResourceMapping, Flags );
        }
        catch( const std::runtime_error& )
        {

        }

        return 0;
    }

}
