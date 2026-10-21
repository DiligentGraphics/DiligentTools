/*
 *  Copyright 2019-2026 Diligent Graphics LLC
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

#include "../interface/PNGCodec.h"
#include "TextureLoader.h"
#include "png.h"

#include "TestingEnvironment.hpp"
#include "gtest/gtest.h"

#include <cstring>
#include <vector>

#include "DataBlobImpl.hpp"

using namespace Diligent;

namespace
{

void EncodeWhitePng(IDataBlob* pPngData, size_t& IDATDataOffset, size_t& IDATDataSize)
{
    constexpr Uint8 WhitePixel[] = {255, 255, 255, 255};
    ASSERT_EQ(EncodePng(WhitePixel, 1, 1, sizeof(WhitePixel), PNG_COLOR_TYPE_RGBA, pPngData), ENCODE_PNG_RESULT_OK);

    // Locate the image-data chunk instead of assuming a particular encoded size.
    // Each chunk contains a four-byte length, four-byte type, data, and CRC.
    const Uint8* pBytes  = pPngData->GetConstDataPtr<Uint8>();
    const size_t PNGSize = pPngData->GetSize();
    for (size_t Offset = 8; Offset + 12 <= PNGSize;)
    {
        const size_t ChunkSize = png_get_uint_32(pBytes + Offset);
        ASSERT_LE(ChunkSize, PNGSize - Offset - 12);
        if (std::memcmp(pBytes + Offset + 4, "IDAT", 4) == 0)
        {
            ASSERT_GT(ChunkSize, 1u);
            IDATDataOffset = Offset + 8;
            IDATDataSize   = ChunkSize;
            return;
        }
        Offset += ChunkSize + 12;
    }
    FAIL() << "Encoded PNG contains no IDAT chunk";
}

TEST(Tools_TextureLoader, PNGCodec)
{
    constexpr Uint32 TestImgWidth  = 256;
    constexpr Uint32 TestImgHeight = 128;

    for (int EncodeAlpha = 0; EncodeAlpha <= 1; ++EncodeAlpha)
    {
        const Uint32 NumComponents = EncodeAlpha ? 4u : 3u;

        std::vector<Uint8> RefPixels(TestImgWidth * TestImgHeight * NumComponents);
        for (Uint32 y = 0; y < TestImgHeight; ++y)
        {
            for (Uint32 x = 0; x < TestImgWidth; ++x)
            {
                auto idx = x + y * TestImgWidth;

                RefPixels[idx * NumComponents + 0] = (16 + idx) & 0xFF;
                RefPixels[idx * NumComponents + 1] = (32 + idx) & 0xFF;
                RefPixels[idx * NumComponents + 2] = (64 + idx) & 0xFF;
                if (NumComponents == 4)
                    RefPixels[idx * NumComponents + 3] = (96 + idx) & 0xFF;
            }
        }

        RefCntAutoPtr<IDataBlob> pPngData = DataBlobImpl::Create();

        ENCODE_PNG_RESULT Res = EncodePng(RefPixels.data(), TestImgWidth, TestImgHeight, TestImgWidth * NumComponents,
                                          EncodeAlpha ? PNG_COLOR_TYPE_RGBA : PNG_COLOR_TYPE_RGB,
                                          pPngData);
        ASSERT_EQ(Res, ENCODE_PNG_RESULT_OK);

        {
            ImageDesc DecodedImgDesc;
            EXPECT_EQ(DecodePng(pPngData->GetConstDataPtr(), pPngData->GetSize(), nullptr, &DecodedImgDesc), DECODE_PNG_RESULT_OK);

            EXPECT_EQ(DecodedImgDesc.Width, TestImgWidth);
            EXPECT_EQ(DecodedImgDesc.Height, TestImgHeight);
            EXPECT_EQ(DecodedImgDesc.NumComponents, NumComponents);
            EXPECT_EQ(DecodedImgDesc.ComponentType, VT_UINT8);
        }

        {
            RefCntAutoPtr<IDataBlob> pDecodedPixelsBlob = DataBlobImpl::Create();

            ImageDesc DecodedImgDesc;
            ASSERT_EQ(DecodePng(pPngData->GetConstDataPtr(), pPngData->GetSize(), pDecodedPixelsBlob, &DecodedImgDesc), DECODE_PNG_RESULT_OK);

            ASSERT_EQ(DecodedImgDesc.Width, TestImgWidth);
            ASSERT_EQ(DecodedImgDesc.Height, TestImgHeight);
            ASSERT_EQ(DecodedImgDesc.NumComponents, NumComponents);
            ASSERT_EQ(DecodedImgDesc.ComponentType, VT_UINT8);

            const Uint8* pTestPixels = pDecodedPixelsBlob->GetConstDataPtr<Uint8>();
            for (Uint32 y = 0; y < TestImgHeight; ++y)
            {
                for (Uint32 x = 0; x < TestImgWidth; ++x)
                {
                    for (Uint32 c = 0; c < NumComponents; ++c)
                    {
                        Uint8 RefVal  = RefPixels[(x + y * TestImgWidth) * NumComponents + c];
                        Uint8 TestVal = pTestPixels[x * DecodedImgDesc.NumComponents + c + y * DecodedImgDesc.RowStride];
                        EXPECT_EQ(static_cast<Uint32>(RefVal), static_cast<Uint32>(TestVal)) << "[" << x << "," << y << "][" << c << "]";
                    }
                }
            }
        }
    }
}

TEST(Tools_TextureLoader, PNGCodecRejectsTruncatedData)
{
    RefCntAutoPtr<IDataBlob> pPngData       = DataBlobImpl::Create();
    size_t                   IDATDataOffset = 0;
    size_t                   IDATDataSize   = 0;
    ASSERT_NO_FATAL_FAILURE(EncodeWhitePng(pPngData, IDATDataOffset, IDATDataSize));
    const Uint8* pPngBytes = pPngData->GetConstDataPtr<Uint8>();

    // A complete 1x1 white RGBA PNG remains in backing storage. Passing shorter
    // declared lengths safely exposes reads beyond PngDataSize in the old code.
    RefCntAutoPtr<IDataBlob> pPixels = DataBlobImpl::Create();
    ImageDesc                Desc;
    ASSERT_EQ(DecodePng(pPngBytes, pPngData->GetSize(), pPixels, &Desc), DECODE_PNG_RESULT_OK);
    EXPECT_EQ(Desc.Width, 1u);
    EXPECT_EQ(Desc.Height, 1u);
    EXPECT_EQ(Desc.NumComponents, 4u);
    EXPECT_EQ(Desc.ComponentType, VT_UINT8);
    EXPECT_EQ(Desc.RowStride, 4u);
    ASSERT_EQ(pPixels->GetSize(), 4u);
    const Uint8* pDecodedPixels = pPixels->GetConstDataPtr<Uint8>();
    for (Uint32 Component = 0; Component < 4; ++Component)
    {
        EXPECT_EQ(pDecodedPixels[Component], 255u);
    }

    // Fewer than eight signature bytes must be rejected before libpng reads.
    for (size_t DataSize = 0; DataSize < 8; ++DataSize)
    {
        SCOPED_TRACE(DataSize);
        EXPECT_EQ(DecodePng(pPngBytes, DataSize, nullptr, &Desc), DECODE_PNG_RESULT_INVALID_SIGNATURE);
        EXPECT_EQ(DecodePng(pPngBytes, DataSize, pPixels, &Desc), DECODE_PNG_RESULT_INVALID_SIGNATURE);
    }

    // Cut off the chunk header, IHDR data/CRC, or following IDAT header.
    // Eight bytes is a valid signature with no chunk data at all.
    const size_t TruncatedHeaderSizes[] = {8, 12, 16, 28, 32, 33, IDATDataOffset - 1};
    for (const size_t DataSize : TruncatedHeaderSizes)
    {
        SCOPED_TRACE(DataSize);
        EXPECT_EQ(DecodePng(pPngBytes, DataSize, nullptr, &Desc), DECODE_PNG_RESULT_DECODING_ERROR);
        EXPECT_EQ(DecodePng(pPngBytes, DataSize, pPixels, &Desc), DECODE_PNG_RESULT_DECODING_ERROR);
    }

    // Keep the IDAT header, but truncate the compressed pixels at several points.
    // Metadata-only decoding succeeds. Pixel decoding reaches png_read_image()
    // after allocating its row pointers, then fails.
    const size_t TruncatedImageSizes[] = {
        IDATDataOffset,
        IDATDataOffset + 1,
        IDATDataOffset + IDATDataSize / 2,
        IDATDataOffset + IDATDataSize - 1,
    };
    for (const size_t DataSize : TruncatedImageSizes)
    {
        SCOPED_TRACE(DataSize);
        ASSERT_EQ(DecodePng(pPngBytes, DataSize, nullptr, &Desc), DECODE_PNG_RESULT_OK);
        EXPECT_EQ(Desc.Width, 1u);
        EXPECT_EQ(Desc.Height, 1u);
        EXPECT_EQ(Desc.NumComponents, 4u);
        EXPECT_EQ(Desc.ComponentType, VT_UINT8);
        EXPECT_EQ(DecodePng(pPngBytes, DataSize, pPixels, &Desc), DECODE_PNG_RESULT_DECODING_ERROR);
    }
}

TEST(Tools_TextureLoader, PNGTextureLoaderRejectsTruncatedData)
{
    RefCntAutoPtr<IDataBlob> pPngData       = DataBlobImpl::Create();
    size_t                   IDATDataOffset = 0;
    size_t                   IDATDataSize   = 0;
    ASSERT_NO_FATAL_FAILURE(EncodeWhitePng(pPngData, IDATDataOffset, IDATDataSize));
    const Uint8* pPngBytes = pPngData->GetConstDataPtr<Uint8>();

    // Test both format inference and an explicit output format. A complete PNG
    // header without pixel data must not become a successful partial image.
    const size_t TruncatedSizes[] = {8, IDATDataOffset};
    for (const TEXTURE_FORMAT Format : {TEX_FORMAT_UNKNOWN, TEX_FORMAT_RGBA8_UNORM})
    {
        SCOPED_TRACE(Format);
        TextureLoadInfo LoadInfo;
        LoadInfo.Format = Format;
        for (const size_t DataSize : TruncatedSizes)
        {
            SCOPED_TRACE(DataSize);
            Testing::TestingEnvironment::ErrorScope ExpectedErrors{
                "Failed to create texture loader from memory",
                "Failed to decode image.",
                "Failed to load png image"};
            RefCntAutoPtr<ITextureLoader> pLoader;
            CreateTextureLoaderFromMemory(pPngBytes, DataSize, false, LoadInfo, &pLoader);
            EXPECT_EQ(pLoader, nullptr);
        }
    }

    RefCntAutoPtr<ITextureLoader> pLoader;
    CreateTextureLoaderFromMemory(pPngBytes, pPngData->GetSize(), false, {}, &pLoader);
    ASSERT_NE(pLoader, nullptr);
    EXPECT_EQ(pLoader->GetTextureDesc().Width, 1u);
    EXPECT_EQ(pLoader->GetTextureDesc().Height, 1u);
    EXPECT_EQ(pLoader->GetTextureDesc().Format, TEX_FORMAT_RGBA8_UNORM);
    const TextureSubResData& Subresource = pLoader->GetSubresourceData(0);
    ASSERT_NE(Subresource.pData, nullptr);
    const Uint8* pPixels = static_cast<const Uint8*>(Subresource.pData);
    for (Uint32 Component = 0; Component < 4; ++Component)
    {
        EXPECT_EQ(pPixels[Component], 255u);
    }
}

} // namespace
