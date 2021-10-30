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

#pragma once

#include "../../../DiligentCore/Primitives/interface/FileStream.h"
#include "../../../DiligentCore/Graphics/GraphicsEngine/interface/RenderDevice.h"
#include "../../../DiligentCore/Graphics/GraphicsEngine/interface/Texture.h"
#include "Image.h"

DILIGENT_BEGIN_NAMESPACE(Diligent)

struct Image;

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


/// Texture loading information
struct TextureLoadInfo
{
    /// Texture name passed over to the texture creation method
    const Char* Name                    DEFAULT_VALUE(nullptr);

    /// Usage
    USAGE Usage                         DEFAULT_VALUE(USAGE_IMMUTABLE);

    /// Bind flags
    BIND_FLAGS BindFlags                DEFAULT_VALUE(BIND_SHADER_RESOURCE);

    /// Number of mip levels
    Uint32 MipLevels                    DEFAULT_VALUE(0);

    /// CPU access flags
    CPU_ACCESS_FLAGS CPUAccessFlags     DEFAULT_VALUE(CPU_ACCESS_NONE);

    /// Flag indicating if this texture uses sRGB gamma encoding
    Bool IsSRGB                         DEFAULT_VALUE(False);

    /// Flag indicating that the procedure should generate lower mip levels
    Bool GenerateMips                   DEFAULT_VALUE(True);

    /// Texture format
    TEXTURE_FORMAT Format               DEFAULT_VALUE(TEX_FORMAT_UNKNOWN);

    /// Alpha cut-off value used to remap alpha channel when generating mip
    /// levels as follows:
    ///
    ///     A_new = max(A_old; 1/3 * A_old + 2/3 * CutoffThreshold)
    ///
    /// \note This value must be in 0 to 1 range and is only
    ///       allowed for 4-channel 8-bit textures.
    float          AlphaCutoff          DEFAULT_VALUE(0);

    /// Coarse mip filter type, see Diligent::TEXTURE_LOAD_MIP_FILTER.
    TEXTURE_LOAD_MIP_FILTER MipFilter   DEFAULT_VALUE(TEXTURE_LOAD_MIP_FILTER_DEFAULT);

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
static const struct INTERFACE_ID IID_TextureLoader =
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
};
DILIGENT_END_INTERFACE
// clang-format on

#include "../../../DiligentCore/Primitives/interface/UndefInterfaceHelperMacros.h"

#if DILIGENT_C_INTERFACE

// clang-format off
#    define ITextureLoader_CreateTexture(This, ...)      CALL_IFACE_METHOD(TextureLoader_CreateTexture,       CreateTexture,       This, __VA_ARGS__)
#    define ITextureLoader_GetTextureDesc(This)          CALL_IFACE_METHOD(TextureLoader_GetTextureDesc,      GetTextureDesc,      This)
#    define ITextureLoader_GetSubresourceData(This, ...) CALL_IFACE_METHOD(TextureLoader_GetSubresourceData,  GetSubresourceData,  This, __VA_ARGS__)
// clang-format on

#endif

#include "../../../DiligentCore/Primitives/interface/DefineGlobalFuncHelperMacros.h"

/// Creates a texture loader from image.

/// \param [in]  pSrcImage   - Pointer to the source image object.
/// \param [in]  TexLoadInfo - Texture loading information, see Diligent::TextureLoadInfo.
/// \param [out] ppLoader    - Memory location where pointer to the created texture loader will be written.
void DILIGENT_GLOBAL_FUNCTION(CreateTextureLoaderFromImage)(struct Image*             pSrcImage,
                                                            const TextureLoadInfo REF TexLoadInfo,
                                                            ITextureLoader**          ppLoader);

/// Creates a texture loader from file.

/// \param [in]  FilePath   - File path.
/// \param [in]  FileFormat - File format. If this parameter is IMAGE_FILE_FORMAT_UNKNOWN,
///                           the format will be derived from the file contents.
/// \param [in]  TexLoadInfo - Texture loading information, see Diligent::TextureLoadInfo.
/// \param [out] ppLoader   - Memory location where pointer to the created texture loader will be written.
void DILIGENT_GLOBAL_FUNCTION(CreateTextureLoaderFromFile)(const char*               FilePath,
                                                           IMAGE_FILE_FORMAT         FileFormat,
                                                           const TextureLoadInfo REF TexLoadInfo,
                                                           ITextureLoader**          ppLoader);

/// Creates a texture loader from memory.

/// \param [in]  pData       - Pointer to the data.
/// \param [in]  Size        - The data size.
/// \param [in]  FileFormat  - File format. If this parameter is IMAGE_FILE_FORMAT_UNKNOWN,
///                            the format will be derived from the contents.
/// \param [in]  MakeCopy    - Whether to make the copy of the data (see remarks).
/// \param [in]  TexLoadInfo - Texture loading information, see Diligent::TextureLoadInfo.
/// \param [out] ppLoader    - Memory location where pointer to the created texture loader will be written.
///
/// \remarks    If MakeCopy is false, the pointer to the memory must remain valid until the
///             texture loader object is destroyed.
void DILIGENT_GLOBAL_FUNCTION(CreateTextureLoaderFromMemory)(const void*               pData,
                                                             size_t                    Size,
                                                             IMAGE_FILE_FORMAT         FileFormat,
                                                             bool                      MakeCopy,
                                                             const TextureLoadInfo REF TexLoadInfo,
                                                             ITextureLoader**          ppLoader);


/// Writes texture data as DDS file.

/// \param [in]  FilePath - DDS file path.
/// \param [in]  Desc     - Texture description.
/// \param [in]  TexData  - Texture subresource data.
/// \return     true if the file has been written successfully, and false otherwise.
bool DILIGENT_GLOBAL_FUNCTION(SaveTextureAsDDS)(const char*           FilePath,
                                                const TextureDesc REF Desc,
                                                const TextureData REF TexData);

#include "../../../DiligentCore/Primitives/interface/UndefGlobalFuncHelperMacros.h"

DILIGENT_END_NAMESPACE // namespace Diligent
