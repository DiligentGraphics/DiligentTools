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

#include "pch.h"
#include <algorithm>
#include <limits>
#include <math.h>
#include <vector>
#include <array>

#include "TextureLoaderImpl.hpp"
#include "GraphicsAccessories.hpp"
#include "GraphicsUtilities.h"
#include "TextureUtilities.h"
#include "PNGCodec.h"
#include "JPEGCodec.h"
#include "ColorConversion.h"
#include "Image.h"
#include "FileWrapper.hpp"
#include "DataBlobImpl.hpp"
#include "Align.hpp"

#define STB_DXT_STATIC
#define STB_DXT_IMPLEMENTATION
#include "../../ThirdParty/stb/stb_dxt.h"

extern "C"
{
    Diligent::DECODE_PNG_RESULT Diligent_DecodePng(const void*          pSrcPngBits,
                                                   size_t               PngDataSize,
                                                   Diligent::IDataBlob* pDstPixels,
                                                   Diligent::ImageDesc* pDstImgDesc);

    Diligent::ENCODE_PNG_RESULT Diligent_EncodePng(const Diligent::Uint8* pSrcPixels,
                                                   Diligent::Uint32       Width,
                                                   Diligent::Uint32       Height,
                                                   Diligent::Uint32       StrideInBytes,
                                                   int                    PngColorType,
                                                   Diligent::IDataBlob*   pDstPngBits);

    Diligent::DECODE_JPEG_RESULT Diligent_DecodeJpeg(const void*          pSrcJpegBits,
                                                     size_t               JpegDataSize,
                                                     Diligent::IDataBlob* pDstPixels,
                                                     Diligent::ImageDesc* pDstImgDesc);

    Diligent::ENCODE_JPEG_RESULT Diligent_EncodeJpeg(Diligent::Uint8*     pSrcRGBData,
                                                     Diligent::Uint32     Width,
                                                     Diligent::Uint32     Height,
                                                     int                  quality,
                                                     Diligent::IDataBlob* pDstJpegBits);

    Diligent::DECODE_JPEG_RESULT Diligent_LoadSGI(const Diligent::IDataBlob* pSrcJpegBits,
                                                  Diligent::IDataBlob*       pDstPixels,
                                                  Diligent::ImageDesc*       pDstImgDesc);
}

namespace Diligent
{

DECODE_PNG_RESULT DecodePng(const void* pSrcPngBits,
                            size_t      PngDataSize,
                            IDataBlob*  pDstPixels,
                            ImageDesc*  pDstImgDesc)
{
    return Diligent_DecodePng(pSrcPngBits, PngDataSize, pDstPixels, pDstImgDesc);
}

ENCODE_PNG_RESULT EncodePng(const Uint8* pSrcPixels,
                            Uint32       Width,
                            Uint32       Height,
                            Uint32       StrideInBytes,
                            int          PngColorType,
                            IDataBlob*   pDstPngBits)
{
    return Diligent_EncodePng(pSrcPixels, Width, Height, StrideInBytes, PngColorType, pDstPngBits);
}


DECODE_JPEG_RESULT DecodeJpeg(const void* pSrcJpegBits,
                              size_t      JpegDataSize,
                              IDataBlob*  pDstPixels,
                              ImageDesc*  pDstImgDesc)
{
    return Diligent_DecodeJpeg(pSrcJpegBits, JpegDataSize, pDstPixels, pDstImgDesc);
}

ENCODE_JPEG_RESULT EncodeJpeg(Uint8*     pSrcRGBPixels,
                              Uint32     Width,
                              Uint32     Height,
                              int        quality,
                              IDataBlob* pDstJpegBits)
{
    return Diligent_EncodeJpeg(pSrcRGBPixels, Width, Height, quality, pDstJpegBits);
}

static TextureDesc TexDescFromTexLoadInfo(const TextureLoadInfo& TexLoadInfo, const std::string& Name)
{
    TextureDesc TexDesc;
    TexDesc.Name           = Name.c_str();
    TexDesc.Format         = TexLoadInfo.Format;
    TexDesc.Usage          = TexLoadInfo.Usage;
    TexDesc.BindFlags      = TexLoadInfo.BindFlags;
    TexDesc.CPUAccessFlags = TexLoadInfo.CPUAccessFlags;
    return TexDesc;
}

TextureLoaderImpl::TextureLoaderImpl(IReferenceCounters*      pRefCounters,
                                     const TextureLoadInfo&   TexLoadInfo,
                                     const Uint8*             pData,
                                     size_t                   DataSize,
                                     RefCntAutoPtr<IDataBlob> pDataBlob) :
    TBase{pRefCounters},
    m_pDataBlob{std::move(pDataBlob)},
    m_Name{TexLoadInfo.Name != nullptr ? TexLoadInfo.Name : ""},
    m_TexDesc{TexDescFromTexLoadInfo(TexLoadInfo, m_Name)}
{
    const IMAGE_FILE_FORMAT ImgFileFormat = Image::GetFileFormat(pData, DataSize);
    if (ImgFileFormat == IMAGE_FILE_FORMAT_UNKNOWN)
    {
        LOG_ERROR_AND_THROW("Unable to derive image format.");
    }

    if (ImgFileFormat == IMAGE_FILE_FORMAT_PNG ||
        ImgFileFormat == IMAGE_FILE_FORMAT_JPEG ||
        ImgFileFormat == IMAGE_FILE_FORMAT_TIFF ||
        ImgFileFormat == IMAGE_FILE_FORMAT_SGI ||
        ImgFileFormat == IMAGE_FILE_FORMAT_HDR ||
        ImgFileFormat == IMAGE_FILE_FORMAT_TGA)
    {
        ImageLoadInfo ImgLoadInfo;
        ImgLoadInfo.Format = ImgFileFormat;
        if (!m_pDataBlob)
        {
            m_pDataBlob = DataBlobImpl::Create(TexLoadInfo.pAllocator, DataSize, pData);
        }
        ImgLoadInfo.IsSRGB           = TexLoadInfo.IsSRGB;
        ImgLoadInfo.PermultiplyAlpha = TexLoadInfo.PermultiplyAlpha;
        ImgLoadInfo.pAllocator       = TexLoadInfo.pAllocator;
        RefCntAutoPtr<Image> pImage;
        Image::CreateFromDataBlob(m_pDataBlob, ImgLoadInfo, &pImage);
        m_pDataBlob.Release();
        LoadFromImage(std::move(pImage), TexLoadInfo);
    }
    else
    {
        if (ImgFileFormat == IMAGE_FILE_FORMAT_DDS)
        {
            LoadFromDDS(TexLoadInfo, pData, DataSize);
        }
        else if (ImgFileFormat == IMAGE_FILE_FORMAT_KTX)
        {
            LoadFromKTX(TexLoadInfo, pData, DataSize);
        }
    }

    if (TexLoadInfo.IsSRGB)
    {
        m_TexDesc.Format = UnormFormatToSRGB(m_TexDesc.Format);
    }
}

TextureLoaderImpl::TextureLoaderImpl(IReferenceCounters*    pRefCounters,
                                     const TextureLoadInfo& TexLoadInfo,
                                     RefCntAutoPtr<Image>   pImage) :
    TBase{pRefCounters},
    m_Name{TexLoadInfo.Name != nullptr ? TexLoadInfo.Name : ""},
    m_TexDesc{TexDescFromTexLoadInfo(TexLoadInfo, m_Name)}
{
    LoadFromImage(std::move(pImage), TexLoadInfo);
}

void TextureLoaderImpl::CreateTexture(IRenderDevice* pDevice,
                                      ITexture**     ppTexture)
{
    TextureData InitData = GetTextureData();
    pDevice->CreateTexture(m_TexDesc, &InitData, ppTexture);
}

void TextureLoaderImpl::LoadFromImage(RefCntAutoPtr<Image> pImage, const TextureLoadInfo& TexLoadInfo)
{
    VERIFY_EXPR(pImage != nullptr);

    ImageDesc ImgDesc = pImage->GetDesc();
    if (TexLoadInfo.UniformImageClipDim != 0 && pImage->IsUniform())
    {
        ImgDesc.Width  = std::min(ImgDesc.Width, TexLoadInfo.UniformImageClipDim);
        ImgDesc.Height = std::min(ImgDesc.Height, TexLoadInfo.UniformImageClipDim);
    }

    m_TexDesc.Type      = RESOURCE_DIM_TEX_2D;
    m_TexDesc.Width     = ImgDesc.Width;
    m_TexDesc.Height    = ImgDesc.Height;
    m_TexDesc.MipLevels = ComputeMipLevelsCount(m_TexDesc.Width, m_TexDesc.Height);
    if (TexLoadInfo.MipLevels > 0)
        m_TexDesc.MipLevels = std::min(m_TexDesc.MipLevels, TexLoadInfo.MipLevels);

    const Uint32 CompSize = GetValueSize(ImgDesc.ComponentType);

    if (m_TexDesc.Format == TEX_FORMAT_UNKNOWN)
    {
        const COMPONENT_TYPE CompType = ValueTypeToComponentType(ImgDesc.ComponentType, /*IsNormalized = */ true, TexLoadInfo.IsSRGB);

        Uint32 NumComponents = ImgDesc.NumComponents;
        if (NumComponents == 3 || CompType == COMPONENT_TYPE_UNORM_SRGB)
        {
            // Note that there is RGB32_FLOAT format, but it can't be filtered, so always extend RGB to RGBA.
            NumComponents = 4;
        }
        DEV_CHECK_ERR(CompType != COMPONENT_TYPE_UNDEFINED, "Failed to deduce component type from image component type ", GetValueTypeString(ImgDesc.ComponentType), " and sRGB flag ", TexLoadInfo.IsSRGB);

        m_TexDesc.Format = TextureComponentAttribsToTextureFormat(CompType, CompSize, NumComponents);
        if (m_TexDesc.Format == TEX_FORMAT_UNKNOWN)
        {
            LOG_ERROR_AND_THROW("Failed to deduce texture format from image component type ", GetValueTypeString(ImgDesc.ComponentType), " and number of components ", ImgDesc.NumComponents);
        }
    }
    const auto&  TexFmtDesc    = GetTextureFormatAttribs(m_TexDesc.Format);
    const Uint32 NumComponents = TexFmtDesc.NumComponents;

    m_SubResources.resize(m_TexDesc.MipLevels);
    m_Mips.resize(m_TexDesc.MipLevels);

    const bool SwizzleRequired =
        (NumComponents >= 1 && TexLoadInfo.Swizzle.R != TEXTURE_COMPONENT_SWIZZLE_IDENTITY && TexLoadInfo.Swizzle.R != TEXTURE_COMPONENT_SWIZZLE_R) ||
        (NumComponents >= 2 && TexLoadInfo.Swizzle.G != TEXTURE_COMPONENT_SWIZZLE_IDENTITY && TexLoadInfo.Swizzle.G != TEXTURE_COMPONENT_SWIZZLE_G) ||
        (NumComponents >= 3 && TexLoadInfo.Swizzle.B != TEXTURE_COMPONENT_SWIZZLE_IDENTITY && TexLoadInfo.Swizzle.B != TEXTURE_COMPONENT_SWIZZLE_B) ||
        (NumComponents >= 4 && TexLoadInfo.Swizzle.A != TEXTURE_COMPONENT_SWIZZLE_IDENTITY && TexLoadInfo.Swizzle.A != TEXTURE_COMPONENT_SWIZZLE_A);

    if (ImgDesc.NumComponents != NumComponents ||
        TexFmtDesc.ComponentSize != CompSize ||
        TexLoadInfo.FlipVertically ||
        SwizzleRequired)
    {
        Uint32 DstStride         = ImgDesc.Width * NumComponents * TexFmtDesc.ComponentSize;
        DstStride                = AlignUp(DstStride, Uint32{4});
        m_Mips[0]                = DataBlobImpl::Create(TexLoadInfo.pAllocator, size_t{DstStride} * size_t{ImgDesc.Height});
        m_SubResources[0].pData  = m_Mips[0]->GetDataPtr();
        m_SubResources[0].Stride = DstStride;

        CopyPixelsAttribs CopyAttribs;
        CopyAttribs.Width            = ImgDesc.Width;
        CopyAttribs.Height           = ImgDesc.Height;
        CopyAttribs.SrcComponentSize = CompSize;
        CopyAttribs.pSrcPixels       = pImage->GetData()->GetConstDataPtr();
        CopyAttribs.SrcStride        = ImgDesc.RowStride;
        CopyAttribs.SrcCompCount     = ImgDesc.NumComponents;
        CopyAttribs.pDstPixels       = m_Mips[0]->GetDataPtr();
        CopyAttribs.DstComponentSize = TexFmtDesc.ComponentSize;
        CopyAttribs.DstStride        = DstStride;
        CopyAttribs.DstCompCount     = NumComponents;
        CopyAttribs.FlipVertically   = TexLoadInfo.FlipVertically;

        if (CopyAttribs.SrcCompCount < 4)
        {
            // Always set alpha to 1 (except for float formats)
            CopyAttribs.Swizzle.A = TexFmtDesc.ComponentType != COMPONENT_TYPE_FLOAT ?
                TEXTURE_COMPONENT_SWIZZLE_ONE :
                TEXTURE_COMPONENT_SWIZZLE_ZERO;
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

        // Combine swizzles
        if (SwizzleRequired)
        {
            CopyAttribs.Swizzle *= TexLoadInfo.Swizzle;
        }

        CopyPixels(CopyAttribs);
        // Release original image
        pImage.Release();
    }
    else
    {
        // Keep strong reference to the image to prevent it from being destroyed
        // since we are going to use its data directly.
        m_pImage                 = std::move(pImage);
        m_SubResources[0].pData  = m_pImage->GetData()->GetConstDataPtr();
        m_SubResources[0].Stride = ImgDesc.RowStride;
    }

    for (Uint32 m = 1; m < m_TexDesc.MipLevels; ++m)
    {
        const MipLevelProperties MipLevelProps = GetMipLevelProperties(m_TexDesc, m);

        Uint64 MipSize = MipLevelProps.MipSize;
        Uint64 RowSize = MipLevelProps.RowSize;
        if ((RowSize % 4) != 0)
        {
            RowSize = AlignUp(RowSize, Uint64{4});
            MipSize = RowSize * MipLevelProps.LogicalHeight;
        }
        m_Mips[m]                = DataBlobImpl::Create(TexLoadInfo.pAllocator, StaticCast<size_t>(MipSize));
        m_SubResources[m].pData  = m_Mips[m]->GetDataPtr();
        m_SubResources[m].Stride = RowSize;

        if (TexLoadInfo.GenerateMips)
        {
            MipLevelProperties FinerMipProps = GetMipLevelProperties(m_TexDesc, m - 1);
            if (TexLoadInfo.GenerateMips)
            {
                ComputeMipLevelAttribs Attribs;
                Attribs.Format          = m_TexDesc.Format;
                Attribs.FineMipWidth    = FinerMipProps.LogicalWidth;
                Attribs.FineMipHeight   = FinerMipProps.LogicalHeight;
                Attribs.pFineMipData    = m_SubResources[m - 1].pData;
                Attribs.FineMipStride   = StaticCast<size_t>(m_SubResources[m - 1].Stride);
                Attribs.pCoarseMipData  = m_Mips[m]->GetDataPtr();
                Attribs.CoarseMipStride = StaticCast<size_t>(m_SubResources[m].Stride);
                Attribs.AlphaCutoff     = TexLoadInfo.AlphaCutoff;
                static_assert(MIP_FILTER_TYPE_DEFAULT == static_cast<MIP_FILTER_TYPE>(TEXTURE_LOAD_MIP_FILTER_DEFAULT), "Inconsistent enum values");
                static_assert(MIP_FILTER_TYPE_BOX_AVERAGE == static_cast<MIP_FILTER_TYPE>(TEXTURE_LOAD_MIP_FILTER_BOX_AVERAGE), "Inconsistent enum values");
                static_assert(MIP_FILTER_TYPE_MOST_FREQUENT == static_cast<MIP_FILTER_TYPE>(TEXTURE_LOAD_MIP_FILTER_MOST_FREQUENT), "Inconsistent enum values");
                Attribs.FilterType = static_cast<MIP_FILTER_TYPE>(TexLoadInfo.MipFilter);
                ComputeMipLevel(Attribs);
            }
        }
    }

    if (TexLoadInfo.CompressMode != TEXTURE_LOAD_COMPRESS_MODE_NONE)
    {
        CompressSubresources(NumComponents, ImgDesc.NumComponents, TexLoadInfo);
    }
}

void TextureLoaderImpl::CompressSubresources(Uint32 NumComponents, Uint32 NumSrcComponents, const TextureLoadInfo& TexLoadInfo)
{
    TEXTURE_FORMAT CompressedFormat = TEX_FORMAT_UNKNOWN;
    switch (NumComponents)
    {
        case 1:
            CompressedFormat = TEX_FORMAT_BC4_UNORM;
            break;

        case 2:
            CompressedFormat = TEX_FORMAT_BC5_UNORM;
            break;

        case 4:
            if (NumSrcComponents == 4)
                CompressedFormat = TexLoadInfo.IsSRGB ? TEX_FORMAT_BC3_UNORM_SRGB : TEX_FORMAT_BC3_UNORM;
            else
                CompressedFormat = TexLoadInfo.IsSRGB ? TEX_FORMAT_BC1_UNORM_SRGB : TEX_FORMAT_BC1_UNORM;
            break;

        default:
            UNEXPECTED("Unexpected number of components ", NumComponents);
    }

    if (CompressedFormat == TEX_FORMAT_UNKNOWN)
        return;

    m_TexDesc.Format                       = CompressedFormat;
    const TextureFormatAttribs& FmtAttribs = GetTextureFormatAttribs(CompressedFormat);

    std::vector<RefCntAutoPtr<IDataBlob>> CompressedMips(m_SubResources.size());
    for (Uint32 slice = 0; slice < m_TexDesc.GetArraySize(); ++slice)
    {
        for (Uint32 mip = 0; mip < m_TexDesc.MipLevels; ++mip)
        {
            const Uint32              SubResIndex   = slice * m_TexDesc.MipLevels + mip;
            TextureSubResData&        SubResData    = m_SubResources[SubResIndex];
            RefCntAutoPtr<IDataBlob>& CompressedMip = CompressedMips[SubResIndex];

            const MipLevelProperties CompressedMipProps = GetMipLevelProperties(m_TexDesc, mip);
            const Uint32             MaxCol             = CompressedMipProps.LogicalWidth - 1;
            const Uint32             MaxRow             = CompressedMipProps.LogicalHeight - 1;
            const size_t             CompressedStride   = static_cast<size_t>(CompressedMipProps.RowSize);
            CompressedMip                               = DataBlobImpl::Create(TexLoadInfo.pAllocator, CompressedStride * CompressedMipProps.StorageHeight);

            for (Uint32 row = 0; row < CompressedMipProps.StorageHeight; row += FmtAttribs.BlockHeight)
            {
                const Uint32 row0 = row;
                const Uint32 row1 = std::min(row + 1, MaxRow);
                const Uint32 row2 = std::min(row + 2, MaxRow);
                const Uint32 row3 = std::min(row + 3, MaxRow);

                for (Uint32 col = 0; col < CompressedMipProps.StorageWidth; col += FmtAttribs.BlockWidth)
                {
                    const Uint32 col0 = col;
                    const Uint32 col1 = std::min(col + 1, MaxCol);
                    const Uint32 col2 = std::min(col + 2, MaxCol);
                    const Uint32 col3 = std::min(col + 3, MaxCol);

                    auto ReadBlockData = [&](auto& BlockData) {
                        using T = typename std::decay_t<decltype(BlockData)>::value_type;

                        const T*     SrcPtr    = static_cast<const T*>(SubResData.pData);
                        const size_t SrcStride = static_cast<size_t>(SubResData.Stride) / sizeof(T);
                        // clang-format off
                        BlockData =
                        {
                            SrcPtr[col0 + SrcStride * row0], SrcPtr[col1 + SrcStride * row0], SrcPtr[col2 + SrcStride * row0], SrcPtr[col3 + SrcStride * row0],
                            SrcPtr[col0 + SrcStride * row1], SrcPtr[col1 + SrcStride * row1], SrcPtr[col2 + SrcStride * row1], SrcPtr[col3 + SrcStride * row1],
                            SrcPtr[col0 + SrcStride * row2], SrcPtr[col1 + SrcStride * row2], SrcPtr[col2 + SrcStride * row2], SrcPtr[col3 + SrcStride * row2],
                            SrcPtr[col0 + SrcStride * row3], SrcPtr[col1 + SrcStride * row3], SrcPtr[col2 + SrcStride * row3], SrcPtr[col3 + SrcStride * row3]
                        };
                        // clang-format on
                        return reinterpret_cast<const unsigned char*>(BlockData.data());
                    };

                    Uint8* pDst = CompressedMip->GetDataPtr<Uint8>() + (col / FmtAttribs.BlockWidth) * FmtAttribs.ComponentSize + CompressedStride * (row / FmtAttribs.BlockHeight);
                    if (NumComponents == 1)
                    {
                        std::array<Uint8, 16> BlockData8;
                        stb_compress_bc4_block(pDst, ReadBlockData(BlockData8));
                    }
                    else if (NumComponents == 2)
                    {
                        std::array<Uint16, 16> BlockData16;
                        stb_compress_bc5_block(pDst, ReadBlockData(BlockData16));
                    }
                    else if (NumComponents == 4)
                    {
                        std::array<Uint32, 16> BlockData32;
                        const int              StbDxtMode = (TexLoadInfo.CompressMode == TEXTURE_LOAD_COMPRESS_MODE_BC_HIGH_QUAL) ? STB_DXT_HIGHQUAL : STB_DXT_NORMAL;
                        const int              StoreAlpha = NumSrcComponents == 4 ? 1 : 0;
                        stb_compress_dxt_block(pDst, ReadBlockData(BlockData32), StoreAlpha, StbDxtMode);
                    }
                    else
                    {
                        UNEXPECTED("Unexpected number of components");
                    }
                }
            }

            SubResData.pData  = CompressedMip->GetDataPtr();
            SubResData.Stride = CompressedStride;
            m_Mips[SubResIndex].Release();
            if (SubResIndex == 0)
            {
                VERIFY(!m_pImage || m_TexDesc.GetArraySize() == 1, "Array textures can't be loaded from an image");
                m_pImage.Release();
            }
        }
    }

    m_TexDesc.Width  = AlignUp(m_TexDesc.Width, FmtAttribs.BlockWidth);
    m_TexDesc.Height = AlignUp(m_TexDesc.Height, FmtAttribs.BlockHeight);
    m_Mips.swap(CompressedMips);
}

void CreateTextureLoaderFromFile(const char*            FilePath,
                                 IMAGE_FILE_FORMAT      FileFormat,
                                 const TextureLoadInfo& TexLoadInfo,
                                 ITextureLoader**       ppLoader)
{
    try
    {
        FileWrapper File{FilePath, EFileAccessMode::Read};
        if (!File)
            LOG_ERROR_AND_THROW("Failed to open file '", FilePath, "'.");

        RefCntAutoPtr<DataBlobImpl> pFileData = DataBlobImpl::Create(TexLoadInfo.pAllocator);
        File->Read(pFileData);

        RefCntAutoPtr<TextureLoaderImpl> pTexLoader{
            MakeNewRCObj<TextureLoaderImpl>()(TexLoadInfo, pFileData->GetConstDataPtr<Uint8>(), pFileData->GetSize(), std::move(pFileData)),
        };
        if (pTexLoader)
            pTexLoader->QueryInterface(IID_TextureLoader, reinterpret_cast<IObject**>(ppLoader));
    }
    catch (std::runtime_error& err)
    {
        LOG_ERROR("Failed to create texture loader from file: ", err.what());
    }
}

void CreateTextureLoaderFromMemory(const void*            pData,
                                   size_t                 Size,
                                   bool                   MakeDataCopy,
                                   const TextureLoadInfo& TexLoadInfo,
                                   ITextureLoader**       ppLoader)
{
    VERIFY_EXPR(pData != nullptr && Size > 0);
    try
    {
        RefCntAutoPtr<IDataBlob> pDataCopy;
        if (MakeDataCopy)
        {
            pDataCopy = DataBlobImpl::Create(TexLoadInfo.pAllocator, Size, pData);
            pData     = pDataCopy->GetConstDataPtr();
        }
        RefCntAutoPtr<ITextureLoader> pTexLoader{MakeNewRCObj<TextureLoaderImpl>()(TexLoadInfo, reinterpret_cast<const Uint8*>(pData), Size, std::move(pDataCopy))};
        if (pTexLoader)
            pTexLoader->QueryInterface(IID_TextureLoader, reinterpret_cast<IObject**>(ppLoader));
    }
    catch (std::runtime_error& err)
    {
        LOG_ERROR("Failed to create texture loader from memory: ", err.what());
    }
}

void CreateTextureLoaderFromDataBlob(RefCntAutoPtr<IDataBlob> pDataBlob,
                                     const TextureLoadInfo&   TexLoadInfo,
                                     ITextureLoader**         ppLoader)
{
    try
    {
        const Uint8* pData = pDataBlob->GetConstDataPtr<Uint8>();
        const size_t Size  = pDataBlob->GetSize();

        RefCntAutoPtr<ITextureLoader> pTexLoader{MakeNewRCObj<TextureLoaderImpl>()(TexLoadInfo, pData, Size, std::move(pDataBlob))};
        if (pTexLoader)
            pTexLoader->QueryInterface(IID_TextureLoader, reinterpret_cast<IObject**>(ppLoader));
    }
    catch (std::runtime_error& err)
    {
        LOG_ERROR("Failed to create texture loader from data blob: ", err.what());
    }
}

void CreateTextureLoaderFromDataBlob(IDataBlob*             pDataBlob,
                                     const TextureLoadInfo& TexLoadInfo,
                                     ITextureLoader**       ppLoader)
{
    CreateTextureLoaderFromDataBlob(RefCntAutoPtr<IDataBlob>{pDataBlob, IID_DataBlob}, TexLoadInfo, ppLoader);
}

void CreateTextureLoaderFromImage(Image*                 pSrcImage,
                                  const TextureLoadInfo& TexLoadInfo,
                                  ITextureLoader**       ppLoader)
{
    VERIFY_EXPR(pSrcImage != nullptr);
    try
    {
        RefCntAutoPtr<ITextureLoader> pTexLoader{MakeNewRCObj<TextureLoaderImpl>()(TexLoadInfo, RefCntAutoPtr<Image>{pSrcImage})};
        if (pTexLoader)
            pTexLoader->QueryInterface(IID_TextureLoader, reinterpret_cast<IObject**>(ppLoader));
    }
    catch (std::runtime_error& err)
    {
        LOG_ERROR("Failed to create texture loader from memory: ", err.what());
    }
}

} // namespace Diligent

extern "C"
{
    void Diligent_CreateTextureLoaderFromFile(const char*                      FilePath,
                                              Diligent::IMAGE_FILE_FORMAT      FileFormat,
                                              const Diligent::TextureLoadInfo& TexLoadInfo,
                                              Diligent::ITextureLoader**       ppLoader)
    {
        Diligent::CreateTextureLoaderFromFile(FilePath, FileFormat, TexLoadInfo, ppLoader);
    }

    void Diligent_CreateTextureLoaderFromMemory(const void*                      pData,
                                                size_t                           Size,
                                                bool                             MakeCopy,
                                                const Diligent::TextureLoadInfo& TexLoadInfo,
                                                Diligent::ITextureLoader**       ppLoader)
    {
        Diligent::CreateTextureLoaderFromMemory(pData, Size, MakeCopy, TexLoadInfo, ppLoader);
    }

    void Diligent_CreateTextureLoaderFromImage(Diligent::Image*                 pSrcImage,
                                               const Diligent::TextureLoadInfo& TexLoadInfo,
                                               Diligent::ITextureLoader**       ppLoader)
    {
        Diligent::CreateTextureLoaderFromImage(pSrcImage, TexLoadInfo, ppLoader);
    }
}
