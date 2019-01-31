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

#pragma once

#include "LuaWrappers.h"
#include "LuaBindings.h"
#include "EngineObjectParserCommon.h"
#include "ClassMethodBinding.h"

namespace Diligent
{
    class TextureViewParser final : public EngineObjectParserCommon<ITextureView>
    {
    public:
        TextureViewParser( class TextureParser *pTexParser, class SamplerParser *pSamplerParser, IRenderDevice *pRenderDevice, lua_State *L );
        static const Char *TextureViewLibName;

    protected:
        virtual void CreateObj( lua_State *L )override final;

    private:
        // TextureViewDesc structure does not provide storage for the Name field.
        // We need to use ObjectDescWrapper<> to be able to store the field.
        typedef ObjectDescWrapper<TextureViewDesc> STexViewDescWrapper;

        const String m_TextureLibMetatableName;
        const String m_SamplerLibMetatableName;

        ClassMethodCaller<TextureViewParser> m_CreateViewBinding;
        int CreateView( lua_State *L );

        ClassMethodCaller<TextureViewParser> m_GetDefaultViewBinding;
        int GetDefaultView( lua_State * );

        ClassMethodCaller<TextureViewParser> m_SetSamplerBinding;
        int SetSampler( lua_State * );

        EnumMapping<TEXTURE_VIEW_TYPE>    m_ViewTypeEnumMapping;
        ResourceDimEnumMapping            m_TexTypeEnumMapping;
        TextureFormatEnumMapping          m_TexFormatEnumMapping;
        EnumMemberBinder<TEXTURE_VIEW_TYPE> m_ViewTypeParser;
        EnumMapping<UAV_ACCESS_FLAG>      m_UAVAccessFlagEnumMapping;
    };
}
