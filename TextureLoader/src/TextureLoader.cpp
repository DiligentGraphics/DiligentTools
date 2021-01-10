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

#include "pch.h"
#include <algorithm>
#include <limits>
#include <math.h>
#include <vector>

#include "TextureLoader.h"
#include "GraphicsAccessories.hpp"
#include "GraphicsUtilities.h"
#include "DDSLoader.h"
#include "PNGCodec.h"
#include "JPEGCodec.h"
#include "ColorConversion.h"
#include "Image.h"

extern "C"
{
    Diligent::DECODE_PNG_RESULT Diligent_DecodePng(Diligent::IDataBlob* pSrcPngBits,
                                                   Diligent::IDataBlob* pDstPixels,
                                                   Diligent::ImageDesc* pDstImgDesc);

    Diligent::ENCODE_PNG_RESULT Diligent_EncodePng(const Diligent::Uint8* pSrcPixels,
                                                   Diligent::Uint32       Width,
                                                   Diligent::Uint32       Height,
                                                   Diligent::Uint32       StrideInBytes,
                                                   int                    PngColorType,
                                                   Diligent::IDataBlob*   pDstPngBits);

    Diligent::DECODE_JPEG_RESULT Diligent_DecodeJpeg(Diligent::IDataBlob* pSrcJpegBits,
                                                     Diligent::IDataBlob* pDstPixels,
                                                     Diligent::ImageDesc* pDstImgDesc);

    Diligent::ENCODE_JPEG_RESULT Diligent_EncodeJpeg(Diligent::Uint8*     pSrcRGBData,
                                                     Diligent::Uint32     Width,
                                                     Diligent::Uint32     Height,
                                                     int                  quality,
                                                     Diligent::IDataBlob* pDstJpegBits);
}

namespace Diligent
{

template <typename ChannelType>
void RGBToRGBA(const void* pRGBData,
               Uint32      RGBStride,
               void*       pRGBAData,
               Uint32      RGBAStride,
               Uint32      Width,
               Uint32      Height)
{
    for (size_t row = 0; row < size_t{Height}; ++row)
        for (size_t col = 0; col < size_t{Width}; ++col)
        {
            for (int c = 0; c < 3; ++c)
            {
                reinterpret_cast<ChannelType*>((reinterpret_cast<Uint8*>(pRGBAData) + size_t{RGBAStride} * row))[col * 4 + c] =
                    reinterpret_cast<const ChannelType*>((reinterpret_cast<const Uint8*>(pRGBData) + size_t{RGBStride} * row))[col * 3 + c];
            }
            reinterpret_cast<ChannelType*>((reinterpret_cast<Uint8*>(pRGBAData) + size_t{RGBAStride} * row))[col * 4 + 3] = std::numeric_limits<ChannelType>::max();
        }
}

void CreateTextureFromImage(Image*                 pSrcImage,
                            const TextureLoadInfo& TexLoadInfo,
                            IRenderDevice*         pDevice,
                            ITexture**             ppTexture)
{
    const auto& ImgDesc = pSrcImage->GetDesc();
    TextureDesc TexDesc;
    TexDesc.Name      = TexLoadInfo.Name;
    TexDesc.Type      = RESOURCE_DIM_TEX_2D;
    TexDesc.Width     = ImgDesc.Width;
    TexDesc.Height    = ImgDesc.Height;
    TexDesc.MipLevels = ComputeMipLevelsCount(TexDesc.Width, TexDesc.Height);
    if (TexLoadInfo.MipLevels > 0)
        TexDesc.MipLevels = std::min(TexDesc.MipLevels, TexLoadInfo.MipLevels);
    TexDesc.Usage          = TexLoadInfo.Usage;
    TexDesc.BindFlags      = TexLoadInfo.BindFlags;
    TexDesc.Format         = TexLoadInfo.Format;
    TexDesc.CPUAccessFlags = TexLoadInfo.CPUAccessFlags;
    auto ChannelDepth      = GetValueSize(ImgDesc.ComponentType) * 8;

    Uint32 NumComponents = ImgDesc.NumComponents == 3 ? 4 : ImgDesc.NumComponents;
    bool   IsSRGB        = (ImgDesc.NumComponents >= 3 && ChannelDepth == 8) ? TexLoadInfo.IsSRGB : false;
    if (TexDesc.Format == TEX_FORMAT_UNKNOWN)
    {
        if (ChannelDepth == 8)
        {
            switch (NumComponents)
            {
                case 1: TexDesc.Format = TEX_FORMAT_R8_UNORM; break;
                case 2: TexDesc.Format = TEX_FORMAT_RG8_UNORM; break;
                case 4: TexDesc.Format = IsSRGB ? TEX_FORMAT_RGBA8_UNORM_SRGB : TEX_FORMAT_RGBA8_UNORM; break;
                default: LOG_ERROR_AND_THROW("Unexpected number of color channels (", ImgDesc.NumComponents, ")");
            }
        }
        else if (ChannelDepth == 16)
        {
            switch (NumComponents)
            {
                case 1: TexDesc.Format = TEX_FORMAT_R16_UNORM; break;
                case 2: TexDesc.Format = TEX_FORMAT_RG16_UNORM; break;
                case 4: TexDesc.Format = TEX_FORMAT_RGBA16_UNORM; break;
                default: LOG_ERROR_AND_THROW("Unexpected number of color channels (", ImgDesc.NumComponents, ")");
            }
        }
        else
            LOG_ERROR_AND_THROW("Unsupported color channel depth (", ChannelDepth, ")");
    }
    else
    {
        const auto& TexFmtDesc = GetTextureFormatAttribs(TexDesc.Format);
        if (TexFmtDesc.NumComponents != NumComponents)
            LOG_ERROR_AND_THROW("Incorrect number of components ", ImgDesc.NumComponents, ") for texture format ", TexFmtDesc.Name);
        if (TexFmtDesc.ComponentSize != ChannelDepth / 8)
            LOG_ERROR_AND_THROW("Incorrect channel size ", ChannelDepth, ") for texture format ", TexFmtDesc.Name);
    }


    std::vector<TextureSubResData>  pSubResources(TexDesc.MipLevels);
    std::vector<std::vector<Uint8>> Mips(TexDesc.MipLevels);

    if (ImgDesc.NumComponents == 3)
    {
        VERIFY_EXPR(NumComponents == 4);
        auto RGBAStride = ImgDesc.Width * NumComponents * ChannelDepth / 8;
        RGBAStride      = (RGBAStride + 3) & (-4);
        Mips[0].resize(size_t{RGBAStride} * size_t{ImgDesc.Height});
        pSubResources[0].pData  = Mips[0].data();
        pSubResources[0].Stride = RGBAStride;
        if (ChannelDepth == 8)
        {
            RGBToRGBA<Uint8>(pSrcImage->GetData()->GetDataPtr(), ImgDesc.RowStride,
                             Mips[0].data(), RGBAStride,
                             ImgDesc.Width, ImgDesc.Height);
        }
        else if (ChannelDepth == 16)
        {
            RGBToRGBA<Uint16>(pSrcImage->GetData()->GetDataPtr(), ImgDesc.RowStride,
                              Mips[0].data(), RGBAStride,
                              ImgDesc.Width, ImgDesc.Height);
        }
    }
    else
    {
        pSubResources[0].pData  = pSrcImage->GetData()->GetDataPtr();
        pSubResources[0].Stride = ImgDesc.RowStride;
    }

    auto MipWidth  = TexDesc.Width;
    auto MipHeight = TexDesc.Height;
    for (Uint32 m = 1; m < TexDesc.MipLevels; ++m)
    {
        auto CoarseMipWidth  = std::max(MipWidth / 2u, 1u);
        auto CoarseMipHeight = std::max(MipHeight / 2u, 1u);
        auto CoarseMipStride = CoarseMipWidth * NumComponents * ChannelDepth / 8;
        CoarseMipStride      = (CoarseMipStride + 3) & (-4);
        Mips[m].resize(size_t{CoarseMipStride} * size_t{CoarseMipHeight});

        if (TexLoadInfo.GenerateMips)
        {
            ComputeMipLevel(MipWidth, MipHeight, TexDesc.Format,
                            pSubResources[m - 1].pData, pSubResources[m - 1].Stride,
                            Mips[m].data(), CoarseMipStride);
        }

        pSubResources[m].pData  = Mips[m].data();
        pSubResources[m].Stride = CoarseMipStride;

        MipWidth  = CoarseMipWidth;
        MipHeight = CoarseMipHeight;
    }

    TextureData TexData;
    TexData.pSubResources   = pSubResources.data();
    TexData.NumSubresources = TexDesc.MipLevels;

    pDevice->CreateTexture(TexDesc, &TexData, ppTexture);
}

void CreateTextureFromDDS(const void*            pDDSData,
                          size_t                 DataSize,
                          const TextureLoadInfo& TexLoadInfo,
                          IRenderDevice*         pDevice,
                          ITexture**             ppTexture)
{
    CreateDDSTextureFromMemoryEx(pDevice,
                                 reinterpret_cast<const Uint8*>(pDDSData),
                                 DataSize,
                                 0, // maxSize
                                 TexLoadInfo.Usage,
                                 TexLoadInfo.Name,
                                 TexLoadInfo.BindFlags,
                                 TexLoadInfo.CPUAccessFlags,
                                 MISC_TEXTURE_FLAG_NONE, // miscFlags
                                 TexLoadInfo.IsSRGB,     // forceSRGB
                                 ppTexture);
}

DECODE_PNG_RESULT DecodePng(IDataBlob* pSrcPngBits,
                            IDataBlob* pDstPixels,
                            ImageDesc* pDstImgDesc)
{
    return Diligent_DecodePng(pSrcPngBits, pDstPixels, pDstImgDesc);
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


DECODE_JPEG_RESULT DecodeJpeg(IDataBlob* pSrcJpegBits,
                              IDataBlob* pDstPixels,
                              ImageDesc* pDstImgDesc)
{
    return Diligent_DecodeJpeg(pSrcJpegBits, pDstPixels, pDstImgDesc);
}

ENCODE_JPEG_RESULT EncodeJpeg(Uint8*     pSrcRGBPixels,
                              Uint32     Width,
                              Uint32     Height,
                              int        quality,
                              IDataBlob* pDstJpegBits)
{
    return Diligent_EncodeJpeg(pSrcRGBPixels, Width, Height, quality, pDstJpegBits);
}

} // namespace Diligent

extern "C"
{
    void Diligent_CreateTextureFromImage(Diligent::Image*                 pSrcImage,
                                         const Diligent::TextureLoadInfo& TexLoadInfo,
                                         Diligent::IRenderDevice*         pDevice,
                                         Diligent::ITexture**             ppTexture)
    {
        Diligent::CreateTextureFromImage(pSrcImage, TexLoadInfo, pDevice, ppTexture);
    }

    void Diligent_CreateTextureFromDDS(const void*                      pDDSData,
                                       size_t                           DataSize,
                                       const Diligent::TextureLoadInfo& TexLoadInfo,
                                       Diligent::IRenderDevice*         pDevice,
                                       Diligent::ITexture**             ppTexture)

    {
        Diligent::CreateTextureFromDDS(pDDSData, DataSize, TexLoadInfo, pDevice, ppTexture);
    }
}
