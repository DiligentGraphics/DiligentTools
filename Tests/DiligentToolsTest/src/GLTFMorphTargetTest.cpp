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
#include "TinyGltfModelView.hpp"
#include "GLTFBuilder.hpp"

#include "gtest/gtest.h"

#include <cstring>
#include <string>
#include <vector>

using namespace Diligent;

namespace
{

template <typename ValueType>
int AddAccessor(tinygltf::Model&              Model,
                const std::vector<ValueType>& Values,
                size_t                        Count,
                int                           Type,
                int                           ComponentType,
                bool                          Normalized = false)
{
    tinygltf::Buffer Buffer;
    Buffer.data.resize(Values.size() * sizeof(ValueType));
    std::memcpy(Buffer.data.data(), Values.data(), Buffer.data.size());
    const int BufferIndex = static_cast<int>(Model.buffers.size());
    Model.buffers.emplace_back(std::move(Buffer));

    tinygltf::BufferView View;
    View.buffer         = BufferIndex;
    View.byteLength     = Values.size() * sizeof(ValueType);
    const int ViewIndex = static_cast<int>(Model.bufferViews.size());
    Model.bufferViews.emplace_back(std::move(View));

    tinygltf::Accessor Accessor;
    Accessor.bufferView     = ViewIndex;
    Accessor.count          = Count;
    Accessor.type           = Type;
    Accessor.componentType  = ComponentType;
    Accessor.normalized     = Normalized;
    const int AccessorIndex = static_cast<int>(Model.accessors.size());
    Model.accessors.emplace_back(std::move(Accessor));
    return AccessorIndex;
}

int AddSparseFloat3Accessor(tinygltf::Model&          Model,
                            size_t                    Count,
                            const std::vector<Uint8>& Indices,
                            const std::vector<float>& Values)
{
    EXPECT_EQ(Values.size(), Indices.size() * 3);

    const int IndicesAccessor = AddAccessor(Model,
                                            Indices,
                                            Indices.size(),
                                            TINYGLTF_TYPE_SCALAR,
                                            TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE);

    const int ValuesAccessor = AddAccessor(Model,
                                           Values,
                                           Indices.size(),
                                           TINYGLTF_TYPE_VEC3,
                                           TINYGLTF_COMPONENT_TYPE_FLOAT);

    tinygltf::Accessor Accessor;
    Accessor.count                        = Count;
    Accessor.type                         = TINYGLTF_TYPE_VEC3;
    Accessor.componentType                = TINYGLTF_COMPONENT_TYPE_FLOAT;
    Accessor.sparse.isSparse              = true;
    Accessor.sparse.count                 = static_cast<int>(Indices.size());
    Accessor.sparse.indices.bufferView    = Model.accessors[IndicesAccessor].bufferView;
    Accessor.sparse.indices.byteOffset    = 0;
    Accessor.sparse.indices.componentType = TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE;
    Accessor.sparse.values.bufferView     = Model.accessors[ValuesAccessor].bufferView;
    Accessor.sparse.values.byteOffset     = 0;

    const int AccessorIndex = static_cast<int>(Model.accessors.size());
    Model.accessors.emplace_back(std::move(Accessor));
    return AccessorIndex;
}

tinygltf::Model CreateMorphModel(Uint32 TargetCount)
{
    tinygltf::Model Model;

    const int Positions = AddAccessor(Model,
                                      std::vector<float>{
                                          0.f, 0.f, 0.f,
                                          1.f, 0.f, 0.f,
                                          0.f, 1.f, 0.f},
                                      3,
                                      TINYGLTF_TYPE_VEC3,
                                      TINYGLTF_COMPONENT_TYPE_FLOAT);

    Model.accessors[Positions].minValues = {0.0, 0.0, 0.0};
    Model.accessors[Positions].maxValues = {1.0, 1.0, 0.0};

    tinygltf::Primitive Primitive;
    Primitive.attributes.emplace("POSITION", Positions);
    for (Uint32 TargetIndex = 0; TargetIndex < TargetCount; ++TargetIndex)
    {
        const float Delta = static_cast<float>(TargetIndex + 1) * 0.25f;

        const int TargetPositions = AddAccessor(Model,
                                                std::vector<float>{
                                                    Delta, 0.f, 0.f,
                                                    0.f, Delta, 0.f,
                                                    0.f, 0.f, Delta},
                                                3,
                                                TINYGLTF_TYPE_VEC3,
                                                TINYGLTF_COMPONENT_TYPE_FLOAT);

        Primitive.targets.push_back({{"POSITION", TargetPositions}});
    }

    tinygltf::Mesh Mesh;
    Mesh.primitives.emplace_back(std::move(Primitive));
    Model.meshes.emplace_back(std::move(Mesh));

    tinygltf::Node Node;
    Node.mesh = 0;
    Model.nodes.emplace_back(std::move(Node));

    tinygltf::Scene Scene;
    Scene.nodes.push_back(0);
    Model.scenes.emplace_back(std::move(Scene));
    Model.defaultScene = 0;
    return Model;
}

void ExpectValues(const float* pActual, const std::vector<float>& Expected)
{
    ASSERT_NE(pActual, nullptr);
    for (size_t Index = 0; Index < Expected.size(); ++Index)
        EXPECT_FLOAT_EQ(pActual[Index], Expected[Index]) << "Value " << Index;
}

TEST(Tools_GLTFLoader, LoadsMorphTargetsWeightsAndNames)
{
    tinygltf::Model Source = CreateMorphModel(2);

    const int Normals = AddAccessor(Source,
                                    std::vector<float>{
                                        0.f, 0.f, 0.25f,
                                        0.f, 0.f, 0.5f,
                                        0.f, 0.f, 0.75f},
                                    3,
                                    TINYGLTF_TYPE_VEC3,
                                    TINYGLTF_COMPONENT_TYPE_FLOAT);

    Source.meshes[0].primitives[0].targets[0].emplace("NORMAL", Normals);

    const int SparsePositions = AddSparseFloat3Accessor(Source,
                                                        3,
                                                        std::vector<Uint8>{1},
                                                        std::vector<float>{2.f, 3.f, 4.f});

    Source.meshes[0].primitives[0].targets[1]["POSITION"] = SparsePositions;

    Source.meshes[0].weights = {0.25, 0.75};
    tinygltf::Value::Array TargetNames;
    TargetNames.emplace_back(std::string{"Raise"});
    TargetNames.emplace_back(std::string{"Twist"});
    tinygltf::Value::Object Extras;
    Extras.emplace("targetNames", tinygltf::Value{std::move(TargetNames)});
    Source.meshes[0].extras = tinygltf::Value{std::move(Extras)};
    Source.nodes[0].weights = {0.5, 0.125};

    GLTF::ModelCreateInfo CI;
    GLTF::Model           Model{CI};
    GLTF::MeshLoader      MeshLoader{CI, Model};
    GLTF::ModelBuilder    Builder{CI, Model};
    Builder.BuildModel(GLTF::TinyGltfModelView{Source}, -1, MeshLoader);

    ASSERT_EQ(Model.Meshes.size(), 1u);
    const GLTF::Mesh& Mesh = Model.Meshes[0];
    ASSERT_EQ(Mesh.Weights.size(), 2u);
    EXPECT_FLOAT_EQ(Mesh.Weights[0], 0.25f);
    EXPECT_FLOAT_EQ(Mesh.Weights[1], 0.75f);
    EXPECT_EQ(Mesh.MorphTargetNames, (std::vector<std::string>{"Raise", "Twist"}));

    ASSERT_EQ(Mesh.Primitives.size(), 1u);
    const GLTF::Primitive& Primitive = Mesh.Primitives[0];
    ASSERT_EQ(Primitive.MorphTargets.size(), 2u);

    const GLTF::MorphTargetAttribute* pPosition = Primitive.MorphTargets[0].FindAttribute("POSITION");
    ASSERT_NE(pPosition, nullptr);
    EXPECT_EQ(pPosition->NumComponents, 3u);
    ExpectValues(Primitive.MorphTargets[0].GetAttributeData(*pPosition),
                 {0.25f, 0.f, 0.f, 0.f, 0.25f, 0.f, 0.f, 0.f, 0.25f});

    const GLTF::MorphTargetAttribute* pNormal = Primitive.MorphTargets[0].FindAttribute("NORMAL");
    ASSERT_NE(pNormal, nullptr);
    ExpectValues(Primitive.MorphTargets[0].GetAttributeData(*pNormal),
                 {0.f, 0.f, 0.25f, 0.f, 0.f, 0.5f, 0.f, 0.f, 0.75f});

    const GLTF::MorphTargetAttribute* pSparsePosition = Primitive.MorphTargets[1].FindAttribute("POSITION");
    ASSERT_NE(pSparsePosition, nullptr);
    ExpectValues(Primitive.MorphTargets[1].GetAttributeData(*pSparsePosition),
                 {0.f, 0.f, 0.f, 2.f, 3.f, 4.f, 0.f, 0.f, 0.f});

    ASSERT_EQ(Model.Nodes.size(), 1u);
    ASSERT_EQ(Model.Nodes[0].Weights.size(), 2u);
    EXPECT_FLOAT_EQ(Model.Nodes[0].Weights[0], 0.5f);
    EXPECT_FLOAT_EQ(Model.Nodes[0].Weights[1], 0.125f);
}

TEST(Tools_GLTFLoader, LoadsMorphWeightAnimationWithoutSkinningVertexAttributes)
{
    tinygltf::Model Source = CreateMorphModel(2);

    const int Times = AddAccessor(Source,
                                  std::vector<float>{0.f, 1.f},
                                  2,
                                  TINYGLTF_TYPE_SCALAR,
                                  TINYGLTF_COMPONENT_TYPE_FLOAT);

    const int Weights = AddAccessor(Source,
                                    std::vector<float>{0.f, 1.f, 0.75f, 0.25f},
                                    4,
                                    TINYGLTF_TYPE_SCALAR,
                                    TINYGLTF_COMPONENT_TYPE_FLOAT);

    tinygltf::AnimationSampler Sampler;
    Sampler.input         = Times;
    Sampler.output        = Weights;
    Sampler.interpolation = "LINEAR";

    tinygltf::AnimationChannel Channel;
    Channel.sampler     = 0;
    Channel.target_node = 0;
    Channel.target_path = "weights";

    tinygltf::Animation Animation;
    Animation.name = "Morph";
    Animation.samplers.emplace_back(std::move(Sampler));
    Animation.channels.emplace_back(std::move(Channel));
    Source.animations.emplace_back(std::move(Animation));

    constexpr GLTF::VertexAttributeDesc PositionAttribute{
        GLTF::PositionAttributeName,
        0,
        VT_FLOAT32,
        3,
    };
    GLTF::ModelCreateInfo CI;
    CI.VertexAttributes    = &PositionAttribute;
    CI.NumVertexAttributes = 1;

    GLTF::Model        Model{CI};
    GLTF::MeshLoader   MeshLoader{CI, Model};
    GLTF::ModelBuilder Builder{CI, Model};
    Builder.BuildModel(GLTF::TinyGltfModelView{Source}, -1, MeshLoader);

    ASSERT_EQ(Model.Animations.size(), 1u);
    const GLTF::Animation& LoadedAnimation = Model.Animations[0];
    EXPECT_EQ(LoadedAnimation.Name, "Morph");
    ASSERT_EQ(LoadedAnimation.Samplers.size(), 1u);
    EXPECT_EQ(LoadedAnimation.Samplers[0].Inputs, (std::vector<float>{0.f, 1.f}));
    EXPECT_EQ(LoadedAnimation.Samplers[0].OutputValueType, VT_FLOAT32);
    EXPECT_EQ(LoadedAnimation.Samplers[0].OutputComponentCount, 1u);
    EXPECT_FALSE(LoadedAnimation.Samplers[0].OutputIsNormalized);
    ASSERT_EQ(LoadedAnimation.Samplers[0].OutputData.size(), 4u * sizeof(float));
    std::vector<float> OutputValues(4);
    std::memcpy(OutputValues.data(), LoadedAnimation.Samplers[0].OutputData.data(),
                LoadedAnimation.Samplers[0].OutputData.size());
    EXPECT_EQ(OutputValues, (std::vector<float>{0.f, 1.f, 0.75f, 0.25f}));

    ASSERT_EQ(LoadedAnimation.Channels.size(), 1u);
    EXPECT_EQ(LoadedAnimation.Channels[0].PathType, GLTF::AnimationChannel::PATH_TYPE::WEIGHTS);
    EXPECT_EQ(LoadedAnimation.Channels[0].SamplerIndex, 0u);
    EXPECT_EQ(LoadedAnimation.Channels[0].ObjectType, GLTF::AnimationChannel::OBJECT_TYPE::NODE);
    EXPECT_EQ(LoadedAnimation.Channels[0].pObject, &Model.Nodes[0]);
}

} // namespace
