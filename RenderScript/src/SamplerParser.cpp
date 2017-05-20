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
#include "SamplerParser.h"

namespace Diligent
{
    const Char* SamplerParser::SamplerLibName = "Sampler";

    SamplerParser::SamplerParser( IRenderDevice *pRenderDevice, lua_State *L ) :
        EngineObjectParserCommon<ISampler>( pRenderDevice, L, SamplerLibName )
    {
        DEFINE_BUFFERED_STRING_BINDER( m_Bindings, SSamDescWrapper, Name, NameBuffer )

        InitSamplerParserBindings<SSamDescWrapper>(m_Bindings, m_FilterTypeEnumMapping, m_TexAddrModeEnumMapping, m_CmpFuncEnumMapping);
    };

    void SamplerParser::CreateObj( lua_State *L )
    {
        INIT_LUA_STACK_TRACKING( L );

        SSamDescWrapper SamplerDesc;
        ParseLuaTable( L, -1, &SamplerDesc, m_Bindings );

        CHECK_LUA_STACK_HEIGHT();

        auto ppSampler = reinterpret_cast<ISampler**>(lua_newuserdata( L, sizeof( ISampler* ) ));
        *ppSampler = nullptr;
        m_pRenderDevice->CreateSampler( SamplerDesc, ppSampler );
        if( *ppSampler == nullptr )
            SCRIPT_PARSING_ERROR(L, "Failed to create a sampler")

        CHECK_LUA_STACK_HEIGHT( +1 );
    }
}
