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

#pragma once

#include "LuaWrappers.h"
#include "LuaBindings.h"
#include "EngineObjectParserBase.h"
#include "ClassMethodBinding.h"
#include "EngineObjectParserCommon.h"

namespace std
{
    DEFINE_ENUM_HASH( Diligent::SHADER_SOURCE_LANGUAGE )
}

namespace Diligent
{
    class ShaderParser : public EngineObjectParserCommon<IShader>
    {
    public:
        ShaderParser( IRenderDevice *pRenderDevice, lua_State *L, const String& ResMappingMetatableName);
        static const Char* ShaderLibName;

        // ShaderCreationAttribs structure does not provide storage for the Name field,
        // nor does it provide storage for FilePath.
        // We need to use ShaderCreationAttribsWrapper to be able to store the data.
        struct ShaderCreationAttribsWrapper : ShaderCreationAttribs
        {
            String NameBuffer;
            String SourceBuffer;
            String FilePathBuffer;
            String EntryPointBuffer;
            String SearchDirectoriesBuffer;
            std::vector<ShaderVariableDesc> m_VarDescBuffer;
            std::vector<String> m_VarNamesBuffer;
            std::vector<StaticSamplerDesc> m_StaticSamplersBuffer;
            std::vector<String> m_StaticSamplerTexNamesBuffer;
        };

    protected:
        virtual void CreateObj( lua_State *L );
        virtual void ReadField( lua_State *L, void *pData, const Char *Field );

    private:

        ClassMethodCaller< ShaderParser > m_BindResourcesBinding;
        int BindResources( lua_State *L );

        const String m_ResMappingMetatableName;

        EnumMapping<SHADER_SOURCE_LANGUAGE>   m_ShaderSourceLangEnumMapping;

        BindShaderResourcesFlagEnumMapping m_BindShaderResFlagEnumMapping;
    };
}
