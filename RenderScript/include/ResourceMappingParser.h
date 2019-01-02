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
#include "EngineObjectParserBase.h"
#include "ClassMethodBinding.h"

namespace Diligent
{
    class ResourceMappingParser final : public EngineObjectParserBase
    {
    public:
        ResourceMappingParser( IRenderDevice *pRenderDevice, lua_State *L, 
                                class TextureViewParser *pTexViewParser, 
                                class BufferParser *pBuffParser,
                                class BufferViewParser *pBuffViewParser );
        void GetObjectByName( lua_State *L, const Char *ShaderName, IResourceMapping** ppObject );

        static const Char* ResourceMappingLibName;

    protected:
        virtual void CreateObj( lua_State *L )override final;
        virtual void ReadField( lua_State *L, void *pData, const Char *Field )override final;
        virtual void UpdateField( lua_State *L, void *pData, const Char *Field )override final;

    private:
        virtual void PushExistingObject( lua_State *L, const void *pObject )override final;
        virtual void DestroyObj( void *pData )override final;

        BindingsMapType m_Bindings;

        TextureViewParser *m_pTexViewParser;
        BufferParser *m_pBuffParser;
        BufferViewParser *m_pBuffViewParser;
        std::vector<String> m_MappedResourceMetatables;

        BindShaderResourcesFlagEnumMapping m_BindShaderResFlagEnumMapping;
    };
}
