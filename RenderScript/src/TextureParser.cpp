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
#include "TextureParser.h"
#include "TextureViewParser.h"

namespace Diligent
{
    const Char* TextureParser::TextureLibName = "Texture";

    template<>
    class MemberBinder<DepthStencilClearValue> : public MemberBinderBase
    {
    public:
        MemberBinder(size_t MemberOffset, int /*Dummy*/) : 
            MemberBinderBase(MemberOffset)
        {
            DEFINE_BINDER_EX( m_Bindings, DepthStencilClearValue, Depth, Float32, Validator<Float32>("Depth clear value", 0.f, 1.f) );
            DEFINE_BINDER( m_Bindings, DepthStencilClearValue, Stencil );
        }

        virtual void GetValue(lua_State *L, const void* pBasePointer)override
        {
            const auto *pDSClearValue = &GetMemberByOffest<DepthStencilClearValue>( pBasePointer, m_MemberOffset );
            PushLuaTable( L, pDSClearValue, m_Bindings );
        }

        virtual void SetValue(lua_State *L, int Index, void* pBasePointer)override
        {
            auto *pDSClearValue = &GetMemberByOffest<DepthStencilClearValue>(pBasePointer, m_MemberOffset);
            ParseLuaTable( L, Index, pDSClearValue, m_Bindings );
        }
    
    private:
        BindingsMapType m_Bindings;
    };

    template<>
    class MemberBinder<OptimizedClearValue> : public MemberBinderBase
    {
    public:
        MemberBinder(size_t MemberOffset, int /*Dummy*/) : 
            MemberBinderBase(MemberOffset)
        {
            DEFINE_ENUM_BINDER( m_Bindings, OptimizedClearValue, Format, m_TexFmtEnumMapping );
            DEFINE_BINDER_EX( m_Bindings, OptimizedClearValue, Color, RGBALoader, 0 );
            DEFINE_BINDER_EX( m_Bindings, OptimizedClearValue, DepthStencil, DepthStencilClearValue, 0 );
        }

        virtual void GetValue(lua_State *L, const void* pBasePointer)override
        {
            const auto *pClearValue = &GetMemberByOffest<OptimizedClearValue>( pBasePointer, m_MemberOffset );
            PushLuaTable( L, pClearValue, m_Bindings );
        }

        virtual void SetValue(lua_State *L, int Index, void* pBasePointer)override
        {
            auto *pClearValue = &GetMemberByOffest<OptimizedClearValue>(pBasePointer, m_MemberOffset);
            ParseLuaTable( L, Index, pClearValue, m_Bindings );
        }
    
    private:
        BindingsMapType m_Bindings;
        TextureFormatEnumMapping m_TexFmtEnumMapping;
    };


    TextureParser::TextureParser( IRenderDevice *pRenderDevice, lua_State *L ) :
        EngineObjectParserCommon<ITexture>( pRenderDevice, L, TextureLibName )
    {
        DEFINE_BUFFERED_STRING_BINDER( m_Bindings, STexDescWrapper, Name, NameBuffer )

        DEFINE_ENUM_BINDER( m_Bindings, STexDescWrapper, Type, m_TexTypeEnumMapping );

        {
            using WidthType = decltype(STexDescWrapper::Width);
            Validator<WidthType> WidthValidator("Width", 1, 16384);
            DEFINE_BINDER_EX( m_Bindings, STexDescWrapper, Width,  WidthType, WidthValidator);
        }

        {
            using HeightType = decltype(STexDescWrapper::Height);
            Validator<HeightType> HeightValidator("Heihght", 1, 16384);
            DEFINE_BINDER_EX( m_Bindings, STexDescWrapper, Height, HeightType, HeightValidator);
        }

        {
            using ArraySizeType = decltype(STexDescWrapper::ArraySize);
            Validator<ArraySizeType> ArraySizeValidator("ArraySize", 1, 16384);
            DEFINE_BINDER_EX( m_Bindings, STexDescWrapper, ArraySize, ArraySizeType, ArraySizeValidator);
        }
        
        {
            using DepthType = decltype(STexDescWrapper::Depth);
            Validator<DepthType> DepthValidator("Depth", 1, 16384);
            DEFINE_BINDER_EX( m_Bindings, STexDescWrapper, Depth,  DepthType, DepthValidator);
        }

        DEFINE_ENUM_BINDER( m_Bindings, STexDescWrapper, Format, m_TexFormatEnumMapping );

        {
            using MipLevelsType = decltype(STexDescWrapper::MipLevels);
            Validator<MipLevelsType> MipLevelsValidator("MipLevels", 1, 20);
            DEFINE_BINDER_EX( m_Bindings, STexDescWrapper, MipLevels, MipLevelsType, MipLevelsValidator);
        }

        {
            using SampleCountType = decltype(STexDescWrapper::SampleCount);
            Validator<SampleCountType> SampleCountValidator("SampleCount", 1, 32);
            DEFINE_BINDER_EX( m_Bindings, STexDescWrapper, SampleCount, SampleCountType, SampleCountValidator);
        }
        
        //DEFINE_ENUM_ELEMENT_MAPPING( m_BindFlagEnumMapping, BIND_VERTEX_BUFFER );
        //DEFINE_ENUM_ELEMENT_MAPPING( m_BindFlagEnumMapping, BIND_INDEX_BUFFER );
        //DEFINE_ENUM_ELEMENT_MAPPING( m_BindFlagEnumMapping, BIND_UNIFORM_BUFFER );
        DEFINE_ENUM_ELEMENT_MAPPING( m_BindFlagEnumMapping, BIND_SHADER_RESOURCE );
        DEFINE_ENUM_ELEMENT_MAPPING( m_BindFlagEnumMapping, BIND_STREAM_OUTPUT );
        DEFINE_ENUM_ELEMENT_MAPPING( m_BindFlagEnumMapping, BIND_RENDER_TARGET );
        DEFINE_ENUM_ELEMENT_MAPPING( m_BindFlagEnumMapping, BIND_DEPTH_STENCIL );
        DEFINE_ENUM_ELEMENT_MAPPING( m_BindFlagEnumMapping, BIND_UNORDERED_ACCESS );
        //DEFINE_ENUM_ELEMENT_MAPPING( m_BindFlagEnumMapping, BIND_INDIRECT_DRAW_ARGS );
        // Explicit namespace declaraion is necesseary to avoid 
        // name conflicts when building for windows store
        DEFINE_FLAGS_BINDER( m_Bindings, STexDescWrapper, BindFlags, Diligent::BIND_FLAGS, m_BindFlagEnumMapping );

        DEFINE_ENUM_BINDER( m_Bindings, STexDescWrapper, Usage, m_UsageEnumMapping );
        DEFINE_FLAGS_BINDER( m_Bindings, STexDescWrapper, CPUAccessFlags, CPU_ACCESS_FLAGS, m_CpuAccessFlagEnumMapping );

        DEFINE_ENUM_ELEMENT_MAPPING( m_MiscFlagEnumMapping, MISC_TEXTURE_FLAG_NONE );
        DEFINE_ENUM_ELEMENT_MAPPING( m_MiscFlagEnumMapping, MISC_TEXTURE_FLAG_GENERATE_MIPS );
        DEFINE_FLAGS_BINDER( m_Bindings, STexDescWrapper, MiscFlags, Diligent::MISC_TEXTURE_FLAGS, m_MiscFlagEnumMapping );

        DEFINE_BINDER_EX( m_Bindings, STexDescWrapper, ClearValue, OptimizedClearValue, 0 );
    };

    void TextureParser::CreateObj( lua_State *L )
    {
        INIT_LUA_STACK_TRACKING(L);
        
        STexDescWrapper TextureDesc;
        ParseLuaTable( L, 1, &TextureDesc, m_Bindings );

        CHECK_LUA_STACK_HEIGHT();

        auto ppTexture = reinterpret_cast<ITexture**>(lua_newuserdata( L, sizeof( ITexture* ) ));
        *ppTexture = nullptr;
        m_pRenderDevice->CreateTexture( TextureDesc, TextureData(), ppTexture );
        if( *ppTexture == nullptr )
            SCRIPT_PARSING_ERROR(L, "Failed to create a texture")

        CHECK_LUA_STACK_HEIGHT(+1);
    }
}
