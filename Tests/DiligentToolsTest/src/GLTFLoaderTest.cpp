/*
 *  Copyright 2026 Diligent Graphics LLC
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

#include "GLTFLoader.hpp"
#include "../../../ThirdParty/tinygltf/tiny_gltf.h"
#include "GLTFBuilder.hpp"
#include "TinyGltfModelView.hpp"

#include "gtest/gtest.h"

#include "Image.h"

#include <initializer_list>
#include <utility>

namespace Diligent
{

namespace GLTF
{

namespace MSFTTextureDDS
{

int GetSource(const tinygltf::Texture& gltf_tex,
              const tinygltf::Model&   gltf_model);

} // namespace MSFTTextureDDS

} // namespace GLTF

} // namespace Diligent

using namespace Diligent;

namespace
{

tinygltf::Texture CreateDDSTexture(int Source)
{
    tinygltf::Value::Object Extension;
    Extension.emplace("source", tinygltf::Value{Source});

    tinygltf::Texture Texture;
    Texture.source = 0;
    Texture.extensions.emplace("MSFT_texture_dds", tinygltf::Value{std::move(Extension)});
    return Texture;
}

tinygltf::Value MakeNumberArray(std::initializer_list<double> Values)
{
    tinygltf::Value::Array Array;
    Array.reserve(Values.size());
    for (double Value : Values)
        Array.emplace_back(Value);
    return tinygltf::Value{std::move(Array)};
}

void SetNodeVisibility(tinygltf::Node& Node, bool Visible)
{
    tinygltf::Value::Object Extension;
    Extension.emplace("visible", tinygltf::Value{Visible});
    Node.extensions.emplace("KHR_node_visibility", tinygltf::Value{std::move(Extension)});
}

tinygltf::Value MakeTextureInfo(int TextureIndex, int TexCoord)
{
    tinygltf::Value::Object TextureInfo;
    TextureInfo.emplace("index", tinygltf::Value{TextureIndex});
    TextureInfo.emplace("texCoord", tinygltf::Value{TexCoord});
    return tinygltf::Value{std::move(TextureInfo)};
}

tinygltf::Value MakeTextureInfoWithTransform(int                           TextureIndex,
                                             int                           TexCoord,
                                             std::initializer_list<double> Scale,
                                             std::initializer_list<double> Offset,
                                             int                           TransformTexCoord)
{
    tinygltf::Value::Object Transform;
    Transform.emplace("scale", MakeNumberArray(Scale));
    Transform.emplace("offset", MakeNumberArray(Offset));
    Transform.emplace("texCoord", tinygltf::Value{TransformTexCoord});

    tinygltf::Value::Object Extensions;
    Extensions.emplace("KHR_texture_transform", tinygltf::Value{std::move(Transform)});

    tinygltf::Value::Object TextureInfo;
    TextureInfo.emplace("index", tinygltf::Value{TextureIndex});
    TextureInfo.emplace("texCoord", tinygltf::Value{TexCoord});
    TextureInfo.emplace("extensions", tinygltf::Value{std::move(Extensions)});
    return tinygltf::Value{std::move(TextureInfo)};
}

tinygltf::Parameter MakeCoreTextureParameter(int TextureIndex, int TexCoord)
{
    tinygltf::Parameter Parameter;
    Parameter.json_double_value.emplace("index", TextureIndex);
    Parameter.json_double_value.emplace("texCoord", TexCoord);
    return Parameter;
}

TEST(Tools_GLTFLoader, MSFTTextureDDSUsesRawDDSImageData)
{
    tinygltf::Image DDSImage;
    DDSImage.uri        = "texture.dds";
    DDSImage.pixel_type = IMAGE_FILE_FORMAT_DDS;
    DDSImage.image      = {'D', 'D', 'S', ' '};

    tinygltf::Model Model;
    Model.images.emplace_back(tinygltf::Image{});
    Model.images.emplace_back(std::move(DDSImage));

    const tinygltf::Texture Texture = CreateDDSTexture(1);

    EXPECT_EQ(GLTF::MSFTTextureDDS::GetSource(Texture, Model), 1);
}

TEST(Tools_GLTFLoader, MSFTTextureDDSRejectsNonDDSImageData)
{
    tinygltf::Image Image;
    Image.width      = 1;
    Image.height     = 1;
    Image.component  = 4;
    Image.bits       = 8;
    Image.pixel_type = TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE;
    Image.image      = {255, 255, 255, 255};

    tinygltf::Model Model;
    Model.images.emplace_back(tinygltf::Image{});
    Model.images.emplace_back(std::move(Image));

    const tinygltf::Texture Texture = CreateDDSTexture(1);

    EXPECT_EQ(GLTF::MSFTTextureDDS::GetSource(Texture, Model), -1);
}

TEST(Tools_GLTFLoader, AnimationSamplerOutputsUseTightlyPackedScalarStorage)
{
    tinygltf::Model Source;
    Source.buffers.emplace_back();
    tinygltf::Buffer& Buffer = Source.buffers.back();

    const auto AppendFloats = [&Buffer](std::initializer_list<float> Values) {
        const size_t Offset = Buffer.data.size();
        const auto*  pBegin = reinterpret_cast<const unsigned char*>(Values.begin());
        Buffer.data.insert(Buffer.data.end(), pBegin, pBegin + Values.size() * sizeof(float));
        return Offset;
    };

    const auto AddAccessor = [&Source](size_t Offset, size_t Size, size_t Count, int Type, size_t Stride = 0) {
        tinygltf::BufferView& View = Source.bufferViews.emplace_back();
        View.buffer                = 0;
        View.byteOffset            = Offset;
        View.byteLength            = Size;
        View.byteStride            = Stride;

        tinygltf::Accessor& Accessor = Source.accessors.emplace_back();
        Accessor.bufferView          = static_cast<int>(Source.bufferViews.size() - 1);
        Accessor.componentType       = TINYGLTF_COMPONENT_TYPE_FLOAT;
        Accessor.count               = Count;
        Accessor.type                = Type;
        return static_cast<int>(Source.accessors.size() - 1);
    };

    const size_t InputOffset   = AppendFloats({0.f, 1.f});
    const int    InputAccessor = AddAccessor(InputOffset, 2 * sizeof(float), 2, TINYGLTF_TYPE_SCALAR);

    const size_t Vec3OutputOffset = AppendFloats({1.f, 2.f, 3.f, -1.f,
                                                  4.f, 5.f, 6.f, -1.f});
    const int    Vec3OutputAccessor =
        AddAccessor(Vec3OutputOffset, 8 * sizeof(float), 2, TINYGLTF_TYPE_VEC3, 4 * sizeof(float));

    const size_t ScalarOutputOffset = AppendFloats({0.25f, 0.75f, 0.5f, 1.f});
    const int    ScalarOutputAccessor =
        AddAccessor(ScalarOutputOffset, 4 * sizeof(float), 4, TINYGLTF_TYPE_SCALAR);

    tinygltf::Animation& Animation = Source.animations.emplace_back();
    Animation.samplers.resize(2);
    Animation.samplers[0].input         = InputAccessor;
    Animation.samplers[0].output        = Vec3OutputAccessor;
    Animation.samplers[0].interpolation = "LINEAR";
    Animation.samplers[1].input         = InputAccessor;
    Animation.samplers[1].output        = ScalarOutputAccessor;
    Animation.samplers[1].interpolation = "LINEAR";
    Animation.channels.emplace_back();
    Animation.channels[0].sampler     = 0;
    Animation.channels[0].target_node = 0;
    Animation.channels[0].target_path = "translation";

    Source.nodes.emplace_back();
    Source.scenes.emplace_back().nodes = {0};
    Source.defaultScene                = 0;

    GLTF::ModelCreateInfo CreateInfo;
    GLTF::Model           Model{CreateInfo};
    GLTF::ModelBuilder    Builder{CreateInfo, Model};
    GLTF::MeshLoader      MeshLoader{CreateInfo, Model};
    Builder.BuildModel(GLTF::TinyGltfModelView{Source}, Source.defaultScene, MeshLoader);

    ASSERT_EQ(Model.Animations.size(), 1u);
    ASSERT_EQ(Model.Animations[0].Samplers.size(), 2u);

    const GLTF::AnimationSampler& Vec3Sampler = Model.Animations[0].Samplers[0];
    EXPECT_EQ(Vec3Sampler.OutputComponentCount, 3u);
    EXPECT_EQ(Vec3Sampler.Outputs, (std::vector<float>{1.f, 2.f, 3.f, 4.f, 5.f, 6.f}));
    EXPECT_EQ(Vec3Sampler.GetOutputElementCount(), 2u);
    EXPECT_EQ(Vec3Sampler.GetOutputElement(1)[0], 4.f);

    GLTF::ModelTransforms Transforms;
    Model.ComputeTransforms(0, Transforms, float4x4::Identity(), 0, 0.5f);
    ASSERT_EQ(Transforms.NodeAnimations.size(), 1u);
    EXPECT_EQ(Transforms.NodeAnimations[0].Translation, (float3{2.5f, 3.5f, 4.5f}));

    const GLTF::AnimationSampler& ScalarSampler = Model.Animations[0].Samplers[1];
    EXPECT_EQ(ScalarSampler.OutputComponentCount, 1u);
    EXPECT_EQ(ScalarSampler.Outputs, (std::vector<float>{0.25f, 0.75f, 0.5f, 1.f}));
    EXPECT_EQ(ScalarSampler.GetOutputElementCount(), 4u);
}

TEST(Tools_GLTFLoader, LoadsNodeVisibility)
{
    tinygltf::Model Source;
    Source.nodes.resize(4);
    Source.nodes[0].name = "Default";
    Source.nodes[1].name = "Hidden parent";
    Source.nodes[1].children.push_back(2);
    Source.nodes[2].name = "Visible child";
    Source.nodes[3].name = "Defaulted extension";

    SetNodeVisibility(Source.nodes[1], false);
    SetNodeVisibility(Source.nodes[2], true);
    Source.nodes[3].extensions.emplace(
        "KHR_node_visibility",
        tinygltf::Value{tinygltf::Value::Object{}});

    Source.scenes.emplace_back().nodes = {0, 1, 3};
    Source.defaultScene                = 0;

    GLTF::ModelCreateInfo CreateInfo;
    GLTF::Model           Model{CreateInfo};
    GLTF::ModelBuilder    Builder{CreateInfo, Model};
    GLTF::MeshLoader      MeshLoader{CreateInfo, Model};
    Builder.BuildModel(GLTF::TinyGltfModelView{Source}, Source.defaultScene, MeshLoader);

    ASSERT_EQ(Model.Nodes.size(), 4u);

    EXPECT_TRUE(Model.Nodes[0].Visible);

    EXPECT_FALSE(Model.Nodes[1].Visible);

    // The loader preserves node-local state. Effective hierarchy visibility is
    // evaluated by the consumer and must not be baked into the child.
    EXPECT_TRUE(Model.Nodes[2].Visible);

    EXPECT_TRUE(Model.Nodes[3].Visible);
}

TEST(Tools_GLTFLoader, InvalidNodeVisibilityExtensionUsesDefault)
{
    tinygltf::Node Node;
    Node.name = "Invalid extension";
    Node.extensions.emplace("KHR_node_visibility", tinygltf::Value{1});

    const GLTF::TinyGltfNodeView View{Node};
    EXPECT_TRUE(View.GetVisible());
}

TEST(Tools_GLTFLoader, NonBooleanNodeVisibilityUsesDefault)
{
    tinygltf::Node          Node;
    tinygltf::Value::Object Extension;
    Node.name = "Invalid property";
    Extension.emplace("visible", tinygltf::Value{1});
    Node.extensions.emplace("KHR_node_visibility", tinygltf::Value{std::move(Extension)});

    const GLTF::TinyGltfNodeView View{Node};
    EXPECT_TRUE(View.GetVisible());
}

TEST(Tools_GLTFLoader, SpecularGlossinessLoadsFactors)
{
    tinygltf::Value::Object Extension;
    Extension.emplace("diffuseFactor", MakeNumberArray({0.1, 0.2, 0.3, 0.4}));
    Extension.emplace("specularFactor", MakeNumberArray({0.5, 0.6, 0.7}));
    Extension.emplace("glossinessFactor", tinygltf::Value{0.8});

    tinygltf::Material Source;
    Source.extensions.emplace("KHR_materials_pbrSpecularGlossiness",
                              tinygltf::Value{std::move(Extension)});

    const GLTF::Material Material = GLTF::LoadMaterial(tinygltf::Model{}, Source);
    EXPECT_EQ(Material.Attribs.Workflow, GLTF::Material::PBR_WORKFLOW_SPEC_GLOSS);
    EXPECT_FLOAT_EQ(Material.Attribs.BaseColorFactor.x, 0.1f);
    EXPECT_FLOAT_EQ(Material.Attribs.BaseColorFactor.y, 0.2f);
    EXPECT_FLOAT_EQ(Material.Attribs.BaseColorFactor.z, 0.3f);
    EXPECT_FLOAT_EQ(Material.Attribs.BaseColorFactor.w, 0.4f);
    EXPECT_FLOAT_EQ(Material.Attribs.SpecularFactor.x, 0.5f);
    EXPECT_FLOAT_EQ(Material.Attribs.SpecularFactor.y, 0.6f);
    EXPECT_FLOAT_EQ(Material.Attribs.SpecularFactor.z, 0.7f);
    EXPECT_FLOAT_EQ(Material.Attribs.RoughnessFactor, 0.8f);
}

TEST(Tools_GLTFLoader, SpecularGlossinessUsesExtensionDefaultsInsteadOfCoreFallback)
{
    tinygltf::Material Source;

    tinygltf::Parameter BaseColorFactor;
    BaseColorFactor.number_array = {0.1, 0.2, 0.3, 0.4};
    Source.values.emplace("baseColorFactor", std::move(BaseColorFactor));

    tinygltf::Parameter RoughnessFactor;
    RoughnessFactor.number_value = 0.25;
    Source.values.emplace("roughnessFactor", std::move(RoughnessFactor));

    Source.values.emplace(GLTF::BaseColorTextureName, MakeCoreTextureParameter(0, 1));
    Source.values.emplace(GLTF::MetallicRoughnessTextureName, MakeCoreTextureParameter(1, 1));
    Source.extensions.emplace("KHR_materials_pbrSpecularGlossiness",
                              tinygltf::Value{tinygltf::Value::Object{}});

    tinygltf::Model Model;
    Model.textures.resize(2);

    const GLTF::Material Material = GLTF::LoadMaterial(Model, Source);
    EXPECT_EQ(Material.Attribs.Workflow, GLTF::Material::PBR_WORKFLOW_SPEC_GLOSS);
    EXPECT_EQ(Material.Attribs.BaseColorFactor, (float4{1, 1, 1, 1}));
    EXPECT_EQ(Material.Attribs.SpecularFactor, (float3{1, 1, 1}));
    EXPECT_FLOAT_EQ(Material.Attribs.RoughnessFactor, 1.f);
    EXPECT_EQ(Material.GetTextureId(GLTF::DefaultDiffuseTextureAttribId), -1);
    EXPECT_EQ(Material.GetTextureId(GLTF::DefaultSpecularGlossinessTextureAttibId), -1);
}

TEST(Tools_GLTFLoader, SpecularGlossinessTexturesOverrideAliasedCoreTextures)
{
    tinygltf::Value::Object Extension;
    Extension.emplace(GLTF::DiffuseTextureName, MakeTextureInfo(2, 1));
    Extension.emplace(GLTF::SpecularGlossinessTextureName, MakeTextureInfo(3, 2));

    tinygltf::Material Source;
    Source.values.emplace(GLTF::BaseColorTextureName, MakeCoreTextureParameter(0, 0));
    Source.values.emplace(GLTF::MetallicRoughnessTextureName, MakeCoreTextureParameter(1, 0));
    Source.extensions.emplace("KHR_materials_pbrSpecularGlossiness",
                              tinygltf::Value{std::move(Extension)});

    tinygltf::Model Model;
    Model.textures.resize(4);

    const GLTF::Material Material = GLTF::LoadMaterial(Model, Source);
    EXPECT_EQ(Material.GetTextureId(GLTF::DefaultDiffuseTextureAttribId), 2);
    EXPECT_EQ(Material.GetTextureAttrib(GLTF::DefaultDiffuseTextureAttribId).GetUVSelector(), 1);
    EXPECT_EQ(Material.GetTextureId(GLTF::DefaultSpecularGlossinessTextureAttibId), 3);
    EXPECT_EQ(Material.GetTextureAttrib(GLTF::DefaultSpecularGlossinessTextureAttibId).GetUVSelector(), 2);
}

TEST(Tools_GLTFLoader, SpecularLoadsFactorsAndTextures)
{
    tinygltf::Value::Object Extension;
    Extension.emplace("specularFactor", tinygltf::Value{0.4});
    Extension.emplace("specularColorFactor", MakeNumberArray({0.2, 0.3, 1.5}));
    Extension.emplace(GLTF::SpecularTextureName, MakeTextureInfo(0, 1));
    Extension.emplace(GLTF::SpecularColorTextureName, MakeTextureInfo(1, 2));

    tinygltf::Material Source;
    Source.extensions.emplace("KHR_materials_specular", tinygltf::Value{std::move(Extension)});

    tinygltf::Model Model;
    Model.textures.resize(2);

    const GLTF::Material Material = GLTF::LoadMaterial(Model, Source);
    ASSERT_NE(Material.Specular, nullptr);
    EXPECT_EQ(Material.Attribs.Workflow, GLTF::Material::PBR_WORKFLOW_METALL_ROUGH);
    EXPECT_FLOAT_EQ(Material.Specular->Factor, 0.4f);
    EXPECT_EQ(Material.Specular->ColorFactor, (float3{0.2f, 0.3f, 1.5f}));
    EXPECT_EQ(Material.GetTextureId(GLTF::DefaultSpecularTextureAttribId), 0);
    EXPECT_EQ(Material.GetTextureAttrib(GLTF::DefaultSpecularTextureAttribId).GetUVSelector(), 1);
    EXPECT_EQ(Material.GetTextureId(GLTF::DefaultSpecularColorTextureAttribId), 1);
    EXPECT_EQ(Material.GetTextureAttrib(GLTF::DefaultSpecularColorTextureAttribId).GetUVSelector(), 2);
}

TEST(Tools_GLTFLoader, SpecularUsesExtensionDefaults)
{
    tinygltf::Material Source;
    Source.extensions.emplace("KHR_materials_specular", tinygltf::Value{tinygltf::Value::Object{}});

    const GLTF::Material Material = GLTF::LoadMaterial(tinygltf::Model{}, Source);
    ASSERT_NE(Material.Specular, nullptr);
    EXPECT_FLOAT_EQ(Material.Specular->Factor, 1.f);
    EXPECT_EQ(Material.Specular->ColorFactor, (float3{1, 1, 1}));
    EXPECT_EQ(Material.GetTextureId(GLTF::DefaultSpecularTextureAttribId), -1);
    EXPECT_EQ(Material.GetTextureId(GLTF::DefaultSpecularColorTextureAttribId), -1);
}

TEST(Tools_GLTFLoader, SpecularLoadsTextureTransforms)
{
    tinygltf::Value::Object Extension;
    Extension.emplace(GLTF::SpecularTextureName,
                      MakeTextureInfoWithTransform(0, 1, {2.0, 3.0}, {0.25, 0.5}, 3));
    Extension.emplace(GLTF::SpecularColorTextureName,
                      MakeTextureInfoWithTransform(1, 2, {0.5, 0.75}, {0.1, 0.2}, 4));

    tinygltf::Material Source;
    Source.extensions.emplace("KHR_materials_specular", tinygltf::Value{std::move(Extension)});

    tinygltf::Model Model;
    Model.textures.resize(2);

    const GLTF::Material Material = GLTF::LoadMaterial(Model, Source);

    const auto& SpecularAttribs = Material.GetTextureAttrib(GLTF::DefaultSpecularTextureAttribId);
    EXPECT_EQ(SpecularAttribs.GetUVSelector(), 3);
    EXPECT_FLOAT_EQ(SpecularAttribs.UVScaleAndRotation._11, 2.f);
    EXPECT_FLOAT_EQ(SpecularAttribs.UVScaleAndRotation._22, 3.f);
    EXPECT_FLOAT_EQ(SpecularAttribs.UBias, 0.25f);
    EXPECT_FLOAT_EQ(SpecularAttribs.VBias, 0.5f);

    const auto& SpecularColorAttribs = Material.GetTextureAttrib(GLTF::DefaultSpecularColorTextureAttribId);
    EXPECT_EQ(SpecularColorAttribs.GetUVSelector(), 4);
    EXPECT_FLOAT_EQ(SpecularColorAttribs.UVScaleAndRotation._11, 0.5f);
    EXPECT_FLOAT_EQ(SpecularColorAttribs.UVScaleAndRotation._22, 0.75f);
    EXPECT_FLOAT_EQ(SpecularColorAttribs.UBias, 0.1f);
    EXPECT_FLOAT_EQ(SpecularColorAttribs.VBias, 0.2f);
}

TEST(Tools_GLTFLoader, SpecularIsIgnoredForIncompatibleWorkflows)
{
    tinygltf::Material UnlitSource;
    UnlitSource.extensions.emplace("KHR_materials_unlit", tinygltf::Value{tinygltf::Value::Object{}});
    UnlitSource.extensions.emplace("KHR_materials_specular", tinygltf::Value{tinygltf::Value::Object{}});

    const GLTF::Material UnlitMaterial = GLTF::LoadMaterial(tinygltf::Model{}, UnlitSource);
    EXPECT_EQ(UnlitMaterial.Attribs.Workflow, GLTF::Material::PBR_WORKFLOW_UNLIT);
    EXPECT_EQ(UnlitMaterial.Specular, nullptr);

    tinygltf::Material SpecGlossSource;
    SpecGlossSource.extensions.emplace("KHR_materials_pbrSpecularGlossiness", tinygltf::Value{tinygltf::Value::Object{}});
    SpecGlossSource.extensions.emplace("KHR_materials_specular", tinygltf::Value{tinygltf::Value::Object{}});

    const GLTF::Material SpecGlossMaterial = GLTF::LoadMaterial(tinygltf::Model{}, SpecGlossSource);
    EXPECT_EQ(SpecGlossMaterial.Attribs.Workflow, GLTF::Material::PBR_WORKFLOW_SPEC_GLOSS);
    EXPECT_EQ(SpecGlossMaterial.Specular, nullptr);
}

} // namespace
