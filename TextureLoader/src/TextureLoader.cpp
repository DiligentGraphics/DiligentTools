/*     Copyright 2015-2018 Egor Yusov
 *  
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 * 
 *     http://www.apache.org/licenses/LICENSE-2.0
 * 
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT OF ANY PROPRIETARY RIGHTS.
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
#include "GraphicsAccessories.h"
#include "DDSLoader.h"

using namespace Diligent;

namespace Diligent
{
    static const float a = 0.055f;

    // https://en.wikipedia.org/wiki/SRGB
    float SRGBToLinear(float SRGB)
    {
        if( SRGB < 0.04045f )
            return SRGB / 12.92f;
        else
            return pow( (SRGB + a) / (1 + a), 2.4f );
    }

    float LinearToSRGB( float c )
    {
        if( c < 0.0031308f )
            return 12.92f * c;
        else
            return (1+a) * pow(c, 1.f/2.4f) - a;
    }
    
    template<typename ChannelType>
    ChannelType SRGBAverage(ChannelType c0, ChannelType c1, ChannelType c2, ChannelType c3)
    {
        const float NormVal = static_cast<float>(std::numeric_limits<ChannelType>::max());
        float fc0 = static_cast<float>(c0) / NormVal;
        float fc1 = static_cast<float>(c1) / NormVal;
        float fc2 = static_cast<float>(c2) / NormVal;
        float fc3 = static_cast<float>(c3) / NormVal;

        float fLinearAverage = (SRGBToLinear( fc0 ) + SRGBToLinear( fc1 ) + SRGBToLinear( fc2 ) + SRGBToLinear( fc3 )) / 4.f;
        float fSRGBAverage = LinearToSRGB(fLinearAverage);
        Int32 SRGBAverage = static_cast<Int32>(fSRGBAverage * NormVal);
        SRGBAverage = std::min(SRGBAverage, static_cast<Int32>( std::numeric_limits<ChannelType>::max()) );
        SRGBAverage = std::max(SRGBAverage, static_cast<Int32>( std::numeric_limits<ChannelType>::min()) );
        return static_cast<ChannelType>(SRGBAverage);
    }

    template < typename ChannelType >
    void ComputeCoarseMip(Uint32 NumChannels, bool IsSRGB,
                          const void *pFineMip, Uint32 FineMipStride,
                          void *pCoarseMip, Uint32 CoarseMipStride,
                          Uint32 CoarseMipWidth, Uint32 CoarseMipHeight)
    {
        for( Uint32 row = 0; row < CoarseMipHeight; ++row )
            for( Uint32 col = 0; col < CoarseMipWidth; ++col )
            {
                auto FineRow0 = reinterpret_cast<const ChannelType*>( reinterpret_cast<const Uint8*>(pFineMip) +  row * 2       * FineMipStride );
                auto FineRow1 = reinterpret_cast<const ChannelType*>( reinterpret_cast<const Uint8*>(pFineMip) + (row * 2 + 1 ) * FineMipStride );
             
                for( Uint32 c = 0; c < NumChannels; ++c )
                {
                    auto Col00 = FineRow0[  col*2    * NumChannels + c ];
                    auto Col01 = FineRow0[ (col*2+1) * NumChannels + c ];
                    auto Col10 = FineRow1[  col*2    * NumChannels + c ];
                    auto Col11 = FineRow1[ (col*2+1) * NumChannels + c ];
                    auto &DstCol = reinterpret_cast<ChannelType*>(reinterpret_cast<Uint8*>(pCoarseMip) + row * CoarseMipStride)[col * NumChannels + c];
                    if( IsSRGB )
                        DstCol = SRGBAverage( Col00, Col01, Col10, Col11 );
                    else
                        DstCol = (Col00 + Col01 + Col10 + Col11)/4;
                }
            }
    }

    template < typename ChannelType >
    void RGBToRGBA(const void *pRGBData, Uint32 RGBStride,
                   void *pRGBAData, Uint32 RGBAStride,
                   Uint32 Width, Uint32 Height)
    {
        for( Uint32 row = 0; row < Height; ++row )
            for( Uint32 col = 0; col < Width; ++col )
            {
                for( int c = 0; c < 3; ++c )
                {
                    reinterpret_cast<ChannelType*>( (reinterpret_cast<Uint8*>(pRGBAData) + RGBAStride * row) ) [col *4 + c] = 
                        reinterpret_cast<const ChannelType*>( (reinterpret_cast<const Uint8*>(pRGBData) + RGBStride * row))[col*3 + c];
                }
                reinterpret_cast<ChannelType*>( (reinterpret_cast<Uint8*>(pRGBAData) + RGBAStride * row) ) [col *4 + 3] = std::numeric_limits<ChannelType>::max();
            }
    }

    void CreateTextureFromImage( Image *pSrcImage,
                                 const TextureLoadInfo& TexLoadInfo, 
                                 Diligent::IRenderDevice *pDevice, 
                                 Diligent::ITexture **ppTexture )
    {
        const auto& ImgDesc = pSrcImage->GetDesc();
        TextureDesc TexDesc;
        TexDesc.Name = TexLoadInfo.Name;
        TexDesc.Type = RESOURCE_DIM_TEX_2D;
        TexDesc.Width = ImgDesc.Width;
        TexDesc.Height = ImgDesc.Height;
        TexDesc.MipLevels = ComputeMipLevelsCount( TexDesc.Width, TexDesc.Height );
        if( TexLoadInfo.MipLevels > 0 )
            TexDesc.MipLevels = std::min(TexDesc.MipLevels, TexLoadInfo.MipLevels);
        TexDesc.Usage = TexLoadInfo.Usage;
        TexDesc.BindFlags = TexLoadInfo.BindFlags;
        TexDesc.Format = TexLoadInfo.Format;
        TexDesc.CPUAccessFlags = TexLoadInfo.CPUAccessFlags;
        auto ChannelDepth = ImgDesc.BitsPerPixel / ImgDesc.NumComponents;

        Uint32 NumComponents = ImgDesc.NumComponents == 3 ? 4 : ImgDesc.NumComponents;
        bool IsSRGB = (ImgDesc.NumComponents >= 3 && ChannelDepth == 8) ? TexLoadInfo.IsSRGB : false;
        if( TexDesc.Format == TEX_FORMAT_UNKNOWN )
        {
            if( ChannelDepth == 8 )
            {
                switch( NumComponents )
                {
                    case 1: TexDesc.Format = TEX_FORMAT_R8_UNORM; break;
                    case 2: TexDesc.Format = TEX_FORMAT_RG8_UNORM; break;
                    case 4: TexDesc.Format = IsSRGB ? TEX_FORMAT_RGBA8_UNORM_SRGB : TEX_FORMAT_RGBA8_UNORM; break;
                    default: LOG_ERROR_AND_THROW( "Unexpected number of color channels (", ImgDesc.NumComponents, ")" );
                }
            }
            else if( ChannelDepth == 16 )
            {
                switch( NumComponents )
                {
                    case 1: TexDesc.Format = TEX_FORMAT_R16_UNORM; break;
                    case 2: TexDesc.Format = TEX_FORMAT_RG16_UNORM; break;
                    case 4: TexDesc.Format = TEX_FORMAT_RGBA16_UNORM; break;
                    default: LOG_ERROR_AND_THROW( "Unexpected number of color channels (", ImgDesc.NumComponents, ")" );
                }
            }
            else
                LOG_ERROR_AND_THROW( "Unsupported color channel depth (", ChannelDepth, ")" );
        }
        else
        {
            const auto& TexFmtDesc = GetTextureFormatAttribs( TexDesc.Format );
            if( TexFmtDesc.NumComponents != NumComponents )
                LOG_ERROR_AND_THROW( "Incorrect number of components ", ImgDesc.NumComponents, ") for texture format ", TexFmtDesc.Name );
            if( TexFmtDesc.ComponentSize != ChannelDepth / 8   )
                LOG_ERROR_AND_THROW( "Incorrect channel size ", ChannelDepth, ") for texture format ", TexFmtDesc.Name );
        }
        

        std::vector<TextureSubResData> pSubResources(TexDesc.MipLevels);
        std::vector< std::vector<Uint8> > Mips(TexDesc.MipLevels);

        if( ImgDesc.NumComponents == 3 )
        {
            VERIFY_EXPR( NumComponents == 4 );
            auto RGBAStride = ImgDesc.Width * NumComponents * ChannelDepth / 8;
            RGBAStride = (RGBAStride + 3) & (-4);
            Mips[0].resize(RGBAStride * ImgDesc.Height);
            pSubResources[0].pData = Mips[0].data();
            pSubResources[0].Stride = RGBAStride;
            if( ChannelDepth == 8 )
                RGBToRGBA<Uint8>( pSrcImage->GetData()->GetDataPtr(), ImgDesc.RowStride,
                                  Mips[0].data(), RGBAStride,
                                  ImgDesc.Width, ImgDesc.Height);
            else if( ChannelDepth == 16 )
                RGBToRGBA<Uint16>( pSrcImage->GetData()->GetDataPtr(), ImgDesc.RowStride,
                                   Mips[0].data(), RGBAStride,
                                   ImgDesc.Width, ImgDesc.Height);
        }
        else
        {
            pSubResources[0].pData = pSrcImage->GetData()->GetDataPtr();
            pSubResources[0].Stride = ImgDesc.RowStride;
        }
        
        auto MipWidth  = TexDesc.Width;
        auto MipHeight = TexDesc.Height;
        for( Uint32 m = 1; m < TexDesc.MipLevels; ++m )
        {
            auto CoarseMipWidth  = std::max(MipWidth/2u, 1u);
            auto CoarseMipHeight = std::max(MipHeight/2u, 1u);
            auto CoarseMipStride = CoarseMipWidth * NumComponents * ChannelDepth / 8;
            CoarseMipStride = (CoarseMipStride + 3) & (-4);
            Mips[m].resize(CoarseMipStride * CoarseMipHeight);

            if (TexLoadInfo.GenerateMips)
            {
                if (ChannelDepth == 8)
                    ComputeCoarseMip<Uint8>(NumComponents, IsSRGB,
                        pSubResources[m - 1].pData, pSubResources[m - 1].Stride,
                        Mips[m].data(), CoarseMipStride,
                        CoarseMipWidth, CoarseMipHeight);
                else if (ChannelDepth == 16)
                    ComputeCoarseMip<Uint16>(NumComponents, IsSRGB,
                        pSubResources[m - 1].pData, pSubResources[m - 1].Stride,
                        Mips[m].data(), CoarseMipStride,
                        CoarseMipWidth, CoarseMipHeight);
            }

            pSubResources[m].pData = Mips[m].data();
            pSubResources[m].Stride = CoarseMipStride;

            MipWidth  = CoarseMipWidth;
            MipHeight = CoarseMipHeight;
        }

        TextureData TexData;
        TexData.pSubResources = pSubResources.data();
        TexData.NumSubresources = TexDesc.MipLevels;

        pDevice->CreateTexture( TexDesc, TexData, ppTexture );
    }

    void CreateTextureFromDDS( IDataBlob *pDDSData,
                               const TextureLoadInfo& TexLoadInfo, 
                               Diligent::IRenderDevice *pDevice, 
                               Diligent::ITexture **ppTexture )
    {
        CreateDDSTextureFromMemoryEx(pDevice, 
                                     reinterpret_cast<const Uint8*>(pDDSData->GetDataPtr()),
                                     static_cast<size_t>(pDDSData->GetSize()),
                                     0, // maxSize
                                     TexLoadInfo.Usage,
                                     TexLoadInfo.Name,
                                     TexLoadInfo.BindFlags,
                                     TexLoadInfo.CPUAccessFlags,
                                     MISC_TEXTURE_FLAG_NONE, // miscFlags
                                     TexLoadInfo.IsSRGB, // forceSRGB
                                     ppTexture );
    }
}
