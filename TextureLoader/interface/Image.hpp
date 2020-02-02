/*
 *  Copyright 2019-2020 Diligent Graphics LLC
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

#include <vector>

#include "../../../DiligentCore/Graphics/GraphicsEngine/interface/GraphicsTypes.h"
#include "../../../DiligentCore/Primitives/interface/FileStream.h"
#include "../../../DiligentCore/Primitives/interface/DataBlob.h"
#include "../../../DiligentCore/Common/interface/RefCntAutoPtr.hpp"
#include "../../../DiligentCore/Common/interface/ObjectBase.hpp"

namespace Diligent
{

/// Image file format
enum class EImageFileFormat
{
    /// Unknown format
    unknown = 0,

    /// The image is encoded in JPEG format
    jpeg,

    /// The image is encoded in PNG format
    png,

    /// The image is encoded in TIFF format
    tiff,

    /// DDS file
    dds,

    /// KTX file
    ktx
};

/// Image loading information
struct ImageLoadInfo
{
    /// Image file format
    EImageFileFormat Format = EImageFileFormat::unknown;
};

/// Image description
struct ImageDesc
{
    /// Image width in pixels
    Uint32 Width = 0;

    /// Image height in pixels
    Uint32 Height = 0;

    /// Component type
    VALUE_TYPE ComponentType = VT_UNDEFINED;

    /// Number of color components
    Uint32 NumComponents = 0;

    /// Image row stride in bytes
    Uint32 RowStride = 0;
};

/// Implementation of a 2D image
struct Image : public ObjectBase<IObject>
{
    typedef ObjectBase<IObject> TBase;

    /// Creates a new image from the data blob

    /// \param [in] pFileData - Pointer to the data blob containing image data
    /// \param [in] LoadInfo - Image loading information
    /// \param [out] ppImage - Memory location where pointer to the created image is written.
    ///                        The image should be released via Release().
    static void CreateFromDataBlob(IDataBlob*           pFileData,
                                   const ImageLoadInfo& LoadInfo,
                                   Image**              ppImage);

    struct EncodeInfo
    {
        Uint32           Width       = 0;
        Uint32           Height      = 0;
        TEXTURE_FORMAT   TexFormat   = TEX_FORMAT_UNKNOWN;
        bool             KeepAlpha   = false;
        const void*      pData       = nullptr;
        Uint32           Stride      = 0;
        EImageFileFormat FileFormat  = EImageFileFormat::jpeg;
        int              JpegQuality = 95;
    };
    static void Encode(const EncodeInfo& Info, IDataBlob** ppEncodedData);

    /// Returns image description
    const ImageDesc& GetDesc() { return m_Desc; }

    /// Returns a pointer to the image data
    IDataBlob* GetData() { return m_pData; }

    static std::vector<Uint8> ConvertImageData(Uint32         Width,
                                               Uint32         Height,
                                               const Uint8*   pData,
                                               Uint32         Stride,
                                               TEXTURE_FORMAT SrcFormat,
                                               TEXTURE_FORMAT DstFormat,
                                               bool           KeepAlpha);

    static EImageFileFormat GetFileFormat(const Uint8* pData, size_t Size);

private:
    template <typename AllocatorType, typename ObjectType>
    friend class MakeNewRCObj;

    Image(IReferenceCounters*  pRefCounters,
          IDataBlob*           pFileData,
          const ImageLoadInfo& LoadInfo);

    void LoadPngFile(IDataBlob* pFileData, const ImageLoadInfo& LoadInfo);
    void LoadTiffFile(IDataBlob* pFileData, const ImageLoadInfo& LoadInfo);
    void LoadJpegFile(IDataBlob* pFileData, const ImageLoadInfo& LoadInfo);

    ImageDesc                m_Desc;
    RefCntAutoPtr<IDataBlob> m_pData;
};


/// Creates an image from file

/// \param [in] FilePath   - Source file path
/// \param [out] ppImage   - Memory location where pointer to the created image will be stored
/// \param [out] ppRawData - If the file format is not recognized by the function, it will load raw bytes
///                          and return them in the data blob. This parameter can be null.
/// \return                  Image file format.
EImageFileFormat CreateImageFromFile(const Char* FilePath,
                                     Image**     ppImage,
                                     IDataBlob** ppRawData = nullptr);

} // namespace Diligent
