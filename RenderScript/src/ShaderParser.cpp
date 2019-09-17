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
#include "ShaderParser.h"

namespace Diligent
{
    const Char* ShaderParser::ShaderLibName = "Shader";

    class ShaderDescLoader;

    template<>
    class MemberBinder<ShaderDescLoader> : public MemberBinderBase
    {
    public:
        MemberBinder( size_t MemberOffset ) :
            MemberBinderBase( MemberOffset )
        {
            auto *pNameBinder = new BufferedStringBinder( offsetof( ShaderDesc, Name ), 
                // This is a small hack: we need to compute an offset from the beginning of ShaderDesc structure to
                // the NameBuffer member of ShaderCreateInfoWrapper:
                offsetof( ShaderParser::ShaderCreateInfoWrapper, NameBuffer ) - offsetof( ShaderParser::ShaderCreateInfoWrapper, Desc.Name ) );
            // No need to make a copy of Name since it is constant and HashMapStringKey will simply
            // keep pointer to it
            m_Bindings.insert( std::make_pair( "Name", std::unique_ptr<MemberBinderBase>(pNameBinder) ) );

            DEFINE_ENUM_BINDER( m_Bindings, ShaderDesc, ShaderType, m_ShaderTypeEnumMapping );
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
    };

    ShaderParser::ShaderParser( IRenderDevice *pRenderDevice, lua_State *L) :
        EngineObjectParserCommon<IShader>( pRenderDevice, L, ShaderLibName )
    {
        DEFINE_BUFFERED_STRING_BINDER( m_Bindings, ShaderCreateInfoWrapper, FilePath, FilePathBuffer )
        DEFINE_BUFFERED_STRING_BINDER( m_Bindings, ShaderCreateInfoWrapper, Source, SourceBuffer )
        DEFINE_BUFFERED_STRING_BINDER( m_Bindings, ShaderCreateInfoWrapper, EntryPoint, EntryPointBuffer )
        DEFINE_BUFFERED_STRING_BINDER( m_Bindings, ShaderCreateInfoWrapper, SearchDirectories, SearchDirectoriesBuffer )
        DEFINE_BUFFERED_STRING_BINDER( m_Bindings, ShaderCreateInfoWrapper, CombinedSamplerSuffix, CombinedSamplerSuffixBuffer )

        DEFINE_BINDER(m_Bindings, ShaderCreateInfoWrapper, UseCombinedTextureSamplers);

        DEFINE_ENUM_ELEMENT_MAPPING( m_ShaderSourceLangEnumMapping, SHADER_SOURCE_LANGUAGE_DEFAULT );
        DEFINE_ENUM_ELEMENT_MAPPING( m_ShaderSourceLangEnumMapping, SHADER_SOURCE_LANGUAGE_HLSL );
        DEFINE_ENUM_ELEMENT_MAPPING( m_ShaderSourceLangEnumMapping, SHADER_SOURCE_LANGUAGE_GLSL );
        DEFINE_ENUM_BINDER( m_Bindings, ShaderCreateInfoWrapper, SourceLanguage, m_ShaderSourceLangEnumMapping );

        auto *pShaderDescBinder = 
            new MemberBinder<ShaderDescLoader>( 
                offsetof(ShaderCreateInfoWrapper, Desc)
            );
        m_Bindings.insert( std::make_pair( "Desc", std::unique_ptr<MemberBinderBase>(pShaderDescBinder) ) );
    };

    void ShaderParser::CreateObj( lua_State *L )
    {
        INIT_LUA_STACK_TRACKING( L );

        ShaderCreateInfoWrapper ShaderCreationAttrs;
        ParseLuaTable( L, -1, &ShaderCreationAttrs, m_Bindings );

        RefCntAutoPtr<IShaderSourceInputStreamFactory> pShaderSourceFactory;
        m_pRenderDevice->GetEngineFactory()->CreateDefaultShaderSourceStreamFactory(ShaderCreationAttrs.SearchDirectories, &pShaderSourceFactory);
        ShaderCreationAttrs.pShaderSourceStreamFactory = pShaderSourceFactory;

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
        ShaderCreateInfoWrapper ShaderCreationAttrs;
        auto *ppShader = reinterpret_cast<IShader**>(pData);
        ShaderCreationAttrs.Desc = (*ppShader)->GetDesc();
        PushField( L, &ShaderCreationAttrs, Field, m_Bindings );
    }
}
