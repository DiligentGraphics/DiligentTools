/*
 *  Copyright 2019-2022 Diligent Graphics LLC
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

#include "SGILoader.h"

#include "DataBlob.h"
#include "PlatformMisc.hpp"
#include "Errors.hpp"

namespace Diligent
{

namespace
{
struct SGIHeader
{
    Uint16  Magic;               // SGI magic number
    uint8_t Compression;         // Whether to use RLE compression or not
    uint8_t BytePerPixelChannel; // 1 for 8-bit channels, or 2 for 16-bit channels
    Uint16  DimensionBE;         // Image dimension, equals 3 for RGBA image
    Uint16  WidthBE;             // Image width
    Uint16  HeightBE;            // Image height
    Uint16  ChannelsBE;          // Number of channels, equals 4 for RGBA image
    Uint32  MinPixelValueBE;     // Smallest pixel value in the image
    Uint32  MaxPixelValueBE;     // Largest pixel value in the image
    Uint32  Reserved1BE;         // Unused
    char    Name[80];            // C string null-terminated name
    Uint32  ColorMapIDBE;        // Only for color map image
    char    Reserved2[404];      // To make header 512 bytes long. Ignore
};

static_assert(sizeof(SGIHeader) == 512, "must be 512 bytes");
} // namespace

// http://paulbourke.net/dataformats/sgirgb/sgiversion.html
bool LoadSGI(IDataBlob* pSGIData,
             IDataBlob* pDstPixels,
             ImageDesc* pDstImgDesc)
{
    VERIFY_EXPR(pSGIData != nullptr && pDstPixels != nullptr && pDstImgDesc != nullptr);
    const auto* pDataStart = reinterpret_cast<const Uint8*>(pSGIData->GetConstDataPtr());
    const auto  Size       = pSGIData->GetSize();
    const auto* pDataEnd   = pDataStart + Size;
    const auto* pSrcPtr    = pDataStart;

    if (Size < sizeof(SGIHeader))
    {
        LOG_ERROR_MESSAGE("The SGI data size (", Size, ") is smaller than the size of required SGI header (", sizeof(SGIHeader), ").");
        return false;
    }

    const auto& Header = reinterpret_cast<const SGIHeader&>(*pSrcPtr);
    pSrcPtr += sizeof(SGIHeader);

    constexpr Uint16 SGIMagic = 0xda01u;
    if (Header.Magic != 0xda01)
    {
        LOG_ERROR_MESSAGE("0x", std::hex, Header.Magic, " is not a valid SGI magic number; 0x", std::hex, SGIMagic, " is expected.");
        return false;
    }

    const auto Width           = PlatformMisc::SwapBytes(Header.WidthBE);
    const auto Height          = PlatformMisc::SwapBytes(Header.HeightBE);
    const auto NumChannels     = PlatformMisc::SwapBytes(Header.ChannelsBE);
    const auto BytesPerChannel = Header.BytePerPixelChannel;
    pDstImgDesc->Width         = Width;
    pDstImgDesc->Height        = Height;
    pDstImgDesc->NumComponents = NumChannels;

    switch (BytesPerChannel)
    {
        case 1:
            pDstImgDesc->ComponentType = VT_UINT8;
            break;

        case 2:
            pDstImgDesc->ComponentType = VT_UINT16;
            break;

        case 4:
            pDstImgDesc->ComponentType = VT_UINT32;
            break;

        default:
            UNEXPECTED(BytesPerChannel, " is not a supported byte count");
            return false;
    }

    pDstImgDesc->RowStride = Width * NumChannels * BytesPerChannel;
    pDstPixels->Resize(size_t{Height} * pDstImgDesc->RowStride);
    auto* pDstPtr = reinterpret_cast<Uint8*>(pDstPixels->GetDataPtr());

    // Offsets table starts at byte 512 and is Height * NumChannels * 4 bytes long.
    const auto  TableSize     = sizeof(Uint32) * Height * NumChannels;
    const auto* OffsetTableBE = reinterpret_cast<const Uint32*>(pSrcPtr);
    pSrcPtr += TableSize;
    if (pSrcPtr > pDataEnd)
        return false;

    // Length table follows the offsets table and is the same size.
    const auto* LengthTableBE = reinterpret_cast<const Uint32*>(pSrcPtr);
    pSrcPtr += TableSize;
    if (pSrcPtr > pDataEnd)
        return false;

    VERIFY(Header.Compression, "Only RLE compressed files are currently supported");
    VERIFY(BytesPerChannel == 1, "Only 8-bit images are currently supported");

    const auto ReadLine = [Width, BytesPerChannel, NumChannels](auto* pDst, const auto* pLineDataStart, const auto* pLineDataEnd) //
    {
        VERIFY_EXPR(sizeof(*pDst) == BytesPerChannel);
        VERIFY_EXPR(sizeof(*pLineDataStart) == BytesPerChannel);
        (void)BytesPerChannel;

        const auto* pSrc = pLineDataStart;

        Uint32 x = 0;
        while (x < Width && pSrc < pLineDataEnd)
        {
            // The lowest 7 bits is the counter
            int Count = *pSrc & 0x7F;

            // If the high order bit of the first byte is 1, then the count is used to specify how many values to copy
            // from the RLE data buffer.
            // If the high order bit of the first byte is 0, then the count is used to specify how many times to repeat
            // the value following the counter.
            auto DistinctValues = (*pSrc & 0x80) != 0;

            ++pSrc;
            while (Count > 0 && pSrc < pLineDataEnd)
            {
                *pDst = *pSrc;
                if (DistinctValues || Count == 1) // Always move the pointer for the last value
                    ++pSrc;

                pDst += NumChannels;
                --Count;
                ++x;
            }
        }
        return x == Width;
    };

    for (Uint32 c = 0; c < NumChannels; ++c)
    {
        for (Uint32 y = 0; y < Height; ++y)
        {
            // Each unsigned int in the offset table is the offset (from the file start) to the
            // start of the compressed data of each scanline for each channel.
            const auto RleOff = PlatformMisc::SwapBytes(OffsetTableBE[y + c * Height]);
            // The size table tells the size of the compressed data (unsigned int) of each scanline.
            const auto RleLen = PlatformMisc::SwapBytes(LengthTableBE[y + c * Height]);

            auto* DstLine = pDstPtr + y * size_t{pDstImgDesc->RowStride} + c;
            if (!ReadLine(DstLine, pDataStart + RleOff, pSrcPtr + RleOff + RleLen))
                return false;
        }
    }

    return true;
}

} // namespace Diligent

extern "C"
{
    void Diligent_LoadSGI(Diligent::IDataBlob* pSGIData,
                          Diligent::IDataBlob* pDstPixels,
                          Diligent::ImageDesc* pDstImgDesc)
    {
        Diligent::LoadSGI(pSGIData, pDstPixels, pDstImgDesc);
    }
}
