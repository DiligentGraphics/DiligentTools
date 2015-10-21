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
#include "ShaderParser.h"
#include "BasicShaderSourceStreamFactory.h"

namespace std
{
    DEFINE_ENUM_HASH( Diligent::SHADER_TYPE )
}

namespace Diligent
{
    const Char* ShaderParser::ShaderLibName = "Shader";

    class ShaderDescLoader;

    template<>
    class MemberBinder<ShaderDescLoader> : public MemberBinderBase
    {
    public:
        MemberBinder( size_t MemberOffset, size_t Dummy ) :
            MemberBinderBase( MemberOffset )
        {
            auto *pNameBinder = new BufferedStringBinder( offsetof( ShaderDesc, Name ), 
                // This is a small hack: we need to compute an offset from the beginning of ShaderDesc structure to
                // the NameBuffer member of ShaderCreationAttribsWrapper:
                offsetof( ShaderParser::ShaderCreationAttribsWrapper, NameBuffer ) - offsetof( ShaderParser::ShaderCreationAttribsWrapper, Desc.Name ) );
            // No need to make a copy of Name since it is constant and HashMapStringKey will simply
            // keep pointer to it
            m_Bindings.insert( std::make_pair( "Name", std::unique_ptr<MemberBinderBase>(pNameBinder) ) );

            DEFINE_ENUM_ELEMENT_MAPPING( m_ShaderTypeEnumMapping, SHADER_TYPE_UNKNOWN );
            DEFINE_ENUM_ELEMENT_MAPPING( m_ShaderTypeEnumMapping, SHADER_TYPE_VERTEX );
            DEFINE_ENUM_ELEMENT_MAPPING( m_ShaderTypeEnumMapping, SHADER_TYPE_PIXEL );
            DEFINE_ENUM_ELEMENT_MAPPING( m_ShaderTypeEnumMapping, SHADER_TYPE_GEOMETRY );
            DEFINE_ENUM_ELEMENT_MAPPING( m_ShaderTypeEnumMapping, SHADER_TYPE_HULL );
            DEFINE_ENUM_ELEMENT_MAPPING( m_ShaderTypeEnumMapping, SHADER_TYPE_DOMAIN );
            DEFINE_ENUM_ELEMENT_MAPPING( m_ShaderTypeEnumMapping, SHADER_TYPE_COMPUTE );
            DEFINE_ENUM_BINDER( m_Bindings, ShaderDesc, ShaderType, SHADER_TYPE, m_ShaderTypeEnumMapping )
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
        EnumMapping<SHADER_TYPE> m_ShaderTypeEnumMapping;
    };

    ShaderParser::ShaderParser( IRenderDevice *pRenderDevice, lua_State *L, const String& ResMappingMetatableName ) :
        EngineObjectParserCommon<IShader>( pRenderDevice, L, ShaderLibName ),
        m_SetShadersBinding( this, L, "Context", "SetShaders", &ShaderParser::SetShaders ),
        m_BindResourcesBinding( this, L, m_MetatableRegistryName.c_str(), "BindResources", &ShaderParser::BindResources ),
        m_ResMappingMetatableName(ResMappingMetatableName)
    {
        DEFINE_BUFFERED_STRING_BINDER( m_Bindings, ShaderCreationAttribsWrapper, FilePath, FilePathBuffer )
        DEFINE_BUFFERED_STRING_BINDER( m_Bindings, ShaderCreationAttribsWrapper, EntryPoint, EntryPointBuffer )
        DEFINE_BUFFERED_STRING_BINDER( m_Bindings, ShaderCreationAttribsWrapper, SearchDirectories, SearchDirectoriesBuffer )

        DEFINE_ENUM_ELEMENT_MAPPING( m_ShaderSourceLangEnumMapping, SHADER_SOURCE_LANGUAGE_DEFAULT );
        DEFINE_ENUM_ELEMENT_MAPPING( m_ShaderSourceLangEnumMapping, SHADER_SOURCE_LANGUAGE_HLSL );
        DEFINE_ENUM_ELEMENT_MAPPING( m_ShaderSourceLangEnumMapping, SHADER_SOURCE_LANGUAGE_GLSL );
        DEFINE_ENUM_BINDER( m_Bindings, ShaderCreationAttribsWrapper, SourceLanguage, SHADER_SOURCE_LANGUAGE, m_ShaderSourceLangEnumMapping );

        DEFINE_BINDER( m_Bindings, ShaderCreationAttribsWrapper, Desc, ShaderDescLoader, 0 )
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

    int ShaderParser::SetShaders( lua_State *L )
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

        auto NumArgs = lua_gettop( L );
        const int MaxShaders = 6;
        IShader* pShaders[MaxShaders] = {};
        VERIFY( NumArgs <= MaxShaders, "Too many shaders are being set" );
        NumArgs = std::min<int>( NumArgs, MaxShaders );
        for( int i = 0; i < NumArgs; ++i )
        {
            pShaders[i] = *GetUserData<IShader**>(L, i+1, m_MetatableRegistryName.c_str() );
        }
        auto *pContext = LoadDeviceContextFromRegistry( L );
        pContext->SetShaders( pShaders, NumArgs );
        
        // Returning no arguments
        return 0;
    }

    int ShaderParser::BindResources( lua_State *L )
    {
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
