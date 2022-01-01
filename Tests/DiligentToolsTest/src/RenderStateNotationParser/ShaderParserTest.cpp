/*
 *  Copyright 2019-2022 Diligent Graphics LLC
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
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

#include "gtest/gtest.h"
#include "DRSNLoader.hpp"

using namespace Diligent;

namespace
{

TEST(Tools_RenderStateNotationParser, ParseShaderEnums)
{
    DynamicLinearAllocator Allocator{DefaultRawMemoryAllocator::GetAllocator()};

    ASSERT_TRUE(TestEnum<SHADER_SOURCE_LANGUAGE>(Allocator, SHADER_SOURCE_LANGUAGE_DEFAULT, SHADER_SOURCE_LANGUAGE_GLSL_VERBATIM));

    ASSERT_TRUE(TestEnum<SHADER_COMPILER>(Allocator, SHADER_COMPILER_DEFAULT, SHADER_COMPILER_LAST));

    ASSERT_TRUE(TestEnum<SHADER_RESOURCE_TYPE>(Allocator, SHADER_RESOURCE_TYPE_UNKNOWN, SHADER_RESOURCE_TYPE_LAST));
}

TEST(Tools_RenderStateNotationParser, ParseShaderDesc)
{
    CHECK_STRUCT_SIZE(ShaderDesc, 16);

    DynamicLinearAllocator Allocator{DefaultRawMemoryAllocator::GetAllocator()};

    nlohmann::json JsonReference = LoadDRSNFromFile("RenderStates/Shader/ShaderDesc.json");

    ShaderDesc DescReference{};
    DescReference.Name       = "TestName";
    DescReference.ShaderType = SHADER_TYPE_VERTEX;

    ShaderDesc Desc{};
    Deserialize(JsonReference, Desc, Allocator);
    ASSERT_EQ(Desc, DescReference);
}

TEST(Tools_RenderStateNotationParser, ParseShaderMacro)
{
    CHECK_STRUCT_SIZE(ShaderMacro, 16);

    DynamicLinearAllocator Allocator{DefaultRawMemoryAllocator::GetAllocator()};

    nlohmann::json JsonReference = LoadDRSNFromFile("RenderStates/Shader/ShaderMacro.json");

    ShaderMacro DescReference{};
    DescReference.Name       = "TestName";
    DescReference.Definition = "TestDefinition";

    ShaderMacro Desc{};
    Deserialize(JsonReference, Desc, Allocator);
    ASSERT_EQ(Desc, DescReference);
}

TEST(Tools_RenderStateNotationParser, ParseShaderResourceDesc)
{
    CHECK_STRUCT_SIZE(ShaderResourceDesc, 16);

    DynamicLinearAllocator Allocator{DefaultRawMemoryAllocator::GetAllocator()};

    nlohmann::json JsonReference = LoadDRSNFromFile("RenderStates/Shader/ShaderResourceDesc.json");

    ShaderResourceDesc DescReference{};
    DescReference.Name      = "TestName";
    DescReference.Type      = SHADER_RESOURCE_TYPE_BUFFER_UAV;
    DescReference.ArraySize = 2;

    ShaderResourceDesc Desc{};
    Deserialize(JsonReference, Desc, Allocator);
    ASSERT_EQ(Desc, DescReference);
}

TEST(Tools_RenderStateNotationParser, ParseShaderCreateInfo)
{
    DynamicLinearAllocator Allocator{DefaultRawMemoryAllocator::GetAllocator()};

    nlohmann::json JsonReference = LoadDRSNFromFile("RenderStates/Shader/ShaderCreateInfo.json");



    ShaderMacro Macros[] = {
        ShaderMacro{"TestName0", "TestDefenition0"},
        ShaderMacro{"TestName1", "TestDefenition1"},
        ShaderMacro{nullptr, nullptr},
    };

    ShaderCreateInfo DescReference{};
    DescReference.Desc.Name                  = "TestName";
    DescReference.Desc.ShaderType            = SHADER_TYPE_PIXEL;
    DescReference.FilePath                   = "TestPath";
    DescReference.EntryPoint                 = "TestEntryPoint";
    DescReference.SourceLanguage             = SHADER_SOURCE_LANGUAGE_HLSL;
    DescReference.UseCombinedTextureSamplers = true;
    DescReference.CombinedSamplerSuffix      = "test";
    DescReference.Macros                     = Macros;

    ShaderCreateInfo Desc{};
    Deserialize(JsonReference, Desc, Allocator);

    ASSERT_EQ(DescReference.Desc, Desc.Desc);
    ASSERT_EQ(DescReference.UseCombinedTextureSamplers, Desc.UseCombinedTextureSamplers);
    ASSERT_EQ(DescReference.SourceLanguage, Desc.SourceLanguage);
    ASSERT_EQ(DescReference.Macros[0], Desc.Macros[0]);
    ASSERT_EQ(DescReference.Macros[1], Desc.Macros[1]);
    ASSERT_EQ(DescReference.Macros[2], Desc.Macros[2]);

    ASSERT_TRUE(SafeStrEqual(DescReference.FilePath, Desc.FilePath));
    ASSERT_TRUE(SafeStrEqual(DescReference.EntryPoint, Desc.EntryPoint));
    ASSERT_TRUE(SafeStrEqual(DescReference.CombinedSamplerSuffix, Desc.CombinedSamplerSuffix));
}


} // namespace
