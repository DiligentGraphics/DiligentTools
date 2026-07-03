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

#pragma once

// This adapter expects GLTFLoader.hpp and tiny_gltf.h to be included before it.

namespace Diligent
{

namespace GLTF
{

inline VALUE_TYPE TinyGltfComponentTypeToValueType(int GltfCompType)
{
    switch (GltfCompType)
    {
            // clang-format off
        case TINYGLTF_COMPONENT_TYPE_BYTE:           return VT_INT8;
        case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE:  return VT_UINT8;
        case TINYGLTF_COMPONENT_TYPE_SHORT:          return VT_INT16;
        case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT: return VT_UINT16;
        case TINYGLTF_COMPONENT_TYPE_INT:            return VT_INT32;
        case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT:   return VT_UINT32;
        case TINYGLTF_COMPONENT_TYPE_FLOAT:          return VT_FLOAT32;
        case TINYGLTF_COMPONENT_TYPE_DOUBLE:         return VT_FLOAT64;
        // clang-format on
        default:
            UNEXPECTED("Unknown GLTF component type");
            return VT_UNDEFINED;
    }
}

struct TinyGltfNodeView
{
    const tinygltf::Node& Node;

    const tinygltf::Node& Get() const { return Node; }

    // clang-format off
    const std::string&         GetName()        const { return Node.name; }
    const std::vector<double>& GetTranslation() const { return Node.translation; }
    const std::vector<double>& GetRotation()    const { return Node.rotation; }
    const std::vector<double>& GetScale()       const { return Node.scale; }
    const std::vector<double>& GetMatrix()      const { return Node.matrix; }
    const std::vector<int>&    GetChildrenIds() const { return Node.children; }

    int GetMeshId()   const { return Node.mesh; }
    int GetCameraId() const { return Node.camera; }
    int GetLightId()  const { return Node.light; }
    int GetSkinId()   const { return Node.skin; }
    // clang-format on
};

struct TinyGltfPrimitiveView
{
    const tinygltf::Primitive& Primitive;

    const int* GetAttribute(const char* Name) const
    {
        auto attrib_it = Primitive.attributes.find(Name);
        return attrib_it != Primitive.attributes.end() ?
            &attrib_it->second :
            nullptr;
    }

    const tinygltf::Primitive& Get() const { return Primitive; }

    int GetIndicesId() const { return Primitive.indices; }
    int GetMaterialId() const { return Primitive.material; }
};

struct TinyGltfMeshView
{
    const tinygltf::Mesh& Mesh;

    const tinygltf::Mesh& Get() const { return Mesh; }
    const std::string&    GetName() const { return Mesh.name; }

    size_t                GetPrimitiveCount() const { return Mesh.primitives.size(); }
    TinyGltfPrimitiveView GetPrimitive(size_t Idx) const { return TinyGltfPrimitiveView{Mesh.primitives[Idx]}; }
};

struct TinyGltfBufferViewView;

struct TinyGltfAccessorView
{
    const tinygltf::Accessor& Accessor;

    size_t GetCount() const { return Accessor.count; }
    float3 GetMinValues() const
    {
        return float3{
            static_cast<float>(Accessor.minValues[0]),
            static_cast<float>(Accessor.minValues[1]),
            static_cast<float>(Accessor.minValues[2]),
        };
    }
    float3 GetMaxValues() const
    {
        return float3{
            static_cast<float>(Accessor.maxValues[0]),
            static_cast<float>(Accessor.maxValues[1]),
            static_cast<float>(Accessor.maxValues[2]),
        };
    }

    // clang-format off
    int        GetBufferViewId()  const { return Accessor.bufferView; }
    size_t     GetByteOffset()    const { return Accessor.byteOffset; }
    VALUE_TYPE GetComponentType() const { return TinyGltfComponentTypeToValueType(Accessor.componentType); }
    int32_t    GetNumComponents() const { return tinygltf::GetNumComponentsInType(Accessor.type); }
    bool       IsNormalized()     const { return Accessor.normalized; }
    // clang-format on

    int GetByteStride(const TinyGltfBufferViewView& View) const;
};

struct TinyGltfPerspectiveCameraView
{
    const tinygltf::PerspectiveCamera& Camera;

    // clang-format off
    double GetAspectRatio() const { return Camera.aspectRatio; }
    double GetYFov()        const { return Camera.yfov; }
    double GetZNear()       const { return Camera.znear; }
    double GetZFar()        const { return Camera.zfar; }
    // clang-format on
};

struct TinyGltfOrthoCameraView
{
    const tinygltf::OrthographicCamera& Camera;

    // clang-format off
    double GetXMag()  const { return Camera.xmag; }
    double GetYMag()  const { return Camera.ymag; }
    double GetZNear() const { return Camera.znear; }
    double GetZFar()  const { return Camera.zfar; }
    // clang-format on
};

struct TinyGltfCameraView
{
    const tinygltf::Camera& Camera;

    const std::string&            GetName() const { return Camera.name; }
    const std::string&            GetType() const { return Camera.type; }
    TinyGltfPerspectiveCameraView GetPerspective() const { return TinyGltfPerspectiveCameraView{Camera.perspective}; }
    TinyGltfOrthoCameraView       GetOrthographic() const { return TinyGltfOrthoCameraView{Camera.orthographic}; }
};

struct TinyGltfLightView
{
    const tinygltf::Light& Light;

    const std::string&         GetName() const { return Light.name; }
    const std::string&         GetType() const { return Light.type; }
    const std::vector<double>& GetColor() const { return Light.color; }
    double                     GetIntensity() const { return Light.intensity; }
    double                     GetRange() const { return Light.range; }
    double                     GetInnerConeAngle() const { return Light.spot.innerConeAngle; }
    double                     GetOuterConeAngle() const { return Light.spot.outerConeAngle; }
};

struct TinyGltfBufferViewView
{
    const tinygltf::BufferView& View;

    int    GetBufferId() const { return View.buffer; }
    size_t GetByteOffset() const { return View.byteOffset; }
};

inline int TinyGltfAccessorView::GetByteStride(const TinyGltfBufferViewView& View) const
{
    return Accessor.ByteStride(View.View);
}

struct TinyGltfBufferView
{
    const tinygltf::Buffer& Buffer;

    const unsigned char* GetData(size_t Offset) const
    {
        return Offset < Buffer.data.size() ? Buffer.data.data() + Offset : nullptr;
    }
};

struct TinyGltfSkinView
{
    const tinygltf::Skin& Skin;

    const std::string&      GetName() const { return Skin.name; }
    int                     GetSkeletonId() const { return Skin.skeleton; }
    const std::vector<int>& GetJointIds() const { return Skin.joints; }
    int                     GetInverseBindMatricesId() const { return Skin.inverseBindMatrices; }
};

struct TinyGltfAnimationSamplerView
{
    const tinygltf::AnimationSampler& Sam;

    AnimationSampler::INTERPOLATION_TYPE GetInterpolation() const
    {
        if (Sam.interpolation == "LINEAR")
            return AnimationSampler::INTERPOLATION_TYPE::LINEAR;
        if (Sam.interpolation == "STEP")
            return AnimationSampler::INTERPOLATION_TYPE::STEP;
        if (Sam.interpolation == "CUBICSPLINE")
            return AnimationSampler::INTERPOLATION_TYPE::CUBICSPLINE;

        UNEXPECTED("Unexpected animation interpolation type: ", Sam.interpolation);
        return AnimationSampler::INTERPOLATION_TYPE::LINEAR;
    }

    int GetInputId() const { return Sam.input; }
    int GetOutputId() const { return Sam.output; }
};

struct TinyGltfAnimationChannelView
{
    const tinygltf::AnimationChannel& Channel;

    AnimationChannel::PATH_TYPE GetPathType() const
    {
        if (Channel.target_path == "translation")
            return AnimationChannel::PATH_TYPE::TRANSLATION;
        if (Channel.target_path == "rotation")
            return AnimationChannel::PATH_TYPE::ROTATION;
        if (Channel.target_path == "scale")
            return AnimationChannel::PATH_TYPE::SCALE;
        if (Channel.target_path == "weights")
            return AnimationChannel::PATH_TYPE::WEIGHTS;

        UNEXPECTED("Unsupported animation channel path ", Channel.target_path);
        return AnimationChannel::PATH_TYPE::ROTATION;
    }
    int GetSamplerId() const { return Channel.sampler; }
    int GetTargetNodeId() const { return Channel.target_node; }
};

struct TinyGltfAnimationView
{
    const tinygltf::Animation& Anim;

    const std::string& GetName() const { return Anim.name; }

    size_t GetSamplerCount() const { return Anim.samplers.size(); }
    size_t GetChannelCount() const { return Anim.channels.size(); }

    TinyGltfAnimationSamplerView GetSampler(size_t Id) const { return TinyGltfAnimationSamplerView{Anim.samplers[Id]}; }
    TinyGltfAnimationChannelView GetChannel(size_t Id) const { return TinyGltfAnimationChannelView{Anim.channels[Id]}; }
};

struct TinyGltfSceneView
{
    const tinygltf::Scene& Scene;

    const std::string& GetName() const { return Scene.name; }
    size_t             GetNodeCount() const { return Scene.nodes.size(); }
    int                GetNodeId(size_t Idx) const { return Scene.nodes[Idx]; }
};

struct TinyGltfModelView
{
    const tinygltf::Model& Model;

    const tinygltf::Model& Get() const { return Model; }

    // clang-format off
    TinyGltfNodeView       GetNode      (int idx) const { return TinyGltfNodeView      {Model.nodes      [idx]}; }
    TinyGltfSceneView      GetScene     (int idx) const { return TinyGltfSceneView     {Model.scenes     [idx]}; }
    TinyGltfMeshView       GetMesh      (int idx) const { return TinyGltfMeshView      {Model.meshes     [idx]}; }
    TinyGltfAccessorView   GetAccessor  (int idx) const { return TinyGltfAccessorView  {Model.accessors  [idx]}; }
    TinyGltfCameraView     GetCamera    (int idx) const { return TinyGltfCameraView    {Model.cameras    [idx]}; }
    TinyGltfLightView      GetLight     (int idx) const { return TinyGltfLightView     {Model.lights     [idx]}; }
    TinyGltfBufferViewView GetBufferView(int idx) const { return TinyGltfBufferViewView{Model.bufferViews[idx]}; }
    TinyGltfBufferView     GetBuffer    (int idx) const { return TinyGltfBufferView    {Model.buffers    [idx]}; }

    TinyGltfSkinView      GetSkin      (size_t idx) const { return TinyGltfSkinView     {Model.skins      [idx]}; }
    TinyGltfAnimationView GetAnimation (size_t idx) const { return TinyGltfAnimationView{Model.animations [idx]}; }

    size_t GetNodeCount()      const { return Model.nodes.size();      }
    size_t GetSceneCount()     const { return Model.scenes.size();     }
    size_t GetMeshCount()      const { return Model.meshes.size();     }
    size_t GetSkinCount()      const { return Model.skins.size();      }
    size_t GetAnimationCount() const { return Model.animations.size(); }

    int GetDefaultSceneId() const { return Model.defaultScene; }
    // clang-format on
};

} // namespace GLTF

} // namespace Diligent
