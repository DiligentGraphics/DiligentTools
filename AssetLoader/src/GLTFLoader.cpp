/*
 *  Copyright 2019-2021 Diligent Graphics LLC
 *  Copyright 2015-2019 Egor Yusov
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

#include <vector>
#include <memory>
#include <cmath>
#include <limits>

#include "GLTFLoader.hpp"
#include "MapHelper.hpp"
#include "CommonlyUsedStates.h"
#include "DataBlobImpl.hpp"
#include "Image.h"
#include "FileSystem.hpp"
#include "FileWrapper.hpp"
#include "GraphicsAccessories.hpp"
#include "TextureLoader.h"
#include "GraphicsUtilities.h"

#define TINYGLTF_IMPLEMENTATION
#define TINYGLTF_NO_STB_IMAGE
#define TINYGLTF_NO_STB_IMAGE_WRITE

#include "../../ThirdParty/tinygltf/tiny_gltf.h"

namespace Diligent
{


namespace GLTF
{

namespace
{

struct TextureInitData : public ObjectBase<IObject>
{
    TextureInitData(IReferenceCounters* pRefCounters) :
        ObjectBase<IObject>{pRefCounters}
    {}

    struct LevelData
    {
        std::vector<unsigned char> Data;

        Uint32 Stride = 0;
        Uint32 Width  = 0;
        Uint32 Height = 0;
    };
    std::vector<LevelData> Levels;

    RefCntAutoPtr<ITexture> pStagingTex;
};

} // namespace


static RefCntAutoPtr<TextureInitData> PrepareGLTFTextureInitData(
    const tinygltf::Image& gltfimage,
    float                  AlphaCutoff,
    Uint32                 NumMipLevels)
{
    VERIFY_EXPR(!gltfimage.image.empty());
    VERIFY_EXPR(gltfimage.width > 0 && gltfimage.height > 0 && gltfimage.component > 0);

    RefCntAutoPtr<TextureInitData> UpdateInfo{MakeNewRCObj<TextureInitData>()()};

    auto& Levels = UpdateInfo->Levels;
    Levels.resize(NumMipLevels);

    auto& Level0  = Levels[0];
    Level0.Width  = static_cast<Uint32>(gltfimage.width);
    Level0.Height = static_cast<Uint32>(gltfimage.height);
    Level0.Stride = Level0.Width * 4;

    if (gltfimage.component == 3)
    {
        Level0.Data.resize(Level0.Stride * gltfimage.height);

        // Due to depressing performance of iterators in debug MSVC we have to use raw pointers here
        const auto* rgb  = gltfimage.image.data();
        auto*       rgba = Level0.Data.data();
        for (int i = 0; i < gltfimage.width * gltfimage.height; ++i)
        {
            rgba[0] = rgb[0];
            rgba[1] = rgb[1];
            rgba[2] = rgb[2];
            rgba[3] = 255;

            rgba += 4;
            rgb += 3;
        }
        VERIFY_EXPR(rgb == gltfimage.image.data() + gltfimage.image.size());
        VERIFY_EXPR(rgba == Level0.Data.data() + Level0.Data.size());
    }
    else if (gltfimage.component == 4)
    {
        if (AlphaCutoff > 0)
        {
            Level0.Data.resize(Level0.Stride * gltfimage.height);

            // Remap alpha channel using the following formula to improve mip maps:
            //
            //      A_new = max(A_old; 1/3 * A_old + 2/3 * CutoffThreshold)
            //
            // https://asawicki.info/articles/alpha_test.php5

            VERIFY_EXPR(AlphaCutoff > 0 && AlphaCutoff <= 1);
            AlphaCutoff *= 255.f;

            // Due to depressing performance of iterators in debug MSVC we have to use raw pointers here
            const auto* src = gltfimage.image.data();
            auto*       dst = Level0.Data.data();
            for (int i = 0; i < gltfimage.width * gltfimage.height; ++i)
            {
                dst[0] = src[0];
                dst[1] = src[1];
                dst[2] = src[2];
                dst[3] = std::max(src[3], static_cast<Uint8>(std::min(1.f / 3.f * src[3] + 2.f / 3.f * AlphaCutoff, 255.f)));

                src += 4;
                dst += 4;
            }
            VERIFY_EXPR(src == gltfimage.image.data() + gltfimage.image.size());
            VERIFY_EXPR(dst == Level0.Data.data() + Level0.Data.size());
        }
        else
        {
            VERIFY_EXPR(gltfimage.image.size() == Level0.Stride * gltfimage.height);
            Level0.Data = std::move(gltfimage.image);
        }
    }
    else
    {
        UNEXPECTED("Unexpected number of color components in gltf image: ", gltfimage.component);
    }

    auto FineMipWidth  = static_cast<Uint32>(gltfimage.width);
    auto FineMipHeight = static_cast<Uint32>(gltfimage.height);
    for (Uint32 mip = 1; mip < NumMipLevels; ++mip)
    {
        auto&       Level     = Levels[mip];
        const auto& FineLevel = Levels[mip - 1];

        const auto MipWidth  = std::max(FineMipWidth / 2u, 1u);
        const auto MipHeight = std::max(FineMipHeight / 2u, 1u);

        Level.Stride = MipWidth * 4;
        Level.Width  = MipWidth;
        Level.Height = MipHeight;
        Level.Data.resize(Level.Stride * MipHeight);

        ComputeMipLevel(FineMipWidth, FineMipHeight, TEX_FORMAT_RGBA8_UNORM, FineLevel.Data.data(), FineLevel.Stride,
                        Level.Data.data(), Level.Stride);

        FineMipWidth  = MipWidth;
        FineMipHeight = MipHeight;
    }

    return UpdateInfo;
}


Mesh::Mesh(const float4x4& matrix)
{
    Transforms.matrix = matrix;
}



float4x4 Node::LocalMatrix() const
{
    // Translation, rotation, and scale properties and local space transformation are
    // mutually exclusive in GLTF.
    // We, however, may use non-trivial Matrix with TRS to apply transform to a model.
    return float4x4::Scale(Scale) * Rotation.ToMatrix() * float4x4::Translation(Translation) * Matrix;
}

float4x4 Node::GetMatrix() const
{
    auto mat = LocalMatrix();

    for (auto* p = Parent; p != nullptr; p = p->Parent)
    {
        mat = mat * p->LocalMatrix();
    }
    return mat;
}

void Node::UpdateTransforms()
{
    const auto NodeTransform = (pMesh || pCamera) ? GetMatrix() : float4x4::Identity();
    if (pMesh)
    {
        pMesh->Transforms.matrix = NodeTransform;
        if (pSkin != nullptr)
        {
            // Update join matrices
            auto InverseTransform = pMesh->Transforms.matrix.Inverse(); // TODO: do not use inverse transform here
            if (pMesh->Transforms.jointMatrices.size() != pSkin->Joints.size())
                pMesh->Transforms.jointMatrices.resize(pSkin->Joints.size());
            for (size_t i = 0; i < pSkin->Joints.size(); i++)
            {
                auto* JointNode = pSkin->Joints[i];
                pMesh->Transforms.jointMatrices[i] =
                    pSkin->InverseBindMatrices[i] * JointNode->GetMatrix() * InverseTransform;
            }
        }
    }

    if (pCamera)
    {
        pCamera->matrix = NodeTransform;
    }

    for (auto& child : Children)
    {
        child->UpdateTransforms();
    }
}




Model::Model(IRenderDevice*    pDevice,
             IDeviceContext*   pContext,
             const CreateInfo& CI)
{
    LoadFromFile(pDevice, pContext, CI);
}

Model::~Model()
{
}

void Model::LoadNode(Node*                            parent,
                     const tinygltf::Node&            gltf_node,
                     uint32_t                         nodeIndex,
                     const tinygltf::Model&           gltf_model,
                     std::vector<Uint32>&             IndexData,
                     std::vector<VertexBasicAttribs>& VertexBasicData,
                     std::vector<VertexSkinAttribs>*  pVertexSkinData)
{
    std::unique_ptr<Node> NewNode{new Node{}};
    NewNode->Index     = nodeIndex;
    NewNode->Parent    = parent;
    NewNode->Name      = gltf_node.name;
    NewNode->SkinIndex = gltf_node.skin;
    NewNode->Matrix    = float4x4::Identity();

    // Any node can define a local space transformation either by supplying a matrix property,
    // or any of translation, rotation, and scale properties (also known as TRS properties).

    // Generate local node matrix
    //float3 Translation;
    if (gltf_node.translation.size() == 3)
    {
        NewNode->Translation = float3::MakeVector(gltf_node.translation.data());
    }

    if (gltf_node.rotation.size() == 4)
    {
        NewNode->Rotation.q = float4::MakeVector(gltf_node.rotation.data());
        //NewNode->rotation = glm::mat4(q);
    }

    if (gltf_node.scale.size() == 3)
    {
        NewNode->Scale = float3::MakeVector(gltf_node.scale.data());
    }

    if (gltf_node.matrix.size() == 16)
    {
        NewNode->Matrix = float4x4::MakeMatrix(gltf_node.matrix.data());
    }

    // Node with children
    if (gltf_node.children.size() > 0)
    {
        for (size_t i = 0; i < gltf_node.children.size(); i++)
        {
            LoadNode(NewNode.get(), gltf_model.nodes[gltf_node.children[i]], gltf_node.children[i], gltf_model,
                     IndexData, VertexBasicData, pVertexSkinData);
        }
    }

    // Node contains mesh data
    if (gltf_node.mesh >= 0)
    {
        const tinygltf::Mesh& gltf_mesh = gltf_model.meshes[gltf_node.mesh];
        std::unique_ptr<Mesh> pNewMesh{new Mesh{NewNode->Matrix}};
        for (size_t j = 0; j < gltf_mesh.primitives.size(); j++)
        {
            const tinygltf::Primitive& primitive = gltf_mesh.primitives[j];

            uint32_t indexStart  = static_cast<uint32_t>(IndexData.size());
            uint32_t vertexStart = static_cast<uint32_t>(VertexBasicData.size());
            VERIFY_EXPR(pVertexSkinData == nullptr || pVertexSkinData->empty() || VertexBasicData.size() == pVertexSkinData->size());

            uint32_t indexCount  = 0;
            uint32_t vertexCount = 0;
            float3   PosMin;
            float3   PosMax;
            bool     hasSkin    = false;
            bool     hasIndices = primitive.indices > -1;

            // Vertices
            {
                const float*    bufferPos          = nullptr;
                const float*    bufferNormals      = nullptr;
                const float*    bufferTexCoordSet0 = nullptr;
                const float*    bufferTexCoordSet1 = nullptr;
                const uint8_t*  bufferJoints8      = nullptr;
                const uint16_t* bufferJoints16     = nullptr;
                const float*    bufferWeights      = nullptr;

                int posStride          = -1;
                int normalsStride      = -1;
                int texCoordSet0Stride = -1;
                int texCoordSet1Stride = -1;
                int jointsStride       = -1;
                int weightsStride      = -1;

                {
                    auto position_it = primitive.attributes.find("POSITION");
                    VERIFY(position_it != primitive.attributes.end(), "Position attribute is required");

                    const tinygltf::Accessor&   posAccessor = gltf_model.accessors[position_it->second];
                    const tinygltf::BufferView& posView     = gltf_model.bufferViews[posAccessor.bufferView];
                    VERIFY(posAccessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT, "Position component type is expected to be float");
                    VERIFY(posAccessor.type == TINYGLTF_TYPE_VEC3, "Position type is expected to be vec3");

                    bufferPos = reinterpret_cast<const float*>(&(gltf_model.buffers[posView.buffer].data[posAccessor.byteOffset + posView.byteOffset]));
                    PosMin =
                        float3 //
                        {
                            static_cast<float>(posAccessor.minValues[0]),
                            static_cast<float>(posAccessor.minValues[1]),
                            static_cast<float>(posAccessor.minValues[2]) //
                        };
                    PosMax =
                        float3 //
                        {
                            static_cast<float>(posAccessor.maxValues[0]),
                            static_cast<float>(posAccessor.maxValues[1]),
                            static_cast<float>(posAccessor.maxValues[2]) //
                        };
                    posStride = posAccessor.ByteStride(posView) / tinygltf::GetComponentSizeInBytes(posAccessor.componentType);
                    VERIFY(posStride > 0, "Position stride is invalid");

                    vertexCount = static_cast<uint32_t>(posAccessor.count);
                }

                if (primitive.attributes.find("NORMAL") != primitive.attributes.end())
                {
                    const tinygltf::Accessor&   normAccessor = gltf_model.accessors[primitive.attributes.find("NORMAL")->second];
                    const tinygltf::BufferView& normView     = gltf_model.bufferViews[normAccessor.bufferView];
                    VERIFY(normAccessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT, "Normal component type is expected to be float");
                    VERIFY(normAccessor.type == TINYGLTF_TYPE_VEC3, "Normal type is expected to be vec3");

                    bufferNormals = reinterpret_cast<const float*>(&(gltf_model.buffers[normView.buffer].data[normAccessor.byteOffset + normView.byteOffset]));
                    normalsStride = normAccessor.ByteStride(normView) / tinygltf::GetComponentSizeInBytes(normAccessor.componentType);
                    VERIFY(normalsStride > 0, "Normal stride is invalid");
                }

                if (primitive.attributes.find("TEXCOORD_0") != primitive.attributes.end())
                {
                    const tinygltf::Accessor&   uvAccessor = gltf_model.accessors[primitive.attributes.find("TEXCOORD_0")->second];
                    const tinygltf::BufferView& uvView     = gltf_model.bufferViews[uvAccessor.bufferView];
                    VERIFY(uvAccessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT, "UV0 component type is expected to be float");
                    VERIFY(uvAccessor.type == TINYGLTF_TYPE_VEC2, "UV0 type is expected to be vec2");

                    bufferTexCoordSet0 = reinterpret_cast<const float*>(&(gltf_model.buffers[uvView.buffer].data[uvAccessor.byteOffset + uvView.byteOffset]));
                    texCoordSet0Stride = uvAccessor.ByteStride(uvView) / tinygltf::GetComponentSizeInBytes(uvAccessor.componentType);
                    VERIFY(texCoordSet0Stride > 0, "Texcoord0 stride is invalid");
                }
                if (primitive.attributes.find("TEXCOORD_1") != primitive.attributes.end())
                {
                    const tinygltf::Accessor&   uvAccessor = gltf_model.accessors[primitive.attributes.find("TEXCOORD_1")->second];
                    const tinygltf::BufferView& uvView     = gltf_model.bufferViews[uvAccessor.bufferView];
                    VERIFY(uvAccessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT, "UV1 component type is expected to be float");
                    VERIFY(uvAccessor.type == TINYGLTF_TYPE_VEC2, "UV1 type is expected to be vec2");

                    bufferTexCoordSet1 = reinterpret_cast<const float*>(&(gltf_model.buffers[uvView.buffer].data[uvAccessor.byteOffset + uvView.byteOffset]));
                    texCoordSet1Stride = uvAccessor.ByteStride(uvView) / tinygltf::GetComponentSizeInBytes(uvAccessor.componentType);
                    VERIFY(texCoordSet1Stride > 0, "Texcoord1 stride is invalid");
                }

                // Skinning
                // Joints
                if (primitive.attributes.find("JOINTS_0") != primitive.attributes.end())
                {
                    const tinygltf::Accessor&   jointAccessor = gltf_model.accessors[primitive.attributes.find("JOINTS_0")->second];
                    const tinygltf::BufferView& jointView     = gltf_model.bufferViews[jointAccessor.bufferView];
                    VERIFY(jointAccessor.type == TINYGLTF_TYPE_VEC4, "Joint type is expected to be vec4");

                    const auto* bufferJoints = &(gltf_model.buffers[jointView.buffer].data[jointAccessor.byteOffset + jointView.byteOffset]);
                    switch (jointAccessor.componentType)
                    {
                        case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT:
                            bufferJoints16 = reinterpret_cast<const uint16_t*>(bufferJoints);
                            break;

                        case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE:
                            bufferJoints8 = reinterpret_cast<const uint8_t*>(bufferJoints);
                            break;

                        default:
                            UNEXPECTED("Joint component type is expected to be unsigned short or byte");
                    }

                    jointsStride = jointAccessor.ByteStride(jointView) / tinygltf::GetComponentSizeInBytes(jointAccessor.componentType);
                    VERIFY(jointsStride > 0, "Joints stride is invalid");
                }

                if (primitive.attributes.find("WEIGHTS_0") != primitive.attributes.end())
                {
                    const tinygltf::Accessor&   weightsAccessor = gltf_model.accessors[primitive.attributes.find("WEIGHTS_0")->second];
                    const tinygltf::BufferView& weightsView     = gltf_model.bufferViews[weightsAccessor.bufferView];
                    VERIFY(weightsAccessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT, "Weights component type is expected to be float");
                    VERIFY(weightsAccessor.type == TINYGLTF_TYPE_VEC4, "Weights type is expected to be vec4");

                    bufferWeights = reinterpret_cast<const float*>(&(gltf_model.buffers[weightsView.buffer].data[weightsAccessor.byteOffset + weightsView.byteOffset]));
                    weightsStride = weightsAccessor.ByteStride(weightsView) / tinygltf::GetComponentSizeInBytes(weightsAccessor.componentType);
                    VERIFY(weightsStride > 0, "Weights stride is invalid");
                }

                hasSkin = bufferWeights != nullptr && (bufferJoints8 != nullptr || bufferJoints16 != nullptr);

                for (uint32_t v = 0; v < vertexCount; v++)
                {
                    VertexBasicAttribs BasicAttribs{};
                    BasicAttribs.pos = float4(float3::MakeVector(bufferPos + v * posStride), 1.0f);
                    // clang-format off
                    BasicAttribs.normal = bufferNormals      != nullptr ? normalize(float3::MakeVector(bufferNormals + v * normalsStride)) : float3{};
                    BasicAttribs.uv0    = bufferTexCoordSet0 != nullptr ? float2::MakeVector(bufferTexCoordSet0 + v * texCoordSet0Stride)  : float2{};
                    BasicAttribs.uv1    = bufferTexCoordSet1 != nullptr ? float2::MakeVector(bufferTexCoordSet1 + v * texCoordSet1Stride)  : float2{};
                    // clang-format on
                    VertexBasicData.push_back(BasicAttribs);

                    if (pVertexSkinData != nullptr)
                    {
                        VertexSkinAttribs SkinAttribs{};
                        if (hasSkin)
                        {
                            SkinAttribs.joint0 = bufferJoints8 != nullptr ?
                                float4::MakeVector(bufferJoints8 + v * jointsStride) :
                                float4::MakeVector(bufferJoints16 + v * jointsStride);
                            SkinAttribs.weight0 = float4::MakeVector(bufferWeights + v * weightsStride);
                        }
                        pVertexSkinData->push_back(SkinAttribs);
                    }
                }
            }

            // Indices
            if (hasIndices)
            {
                const tinygltf::Accessor&   accessor   = gltf_model.accessors[primitive.indices > -1 ? primitive.indices : 0];
                const tinygltf::BufferView& bufferView = gltf_model.bufferViews[accessor.bufferView];
                const tinygltf::Buffer&     buffer     = gltf_model.buffers[bufferView.buffer];

                indexCount = static_cast<uint32_t>(accessor.count);

                const void* dataPtr = &(buffer.data[accessor.byteOffset + bufferView.byteOffset]);

                IndexData.reserve(IndexData.size() + accessor.count);
                switch (accessor.componentType)
                {
                    case TINYGLTF_PARAMETER_TYPE_UNSIGNED_INT:
                    {
                        const uint32_t* buf = static_cast<const uint32_t*>(dataPtr);
                        for (size_t index = 0; index < accessor.count; index++)
                        {
                            IndexData.push_back(buf[index] + vertexStart);
                        }
                        break;
                    }
                    case TINYGLTF_PARAMETER_TYPE_UNSIGNED_SHORT:
                    {
                        const uint16_t* buf = static_cast<const uint16_t*>(dataPtr);
                        for (size_t index = 0; index < accessor.count; index++)
                        {
                            IndexData.push_back(buf[index] + vertexStart);
                        }
                        break;
                    }
                    case TINYGLTF_PARAMETER_TYPE_UNSIGNED_BYTE:
                    {
                        const uint8_t* buf = static_cast<const uint8_t*>(dataPtr);
                        for (size_t index = 0; index < accessor.count; index++)
                        {
                            IndexData.push_back(buf[index] + vertexStart);
                        }
                        break;
                    }
                    default:
                        std::cerr << "Index component type " << accessor.componentType << " not supported!" << std::endl;
                        return;
                }
            }
            pNewMesh->Primitives.emplace_back( //
                indexStart,
                indexCount,
                vertexCount,
                primitive.material >= 0 ? static_cast<Uint32>(primitive.material) : static_cast<Uint32>(Materials.size() - 1),
                PosMin,
                PosMax
                //
            );
        }

        if (!pNewMesh->Primitives.empty())
        {
            // Mesh BB from BBs of primitives
            pNewMesh->BB = pNewMesh->Primitives[0].BB;
            for (size_t prim = 1; prim < pNewMesh->Primitives.size(); ++prim)
            {
                const auto& PrimBB = pNewMesh->Primitives[prim].BB;
                pNewMesh->BB.Min   = std::min(pNewMesh->BB.Min, PrimBB.Min);
                pNewMesh->BB.Max   = std::max(pNewMesh->BB.Max, PrimBB.Max);
            }
        }

        NewNode->pMesh = std::move(pNewMesh);
    }

    // Node contains camera
    if (gltf_node.camera >= 0)
    {
        const auto& gltf_cam = gltf_model.cameras[gltf_node.camera];

        std::unique_ptr<Camera> pNewCamera{new Camera{}};
        pNewCamera->Name = gltf_cam.name;

        if (gltf_cam.type == "perspective")
        {
            pNewCamera->Type                    = Camera::Projection::Perspective;
            pNewCamera->Perspective.AspectRatio = static_cast<float>(gltf_cam.perspective.aspectRatio);
            pNewCamera->Perspective.YFov        = static_cast<float>(gltf_cam.perspective.yfov);
            pNewCamera->Perspective.ZNear       = static_cast<float>(gltf_cam.perspective.znear);
            pNewCamera->Perspective.ZFar        = static_cast<float>(gltf_cam.perspective.zfar);
        }
        else if (gltf_cam.type == "orthographic")
        {
            pNewCamera->Type               = Camera::Projection::Orthographic;
            pNewCamera->Orthographic.XMag  = static_cast<float>(gltf_cam.orthographic.xmag);
            pNewCamera->Orthographic.YMag  = static_cast<float>(gltf_cam.orthographic.ymag);
            pNewCamera->Orthographic.ZNear = static_cast<float>(gltf_cam.orthographic.znear);
            pNewCamera->Orthographic.ZFar  = static_cast<float>(gltf_cam.orthographic.zfar);
        }
        else
        {
            UNEXPECTED("Unexpected camera type: ", gltf_cam.type);
            pNewCamera.reset();
        }

        if (pNewCamera)
            NewNode->pCamera = std::move(pNewCamera);
    }

    LinearNodes.push_back(NewNode.get());
    if (parent)
    {
        parent->Children.push_back(std::move(NewNode));
    }
    else
    {
        Nodes.push_back(std::move(NewNode));
    }
}


void Model::LoadSkins(const tinygltf::Model& gltf_model)
{
    for (const auto& source : gltf_model.skins)
    {
        std::unique_ptr<Skin> NewSkin{new Skin{}};
        NewSkin->Name = source.name;

        // Find skeleton root node
        if (source.skeleton > -1)
        {
            NewSkin->pSkeletonRoot = NodeFromIndex(source.skeleton);
        }

        // Find joint nodes
        for (int jointIndex : source.joints)
        {
            Node* node = NodeFromIndex(jointIndex);
            if (node)
            {
                NewSkin->Joints.push_back(NodeFromIndex(jointIndex));
            }
        }

        // Get inverse bind matrices from buffer
        if (source.inverseBindMatrices > -1)
        {
            const tinygltf::Accessor&   accessor   = gltf_model.accessors[source.inverseBindMatrices];
            const tinygltf::BufferView& bufferView = gltf_model.bufferViews[accessor.bufferView];
            const tinygltf::Buffer&     buffer     = gltf_model.buffers[bufferView.buffer];
            NewSkin->InverseBindMatrices.resize(accessor.count);
            memcpy(NewSkin->InverseBindMatrices.data(), &buffer.data[accessor.byteOffset + bufferView.byteOffset], accessor.count * sizeof(float4x4));
        }

        Skins.push_back(std::move(NewSkin));
    }
}

static float GetTextureAlphaCutoffValue(const tinygltf::Model& gltf_model, int TextureIndex)
{
    float AlphaCutoff = -1.f;

    for (const auto& gltf_mat : gltf_model.materials)
    {
        auto base_color_tex_it = gltf_mat.values.find("baseColorTexture");
        if (base_color_tex_it == gltf_mat.values.end())
        {
            // The material has no base texture
            continue;
        }

        if (base_color_tex_it->second.TextureIndex() != TextureIndex)
        {
            // The material does not use this texture
            continue;
        }

        auto alpha_mode_it = gltf_mat.additionalValues.find("alphaMode");
        if (alpha_mode_it == gltf_mat.additionalValues.end())
        {
            // The material uses this texture, but it is not an alpha-blended or an alpha-cut material
            AlphaCutoff = 0.f;
            continue;
        }

        const tinygltf::Parameter& param = alpha_mode_it->second;
        if (param.string_value == "MASK")
        {
            auto MaterialAlphaCutoff = 0.5f;
            auto alpha_cutoff_it     = gltf_mat.additionalValues.find("alphaCutoff");
            if (alpha_cutoff_it != gltf_mat.additionalValues.end())
            {
                MaterialAlphaCutoff = static_cast<float>(alpha_cutoff_it->second.Factor());
            }

            if (AlphaCutoff < 0)
            {
                AlphaCutoff = MaterialAlphaCutoff;
            }
            else if (AlphaCutoff != MaterialAlphaCutoff)
            {
                if (AlphaCutoff == 0)
                {
                    LOG_WARNING_MESSAGE("Texture ", TextureIndex,
                                        " is used in an alpha-cut material with threshold ", MaterialAlphaCutoff,
                                        " as well as in a non-alpha-cut material."
                                        " Alpha remapping to improve mipmap generation will be disabled.");
                }
                else
                {
                    LOG_WARNING_MESSAGE("Texture ", TextureIndex,
                                        " is used in alpha-cut materials with different cutoff thresholds (", AlphaCutoff, ", ", MaterialAlphaCutoff,
                                        "). Alpha remapping to improve mipmap generation will use ",
                                        AlphaCutoff, '.');
                }
            }
        }
        else
        {
            // The material is not an alpha-cut material
            if (AlphaCutoff > 0)
            {
                LOG_WARNING_MESSAGE("Texture ", TextureIndex,
                                    " is used in an alpha-cut material as well as in a non-alpha-cut material."
                                    " Alpha remapping to improve mipmap generation will be disabled.");
            }
            AlphaCutoff = 0.f;
        }
    }

    return std::max(AlphaCutoff, 0.f);
}

void Model::LoadTextures(IRenderDevice*         pDevice,
                         const tinygltf::Model& gltf_model,
                         const std::string&     BaseDir,
                         TextureCacheType*      pTextureCache,
                         ResourceManager*       pResourceMgr)
{
    for (const tinygltf::Texture& gltf_tex : gltf_model.textures)
    {
        const tinygltf::Image& gltf_image = gltf_model.images[gltf_tex.source];

        // TODO: simplify path
        const auto CacheId = !gltf_image.uri.empty() ? BaseDir + gltf_image.uri : "";

        TextureInfo TexInfo;
        if (!CacheId.empty())
        {
            if (pResourceMgr != nullptr)
            {
                TexInfo.pAtlasSuballocation = pResourceMgr->FindAllocation(CacheId.c_str());
                if (TexInfo.pAtlasSuballocation)
                {
                    // Note that the texture may appear in the cache after the call to LoadImageData because
                    // it can be loaded by another thread
                    VERIFY_EXPR(gltf_image.width == -1 || gltf_image.width == static_cast<int>(TexInfo.pAtlasSuballocation->GetSize().x));
                    VERIFY_EXPR(gltf_image.height == -1 || gltf_image.height == static_cast<int>(TexInfo.pAtlasSuballocation->GetSize().y));
                }
            }
            else if (pTextureCache != nullptr)
            {
                std::lock_guard<std::mutex> Lock{pTextureCache->TexturesMtx};

                auto it = pTextureCache->Textures.find(CacheId);
                if (it != pTextureCache->Textures.end())
                {
                    TexInfo.pTexture = it->second.Lock();
                    if (!TexInfo.pTexture)
                    {
                        // Image width and height (or pixel_type for dds/ktx) are initialized by LoadImageData()
                        // if the texture is found in the cache.
                        if ((gltf_image.width > 0 && gltf_image.height > 0) ||
                            (gltf_image.pixel_type == IMAGE_FILE_FORMAT_DDS || gltf_image.pixel_type == IMAGE_FILE_FORMAT_KTX))
                        {
                            UNEXPECTED("Stale textures should not be found in the texture cache because we hold strong references. "
                                       "This must be an unexpected effect of loading resources from multiple threads or a bug.");
                        }
                        else
                        {
                            pTextureCache->Textures.erase(it);
                        }
                    }
                }
            }
        }

        if (!TexInfo.IsValid())
        {
            RefCntAutoPtr<ISampler> pSampler;
            if (gltf_tex.sampler == -1)
            {
                // No sampler specified, use a default one
                pDevice->CreateSampler(Sam_LinearWrap, &pSampler);
            }
            else
            {
                pSampler = TextureSamplers[gltf_tex.sampler];
            }

            // Check if the texture is used in an alpha-cut material
            const float AlphaCutoff = GetTextureAlphaCutoffValue(gltf_model, static_cast<int>(Textures.size()));

            if (gltf_image.width > 0 && gltf_image.height > 0)
            {
                if (pResourceMgr != nullptr)
                {
                    // No reference
                    const TextureDesc AtlasDesc = pResourceMgr->GetAtlasDesc(TEX_FORMAT_RGBA8_UNORM);

                    // Load all mip levels.
                    auto pInitData = PrepareGLTFTextureInitData(gltf_image, AlphaCutoff, AtlasDesc.MipLevels);

                    // pInitData will be atomically set in the allocation before any other thread may be able to
                    // access it.
                    // Note that it is possible that more than one thread prepares pInitData for the same allocation.
                    // It it also possible that multiple instances of the same allocation are created before the first
                    // is added to the cache. This is all OK though.
                    TexInfo.pAtlasSuballocation =
                        pResourceMgr->AllocateTextureSpace(TEX_FORMAT_RGBA8_UNORM, gltf_image.width, gltf_image.height, CacheId.c_str(), pInitData);

                    VERIFY_EXPR(TexInfo.pAtlasSuballocation->GetAtlas()->GetAtlasDesc().MipLevels == AtlasDesc.MipLevels);
                }
                else
                {
                    TextureDesc TexDesc;
                    TexDesc.Name      = "GLTF Texture";
                    TexDesc.Type      = RESOURCE_DIM_TEX_2D_ARRAY;
                    TexDesc.Usage     = USAGE_DEFAULT;
                    TexDesc.BindFlags = BIND_SHADER_RESOURCE;
                    TexDesc.Width     = gltf_image.width;
                    TexDesc.Height    = gltf_image.height;
                    TexDesc.Format    = TEX_FORMAT_RGBA8_UNORM;
                    TexDesc.MipLevels = 0;
                    TexDesc.MiscFlags = MISC_TEXTURE_FLAG_GENERATE_MIPS;

                    pDevice->CreateTexture(TexDesc, nullptr, &TexInfo.pTexture);
                    TexInfo.pTexture->GetDefaultView(TEXTURE_VIEW_SHADER_RESOURCE)->SetSampler(pSampler);

                    // Load only the lowest mip level; other mip levels will be generated on the GPU.
                    auto pTexInitData = PrepareGLTFTextureInitData(gltf_image, AlphaCutoff, 1);
                    TexInfo.pTexture->SetUserData(pTexInitData);
                }
            }
            else if (gltf_image.pixel_type == IMAGE_FILE_FORMAT_DDS || gltf_image.pixel_type == IMAGE_FILE_FORMAT_KTX)
            {
                // Create the texture from raw bits

                RefCntAutoPtr<ITexture> pStagingTex;

                RefCntAutoPtr<TextureInitData> pTexInitData{MakeNewRCObj<TextureInitData>()()};

                TextureLoadInfo LoadInfo;
                if (pResourceMgr != nullptr)
                {
                    LoadInfo.Name           = "Staging compressed upload texture";
                    LoadInfo.Usage          = USAGE_STAGING;
                    LoadInfo.BindFlags      = BIND_NONE;
                    LoadInfo.CPUAccessFlags = CPU_ACCESS_WRITE;
                }
                else
                {
                    LoadInfo.Name = "Compressed texture for GLTF model";
                }
                switch (gltf_image.pixel_type)
                {
                    case IMAGE_FILE_FORMAT_DDS:
                        CreateTextureFromDDS(gltf_image.image.data(), gltf_image.image.size(), LoadInfo, pDevice, pResourceMgr != nullptr ? &pStagingTex : &TexInfo.pTexture);
                        break;

                    case IMAGE_FILE_FORMAT_KTX:
                        CreateTextureFromKTX(gltf_image.image.data(), gltf_image.image.size(), LoadInfo, pDevice, pResourceMgr != nullptr ? &pStagingTex : &TexInfo.pTexture);
                        break;

                    default:
                        UNEXPECTED("Unknown raw image format");
                }
                if (TexInfo.pTexture)
                {
                    // Set empty init data to inidicate that the texture needs to be transitioned to correct state
                    TexInfo.pTexture->SetUserData(pTexInitData);
                }
                else if (pResourceMgr != nullptr && pStagingTex)
                {
                    const auto& TexDesc = pStagingTex->GetDesc();

                    pTexInitData->pStagingTex = std::move(pStagingTex);

                    // pTexInitData will be atomically set in the allocation before any other thread may be able to
                    // access it.
                    // Note that it is possible that more than one thread prepares pTexInitData for the same allocation.
                    // It it also possible that multiple instances of the same allocation are created before the first
                    // is added to the cache. This is all OK though.
                    TexInfo.pAtlasSuballocation = pResourceMgr->AllocateTextureSpace(TexDesc.Format, TexDesc.Width, TexDesc.Height, CacheId.c_str(), pTexInitData);
                }
            }

            if (pResourceMgr == nullptr && !TexInfo.pTexture)
            {
                // Create stub texture
                TextureDesc TexDesc;
                TexDesc.Name      = "Checkerboard stub texture";
                TexDesc.Type      = RESOURCE_DIM_TEX_2D_ARRAY;
                TexDesc.Width     = 32;
                TexDesc.Height    = 32;
                TexDesc.Format    = TEX_FORMAT_RGBA8_UNORM;
                TexDesc.MipLevels = 1;
                TexDesc.Usage     = USAGE_DEFAULT;
                TexDesc.BindFlags = BIND_SHADER_RESOURCE;

                RefCntAutoPtr<TextureInitData> pTexInitData{MakeNewRCObj<TextureInitData>()()};

                pTexInitData->Levels.resize(1);
                auto& Level0  = pTexInitData->Levels[0];
                Level0.Width  = TexDesc.Width;
                Level0.Height = TexDesc.Height;
                Level0.Stride = Level0.Width * 4;
                Level0.Data.resize(Level0.Stride * TexDesc.Height);
                GenerateCheckerBoardPattern(TexDesc.Width, TexDesc.Height, TexDesc.Format, 4, 4, Level0.Data.data(), Level0.Stride);

                pDevice->CreateTexture(TexDesc, nullptr, &TexInfo.pTexture);
                TexInfo.pTexture->SetUserData(pTexInitData);
            }

            if (TexInfo.pTexture && pTextureCache != nullptr)
            {
                std::lock_guard<std::mutex> Lock{pTextureCache->TexturesMtx};
                pTextureCache->Textures.emplace(CacheId, TexInfo.pTexture);
            }
        }

        Textures.emplace_back(std::move(TexInfo));
    }
}

void Model::PrepareGPUResources(IRenderDevice* pDevice, IDeviceContext* pCtx)
{
    if (GPUDataInitialized.load())
        return;

    std::vector<StateTransitionDesc> Barriers;

    for (Uint32 i = 0; i < Textures.size(); ++i)
    {
        auto&     DstTexInfo = Textures[i];
        ITexture* pTexture   = nullptr;

        RefCntAutoPtr<TextureInitData> pInitData;
        if (DstTexInfo.pAtlasSuballocation)
        {
            pTexture  = DstTexInfo.pAtlasSuballocation->GetAtlas()->GetTexture(pDevice, pCtx);
            pInitData = ValidatedCast<TextureInitData>(DstTexInfo.pAtlasSuballocation->GetUserData());
            // User data is only set when the allocation is created, so no other
            // thread can call SetUserData() in parallel.
            DstTexInfo.pAtlasSuballocation->SetUserData(nullptr);
        }
        else if (DstTexInfo.pTexture)
        {
            pTexture  = DstTexInfo.pTexture;
            pInitData = ValidatedCast<TextureInitData>(pTexture->GetUserData());
            // User data is only set when the texture is created, so no other
            // thread can call SetUserData() in parallel.
            pTexture->SetUserData(nullptr);
        }

        if (!pTexture)
            continue;

        if (pInitData == nullptr)
        {
            // Shared texture has already been initialized by another model
            continue;
        }

        const auto& Levels   = pInitData->Levels;
        const auto  DstSlice = DstTexInfo.pAtlasSuballocation ? DstTexInfo.pAtlasSuballocation->GetSlice() : 0;
        if (!Levels.empty())
        {
            Uint32 DstX = 0;
            Uint32 DstY = 0;
            if (DstTexInfo.pAtlasSuballocation)
            {
                const auto& Origin = DstTexInfo.pAtlasSuballocation->GetOrigin();

                DstX = Origin.x;
                DstY = Origin.y;
            }
            VERIFY_EXPR(Levels.size() == 1 || Levels.size() == pTexture->GetDesc().MipLevels);
            for (Uint32 mip = 0; mip < Levels.size(); ++mip)
            {
                const auto& Level = Levels[mip];

                Box UpdateBox;
                UpdateBox.MinX = DstX >> mip;
                UpdateBox.MaxX = UpdateBox.MinX + Level.Width;
                UpdateBox.MinY = DstY >> mip;
                UpdateBox.MaxY = UpdateBox.MinY + Level.Height;
                TextureSubResData SubresData{Level.Data.data(), Level.Stride};
                pCtx->UpdateTexture(pTexture, mip, DstSlice, UpdateBox, SubresData, RESOURCE_STATE_TRANSITION_MODE_NONE, RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
            }

            if (Levels.size() == 1 && pTexture->GetDesc().MipLevels > 1)
            {
                pCtx->GenerateMips(pTexture->GetDefaultView(TEXTURE_VIEW_SHADER_RESOURCE));
            }
        }
        else if (pInitData->pStagingTex)
        {
            CopyTextureAttribs CopyAttribs;
            CopyAttribs.pSrcTexture              = pInitData->pStagingTex;
            CopyAttribs.pDstTexture              = pTexture;
            CopyAttribs.SrcTextureTransitionMode = RESOURCE_STATE_TRANSITION_MODE_TRANSITION;
            CopyAttribs.DstTextureTransitionMode = RESOURCE_STATE_TRANSITION_MODE_TRANSITION;
            CopyAttribs.SrcSlice                 = 0;
            CopyAttribs.DstSlice                 = DstSlice;

            if (DstTexInfo.pAtlasSuballocation)
            {
                const auto& Origin = DstTexInfo.pAtlasSuballocation->GetOrigin();
                CopyAttribs.DstX   = Origin.x;
                CopyAttribs.DstY   = Origin.y;
            }
            const auto& DstTexDesc   = pTexture->GetDesc();
            auto        NumMipLevels = std::min(DstTexDesc.MipLevels, pInitData->pStagingTex->GetDesc().MipLevels);
            for (Uint32 mip = 0; mip < NumMipLevels; ++mip)
            {
                CopyAttribs.SrcMipLevel = mip;
                CopyAttribs.DstMipLevel = mip;
                pCtx->CopyTexture(CopyAttribs);
                CopyAttribs.DstX /= 2;
                CopyAttribs.DstY /= 2;
            }
        }
        else
        {
            // Texture is already initialized
        }

        if (DstTexInfo.pTexture)
        {
            // Note that we may need to transition a texture even if it has been fully initialized,
            // as is the case with KTX/DDS textures.
            VERIFY_EXPR(pTexture == DstTexInfo.pTexture);
            Barriers.emplace_back(StateTransitionDesc{pTexture, RESOURCE_STATE_UNKNOWN, RESOURCE_STATE_SHADER_RESOURCE, true});
        }
    }

    auto UpdateBuffer = [&](BUFFER_ID BuffId) //
    {
        auto&    BuffInfo = Buffers[BuffId];
        IBuffer* pBuffer  = nullptr;
        Uint32   Offset   = 0;

        RefCntAutoPtr<IDataBlob> pInitData;
        if (BuffInfo.pSuballocation)
        {
            pBuffer   = BuffInfo.pSuballocation->GetAllocator()->GetBuffer(pDevice, pCtx);
            Offset    = BuffInfo.pSuballocation->GetOffset();
            pInitData = RefCntAutoPtr<IDataBlob>{BuffInfo.pSuballocation->GetUserData(), IID_DataBlob};
            BuffInfo.pSuballocation->SetUserData(nullptr);
        }
        else if (BuffInfo.pBuffer)
        {
            pBuffer   = BuffInfo.pBuffer;
            pInitData = RefCntAutoPtr<IDataBlob>{pBuffer->GetUserData(), IID_DataBlob};
            pBuffer->SetUserData(nullptr);
        }
        else
        {
            return;
        }

        if (pInitData)
        {
            pCtx->UpdateBuffer(pBuffer, Offset, static_cast<Uint32>(pInitData->GetSize()), pInitData->GetConstDataPtr(), RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
            if (Buffers[BuffId].pBuffer != nullptr)
            {
                VERIFY_EXPR(Buffers[BuffId].pBuffer == pBuffer);
                Barriers.emplace_back(StateTransitionDesc{pBuffer, RESOURCE_STATE_UNKNOWN, BuffId == BUFFER_ID_INDEX ? RESOURCE_STATE_INDEX_BUFFER : RESOURCE_STATE_VERTEX_BUFFER, true});
            }
        }
    };

    UpdateBuffer(BUFFER_ID_VERTEX_BASIC_ATTRIBS);
    UpdateBuffer(BUFFER_ID_VERTEX_SKIN_ATTRIBS);
    UpdateBuffer(BUFFER_ID_INDEX);

    if (!Barriers.empty())
        pCtx->TransitionResourceStates(static_cast<Uint32>(Barriers.size()), Barriers.data());

    GPUDataInitialized.store(true);
}

namespace
{

TEXTURE_ADDRESS_MODE GetWrapMode(int32_t wrapMode)
{
    switch (wrapMode)
    {
        case 10497:
            return TEXTURE_ADDRESS_WRAP;
        case 33071:
            return TEXTURE_ADDRESS_CLAMP;
        case 33648:
            return TEXTURE_ADDRESS_MIRROR;
        default:
            LOG_WARNING_MESSAGE("Unknown gltf address wrap mode: ", wrapMode, ". Defaulting to WRAP.");
            return TEXTURE_ADDRESS_WRAP;
    }
}

std::pair<FILTER_TYPE, FILTER_TYPE> GetFilterMode(int32_t filterMode)
{
    switch (filterMode)
    {
        case 9728: // NEAREST
            return {FILTER_TYPE_POINT, FILTER_TYPE_POINT};
        case 9729: // LINEAR
            return {FILTER_TYPE_LINEAR, FILTER_TYPE_LINEAR};
        case 9984: // NEAREST_MIPMAP_NEAREST
            return {FILTER_TYPE_POINT, FILTER_TYPE_POINT};
        case 9985: // LINEAR_MIPMAP_NEAREST
            return {FILTER_TYPE_LINEAR, FILTER_TYPE_POINT};
        case 9986:                                           // NEAREST_MIPMAP_LINEAR
            return {FILTER_TYPE_LINEAR, FILTER_TYPE_LINEAR}; // use linear min filter instead as point makes no sesne
        case 9987:                                           // LINEAR_MIPMAP_LINEAR
            return {FILTER_TYPE_LINEAR, FILTER_TYPE_LINEAR};
        default:
            LOG_WARNING_MESSAGE("Unknown gltf filter mode: ", filterMode, ". Defaulting to linear.");
            return {FILTER_TYPE_LINEAR, FILTER_TYPE_LINEAR};
    }
}

} // namespace

void Model::LoadTextureSamplers(IRenderDevice* pDevice, const tinygltf::Model& gltf_model)
{
    for (const tinygltf::Sampler& smpl : gltf_model.samplers)
    {
        SamplerDesc SamDesc;
        SamDesc.MagFilter = GetFilterMode(smpl.magFilter).first;
        auto MinMipFilter = GetFilterMode(smpl.minFilter);
        SamDesc.MinFilter = MinMipFilter.first;
        SamDesc.MipFilter = MinMipFilter.second;
        SamDesc.AddressU  = GetWrapMode(smpl.wrapS);
        SamDesc.AddressV  = GetWrapMode(smpl.wrapT);
        SamDesc.AddressW  = SamDesc.AddressV;
        RefCntAutoPtr<ISampler> pSampler;
        pDevice->CreateSampler(SamDesc, &pSampler);
        TextureSamplers.push_back(std::move(pSampler));
    }
}


void Model::LoadMaterials(const tinygltf::Model& gltf_model)
{
    for (const tinygltf::Material& gltf_mat : gltf_model.materials)
    {
        Material Mat;

        struct TextureParameterInfo
        {
            const Material::TEXTURE_ID    TextureId;
            float&                        UVSelector;
            float4&                       UVScaleBias;
            float&                        Slice;
            const char* const             TextureName;
            const tinygltf::ParameterMap& Params;
        };
        // clang-format off
        std::array<TextureParameterInfo, 5> TextureParams =
        {
            TextureParameterInfo{Material::TEXTURE_ID_BASE_COLOR,    Mat.Attribs.BaseColorUVSelector,          Mat.Attribs.BaseColorUVScaleBias,          Mat.Attribs.BaseColorSlice,          "baseColorTexture",         gltf_mat.values},
            TextureParameterInfo{Material::TEXTURE_ID_PHYSICAL_DESC, Mat.Attribs.PhysicalDescriptorUVSelector, Mat.Attribs.PhysicalDescriptorUVScaleBias, Mat.Attribs.PhysicalDescriptorSlice, "metallicRoughnessTexture", gltf_mat.values},
            TextureParameterInfo{Material::TEXTURE_ID_NORMAL_MAP,    Mat.Attribs.NormalUVSelector,             Mat.Attribs.NormalUVScaleBias,             Mat.Attribs.NormalSlice,             "normalTexture",            gltf_mat.additionalValues},
            TextureParameterInfo{Material::TEXTURE_ID_OCCLUSION,     Mat.Attribs.OcclusionUVSelector,          Mat.Attribs.OcclusionUVScaleBias,          Mat.Attribs.OcclusionSlice,          "occlusionTexture",         gltf_mat.additionalValues},
            TextureParameterInfo{Material::TEXTURE_ID_EMISSIVE,      Mat.Attribs.EmissiveUVSelector,           Mat.Attribs.EmissiveUVScaleBias,           Mat.Attribs.EmissiveSlice,           "emissiveTexture",          gltf_mat.additionalValues}
        };
        // clang-format on

        for (const auto& Param : TextureParams)
        {
            auto tex_it = Param.Params.find(Param.TextureName);
            if (tex_it != Param.Params.end())
            {
                Mat.TextureIds[Param.TextureId] = tex_it->second.TextureIndex();
                Param.UVSelector                = static_cast<float>(tex_it->second.TextureTexCoord());
            }
        }

        auto ReadFactor = [](float& Factor, const tinygltf::ParameterMap& Params, const char* Name) //
        {
            auto it = Params.find(Name);
            if (it != Params.end())
            {
                Factor = static_cast<float>(it->second.Factor());
            }
        };
        ReadFactor(Mat.Attribs.RoughnessFactor, gltf_mat.values, "roughnessFactor");
        ReadFactor(Mat.Attribs.MetallicFactor, gltf_mat.values, "metallicFactor");

        auto ReadColorFactor = [](float4& Factor, const tinygltf::ParameterMap& Params, const char* Name) //
        {
            auto it = Params.find(Name);
            if (it != Params.end())
            {
                Factor = float4::MakeVector(it->second.ColorFactor().data());
            }
        };

        ReadColorFactor(Mat.Attribs.BaseColorFactor, gltf_mat.values, "baseColorFactor");
        ReadColorFactor(Mat.Attribs.EmissiveFactor, gltf_mat.additionalValues, "emissiveFactor");

        {
            auto alpha_mode_it = gltf_mat.additionalValues.find("alphaMode");
            if (alpha_mode_it != gltf_mat.additionalValues.end())
            {
                const tinygltf::Parameter& param = alpha_mode_it->second;
                if (param.string_value == "BLEND")
                {
                    Mat.Attribs.AlphaMode = Material::ALPHA_MODE_BLEND;
                }
                if (param.string_value == "MASK")
                {
                    Mat.Attribs.AlphaMode   = Material::ALPHA_MODE_MASK;
                    Mat.Attribs.AlphaCutoff = 0.5f;
                }
            }
        }

        ReadFactor(Mat.Attribs.AlphaCutoff, gltf_mat.additionalValues, "alphaCutoff");

        {
            auto double_sided_it = gltf_mat.additionalValues.find("doubleSided");
            if (double_sided_it != gltf_mat.additionalValues.end())
            {
                Mat.DoubleSided = double_sided_it->second.bool_value;
            }
        }

        Mat.Attribs.Workflow = Material::PBR_WORKFLOW_METALL_ROUGH;

        // Extensions
        // @TODO: Find out if there is a nicer way of reading these properties with recent tinygltf headers
        {
            auto ext_it = gltf_mat.extensions.find("KHR_materials_pbrSpecularGlossiness");
            if (ext_it != gltf_mat.extensions.end())
            {
                if (ext_it->second.Has("specularGlossinessTexture"))
                {
                    auto index       = ext_it->second.Get("specularGlossinessTexture").Get("index");
                    auto texCoordSet = ext_it->second.Get("specularGlossinessTexture").Get("texCoord");

                    Mat.TextureIds[Material::TEXTURE_ID_PHYSICAL_DESC] = index.Get<int>();
                    Mat.Attribs.PhysicalDescriptorUVSelector           = static_cast<float>(texCoordSet.Get<int>());

                    Mat.Attribs.Workflow = Material::PBR_WORKFLOW_SPEC_GLOSS;
                }

                if (ext_it->second.Has("diffuseTexture"))
                {
                    auto index       = ext_it->second.Get("diffuseTexture").Get("index");
                    auto texCoordSet = ext_it->second.Get("diffuseTexture").Get("texCoord");

                    Mat.TextureIds[Material::TEXTURE_ID_BASE_COLOR] = index.Get<int>();
                    Mat.Attribs.BaseColorUVSelector                 = static_cast<float>(texCoordSet.Get<int>());
                }

                if (ext_it->second.Has("diffuseFactor"))
                {
                    auto factor = ext_it->second.Get("diffuseFactor");
                    for (uint32_t i = 0; i < factor.ArrayLen(); i++)
                    {
                        const auto val = factor.Get(i);
                        Mat.Attribs.BaseColorFactor[i] =
                            val.IsNumber() ? (float)val.Get<double>() : (float)val.Get<int>();
                    }
                }

                if (ext_it->second.Has("specularFactor"))
                {
                    auto factor = ext_it->second.Get("specularFactor");
                    for (uint32_t i = 0; i < factor.ArrayLen(); i++)
                    {
                        const auto val = factor.Get(i);
                        Mat.Attribs.SpecularFactor[i] =
                            val.IsNumber() ? (float)val.Get<double>() : (float)val.Get<int>();
                    }
                }
            }
        }

        for (const auto& Param : TextureParams)
        {
            auto TexIndex = Mat.TextureIds[Param.TextureId];
            if (TexIndex >= 0)
            {
                const auto& TexInfo = Textures[TexIndex];
                if (TexInfo.pAtlasSuballocation)
                {
                    Param.UVScaleBias = TexInfo.pAtlasSuballocation->GetUVScaleBias();
                    Param.Slice       = static_cast<float>(TexInfo.pAtlasSuballocation->GetSlice());
                }
            }
        }

        Materials.push_back(Mat);
    }

    // Push a default material at the end of the list for meshes with no material assigned
    Materials.push_back(Material{});
}


void Model::LoadAnimations(const tinygltf::Model& gltf_model)
{
    for (const tinygltf::Animation& gltf_anim : gltf_model.animations)
    {
        Animation animation{};
        animation.Name = gltf_anim.name;
        if (gltf_anim.name.empty())
        {
            animation.Name = std::to_string(Animations.size());
        }

        // Samplers
        for (auto& samp : gltf_anim.samplers)
        {
            AnimationSampler AnimSampler{};

            if (samp.interpolation == "LINEAR")
            {
                AnimSampler.Interpolation = AnimationSampler::INTERPOLATION_TYPE::LINEAR;
            }
            else if (samp.interpolation == "STEP")
            {
                AnimSampler.Interpolation = AnimationSampler::INTERPOLATION_TYPE::STEP;
            }
            else if (samp.interpolation == "CUBICSPLINE")
            {
                AnimSampler.Interpolation = AnimationSampler::INTERPOLATION_TYPE::CUBICSPLINE;
            }

            // Read sampler input time values
            {
                const tinygltf::Accessor&   accessor   = gltf_model.accessors[samp.input];
                const tinygltf::BufferView& bufferView = gltf_model.bufferViews[accessor.bufferView];
                const tinygltf::Buffer&     buffer     = gltf_model.buffers[bufferView.buffer];

                VERIFY_EXPR(accessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT);

                const void*  dataPtr = &buffer.data[accessor.byteOffset + bufferView.byteOffset];
                const float* buf     = static_cast<const float*>(dataPtr);
                for (size_t index = 0; index < accessor.count; index++)
                {
                    AnimSampler.Inputs.push_back(buf[index]);
                }

                for (auto input : AnimSampler.Inputs)
                {
                    if (input < animation.Start)
                    {
                        animation.Start = input;
                    }
                    if (input > animation.End)
                    {
                        animation.End = input;
                    }
                }
            }

            // Read sampler output T/R/S values
            {
                const tinygltf::Accessor&   accessor   = gltf_model.accessors[samp.output];
                const tinygltf::BufferView& bufferView = gltf_model.bufferViews[accessor.bufferView];
                const tinygltf::Buffer&     buffer     = gltf_model.buffers[bufferView.buffer];

                VERIFY_EXPR(accessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT);

                const void* dataPtr = &buffer.data[accessor.byteOffset + bufferView.byteOffset];

                switch (accessor.type)
                {
                    case TINYGLTF_TYPE_VEC3:
                    {
                        const float3* buf = static_cast<const float3*>(dataPtr);
                        for (size_t index = 0; index < accessor.count; index++)
                        {
                            AnimSampler.OutputsVec4.push_back(float4(buf[index], 0.0f));
                        }
                        break;
                    }

                    case TINYGLTF_TYPE_VEC4:
                    {
                        const float4* buf = static_cast<const float4*>(dataPtr);
                        for (size_t index = 0; index < accessor.count; index++)
                        {
                            AnimSampler.OutputsVec4.push_back(buf[index]);
                        }
                        break;
                    }

                    default:
                    {
                        LOG_WARNING_MESSAGE("Unknown type", accessor.type);
                        break;
                    }
                }
            }

            animation.Samplers.push_back(AnimSampler);
        }


        for (auto& source : gltf_anim.channels)
        {
            AnimationChannel channel{};

            if (source.target_path == "rotation")
            {
                channel.PathType = AnimationChannel::PATH_TYPE::ROTATION;
            }
            else if (source.target_path == "translation")
            {
                channel.PathType = AnimationChannel::PATH_TYPE::TRANSLATION;
            }
            else if (source.target_path == "scale")
            {
                channel.PathType = AnimationChannel::PATH_TYPE::SCALE;
            }
            else if (source.target_path == "weights")
            {
                LOG_WARNING_MESSAGE("Weights not yet supported, skipping channel");
                continue;
            }

            channel.SamplerIndex = source.sampler;
            channel.pNode        = NodeFromIndex(source.target_node);
            if (!channel.pNode)
            {
                continue;
            }

            animation.Channels.push_back(channel);
        }

        Animations.push_back(animation);
    }
}

namespace Callbacks
{

namespace
{

struct ImageLoaderData
{
    Model::TextureCacheType* const pTextureCache;
    ResourceManager* const         pResourceMgr;

    std::vector<RefCntAutoPtr<IObject>> TexturesHold;

    std::string BaseDir;
};


bool LoadImageData(tinygltf::Image*     gltf_image,
                   const int            gltf_image_idx,
                   std::string*         error,
                   std::string*         warning,
                   int                  req_width,
                   int                  req_height,
                   const unsigned char* image_data,
                   int                  size,
                   void*                user_data)
{
    (void)warning;

    auto* pLoaderData = reinterpret_cast<ImageLoaderData*>(user_data);
    if (pLoaderData != nullptr)
    {
        // TODO: simplify path
        auto CacheId = !gltf_image->uri.empty() ? pLoaderData->BaseDir + gltf_image->uri : "";

        if (pLoaderData->pResourceMgr != nullptr)
        {
            if (auto pAllocation = pLoaderData->pResourceMgr->FindAllocation(CacheId.c_str()))
            {
                const auto& TexDesc    = pAllocation->GetAtlas()->GetAtlasDesc();
                const auto& FmtAttribs = GetTextureFormatAttribs(TexDesc.Format);
                const auto  Size       = pAllocation->GetSize();

                gltf_image->width      = Size.x;
                gltf_image->height     = Size.y;
                gltf_image->component  = FmtAttribs.NumComponents;
                gltf_image->bits       = FmtAttribs.ComponentSize * 8;
                gltf_image->pixel_type = TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE;

                // Keep strong reference to ensure the allocation is alive (second time, but that's fine).
                pLoaderData->TexturesHold.emplace_back(std::move(pAllocation));

                return true;
            }
        }
        else if (pLoaderData->pTextureCache != nullptr)
        {
            auto& TexCache = *pLoaderData->pTextureCache;

            std::lock_guard<std::mutex> Lock{TexCache.TexturesMtx};

            auto it = TexCache.Textures.find(CacheId);
            if (it != TexCache.Textures.end())
            {
                if (auto pTexture = it->second.Lock())
                {
                    const auto& TexDesc    = pTexture->GetDesc();
                    const auto& FmtAttribs = GetTextureFormatAttribs(TexDesc.Format);

                    gltf_image->width      = TexDesc.Width;
                    gltf_image->height     = TexDesc.Height;
                    gltf_image->component  = FmtAttribs.NumComponents;
                    gltf_image->bits       = FmtAttribs.ComponentSize * 8;
                    gltf_image->pixel_type = TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE;

                    // Keep strong reference to ensure the texture is alive (second time, but that's fine).
                    pLoaderData->TexturesHold.emplace_back(std::move(pTexture));

                    return true;
                }
                else
                {
                    // Texture is stale - remove it from the cache
                    TexCache.Textures.erase(it);
                }
            }
        }
    }

    VERIFY(size != 1, "The texture was previously cached, but was not found in the cache now");

    ImageLoadInfo LoadInfo;
    LoadInfo.Format = Image::GetFileFormat(image_data, size);
    if (LoadInfo.Format == IMAGE_FILE_FORMAT_UNKNOWN)
    {
        if (error != nullptr)
        {
            *error += FormatString("Unknown format for image[", gltf_image_idx, "] name = '", gltf_image->name, "'");
        }
        return false;
    }

    if (LoadInfo.Format == IMAGE_FILE_FORMAT_DDS || LoadInfo.Format == IMAGE_FILE_FORMAT_KTX)
    {
        // Store binary data directly
        gltf_image->image.resize(size);
        memcpy(gltf_image->image.data(), image_data, size);
        // Use pixel_type field to indicate the file format
        gltf_image->pixel_type = LoadInfo.Format;
    }
    else
    {
        RefCntAutoPtr<DataBlobImpl> pImageData(MakeNewRCObj<DataBlobImpl>()(size));
        memcpy(pImageData->GetDataPtr(), image_data, size);
        RefCntAutoPtr<Image> pImage;
        Image::CreateFromDataBlob(pImageData, LoadInfo, &pImage);
        if (!pImage)
        {
            if (error != nullptr)
            {
                *error += FormatString("Failed to load image[", gltf_image_idx, "] name = '", gltf_image->name, "'");
            }
            return false;
        }
        const auto& ImgDesc = pImage->GetDesc();

        if (req_width > 0)
        {
            if (static_cast<Uint32>(req_width) != ImgDesc.Width)
            {
                if (error != nullptr)
                {
                    (*error) += FormatString("Image width mismatch for image[",
                                             gltf_image_idx, "] name = '", gltf_image->name,
                                             "': requested width: ",
                                             req_width, ", actual width: ",
                                             ImgDesc.Width);
                }
                return false;
            }
        }

        if (req_height > 0)
        {
            if (static_cast<Uint32>(req_height) != ImgDesc.Height)
            {
                if (error != nullptr)
                {
                    (*error) += FormatString("Image height mismatch for image[",
                                             gltf_image_idx, "] name = '", gltf_image->name,
                                             "': requested height: ",
                                             req_height, ", actual height: ",
                                             ImgDesc.Height);
                }
                return false;
            }
        }

        gltf_image->width      = ImgDesc.Width;
        gltf_image->height     = ImgDesc.Height;
        gltf_image->component  = 4;
        gltf_image->bits       = GetValueSize(ImgDesc.ComponentType) * 8;
        gltf_image->pixel_type = TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE;
        auto DstRowSize        = gltf_image->width * gltf_image->component * (gltf_image->bits / 8);
        gltf_image->image.resize(static_cast<size_t>(gltf_image->height * DstRowSize));
        auto*        pPixelsBlob = pImage->GetData();
        const Uint8* pSrcPixels  = reinterpret_cast<const Uint8*>(pPixelsBlob->GetDataPtr());
        if (ImgDesc.NumComponents == 3)
        {
            for (Uint32 row = 0; row < ImgDesc.Height; ++row)
            {
                for (Uint32 col = 0; col < ImgDesc.Width; ++col)
                {
                    Uint8*       DstPixel = gltf_image->image.data() + DstRowSize * row + col * gltf_image->component;
                    const Uint8* SrcPixel = pSrcPixels + ImgDesc.RowStride * row + col * ImgDesc.NumComponents;

                    DstPixel[0] = SrcPixel[0];
                    DstPixel[1] = SrcPixel[1];
                    DstPixel[2] = SrcPixel[2];
                    DstPixel[3] = 255;
                }
            }
        }
        else if (gltf_image->component == 4)
        {
            for (Uint32 row = 0; row < ImgDesc.Height; ++row)
            {
                memcpy(gltf_image->image.data() + DstRowSize * row, pSrcPixels + ImgDesc.RowStride * row, DstRowSize);
            }
        }
        else
        {
            *error += FormatString("Unexpected number of image comonents (", ImgDesc.NumComponents, ")");
            return false;
        }
    }

    return true;
}

bool FileExists(const std::string& abs_filename, void*)
{
    return FileSystem::FileExists(abs_filename.c_str());
}

bool ReadWholeFile(std::vector<unsigned char>* out,
                   std::string*                err,
                   const std::string&          filepath,
                   void*                       user_data)
{
    // Try to find the file in the texture cache to avoid reading it
    if (auto* pLoaderData = reinterpret_cast<ImageLoaderData*>(user_data))
    {
        if (pLoaderData->pResourceMgr != nullptr)
        {
            if (auto pAllocation = pLoaderData->pResourceMgr->FindAllocation(filepath.c_str()))
            {
                // Keep strong reference to ensure the allocation is alive.
                pLoaderData->TexturesHold.emplace_back(std::move(pAllocation));
                // Tiny GLTF checks the size of 'out', it can't be empty
                out->resize(1);
                return true;
            }
        }
        else if (pLoaderData->pTextureCache != nullptr)
        {
            std::lock_guard<std::mutex> Lock{pLoaderData->pTextureCache->TexturesMtx};

            auto it = pLoaderData->pTextureCache->Textures.find(filepath.c_str());
            if (it != pLoaderData->pTextureCache->Textures.end())
            {
                if (auto pTexture = it->second.Lock())
                {
                    // Keep strong reference to ensure the texture is alive.
                    pLoaderData->TexturesHold.emplace_back(std::move(pTexture));
                    // Tiny GLTF checks the size of 'out', it can't be empty
                    out->resize(1);
                    return true;
                }
            }
        }
    }

    FileWrapper pFile{filepath.c_str(), EFileAccessMode::Read};
    if (!pFile)
    {
        if (err)
        {
            (*err) += FormatString("Unable to open file ", filepath, "\n");
        }
        return false;
    }

    auto size = pFile->GetSize();
    if (size == 0)
    {
        if (err)
        {
            (*err) += FormatString("File is empty: ", filepath, "\n");
        }
        return false;
    }

    out->resize(size);
    pFile->Read(out->data(), size);

    return true;
}

} // namespace

} // namespace Callbacks

void Model::LoadFromFile(IRenderDevice*    pDevice,
                         IDeviceContext*   pContext,
                         const CreateInfo& CI)
{
    if (CI.FileName == nullptr || *CI.FileName == 0)
        LOG_ERROR_AND_THROW("File path must not be empty");

    auto* const pTextureCache = CI.pTextureCache;
    auto* const pResourceMgr  = CI.pCacheInfo != nullptr ? CI.pCacheInfo->pResourceMgr : nullptr;
    if (CI.pTextureCache != nullptr && pResourceMgr != nullptr)
        LOG_WARNING_MESSAGE("Texture cache is ignored when resource manager is used");

    Callbacks::ImageLoaderData LoaderData{pTextureCache, pResourceMgr, {}, ""};

    const std::string filename{CI.FileName};
    if (filename.find_last_of("/\\") != std::string::npos)
        LoaderData.BaseDir = filename.substr(0, filename.find_last_of("/\\"));
    LoaderData.BaseDir += '/';

    tinygltf::TinyGLTF gltf_context;
    gltf_context.SetImageLoader(Callbacks::LoadImageData, &LoaderData);
    tinygltf::FsCallbacks fsCallbacks = {};
    fsCallbacks.ExpandFilePath        = tinygltf::ExpandFilePath;
    fsCallbacks.FileExists            = Callbacks::FileExists;
    fsCallbacks.ReadWholeFile         = Callbacks::ReadWholeFile;
    fsCallbacks.WriteWholeFile        = tinygltf::WriteWholeFile;
    fsCallbacks.user_data             = &LoaderData;
    gltf_context.SetFsCallbacks(fsCallbacks);

    bool   binary = false;
    size_t extpos = filename.rfind('.', filename.length());
    if (extpos != std::string::npos)
    {
        binary = (filename.substr(extpos + 1, filename.length() - extpos) == "glb");
    }

    std::string     error;
    std::string     warning;
    tinygltf::Model gltf_model;

    bool fileLoaded = false;
    if (binary)
        fileLoaded = gltf_context.LoadBinaryFromFile(&gltf_model, &error, &warning, filename.c_str());
    else
        fileLoaded = gltf_context.LoadASCIIFromFile(&gltf_model, &error, &warning, filename.c_str());
    if (!fileLoaded)
    {
        LOG_ERROR_AND_THROW("Failed to load gltf file ", filename, ": ", error);
    }
    if (!warning.empty())
    {
        LOG_WARNING_MESSAGE("Loaded gltf file ", filename, " with the following warning:", warning);
    }

    LoadTextureSamplers(pDevice, gltf_model);
    LoadTextures(pDevice, gltf_model, LoaderData.BaseDir, pTextureCache, pResourceMgr);
    LoadMaterials(gltf_model);

    std::vector<Uint32>             IndexData;
    std::vector<VertexBasicAttribs> VertexBasicData;
    std::vector<VertexSkinAttribs>  VertexSkinData;

    // TODO: scene handling with no default scene
    const tinygltf::Scene& scene = gltf_model.scenes[gltf_model.defaultScene > -1 ? gltf_model.defaultScene : 0];
    for (size_t i = 0; i < scene.nodes.size(); i++)
    {
        const tinygltf::Node node = gltf_model.nodes[scene.nodes[i]];
        LoadNode(nullptr, node, scene.nodes[i], gltf_model,
                 IndexData, VertexBasicData,
                 CI.LoadAnimationAndSkin ? &VertexSkinData : nullptr);
    }

    if (CI.LoadAnimationAndSkin)
    {
        if (gltf_model.animations.size() > 0)
        {
            LoadAnimations(gltf_model);
        }
        LoadSkins(gltf_model);
    }

    for (auto* node : LinearNodes)
    {
        // Assign skins
        if (node->SkinIndex >= 0)
        {
            node->pSkin = Skins[node->SkinIndex].get();
        }
    }

    // Initial pose
    for (auto& root_node : Nodes)
    {
        root_node->UpdateTransforms();
    }
    CalculateSceneDimensions();


    Extensions = gltf_model.extensionsUsed;

    auto CreateBuffer = [&](BUFFER_ID BuffId, const void* pData, size_t Size, BIND_FLAGS BindFlags, const char* Name) //
    {
        VERIFY_EXPR(Size > 0);

        auto BufferSize = static_cast<Uint32>(Size);
        if (pResourceMgr != nullptr)
        {
            Uint32 CacheBufferIndex = 0;
            switch (BuffId)
            {
                case BUFFER_ID_INDEX:
                    CacheBufferIndex = CI.pCacheInfo->IndexBufferIdx;
                    break;
                case BUFFER_ID_VERTEX_BASIC_ATTRIBS:
                    CacheBufferIndex = CI.pCacheInfo->VertexBuffer0Idx;
                    break;
                case BUFFER_ID_VERTEX_SKIN_ATTRIBS:
                    CacheBufferIndex = CI.pCacheInfo->VertexBuffer1Idx;
                    break;
                default:
                    UNEXPECTED("Unknown buffer id ", BuffId);
            }
            Buffers[BuffId].pSuballocation = pResourceMgr->AllocateBufferSpace(CacheBufferIndex, BufferSize, 1);

            RefCntAutoPtr<DataBlobImpl> pBuffInitData{MakeNewRCObj<DataBlobImpl>()(Size)};
            memcpy(pBuffInitData->GetDataPtr(), pData, Size);
            Buffers[BuffId].pSuballocation->SetUserData(pBuffInitData);
        }
        else
        {
            BufferDesc BuffDesc;
            BuffDesc.Name          = Name;
            BuffDesc.uiSizeInBytes = BufferSize;
            BuffDesc.BindFlags     = BindFlags;
            BuffDesc.Usage         = USAGE_IMMUTABLE;

            BufferData BuffData{pData, BuffDesc.uiSizeInBytes};
            pDevice->CreateBuffer(BuffDesc, &BuffData, &Buffers[BuffId].pBuffer);
        }
    };

    CreateBuffer(BUFFER_ID_VERTEX_BASIC_ATTRIBS, VertexBasicData.data(), VertexBasicData.size() * sizeof(VertexBasicData[0]),
                 BIND_VERTEX_BUFFER, "GLTF vertex attribs 0 buffer");

    if (!VertexSkinData.empty())
    {
        CreateBuffer(BUFFER_ID_VERTEX_SKIN_ATTRIBS, VertexSkinData.data(), VertexSkinData.size() * sizeof(VertexSkinData[0]),
                     BIND_VERTEX_BUFFER, "GLTF vertex attribs 1 buffer");
    }

    if (!IndexData.empty())
    {
        CreateBuffer(BUFFER_ID_INDEX, IndexData.data(), IndexData.size() * sizeof(IndexData[0]),
                     BIND_INDEX_BUFFER, "GLTF index buffer");
    }

    if (pContext != nullptr)
    {
        PrepareGPUResources(pDevice, pContext);
    }
}

void Model::CalculateBoundingBox(Node* node, const Node* parent)
{
    BoundBox parentBvh = parent ? parent->BVH : BoundBox{dimensions.min, dimensions.max};

    if (node->pMesh)
    {
        if (node->pMesh->IsValidBB())
        {
            node->AABB = node->pMesh->BB.Transform(node->GetMatrix());
            if (node->Children.empty())
            {
                node->BVH.Min    = node->AABB.Min;
                node->BVH.Max    = node->AABB.Max;
                node->IsValidBVH = true;
            }
        }
    }

    parentBvh.Min = std::min(parentBvh.Min, node->BVH.Min);
    parentBvh.Max = std::max(parentBvh.Max, node->BVH.Max);

    for (auto& child : node->Children)
    {
        CalculateBoundingBox(child.get(), node);
    }
}

void Model::CalculateSceneDimensions()
{
    // Calculate binary volume hierarchy for all nodes in the scene
    for (auto* node : LinearNodes)
    {
        CalculateBoundingBox(node, nullptr);
    }

    dimensions.min = float3{+FLT_MAX, +FLT_MAX, +FLT_MAX};
    dimensions.max = float3{-FLT_MAX, -FLT_MAX, -FLT_MAX};

    for (const auto* node : LinearNodes)
    {
        if (node->IsValidBVH)
        {
            dimensions.min = std::min(dimensions.min, node->BVH.Min);
            dimensions.max = std::max(dimensions.max, node->BVH.Max);
        }
    }

    // Calculate scene AABBTransform
    AABBTransform       = float4x4::Scale(dimensions.max[0] - dimensions.min[0], dimensions.max[1] - dimensions.min[1], dimensions.max[2] - dimensions.min[2]);
    AABBTransform[3][0] = dimensions.min[0];
    AABBTransform[3][1] = dimensions.min[1];
    AABBTransform[3][2] = dimensions.min[2];
}

void Model::UpdateAnimation(Uint32 index, float time)
{
    if (index > static_cast<Uint32>(Animations.size()) - 1)
    {
        LOG_WARNING_MESSAGE("No animation with index ", index);
        return;
    }
    Animation& animation = Animations[index];

    bool updated = false;
    for (auto& channel : animation.Channels)
    {
        AnimationSampler& sampler = animation.Samplers[channel.SamplerIndex];
        if (sampler.Inputs.size() > sampler.OutputsVec4.size())
        {
            continue;
        }

        for (size_t i = 0; i < sampler.Inputs.size() - 1; i++)
        {
            if ((time >= sampler.Inputs[i]) && (time <= sampler.Inputs[i + 1]))
            {
                float u = std::max(0.0f, time - sampler.Inputs[i]) / (sampler.Inputs[i + 1] - sampler.Inputs[i]);
                if (u <= 1.0f)
                {
                    switch (channel.PathType)
                    {
                        case AnimationChannel::PATH_TYPE::TRANSLATION:
                        {
                            float4 trans               = lerp(sampler.OutputsVec4[i], sampler.OutputsVec4[i + 1], u);
                            channel.pNode->Translation = float3(trans);
                            break;
                        }

                        case AnimationChannel::PATH_TYPE::SCALE:
                        {
                            float4 scale         = lerp(sampler.OutputsVec4[i], sampler.OutputsVec4[i + 1], u);
                            channel.pNode->Scale = float3(scale);
                            break;
                        }

                        case AnimationChannel::PATH_TYPE::ROTATION:
                        {
                            Quaternion q1;
                            q1.q.x = sampler.OutputsVec4[i].x;
                            q1.q.y = sampler.OutputsVec4[i].y;
                            q1.q.z = sampler.OutputsVec4[i].z;
                            q1.q.w = sampler.OutputsVec4[i].w;

                            Quaternion q2;
                            q2.q.x = sampler.OutputsVec4[i + 1].x;
                            q2.q.y = sampler.OutputsVec4[i + 1].y;
                            q2.q.z = sampler.OutputsVec4[i + 1].z;
                            q2.q.w = sampler.OutputsVec4[i + 1].w;

                            channel.pNode->Rotation = normalize(slerp(q1, q2, u));
                            break;
                        }
                    }
                    updated = true;
                }
            }
        }
    }

    if (updated)
    {
        for (auto& node : Nodes)
        {
            node->UpdateTransforms();
        }
    }
}

void Model::Transform(const float4x4& Matrix)
{
    for (auto& root_node : Nodes)
    {
        root_node->Matrix *= Matrix;
        root_node->UpdateTransforms();
    }

    CalculateSceneDimensions();
}

Node* Model::FindNode(Node* parent, Uint32 index)
{
    Node* nodeFound = nullptr;
    if (parent->Index == index)
    {
        return parent;
    }
    for (auto& child : parent->Children)
    {
        nodeFound = FindNode(child.get(), index);
        if (nodeFound)
        {
            break;
        }
    }
    return nodeFound;
}


Node* Model::NodeFromIndex(uint32_t index)
{
    Node* nodeFound = nullptr;
    for (auto& node : Nodes)
    {
        nodeFound = FindNode(node.get(), index);
        if (nodeFound)
        {
            break;
        }
    }
    return nodeFound;
}


} // namespace GLTF

} // namespace Diligent
