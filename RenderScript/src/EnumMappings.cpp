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
#include "EnumMappings.h"

using namespace std;

namespace Diligent
{
    CpuAccessFlagEnumMapping::CpuAccessFlagEnumMapping()
    {
        DEFINE_ENUM_ELEMENT_MAPPING( (*this), CPU_ACCESS_READ );
        DEFINE_ENUM_ELEMENT_MAPPING( (*this), CPU_ACCESS_WRITE );
    }

    UsageEnumMapping::UsageEnumMapping()
    {
        DEFINE_ENUM_ELEMENT_MAPPING( (*this), USAGE_STATIC );
        DEFINE_ENUM_ELEMENT_MAPPING( (*this), USAGE_DEFAULT );
        DEFINE_ENUM_ELEMENT_MAPPING( (*this), USAGE_DYNAMIC );
        DEFINE_ENUM_ELEMENT_MAPPING( (*this), USAGE_CPU_ACCESSIBLE );
    }

    TextureFormatEnumMapping::TextureFormatEnumMapping()
    {
        DEFINE_ENUM_ELEMENT_MAPPING( (*this), TEX_FORMAT_RGBA32_TYPELESS );
        DEFINE_ENUM_ELEMENT_MAPPING( (*this), TEX_FORMAT_RGBA32_FLOAT );
        DEFINE_ENUM_ELEMENT_MAPPING( (*this), TEX_FORMAT_RGBA32_UINT );
        DEFINE_ENUM_ELEMENT_MAPPING( (*this), TEX_FORMAT_RGBA32_SINT );
        DEFINE_ENUM_ELEMENT_MAPPING( (*this), TEX_FORMAT_RGB32_TYPELESS );
        DEFINE_ENUM_ELEMENT_MAPPING( (*this), TEX_FORMAT_RGB32_FLOAT );
        DEFINE_ENUM_ELEMENT_MAPPING( (*this), TEX_FORMAT_RGB32_UINT );
        DEFINE_ENUM_ELEMENT_MAPPING( (*this), TEX_FORMAT_RGB32_SINT );
        DEFINE_ENUM_ELEMENT_MAPPING( (*this), TEX_FORMAT_RGBA16_TYPELESS );
        DEFINE_ENUM_ELEMENT_MAPPING( (*this), TEX_FORMAT_RGBA16_FLOAT );
        DEFINE_ENUM_ELEMENT_MAPPING( (*this), TEX_FORMAT_RGBA16_UNORM );
        DEFINE_ENUM_ELEMENT_MAPPING( (*this), TEX_FORMAT_RGBA16_UINT );
        DEFINE_ENUM_ELEMENT_MAPPING( (*this), TEX_FORMAT_RGBA16_SNORM );
        DEFINE_ENUM_ELEMENT_MAPPING( (*this), TEX_FORMAT_RGBA16_SINT );
        DEFINE_ENUM_ELEMENT_MAPPING( (*this), TEX_FORMAT_RG32_TYPELESS );
        DEFINE_ENUM_ELEMENT_MAPPING( (*this), TEX_FORMAT_RG32_FLOAT );
        DEFINE_ENUM_ELEMENT_MAPPING( (*this), TEX_FORMAT_RG32_UINT );
        DEFINE_ENUM_ELEMENT_MAPPING( (*this), TEX_FORMAT_RG32_SINT );
        DEFINE_ENUM_ELEMENT_MAPPING( (*this), TEX_FORMAT_R32G8X24_TYPELESS );
        DEFINE_ENUM_ELEMENT_MAPPING( (*this), TEX_FORMAT_D32_FLOAT_S8X24_UINT );
        DEFINE_ENUM_ELEMENT_MAPPING( (*this), TEX_FORMAT_R32_FLOAT_X8X24_TYPELESS );
        DEFINE_ENUM_ELEMENT_MAPPING( (*this), TEX_FORMAT_X32_TYPELESS_G8X24_UINT );
        DEFINE_ENUM_ELEMENT_MAPPING( (*this), TEX_FORMAT_RGB10A2_TYPELESS );
        DEFINE_ENUM_ELEMENT_MAPPING( (*this), TEX_FORMAT_RGB10A2_UNORM );
        DEFINE_ENUM_ELEMENT_MAPPING( (*this), TEX_FORMAT_RGB10A2_UINT );
        DEFINE_ENUM_ELEMENT_MAPPING( (*this), TEX_FORMAT_R11G11B10_FLOAT );
        DEFINE_ENUM_ELEMENT_MAPPING( (*this), TEX_FORMAT_RGBA8_TYPELESS );
        DEFINE_ENUM_ELEMENT_MAPPING( (*this), TEX_FORMAT_RGBA8_UNORM );
        DEFINE_ENUM_ELEMENT_MAPPING( (*this), TEX_FORMAT_RGBA8_UNORM_SRGB );
        DEFINE_ENUM_ELEMENT_MAPPING( (*this), TEX_FORMAT_RGBA8_UINT );
        DEFINE_ENUM_ELEMENT_MAPPING( (*this), TEX_FORMAT_RGBA8_SNORM );
        DEFINE_ENUM_ELEMENT_MAPPING( (*this), TEX_FORMAT_RGBA8_SINT );
        DEFINE_ENUM_ELEMENT_MAPPING( (*this), TEX_FORMAT_RG16_TYPELESS );
        DEFINE_ENUM_ELEMENT_MAPPING( (*this), TEX_FORMAT_RG16_FLOAT );
        DEFINE_ENUM_ELEMENT_MAPPING( (*this), TEX_FORMAT_RG16_UNORM );
        DEFINE_ENUM_ELEMENT_MAPPING( (*this), TEX_FORMAT_RG16_UINT );
        DEFINE_ENUM_ELEMENT_MAPPING( (*this), TEX_FORMAT_RG16_SNORM );
        DEFINE_ENUM_ELEMENT_MAPPING( (*this), TEX_FORMAT_RG16_SINT );
        DEFINE_ENUM_ELEMENT_MAPPING( (*this), TEX_FORMAT_R32_TYPELESS );
        DEFINE_ENUM_ELEMENT_MAPPING( (*this), TEX_FORMAT_D32_FLOAT );
        DEFINE_ENUM_ELEMENT_MAPPING( (*this), TEX_FORMAT_R32_FLOAT );
        DEFINE_ENUM_ELEMENT_MAPPING( (*this), TEX_FORMAT_R32_UINT );
        DEFINE_ENUM_ELEMENT_MAPPING( (*this), TEX_FORMAT_R32_SINT );
        DEFINE_ENUM_ELEMENT_MAPPING( (*this), TEX_FORMAT_R24G8_TYPELESS );
        DEFINE_ENUM_ELEMENT_MAPPING( (*this), TEX_FORMAT_D24_UNORM_S8_UINT );
        DEFINE_ENUM_ELEMENT_MAPPING( (*this), TEX_FORMAT_R24_UNORM_X8_TYPELESS );
        DEFINE_ENUM_ELEMENT_MAPPING( (*this), TEX_FORMAT_X24_TYPELESS_G8_UINT );
        DEFINE_ENUM_ELEMENT_MAPPING( (*this), TEX_FORMAT_RG8_TYPELESS );
        DEFINE_ENUM_ELEMENT_MAPPING( (*this), TEX_FORMAT_RG8_UNORM );
        DEFINE_ENUM_ELEMENT_MAPPING( (*this), TEX_FORMAT_RG8_UINT );
        DEFINE_ENUM_ELEMENT_MAPPING( (*this), TEX_FORMAT_RG8_SNORM );
        DEFINE_ENUM_ELEMENT_MAPPING( (*this), TEX_FORMAT_RG8_SINT );
        DEFINE_ENUM_ELEMENT_MAPPING( (*this), TEX_FORMAT_R16_TYPELESS );
        DEFINE_ENUM_ELEMENT_MAPPING( (*this), TEX_FORMAT_R16_FLOAT );
        DEFINE_ENUM_ELEMENT_MAPPING( (*this), TEX_FORMAT_D16_UNORM );
        DEFINE_ENUM_ELEMENT_MAPPING( (*this), TEX_FORMAT_R16_UNORM );
        DEFINE_ENUM_ELEMENT_MAPPING( (*this), TEX_FORMAT_R16_UINT );
        DEFINE_ENUM_ELEMENT_MAPPING( (*this), TEX_FORMAT_R16_SNORM );
        DEFINE_ENUM_ELEMENT_MAPPING( (*this), TEX_FORMAT_R16_SINT );
        DEFINE_ENUM_ELEMENT_MAPPING( (*this), TEX_FORMAT_R8_TYPELESS );
        DEFINE_ENUM_ELEMENT_MAPPING( (*this), TEX_FORMAT_R8_UNORM );
        DEFINE_ENUM_ELEMENT_MAPPING( (*this), TEX_FORMAT_R8_UINT );
        DEFINE_ENUM_ELEMENT_MAPPING( (*this), TEX_FORMAT_R8_SNORM );
        DEFINE_ENUM_ELEMENT_MAPPING( (*this), TEX_FORMAT_R8_SINT );
        DEFINE_ENUM_ELEMENT_MAPPING( (*this), TEX_FORMAT_A8_UNORM );
        DEFINE_ENUM_ELEMENT_MAPPING( (*this), TEX_FORMAT_R1_UNORM );
        DEFINE_ENUM_ELEMENT_MAPPING( (*this), TEX_FORMAT_RGB9E5_SHAREDEXP );
        DEFINE_ENUM_ELEMENT_MAPPING( (*this), TEX_FORMAT_RG8_B8G8_UNORM );
        DEFINE_ENUM_ELEMENT_MAPPING( (*this), TEX_FORMAT_G8R8_G8B8_UNORM );
        DEFINE_ENUM_ELEMENT_MAPPING( (*this), TEX_FORMAT_BC1_TYPELESS );
        DEFINE_ENUM_ELEMENT_MAPPING( (*this), TEX_FORMAT_BC1_UNORM );
        DEFINE_ENUM_ELEMENT_MAPPING( (*this), TEX_FORMAT_BC1_UNORM_SRGB );
        DEFINE_ENUM_ELEMENT_MAPPING( (*this), TEX_FORMAT_BC2_TYPELESS );
        DEFINE_ENUM_ELEMENT_MAPPING( (*this), TEX_FORMAT_BC2_UNORM );
        DEFINE_ENUM_ELEMENT_MAPPING( (*this), TEX_FORMAT_BC2_UNORM_SRGB );
        DEFINE_ENUM_ELEMENT_MAPPING( (*this), TEX_FORMAT_BC3_TYPELESS );
        DEFINE_ENUM_ELEMENT_MAPPING( (*this), TEX_FORMAT_BC3_UNORM );
        DEFINE_ENUM_ELEMENT_MAPPING( (*this), TEX_FORMAT_BC3_UNORM_SRGB );
        DEFINE_ENUM_ELEMENT_MAPPING( (*this), TEX_FORMAT_BC4_TYPELESS );
        DEFINE_ENUM_ELEMENT_MAPPING( (*this), TEX_FORMAT_BC4_UNORM );
        DEFINE_ENUM_ELEMENT_MAPPING( (*this), TEX_FORMAT_BC4_SNORM );
        DEFINE_ENUM_ELEMENT_MAPPING( (*this), TEX_FORMAT_BC5_TYPELESS );
        DEFINE_ENUM_ELEMENT_MAPPING( (*this), TEX_FORMAT_BC5_UNORM );
        DEFINE_ENUM_ELEMENT_MAPPING( (*this), TEX_FORMAT_BC5_SNORM );
        DEFINE_ENUM_ELEMENT_MAPPING( (*this), TEX_FORMAT_B5G6R5_UNORM );    // Deprecated format, unavailable in D3D11+
        DEFINE_ENUM_ELEMENT_MAPPING( (*this), TEX_FORMAT_B5G5R5A1_UNORM );  // Deprecated format, unavailable in D3D11+
        DEFINE_ENUM_ELEMENT_MAPPING( (*this), TEX_FORMAT_BGRA8_UNORM );     // Deprecated format, unavailable in D3D11+
        DEFINE_ENUM_ELEMENT_MAPPING( (*this), TEX_FORMAT_BGRX8_UNORM );     // Deprecated format, unavailable in D3D11+
        DEFINE_ENUM_ELEMENT_MAPPING( (*this), TEX_FORMAT_R10G10B10_XR_BIAS_A2_UNORM );
        DEFINE_ENUM_ELEMENT_MAPPING( (*this), TEX_FORMAT_BGRA8_TYPELESS );
        DEFINE_ENUM_ELEMENT_MAPPING( (*this), TEX_FORMAT_BGRA8_UNORM_SRGB );
        DEFINE_ENUM_ELEMENT_MAPPING( (*this), TEX_FORMAT_BGRX8_TYPELESS );
        DEFINE_ENUM_ELEMENT_MAPPING( (*this), TEX_FORMAT_BGRX8_UNORM_SRGB );
        DEFINE_ENUM_ELEMENT_MAPPING( (*this), TEX_FORMAT_BC6H_TYPELESS );
        DEFINE_ENUM_ELEMENT_MAPPING( (*this), TEX_FORMAT_BC6H_UF16 );
        DEFINE_ENUM_ELEMENT_MAPPING( (*this), TEX_FORMAT_BC6H_SF16 );
        DEFINE_ENUM_ELEMENT_MAPPING( (*this), TEX_FORMAT_BC7_TYPELESS );
        DEFINE_ENUM_ELEMENT_MAPPING( (*this), TEX_FORMAT_BC7_UNORM );
        DEFINE_ENUM_ELEMENT_MAPPING( (*this), TEX_FORMAT_BC7_UNORM_SRGB );
        static_assert(TEX_FORMAT_NUM_FORMATS == TEX_FORMAT_BC7_UNORM_SRGB + 1, "Not all texture formats initialized.");
        VERIFY( m_Str2ValMap.size() == TEX_FORMAT_NUM_FORMATS - 1,
                "Unexpected map size. Did you update TEXTURE_FORMAT enum?" );
        VERIFY( m_Val2StrMap.size() == TEX_FORMAT_NUM_FORMATS - 1,
                "Unexpected map size. Did you update TEXTURE_FORMAT enum?" );
    }

    TextureTypeEnumMapping::TextureTypeEnumMapping()
    {
        DEFINE_ENUM_ELEMENT_MAPPING( (*this), TEXTURE_TYPE_1D );
        DEFINE_ENUM_ELEMENT_MAPPING( (*this), TEXTURE_TYPE_1D_ARRAY );
        DEFINE_ENUM_ELEMENT_MAPPING( (*this), TEXTURE_TYPE_2D );
        DEFINE_ENUM_ELEMENT_MAPPING( (*this), TEXTURE_TYPE_2D_ARRAY );
        DEFINE_ENUM_ELEMENT_MAPPING( (*this), TEXTURE_TYPE_3D );
        DEFINE_ENUM_ELEMENT_MAPPING( (*this), TEXTURE_TYPE_CUBE );
        DEFINE_ENUM_ELEMENT_MAPPING( (*this), TEXTURE_TYPE_CUBE_ARRAY );
        static_assert(TEXTURE_TYPE_NUM_TYPES == TEXTURE_TYPE_CUBE_ARRAY + 1, "Not all texture types initialized.");
        VERIFY( m_Str2ValMap.size() == TEXTURE_TYPE_NUM_TYPES - 1,
                "Unexpected map size. Did you update TEXTURE_TYPE enum?" );
        VERIFY( m_Val2StrMap.size() == TEXTURE_TYPE_NUM_TYPES - 1,
                "Unexpected map size. Did you update TEXTURE_TYPE enum?" );
    }

    ValueTypeEnumMapping::ValueTypeEnumMapping()
    {
        DEFINE_ENUM_ELEMENT_MAPPING( (*this), VT_UNDEFINED );
        DEFINE_ENUM_ELEMENT_MAPPING( (*this), VT_INT8 );
        DEFINE_ENUM_ELEMENT_MAPPING( (*this), VT_INT16 );
        DEFINE_ENUM_ELEMENT_MAPPING( (*this), VT_INT32 );
        DEFINE_ENUM_ELEMENT_MAPPING( (*this), VT_UINT8 );
        DEFINE_ENUM_ELEMENT_MAPPING( (*this), VT_UINT16 );
        DEFINE_ENUM_ELEMENT_MAPPING( (*this), VT_UINT32 );
        DEFINE_ENUM_ELEMENT_MAPPING( (*this), VT_FLOAT16 );
        DEFINE_ENUM_ELEMENT_MAPPING( (*this), VT_FLOAT32 );
        static_assert(VT_NUM_TYPES == VT_FLOAT32 + 1, "Not all value types initialized.");
        VERIFY( m_Str2ValMap.size() == VT_NUM_TYPES,
                "Unexpected map size. Did you update VALUE_TYPE enum?" );
        VERIFY( m_Val2StrMap.size() == VT_NUM_TYPES,
                "Unexpected map size. Did you update VALUE_TYPE enum?" );
    }

    ComparisonFuncEnumMapping::ComparisonFuncEnumMapping()
    {
        DEFINE_ENUM_ELEMENT_MAPPING( (*this), COMPARISON_FUNC_NEVER );
        DEFINE_ENUM_ELEMENT_MAPPING( (*this), COMPARISON_FUNC_LESS );
        DEFINE_ENUM_ELEMENT_MAPPING( (*this), COMPARISON_FUNC_EQUAL );
        DEFINE_ENUM_ELEMENT_MAPPING( (*this), COMPARISON_FUNC_LESS_EQUAL );
        DEFINE_ENUM_ELEMENT_MAPPING( (*this), COMPARISON_FUNC_GREATER );
        DEFINE_ENUM_ELEMENT_MAPPING( (*this), COMPARISON_FUNC_NOT_EQUAL );
        DEFINE_ENUM_ELEMENT_MAPPING( (*this), COMPARISON_FUNC_GREATER_EQUAL );
        DEFINE_ENUM_ELEMENT_MAPPING( (*this), COMPARISON_FUNC_ALWAYS );
        static_assert(COMPARISON_FUNC_NUM_FUNCTIONS == COMPARISON_FUNC_ALWAYS + 1, "Not all comparison functions initialized.");
        VERIFY( m_Str2ValMap.size() == COMPARISON_FUNC_NUM_FUNCTIONS - 1,
                "Unexpected map size. Did you update COMPARISON_FUNCTION enum?" );
        VERIFY( m_Val2StrMap.size() == COMPARISON_FUNC_NUM_FUNCTIONS - 1,
                "Unexpected map size. Did you update COMPARISON_FUNCTION enum?" );
    }

    BindShaderResourcesFlagEnumMapping::BindShaderResourcesFlagEnumMapping()
    {
        DEFINE_ENUM_ELEMENT_MAPPING( (*this), BIND_SHADER_RESOURCES_RESET_BINDINGS );
        DEFINE_ENUM_ELEMENT_MAPPING( (*this), BIND_SHADER_RESOURCES_UPDATE_UNRESOLVED );
        DEFINE_ENUM_ELEMENT_MAPPING( (*this), BIND_SHADER_RESOURCES_ALL_RESOLVED );
    }
}
