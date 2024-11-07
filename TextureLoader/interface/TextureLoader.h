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

#pragma once

#include "../../../DiligentCore/Primitives/interface/FileStream.h"
#include "../../../DiligentCore/Graphics/GraphicsEngine/interface/RenderDevice.h"
#include "../../../DiligentCore/Graphics/GraphicsEngine/interface/Texture.h"
#include "Image.h"

#if DILIGENT_CPP_INTERFACE
#    include "../../../DiligentCore/Common/interface/RefCntAutoPtr.hpp"
#endif

DILIGENT_BEGIN_NAMESPACE(Diligent)

struct Image;
struct IMemoryAllocator;

// clang-format off

/// Coarse mip filter type
DILIGENT_TYPED_ENUM(TEXTURE_LOAD_MIP_FILTER, Uint8)
{
    /// Default filter type: BOX_AVERAGE for UNORM/SNORM and FP formats, and
    /// MOST_FREQUENT for UINT/SINT formats.
    TEXTURE_LOAD_MIP_FILTER_DEFAULT = 0,

    /// 2x2 box average.
    TEXTURE_LOAD_MIP_FILTER_BOX_AVERAGE,

    /// Use the most frequent element from the 2x2 box.
    /// This filter does not introduce new values and should be used
    /// for integer textures that contain non-filterable data (e.g. indices).
    TEXTURE_LOAD_MIP_FILTER_MOST_FREQUENT
};

/// Texture compression mode
DILIGENT_TYPED_ENUM(TEXTURE_LOAD_COMPRESS_MODE, Uint8)
{
    /// Do not compress the texture.
    TEXTURE_LOAD_COMPRESS_MODE_NONE = 0,

    /// Compress the texture using BC compression.
    /// 
    /// \remarks    The BC texture format is selected based on the number of channels in the
    ///             source image:
    ///                 * R8    -> BC4_UNORM
    ///                 * RG8   -> BC5_UNORM
    ///                 * RGB8  -> BC1_UNORM / BC1_UNORM_SRGB
    ///                 * RGBA8 -> BC3_UNORM / BC3_UNORM_SRGB
    TEXTURE_LOAD_COMPRESS_MODE_BC,

    /// Compress the texture using high-quality BC compression.
    ///
    /// \remarks    This mode is similar to TEXTURE_LOAD_COMPRESS_MODE_BC, but uses higher
    ///             quality settings that result in better image quality at the cost of
    ///             30%-40% longer compression time.
    TEXTURE_LOAD_COMPRESS_MODE_BC_HIGH_QUAL,
};

/// Texture loading information
struct TextureLoadInfo
{
    /// Texture name passed over to the texture creation method
    const Char* Name                    DEFAULT_INITIALIZER(nullptr);

    /// Usage
    USAGE Usage                         DEFAULT_INITIALIZER(USAGE_IMMUTABLE);

    /// Bind flags
    BIND_FLAGS BindFlags                DEFAULT_INITIALIZER(BIND_SHADER_RESOURCE);

    /// Number of mip levels
    Uint32 MipLevels                    DEFAULT_INITIALIZER(0);

    /// CPU access flags
    CPU_ACCESS_FLAGS CPUAccessFlags     DEFAULT_INITIALIZER(CPU_ACCESS_NONE);

    /// Flag indicating if this texture uses sRGB gamma encoding
    Bool IsSRGB                         DEFAULT_INITIALIZER(False);

    /// Flag indicating that the procedure should generate lower mip levels
    Bool GenerateMips                   DEFAULT_INITIALIZER(True);

    /// Flag indicating that the image should be flipped vertically
    Bool FlipVertically                 DEFAULT_INITIALIZER(False);

    /// Flag indicating that RGB channels should be premultiplied by alpha
    Bool PermultiplyAlpha               DEFAULT_INITIALIZER(False);

    /// Texture format
    TEXTURE_FORMAT Format               DEFAULT_INITIALIZER(TEX_FORMAT_UNKNOWN);

    /// Alpha cut-off value used to remap alpha channel when generating mip
    /// levels as follows:
    ///
    ///     A_new = max(A_old; 1/3 * A_old + 2/3 * CutoffThreshold)
    ///
    /// \note This value must be in 0 to 1 range and is only
    ///       allowed for 4-channel 8-bit textures.
    float          AlphaCutoff          DEFAULT_INITIALIZER(0);

    /// Coarse mip filter type, see Diligent::TEXTURE_LOAD_MIP_FILTER.
    TEXTURE_LOAD_MIP_FILTER MipFilter   DEFAULT_INITIALIZER(TEXTURE_LOAD_MIP_FILTER_DEFAULT);

    /// Texture compression mode, see Diligent::TEXTURE_LOAD_COMPRESS_MODE.
    TEXTURE_LOAD_COMPRESS_MODE CompressMode DEFAULT_INITIALIZER(TEXTURE_LOAD_COMPRESS_MODE_NONE);

    /// Texture component swizzle.
    ///
    /// \remarks    When the number of channels in the source image is less than
    ///             the number of channels in the destination texture, the following
    ///             rules apply:
    ///             - Alpha channel is always set to 1.
    ///             - Single-channel source image is replicated to all channels.
    ///             - Two-channel source image is replicated to RG channels, B channel is set to 0.
    TextureComponentMapping Swizzle DEFAULT_INITIALIZER(TextureComponentMapping::Identity());

    /// When non-zero, specifies the dimension that uniform images should be clipped to.
    ///
    /// \remarks    When this parameter is non-zero, the loader will check if all pixels
    ///             in the image have the same value. If this is the case, the image will
    ///             be clipped to the specified dimension.
    Uint32 UniformImageClipDim DEFAULT_INITIALIZER(0);

    /// An optional memory allocator to allocate memory for the texture.
    struct IMemoryAllocator* pAllocator DEFAULT_INITIALIZER(nullptr);

#if DILIGENT_CPP_INTERFACE
    explicit TextureLoadInfo(const Char*         _Name,
                             USAGE               _Usage             = TextureLoadInfo{}.Usage,
                             BIND_FLAGS          _BindFlags         = TextureLoadInfo{}.BindFlags,
                             Uint32              _MipLevels         = TextureLoadInfo{}.MipLevels,
                             CPU_ACCESS_FLAGS    _CPUAccessFlags    = TextureLoadInfo{}.CPUAccessFlags,
                             Bool                _IsSRGB            = TextureLoadInfo{}.IsSRGB,
                             Bool                _GenerateMips      = TextureLoadInfo{}.GenerateMips,
                             TEXTURE_FORMAT      _Format            = TextureLoadInfo{}.Format) :
        Name            {_Name},
        Usage           {_Usage},
        BindFlags       {_BindFlags},
        MipLevels       {_MipLevels},
        CPUAccessFlags  {_CPUAccessFlags},
        IsSRGB          {_IsSRGB},
        GenerateMips    {_GenerateMips},
        Format          {_Format}
    {}

    TextureLoadInfo(){};
#endif
};
typedef struct TextureLoadInfo TextureLoadInfo;
// clang-format on


// {E04FE6D5-8665-4183-A872-852E0F7CE242}
static DILIGENT_CONSTEXPR struct INTERFACE_ID IID_TextureLoader =
    {0xe04fe6d5, 0x8665, 0x4183, {0xa8, 0x72, 0x85, 0x2e, 0xf, 0x7c, 0xe2, 0x42}};

#define DILIGENT_INTERFACE_NAME ITextureLoader
#include "../../../DiligentCore/Primitives/interface/DefineInterfaceHelperMacros.h"

#define ITextureLoaderInclusiveMethods \
    IObjectInclusiveMethods;           \
    ITextureLoader TextureLoader

// clang-format off

/// Texture loader object.
DILIGENT_BEGIN_INTERFACE(ITextureLoader, IObject)
{
    /// Creates a texture using the prepared subresource data.
    VIRTUAL void METHOD(CreateTexture)(THIS_
                                       IRenderDevice* pDevice,
                                       ITexture**     ppTexture) PURE;

    /// Returns the texture description.
    VIRTUAL const TextureDesc REF METHOD(GetTextureDesc)(THIS) CONST PURE;

    /// Returns the subresource data for the given subresource.
    VIRTUAL const TextureSubResData REF METHOD(GetSubresourceData)(THIS_
                                                                   Uint32 MipLevel,
                                                                   Uint32 ArraySlice DEFAULT_VALUE(0)) CONST PURE;

    /// Returns the texture initialization data.
    VIRTUAL TextureData METHOD(GetTextureData)(THIS) PURE;
};
DILIGENT_END_INTERFACE
// clang-format on

#include "../../../DiligentCore/Primitives/interface/UndefInterfaceHelperMacros.h"

#if DILIGENT_C_INTERFACE

// clang-format off
#    define ITextureLoader_CreateTexture(This, ...)      CALL_IFACE_METHOD(TextureLoader_CreateTexture,       CreateTexture,       This, __VA_ARGS__)
#    define ITextureLoader_GetTextureDesc(This)          CALL_IFACE_METHOD(TextureLoader_GetTextureDesc,      GetTextureDesc,      This)
#    define ITextureLoader_GetSubresourceData(This, ...) CALL_IFACE_METHOD(TextureLoader_GetSubresourceData,  GetSubresourceData,  This, __VA_ARGS__)
#    define ITextureLoader_GetTextureData(This)          CALL_IFACE_METHOD(TextureLoader_GetTextureData,      GetTextureData,      This)
// clang-format on

#endif

#include "../../../DiligentCore/Primitives/interface/DefineGlobalFuncHelperMacros.h"

/// Creates a texture loader from image.

/// \param [in]  pSrcImage   - Pointer to the source image object.
/// \param [in]  TexLoadInfo - Texture loading information, see Diligent::TextureLoadInfo.
/// \param [out] ppLoader    - Memory location where a pointer to the created texture loader will be written.
void DILIGENT_GLOBAL_FUNCTION(CreateTextureLoaderFromImage)(struct Image*             pSrcImage,
                                                            const TextureLoadInfo REF TexLoadInfo,
                                                            ITextureLoader**          ppLoader);

/// Creates a texture loader from file.

/// \param [in]  FilePath   - File path.
/// \param [in]  FileFormat - File format. If this parameter is IMAGE_FILE_FORMAT_UNKNOWN,
///                           the format will be derived from the file contents.
/// \param [in]  TexLoadInfo - Texture loading information, see Diligent::TextureLoadInfo.
/// \param [out] ppLoader   - Memory location where a pointer to the created texture loader will be written.
void DILIGENT_GLOBAL_FUNCTION(CreateTextureLoaderFromFile)(const char*               FilePath,
                                                           IMAGE_FILE_FORMAT         FileFormat,
                                                           const TextureLoadInfo REF TexLoadInfo,
                                                           ITextureLoader**          ppLoader);

/// Creates a texture loader from memory.

/// \param [in]  pData       - Pointer to the texture data.
/// \param [in]  Size        - The data size.
/// \param [in]  MakeCopy    - Whether to make the copy of the data (see remarks).
/// \param [in]  TexLoadInfo - Texture loading information, see Diligent::TextureLoadInfo.
/// \param [out] ppLoader    - Memory location where a pointer to the created texture loader will be written.
///
/// \remarks    If MakeCopy is false, the pointer to the memory must remain valid until the
///             texture loader object is destroyed.
void DILIGENT_GLOBAL_FUNCTION(CreateTextureLoaderFromMemory)(const void*               pData,
                                                             size_t                    Size,
                                                             bool                      MakeCopy,
                                                             const TextureLoadInfo REF TexLoadInfo,
                                                             ITextureLoader**          ppLoader);

/// Creates a texture loader from data blob.
///
/// \param [in]  pDataBlob   - Pointer to the data blob that contains the texture data.
/// \param [in]  TexLoadInfo - Texture loading information, see Diligent::TextureLoadInfo.
/// \param [out] ppLoader    - Memory location where a pointer to the created texture loader will be written.
///
/// \remarks    If needed, the loader will keep a strong reference to the data blob.
void DILIGENT_GLOBAL_FUNCTION(CreateTextureLoaderFromDataBlob)(IDataBlob*                pDataBlob,
                                                               const TextureLoadInfo REF TexLoadInfo,
                                                               ITextureLoader**          ppLoader);

#if DILIGENT_CPP_INTERFACE
void CreateTextureLoaderFromDataBlob(RefCntAutoPtr<IDataBlob> pDataBlob,
                                     const TextureLoadInfo&   TexLoadInfo,
                                     ITextureLoader**         ppLoader);
#endif


/// Returns the memory requirement for the texture loader.
///
/// \param [in]  pData       - Pointer to the source image data.
/// \param [in]  Size        - The data size.
/// \param [in]  TexLoadInfo - Texture loading information, see Diligent::TextureLoadInfo.
/// \return     The memory requirement in bytes.
///
/// \remarks    This function can be used to estimate the memory requirement for the texture loader.
///             The memory requirement includes the size of the texture data plus the size of the
///             intermediate data structures used by the loader. It does not include the size of
///             the source image data.
///             The actual memory used by the loader may be slightly different.
size_t DILIGENT_GLOBAL_FUNCTION(GetTextureLoaderMemoryRequirement)(const void*               pData,
                                                                   size_t                    Size,
                                                                   const TextureLoadInfo REF TexLoadInfo);


/// Writes texture data as DDS file.

/// \param [in]  FilePath - DDS file path.
/// \param [in]  Desc     - Texture description.
/// \param [in]  TexData  - Texture subresource data.
/// \return     true if the file has been written successfully, and false otherwise.
bool DILIGENT_GLOBAL_FUNCTION(SaveTextureAsDDS)(const char*           FilePath,
                                                const TextureDesc REF Desc,
                                                const TextureData REF TexData);


/// Writes texture as DDS to a file stream.

/// \param [in]  pFileStream - File stream.
/// \param [in]  Desc        - Texture description.
/// \param [in]  TexData     - Texture subresource data.
/// \return     true if the texture has been written successfully, and false otherwise.
bool DILIGENT_GLOBAL_FUNCTION(WriteDDSToStream)(IFileStream*          pFileStream,
                                                const TextureDesc REF Desc,
                                                const TextureData REF TexData);

#include "../../../DiligentCore/Primitives/interface/UndefGlobalFuncHelperMacros.h"

DILIGENT_END_NAMESPACE // namespace Diligent
