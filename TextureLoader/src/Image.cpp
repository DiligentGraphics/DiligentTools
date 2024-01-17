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
#include <array>

#include "Image.h"
#include "Errors.hpp"

#include "tiffio.h"
#include "png.h"
#include "PNGCodec.h"
#include "JPEGCodec.h"
#include "SGILoader.h"

#include "DataBlobImpl.hpp"
#include "DebugUtilities.hpp"
#include "RefCntAutoPtr.hpp"
#include "Align.hpp"
#include "GraphicsAccessories.hpp"
#include "BasicFileStream.hpp"
#include "StringTools.hpp"
#include "TextureUtilities.h"

namespace Diligent
{

class TIFFClientOpenWrapper
{
public:
    explicit TIFFClientOpenWrapper(IDataBlob* pData) noexcept :
        m_Offset{0},
        m_Size{pData->GetSize()},
        m_pData{pData}
    {
    }

    static tmsize_t TIFFReadProc(thandle_t pClientData, void* pBuffer, tmsize_t Size)
    {
        auto* pThis   = reinterpret_cast<TIFFClientOpenWrapper*>(pClientData);
        auto* pSrcPtr = reinterpret_cast<Uint8*>(pThis->m_pData->GetDataPtr()) + pThis->m_Offset;
        memcpy(pBuffer, pSrcPtr, Size);
        pThis->m_Offset += Size;
        return Size;
    }

    static tmsize_t TIFFWriteProc(thandle_t pClientData, void* pBuffer, tmsize_t Size)
    {
        auto* pThis = reinterpret_cast<TIFFClientOpenWrapper*>(pClientData);
        if (pThis->m_Offset + Size > pThis->m_Size)
        {
            pThis->m_Size = pThis->m_Offset + Size;
            pThis->m_pData->Resize(pThis->m_Size);
        }
        auto* pDstPtr = reinterpret_cast<Uint8*>(pThis->m_pData->GetDataPtr()) + pThis->m_Offset;
        memcpy(pDstPtr, pBuffer, Size);
        pThis->m_Offset += Size;
        return Size;
    }

    static toff_t TIFFSeekProc(thandle_t pClientData, toff_t Offset, int Whence)
    {
        auto* pThis = reinterpret_cast<TIFFClientOpenWrapper*>(pClientData);
        switch (Whence)
        {
            case SEEK_SET: pThis->m_Offset = static_cast<size_t>(Offset); break;
            case SEEK_CUR: pThis->m_Offset += static_cast<size_t>(Offset); break;
            case SEEK_END: pThis->m_Offset = pThis->m_Size + static_cast<size_t>(Offset); break;
            default: UNEXPECTED("Unexpected whence");
        }

        return pThis->m_Offset;
    }

    static int TIFFCloseProc(thandle_t pClientData)
    {
        auto* pThis = reinterpret_cast<TIFFClientOpenWrapper*>(pClientData);
        pThis->m_pData.Release();
        pThis->m_Size   = 0;
        pThis->m_Offset = 0;
        return 0;
    }

    static toff_t TIFFSizeProc(thandle_t pClientData)
    {
        auto* pThis = reinterpret_cast<TIFFClientOpenWrapper*>(pClientData);
        return pThis->m_Size;
    }

    static int TIFFMapFileProc(thandle_t pClientData, void** base, toff_t* size)
    {
        UNEXPECTED("Client file mapping is not implemented. Use \'m\' when opening TIFF file to disable file mapping.");
        return 0;
    }

    static void TIFFUnmapFileProc(thandle_t pClientData, void* base, toff_t size)
    {
        UNEXPECTED("Client file mapping is not implemented. Use \'m\' when opening TIFF file to disable file mapping.");
    }

private:
    size_t                   m_Offset;
    size_t                   m_Size;
    RefCntAutoPtr<IDataBlob> m_pData;
};

void Image::LoadTiffFile(IDataBlob* pFileData, const ImageLoadInfo& LoadInfo)
{
    TIFFClientOpenWrapper TiffClientOpenWrpr(pFileData);

    auto TiffFile = TIFFClientOpen("", "rm", &TiffClientOpenWrpr,
                                   TIFFClientOpenWrapper::TIFFReadProc,
                                   TIFFClientOpenWrapper::TIFFWriteProc,
                                   TIFFClientOpenWrapper::TIFFSeekProc,
                                   TIFFClientOpenWrapper::TIFFCloseProc,
                                   TIFFClientOpenWrapper::TIFFSizeProc,
                                   TIFFClientOpenWrapper::TIFFMapFileProc,
                                   TIFFClientOpenWrapper::TIFFUnmapFileProc);

    TIFFGetField(TiffFile, TIFFTAG_IMAGEWIDTH, &m_Desc.Width);
    TIFFGetField(TiffFile, TIFFTAG_IMAGELENGTH, &m_Desc.Height);

    Uint16 SamplesPerPixel = 0;
    // SamplesPerPixel is usually 1 for bilevel, grayscale, and palette-color images.
    // SamplesPerPixel is usually 3 for RGB images. If this value is higher, ExtraSamples
    // should give an indication of the meaning of the additional channels.
    TIFFGetField(TiffFile, TIFFTAG_SAMPLESPERPIXEL, &SamplesPerPixel);
    m_Desc.NumComponents = SamplesPerPixel;

    Uint16 BitsPerSample = 0;
    TIFFGetField(TiffFile, TIFFTAG_BITSPERSAMPLE, &BitsPerSample);

    Uint16 SampleFormat = 0;
    TIFFGetField(TiffFile, TIFFTAG_SAMPLEFORMAT, &SampleFormat);
    if (SampleFormat == 0)
        SampleFormat = SAMPLEFORMAT_UINT;

    switch (SampleFormat)
    {
        case SAMPLEFORMAT_UINT:
            switch (BitsPerSample)
            {
                case 8: m_Desc.ComponentType = VT_UINT8; break;
                case 16: m_Desc.ComponentType = VT_UINT16; break;
                case 32: m_Desc.ComponentType = VT_UINT32; break;
                default: LOG_ERROR_AND_THROW(BitsPerSample, " is not a valid UINT component bit depth. Only 8, 16 and 32 are allowed");
            }
            break;

        case SAMPLEFORMAT_INT:
            switch (BitsPerSample)
            {
                case 8: m_Desc.ComponentType = VT_INT8; break;
                case 16: m_Desc.ComponentType = VT_INT16; break;
                case 32: m_Desc.ComponentType = VT_INT32; break;
                default: LOG_ERROR_AND_THROW(BitsPerSample, " is not a valid INT component bit depth. Only 8, 16 and 32 are allowed");
            }
            break;

        case SAMPLEFORMAT_IEEEFP:
            switch (BitsPerSample)
            {
                case 16: m_Desc.ComponentType = VT_FLOAT16; break;
                case 32: m_Desc.ComponentType = VT_FLOAT32; break;
                default: LOG_ERROR_AND_THROW(BitsPerSample, " is not a valid FLOAT component bit depth. Only 16 and 32 are allowed");
            }
            break;

        case SAMPLEFORMAT_VOID:
            LOG_ERROR_AND_THROW("Untyped tif images are not supported");
            break;

        case SAMPLEFORMAT_COMPLEXINT:
            LOG_ERROR_AND_THROW("Complex int tif images are not supported");
            break;

        case SAMPLEFORMAT_COMPLEXIEEEFP:
            LOG_ERROR_AND_THROW("Complex floating point tif images are not supported");
            break;

        default:
            LOG_ERROR_AND_THROW("Unknown sample format: ", Uint32{SampleFormat});
    }

    size_t ScanlineSize = TIFFScanlineSize(TiffFile);
    m_Desc.RowStride    = AlignUp(m_Desc.Width * m_Desc.NumComponents * (BitsPerSample / 8), 4u);
    m_pData->Resize(size_t{m_Desc.Height} * size_t{m_Desc.RowStride});

    Uint16 PlanarConfig = 0;
    TIFFGetField(TiffFile, TIFFTAG_PLANARCONFIG, &PlanarConfig);
    if (PlanarConfig == PLANARCONFIG_CONTIG || m_Desc.NumComponents == 1)
    {
        VERIFY_EXPR(m_Desc.RowStride >= ScanlineSize);
        auto* pDataPtr = reinterpret_cast<Uint8*>(m_pData->GetDataPtr());
        for (Uint32 row = 0; row < m_Desc.Height; row++, pDataPtr += m_Desc.RowStride)
        {
            TIFFReadScanline(TiffFile, pDataPtr, row);
        }
    }
    else if (PlanarConfig == PLANARCONFIG_SEPARATE)
    {
        std::vector<Uint8> ScanlineData(ScanlineSize);
        for (Uint32 row = 0; row < m_Desc.Height; ++row)
        {
            for (Uint16 comp = 0; comp < m_Desc.NumComponents; ++comp)
            {
                auto* const pDstRow = reinterpret_cast<Uint8*>(m_pData->GetDataPtr()) + m_Desc.RowStride * row + comp;

                TIFFReadScanline(TiffFile, ScanlineData.data(), row, comp);

                auto CopyComponet = [Width = m_Desc.Width, NumComp = m_Desc.NumComponents](const auto* pSrc, auto* pDst) {
                    for (Uint32 x = 0; x < Width; ++x)
                    {
                        pDst[x * NumComp] = pSrc[x];
                    }
                };

                switch (BitsPerSample)
                {
                    case 8:
                        CopyComponet(reinterpret_cast<const Uint8*>(ScanlineData.data()), reinterpret_cast<Uint8*>(pDstRow));
                        break;

                    case 16:
                        CopyComponet(reinterpret_cast<const Uint16*>(ScanlineData.data()), reinterpret_cast<Uint16*>(pDstRow));
                        break;

                    case 32:
                        CopyComponet(reinterpret_cast<const Uint32*>(ScanlineData.data()), reinterpret_cast<Uint32*>(pDstRow));
                        break;

                    default:
                        UNEXPECTED("Unexpected component bit depth (", BitsPerSample, ").");
                }
            }
        }
    }
    else
    {
        UNEXPECTED("Unexpected planar configuration (", PlanarConfig, ").");
    }
    TIFFClose(TiffFile);
}


Image::Image(IReferenceCounters*  pRefCounters,
             IDataBlob*           pFileData,
             const ImageLoadInfo& LoadInfo) :
    TBase{pRefCounters},
    m_pData{DataBlobImpl::Create()}
{
    if (LoadInfo.Format == IMAGE_FILE_FORMAT_TIFF)
    {
        LoadTiffFile(pFileData, LoadInfo);
    }
    else if (LoadInfo.Format == IMAGE_FILE_FORMAT_PNG)
    {
        auto Res = DecodePng(pFileData, m_pData.RawPtr(), &m_Desc);
        if (Res != DECODE_PNG_RESULT_OK)
            LOG_ERROR_MESSAGE("Failed to decode png image");
    }
    else if (LoadInfo.Format == IMAGE_FILE_FORMAT_JPEG)
    {
        auto Res = DecodeJpeg(pFileData, m_pData.RawPtr(), &m_Desc);
        if (Res != DECODE_JPEG_RESULT_OK)
            LOG_ERROR_MESSAGE("Failed to decode jpeg image");
    }
    else if (LoadInfo.Format == IMAGE_FILE_FORMAT_SGI)
    {
        auto Res = LoadSGI(pFileData, m_pData.RawPtr(), &m_Desc);
        if (!Res)
            LOG_ERROR_MESSAGE("Failed to load SGI image");
    }
    else if (LoadInfo.Format == IMAGE_FILE_FORMAT_DDS)
    {
        LOG_ERROR_MESSAGE("An image can't be created from DDS file. Use CreateTextureFromFile() or CreateTextureFromDDS() functions.");
    }
    else if (LoadInfo.Format == IMAGE_FILE_FORMAT_KTX)
    {
        LOG_ERROR_MESSAGE("An image can't be created from KTX file. Use CreateTextureFromFile() or CreateTextureFromKTX() functions.");
    }
    else
    {
        LOG_ERROR_MESSAGE("Unknown image format.");
    }

    if (LoadInfo.PermultiplyAlpha && m_Desc.NumComponents == 4)
    {
        PremultiplyAlphaAttribs PremultAttribs;
        PremultAttribs.Width          = m_Desc.Width;
        PremultAttribs.Height         = m_Desc.Height;
        PremultAttribs.ComponentType  = m_Desc.ComponentType;
        PremultAttribs.ComponentCount = m_Desc.NumComponents;
        PremultAttribs.Stride         = m_Desc.RowStride;
        PremultAttribs.pPixels        = m_pData->GetDataPtr();
        PremultAttribs.IsSRGB         = LoadInfo.IsSRGB;
        PremultiplyAlpha(PremultAttribs);
    }
}

void Image::CreateFromDataBlob(IDataBlob*           pFileData,
                               const ImageLoadInfo& LoadInfo,
                               Image**              ppImage)
{
    *ppImage = MakeNewRCObj<Image>()(pFileData, LoadInfo);
    (*ppImage)->AddRef();
}

Image::Image(IReferenceCounters* pRefCounters,
             const ImageDesc&    Desc,
             IDataBlob*          pPixels) :
    TBase{pRefCounters},
    m_Desc{Desc},
    m_pData{pPixels}
{
}

void Image::CreateFromMemory(const ImageDesc& Desc,
                             IDataBlob*       pPixels,
                             Image**          ppImage)
{
    *ppImage = MakeNewRCObj<Image>()(Desc, pPixels);
    (*ppImage)->AddRef();
}



static const std::array<Uint8, 4> GetRGBAOffsets(TEXTURE_FORMAT Format)
{
    switch (Format)
    {
        case TEX_FORMAT_BGRA8_TYPELESS:
        case TEX_FORMAT_BGRA8_UNORM:
        case TEX_FORMAT_BGRA8_UNORM_SRGB:
            return {{2, 1, 0, 3}};
        default:
            return {{0, 1, 2, 3}};
    }
}

std::vector<Uint8> Image::ConvertImageData(Uint32         Width,
                                           Uint32         Height,
                                           const Uint8*   pData,
                                           Uint32         Stride,
                                           TEXTURE_FORMAT SrcFormat,
                                           TEXTURE_FORMAT DstFormat,
                                           bool           KeepAlpha)
{
    const auto& SrcFmtAttribs = GetTextureFormatAttribs(SrcFormat);
    const auto& DstFmtAttribs = GetTextureFormatAttribs(DstFormat);
    VERIFY(SrcFmtAttribs.ComponentSize == 1, "Only 8-bit formats are currently supported");
    VERIFY(DstFmtAttribs.ComponentSize == 1, "Only 8-bit formats are currently supported");

    auto NumDstComponents = SrcFmtAttribs.NumComponents;
    if (!KeepAlpha)
        NumDstComponents = std::min(NumDstComponents, Uint8{3});

    auto SrcOffsets = GetRGBAOffsets(SrcFormat);
    auto DstOffsets = GetRGBAOffsets(DstFormat);

    std::vector<Uint8> ConvertedData(size_t{DstFmtAttribs.ComponentSize} * size_t{NumDstComponents} * Width * Height);

    for (size_t j = 0; j < Height; ++j)
    {
        for (size_t i = 0; i < Width; ++i)
        {
            for (Uint32 c = 0; c < NumDstComponents; ++c)
            {
                ConvertedData[j * Width * NumDstComponents + i * NumDstComponents + DstOffsets[c]] =
                    pData[j * Stride + i * SrcFmtAttribs.NumComponents + SrcOffsets[c]];
            }
        }
    }

    return ConvertedData;
}


void Image::Encode(const EncodeInfo& Info, IDataBlob** ppEncodedData)
{
    auto pEncodedData = DataBlobImpl::Create();
    if (Info.FileFormat == IMAGE_FILE_FORMAT_JPEG)
    {
        auto RGBData = ConvertImageData(Info.Width, Info.Height, reinterpret_cast<const Uint8*>(Info.pData), Info.Stride, Info.TexFormat, TEX_FORMAT_RGBA8_UNORM, false);

        auto Res = EncodeJpeg(RGBData.data(), Info.Width, Info.Height, Info.JpegQuality, pEncodedData.RawPtr());
        if (Res != ENCODE_JPEG_RESULT_OK)
            LOG_ERROR_MESSAGE("Failed to encode jpeg file");
    }
    else if (Info.FileFormat == IMAGE_FILE_FORMAT_PNG)
    {
        const auto*        pData  = reinterpret_cast<const Uint8*>(Info.pData);
        auto               Stride = Info.Stride;
        std::vector<Uint8> ConvertedData;
        if (!((Info.TexFormat == TEX_FORMAT_RGBA8_UNORM || Info.TexFormat == TEX_FORMAT_RGBA8_UNORM_SRGB) && Info.KeepAlpha))
        {
            ConvertedData = ConvertImageData(Info.Width, Info.Height, reinterpret_cast<const Uint8*>(Info.pData), Info.Stride, Info.TexFormat, TEX_FORMAT_RGBA8_UNORM, Info.KeepAlpha);
            pData         = ConvertedData.data();
            Stride        = Info.Width * (Info.KeepAlpha ? 4 : 3);
        }

        auto Res = EncodePng(pData, Info.Width, Info.Height, Stride, Info.KeepAlpha ? PNG_COLOR_TYPE_RGBA : PNG_COLOR_TYPE_RGB, pEncodedData.RawPtr());
        if (Res != ENCODE_PNG_RESULT_OK)
            LOG_ERROR_MESSAGE("Failed to encode png file");
    }
    else
    {
        UNSUPPORTED("Unsupported image file format");
    }
    pEncodedData->QueryInterface(IID_DataBlob, reinterpret_cast<IObject**>(ppEncodedData));
}

IMAGE_FILE_FORMAT Image::GetFileFormat(const Uint8* pData, size_t Size, const char* FilePath)
{
    if (pData != nullptr)
    {
        if (Size >= 3 && pData[0] == 0xFF && pData[1] == 0xD8 && pData[2] == 0xFF)
            return IMAGE_FILE_FORMAT_JPEG;

        if (Size >= 8 &&
            pData[0] == 0x89 && pData[1] == 0x50 && pData[2] == 0x4E && pData[3] == 0x47 &&
            pData[4] == 0x0D && pData[5] == 0x0A && pData[6] == 0x1A && pData[7] == 0x0A)
            return IMAGE_FILE_FORMAT_PNG;

        if (Size >= 4 &&
            ((pData[0] == 0x49 && pData[1] == 0x20 && pData[2] == 0x49) ||
             (pData[0] == 0x49 && pData[1] == 0x49 && pData[2] == 0x2A && pData[3] == 0x00) ||
             (pData[0] == 0x4D && pData[1] == 0x4D && pData[2] == 0x00 && pData[3] == 0x2A) ||
             (pData[0] == 0x4D && pData[1] == 0x4D && pData[2] == 0x00 && pData[3] == 0x2B)))
            return IMAGE_FILE_FORMAT_TIFF;

        if (Size >= 4 && pData[0] == 0x44 && pData[1] == 0x44 && pData[2] == 0x53 && pData[3] == 0x20)
            return IMAGE_FILE_FORMAT_DDS;

        static constexpr Uint8 KTX10FileIdentifier[12] = {0xAB, 0x4B, 0x54, 0x58, 0x20, 0x31, 0x31, 0xBB, 0x0D, 0x0A, 0x1A, 0x0A};
        static constexpr Uint8 KTX20FileIdentifier[12] = {0xAB, 0x4B, 0x54, 0x58, 0x20, 0x32, 0x30, 0xBB, 0x0D, 0x0A, 0x1A, 0x0A};
        if (Size >= 12 &&
            (memcmp(pData, KTX10FileIdentifier, sizeof(KTX10FileIdentifier)) == 0 ||
             memcmp(pData, KTX20FileIdentifier, sizeof(KTX20FileIdentifier)) == 0))
            return IMAGE_FILE_FORMAT_KTX;

        if (Size >= 2 && pData[0] == 0x01 && pData[1] == 0xDA)
            return IMAGE_FILE_FORMAT_SGI;
    }

    if (FilePath != nullptr)
    {
        // Try to use extension to derive format
        auto* pDotPos = strrchr(FilePath, '.');
        if (pDotPos == nullptr)
        {
            LOG_ERROR_MESSAGE("Unable to recognize file format: file name '", FilePath, "' does not contain extension");
            return IMAGE_FILE_FORMAT_UNKNOWN;
        }

        auto* pExtension = pDotPos + 1;
        if (*pExtension == 0)
        {
            LOG_ERROR_MESSAGE("Unable to recognize file format: file name '", FilePath, "' contain empty extension");
            return IMAGE_FILE_FORMAT_UNKNOWN;
        }

        String Extension = StrToLower(pExtension);
        if (Extension == "png")
            return IMAGE_FILE_FORMAT_PNG;
        else if (Extension == "jpeg" || Extension == "jpg")
            return IMAGE_FILE_FORMAT_JPEG;
        else if (Extension == "tiff" || Extension == "tif")
            return IMAGE_FILE_FORMAT_TIFF;
        else if (Extension == "dds")
            return IMAGE_FILE_FORMAT_DDS;
        else if (Extension == "ktx")
            return IMAGE_FILE_FORMAT_KTX;
        else if (Extension == "sgi" || Extension == "rgb" || Extension == "rgba" || Extension == "bw" || Extension == "int" || Extension == "inta")
            return IMAGE_FILE_FORMAT_SGI;
        else
            LOG_ERROR_MESSAGE("Unrecognized image file extension", Extension);
    }

    return IMAGE_FILE_FORMAT_UNKNOWN;
}


IMAGE_FILE_FORMAT CreateImageFromFile(const Char* FilePath,
                                      Image**     ppImage,
                                      IDataBlob** ppRawData)
{
    auto ImgFileFormat = IMAGE_FILE_FORMAT_UNKNOWN;
    try
    {
        RefCntAutoPtr<BasicFileStream> pFileStream{MakeNewRCObj<BasicFileStream>()(FilePath, EFileAccessMode::Read)};
        if (!pFileStream->IsValid())
            LOG_ERROR_AND_THROW("Failed to open image file \"", FilePath, '\"');

        auto pFileData = DataBlobImpl::Create();
        pFileStream->ReadBlob(pFileData);

        ImgFileFormat = Image::GetFileFormat(reinterpret_cast<Uint8*>(pFileData->GetDataPtr()), pFileData->GetSize(), FilePath);
        if (ImgFileFormat == IMAGE_FILE_FORMAT_UNKNOWN)
        {
            LOG_ERROR_AND_THROW("Unable to derive image format for file '", FilePath, "\".");
        }

        if (ImgFileFormat == IMAGE_FILE_FORMAT_PNG ||
            ImgFileFormat == IMAGE_FILE_FORMAT_JPEG ||
            ImgFileFormat == IMAGE_FILE_FORMAT_TIFF ||
            ImgFileFormat == IMAGE_FILE_FORMAT_SGI)
        {
            ImageLoadInfo ImgLoadInfo;
            ImgLoadInfo.Format = ImgFileFormat;
            Image::CreateFromDataBlob(pFileData, ImgLoadInfo, ppImage);
        }
        else if (ppRawData != nullptr)
        {
            *ppRawData = pFileData.Detach();
        }
    }
    catch (std::runtime_error& err)
    {
        LOG_ERROR("Failed to create image from file: ", err.what());
    }

    return ImgFileFormat;
}

} // namespace Diligent
