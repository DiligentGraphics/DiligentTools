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
#include "RasterizerStateDescParser.h"

namespace Diligent
{

    MemberBinder<RasterizerStateDesc>::MemberBinder( size_t MemberOffset, size_t Dummy ) :
        MemberBinderBase(MemberOffset)
    {
        DEFINE_ENUM_ELEMENT_MAPPING( m_FillModeEnumMapping, FILL_MODE_WIREFRAME );
        DEFINE_ENUM_ELEMENT_MAPPING( m_FillModeEnumMapping, FILL_MODE_SOLID );
        VERIFY( m_FillModeEnumMapping.m_Str2ValMap.size() == FILL_MODE_NUM_MODES-1,
                "Unexpected map size. Did you update FILL_MODE enum?" );
        VERIFY( m_FillModeEnumMapping.m_Val2StrMap.size() == FILL_MODE_NUM_MODES-1,
                "Unexpected map size. Did you update FILL_MODE enum?" );
        DEFINE_ENUM_BINDER( m_Bindings, RasterizerStateDesc, FillMode, FILL_MODE, m_FillModeEnumMapping );

        DEFINE_ENUM_ELEMENT_MAPPING( m_CullModeEnumMapping, CULL_MODE_NONE );
        DEFINE_ENUM_ELEMENT_MAPPING( m_CullModeEnumMapping, CULL_MODE_FRONT );
        DEFINE_ENUM_ELEMENT_MAPPING( m_CullModeEnumMapping, CULL_MODE_BACK );
        VERIFY( m_CullModeEnumMapping.m_Str2ValMap.size() == CULL_MODE_NUM_MODES-1,
                "Unexpected map size. Did you update CULL_MODE enum?" );
        VERIFY( m_CullModeEnumMapping.m_Val2StrMap.size() == CULL_MODE_NUM_MODES-1,
                "Unexpected map size. Did you update CULL_MODE enum?" );
        DEFINE_ENUM_BINDER( m_Bindings, RasterizerStateDesc, CullMode, CULL_MODE, m_CullModeEnumMapping );

        DEFINE_BINDER( m_Bindings, RasterizerStateDesc, FrontCounterClockwise, Bool,    Validator<Bool>() );
        DEFINE_BINDER( m_Bindings, RasterizerStateDesc, DepthBias,             Int32,   Validator<Int32>() );
        DEFINE_BINDER( m_Bindings, RasterizerStateDesc, DepthBiasClamp,        Float32, Validator<Float32>() );
        DEFINE_BINDER( m_Bindings, RasterizerStateDesc, SlopeScaledDepthBias,  Float32, Validator<Float32>() );
        DEFINE_BINDER( m_Bindings, RasterizerStateDesc, DepthClipEnable,       Bool,    Validator<Bool>() );
        DEFINE_BINDER( m_Bindings, RasterizerStateDesc, ScissorEnable,         Bool,    Validator<Bool>() );
        DEFINE_BINDER( m_Bindings, RasterizerStateDesc, AntialiasedLineEnable, Bool,    Validator<Bool>() );
    };

    void MemberBinder<RasterizerStateDesc> ::GetValue(lua_State *L, const void* pBasePointer)
    {
        const auto &RasterizerDesc = GetMemberByOffest< RasterizerStateDesc >(pBasePointer, m_MemberOffset);
        PushLuaTable(L, &RasterizerDesc, m_Bindings);
    }

    void MemberBinder<RasterizerStateDesc> ::SetValue(lua_State *L, int Index, void* pBasePointer)
    {
        auto &RasterizerDesc = GetMemberByOffest< RasterizerStateDesc>( pBasePointer, m_MemberOffset );
        ParseLuaTable( L, Index, &RasterizerDesc, m_Bindings );
    }
}
