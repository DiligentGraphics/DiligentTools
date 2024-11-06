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
#include "ProxyDataBlob.hpp"
#include "DebugUtilities.hpp"
#include "RefCntAutoPtr.hpp"
#include "Align.hpp"
#include "GraphicsAccessories.hpp"
#include "BasicFileStream.hpp"
#include "StringTools.hpp"
#include "TextureUtilities.h"

#ifdef __clang__
#    pragma clang diagnostic push
#    pragma clang diagnostic ignored "-Wunused-function"
#endif
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_STATIC
#define STBI_ONLY_HDR
#define STBI_ONLY_TGA
#include "../../ThirdParty/stb/stb_image.h"
#ifdef __clang__
#    pragma clang diagnostic pop
#endif

namespace Diligent
{

class TIFFClientOpenWrapper
{
public:
    explicit TIFFClientOpenWrapper(const IDataBlob* pData) noexcept :
        m_Offset{0},
        m_Size{pData->GetSize()},
        m_pData{pData}
    {
    }

    static tmsize_t TIFFReadProc(thandle_t pClientData, void* pBuffer, tmsize_t Size)
    {
        TIFFClientOpenWrapper* pThis   = static_cast<TIFFClientOpenWrapper*>(pClientData);
        const void*            pSrcPtr = pThis->m_pData->GetConstDataPtr(pThis->m_Offset);
        memcpy(pBuffer, pSrcPtr, Size);
        pThis->m_Offset += Size;
        return Size;
    }

    static tmsize_t TIFFWriteProc(thandle_t pClientData, void* pBuffer, tmsize_t Size)
    {
#if 0
        TIFFClientOpenWrapper* pThis = static_cast<TIFFClientOpenWrapper*>(pClientData);
        if (pThis->m_Offset + Size > pThis->m_Size)
        {
            pThis->m_Size = pThis->m_Offset + Size;
            pThis->m_pData->Resize(pThis->m_Size);
        }
        auto* pDstPtr = pThis->m_pData->GetDataPtr(pThis->m_Offset);
        memcpy(pDstPtr, pBuffer, Size);
        pThis->m_Offset += Size;
        return Size;
#endif
        UNSUPPORTED("TIFF write is not supported");
        return 0;
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
        auto* pThis     = reinterpret_cast<TIFFClientOpenWrapper*>(pClientData);
        pThis->m_pData  = nullptr;
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
    size_t           m_Offset = 0;
    size_t           m_Size   = 0;
    const IDataBlob* m_pData  = nullptr;
};

void Image::LoadTiffFile(const IDataBlob* pFileData, const ImageLoadInfo& LoadInfo)
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
        Uint8* pDataPtr = m_pData->GetDataPtr<Uint8>();
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
                Uint8* const pDstRow = m_pData->GetDataPtr<Uint8>() + m_Desc.RowStride * row + comp;

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


static bool LoadImageSTB(const void* pSrcImage,
                         size_t      ImageSize,
                         VALUE_TYPE  ComponentType,
                         IDataBlob*  pDstPixels,
                         ImageDesc*  pDstImgDesc)
{
    int   Width         = 0;
    int   Height        = 0;
    int   NumComponents = 0;
    void* pDecodedData  = nullptr;
    if (pDstPixels != nullptr)
    {
        switch (ComponentType)
        {
            case VT_FLOAT32:
                pDecodedData = stbi_loadf_from_memory(static_cast<const stbi_uc*>(pSrcImage), static_cast<int>(ImageSize), &Width, &Height, &NumComponents, 0);
                break;

            case VT_UINT8:
            case VT_INT8:
                pDecodedData = stbi_load_from_memory(static_cast<const stbi_uc*>(pSrcImage), static_cast<int>(ImageSize), &Width, &Height, &NumComponents, 0);
                break;

            case VT_UINT16:
            case VT_INT16:
                pDecodedData = stbi_load_16_from_memory(static_cast<const stbi_uc*>(pSrcImage), static_cast<int>(ImageSize), &Width, &Height, &NumComponents, 0);
                break;

            default:
                UNEXPECTED("Unexpected component type");
        }

        if (pDecodedData == nullptr)
        {
            return false;
        }
    }
    else
    {
        if (!stbi_info_from_memory(static_cast<const stbi_uc*>(pSrcImage), static_cast<int>(ImageSize), &Width, &Height, &NumComponents))
        {
            return false;
        }
    }

    pDstImgDesc->ComponentType = ComponentType;
    pDstImgDesc->Width         = static_cast<Uint32>(Width);
    pDstImgDesc->Height        = static_cast<Uint32>(Height);
    pDstImgDesc->NumComponents = NumComponents;

    if (pDstPixels != nullptr)
    {
        pDstImgDesc->RowStride = pDstImgDesc->Width * pDstImgDesc->NumComponents * GetValueSize(ComponentType);
        pDstPixels->Resize(pDstImgDesc->Height * pDstImgDesc->RowStride);
        memcpy(pDstPixels->GetDataPtr(), pDecodedData, pDstImgDesc->Height * pDstImgDesc->RowStride);
        stbi_image_free(pDecodedData);
    }

    return true;
}

Image::Image(IReferenceCounters*  pRefCounters,
             const IDataBlob*     pFileData,
             const ImageLoadInfo& LoadInfo) :
    TBase{pRefCounters},
    m_pData{DataBlobImpl::Create(LoadInfo.pAllocator)}
{
    if (LoadInfo.Format == IMAGE_FILE_FORMAT_TIFF)
    {
        LoadTiffFile(pFileData, LoadInfo);
    }
    else if (LoadInfo.Format == IMAGE_FILE_FORMAT_HDR)
    {
        if (!LoadImageSTB(pFileData->GetConstDataPtr(), pFileData->GetSize(), VT_FLOAT32, m_pData, &m_Desc))
        {
            LOG_ERROR_MESSAGE("Failed to load HDR image from memory. STB only supports 32-bit rle rgbe textures");
            return;
        }
    }
    else if (LoadInfo.Format == IMAGE_FILE_FORMAT_TGA)
    {
        if (!LoadImageSTB(pFileData->GetConstDataPtr(), pFileData->GetSize(), VT_UINT8, m_pData, &m_Desc))
        {
            LOG_ERROR_MESSAGE("Failed to load TGA image");
            return;
        }
    }
    else if (LoadInfo.Format == IMAGE_FILE_FORMAT_PNG)
    {
        if (DecodePng(pFileData->GetConstDataPtr(), pFileData->GetSize(), m_pData, &m_Desc) != DECODE_PNG_RESULT_OK)
        {
            LOG_ERROR_MESSAGE("Failed to decode png image");
            return;
        }
    }
    else if (LoadInfo.Format == IMAGE_FILE_FORMAT_JPEG)
    {
        if (DecodeJpeg(pFileData->GetConstDataPtr(), pFileData->GetSize(), m_pData, &m_Desc) != DECODE_JPEG_RESULT_OK)
        {
            LOG_ERROR_MESSAGE("Failed to decode jpeg image");
            return;
        }
    }
    else if (LoadInfo.Format == IMAGE_FILE_FORMAT_SGI)
    {
        if (!LoadSGI(pFileData, m_pData, &m_Desc))
        {
            LOG_ERROR_MESSAGE("Failed to load SGI image");
            return;
        }
    }
    else if (LoadInfo.Format == IMAGE_FILE_FORMAT_DDS)
    {
        LOG_ERROR_MESSAGE("An image can't be created from DDS file. Use CreateTextureFromFile() or CreateTextureFromDDS() functions.");
        return;
    }
    else if (LoadInfo.Format == IMAGE_FILE_FORMAT_KTX)
    {
        LOG_ERROR_MESSAGE("An image can't be created from KTX file. Use CreateTextureFromFile() or CreateTextureFromKTX() functions.");
        return;
    }
    else
    {
        LOG_ERROR_MESSAGE("Unknown image format.");
        return;
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

void Image::CreateFromDataBlob(const IDataBlob*     pFileData,
                               const ImageLoadInfo& LoadInfo,
                               Image**              ppImage)
{
    *ppImage = MakeNewRCObj<Image>()(pFileData, LoadInfo);
    (*ppImage)->AddRef();
}

Image::Image(IReferenceCounters*      pRefCounters,
             const ImageDesc&         Desc,
             RefCntAutoPtr<IDataBlob> pPixels) :
    TBase{pRefCounters},
    m_Desc{Desc},
    m_pData{std::move(pPixels)}
{
}

void Image::CreateFromMemory(const ImageDesc&         Desc,
                             RefCntAutoPtr<IDataBlob> pPixels,
                             Image**                  ppImage)
{
    *ppImage = MakeNewRCObj<Image>()(Desc, std::move(pPixels));
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
                                           bool           KeepAlpha,
                                           bool           FlipY)
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
        size_t SrcJ = FlipY ? Height - 1 - j : j;
        for (size_t i = 0; i < Width; ++i)
        {
            for (Uint32 c = 0; c < NumDstComponents; ++c)
            {
                ConvertedData[j * Width * NumDstComponents + i * NumDstComponents + DstOffsets[c]] =
                    pData[SrcJ * Stride + i * SrcFmtAttribs.NumComponents + SrcOffsets[c]];
            }
        }
    }

    return ConvertedData;
}


void Image::Encode(const EncodeInfo& Info, IDataBlob** ppEncodedData)
{
    auto pEncodedData = DataBlobImpl::Create(Info.pAllocator);
    if (Info.FileFormat == IMAGE_FILE_FORMAT_JPEG)
    {
        auto RGBData = ConvertImageData(Info.Width, Info.Height, reinterpret_cast<const Uint8*>(Info.pData), Info.Stride, Info.TexFormat, TEX_FORMAT_RGBA8_UNORM, false, Info.FlipY);

        auto Res = EncodeJpeg(RGBData.data(), Info.Width, Info.Height, Info.JpegQuality, pEncodedData);
        if (Res != ENCODE_JPEG_RESULT_OK)
            LOG_ERROR_MESSAGE("Failed to encode jpeg file");
    }
    else if (Info.FileFormat == IMAGE_FILE_FORMAT_PNG)
    {
        const auto*        pData  = reinterpret_cast<const Uint8*>(Info.pData);
        auto               Stride = Info.Stride;
        std::vector<Uint8> ConvertedData;
        if (!((Info.TexFormat == TEX_FORMAT_RGBA8_UNORM || Info.TexFormat == TEX_FORMAT_RGBA8_UNORM_SRGB) && Info.KeepAlpha && !Info.FlipY))
        {
            ConvertedData = ConvertImageData(Info.Width, Info.Height, reinterpret_cast<const Uint8*>(Info.pData), Info.Stride, Info.TexFormat, TEX_FORMAT_RGBA8_UNORM, Info.KeepAlpha, Info.FlipY);
            pData         = ConvertedData.data();
            Stride        = Info.Width * (Info.KeepAlpha ? 4 : 3);
        }

        auto Res = EncodePng(pData, Info.Width, Info.Height, Stride, Info.KeepAlpha ? PNG_COLOR_TYPE_RGBA : PNG_COLOR_TYPE_RGB, pEncodedData);
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

        static constexpr Uint8 HDRFileIdentifier[11] = {0x23, 0x3F, 0x52, 0x41, 0x44, 0x49, 0x41, 0x4E, 0x43, 0x45, 0x0A};
        if (Size >= 11 && memcmp(pData, HDRFileIdentifier, sizeof(HDRFileIdentifier)) == 0)
            return IMAGE_FILE_FORMAT_HDR;

        if (Size >= 3 && pData[0] == 0x00 && pData[1] == 0x00 && pData[2] == 0x02)
            return IMAGE_FILE_FORMAT_TGA;

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
        else if (Extension == "hdr")
            return IMAGE_FILE_FORMAT_HDR;
        else if (Extension == "tga")
            return IMAGE_FILE_FORMAT_TGA;
        else
            LOG_ERROR_MESSAGE("Unrecognized image file extension", Extension);
    }

    return IMAGE_FILE_FORMAT_UNKNOWN;
}

template <typename T>
bool IsImageUniform(const void* pData, Uint32 Width, Uint32 Height, Uint32 NumComponents, Uint32 RowStride)
{
    if (Width == 0 || Height == 0 || NumComponents == 0)
        return false;

    const T* pFirstPixel = static_cast<const T*>(pData);
    for (Uint32 Pass = 0; Pass < 2; ++Pass)
    {
        // On the first pass, sparsely sample the image to quickly detect non-uniform images
        const Uint32 Step = (Pass == 0) ? 32 : 1;
        for (Uint32 y = 0; y < Height; y += Step)
        {
            const T* pRow = reinterpret_cast<const T*>(static_cast<const Uint8*>(pData) + y * RowStride);
            for (Uint32 x = 0; x < Width; x += Step)
            {
                for (Uint32 c = 0; c < NumComponents; ++c)
                {
                    if (pRow[x * NumComponents + c] != pFirstPixel[c])
                        return false;
                }
            }
        }
    }

    return true;
}

bool Image::IsUniform() const
{
    if (!m_pData)
        return false;

    Uint32 ComponentSize = GetValueSize(m_Desc.ComponentType);
    switch (ComponentSize)
    {
        case 1:
            return IsImageUniform<Uint8>(m_pData->GetConstDataPtr(), m_Desc.Width, m_Desc.Height, m_Desc.NumComponents, m_Desc.RowStride);

        case 2:
            return IsImageUniform<Uint16>(m_pData->GetConstDataPtr(), m_Desc.Width, m_Desc.Height, m_Desc.NumComponents, m_Desc.RowStride);

        case 4:
            return IsImageUniform<Uint32>(m_pData->GetConstDataPtr(), m_Desc.Width, m_Desc.Height, m_Desc.NumComponents, m_Desc.RowStride);

        case 8:
            return IsImageUniform<Uint64>(m_pData->GetConstDataPtr(), m_Desc.Width, m_Desc.Height, m_Desc.NumComponents, m_Desc.RowStride);

        default:
            UNEXPECTED("Unexpected component size (", ComponentSize, ")");
            return false;
    }
}

IMAGE_FILE_FORMAT CreateImageFromFile(const Char* FilePath,
                                      Image**     ppImage,
                                      IDataBlob** ppRawData)
{
    IMAGE_FILE_FORMAT ImgFileFormat = IMAGE_FILE_FORMAT_UNKNOWN;
    try
    {
        RefCntAutoPtr<BasicFileStream> pFileStream{MakeNewRCObj<BasicFileStream>()(FilePath, EFileAccessMode::Read)};
        if (!pFileStream->IsValid())
            LOG_ERROR_AND_THROW("Failed to open image file \"", FilePath, '\"');

        auto pFileData = DataBlobImpl::Create();
        pFileStream->ReadBlob(pFileData);

        ImgFileFormat = Image::GetFileFormat(pFileData->GetConstDataPtr<Uint8>(), pFileData->GetSize(), FilePath);
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

IMAGE_FILE_FORMAT CreateImageFromMemory(const void* pImageData,
                                        size_t      DataSize,
                                        Image**     ppImage)
{
    IMAGE_FILE_FORMAT ImgFileFormat = IMAGE_FILE_FORMAT_UNKNOWN;
    try
    {
        ImgFileFormat = Image::GetFileFormat(static_cast<const Uint8*>(pImageData), DataSize);
        if (ImgFileFormat == IMAGE_FILE_FORMAT_UNKNOWN)
        {
            LOG_ERROR_AND_THROW("Unable to derive image format");
        }

        if (ImgFileFormat == IMAGE_FILE_FORMAT_PNG ||
            ImgFileFormat == IMAGE_FILE_FORMAT_JPEG ||
            ImgFileFormat == IMAGE_FILE_FORMAT_TIFF ||
            ImgFileFormat == IMAGE_FILE_FORMAT_SGI)
        {
            ImageLoadInfo ImgLoadInfo;
            ImgLoadInfo.Format = ImgFileFormat;
            RefCntAutoPtr<IDataBlob> pImageDataBlob{ProxyDataBlob::Create(pImageData, DataSize)};
            Image::CreateFromDataBlob(pImageDataBlob, ImgLoadInfo, ppImage);
        }
    }
    catch (std::runtime_error& err)
    {
        LOG_ERROR("Failed to create image from file: ", err.what());
    }

    return ImgFileFormat;
}

} // namespace Diligent
