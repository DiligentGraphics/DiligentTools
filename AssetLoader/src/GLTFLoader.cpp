/*
 *  Copyright 2019-2024 Diligent Graphics LLC
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
#include "TextureUtilities.h"
#include "GraphicsUtilities.h"
#include "Align.hpp"
#include "GLTFBuilder.hpp"
#include "FixedLinearAllocator.hpp"
#include "DefaultRawMemoryAllocator.hpp"

#define TINYGLTF_IMPLEMENTATION
#define TINYGLTF_NO_STB_IMAGE
#define TINYGLTF_NO_STB_IMAGE_WRITE

#if defined(_MSC_VER) && defined(TINYGLTF_ENABLE_DRACO)
#    pragma warning(disable : 4127) // warning C4127: conditional expression is constant
#endif
#include "../../ThirdParty/tinygltf/tiny_gltf.h"

namespace Diligent
{

namespace GLTF
{

InputLayoutDescX VertexAttributesToInputLayout(const VertexAttributeDesc* pAttributes, size_t NumAttributes)
{
    VERIFY_EXPR(pAttributes != nullptr || NumAttributes == 0);
    InputLayoutDescX InputLayout;
    for (Uint32 i = 0; i < NumAttributes; ++i)
    {
        const auto& Attrib       = pAttributes[i];
        const auto  IsNormalized = (Attrib.ValueType == VT_UINT8 || Attrib.ValueType == VT_INT8);
        InputLayout.Add(i, Attrib.BufferId, Attrib.NumComponents, Attrib.ValueType, IsNormalized, Attrib.RelativeOffset);
    }
    return InputLayout;
}

namespace
{

VALUE_TYPE TinyGltfComponentTypeToValueType(int GltfCompType)
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


struct TinyGltfNodeWrapper
{
    const tinygltf::Node& Node;

    const auto& Get() const { return Node; }

    // clang-format off
    const auto& GetName()        const { return Node.name; }
    const auto& GetTranslation() const { return Node.translation; }
    const auto& GetRotation()    const { return Node.rotation; }
    const auto& GetScale()       const { return Node.scale; }
    const auto& GetMatrix()      const { return Node.matrix; }
    const auto& GetChildrenIds() const { return Node.children; }
    auto        GetMeshId()      const { return Node.mesh; }
    auto        GetCameraId()    const { return Node.camera; }
    auto        GetLightId()     const { return Node.light; }
    auto        GetSkinId()      const { return Node.skin; }
    // clang-format on
};

struct TinyGltfPrimitiveWrapper
{
    const tinygltf::Primitive& Primitive;

    const int* GetAttribute(const char* Name) const
    {
        auto attrib_it = Primitive.attributes.find(Name);
        return attrib_it != Primitive.attributes.end() ?
            &attrib_it->second :
            nullptr;
    }

    const auto& Get() const { return Primitive; }

    auto GetIndicesId() const { return Primitive.indices; }
    auto GetMaterialId() const { return Primitive.material; }
};

struct TinyGltfMeshWrapper
{
    const tinygltf::Mesh& Mesh;

    const auto& Get() const { return Mesh; }
    const auto& GetName() const { return Mesh.name; }

    auto GetPrimitiveCount() const { return Mesh.primitives.size(); }
    auto GetPrimitive(size_t Idx) const { return TinyGltfPrimitiveWrapper{Mesh.primitives[Idx]}; };
};

struct TinyGltfBufferViewWrapper;
struct TinyGltfAccessorWrapper
{
    const tinygltf::Accessor& Accessor;

    auto GetCount() const { return Accessor.count; }
    auto GetMinValues() const
    {
        return float3{
            static_cast<float>(Accessor.minValues[0]),
            static_cast<float>(Accessor.minValues[1]),
            static_cast<float>(Accessor.minValues[2]),
        };
    }
    auto GetMaxValues() const
    {
        return float3{
            static_cast<float>(Accessor.maxValues[0]),
            static_cast<float>(Accessor.maxValues[1]),
            static_cast<float>(Accessor.maxValues[2]),
        };
    }

    // clang-format off
    auto GetBufferViewId()  const { return Accessor.bufferView; }
    auto GetByteOffset()    const { return Accessor.byteOffset; }
    auto GetComponentType() const { return TinyGltfComponentTypeToValueType(Accessor.componentType); }
    auto GetNumComponents() const { return tinygltf::GetNumComponentsInType(Accessor.type); }
    bool IsNormalized()     const { return Accessor.normalized; }
    // clang-format on
    auto GetByteStride(const TinyGltfBufferViewWrapper& View) const;
};

struct TinyGltfPerspectiveCameraWrapper
{
    const tinygltf::PerspectiveCamera& Camera;

    // clang-format off
    auto GetAspectRatio() const { return Camera.aspectRatio; }
    auto GetYFov()        const { return Camera.yfov; }
    auto GetZNear()       const { return Camera.znear; }
    auto GetZFar()        const { return Camera.zfar; }
    // clang-format on
};

struct TinyGltfOrthoCameraWrapper
{
    const tinygltf::OrthographicCamera& Camera;

    // clang-format off
    auto GetXMag()  const { return Camera.xmag; }
    auto GetYMag()  const { return Camera.ymag; }
    auto GetZNear() const { return Camera.znear; }
    auto GetZFar()  const { return Camera.zfar; }
    // clang-format on
};

struct TinyGltfCameraWrapper
{
    const tinygltf::Camera& Camera;

    const auto& GetName() const { return Camera.name; }
    const auto& GetType() const { return Camera.type; }
    auto        GetPerspective() const { return TinyGltfPerspectiveCameraWrapper{Camera.perspective}; }
    auto        GetOrthographic() const { return TinyGltfOrthoCameraWrapper{Camera.orthographic}; }
};

struct TinyGltfLightWrapper
{
    const tinygltf::Light& Light;

    // clang-format off
    const auto& GetName()           const { return Light.name; }
    const auto& GetType()           const { return Light.type; }
    const auto& GetColor()          const { return Light.color; }
    const auto& GetIntensity()      const { return Light.intensity; }
    const auto& GetRange()          const { return Light.range; }
    const auto& GetInnerConeAngle() const { return Light.spot.innerConeAngle; }
    const auto& GetOuterConeAngle() const { return Light.spot.outerConeAngle; }
    // clang-format on
};

struct TinyGltfBufferViewWrapper
{
    const tinygltf::BufferView& View;

    auto GetBufferId() const { return View.buffer; }
    auto GetByteOffset() const { return View.byteOffset; }
};

struct TinyGltfBufferWrapper
{
    const tinygltf::Buffer& Buffer;

    const auto* GetData(size_t Offset) const { return &Buffer.data[Offset]; }
};

struct TinyGltfSkinWrapper
{
    const tinygltf::Skin& Skin;

    const auto& GetName() const { return Skin.name; }
    auto        GetSkeletonId() const { return Skin.skeleton; }
    auto        GetInverseBindMatricesId() const { return Skin.inverseBindMatrices; }
    const auto& GetJointIds() const { return Skin.joints; }
};

struct TinyGltfAnimationSamplerWrapper
{
    const tinygltf::AnimationSampler& Sam;

    AnimationSampler::INTERPOLATION_TYPE GetInterpolation() const
    {
        if (Sam.interpolation == "LINEAR")
            return AnimationSampler::INTERPOLATION_TYPE::LINEAR;
        else if (Sam.interpolation == "STEP")
            return AnimationSampler::INTERPOLATION_TYPE::STEP;
        else if (Sam.interpolation == "CUBICSPLINE")
            return AnimationSampler::INTERPOLATION_TYPE::CUBICSPLINE;
        else
        {
            UNEXPECTED("Unexpected animation interpolation type: ", Sam.interpolation);
            return AnimationSampler::INTERPOLATION_TYPE::LINEAR;
        }
    }

    auto GetInputId() const { return Sam.input; }
    auto GetOutputId() const { return Sam.output; }
};


struct TinyGltfAnimationChannelWrapper
{
    const tinygltf::AnimationChannel& Channel;

    AnimationChannel::PATH_TYPE GetPathType() const
    {
        if (Channel.target_path == "rotation")
            return AnimationChannel::PATH_TYPE::ROTATION;
        else if (Channel.target_path == "translation")
            return AnimationChannel::PATH_TYPE::TRANSLATION;
        else if (Channel.target_path == "scale")
            return AnimationChannel::PATH_TYPE::SCALE;
        else if (Channel.target_path == "weights")
            return AnimationChannel::PATH_TYPE::WEIGHTS;
        else
        {
            UNEXPECTED("Unsupported animation channel path ", Channel.target_path);
            return AnimationChannel::PATH_TYPE::ROTATION;
        }
    }

    auto GetSamplerId() const { return Channel.sampler; }
    auto GetTargetNodeId() const { return Channel.target_node; }
};

struct TinyGltfAnimationWrapper
{
    const tinygltf::Animation& Anim;

    const auto& GetName() const { return Anim.name; }

    auto GetSamplerCount() const { return Anim.samplers.size(); }
    auto GetChannelCount() const { return Anim.channels.size(); }
    auto GetSampler(size_t Id) const { return TinyGltfAnimationSamplerWrapper{Anim.samplers[Id]}; }
    auto GetChannel(size_t Id) const { return TinyGltfAnimationChannelWrapper{Anim.channels[Id]}; }
};

struct TinyGltfSceneWrapper
{
    const tinygltf::Scene& Scene;

    const auto& GetName() const { return Scene.name; }
    auto        GetNodeCount() const { return Scene.nodes.size(); }
    auto        GetNodeId(size_t Idx) const { return Scene.nodes[Idx]; }
};

struct TinyGltfModelWrapper
{
    const tinygltf::Model& Model;

    // clang-format off
    auto GetNode      (int idx) const { return TinyGltfNodeWrapper      {Model.nodes      [idx]}; }
    auto GetScene     (int idx) const { return TinyGltfSceneWrapper     {Model.scenes     [idx]}; }
    auto GetMesh      (int idx) const { return TinyGltfMeshWrapper      {Model.meshes     [idx]}; }
    auto GetAccessor  (int idx) const { return TinyGltfAccessorWrapper  {Model.accessors  [idx]}; }
    auto GetCamera    (int idx) const { return TinyGltfCameraWrapper    {Model.cameras    [idx]}; }
    auto GetLight     (int idx) const { return TinyGltfLightWrapper     {Model.lights     [idx]}; }
    auto GetBufferView(int idx) const { return TinyGltfBufferViewWrapper{Model.bufferViews[idx]}; }
    auto GetBuffer    (int idx) const { return TinyGltfBufferWrapper    {Model.buffers    [idx]}; }

    auto GetSkin      (size_t idx) const { return TinyGltfSkinWrapper      {Model.skins      [idx]}; }
    auto GetAnimation (size_t idx) const { return TinyGltfAnimationWrapper {Model.animations [idx]}; }

    auto GetNodeCount()      const { return Model.nodes.size();      }
    auto GetSceneCount()     const { return Model.scenes.size();     }
    auto GetMeshCount()      const { return Model.meshes.size();     }
    auto GetSkinCount()      const { return Model.skins.size();      }
    auto GetAnimationCount() const { return Model.animations.size(); }

    auto GetDefaultSceneId() const { return Model.defaultScene; }
    // clang-format on
};

auto TinyGltfAccessorWrapper::GetByteStride(const TinyGltfBufferViewWrapper& View) const
{
    return Accessor.ByteStride(View.View);
}


struct TextureInitData : public ObjectBase<IObject>
{
    TextureInitData(IReferenceCounters* pRefCounters, TEXTURE_FORMAT _Format) :
        ObjectBase<IObject>{pRefCounters},
        Format{_Format}
    {}

    const TEXTURE_FORMAT Format;

    struct LevelData
    {
        std::vector<unsigned char> Data;

        TextureSubResData SubResData;

        Uint32 Width  = 0;
        Uint32 Height = 0;
    };
    std::vector<LevelData> Levels;

    RefCntAutoPtr<ITexture> pStagingTex;

    void GenerateMipLevels(Uint32 StartMipLevel)
    {
        VERIFY_EXPR(StartMipLevel > 0);
        VERIFY_EXPR(Format != TEX_FORMAT_UNKNOWN);

        const auto& FmtAttribs = GetTextureFormatAttribs(Format);

        // Note: this will work even when NumMipLevels is greater than
        //       finest mip resolution. All coarser mip levels will be 1x1.
        for (Uint32 mip = StartMipLevel; mip < Levels.size(); ++mip)
        {
            auto&       Level     = Levels[mip];
            const auto& FineLevel = Levels[mip - 1];

            // Note that we can't use GetMipLevelProperties here
            Level.Width  = AlignUp(std::max(FineLevel.Width / 2u, 1u), Uint32{FmtAttribs.BlockWidth});
            Level.Height = AlignUp(std::max(FineLevel.Height / 2u, 1u), Uint32{FmtAttribs.BlockHeight});

            Level.SubResData.Stride =
                Uint64{Level.Width} / Uint64{FmtAttribs.BlockWidth} * Uint64{FmtAttribs.ComponentSize} *
                (FmtAttribs.ComponentType != COMPONENT_TYPE_COMPRESSED ? Uint64{FmtAttribs.NumComponents} : 1);
            Level.SubResData.Stride = AlignUp(Level.SubResData.Stride, Uint64{4});

            const auto MipSize = Level.SubResData.Stride * Level.Height / Uint32{FmtAttribs.BlockHeight};

            Level.Data.resize(static_cast<size_t>(MipSize));
            Level.SubResData.pData = Level.Data.data();

            if (FmtAttribs.ComponentType != COMPONENT_TYPE_COMPRESSED)
            {
                ComputeMipLevel({Format, FineLevel.Width, FineLevel.Height,
                                 FineLevel.Data.data(), StaticCast<size_t>(FineLevel.SubResData.Stride),
                                 Level.Data.data(), StaticCast<size_t>(Level.SubResData.Stride)});
            }
            else
            {
                UNSUPPORTED("Mip generation for compressed formats is not currently implemented");
            }
        }
    }
};


TEXTURE_FORMAT GetModelImageDataTextureFormat(const Model::ImageData& Image)
{
    VERIFY_EXPR(Image.NumComponents != 0 && Image.ComponentSize != 0);

    if (Image.TexFormat != TEX_FORMAT_UNKNOWN)
        return Image.TexFormat;

    VERIFY(Image.ComponentSize == 1, "Only 8-bit image components are currently supported");
    switch (Image.NumComponents)
    {
        case 1: return TEX_FORMAT_R8_UNORM;
        case 2: return TEX_FORMAT_RG8_UNORM;
        case 3:
        case 4: return TEX_FORMAT_RGBA8_UNORM;
        default:
            UNEXPECTED("Unsupported number of color components in gltf image: ", Image.NumComponents);
            return TEX_FORMAT_UNKNOWN;
    }
}

RefCntAutoPtr<TextureInitData> PrepareGLTFTextureInitData(
    const Model::ImageData& _Image,
    float                   AlphaCutoff,
    Uint32                  NumMipLevels,
    int                     SizeAlignment = -1)
{
    if (_Image.pData == nullptr)
    {
        UNEXPECTED("Image data is null");
        return {};
    }

    if (_Image.Width <= 0 || _Image.Height <= 0 || _Image.NumComponents <= 0 || _Image.ComponentSize <= 0)
    {
        UNEXPECTED("Invalid image attributes. Size: ", _Image.Width, "x", _Image.Height, ", num components: ", _Image.NumComponents, ", component size: ", _Image.ComponentSize);
        return {};
    }

    const auto  TexFormat  = GetModelImageDataTextureFormat(_Image);
    const auto& FmtAttribs = GetTextureFormatAttribs(TexFormat);

    std::vector<Uint8> ExpandedPixels;
    Model::ImageData   AlignedImage;
    const auto&        GetAlignedImage = [&]() {
        AlignedImage        = _Image;
        AlignedImage.Width  = AlignUpNonPw2(_Image.Width, SizeAlignment);
        AlignedImage.Height = AlignUpNonPw2(_Image.Height, SizeAlignment);
        if (AlignedImage.Width == _Image.Width &&
            AlignedImage.Height == _Image.Height)
        {
            return _Image;
        }

        // Expand pixels to make sure there are no black gaps between allocations in the atlas
        // as they will result in texture filtering artifacts at boundaries.
        ExpandPixelsAttribs ExpandAttribs;
        ExpandAttribs.SrcWidth       = _Image.Width;
        ExpandAttribs.SrcHeight      = _Image.Height;
        ExpandAttribs.ComponentSize  = _Image.ComponentSize;
        ExpandAttribs.ComponentCount = _Image.NumComponents;
        ExpandAttribs.pSrcPixels     = _Image.pData;
        ExpandAttribs.SrcStride      = ExpandAttribs.SrcWidth * _Image.ComponentSize * _Image.NumComponents;
        ExpandAttribs.DstWidth       = AlignedImage.Width;
        ExpandAttribs.DstHeight      = AlignedImage.Height;
        ExpandAttribs.DstStride      = ExpandAttribs.DstWidth * AlignedImage.ComponentSize * AlignedImage.NumComponents;
        ExpandedPixels.resize(size_t{ExpandAttribs.DstHeight} * size_t{ExpandAttribs.DstStride});
        ExpandAttribs.pDstPixels = ExpandedPixels.data();
        ExpandPixels(ExpandAttribs);

        AlignedImage.pData    = ExpandedPixels.data();
        AlignedImage.DataSize = ExpandedPixels.size();

        return AlignedImage;
    };
    const auto& Image = SizeAlignment > 0 ? GetAlignedImage() : _Image;

    RefCntAutoPtr<TextureInitData> UpdateInfo{MakeNewRCObj<TextureInitData>()(FmtAttribs.Format)};

    auto& Levels = UpdateInfo->Levels;
    Levels.resize(NumMipLevels);

    auto& Level0  = Levels[0];
    Level0.Width  = Image.Width;
    Level0.Height = Image.Height;

    auto& Level0Stride{Level0.SubResData.Stride};
    Level0Stride = AlignUp(Uint64{Level0.Width} * FmtAttribs.ComponentSize * FmtAttribs.NumComponents, Uint64{4});
    Level0.Data.resize(static_cast<size_t>(Level0Stride * Image.Height));
    Level0.SubResData.pData = Level0.Data.data();

    const auto* pSrcData  = static_cast<const Uint8*>(Image.pData);
    const auto  SrcStride = Image.Width * Image.ComponentSize * Image.NumComponents;
    if (Image.ComponentSize == 1 && Image.NumComponents == 4 && FmtAttribs.NumComponents == 4 && AlphaCutoff > 0)
    {
        // Remap alpha channel using the following formula to improve mip maps:
        //
        //      A_new = max(A_old; 1/3 * A_old + 2/3 * CutoffThreshold)
        //
        // https://asawicki.info/articles/alpha_test.php5

        VERIFY_EXPR(AlphaCutoff > 0 && AlphaCutoff <= 1);
        AlphaCutoff *= 255.f;

        // Due to depressing performance of iterators in debug MSVC we have to use raw pointers here
        for (size_t row = 0; row < static_cast<size_t>(Image.Height); ++row)
        {
            const auto* src = pSrcData + row * static_cast<size_t>(SrcStride);
            auto*       dst = &Level0.Data[static_cast<size_t>(row * Level0Stride)];
            for (int i = 0; i < Image.Width; ++i)
            {
                dst[0] = src[0];
                dst[1] = src[1];
                dst[2] = src[2];
                dst[3] = std::max(src[3], static_cast<Uint8>(std::min(1.f / 3.f * src[3] + 2.f / 3.f * AlphaCutoff, 255.f)));

                src += 4;
                dst += 4;
            }
        }
    }
    else
    {
        CopyPixelsAttribs CopyAttribs;
        CopyAttribs.Width            = Image.Width;
        CopyAttribs.Height           = Image.Height;
        CopyAttribs.SrcComponentSize = Image.ComponentSize;
        CopyAttribs.pSrcPixels       = Image.pData;
        CopyAttribs.SrcStride        = SrcStride;
        CopyAttribs.SrcCompCount     = Image.NumComponents;
        CopyAttribs.pDstPixels       = Level0.Data.data();
        CopyAttribs.DstComponentSize = FmtAttribs.ComponentSize;
        CopyAttribs.DstStride        = static_cast<Uint32>(Level0Stride);
        CopyAttribs.DstCompCount     = FmtAttribs.NumComponents;
        CopyPixels(CopyAttribs);
    }

    UpdateInfo->GenerateMipLevels(1);

    return UpdateInfo;
}

} // namespace

Model::Model(const ModelCreateInfo& CI)
{
    DEV_CHECK_ERR(CI.IndexType == VT_UINT16 || CI.IndexType == VT_UINT32, "Invalid index type");
    DEV_CHECK_ERR(CI.NumVertexAttributes == 0 || CI.VertexAttributes != nullptr, "VertexAttributes must not be null when NumVertexAttributes > 0");
    DEV_CHECK_ERR(CI.NumTextureAttributes == 0 || CI.TextureAttributes != nullptr, "TextureAttributes must not be null when NumTextureAttributes > 0");
    DEV_CHECK_ERR(CI.NumTextureAttributes <= Material::MaxTextureAttribs, "Too many texture attributes (", CI.NumTextureAttributes, "). Maximum supported: ", Uint32{Material::MaxTextureAttribs});

    const auto* pSrcVertAttribs = CI.VertexAttributes != nullptr ? CI.VertexAttributes : DefaultVertexAttributes.data();
    const auto* pSrcTexAttribs  = CI.TextureAttributes != nullptr ? CI.TextureAttributes : DefaultTextureAttributes.data();
    NumVertexAttributes         = CI.VertexAttributes != nullptr ? CI.NumVertexAttributes : static_cast<Uint32>(DefaultVertexAttributes.size());
    NumTextureAttributes        = CI.TextureAttributes != nullptr ? CI.NumTextureAttributes : static_cast<Uint32>(DefaultTextureAttributes.size());

    auto&                RawAllocator = DefaultRawMemoryAllocator::GetAllocator();
    FixedLinearAllocator Allocator{RawAllocator};
    Allocator.AddSpace<VertexAttributeDesc>(NumVertexAttributes);
    Allocator.AddSpace<TextureAttributeDesc>(NumTextureAttributes);

    Uint32 MaxBufferId = 0;
    for (size_t i = 0; i < NumVertexAttributes; ++i)
    {
        const auto& Attrib = pSrcVertAttribs[i];

        DEV_CHECK_ERR(Attrib.Name != nullptr, "Vertex attribute name must not be null");
        DEV_CHECK_ERR(Attrib.ValueType != VT_UNDEFINED, "Undefined vertex attribute value type");
        DEV_CHECK_ERR(Attrib.NumComponents != 0, "The number of components must not be null");

        MaxBufferId = std::max<Uint32>(MaxBufferId, Attrib.BufferId);

        Allocator.AddSpaceForString(pSrcVertAttribs[i].Name);
    }
    VertexData.Strides.resize(size_t{MaxBufferId} + 1);

    for (size_t i = 0; i < NumTextureAttributes; ++i)
    {
        const auto& Attrib = pSrcTexAttribs[i];

        DEV_CHECK_ERR(Attrib.Name != nullptr, "Texture attribute name must not be null");
        Allocator.AddSpaceForString(Attrib.Name);
    }

    Allocator.Reserve();

    auto* pDstVertAttribs = Allocator.CopyArray<VertexAttributeDesc>(pSrcVertAttribs, NumVertexAttributes);
    auto* pDstTexAttribs  = Allocator.CopyArray<TextureAttributeDesc>(pSrcTexAttribs, NumTextureAttributes);
    for (size_t i = 0; i < NumVertexAttributes; ++i)
        pDstVertAttribs[i].Name = Allocator.CopyString(pSrcVertAttribs[i].Name);
    for (size_t i = 0; i < NumTextureAttributes; ++i)
        pDstTexAttribs[i].Name = Allocator.CopyString(pSrcTexAttribs[i].Name);

    for (size_t i = 0; i < NumVertexAttributes; ++i)
    {
        auto& Attrib = pDstVertAttribs[i];

        auto& ElementStride = VertexData.Strides[Attrib.BufferId];
        if (Attrib.RelativeOffset == VertexAttributeDesc{}.RelativeOffset)
        {
            Attrib.RelativeOffset = ElementStride;
        }
        else
        {
            DEV_CHECK_ERR(Attrib.RelativeOffset >= ElementStride, "Invalid offset: the attribute overlaps with previous attributes.");
        }

        ElementStride = Attrib.RelativeOffset + GetValueSize(Attrib.ValueType) * Attrib.NumComponents;
    }


    IndexData.IndexSize = CI.IndexType == VT_UINT32 ? 4 : 2;

    pAttributesData   = decltype(pAttributesData){Allocator.ReleaseOwnership(), RawAllocator};
    VertexAttributes  = pDstVertAttribs;
    TextureAttributes = pDstTexAttribs;
}

Model::Model(IRenderDevice*         pDevice,
             IDeviceContext*        pContext,
             const ModelCreateInfo& CI) :
    Model{CI}
{
    LoadFromFile(pDevice, pContext, CI);
}

Model::Model() noexcept
{
}

Model::~Model()
{
}

int Model::GetTextureAttributeIndex(const char* Name) const
{
    DEV_CHECK_ERR(Name != nullptr, "Name must not be null");
    for (size_t i = 0; i < NumTextureAttributes; ++i)
    {
        const auto& Attrib = GetTextureAttribute(i);
        if (SafeStrEqual(Attrib.Name, Name))
            return static_cast<int>(Attrib.Index);
    }
    return -1;
}

float Model::GetTextureAlphaCutoffValue(int TextureIndex) const
{
    const auto BaseTexAttribIdx = GetTextureAttributeIndex(BaseColorTextureName);
    if (BaseTexAttribIdx < 0)
        return 0;

    float AlphaCutoff = -1.f;
    for (const auto& Mat : Materials)
    {
        if (Mat.GetTextureId(BaseTexAttribIdx) != TextureIndex)
        {
            // The material does not use this texture as base color.
            continue;
        }

        if (Mat.Attribs.AlphaMode == Material::ALPHA_MODE_OPAQUE)
        {
            // The material is opaque, so alpha remapping mode does not matter.
            continue;
        }

        VERIFY_EXPR(Mat.Attribs.AlphaMode == Material::ALPHA_MODE_BLEND || Mat.Attribs.AlphaMode == Material::ALPHA_MODE_MASK);
        const float NewAlphaCutoff = Mat.Attribs.AlphaMode == Material::ALPHA_MODE_MASK ? Mat.Attribs.AlphaCutoff : 0;
        if (AlphaCutoff < 0)
        {
            AlphaCutoff = NewAlphaCutoff;
        }
        else if (AlphaCutoff != NewAlphaCutoff)
        {
            if (AlphaCutoff == 0 || NewAlphaCutoff == 0)
            {
                LOG_WARNING_MESSAGE("Texture ", TextureIndex,
                                    " is used in an alpha-cut material with threshold ", std::max(AlphaCutoff, NewAlphaCutoff),
                                    " as well as in an alpha-blend material."
                                    " Alpha remapping to improve mipmap generation will be disabled.");
                return 0;
            }
            else
            {
                LOG_WARNING_MESSAGE("Texture ", TextureIndex,
                                    " is used in alpha-cut materials with different cutoff thresholds (", AlphaCutoff, " and ", NewAlphaCutoff,
                                    "). Alpha remapping to improve mipmap generation will use ",
                                    std::min(AlphaCutoff, NewAlphaCutoff), '.');
                AlphaCutoff = std::min(AlphaCutoff, NewAlphaCutoff);
            }
        }
    }

    return std::max(AlphaCutoff, 0.f);
}

Uint32 Model::AddTexture(IRenderDevice*     pDevice,
                         TextureCacheType*  pTextureCache,
                         ResourceManager*   pResourceMgr,
                         const ImageData&   Image,
                         int                GltfSamplerId,
                         const std::string& CacheId)
{
    const auto NewTexId = static_cast<int>(Textures.size());

    TextureInfo TexInfo;
    if (!CacheId.empty())
    {
        if (pResourceMgr != nullptr)
        {
            TexInfo.pAtlasSuballocation = pResourceMgr->FindTextureAllocation(CacheId.c_str());
            if (TexInfo.pAtlasSuballocation)
            {
                // Note that the texture may appear in the cache after the call to LoadImageData because
                // it can be loaded by another thread
                VERIFY_EXPR(Image.Width == -1 || Image.Width == static_cast<int>(TexInfo.pAtlasSuballocation->GetSize().x));
                VERIFY_EXPR(Image.Height == -1 || Image.Height == static_cast<int>(TexInfo.pAtlasSuballocation->GetSize().y));
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
                    if ((Image.Width > 0 && Image.Height > 0) ||
                        (Image.FileFormat == IMAGE_FILE_FORMAT_DDS || Image.FileFormat == IMAGE_FILE_FORMAT_KTX))
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

    if (!TexInfo)
    {
        RefCntAutoPtr<ISampler> pSampler;
        if (GltfSamplerId == -1)
        {
            // No sampler specified, use default one
            pDevice->CreateSampler(Sam_LinearWrap, &pSampler);
        }
        else
        {
            pSampler = TextureSamplers[GltfSamplerId];
        }

        // Check if the texture is used in an alpha-cut material
        const float AlphaCutoff = GetTextureAlphaCutoffValue(NewTexId);

        if (Image.Width > 0 && Image.Height > 0)
        {
            if (pResourceMgr != nullptr)
            {
                const auto TexFormat = GetModelImageDataTextureFormat(Image);
                // No reference
                const TextureDesc AtlasDesc = pResourceMgr->GetAtlasDesc(TexFormat);

                // Load all mip levels.
                const auto AllocationAlignment = pResourceMgr->GetAllocationAlignment(TexFormat, Image.Width, Image.Height);
                auto       pInitData           = PrepareGLTFTextureInitData(Image, AlphaCutoff, AtlasDesc.MipLevels, AllocationAlignment);
                VERIFY_EXPR(pInitData->Format == TexFormat);

                // pInitData will be atomically set in the allocation before any other thread may be able to
                // access it.
                // Note that it is possible that more than one thread prepares pInitData for the same allocation.
                // It it also possible that multiple instances of the same allocation are created before the first
                // is added to the cache. This is all OK though.
                TexInfo.pAtlasSuballocation =
                    pResourceMgr->AllocateTextureSpace(TexFormat, Image.Width, Image.Height, CacheId.c_str(), pInitData);
                VERIFY_EXPR(TexInfo.pAtlasSuballocation->GetAtlas()->GetAtlasDesc().MipLevels == AtlasDesc.MipLevels);
                VERIFY_EXPR(TexInfo.pAtlasSuballocation->GetAlignment() == AllocationAlignment);
            }
            else
            {
                // Load only the lowest mip level; other mip levels will be generated on the GPU.
                auto pTexInitData = PrepareGLTFTextureInitData(Image, AlphaCutoff, 1);

                TextureDesc TexDesc;
                TexDesc.Name      = "GLTF Texture";
                TexDesc.Type      = RESOURCE_DIM_TEX_2D_ARRAY;
                TexDesc.Usage     = USAGE_DEFAULT;
                TexDesc.BindFlags = BIND_SHADER_RESOURCE;
                TexDesc.Width     = Image.Width;
                TexDesc.Height    = Image.Height;
                TexDesc.Format    = pTexInitData->Format;
                TexDesc.MipLevels = 0;
                TexDesc.MiscFlags = MISC_TEXTURE_FLAG_GENERATE_MIPS;

                pDevice->CreateTexture(TexDesc, nullptr, &TexInfo.pTexture);
                TexInfo.pTexture->GetDefaultView(TEXTURE_VIEW_SHADER_RESOURCE)->SetSampler(pSampler);

                TexInfo.pTexture->SetUserData(pTexInitData);
            }
        }
        else if (Image.FileFormat == IMAGE_FILE_FORMAT_DDS || Image.FileFormat == IMAGE_FILE_FORMAT_KTX)
        {
            RefCntAutoPtr<TextureInitData> pTexInitData{MakeNewRCObj<TextureInitData>()(TEX_FORMAT_UNKNOWN)};

            // Create the texture from raw bits
            RefCntAutoPtr<ITextureLoader> pTexLoader;

            TextureLoadInfo LoadInfo;
            LoadInfo.Name = "GLTF texture";
            if (pResourceMgr != nullptr)
            {
                LoadInfo.Usage          = USAGE_STAGING;
                LoadInfo.BindFlags      = BIND_NONE;
                LoadInfo.CPUAccessFlags = CPU_ACCESS_WRITE;
            }
            CreateTextureLoaderFromMemory(Image.pData, Image.DataSize, false /*MakeDataCopy*/, LoadInfo, &pTexLoader);
            if (pTexLoader)
            {
                if (pResourceMgr == nullptr)
                {
                    pTexLoader->CreateTexture(pDevice, &TexInfo.pTexture);
                    // Set empty init data to indicate that the texture needs to be transitioned to correct state
                    TexInfo.pTexture->SetUserData(pTexInitData);
                }
                else
                {
                    const auto& TexDesc = pTexLoader->GetTextureDesc();

                    // pTexInitData will be atomically set in the allocation before any other thread may be able to
                    // access it.
                    // Note that it is possible that more than one thread prepares pTexInitData for the same allocation.
                    // It it also possible that multiple instances of the same allocation are created before the first
                    // is added to the cache. This is all OK though.
                    TexInfo.pAtlasSuballocation = pResourceMgr->AllocateTextureSpace(TexDesc.Format, TexDesc.Width, TexDesc.Height, CacheId.c_str(), pTexInitData);

                    // NB: create staging texture to save work in the main thread when
                    //     this function is called from a worker thread
                    pTexLoader->CreateTexture(pDevice, &pTexInitData->pStagingTex);
                }
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

            RefCntAutoPtr<TextureInitData> pTexInitData{MakeNewRCObj<TextureInitData>()(TexDesc.Format)};

            pTexInitData->Levels.resize(1);
            auto& Level0  = pTexInitData->Levels[0];
            Level0.Width  = TexDesc.Width;
            Level0.Height = TexDesc.Height;

            auto& Level0Stride{Level0.SubResData.Stride};
            Level0Stride = Uint64{Level0.Width} * 4;
            Level0.Data.resize(static_cast<size_t>(Level0Stride * TexDesc.Height));
            Level0.SubResData.pData = Level0.Data.data();
            GenerateCheckerBoardPattern(TexDesc.Width, TexDesc.Height, TexDesc.Format, 4, 4, Level0.Data.data(), Level0Stride);

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
    for (auto& Mat : Materials)
    {
        InitMaterialTextureAddressingAttribs(Mat, static_cast<Uint32>(NewTexId));
    }
    return static_cast<Uint32>(NewTexId);
}

void Model::InitMaterialTextureAddressingAttribs(Material& Mat, Uint32 TextureIndex)
{
    const auto& TexInfo = Textures[TextureIndex];

    if (TexInfo.pAtlasSuballocation)
    {
        Mat.ProcessActiveTextureAttibs([&](Uint32 Idx, Material::TextureShaderAttribs& TexAttribs, int TexAttribTextureId) {
            if (TexAttribTextureId == static_cast<int>(TextureIndex))
            {
                TexAttribs.AtlasUVScaleAndBias = TexInfo.pAtlasSuballocation->GetUVScaleBias();
                TexAttribs.TextureSlice        = static_cast<float>(TexInfo.pAtlasSuballocation->GetSlice());
            }
            // Note: we need to process all attributes as the same texture may be referenced by multiple attributes
            return true;
        });
    }
}

void Model::LoadTextures(IRenderDevice*         pDevice,
                         const tinygltf::Model& gltf_model,
                         const std::string&     BaseDir,
                         TextureCacheType*      pTextureCache,
                         ResourceManager*       pResourceMgr)
{
    Textures.reserve(gltf_model.textures.size());
    for (const tinygltf::Texture& gltf_tex : gltf_model.textures)
    {
        const auto& gltf_image = gltf_model.images[gltf_tex.source];
        const auto  CacheId    = !gltf_image.uri.empty() ? FileSystem::SimplifyPath((BaseDir + gltf_image.uri).c_str()) : "";

        ImageData Image;
        Image.Width         = gltf_image.width;
        Image.Height        = gltf_image.height;
        Image.NumComponents = gltf_image.component;
        Image.ComponentSize = gltf_image.bits / 8;
        Image.FileFormat    = (gltf_image.width < 0 && gltf_image.height < 0) ? static_cast<IMAGE_FILE_FORMAT>(gltf_image.pixel_type) : IMAGE_FILE_FORMAT_UNKNOWN;
        Image.pData         = gltf_image.image.data();
        Image.DataSize      = gltf_image.image.size();

        AddTexture(pDevice, pTextureCache, pResourceMgr, Image, gltf_tex.sampler, CacheId);
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
            pTexture  = DstTexInfo.pAtlasSuballocation->GetAtlas()->Update(pDevice, pCtx);
            pInitData = ClassPtrCast<TextureInitData>(DstTexInfo.pAtlasSuballocation->GetUserData());
            // User data is only set when the allocation is created, so no other
            // thread can call SetUserData() in parallel.
            DstTexInfo.pAtlasSuballocation->SetUserData(nullptr);
        }
        else if (DstTexInfo.pTexture)
        {
            pTexture  = DstTexInfo.pTexture;
            pInitData = ClassPtrCast<TextureInitData>(pTexture->GetUserData());
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

        const auto& Levels      = pInitData->Levels;
        auto&       pStagingTex = pInitData->pStagingTex;
        const auto  DstSlice    = DstTexInfo.pAtlasSuballocation ? DstTexInfo.pAtlasSuballocation->GetSlice() : 0;
        const auto& TexDesc     = pTexture->GetDesc();

        if (!Levels.empty() || pStagingTex)
        {
            Uint32 DstX = 0;
            Uint32 DstY = 0;
            if (DstTexInfo.pAtlasSuballocation)
            {
                const auto& Origin = DstTexInfo.pAtlasSuballocation->GetOrigin();

                DstX = Origin.x;
                DstY = Origin.y;
            }

            if (!Levels.empty())
            {
                VERIFY(!pStagingTex, "Staging texture and levels are mutually exclusive");
                VERIFY_EXPR(Levels.size() == 1 || Levels.size() == TexDesc.MipLevels);
                for (Uint32 mip = 0; mip < Levels.size(); ++mip)
                {
                    const auto& Level = Levels[mip];

                    Box UpdateBox;
                    UpdateBox.MinX = DstX >> mip;
                    UpdateBox.MaxX = UpdateBox.MinX + Level.Width;
                    UpdateBox.MinY = DstY >> mip;
                    UpdateBox.MaxY = UpdateBox.MinY + Level.Height;
                    pCtx->UpdateTexture(pTexture, mip, DstSlice, UpdateBox, Level.SubResData, RESOURCE_STATE_TRANSITION_MODE_NONE, RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
                }

                if (Levels.size() == 1 && TexDesc.MipLevels > 1 && DstTexInfo.pTexture)
                {
                    // Only generate mips when texture atlas is not used
                    pCtx->GenerateMips(pTexture->GetDefaultView(TEXTURE_VIEW_SHADER_RESOURCE));
                }
            }
            else if (pStagingTex)
            {
                VERIFY(DstTexInfo.pAtlasSuballocation, "Staging texture is expected to be used with the atlas");
                const auto& FmtAttribs = GetTextureFormatAttribs(TexDesc.Format);
                const auto& SrcTexDesc = pStagingTex->GetDesc();

                auto SrcMips = std::min(SrcTexDesc.MipLevels, TexDesc.MipLevels);
                if (FmtAttribs.ComponentType == COMPONENT_TYPE_COMPRESSED)
                {
                    // Do not copy mip levels that are smaller than the block size
                    for (; SrcMips > 0; --SrcMips)
                    {
                        const auto MipProps = GetMipLevelProperties(SrcTexDesc, SrcMips - 1);
                        if (MipProps.LogicalWidth >= FmtAttribs.BlockWidth &&
                            MipProps.LogicalHeight >= FmtAttribs.BlockHeight)
                            break;
                    }
                }
                for (Uint32 mip = 0; mip < SrcMips; ++mip)
                {
                    CopyTextureAttribs CopyAttribs{pStagingTex, RESOURCE_STATE_TRANSITION_MODE_TRANSITION, pTexture, RESOURCE_STATE_TRANSITION_MODE_TRANSITION};
                    CopyAttribs.SrcMipLevel = mip;
                    CopyAttribs.DstMipLevel = mip;
                    CopyAttribs.DstSlice    = DstSlice;
                    CopyAttribs.DstX        = DstX >> mip;
                    CopyAttribs.DstY        = DstY >> mip;
                    pCtx->CopyTexture(CopyAttribs);
                }
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
            Barriers.emplace_back(StateTransitionDesc{pTexture, RESOURCE_STATE_UNKNOWN, RESOURCE_STATE_SHADER_RESOURCE, STATE_TRANSITION_FLAG_UPDATE_STATE});
        }
    }

    if (IndexData.pBuffer || IndexData.pAllocation)
    {
        IBuffer* pBuffer = IndexData.pAllocation ?
            IndexData.pAllocation->Update(pDevice, pCtx) :
            IndexData.pBuffer;

        if (pBuffer != nullptr)
        {
            RefCntAutoPtr<BufferInitData> pInitData;
            if (IndexData.pAllocation)
            {
                pInitData = RefCntAutoPtr<BufferInitData>{IndexData.pAllocation->GetUserData(), IID_BufferInitData};
                IndexData.pAllocation->SetUserData(nullptr);
            }
            else if (IndexData.pBuffer)
            {
                pInitData = RefCntAutoPtr<BufferInitData>{IndexData.pBuffer->GetUserData(), IID_BufferInitData};
                IndexData.pBuffer->SetUserData(nullptr);
            }

            if (pInitData)
            {
                const auto Offset = IndexData.pAllocation ? IndexData.pAllocation->GetOffset() : 0;

                VERIFY_EXPR(pInitData->Data.size() == 1);
                pCtx->UpdateBuffer(pBuffer, Offset, static_cast<Uint32>(pInitData->Data[0].size()), pInitData->Data[0].data(), RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
            }
            if (IndexData.pBuffer != nullptr)
            {
                VERIFY_EXPR(IndexData.pBuffer == pBuffer);
                Barriers.emplace_back(StateTransitionDesc{pBuffer, RESOURCE_STATE_UNKNOWN, RESOURCE_STATE_INDEX_BUFFER, STATE_TRANSITION_FLAG_UPDATE_STATE});
            }
        }
    }

    for (Uint32 BuffId = 0; BuffId < GetVertexBufferCount(); ++BuffId)
    {
        IBuffer* pBuffer = VertexData.pAllocation ?
            VertexData.pAllocation->Update(BuffId, pDevice, pCtx) :
            VertexData.Buffers[BuffId];
        if (pBuffer == nullptr)
            continue;

        RefCntAutoPtr<BufferInitData> pInitData;
        if (VertexData.pAllocation)
        {
            pInitData = RefCntAutoPtr<BufferInitData>{VertexData.pAllocation->GetUserData(), IID_BufferInitData};
            VERIFY_EXPR(!pInitData || pInitData->Data.size() == GetVertexBufferCount());
        }
        else if (VertexData.Buffers[BuffId])
        {
            pInitData = RefCntAutoPtr<BufferInitData>{VertexData.Buffers[BuffId]->GetUserData(), IID_BufferInitData};
            VertexData.Buffers[BuffId]->SetUserData(nullptr);
            VERIFY_EXPR(!pInitData || pInitData->Data.size() == 1);
        }

        if (pInitData)
        {
            const auto Offset = VertexData.pAllocation ?
                VertexData.pAllocation->GetStartVertex() * VertexData.Strides[BuffId] :
                0;

            const auto& Data = VertexData.pAllocation ? pInitData->Data[BuffId] : pInitData->Data[0];
            if (!Data.empty())
            {
                pCtx->UpdateBuffer(pBuffer, Offset, static_cast<Uint32>(Data.size()), Data.data(), RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
            }
        }

        if (!VertexData.Buffers.empty() && VertexData.Buffers[BuffId])
        {
            VERIFY_EXPR(VertexData.Buffers[BuffId] == pBuffer);
            if (pBuffer->GetDesc().BindFlags & BIND_VERTEX_BUFFER)
                Barriers.emplace_back(StateTransitionDesc{pBuffer, RESOURCE_STATE_UNKNOWN, RESOURCE_STATE_VERTEX_BUFFER, STATE_TRANSITION_FLAG_UPDATE_STATE});
            else if (pBuffer->GetDesc().BindFlags & BIND_SHADER_RESOURCE)
                Barriers.emplace_back(StateTransitionDesc{pBuffer, RESOURCE_STATE_UNKNOWN, RESOURCE_STATE_SHADER_RESOURCE, STATE_TRANSITION_FLAG_UPDATE_STATE});
        }
    }
    if (VertexData.pAllocation)
        VertexData.pAllocation->SetUserData(nullptr);

    if (!Barriers.empty())
        pCtx->TransitionResourceStates(static_cast<Uint32>(Barriers.size()), Barriers.data());

    GPUDataInitialized.store(true);
}

void Model::LoadTextureSamplers(IRenderDevice* pDevice, const tinygltf::Model& gltf_model)
{
    for (const tinygltf::Sampler& smpl : gltf_model.samplers)
    {
        SamplerDesc SamDesc;
        SamDesc.MagFilter = ModelBuilder::GetFilterType(smpl.magFilter).first;
        auto MinMipFilter = ModelBuilder::GetFilterType(smpl.minFilter);
        SamDesc.MinFilter = MinMipFilter.first;
        SamDesc.MipFilter = MinMipFilter.second;
        SamDesc.AddressU  = ModelBuilder::GetAddressMode(smpl.wrapS);
        SamDesc.AddressV  = ModelBuilder::GetAddressMode(smpl.wrapT);
        SamDesc.AddressW  = SamDesc.AddressV;
        RefCntAutoPtr<ISampler> pSampler;
        pDevice->CreateSampler(SamDesc, &pSampler);
        TextureSamplers.push_back(std::move(pSampler));
    }
}

static void ReadKhrTextureTransform(const Model&                  model,
                                    const tinygltf::ExtensionMap& Extensions,
                                    MaterialBuilder&              Mat,
                                    const char*                   TextureName)
{
    auto ext_it = Extensions.find("KHR_texture_transform");
    if (ext_it == Extensions.end())
        return;

    const int TexAttribIdx = model.GetTextureAttributeIndex(TextureName);
    if (TexAttribIdx < 0)
        return;

    Material::TextureShaderAttribs& TexAttribs{Mat.GetTextureAttrib(TexAttribIdx)};
    const tinygltf::Value&          ext_value = ext_it->second;
    if (ext_value.Has("scale"))
    {
        const tinygltf::Value& scale = ext_value.Get("scale");
        if (scale.IsArray() && scale.ArrayLen() >= 2)
        {
            const float UScale = static_cast<float>(scale.Get(0).Get<double>());
            const float VScale = static_cast<float>(scale.Get(1).Get<double>());

            TexAttribs.UVScaleAndRotation = float2x2::Scale(UScale, VScale);
        }
        else
        {
            LOG_ERROR_MESSAGE("Texture scale value is expected to be a 2-element array. Refer to KHR_texture_transform specification.");
        }
    }

    if (ext_value.Has("rotation"))
    {
        const float rotation = static_cast<float>(ext_value.Get("rotation").Get<double>());
        // UV coordinate rotation is defined counter-clockwise, which is clockwise rotation of the image.
        TexAttribs.UVScaleAndRotation *= float2x2::Rotation(-rotation);
    }

    if (ext_value.Has("offset"))
    {
        const tinygltf::Value& offset = ext_value.Get("offset");
        if (offset.IsArray() && offset.ArrayLen() >= 2)
        {
            TexAttribs.UBias = static_cast<float>(offset.Get(0).Get<double>());
            TexAttribs.VBias = static_cast<float>(offset.Get(1).Get<double>());
        }
        else
        {
            LOG_ERROR_MESSAGE("Texture offset value is expected to be a 2-element array. Refer to KHR_texture_transform specification.");
        }
    }

    if (ext_value.Has("texCoord"))
    {
        const tinygltf::Value& texCoord = ext_value.Get("texCoord");
        TexAttribs.UVSelector           = static_cast<float>(texCoord.Get<int>());
    }
}

static tinygltf::ExtensionMap ReadExtensions(const tinygltf::Value& ExtVal)
{
    tinygltf::ExtensionMap Extensions;
    for (const std::string& Key : ExtVal.Keys())
    {
        Extensions.emplace(Key, ExtVal.Get(Key));
    }
    return Extensions;
}

static void LoadExtensionTexture(const Model& model, const tinygltf::Value& Ext, MaterialBuilder& Mat, const char* Name)
{
    if (!Ext.Has(Name))
        return;

    const tinygltf::Value& TexInfo = Ext.Get(Name);

    const auto TexAttribIdx = model.GetTextureAttributeIndex(Name);
    if (TexAttribIdx < 0)
        return;

    int TexId = -1;
    if (TexInfo.Has("index")) // Required
    {
        TexId = TexInfo.Get("index").Get<int>();
    }
    else
    {
        LOG_ERROR_MESSAGE("Required value 'index' is not specified for texture '", Name, "'.");
    }

    float UVSelector = 0;
    if (TexInfo.Has("texCoord")) // Optional
    {
        UVSelector = static_cast<float>(TexInfo.Get("texCoord").Get<int>());
    }

    Mat.SetTextureId(TexAttribIdx, TexId);
    Mat.GetTextureAttrib(TexAttribIdx).UVSelector = UVSelector;

    if (TexInfo.Has("extensions"))
    {
        const tinygltf::ExtensionMap TexExt = ReadExtensions(TexInfo.Get("extensions"));
        ReadKhrTextureTransform(model, TexExt, Mat, Name);
    }
}

static void LoadExtensionParameter(const tinygltf::Value& Ext, const char* Name, float& Val)
{
    if (!Ext.Has(Name))
        return;

    const tinygltf::Value& Param = Ext.Get(Name);
    if (!Param.IsNumber())
        return;

    Val = static_cast<float>(Param.Get<double>());
}


template <typename VectorType>
static void LoadExtensionParameter(const tinygltf::Value& Ext, const char* Name, VectorType& Val)
{
    if (!Ext.Has(Name))
        return;

    const tinygltf::Value& Param = Ext.Get(Name);
    if (Param.IsArray())
    {
        for (size_t i = 0; i < std::min(VectorType::GetComponentCount(), Param.ArrayLen()); ++i)
        {
            Val[i] = static_cast<typename VectorType::ValueType>(Param.Get(static_cast<int>(i)).Get<double>());
        }
    }
    else if (Param.IsNumber())
    {
        Val = VectorType{static_cast<typename VectorType::ValueType>(Param.Get<double>())};
    }
}

void Model::LoadMaterials(const tinygltf::Model& gltf_model, const ModelCreateInfo::MaterialLoadCallbackType& MaterialLoadCallback)
{
    Materials.reserve(gltf_model.materials.size());
    for (const tinygltf::Material& gltf_mat : gltf_model.materials)
    {
        Material        Mat;
        MaterialBuilder MatBuilder{Mat};

        auto FindTexture = [&MatBuilder](const TextureAttributeDesc& Attrib, const tinygltf::ParameterMap& Mapping) {
            auto tex_it = Mapping.find(Attrib.Name);
            if (tex_it == Mapping.end())
                return false;

            MatBuilder.SetTextureId(Attrib.Index, tex_it->second.TextureIndex());
            MatBuilder.GetTextureAttrib(Attrib.Index).UVSelector = static_cast<float>(tex_it->second.TextureTexCoord());

            return true;
        };

        for (size_t i = 0; i < NumTextureAttributes; ++i)
        {
            const auto& Attrib = GetTextureAttribute(i);
            // Search in values
            auto TexFound = FindTexture(Attrib, gltf_mat.values);

            // Search in additional values
            if (!TexFound)
                TexFound = FindTexture(Attrib, gltf_mat.additionalValues);
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

        ReadKhrTextureTransform(*this, gltf_mat.pbrMetallicRoughness.baseColorTexture.extensions, MatBuilder, BaseColorTextureName);
        ReadKhrTextureTransform(*this, gltf_mat.pbrMetallicRoughness.metallicRoughnessTexture.extensions, MatBuilder, MetallicRoughnessTextureName);
        ReadKhrTextureTransform(*this, gltf_mat.normalTexture.extensions, MatBuilder, NormalTextureName);
        ReadKhrTextureTransform(*this, gltf_mat.emissiveTexture.extensions, MatBuilder, EmissiveTextureName);
        ReadKhrTextureTransform(*this, gltf_mat.occlusionTexture.extensions, MatBuilder, OcclusionTextureName);

        // Extensions

        // https://github.com/KhronosGroup/glTF/tree/main/extensions/2.0/Khronos/KHR_materials_unlit
        {
            auto ext_it = gltf_mat.extensions.find("KHR_materials_unlit");
            if (ext_it != gltf_mat.extensions.end())
            {
                Mat.Attribs.Workflow = Material::PBR_WORKFLOW_UNLIT;
            }
        }

        {
            auto ext_it = gltf_mat.extensions.find("KHR_materials_pbrSpecularGlossiness");
            if (ext_it != gltf_mat.extensions.end())
            {
                Mat.Attribs.Workflow = Material::PBR_WORKFLOW_SPEC_GLOSS;

                const auto& SpecGlossExt = ext_it->second;
                LoadExtensionTexture(*this, SpecGlossExt, MatBuilder, SpecularGlossinessTextureName);
                LoadExtensionTexture(*this, SpecGlossExt, MatBuilder, DiffuseTextureName);
                LoadExtensionParameter(SpecGlossExt, "diffuseFactor", Mat.Attribs.BaseColorFactor);
                LoadExtensionParameter(SpecGlossExt, "specularFactor", Mat.Attribs.SpecularFactor);
            }
        }

        // https://github.com/KhronosGroup/glTF/tree/main/extensions/2.0/Khronos/KHR_materials_clearcoat
        if (Mat.Attribs.Workflow == Material::PBR_WORKFLOW_METALL_ROUGH) // Clearcoat is incompatible with spec-gloss workflow and unlit materials
        {
            auto ext_it = gltf_mat.extensions.find("KHR_materials_clearcoat");
            if (ext_it != gltf_mat.extensions.end())
            {
                const auto& ClearcoatExt = ext_it->second;
                LoadExtensionTexture(*this, ClearcoatExt, MatBuilder, ClearcoatTextureName);
                LoadExtensionTexture(*this, ClearcoatExt, MatBuilder, ClearcoatRoughnessTextureName);
                LoadExtensionTexture(*this, ClearcoatExt, MatBuilder, ClearcoatNormalTextureName);
                LoadExtensionParameter(ClearcoatExt, "clearcoatFactor", Mat.Attribs.ClearcoatFactor);
                LoadExtensionParameter(ClearcoatExt, "clearcoatRoughnessFactor", Mat.Attribs.ClearcoatRoughnessFactor);

                // The spec says that clear coat factor is zero, the whole clear coat layer is disabled.
                Mat.HasClearcoat = Mat.Attribs.ClearcoatFactor != 0;
            }
        }

        // https://github.com/KhronosGroup/glTF/tree/main/extensions/2.0/Khronos/KHR_materials_sheen
        if (Mat.Attribs.Workflow == Material::PBR_WORKFLOW_METALL_ROUGH) // Sheen is incompatible with spec-gloss workflow and unlit materials
        {
            auto ext_it = gltf_mat.extensions.find("KHR_materials_sheen");
            if (ext_it != gltf_mat.extensions.end())
            {
                Mat.Sheen = std::make_unique<Material::SheenShaderAttribs>();

                const auto& SheenExt = ext_it->second;
                LoadExtensionTexture(*this, SheenExt, MatBuilder, SheenColorTextureName);
                LoadExtensionTexture(*this, SheenExt, MatBuilder, SheenRoughnessTextureName);
                LoadExtensionParameter(SheenExt, "sheenColorFactor", Mat.Sheen->ColorFactor);
                LoadExtensionParameter(SheenExt, "sheenRoughnessFactor", Mat.Sheen->RoughnessFactor);
            }
        }

        // https://github.com/KhronosGroup/glTF/tree/main/extensions/2.0/Khronos/KHR_materials_anisotropy
        if (Mat.Attribs.Workflow == Material::PBR_WORKFLOW_METALL_ROUGH) // Anisotropy is incompatible with spec-gloss workflow and unlit materials
        {
            auto ext_it = gltf_mat.extensions.find("KHR_materials_anisotropy");
            if (ext_it != gltf_mat.extensions.end())
            {
                Mat.Anisotropy = std::make_unique<Material::AnisotropyShaderAttribs>();

                const auto& AnisoExt = ext_it->second;
                LoadExtensionTexture(*this, AnisoExt, MatBuilder, AnisotropyTextureName);
                LoadExtensionParameter(AnisoExt, "anisotropyRotation", Mat.Anisotropy->Rotation);
                LoadExtensionParameter(AnisoExt, "anisotropyStrength", Mat.Anisotropy->Strength);
            }
        }

        // https://github.com/KhronosGroup/glTF/tree/main/extensions/2.0/Khronos/KHR_materials_iridescence
        if (Mat.Attribs.Workflow == Material::PBR_WORKFLOW_METALL_ROUGH) // Iridescence is incompatible with spec-gloss workflow and unlit materials
        {
            auto ext_it = gltf_mat.extensions.find("KHR_materials_iridescence");
            if (ext_it != gltf_mat.extensions.end())
            {
                Mat.Iridescence = std::make_unique<Material::IridescenceShaderAttribs>();

                const auto& IridExt = ext_it->second;
                LoadExtensionTexture(*this, IridExt, MatBuilder, IridescenceTextureName);
                LoadExtensionTexture(*this, IridExt, MatBuilder, IridescenceThicknessTextureName);
                LoadExtensionParameter(IridExt, "iridescenceFactor", Mat.Iridescence->Factor);
                LoadExtensionParameter(IridExt, "iridescenceIor", Mat.Iridescence->IOR);
                LoadExtensionParameter(IridExt, "iridescenceThicknessMinimum", Mat.Iridescence->ThicknessMinimum);
                LoadExtensionParameter(IridExt, "iridescenceThicknessMaximum", Mat.Iridescence->ThicknessMaximum);
            }
        }

        // https://github.com/KhronosGroup/glTF/tree/main/extensions/2.0/Khronos/KHR_materials_transmission
        if (Mat.Attribs.Workflow == Material::PBR_WORKFLOW_METALL_ROUGH) // Transmission is incompatible with spec-gloss workflow and unlit materials
        {
            auto ext_it = gltf_mat.extensions.find("KHR_materials_transmission");
            if (ext_it != gltf_mat.extensions.end())
            {
                Mat.Attribs.AlphaMode = Material::ALPHA_MODE_BLEND;

                Mat.Transmission = std::make_unique<Material::TransmissionShaderAttribs>();

                const auto& TransExt = ext_it->second;
                LoadExtensionTexture(*this, TransExt, MatBuilder, TransmissionTextureName);
                LoadExtensionParameter(TransExt, "transmissionFactor", Mat.Transmission->Factor);
            }
        }

        // https://github.com/KhronosGroup/glTF/tree/main/extensions/2.0/Khronos/KHR_materials_volume
        if (Mat.Attribs.Workflow == Material::PBR_WORKFLOW_METALL_ROUGH) // Transmission is incompatible with spec-gloss workflow and unlit materials
        {
            auto ext_it = gltf_mat.extensions.find("KHR_materials_volume");
            if (ext_it != gltf_mat.extensions.end())
            {
                Mat.Volume = std::make_unique<Material::VolumeShaderAttribs>();

                const auto& VolExt = ext_it->second;
                LoadExtensionTexture(*this, VolExt, MatBuilder, ThicknessTextureName);
                LoadExtensionParameter(VolExt, "thicknessFactor", Mat.Volume->ThicknessFactor);
                LoadExtensionParameter(VolExt, "attenuationDistance", Mat.Volume->AttenuationDistance);
                LoadExtensionParameter(VolExt, "attenuationColor", Mat.Volume->AttenuationColor);
            }
        }

        // https://github.com/KhronosGroup/glTF/tree/main/extensions/2.0/Khronos/KHR_materials_emissive_strength
        if (Mat.Attribs.Workflow != Material::PBR_WORKFLOW_UNLIT) // Incompatible with unlit materials
        {
            auto ext_it = gltf_mat.extensions.find("KHR_materials_emissive_strength");
            if (ext_it != gltf_mat.extensions.end())
            {
                const auto& EmissiveStrengthExt = ext_it->second;
                float       EmissiveStrength    = 1.f;
                LoadExtensionParameter(EmissiveStrengthExt, "emissiveStrength", EmissiveStrength);
                Mat.Attribs.EmissiveFactor *= EmissiveStrength;
            }
        }

        MatBuilder.Finalize();

        if (MaterialLoadCallback != nullptr)
            MaterialLoadCallback(&gltf_mat, Mat);

        Materials.push_back(std::move(Mat));
    }

    if (Materials.empty())
    {
        // Push a default material for meshes with no material assigned
        Materials.push_back(Material{});
    }
}

namespace Callbacks
{

namespace
{

struct LoaderData
{
    TextureCacheType* const pTextureCache;
    ResourceManager* const  pResourceMgr;

    std::vector<RefCntAutoPtr<IObject>> TexturesHold;

    std::string BaseDir;

    ModelCreateInfo::FileExistsCallbackType    FileExists    = nullptr;
    ModelCreateInfo::ReadWholeFileCallbackType ReadWholeFile = nullptr;
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

    auto* pLoaderData = static_cast<LoaderData*>(user_data);
    if (pLoaderData != nullptr)
    {
        const auto CacheId = !gltf_image->uri.empty() ? FileSystem::SimplifyPath((pLoaderData->BaseDir + gltf_image->uri).c_str()) : "";

        if (pLoaderData->pResourceMgr != nullptr)
        {
            if (auto pAllocation = pLoaderData->pResourceMgr->FindTextureAllocation(CacheId.c_str()))
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
        auto pImageData = DataBlobImpl::Create(size);
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
        size_t DstRowSize      = static_cast<size_t>(gltf_image->width) * gltf_image->component * (gltf_image->bits / 8);
        gltf_image->image.resize(static_cast<size_t>(gltf_image->height) * DstRowSize);

        CopyPixelsAttribs CopyAttribs;
        CopyAttribs.Width            = ImgDesc.Width;
        CopyAttribs.Height           = ImgDesc.Height;
        CopyAttribs.SrcComponentSize = gltf_image->bits / 8;
        CopyAttribs.pSrcPixels       = pImage->GetData()->GetDataPtr();
        CopyAttribs.SrcStride        = ImgDesc.RowStride;
        CopyAttribs.SrcCompCount     = ImgDesc.NumComponents;
        CopyAttribs.pDstPixels       = gltf_image->image.data();
        CopyAttribs.DstComponentSize = gltf_image->bits / 8;
        CopyAttribs.DstStride        = static_cast<Uint32>(DstRowSize);
        CopyAttribs.DstCompCount     = gltf_image->component;
        if (CopyAttribs.SrcCompCount < 4)
        {
            // Always set alpha to 1
            CopyAttribs.Swizzle.A = TEXTURE_COMPONENT_SWIZZLE_ONE;
            if (CopyAttribs.SrcCompCount == 1)
            {
                // Expand R to RGB
                CopyAttribs.Swizzle.R = TEXTURE_COMPONENT_SWIZZLE_R;
                CopyAttribs.Swizzle.G = TEXTURE_COMPONENT_SWIZZLE_R;
                CopyAttribs.Swizzle.B = TEXTURE_COMPONENT_SWIZZLE_R;
            }
            else if (CopyAttribs.SrcCompCount == 2)
            {
                // RG -> RG01
                CopyAttribs.Swizzle.B = TEXTURE_COMPONENT_SWIZZLE_ZERO;
            }
            else
            {
                VERIFY(CopyAttribs.SrcCompCount == 3, "Unexpected number of components");
            }
        }
        CopyPixels(CopyAttribs);
    }

    return true;
}

bool FileExists(const std::string& abs_filename, void* user_data)
{
    // FileSystem::FileExists() is a pretty slow function.
    // Try to find the file in the cache first to avoid calling it.
    if (auto* pLoaderData = static_cast<LoaderData*>(user_data))
    {
        const auto CacheId = FileSystem::SimplifyPath(abs_filename.c_str());
        if (pLoaderData->pResourceMgr != nullptr)
        {
            if (pLoaderData->pResourceMgr->FindTextureAllocation(CacheId.c_str()) != nullptr)
                return true;
        }
        else if (pLoaderData->pTextureCache != nullptr)
        {
            std::lock_guard<std::mutex> Lock{pLoaderData->pTextureCache->TexturesMtx};

            auto it = pLoaderData->pTextureCache->Textures.find(CacheId.c_str());
            if (it != pLoaderData->pTextureCache->Textures.end())
                return true;
        }

        if (pLoaderData->FileExists)
            return pLoaderData->FileExists(abs_filename.c_str());
    }

    return FileSystem::FileExists(abs_filename.c_str());
}

bool ReadWholeFile(std::vector<unsigned char>* out,
                   std::string*                err,
                   const std::string&          filepath,
                   void*                       user_data)
{
    VERIFY_EXPR(out != nullptr);
    VERIFY_EXPR(err != nullptr);

    // Try to find the file in the texture cache to avoid reading it
    if (auto* pLoaderData = static_cast<LoaderData*>(user_data))
    {
        const auto CacheId = FileSystem::SimplifyPath(filepath.c_str());
        if (pLoaderData->pResourceMgr != nullptr)
        {
            if (auto pAllocation = pLoaderData->pResourceMgr->FindTextureAllocation(CacheId.c_str()))
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

            auto it = pLoaderData->pTextureCache->Textures.find(CacheId.c_str());
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

        if (pLoaderData->ReadWholeFile)
            return pLoaderData->ReadWholeFile(filepath.c_str(), *out, *err);
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

void Model::LoadFromFile(IRenderDevice*         pDevice,
                         IDeviceContext*        pContext,
                         const ModelCreateInfo& CI)
{
    if (CI.FileName == nullptr || *CI.FileName == 0)
        LOG_ERROR_AND_THROW("File path must not be empty");

    auto* const pTextureCache = CI.pTextureCache;
    auto* const pResourceMgr  = CI.pResourceManager;
    if (CI.pTextureCache != nullptr && pResourceMgr != nullptr)
        LOG_WARNING_MESSAGE("Texture cache is ignored when resource manager is used");

    Callbacks::LoaderData LoaderData{pTextureCache, pResourceMgr, {}, ""};

    const std::string filename{CI.FileName};
    if (filename.find_last_of("/\\") != std::string::npos)
        LoaderData.BaseDir = filename.substr(0, filename.find_last_of("/\\"));
    LoaderData.BaseDir += '/';

    LoaderData.FileExists    = CI.FileExistsCallback;
    LoaderData.ReadWholeFile = CI.ReadWholeFileCallback;

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

    // Load materials first as the LoadTextures() function needs them to determine the alpha-cut value.
    LoadMaterials(gltf_model, CI.MaterialLoadCallback);
    LoadTextureSamplers(pDevice, gltf_model);
    LoadTextures(pDevice, gltf_model, LoaderData.BaseDir, pTextureCache, pResourceMgr);

    ModelBuilder Builder{CI, *this};
    Builder.Execute(TinyGltfModelWrapper{gltf_model}, CI.SceneId, pDevice);

    if (pContext != nullptr)
    {
        PrepareGPUResources(pDevice, pContext);
    }

    Extensions = gltf_model.extensionsUsed;
}

BoundBox Model::ComputeBoundingBox(Uint32 SceneIndex, const ModelTransforms& Transforms) const
{
    BoundBox ModelAABB;

    if (CompatibleWithTransforms(Transforms))
    {
        VERIFY_EXPR(SceneIndex < Scenes.size());
        const auto& scene = Scenes[SceneIndex];

        ModelAABB.Min = float3{+FLT_MAX, +FLT_MAX, +FLT_MAX};
        ModelAABB.Max = float3{-FLT_MAX, -FLT_MAX, -FLT_MAX};

        for (const auto* pN : scene.LinearNodes)
        {
            VERIFY_EXPR(pN != nullptr);
            if (pN->pMesh != nullptr && pN->pMesh->IsValidBB())
            {
                const auto& GlobalMatrix = Transforms.NodeGlobalMatrices[pN->Index];
                const auto  NodeAABB     = pN->pMesh->BB.Transform(GlobalMatrix);

                ModelAABB.Min = std::min(ModelAABB.Min, NodeAABB.Min);
                ModelAABB.Max = std::max(ModelAABB.Max, NodeAABB.Max);
            }
        }
    }
    else
    {
        UNEXPECTED("Incompatible transforms. Please use the ComputeTransforms() method first.");
    }

    return ModelAABB;
}

static void UpdateNodeGlobalTransform(const Node& node, const float4x4& ParentMatrix, ModelTransforms& Transforms)
{
    const auto& LocalMat  = Transforms.NodeLocalMatrices[node.Index];
    auto&       GlobalMat = Transforms.NodeGlobalMatrices[node.Index];
    GlobalMat             = LocalMat * ParentMatrix;
    for (auto* pChild : node.Children)
    {
        UpdateNodeGlobalTransform(*pChild, GlobalMat, Transforms);
    }
}

void Model::ComputeTransforms(Uint32           SceneIndex,
                              ModelTransforms& Transforms,
                              const float4x4&  RootTransform,
                              Int32            AnimationIndex,
                              float            Time) const
{
    if (SceneIndex >= Scenes.size())
    {
        DEV_ERROR("Invalid scene index ", SceneIndex);
        return;
    }
    const auto& scene = Scenes[SceneIndex];

    // Note that the matrices are indexed by the global node index,
    // not the linear node index in the scene.
    Transforms.NodeGlobalMatrices.resize(Nodes.size());
    Transforms.NodeLocalMatrices.resize(Nodes.size());

    // Update node animation
    if (AnimationIndex >= 0)
    {
        Transforms.Skins.resize(SkinTransformsCount);
        UpdateAnimation(SceneIndex, AnimationIndex, Time, Transforms);
    }
    else
    {
        Transforms.Skins.clear();
        for (auto* pNode : scene.LinearNodes)
        {
            VERIFY_EXPR(pNode != nullptr);
            Transforms.NodeLocalMatrices[pNode->Index] = pNode->ComputeLocalTransform();
        }
    }

    // Compute global transforms
    for (auto* pRoot : scene.RootNodes)
        UpdateNodeGlobalTransform(*pRoot, RootTransform, Transforms);

    // Update join matrices
    if (!Transforms.Skins.empty())
    {
        for (const auto* pNode : scene.LinearNodes)
        {
            VERIFY_EXPR(pNode != nullptr);
            auto* pMesh = pNode->pMesh;
            auto* pSkin = pNode->pSkin;
            if (pMesh == nullptr || pSkin == nullptr)
                continue;

            const auto& NodeGlobalMat = Transforms.NodeGlobalMatrices[pNode->Index];
            VERIFY(pNode->SkinTransformsIndex < static_cast<int>(SkinTransformsCount),
                   "Skin transform index (", pNode->SkinTransformsIndex, ") exceeds the skin transform count in this mesh (", SkinTransformsCount,
                   "). This appears to be a bug.");
            auto& JointMatrices = Transforms.Skins[pNode->SkinTransformsIndex].JointMatrices;
            if (JointMatrices.size() != pSkin->Joints.size())
                JointMatrices.resize(pSkin->Joints.size());

            const auto InverseTransform = NodeGlobalMat.Inverse();
            for (size_t i = 0; i < pSkin->Joints.size(); i++)
            {
                const auto* JointNode          = pSkin->Joints[i];
                const auto& JointNodeGlobalMat = Transforms.NodeGlobalMatrices[JointNode->Index];
                JointMatrices[i] =
                    pSkin->InverseBindMatrices[i] * JointNodeGlobalMat * InverseTransform;
            }
        }
    }
}

bool Model::CompatibleWithTransforms(const ModelTransforms& Transforms) const
{
    return (Transforms.NodeLocalMatrices.size() == Nodes.size() &&
            Transforms.NodeGlobalMatrices.size() == Nodes.size());
}

void Model::UpdateAnimation(Uint32 SceneIndex, Uint32 AnimationIndex, float time, ModelTransforms& Transforms) const
{
    if (AnimationIndex >= Animations.size())
    {
        LOG_WARNING_MESSAGE("No animation with index ", AnimationIndex);
        return;
    }

    VERIFY_EXPR(SceneIndex < Scenes.size());
    const auto& animation = Animations[AnimationIndex];

    time = clamp(time, animation.Start, animation.End);

    const auto& scene = Scenes[SceneIndex];
    if (Transforms.NodeAnimations.size() != scene.LinearNodes.size())
        Transforms.NodeAnimations.resize(scene.LinearNodes.size());
    VERIFY_EXPR(Transforms.NodeAnimations.size() == Transforms.NodeLocalMatrices.size());

    for (const auto* pN : scene.LinearNodes)
    {
        VERIFY_EXPR(pN != nullptr);
        auto& A = Transforms.NodeAnimations[pN->Index];

        // NB: not each component has to be animated (e.g. 'Fox' test model)
        A.Translation = pN->Translation;
        A.Rotation    = pN->Rotation;
        A.Scale       = pN->Scale;
    }

    for (auto& channel : animation.Channels)
    {
        const auto& sampler = animation.Samplers[channel.SamplerIndex];
        if (sampler.Inputs.size() > sampler.OutputsVec4.size())
        {
            continue;
        }

        auto& NodeAnim = Transforms.NodeAnimations[channel.pNode->Index];

        // Get the keyframe index.
        // Note that different channels may have different time ranges.
        auto Idx = sampler.FindKeyFrame(time);

        // STEP: The animated values remain constant to the output of the first keyframe, until the next keyframe.
        //       The number of output elements **MUST** equal the number of input elements.
        float u = 0;

        // LINEAR: The animated values are linearly interpolated between keyframes.
        //         The number of output elements **MUST** equal the number of input elements.
        if (sampler.Interpolation == AnimationSampler::INTERPOLATION_TYPE::LINEAR)
        {
            if (sampler.Inputs.size() < 2)
                continue;

            Idx = std::min(Idx, sampler.Inputs.size() - 2);
            u   = (time - sampler.Inputs[Idx]) / (sampler.Inputs[Idx + 1] - sampler.Inputs[Idx]);
        }

        // CUBICSPLINE: The animation's interpolation is computed using a cubic spline with specified tangents.
        //              The number of output elements **MUST** equal three times the number of input elements.
        //              For each input element, the output stores three elements, an in-tangent, a spline vertex,
        //              and an out-tangent. There **MUST** be at least two keyframes when using this interpolation.
        //if (sampler.Interpolation == AnimationSampler::INTERPOLATION_TYPE::CUBICSPLINE)
        // Not supported

        u = clamp(u, 0.f, 1.f);
        switch (channel.PathType)
        {
            case AnimationChannel::PATH_TYPE::TRANSLATION:
            {
                const float3 f3Start = sampler.OutputsVec4[Idx];
                const float3 f3End   = sampler.OutputsVec4[Idx + 1];
                NodeAnim.Translation = lerp(f3Start, f3End, u);
                break;
            }

            case AnimationChannel::PATH_TYPE::SCALE:
            {
                const float3 f3Start = sampler.OutputsVec4[Idx];
                const float3 f3End   = sampler.OutputsVec4[Idx + 1];
                NodeAnim.Scale       = lerp(f3Start, f3End, u);
                break;
            }

            case AnimationChannel::PATH_TYPE::ROTATION:
            {
                QuaternionF q1;
                q1.q.x = sampler.OutputsVec4[Idx].x;
                q1.q.y = sampler.OutputsVec4[Idx].y;
                q1.q.z = sampler.OutputsVec4[Idx].z;
                q1.q.w = sampler.OutputsVec4[Idx].w;

                QuaternionF q2;
                q2.q.x = sampler.OutputsVec4[Idx + 1].x;
                q2.q.y = sampler.OutputsVec4[Idx + 1].y;
                q2.q.z = sampler.OutputsVec4[Idx + 1].z;
                q2.q.w = sampler.OutputsVec4[Idx + 1].w;

                NodeAnim.Rotation = normalize(slerp(q1, q2, u));
                break;
            }

            case AnimationChannel::PATH_TYPE::WEIGHTS:
            {
                UNEXPECTED("Weights are not currently supported");
                break;
            }
        }
    }

    for (const auto* pN : scene.LinearNodes)
    {
        VERIFY_EXPR(pN != nullptr);
        const auto& A = Transforms.NodeAnimations[pN->Index];

        Transforms.NodeLocalMatrices[pN->Index] = ComputeNodeLocalMatrix(A.Scale, A.Rotation, A.Translation, pN->Matrix);
    }
}

} // namespace GLTF

} // namespace Diligent
