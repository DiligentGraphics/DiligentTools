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
#include "TextureViewParser.h"
#include "TextureParser.h"
#include "SamplerParser.h"
#include "GraphicsAccessories.h"

namespace Diligent
{
    const Char* TextureViewParser::TextureViewLibName = "TextureView";

    TextureViewParser::TextureViewParser( TextureParser *pTexParser, 
                                            SamplerParser *pSamplerParser, 
                                            IRenderDevice *pRenderDevice, lua_State *L ) :
        EngineObjectParserCommon<ITextureView>( pRenderDevice, L, TextureViewLibName ),
        m_TextureLibMetatableName(pTexParser->GetMetatableName()),
        m_SamplerLibMetatableName( pSamplerParser->GetMetatableName() ),
        m_CreateViewBinding( this, L, m_TextureLibMetatableName.c_str(), "CreateView", &TextureViewParser::CreateView ),
        m_GetDefaultViewBinding( this, L, m_TextureLibMetatableName.c_str(), "GetDefaultView", &TextureViewParser::GetDefaultView ),
        m_SetSamplerBinding( this, L, m_MetatableRegistryName.c_str(), "SetSampler", &TextureViewParser::SetSampler ),
        m_ViewTypeParser( 0, "ViewType", m_ViewTypeEnumMapping )
    {
        DEFINE_BUFFERED_STRING_BINDER( m_Bindings, STexViewDescWrapper, Name, NameBuffer )

        DEFINE_ENUM_ELEMENT_MAPPING( m_ViewTypeEnumMapping, TEXTURE_VIEW_SHADER_RESOURCE );
        DEFINE_ENUM_ELEMENT_MAPPING( m_ViewTypeEnumMapping, TEXTURE_VIEW_RENDER_TARGET );
        DEFINE_ENUM_ELEMENT_MAPPING( m_ViewTypeEnumMapping, TEXTURE_VIEW_DEPTH_STENCIL );
        DEFINE_ENUM_ELEMENT_MAPPING( m_ViewTypeEnumMapping, TEXTURE_VIEW_UNORDERED_ACCESS );
        VERIFY( m_ViewTypeEnumMapping.m_Str2ValMap.size() == TEXTURE_VIEW_NUM_VIEWS - 1,
                "Unexpected map size. Did you update TEXTURE_VIEW_TYPE enum?" );
        VERIFY( m_ViewTypeEnumMapping.m_Val2StrMap.size() == TEXTURE_VIEW_NUM_VIEWS - 1,
                "Unexpected map size. Did you update TEXTURE_VIEW_TYPE enum?" );
        DEFINE_ENUM_BINDER( m_Bindings, STexViewDescWrapper, ViewType, m_ViewTypeEnumMapping );
        
        DEFINE_ENUM_BINDER( m_Bindings, STexViewDescWrapper, TextureDim, m_TexTypeEnumMapping );
        DEFINE_ENUM_BINDER( m_Bindings, STexViewDescWrapper, Format, m_TexFormatEnumMapping );

        {
            using MostDetailedMipType = decltype(STexViewDescWrapper::MostDetailedMip);
            Validator<MostDetailedMipType> MostDetailedMipValidator("MostDetailedMip", 0, 16384);
            DEFINE_BINDER_EX( m_Bindings, STexViewDescWrapper, MostDetailedMip, MostDetailedMipType, MostDetailedMipValidator);
        }

        {
            using NumMipLevelsType = decltype(STexViewDescWrapper::NumMipLevels);
            Validator<NumMipLevelsType> NumMipLevelsValidator("NumMipLevels", 1, 16384);
            DEFINE_BINDER_EX( m_Bindings, STexViewDescWrapper, NumMipLevels, NumMipLevelsType, NumMipLevelsValidator);
        }

        // The following two members are in union
        {
            using FirstArraySliceType = decltype(STexViewDescWrapper::FirstArraySlice);
            Validator<FirstArraySliceType> FirstArraySliceValidator("FirstArraySlice", 0, 16384);
            DEFINE_BINDER_EX( m_Bindings, STexViewDescWrapper, FirstArraySlice, FirstArraySliceType, FirstArraySliceValidator);
        }

        {
            using FirstDepthSliceType = decltype(STexViewDescWrapper::FirstDepthSlice);
            Validator<FirstDepthSliceType> FirstDepthSliceValidator("FirstDepthSlice", 0, 16384);
            DEFINE_BINDER_EX( m_Bindings, STexViewDescWrapper, FirstDepthSlice, FirstDepthSliceType, FirstDepthSliceValidator);
        }

        // The following two members are in union
        {
            using NumArraySlicesType = decltype(STexViewDescWrapper::NumArraySlices);
            Validator<NumArraySlicesType> NumArraySlicesValidator("NumArraySlices", 0, 16384);
            DEFINE_BINDER_EX( m_Bindings, STexViewDescWrapper, NumArraySlices, NumArraySlicesType, NumArraySlicesValidator);
        }

        {
            using NumDepthSlicesType = decltype(STexViewDescWrapper::NumDepthSlices);
            Validator<NumDepthSlicesType> NumDepthSlicesValidator("NumDepthSlices", 0, 16384);
            DEFINE_BINDER_EX( m_Bindings, STexViewDescWrapper, NumDepthSlices, NumDepthSlicesType, NumDepthSlicesValidator);
        }

        DEFINE_ENUM_ELEMENT_MAPPING( m_UAVAccessFlagEnumMapping, UAV_ACCESS_FLAG_READ );
        DEFINE_ENUM_ELEMENT_MAPPING( m_UAVAccessFlagEnumMapping, UAV_ACCESS_FLAG_WRITE );
        DEFINE_ENUM_ELEMENT_MAPPING( m_UAVAccessFlagEnumMapping, UAV_ACCESS_FLAG_READ_WRITE );
        DEFINE_FLAGS_BINDER( m_Bindings, STexViewDescWrapper, AccessFlags, UAV_ACCESS_FLAG, m_UAVAccessFlagEnumMapping );
    };

    void TextureViewParser::CreateObj( lua_State *L )
    {
        INIT_LUA_STACK_TRACKING(L);
        
        auto *pTexture = *GetUserData<ITexture**>( L, 1, m_TextureLibMetatableName.c_str() );

        STexViewDescWrapper TextureViewDesc;
        ParseLuaTable( L, 2, &TextureViewDesc, m_Bindings );
        CHECK_LUA_STACK_HEIGHT();

        auto ppTextureView = reinterpret_cast<ITextureView**>(lua_newuserdata( L, sizeof( ITextureView* ) ));
        *ppTextureView = nullptr;
        pTexture->CreateView( TextureViewDesc, ppTextureView );
        if( *ppTextureView == nullptr )
            SCRIPT_PARSING_ERROR(L, "Failed to create texture view")

        CHECK_LUA_STACK_HEIGHT( +1 );
    }

    int TextureViewParser::CreateView( lua_State *L )
    {
        // Note that LuaCreate is a static function.
        // However, this pointer to the TextureViewParser object
        // is stored in the upvalue. So LuaCreate will call 
        // TextureViewParser::CreateObj()
        return LuaCreate(L);
    }

    int TextureViewParser::GetDefaultView( lua_State *L )
    {
        INIT_LUA_STACK_TRACKING( L );

        // Texture should be the first argument
        auto *pTexture = *GetUserData<ITexture**>( L, 1, m_TextureLibMetatableName.c_str() );
        
        // View type should be the second argument
        TEXTURE_VIEW_TYPE ViewType;
        m_ViewTypeParser.SetValue( L, 2, &ViewType );

        auto pView = pTexture->GetDefaultView( ViewType );
        if( !pView )
            SCRIPT_PARSING_ERROR( L, "Failed to get default texture view of type ", GetTexViewTypeLiteralName( ViewType ) );

        // Push existing object
        PushObject(L, pView);

        CHECK_LUA_STACK_HEIGHT( +1 );

        // Returning one value to Lua
        return 1;
    }

    int TextureViewParser::SetSampler( lua_State *L )
    {
        auto *pTexView = *GetUserData<ITextureView**>( L, 1, m_MetatableRegistryName.c_str() );
        auto *pSampler = *GetUserData<ISampler**>( L, 2, m_SamplerLibMetatableName.c_str() );
        pTexView->SetSampler( pSampler );

        // Returning nothing
        return 0;
    }
}
