/*
 *  Copyright 2026 Diligent Graphics LLC
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
 *  for loss of goodwill, work stoppage, computer failure or malfunction, and any and
 *  all other commercial damages or losses), even if such Contributor has been advised
 *  of the possibility of such damages.
 */

#include "TextureLoader.h"

#include "DataBlobImpl.hpp"
#include "DefaultRawMemoryAllocator.hpp"
#include "GraphicsAccessories.hpp"
#include "MemoryFileStream.hpp"

#include "TestingEnvironment.hpp"
#include "gtest/gtest.h"

#include <algorithm>
#include <array>
#include <cstring>
#include <limits>
#include <string>
#include <vector>

using namespace Diligent;

namespace
{

TextureDesc MakeRGBA8TextureDesc()
{
    TextureDesc Desc;
    Desc.Name      = "Raw RGBA8 texture";
    Desc.Type      = RESOURCE_DIM_TEX_2D;
    Desc.Width     = 2;
    Desc.Height    = 2;
    Desc.MipLevels = 1;
    Desc.Format    = TEX_FORMAT_RGBA8_UNORM;
    Desc.Usage     = USAGE_DEFAULT;
    Desc.BindFlags = BIND_SHADER_RESOURCE;
    return Desc;
}

} // namespace

TEST(Tools_TextureLoader, CreatesFromBorrowedTextureData)
{
    std::array<Uint8, 16> Pixels{};
    for (Uint32 i = 0; i < Pixels.size(); ++i)
        Pixels[i] = static_cast<Uint8>(i);

    TextureDesc       Desc = MakeRGBA8TextureDesc();
    TextureSubResData Subres{Pixels.data(), 8};
    TextureData       TexData{&Subres, 1};

    RefCntAutoPtr<ITextureLoader> pLoader;
    CreateTextureLoaderFromTextureData(Desc, TexData, false, nullptr, &pLoader);
    ASSERT_NE(pLoader, nullptr);

    const TextureDesc& LoadedDesc = pLoader->GetTextureDesc();
    EXPECT_STREQ(LoadedDesc.Name, Desc.Name);
    EXPECT_EQ(LoadedDesc.Type, Desc.Type);
    EXPECT_EQ(LoadedDesc.Width, Desc.Width);
    EXPECT_EQ(LoadedDesc.Height, Desc.Height);
    EXPECT_EQ(LoadedDesc.MipLevels, Desc.MipLevels);
    EXPECT_EQ(LoadedDesc.Format, Desc.Format);

    const TextureSubResData& LoadedSubres = pLoader->GetSubresourceData(0);
    EXPECT_EQ(LoadedSubres.pData, static_cast<const void*>(Pixels.data()));
    EXPECT_EQ(LoadedSubres.Stride, 8u);
    EXPECT_EQ(LoadedSubres.DepthStride, 16u);

    TextureData LoadedData = pLoader->GetTextureData();
    ASSERT_NE(LoadedData.pSubResources, nullptr);
    EXPECT_EQ(LoadedData.pSubResources[0].pData, LoadedSubres.pData);
    EXPECT_EQ(LoadedData.NumSubresources, 1u);
}

TEST(Tools_TextureLoader, CopiesTextureDataWithSourcePadding)
{
    std::array<Uint8, 24> PaddedPixels{};
    for (Uint32 i = 0; i < PaddedPixels.size(); ++i)
        PaddedPixels[i] = static_cast<Uint8>(i);

    TextureDesc       Desc = MakeRGBA8TextureDesc();
    TextureSubResData Subres{PaddedPixels.data(), 12};
    TextureData       TexData{&Subres, 1};

    RefCntAutoPtr<ITextureLoader> pLoader;
    CreateTextureLoaderFromTextureData(Desc, TexData, true, nullptr, &pLoader);
    ASSERT_NE(pLoader, nullptr);

    const TextureSubResData& LoadedSubres = pLoader->GetSubresourceData(0);
    ASSERT_NE(LoadedSubres.pData, nullptr);
    EXPECT_NE(LoadedSubres.pData, static_cast<const void*>(PaddedPixels.data()));
    EXPECT_EQ(LoadedSubres.Stride, 8u);
    EXPECT_EQ(LoadedSubres.DepthStride, 16u);

    const Uint8* const          pCopiedPixels = static_cast<const Uint8*>(LoadedSubres.pData);
    const std::array<Uint8, 16> ExpectedPixels{
        0, 1, 2, 3, 4, 5, 6, 7,
        12, 13, 14, 15, 16, 17, 18, 19};

    PaddedPixels.fill(0xFF);
    EXPECT_TRUE(std::equal(ExpectedPixels.begin(), ExpectedPixels.end(), pCopiedPixels));
}

TEST(Tools_TextureLoader, SupportsPitchedTextureDataWithoutFinalRowPadding)
{
    constexpr Uint32 Width          = 3;
    constexpr Uint32 Height         = 2;
    constexpr Uint32 RowSize        = Width * 4;
    constexpr Uint32 RowStride      = 16;
    constexpr Uint32 SourceDataSize = (Height - 1) * RowStride + RowSize;

    // The source ends at the final active pixel; there is no padding after the last row.
    const std::array<Uint8, SourceDataSize> Pixels{
        1, 2, 3, 255, 4, 5, 6, 255, 7, 8, 9, 255,
        0xAA, 0xBB, 0xCC, 0xDD,
        10, 11, 12, 255, 13, 14, 15, 255, 16, 17, 18, 255};

    TextureDesc Desc = MakeRGBA8TextureDesc();
    Desc.Width       = Width;
    Desc.Height      = Height;

    TextureSubResData Subres{Pixels.data(), RowStride};
    TextureData       TexData{&Subres, 1};

    TextureLoadInfo LoadInfo{Desc.Name};
    LoadInfo.Format       = Desc.Format;
    LoadInfo.MipLevels    = 1;
    LoadInfo.GenerateMips = False;

    const auto CheckPixels = [&](ITextureLoader* pLoader, bool ExpectCopy) {
        ASSERT_NE(pLoader, nullptr);

        const TextureSubResData& LoadedSubres = pLoader->GetSubresourceData(0);
        ASSERT_NE(LoadedSubres.pData, nullptr);
        EXPECT_EQ(LoadedSubres.Stride, RowStride);
        EXPECT_EQ(LoadedSubres.pData != Pixels.data(), ExpectCopy);

        const Uint8* pLoadedPixels = static_cast<const Uint8*>(LoadedSubres.pData);
        for (Uint32 Row = 0; Row < Height; ++Row)
        {
            EXPECT_TRUE(std::equal(Pixels.data() + Row * RowStride,
                                   Pixels.data() + Row * RowStride + RowSize,
                                   pLoadedPixels + Row * LoadedSubres.Stride));
        }
    };

    RefCntAutoPtr<ITextureLoader> pBorrowedLoader;
    CreateTextureLoaderFromTextureData(Desc, TexData, false, &LoadInfo, &pBorrowedLoader);
    CheckPixels(pBorrowedLoader, false);

    LoadInfo.PermultiplyAlpha = True;
    RefCntAutoPtr<ITextureLoader> pPremultipliedLoader;
    CreateTextureLoaderFromTextureData(Desc, TexData, false, &LoadInfo, &pPremultipliedLoader);
    CheckPixels(pPremultipliedLoader, true);
}

TEST(Tools_TextureLoader, HandlesArrayMipSubresources)
{
    TextureDesc Desc;
    Desc.Name      = "Raw R8 array texture";
    Desc.Type      = RESOURCE_DIM_TEX_2D_ARRAY;
    Desc.Width     = 4;
    Desc.Height    = 4;
    Desc.ArraySize = 2;
    Desc.MipLevels = 3;
    Desc.Format    = TEX_FORMAT_R8_UNORM;
    Desc.Usage     = USAGE_DEFAULT;
    Desc.BindFlags = BIND_SHADER_RESOURCE;

    std::vector<std::vector<Uint8>> SubresourceData;
    std::vector<TextureSubResData>  Subresources;
    SubresourceData.reserve(Desc.GetSubresourceCount());
    Subresources.reserve(Desc.GetSubresourceCount());

    for (Uint32 Slice = 0; Slice < Desc.GetArraySize(); ++Slice)
    {
        for (Uint32 Mip = 0; Mip < Desc.MipLevels; ++Mip)
        {
            const Uint32 Width  = std::max(Desc.Width >> Mip, 1u);
            const Uint32 Height = std::max(Desc.Height >> Mip, 1u);

            SubresourceData.emplace_back(Width * Height, static_cast<Uint8>(Slice * 16 + Mip));
            Subresources.emplace_back(SubresourceData.back().data(), Width);
        }
    }

    TextureData TexData{Subresources.data(), static_cast<Uint32>(Subresources.size())};

    RefCntAutoPtr<ITextureLoader> pLoader;
    CreateTextureLoaderFromTextureData(Desc, TexData, false, nullptr, &pLoader);
    ASSERT_NE(pLoader, nullptr);

    for (Uint32 Slice = 0; Slice < Desc.GetArraySize(); ++Slice)
    {
        for (Uint32 Mip = 0; Mip < Desc.MipLevels; ++Mip)
        {
            const Uint32 SubresIndex = Slice * Desc.MipLevels + Mip;
            const Uint32 Width       = std::max(Desc.Width >> Mip, 1u);
            const Uint32 Height      = std::max(Desc.Height >> Mip, 1u);

            const TextureSubResData& LoadedSubres = pLoader->GetSubresourceData(Mip, Slice);
            EXPECT_EQ(LoadedSubres.pData, static_cast<const void*>(SubresourceData[SubresIndex].data()));
            EXPECT_EQ(LoadedSubres.Stride, Width);
            EXPECT_EQ(LoadedSubres.DepthStride, Width * Height);
        }
    }
}

TEST(Tools_TextureLoader, AppliesTextureLoadInfoToRawTextureData)
{
    const std::array<Uint8, 16> Pixels{
        1, 2, 3, 4, 5, 6, 7, 8,
        9, 10, 11, 12, 13, 14, 15, 16};

    TextureDesc       Desc = MakeRGBA8TextureDesc();
    TextureSubResData Subres{Pixels.data(), 8};
    TextureData       TexData{&Subres, 1};

    TextureLoadInfo LoadInfo{Desc.Name};
    LoadInfo.Format         = TEX_FORMAT_RGBA8_UNORM;
    LoadInfo.MipLevels      = 1;
    LoadInfo.GenerateMips   = False;
    LoadInfo.FlipVertically = True;
    LoadInfo.Swizzle        = TextureComponentMapping{
        TEXTURE_COMPONENT_SWIZZLE_B,
        TEXTURE_COMPONENT_SWIZZLE_G,
        TEXTURE_COMPONENT_SWIZZLE_R,
        TEXTURE_COMPONENT_SWIZZLE_A};

    RefCntAutoPtr<ITextureLoader> pLoader;
    CreateTextureLoaderFromTextureData(Desc, TexData, false, &LoadInfo, &pLoader);
    ASSERT_NE(pLoader, nullptr);

    const TextureDesc& LoadedDesc = pLoader->GetTextureDesc();
    EXPECT_EQ(LoadedDesc.Format, TEX_FORMAT_RGBA8_UNORM);
    EXPECT_EQ(LoadedDesc.MipLevels, 1u);

    const TextureSubResData& LoadedSubres = pLoader->GetSubresourceData(0);
    ASSERT_NE(LoadedSubres.pData, nullptr);
    EXPECT_NE(LoadedSubres.pData, static_cast<const void*>(Pixels.data()));

    const Uint8* const          pLoadedPixels = static_cast<const Uint8*>(LoadedSubres.pData);
    const std::array<Uint8, 16> ExpectedPixels{
        11, 10, 9, 12, 15, 14, 13, 16,
        3, 2, 1, 4, 7, 6, 5, 8};
    EXPECT_TRUE(std::equal(ExpectedPixels.begin(), ExpectedPixels.end(), pLoadedPixels));
}

TEST(Tools_TextureLoader, GeneratesMipsFromRawTextureData)
{
    const std::array<Uint8, 16> Pixels{
        1, 3, 20, 24,
        5, 7, 28, 32,
        40, 44, 80, 88,
        52, 56, 94, 98};

    TextureDesc Desc;
    Desc.Name      = "Raw R8 texture";
    Desc.Type      = RESOURCE_DIM_TEX_2D;
    Desc.Width     = 4;
    Desc.Height    = 4;
    Desc.MipLevels = 1;
    Desc.Format    = TEX_FORMAT_R8_UNORM;
    Desc.Usage     = USAGE_DEFAULT;
    Desc.BindFlags = BIND_SHADER_RESOURCE;

    TextureSubResData Subres{Pixels.data(), 4};
    TextureData       TexData{&Subres, 1};

    TextureLoadInfo LoadInfo{Desc.Name};
    LoadInfo.Format       = TEX_FORMAT_R8_UNORM;
    LoadInfo.GenerateMips = True;

    RefCntAutoPtr<ITextureLoader> pLoader;
    CreateTextureLoaderFromTextureData(Desc, TexData, false, &LoadInfo, &pLoader);
    ASSERT_NE(pLoader, nullptr);

    const TextureDesc& LoadedDesc = pLoader->GetTextureDesc();
    EXPECT_EQ(LoadedDesc.Format, TEX_FORMAT_R8_UNORM);
    EXPECT_EQ(LoadedDesc.MipLevels, 3u);

    const std::array<Uint8, 4> ExpectedMip1{
        4, 26,
        48, 90};
    const std::array<Uint8, 1> ExpectedMip2{42};

    const auto CheckMip = [&](Uint32 Mip, Uint32 Width, Uint32 Height, const Uint8* pExpected) {
        const TextureSubResData& LoadedSubres = pLoader->GetSubresourceData(Mip);
        ASSERT_NE(LoadedSubres.pData, nullptr);

        const Uint8* pData = static_cast<const Uint8*>(LoadedSubres.pData);
        for (Uint32 y = 0; y < Height; ++y)
        {
            const Uint8* pRow = pData + LoadedSubres.Stride * y;
            for (Uint32 x = 0; x < Width; ++x)
                EXPECT_EQ(pRow[x], pExpected[y * Width + x]);
        }
    };

    CheckMip(0, 4, 4, Pixels.data());
    CheckMip(1, 2, 2, ExpectedMip1.data());
    CheckMip(2, 1, 1, ExpectedMip2.data());
}

TEST(Tools_TextureLoader, CompressesRawTextureData)
{
    std::array<Uint8, 4 * 4 * 4> Pixels{};
    for (Uint32 i = 0; i < Pixels.size(); i += 4)
    {
        Pixels[i + 0] = 255;
        Pixels[i + 1] = 128;
        Pixels[i + 2] = 64;
        Pixels[i + 3] = 255;
    }

    TextureDesc Desc;
    Desc.Name      = "Raw RGBA8 texture";
    Desc.Type      = RESOURCE_DIM_TEX_2D;
    Desc.Width     = 4;
    Desc.Height    = 4;
    Desc.MipLevels = 1;
    Desc.Format    = TEX_FORMAT_RGBA8_UNORM;
    Desc.Usage     = USAGE_DEFAULT;
    Desc.BindFlags = BIND_SHADER_RESOURCE;

    TextureSubResData Subres{Pixels.data(), 16};
    TextureData       TexData{&Subres, 1};

    TextureLoadInfo LoadInfo{Desc.Name};
    LoadInfo.Format       = TEX_FORMAT_RGBA8_UNORM;
    LoadInfo.MipLevels    = 1;
    LoadInfo.GenerateMips = False;
    LoadInfo.CompressMode = TEXTURE_LOAD_COMPRESS_MODE_BC;

    RefCntAutoPtr<ITextureLoader> pLoader;
    CreateTextureLoaderFromTextureData(Desc, TexData, false, &LoadInfo, &pLoader);
    ASSERT_NE(pLoader, nullptr);

    EXPECT_EQ(pLoader->GetTextureDesc().Format, TEX_FORMAT_BC3_UNORM);
    EXPECT_EQ(pLoader->GetTextureDesc().MipLevels, 1u);
    EXPECT_NE(pLoader->GetSubresourceData(0).pData, nullptr);
}

TEST(Tools_TextureLoader, PremultipliesRawTextureAlpha)
{
    const std::array<Uint8, 4> Pixel{128, 64, 32, 128};

    TextureDesc Desc;
    Desc.Name      = "Raw RGBA8 texture";
    Desc.Type      = RESOURCE_DIM_TEX_2D;
    Desc.Width     = 1;
    Desc.Height    = 1;
    Desc.MipLevels = 1;
    Desc.Format    = TEX_FORMAT_RGBA8_UNORM;
    Desc.Usage     = USAGE_DEFAULT;
    Desc.BindFlags = BIND_SHADER_RESOURCE;

    TextureSubResData Subres{Pixel.data(), 4};
    TextureData       TexData{&Subres, 1};

    TextureLoadInfo LoadInfo{Desc.Name};
    LoadInfo.Format           = TEX_FORMAT_RGBA8_UNORM;
    LoadInfo.MipLevels        = 1;
    LoadInfo.GenerateMips     = False;
    LoadInfo.PermultiplyAlpha = True;

    RefCntAutoPtr<ITextureLoader> pLoader;
    CreateTextureLoaderFromTextureData(Desc, TexData, false, &LoadInfo, &pLoader);
    ASSERT_NE(pLoader, nullptr);

    const TextureSubResData& LoadedSubres = pLoader->GetSubresourceData(0);
    ASSERT_NE(LoadedSubres.pData, nullptr);

    const Uint8* const pLoadedPixel = static_cast<const Uint8*>(LoadedSubres.pData);
    EXPECT_EQ(pLoadedPixel[0], 64u);
    EXPECT_EQ(pLoadedPixel[1], 32u);
    EXPECT_EQ(pLoadedPixel[2], 16u);
    EXPECT_EQ(pLoadedPixel[3], 128u);
}

TEST(Tools_TextureLoader, RejectsInvalidTextureData)
{
    TextureDesc Desc = MakeRGBA8TextureDesc();
    TextureData TexData{};

    Testing::TestingEnvironment::ErrorScope ExpectedErrors{
        "Failed to create texture loader from texture data",
        "Texture data must contain exactly one entry"};

    RefCntAutoPtr<ITextureLoader> pLoader;
    CreateTextureLoaderFromTextureData(Desc, TexData, false, nullptr, &pLoader);
    EXPECT_EQ(pLoader, nullptr);
}

namespace
{

class ScopedWarningCapture final
{
public:
    ScopedWarningCapture() :
        m_PreviousCallback{DebugMessageCallback}
    {
        EXPECT_EQ(s_pActive, nullptr);
        s_pActive = this;
        SetDebugMessageCallback(Callback);
    }

    ~ScopedWarningCapture()
    {
        SetDebugMessageCallback(m_PreviousCallback);
        s_pActive = nullptr;
    }

    // clang-format off
    ScopedWarningCapture(const ScopedWarningCapture&)            = delete;
    ScopedWarningCapture& operator=(const ScopedWarningCapture&) = delete;
    // clang-format on

    std::vector<std::string> Warnings;

private:
    static void DILIGENT_CALL_TYPE Callback(DEBUG_MESSAGE_SEVERITY Severity,
                                            const Char*            Message,
                                            const Char*            Function,
                                            const Char*            File,
                                            int                    Line)
    {
        if (Severity == DEBUG_MESSAGE_SEVERITY_WARNING)
            s_pActive->Warnings.emplace_back(Message);
        if (s_pActive->m_PreviousCallback != nullptr)
            s_pActive->m_PreviousCallback(Severity, Message, Function, File, Line);
    }

    const DebugMessageCallbackType      m_PreviousCallback;
    static inline ScopedWarningCapture* s_pActive = nullptr;
};

template <typename T, size_t N>
void ExpectMipPixels(ITextureLoader& Loader, Uint32 Mip, Uint32 Width, Uint32 Height, Uint32 Components, const std::array<T, N>& Expected)
{
    ASSERT_EQ(N, Width * Height * Components);
    const TextureSubResData& Subres = Loader.GetSubresourceData(Mip);
    ASSERT_NE(Subres.pData, nullptr);
    for (Uint32 Row = 0; Row < Height; ++Row)
    {
        const T* pActual   = reinterpret_cast<const T*>(static_cast<const Uint8*>(Subres.pData) + Row * Subres.Stride);
        const T* pExpected = Expected.data() + Row * Width * Components;
        EXPECT_TRUE(std::equal(pExpected, pExpected + Width * Components, pActual)) << "Mip " << Mip << ", row " << Row;
    }
}

} // namespace

TEST(Tools_TextureLoader, PreservesSuppliedMipChainsAndRequestedCount)
{
    std::array<Uint8, 16>      Base;
    std::array<Uint8, 4>       Tail;
    const std::array<Uint8, 1> Last{191};
    Base.fill(17);
    Tail.fill(83);
    TextureSubResData Subresources[] = {{Base.data(), 4}, {Tail.data(), 2}, {Last.data(), 1}};
    for (Uint32 SuppliedCount : {2u, 3u})
    {
        for (Uint32 RequestedCount : {0u, 1u, 2u})
        {
            SCOPED_TRACE(SuppliedCount);
            SCOPED_TRACE(RequestedCount);
            TextureDesc Desc = MakeRGBA8TextureDesc();
            Desc.Width = Desc.Height = 4;
            Desc.Format              = TEX_FORMAT_R8_UNORM;
            Desc.MipLevels           = SuppliedCount;
            TextureLoadInfo LoadInfo;
            LoadInfo.MipLevels = RequestedCount;
            RefCntAutoPtr<ITextureLoader> pLoader;
            CreateTextureLoaderFromTextureData(Desc, TextureData{Subresources, SuppliedCount}, false, &LoadInfo, &pLoader);
            ASSERT_NE(pLoader, nullptr);
            const Uint32 ExpectedCount = RequestedCount != 0 ? RequestedCount : 3;
            ASSERT_EQ(pLoader->GetTextureDesc().MipLevels, ExpectedCount);
            EXPECT_EQ(pLoader->GetTextureData().NumSubresources, ExpectedCount);
            for (Uint32 Mip = 0; Mip < std::min(SuppliedCount, ExpectedCount); ++Mip)
            {
                EXPECT_EQ(pLoader->GetSubresourceData(Mip).pData, Subresources[Mip].pData);
                EXPECT_EQ(pLoader->GetSubresourceData(Mip).Stride, Subresources[Mip].Stride);
            }
        }
    }
}

TEST(Tools_TextureLoader, GeneratesMissingMipsFromLastSuppliedLevel)
{
    std::array<Uint8, 64> Base;
    Base.fill(99);
    const std::array<Uint8, 22> Tail{
        4, 4, 12, 12, 0xEE, 0xEE,
        4, 4, 12, 12, 0xEE, 0xEE,
        20, 20, 28, 28, 0xEE, 0xEE,
        20, 20, 28, 28};
    TextureSubResData Subresources[] = {{Base.data(), 8}, {Tail.data(), 6}};
    for (Uint32 RequestedCount : {0u, 3u})
    {
        SCOPED_TRACE(RequestedCount);
        TextureDesc Desc = MakeRGBA8TextureDesc();
        Desc.Width = Desc.Height = 8;
        Desc.Format              = TEX_FORMAT_R8_UNORM;
        Desc.MipLevels           = 2;
        TextureLoadInfo LoadInfo;
        LoadInfo.MipLevels = RequestedCount;
        RefCntAutoPtr<ITextureLoader> pLoader;
        CreateTextureLoaderFromTextureData(Desc, TextureData{Subresources, 2}, false, &LoadInfo, &pLoader);
        ASSERT_NE(pLoader, nullptr);
        ASSERT_EQ(pLoader->GetTextureDesc().MipLevels, RequestedCount != 0 ? RequestedCount : 4);
        EXPECT_EQ(pLoader->GetSubresourceData(0).pData, Base.data());
        EXPECT_EQ(pLoader->GetSubresourceData(1).pData, Tail.data());
        ExpectMipPixels(*pLoader, 2, 2, 2, 1, std::array<Uint8, 4>{4, 12, 20, 28});
        if (RequestedCount == 0)
            ExpectMipPixels(*pLoader, 3, 1, 1, 1, std::array<Uint8, 1>{16});
    }
}

TEST(Tools_TextureLoader, KeepsMissingMipsZeroWhenGenerationIsDisabled)
{
    std::array<Uint8, 16> Base;
    std::array<Uint8, 4>  Tail;
    Base.fill(77);
    Tail.fill(123);
    TextureSubResData Subresources[] = {{Base.data(), 4}, {Tail.data(), 2}};
    for (Uint32 SuppliedCount : {1u, 2u})
    {
        for (Uint32 RequestedCount : {0u, 3u})
        {
            SCOPED_TRACE(SuppliedCount);
            SCOPED_TRACE(RequestedCount);
            TextureDesc Desc = MakeRGBA8TextureDesc();
            Desc.Width = Desc.Height = 4;
            Desc.Format              = TEX_FORMAT_R8_UNORM;
            Desc.MipLevels           = SuppliedCount;
            TextureLoadInfo LoadInfo;
            LoadInfo.GenerateMips = False;
            LoadInfo.MipLevels    = RequestedCount;
            RefCntAutoPtr<ITextureLoader> pLoader;
            CreateTextureLoaderFromTextureData(Desc, TextureData{Subresources, SuppliedCount}, false, &LoadInfo, &pLoader);
            ASSERT_NE(pLoader, nullptr);
            ASSERT_EQ(pLoader->GetTextureDesc().MipLevels, 3u);
            for (Uint32 Mip = 0; Mip < SuppliedCount; ++Mip)
                EXPECT_EQ(pLoader->GetSubresourceData(Mip).pData, Subresources[Mip].pData);
            for (Uint32 Mip = SuppliedCount; Mip < 3; ++Mip)
            {
                const TextureSubResData& Loaded = pLoader->GetSubresourceData(Mip);
                ASSERT_NE(Loaded.pData, nullptr);
                const Uint32 Width = 4u >> Mip;
                for (Uint32 Row = 0; Row < Width; ++Row)
                {
                    const Uint8* pRow = static_cast<const Uint8*>(Loaded.pData) + Row * Loaded.Stride;
                    EXPECT_TRUE(std::all_of(pRow, pRow + Width, [](Uint8 Value) { return Value == 0; }));
                }
            }
        }
    }
}

TEST(Tools_TextureLoader, ProcessesSuppliedMipsLikeIndividualImages)
{
    std::array<Uint8, 64> Base;
    for (Uint32 i = 0; i < Base.size(); ++i)
        Base[i] = static_cast<Uint8>(i + 64);

    std::array<Uint8, 16> Tail{128, 64, 32, 128, 64, 32, 16, 255,
                               192, 128, 64, 128, 32, 64, 128, 255};

    const std::array<Uint8, 64>    OriginalBase   = Base;
    const std::array<Uint8, 16>    OriginalTail   = Tail;
    TextureSubResData              Subresources[] = {{Base.data(), 16}, {Tail.data(), 8}};
    std::array<TextureLoadInfo, 4> Cases;
    Cases[0].Format         = TEX_FORMAT_BGRA8_UNORM;
    Cases[0].FlipVertically = True;
    Cases[0].Swizzle        = TextureComponentMapping{
        TEXTURE_COMPONENT_SWIZZLE_B,
        TEXTURE_COMPONENT_SWIZZLE_G,
        TEXTURE_COMPONENT_SWIZZLE_R,
        TEXTURE_COMPONENT_SWIZZLE_A,
    };

    Cases[1].Format           = TEX_FORMAT_RGBA16_UNORM;
    Cases[2].PermultiplyAlpha = True;
    Cases[3].CompressMode     = TEXTURE_LOAD_COMPRESS_MODE_BC;
    for (size_t Case = 0; Case < Cases.size(); ++Case)
    {
        SCOPED_TRACE(Case);
        TextureDesc Desc = MakeRGBA8TextureDesc();
        Desc.Width = Desc.Height = 4;
        Desc.MipLevels           = 2;
        RefCntAutoPtr<ITextureLoader> pLoader;
        CreateTextureLoaderFromTextureData(Desc, TextureData{Subresources, 2}, false, &Cases[Case], &pLoader);
        ASSERT_NE(pLoader, nullptr);
        ASSERT_EQ(pLoader->GetTextureDesc().MipLevels, 3u);
        for (Uint32 Mip = 0; Mip < 2; ++Mip)
        {
            TextureDesc SingleDesc = Desc;
            SingleDesc.Width = SingleDesc.Height = 4u >> Mip;
            SingleDesc.MipLevels                 = 1;
            RefCntAutoPtr<ITextureLoader> pSingle;
            CreateTextureLoaderFromTextureData(SingleDesc, TextureData{&Subresources[Mip], 1}, false, &Cases[Case], &pSingle);
            ASSERT_NE(pSingle, nullptr);
            EXPECT_EQ(pLoader->GetTextureDesc().Format, pSingle->GetTextureDesc().Format);
            for (Uint32 LocalMip = 0; LocalMip < (Mip == 0 ? 1u : 2u); ++LocalMip)
            {
                const TextureSubResData& Actual   = pLoader->GetSubresourceData(Mip + LocalMip);
                const TextureSubResData& Expected = pSingle->GetSubresourceData(LocalMip);
                ASSERT_EQ(Actual.Stride, Expected.Stride);
                const Uint32 Rows      = Cases[Case].CompressMode != TEXTURE_LOAD_COMPRESS_MODE_NONE ? 1u : std::max(SingleDesc.Height >> LocalMip, 1u);
                const Uint8* pExpected = static_cast<const Uint8*>(Expected.pData);
                EXPECT_TRUE(std::equal(pExpected, pExpected + Expected.Stride * Rows, static_cast<const Uint8*>(Actual.pData)));
            }
        }
        EXPECT_EQ(Base, OriginalBase);
        EXPECT_EQ(Tail, OriginalTail);
    }
}

TEST(Tools_TextureLoader, KeepsLegacyAutomaticFormatInference)
{
    const auto Check = [](auto Base, auto Tail, TEXTURE_FORMAT SourceFormat, Bool IsSRGB, TEXTURE_FORMAT ExpectedFormat,
                          Uint32 SourceComponents, Uint32 OutputComponents, const auto& ExpectedBase, const auto& ExpectedTail,
                          const auto& ExpectedGenerated) {
        using T = typename decltype(Base)::value_type;
        SCOPED_TRACE(SourceFormat);
        SCOPED_TRACE(IsSRGB);
        TextureSubResData Subresources[] = {{Base.data(), 2 * SourceComponents * sizeof(T)}, {Tail.data(), SourceComponents * sizeof(T)}};
        for (Uint32 SuppliedCount : {1u, 2u})
        {
            TextureDesc Desc = MakeRGBA8TextureDesc();
            Desc.Format      = SourceFormat;
            Desc.MipLevels   = SuppliedCount;
            TextureLoadInfo LoadInfo;
            LoadInfo.IsSRGB = IsSRGB;
            RefCntAutoPtr<ITextureLoader> pLoader;
            CreateTextureLoaderFromTextureData(Desc, TextureData{Subresources, SuppliedCount}, false, &LoadInfo, &pLoader);
            ASSERT_NE(pLoader, nullptr);
            EXPECT_EQ(pLoader->GetTextureDesc().Format, ExpectedFormat);
            ASSERT_EQ(pLoader->GetTextureDesc().MipLevels, 2u);
            ExpectMipPixels(*pLoader, 0, 2, 2, OutputComponents, ExpectedBase);
            if (SuppliedCount == 2)
                ExpectMipPixels(*pLoader, 1, 1, 1, OutputComponents, ExpectedTail);
            else
            {
                const T* pGenerated = static_cast<const T*>(pLoader->GetSubresourceData(1).pData);
                for (Uint32 Component = 0; Component < OutputComponents; ++Component)
                    EXPECT_NEAR(pGenerated[Component], ExpectedGenerated[Component], IsSRGB ? 1.0 : 0.0);
            }
        }
    };
    Check(std::array<Uint8, 4>{7, 7, 7, 3}, std::array<Uint8, 1>{19}, TEX_FORMAT_R8_UINT, False, TEX_FORMAT_R8_UNORM, 1, 1,
          std::array<Uint8, 4>{7, 7, 7, 3}, std::array<Uint8, 1>{19}, std::array<Uint8, 1>{6});
    Check(std::array<Uint16, 4>{7, 7, 7, 3}, std::array<Uint16, 1>{19}, TEX_FORMAT_R16_UINT, False, TEX_FORMAT_R16_UNORM, 1, 1,
          std::array<Uint16, 4>{7, 7, 7, 3}, std::array<Uint16, 1>{19}, std::array<Uint16, 1>{6});
    Check(std::array<float, 12>{1, 2, 3, 5, 6, 7, 9, 10, 11, 13, 14, 15}, std::array<float, 3>{21, 22, 23},
          TEX_FORMAT_RGB32_FLOAT, False, TEX_FORMAT_RGBA32_FLOAT, 3, 4,
          std::array<float, 16>{1, 2, 3, 0, 5, 6, 7, 0, 9, 10, 11, 0, 13, 14, 15, 0},
          std::array<float, 4>{21, 22, 23, 0}, std::array<float, 4>{7, 8, 9, 0});
    Check(std::array<Uint8, 4>{0, 255, 0, 255}, std::array<Uint8, 1>{128}, TEX_FORMAT_R8_UNORM, True, TEX_FORMAT_RGBA8_UNORM_SRGB, 1, 4,
          std::array<Uint8, 16>{0, 0, 0, 255, 255, 255, 255, 255, 0, 0, 0, 255, 255, 255, 255, 255},
          std::array<Uint8, 4>{128, 128, 128, 255}, std::array<Uint8, 4>{188, 188, 188, 255});
}

TEST(Tools_TextureLoader, ClipsSingleMipAndWarnsForMultipleSuppliedMips)
{
    std::array<Uint8, 16> Base;
    Base.fill(17);
    const std::array<Uint8, 4> Tail{37, 37, 37, 37};
    TextureSubResData          Subresources[] = {{Base.data(), 4}, {Tail.data(), 2}};
    for (Uint32 SuppliedCount : {1u, 2u})
    {
        SCOPED_TRACE(SuppliedCount);
        TextureDesc Desc = MakeRGBA8TextureDesc();
        Desc.Width = Desc.Height = 4;
        Desc.Format              = TEX_FORMAT_R8_UNORM;
        Desc.MipLevels           = SuppliedCount;
        TextureLoadInfo LoadInfo;
        LoadInfo.UniformImageClipDim = 2;
        ScopedWarningCapture          Capture;
        RefCntAutoPtr<ITextureLoader> pLoader;
        CreateTextureLoaderFromTextureData(Desc, TextureData{Subresources, SuppliedCount}, false, &LoadInfo, &pLoader);
        ASSERT_NE(pLoader, nullptr);
        const bool Multiple = SuppliedCount > 1;
        ASSERT_EQ(Capture.Warnings.size(), Multiple ? 1u : 0u);
        if (Multiple)
            EXPECT_EQ(Capture.Warnings[0], "UniformImageClipDim is ignored when multiple mip levels are supplied.");
        EXPECT_EQ(pLoader->GetTextureDesc().Width, Multiple ? 4u : 2u);
        EXPECT_EQ(pLoader->GetTextureDesc().Height, Multiple ? 4u : 2u);
        ASSERT_EQ(pLoader->GetTextureDesc().MipLevels, Multiple ? 3u : 2u);
        EXPECT_EQ(*static_cast<const Uint8*>(pLoader->GetSubresourceData(Multiple ? 2 : 1).pData), Multiple ? 37u : 17u);
    }
}

TEST(Tools_TextureLoader, RoundTripsCubeAndArrayMipDataThroughDDS)
{
    struct TextureCase
    {
        RESOURCE_DIMENSION Type;
        Uint32             SliceCount;
    };
    const TextureCase Cases[] = {
        {RESOURCE_DIM_TEX_2D_ARRAY, 2},
        {RESOURCE_DIM_TEX_CUBE, 6},
        {RESOURCE_DIM_TEX_CUBE_ARRAY, 12},
    };
    for (TEXTURE_FORMAT Format : {TEX_FORMAT_RGBA8_UNORM, TEX_FORMAT_BC3_UNORM})
    {
        for (const TextureCase& Case : Cases)
        {
            for (Uint32 MipCount : {1u, 3u})
            {
                SCOPED_TRACE(Format);
                SCOPED_TRACE(Case.Type);
                SCOPED_TRACE(MipCount);
                TextureDesc Desc;
                Desc.Type  = Case.Type;
                Desc.Width = Desc.Height = 4;
                Desc.ArraySize           = Case.SliceCount;
                Desc.MipLevels           = MipCount;
                Desc.Format              = Format;
                std::vector<std::vector<Uint8>> Pixels(Desc.GetSubresourceCount());
                std::vector<TextureSubResData>  Subresources(Desc.GetSubresourceCount());
                for (Uint32 Slice = 0; Slice < Case.SliceCount; ++Slice)
                {
                    for (Uint32 Mip = 0; Mip < Desc.MipLevels; ++Mip)
                    {
                        const Uint32             Index = Slice * Desc.MipLevels + Mip;
                        const MipLevelProperties Props = GetMipLevelProperties(Desc, Mip);
                        Pixels[Index].resize(static_cast<size_t>(Props.DepthSliceSize));
                        for (size_t Byte = 0; Byte < Pixels[Index].size(); ++Byte)
                            Pixels[Index][Byte] = static_cast<Uint8>((Slice * 29 + Mip * 7 + Byte) & 0xFF);
                        Subresources[Index] = TextureSubResData{Pixels[Index].data(), Props.RowSize, Props.DepthSliceSize};
                    }
                }
                RefCntAutoPtr<DataBlobImpl>     pDDS    = DataBlobImpl::Create();
                RefCntAutoPtr<MemoryFileStream> pStream = MemoryFileStream::Create(pDDS);
                ASSERT_TRUE(WriteDDSToStream(pStream, Desc, TextureData{Subresources.data(), Desc.GetSubresourceCount()}));
                // Inspect legacy caps as well as DX10 fields: the local reader relies on the latter.
                std::array<Uint32, 37> HeaderWords;
                ASSERT_GE(pDDS->GetSize(), sizeof(HeaderWords));
                std::memcpy(HeaderWords.data(), pDDS->GetConstDataPtr(), sizeof(HeaderWords));
                const bool IsCube = Case.Type == RESOURCE_DIM_TEX_CUBE || Case.Type == RESOURCE_DIM_TEX_CUBE_ARRAY;
                EXPECT_EQ(HeaderWords[34], IsCube ? 0x4u : 0u) << "DX10 miscFlag";
                EXPECT_EQ(HeaderWords[35], IsCube ? Case.SliceCount / 6 : Case.SliceCount) << "DX10 arraySize";
                if (IsCube)
                {
                    EXPECT_EQ(HeaderWords[27], MipCount > 1 ? 0x00401008u : 0x00001008u) << "DDS caps";
                    EXPECT_EQ(HeaderWords[28], 0xFE00u) << "DDS caps2: cube and all six faces";
                }
                TextureLoadInfo LoadInfo;
                LoadInfo.GenerateMips = False;
                RefCntAutoPtr<ITextureLoader> pLoader;
                CreateTextureLoaderFromDataBlob(pDDS.RawPtr(), LoadInfo, &pLoader);
                ASSERT_NE(pLoader, nullptr);
                const TextureDesc& LoadedDesc = pLoader->GetTextureDesc();
                EXPECT_EQ(LoadedDesc.Type, Desc.Type);
                EXPECT_EQ(LoadedDesc.Width, Desc.Width);
                EXPECT_EQ(LoadedDesc.Height, Desc.Height);
                ASSERT_EQ(LoadedDesc.ArraySize, Desc.ArraySize);
                ASSERT_EQ(LoadedDesc.MipLevels, Desc.MipLevels);
                EXPECT_EQ(LoadedDesc.Format, Desc.Format);
                ASSERT_EQ(pLoader->GetTextureData().NumSubresources, Desc.GetSubresourceCount());
                for (Uint32 Slice = 0; Slice < Case.SliceCount; ++Slice)
                {
                    for (Uint32 Mip = 0; Mip < Desc.MipLevels; ++Mip)
                    {
                        SCOPED_TRACE(Slice);
                        SCOPED_TRACE(Mip);
                        const Uint32             Index  = Slice * Desc.MipLevels + Mip;
                        const TextureSubResData& Loaded = pLoader->GetSubresourceData(Mip, Slice);
                        ASSERT_NE(Loaded.pData, nullptr);
                        ASSERT_EQ(Loaded.Stride, Subresources[Index].Stride);
                        ASSERT_EQ(Loaded.DepthStride, Subresources[Index].DepthStride);
                        EXPECT_TRUE(std::equal(Pixels[Index].begin(), Pixels[Index].end(), static_cast<const Uint8*>(Loaded.pData)));
                    }
                }
            }
        }
    }
}

TEST(Tools_TextureLoader, RejectsDDSFormatsWithoutTextureFormatMapping)
{
    const std::array<Uint16, 4> Pixels{0x1234, 0x5678, 0x9ABC, 0xDEF0};
    TextureDesc                 Desc = MakeRGBA8TextureDesc();
    Desc.Format                      = TEX_FORMAT_R16_UNORM;
    TextureSubResData Subres{Pixels.data(), Desc.Width * sizeof(Uint16)};

    RefCntAutoPtr<DataBlobImpl>     pDDS    = DataBlobImpl::Create();
    RefCntAutoPtr<MemoryFileStream> pStream = MemoryFileStream::Create(pDDS);
    ASSERT_TRUE(WriteDDSToStream(pStream, Desc, TextureData{&Subres, 1}));

    // DDS magic is followed by the 124-byte header and the optional 20-byte DX10 header.
    constexpr size_t LegacyHeaderSize  = 4 + 124;
    constexpr size_t DX10HeaderSize    = 20;
    constexpr size_t PixelFormatOffset = 4 + 72;
    constexpr size_t DXGIFormatOffset  = LegacyHeaderSize;
    ASSERT_EQ(pDDS->GetSize(), LegacyHeaderSize + DX10HeaderSize + sizeof(Pixels));
    const Uint8* pDDSBytes = static_cast<const Uint8*>(pDDS->GetConstDataPtr());

    for (bool UseDX10Header : {true, false})
    {
        SCOPED_TRACE(UseDX10Header);
        std::vector<Uint8> Bytes{pDDSBytes, pDDSBytes + pDDS->GetSize()};
        if (!UseDX10Header)
        {
            Bytes.erase(Bytes.begin() + LegacyHeaderSize, Bytes.begin() + LegacyHeaderSize + DX10HeaderSize);
            // DDS_PIXELFORMAT: size, flags (DDS_LUMINANCE), FourCC, bit count, RGBA masks.
            const std::array<Uint32, 8> R16PixelFormat{32, 0x20000, 0, 16, 0xFFFF, 0, 0, 0};
            std::memcpy(Bytes.data() + PixelFormatOffset, R16PixelFormat.data(), sizeof(R16PixelFormat));
        }

        TextureLoadInfo LoadInfo;
        LoadInfo.GenerateMips = False;
        RefCntAutoPtr<ITextureLoader> pValidLoader;
        CreateTextureLoaderFromMemory(Bytes.data(), Bytes.size(), false, LoadInfo, &pValidLoader);
        ASSERT_NE(pValidLoader, nullptr);
        EXPECT_EQ(pValidLoader->GetTextureDesc().Format, Desc.Format);
        EXPECT_EQ(pValidLoader->GetTextureDesc().Width, Desc.Width);
        EXPECT_EQ(pValidLoader->GetTextureDesc().Height, Desc.Height);
        ASSERT_NE(pValidLoader->GetSubresourceData(0).pData, nullptr);
        EXPECT_EQ(std::memcmp(pValidLoader->GetSubresourceData(0).pData, Pixels.data(), sizeof(Pixels)), 0);
        pValidLoader.Release();

        // B4G4R4A4_UNORM has a known DDS bit count, but no Diligent texture format mapping.
        // Both formats use 16 bits per pixel, so the complete payload remains the same size.
        if (UseDX10Header)
        {
            constexpr Uint32 B4G4R4A4_UNORM = 115; // DXGI_FORMAT_B4G4R4A4_UNORM
            std::memcpy(Bytes.data() + DXGIFormatOffset, &B4G4R4A4_UNORM, sizeof(B4G4R4A4_UNORM));
        }
        else
        {
            // DDS_RGB | DDS_ALPHAPIXELS with legacy 4:4:4:4 channel masks.
            const std::array<Uint32, 8> B4G4R4A4PixelFormat{32, 0x41, 0, 16, 0x0F00, 0x00F0, 0x000F, 0xF000};
            std::memcpy(Bytes.data() + PixelFormatOffset, B4G4R4A4PixelFormat.data(), sizeof(B4G4R4A4PixelFormat));
        }

        Testing::TestingEnvironment::ErrorScope ExpectedErrors{
            "Failed to create texture loader from memory",
            "Unsupported DDS texture format (DXGI format 115)"};
        RefCntAutoPtr<ITextureLoader> pLoader;
        CreateTextureLoaderFromMemory(Bytes.data(), Bytes.size(), false, LoadInfo, &pLoader);
        EXPECT_EQ(pLoader, nullptr);
    }
}

namespace
{

struct KTX1TestHeader
{
    Uint32 Endianness            = 0x04030201;
    Uint32 GLType                = 0x1401; // GL_UNSIGNED_BYTE
    Uint32 GLTypeSize            = 1;
    Uint32 GLFormat              = 0x1908; // GL_RGBA
    Uint32 GLInternalFormat      = 0x8058; // GL_RGBA8
    Uint32 GLBaseInternalFormat  = 0x1908; // GL_RGBA
    Uint32 Width                 = 4;
    Uint32 Height                = 4;
    Uint32 Depth                 = 0;
    Uint32 NumberOfArrayElements = 0;
    Uint32 NumberOfFaces         = 1;
    Uint32 NumberOfMipmapLevels  = 1;
    Uint32 BytesOfKeyValueData   = 0;
};
static_assert(sizeof(KTX1TestHeader) == 52, "Unexpected KTX1 header size");

struct KTX1TestFile
{
    struct Subresource
    {
        size_t Offset;
        Uint32 RowStride;
        Uint32 DepthStride;
        Uint32 Size;
        Uint8  Value;
    };

    KTX1TestHeader           Header;
    std::vector<Uint8>       Bytes;
    std::vector<size_t>      MipOffsets;
    std::vector<Subresource> Subresources;

    void WriteHeader()
    {
        std::memcpy(Bytes.data() + 12, &Header, sizeof(Header));
    }
};

void AppendKTX1Word(std::vector<Uint8>& Bytes, Uint32 Value)
{
    for (Uint32 Byte = 0; Byte < sizeof(Value); ++Byte)
    {
        Bytes.push_back(static_cast<Uint8>(Value >> (Byte * 8)));
    }
}

KTX1TestFile MakeKTX1File(const KTX1TestHeader& Header = KTX1TestHeader{})
{
    const std::array<Uint8, 12> Identifier{0xAB, 0x4B, 0x54, 0x58, 0x20, 0x31, 0x31, 0xBB, 0x0D, 0x0A, 0x1A, 0x0A};
    KTX1TestFile                File;
    File.Header = Header;
    File.Bytes.assign(Identifier.begin(), Identifier.end());
    File.Bytes.resize(Identifier.size() + sizeof(Header));
    File.WriteHeader();
    if (Header.BytesOfKeyValueData != 0)
    {
        // One key/value pair: the key "k" and value "v", both null-terminated.
        AppendKTX1Word(File.Bytes, 4);
        File.Bytes.push_back('k');
        File.Bytes.push_back(0);
        File.Bytes.push_back('v');
        File.Bytes.push_back(0);
    }

    const Uint32 MipCount   = std::max(Header.NumberOfMipmapLevels, 1u);
    const Uint32 SliceCount = std::max(Header.NumberOfArrayElements, 1u) * Header.NumberOfFaces;
    const bool   IsBC1      = Header.GLInternalFormat == 0x83F0;           // GL_COMPRESSED_RGB_S3TC_DXT1_EXT
    const Uint32 Components = Header.GLInternalFormat == 0x8229 ? 1u : 4u; // GL_R8 or GL_RGBA8
    File.Subresources.resize(MipCount * SliceCount);
    for (Uint32 Mip = 0; Mip < MipCount; ++Mip)
    {
        const Uint32 Width       = std::max(Header.Width >> Mip, 1u);
        const Uint32 Height      = std::max(Header.Height >> Mip, 1u);
        const Uint32 Depth       = std::max(Header.Depth >> Mip, 1u);
        const Uint32 RowStride   = IsBC1 ? ((Width + 3) / 4) * 8 : (Width * Components + 3) & ~3u;
        const Uint32 DepthStride = RowStride * (IsBC1 ? (Height + 3) / 4 : Height);
        const Uint32 SliceSize   = DepthStride * Depth;
        File.MipOffsets.push_back(File.Bytes.size());
        // For a non-array cube imageSize is per face; otherwise it covers the entire mip.
        const bool IsNonArrayCube = Header.NumberOfFaces == 6 && Header.NumberOfArrayElements == 0;
        AppendKTX1Word(File.Bytes, SliceSize * (IsNonArrayCube ? 1u : SliceCount));
        for (Uint32 Slice = 0; Slice < SliceCount; ++Slice)
        {
            const Uint8 Value                         = static_cast<Uint8>(1 + Mip * 16 + Slice);
            File.Subresources[Slice * MipCount + Mip] = {File.Bytes.size(), RowStride, DepthStride, SliceSize, Value};
            File.Bytes.insert(File.Bytes.end(), SliceSize, Value);
        }
    }
    return File;
}

void ExpectKTX1Rejected(const KTX1TestFile& File, size_t DataSize, Uint32 RequestedMipCount = 0)
{
    ASSERT_LE(DataSize, File.Bytes.size());
    TextureLoadInfo LoadInfo;
    LoadInfo.GenerateMips = False;
    LoadInfo.MipLevels    = RequestedMipCount;
    Testing::TestingEnvironment::ErrorScope ExpectedErrors{"Failed to create texture loader from memory", "KTX"};
    RefCntAutoPtr<ITextureLoader>           pLoader;
    CreateTextureLoaderFromMemory(File.Bytes.data(), DataSize, false, LoadInfo, &pLoader);
    EXPECT_EQ(pLoader, nullptr);
}

} // namespace

TEST(Tools_TextureLoader, RejectsTruncatedKTXHeaderAndImageData)
{
    const KTX1TestFile File = MakeKTX1File();
    ASSERT_EQ(File.Bytes.size(), 132u);
    // Retain the complete allocation: only the caller's declared length is truncated.
    // In particular, 68 bytes contain the header and imageSize but no 4x4 RGBA8 pixels.
    for (size_t DataSize : {12u, 63u, 64u, 67u, 68u, 69u, 131u})
    {
        SCOPED_TRACE(DataSize);
        ExpectKTX1Rejected(File, DataSize);
    }
}

TEST(Tools_TextureLoader, RejectsTruncatedKTXMetadata)
{
    KTX1TestHeader Header;
    Header.BytesOfKeyValueData = 8;
    KTX1TestFile File          = MakeKTX1File(Header);
    for (size_t DataSize : {64u, 67u, 71u, 72u, 75u})
    {
        SCOPED_TRACE(DataSize);
        ExpectKTX1Rejected(File, DataSize);
    }
    File.Header.BytesOfKeyValueData = std::numeric_limits<Uint32>::max();
    File.WriteHeader();
    ExpectKTX1Rejected(File, File.Bytes.size());
}

TEST(Tools_TextureLoader, RejectsTruncatedKTXLaterAndSkippedMips)
{
    KTX1TestHeader Header;
    Header.NumberOfMipmapLevels = 3;
    const KTX1TestFile File     = MakeKTX1File(Header);
    for (Uint32 RequestedMipCount : {0u, 1u})
    {
        for (Uint32 Mip = 1; Mip < Header.NumberOfMipmapLevels; ++Mip)
        {
            const KTX1TestFile::Subresource& Subres = File.Subresources[Mip];
            for (size_t DataSize : {File.MipOffsets[Mip], File.MipOffsets[Mip] + 3,
                                    Subres.Offset, Subres.Offset + Subres.Size - 1})
            {
                SCOPED_TRACE(RequestedMipCount);
                SCOPED_TRACE(Mip);
                SCOPED_TRACE(DataSize);
                ExpectKTX1Rejected(File, DataSize, RequestedMipCount);
            }
        }
    }
}

TEST(Tools_TextureLoader, RejectsTruncatedKTXLayersFacesAndDepth)
{
    for (Uint32 ArrayElements : {0u, 2u})
    {
        for (Uint32 Faces : {1u, 6u})
        {
            KTX1TestHeader Header;
            Header.NumberOfArrayElements = ArrayElements;
            Header.NumberOfFaces         = Faces;
            const KTX1TestFile File      = MakeKTX1File(Header);
            for (const KTX1TestFile::Subresource& Subres : File.Subresources)
            {
                SCOPED_TRACE(ArrayElements);
                SCOPED_TRACE(Faces);
                SCOPED_TRACE(Subres.Offset);
                ExpectKTX1Rejected(File, Subres.Offset + Subres.Size - 1);
            }
        }
    }
    KTX1TestHeader Header;
    Header.Depth            = 4;
    const KTX1TestFile File = MakeKTX1File(Header);
    ExpectKTX1Rejected(File, File.Subresources[0].Offset + File.Subresources[0].DepthStride);
    ExpectKTX1Rejected(File, File.Bytes.size() - 1);

    Header.Depth                  = 0;
    Header.GLType                 = 0;
    Header.GLFormat               = 0;
    Header.GLInternalFormat       = 0x83F0; // GL_COMPRESSED_RGB_S3TC_DXT1_EXT
    Header.GLBaseInternalFormat   = 0x1907; // GL_RGB
    const KTX1TestFile Compressed = MakeKTX1File(Header);
    ExpectKTX1Rejected(Compressed, Compressed.Bytes.size() - 1);
}

TEST(Tools_TextureLoader, RejectsTruncatedKTXRowPadding)
{
    KTX1TestHeader Header;
    Header.GLFormat = Header.GLBaseInternalFormat = 0x1903; // GL_RED
    Header.GLInternalFormat                       = 0x8229; // GL_R8
    Header.Width                                  = 1;
    Header.Height                                 = 2;
    const KTX1TestFile File                       = MakeKTX1File(Header);
    ASSERT_EQ(File.Bytes.size(), 76u);
    for (size_t DataSize : {69u, 71u, 73u, 75u})
    {
        SCOPED_TRACE(DataSize);
        ExpectKTX1Rejected(File, DataSize);
    }
}

TEST(Tools_TextureLoader, RejectsInvalidKTXImageSizesAndExtremeHeaders)
{
    const KTX1TestFile Original = MakeKTX1File();
    for (Uint32 ImageSize : {0u, 63u, 65u, std::numeric_limits<Uint32>::max()})
    {
        KTX1TestFile File = Original;
        std::memcpy(File.Bytes.data() + File.MipOffsets[0], &ImageSize, sizeof(ImageSize));
        SCOPED_TRACE(ImageSize);
        ExpectKTX1Rejected(File, File.Bytes.size());
    }
    const Uint32                  Max = std::numeric_limits<Uint32>::max();
    std::array<KTX1TestHeader, 6> Headers;
    Headers[0].Endianness = 0x01020304;
    Headers[1].Width = Headers[1].Height = Max;
    Headers[2].Depth                     = Max;
    Headers[3].NumberOfArrayElements     = Max;
    Headers[4].NumberOfArrayElements     = 0x80000000u;
    Headers[4].NumberOfFaces             = 6;
    Headers[5].NumberOfMipmapLevels      = Max;
    for (size_t Case = 0; Case < Headers.size(); ++Case)
    {
        SCOPED_TRACE(Case);
        KTX1TestFile File = Original;
        File.Header       = Headers[Case];
        File.WriteHeader();
        ExpectKTX1Rejected(File, File.Bytes.size(), 1);
    }
}

TEST(Tools_TextureLoader, LoadsValidKTXFormatsMipsArraysCubesAndDepth)
{
    struct TextureCase
    {
        Uint32             Width;
        Uint32             Height;
        Uint32             Depth;
        Uint32             ArrayElements;
        Uint32             Faces;
        TEXTURE_FORMAT     Format;
        RESOURCE_DIMENSION Type;
    };
    const TextureCase Cases[] = {
        {4, 4, 0, 0, 1, TEX_FORMAT_RGBA8_UNORM, RESOURCE_DIM_TEX_2D},
        {4, 4, 0, 0, 1, TEX_FORMAT_BC1_UNORM, RESOURCE_DIM_TEX_2D},
        {4, 4, 0, 2, 1, TEX_FORMAT_RGBA8_UNORM, RESOURCE_DIM_TEX_2D_ARRAY},
        {4, 4, 0, 0, 6, TEX_FORMAT_RGBA8_UNORM, RESOURCE_DIM_TEX_CUBE},
        {4, 4, 0, 2, 6, TEX_FORMAT_RGBA8_UNORM, RESOURCE_DIM_TEX_CUBE_ARRAY},
        {4, 4, 4, 0, 1, TEX_FORMAT_RGBA8_UNORM, RESOURCE_DIM_TEX_3D},
        {1, 2, 0, 0, 1, TEX_FORMAT_R8_UNORM, RESOURCE_DIM_TEX_2D},
        {3, 2, 0, 0, 1, TEX_FORMAT_R8_UNORM, RESOURCE_DIM_TEX_2D},
    };
    for (const TextureCase& Case : Cases)
    {
        KTX1TestHeader Header;
        Header.Width                 = Case.Width;
        Header.Height                = Case.Height;
        Header.Depth                 = Case.Depth;
        Header.NumberOfArrayElements = Case.ArrayElements;
        Header.NumberOfFaces         = Case.Faces;
        Header.NumberOfMipmapLevels  = Case.Format == TEX_FORMAT_R8_UNORM ? 2 : 3;
        Header.BytesOfKeyValueData   = 8;
        if (Case.Format == TEX_FORMAT_BC1_UNORM)
        {
            Header.GLType = Header.GLFormat = 0;
            Header.GLInternalFormat         = 0x83F0; // GL_COMPRESSED_RGB_S3TC_DXT1_EXT
            Header.GLBaseInternalFormat     = 0x1907; // GL_RGB
        }
        else if (Case.Format == TEX_FORMAT_R8_UNORM)
        {
            Header.GLFormat = Header.GLBaseInternalFormat = 0x1903; // GL_RED
            Header.GLInternalFormat                       = 0x8229; // GL_R8
        }
        const KTX1TestFile File = MakeKTX1File(Header);
        for (Uint32 RequestedMipCount : {0u, 1u})
        {
            SCOPED_TRACE(Case.Format);
            SCOPED_TRACE(Case.Type);
            SCOPED_TRACE(Case.Width);
            SCOPED_TRACE(RequestedMipCount);
            TextureLoadInfo LoadInfo;
            LoadInfo.GenerateMips = False;
            LoadInfo.MipLevels    = RequestedMipCount;
            RefCntAutoPtr<ITextureLoader> pLoader;
            CreateTextureLoaderFromMemory(File.Bytes.data(), File.Bytes.size(), false, LoadInfo, &pLoader);
            ASSERT_NE(pLoader, nullptr);
            const TextureDesc& Desc = pLoader->GetTextureDesc();
            EXPECT_EQ(Desc.Type, Case.Type);
            EXPECT_EQ(Desc.Format, Case.Format);
            EXPECT_EQ(Desc.Width, Case.Width);
            EXPECT_EQ(Desc.Height, Case.Height);
            EXPECT_EQ(Desc.GetDepth(), std::max(Case.Depth, 1u));
            EXPECT_EQ(Desc.GetArraySize(), std::max(Case.ArrayElements, 1u) * Case.Faces);
            const Uint32 MipCount = RequestedMipCount == 0 ? Header.NumberOfMipmapLevels : RequestedMipCount;
            ASSERT_EQ(Desc.MipLevels, MipCount);
            ASSERT_EQ(pLoader->GetTextureData().NumSubresources, MipCount * Desc.GetArraySize());
            for (Uint32 Slice = 0; Slice < Desc.GetArraySize(); ++Slice)
            {
                for (Uint32 Mip = 0; Mip < MipCount; ++Mip)
                {
                    const KTX1TestFile::Subresource& Expected = File.Subresources[Slice * Header.NumberOfMipmapLevels + Mip];
                    const TextureSubResData&         Actual   = pLoader->GetSubresourceData(Mip, Slice);
                    ASSERT_EQ(Actual.pData, File.Bytes.data() + Expected.Offset);
                    EXPECT_EQ(Actual.Stride, Expected.RowStride);
                    EXPECT_EQ(Actual.DepthStride, Expected.DepthStride);
                    const Uint8* pPixels = static_cast<const Uint8*>(Actual.pData);
                    ASSERT_NE(pPixels, nullptr);
                    EXPECT_TRUE(std::all_of(pPixels, pPixels + Expected.Size, [&](Uint8 Value) { return Value == Expected.Value; }));
                }
            }
        }
    }
}

namespace
{

class CountingImageAllocator final : public IMemoryAllocator
{
public:
    void* Allocate(size_t Size, const Char* Description, const char* FileName, const Int32 LineNumber) override
    {
        return RecordAllocation(DefaultRawMemoryAllocator::GetAllocator().Allocate(Size, Description, FileName, LineNumber), Size);
    }

    void Free(void* Ptr) override
    {
        if (Ptr != nullptr)
        {
            ++FreeCount;
        }
        DefaultRawMemoryAllocator::GetAllocator().Free(Ptr);
    }

    void* AllocateAligned(size_t Size, size_t Alignment, const Char* Description, const char* FileName, const Int32 LineNumber) override
    {
        return RecordAllocation(DefaultRawMemoryAllocator::GetAllocator().AllocateAligned(Size, Alignment, Description, FileName, LineNumber), Size);
    }

    void FreeAligned(void* Ptr) override
    {
        if (Ptr != nullptr)
        {
            ++FreeCount;
        }
        DefaultRawMemoryAllocator::GetAllocator().FreeAligned(Ptr);
    }

    size_t AllocationCount    = 0;
    size_t FreeCount          = 0;
    void*  pLastAllocation    = nullptr;
    size_t LastAllocationSize = 0;

private:
    void* RecordAllocation(void* Ptr, size_t Size)
    {
        if (Ptr != nullptr)
        {
            ++AllocationCount;
            pLastAllocation    = Ptr;
            LastAllocationSize = Size;
        }
        return Ptr;
    }
};

struct STBTestImage
{
    ImageDesc          Desc;
    TEXTURE_FORMAT     TextureFormat;
    std::vector<Uint8> Bytes;
    std::vector<Uint8> Pixels;
};

STBTestImage MakeSTBTestImage(IMAGE_FILE_FORMAT Format)
{
    STBTestImage File;
    File.Desc.Width = File.Desc.Height = 2;
    if (Format == IMAGE_FILE_FORMAT_TGA)
    {
        File.Desc.ComponentType = VT_UINT8;
        File.Desc.NumComponents = 4;
        File.TextureFormat      = TEX_FORMAT_RGBA8_UNORM;
        File.Pixels             = {
            128, 64, 32, 128, 255, 127, 31, 255,
            90, 60, 30, 0, 24, 12, 6, 85};

        File.Bytes.resize(18, 0); // TGA header: no image ID or color map, zero origin.
        File.Bytes[2]  = 2;       // Uncompressed true-color image.
        File.Bytes[12] = 2;       // Width, little-endian.
        File.Bytes[14] = 2;       // Height, little-endian.
        File.Bytes[16] = 32;      // Bits per pixel.
        File.Bytes[17] = 0x28;    // Top-left origin, eight alpha bits.
        for (size_t Pixel = 0; Pixel < File.Pixels.size(); Pixel += 4)
        {
            // TGA stores color channels in BGRA order.
            File.Bytes.push_back(File.Pixels[Pixel + 2]);
            File.Bytes.push_back(File.Pixels[Pixel + 1]);
            File.Bytes.push_back(File.Pixels[Pixel + 0]);
            File.Bytes.push_back(File.Pixels[Pixel + 3]);
        }
    }
    else
    {
        File.Desc.ComponentType  = VT_FLOAT32;
        File.Desc.NumComponents  = 3;
        File.TextureFormat       = TEX_FORMAT_RGB32_FLOAT;
        const std::string Header = "#?RADIANCE\nFORMAT=32-bit_rle_rgbe\n\n-Y 2 +X 2\n";
        File.Bytes.assign(Header.begin(), Header.end());
        // Widths below eight use flat RGBE pixels instead of scanline RLE.
        const std::array<Uint8, 16> RGBE{
            128, 64, 32, 129, 32, 64, 128, 130,
            0, 0, 0, 0, 96, 64, 32, 128};
        File.Bytes.insert(File.Bytes.end(), RGBE.begin(), RGBE.end());
        const std::array<float, 12> Pixels{
            1.0f, 0.5f, 0.25f, 0.5f, 1.0f, 2.0f,
            0.0f, 0.0f, 0.0f, 0.375f, 0.25f, 0.125f};
        File.Pixels.resize(sizeof(Pixels));
        std::memcpy(File.Pixels.data(), Pixels.data(), sizeof(Pixels));
    }
    File.Desc.RowStride = File.Desc.Width * File.Desc.NumComponents * GetValueSize(File.Desc.ComponentType);
    return File;
}

void ExpectSTBBlobContents(const IDataBlob& Blob, const std::vector<Uint8>& Pixels)
{
    ASSERT_EQ(Blob.GetSize(), Pixels.size());
    const Uint8* pData = static_cast<const Uint8*>(Blob.GetConstDataPtr());
    ASSERT_NE(pData, nullptr);
    EXPECT_TRUE(std::equal(Pixels.begin(), Pixels.end(), pData));
}

} // namespace

TEST(Tools_TextureLoader, RetainsDecodedSTBPixelStorage)
{
    for (IMAGE_FILE_FORMAT Format : {IMAGE_FILE_FORMAT_TGA, IMAGE_FILE_FORMAT_HDR})
    {
        for (bool UseCustomAllocator : {false, true})
        {
            SCOPED_TRACE(static_cast<int>(Format));
            SCOPED_TRACE(UseCustomAllocator);
            STBTestImage    File     = MakeSTBTestImage(Format);
            const ImageDesc Metadata = Image::GetDesc(Format, File.Bytes.data(), File.Bytes.size());
            EXPECT_EQ(Metadata.Width, File.Desc.Width);
            EXPECT_EQ(Metadata.Height, File.Desc.Height);
            EXPECT_EQ(Metadata.ComponentType, File.Desc.ComponentType);
            EXPECT_EQ(Metadata.NumComponents, File.Desc.NumComponents);
            EXPECT_EQ(Metadata.RowStride, 0u);

            CountingImageAllocator Allocator;
            ImageLoadInfo          LoadInfo;
            LoadInfo.Format     = Format;
            LoadInfo.pAllocator = UseCustomAllocator ? &Allocator : nullptr;
            RefCntAutoPtr<Image> pImage;
            Image::CreateFromMemory(File.Bytes.data(), File.Bytes.size(), LoadInfo, &pImage);
            ASSERT_NE(pImage, nullptr);
            const ImageDesc& Desc = pImage->GetDesc();
            EXPECT_EQ(Desc.Width, File.Desc.Width);
            EXPECT_EQ(Desc.Height, File.Desc.Height);
            EXPECT_EQ(Desc.ComponentType, File.Desc.ComponentType);
            EXPECT_EQ(Desc.NumComponents, File.Desc.NumComponents);
            EXPECT_EQ(Desc.RowStride, File.Desc.RowStride);

            RefCntAutoPtr<IDataBlob> pPixels;
            pImage->GetData()->QueryInterface(IID_DataBlob, pPixels.GetAddressOfEmpty());
            ASSERT_NE(pPixels, nullptr);
            ASSERT_EQ(pPixels->GetSize(), File.Pixels.size());
            const void* pPixelData = pPixels->GetConstDataPtr();
            if (UseCustomAllocator)
            {
                EXPECT_GT(Allocator.AllocationCount, 0u);
                EXPECT_EQ(Allocator.LastAllocationSize, File.Pixels.size());
                EXPECT_EQ(pPixelData, Allocator.pLastAllocation);
            }
            else
            {
                EXPECT_EQ(Allocator.AllocationCount, 0u);
            }

            TextureLoadInfo TexLoadInfo;
            TexLoadInfo.Format       = File.TextureFormat;
            TexLoadInfo.MipLevels    = 1;
            TexLoadInfo.GenerateMips = False;
            RefCntAutoPtr<ITextureLoader> pLoader;
            CreateTextureLoaderFromImage(pImage, TexLoadInfo, &pLoader);
            ASSERT_NE(pLoader, nullptr);
            EXPECT_EQ(pLoader->GetTextureDesc().Width, File.Desc.Width);
            EXPECT_EQ(pLoader->GetTextureDesc().Height, File.Desc.Height);
            EXPECT_EQ(pLoader->GetTextureDesc().Format, File.TextureFormat);
            EXPECT_EQ(pLoader->GetTextureDesc().MipLevels, 1u);
            EXPECT_EQ(pLoader->GetSubresourceData(0).pData, pPixelData);
            EXPECT_EQ(pLoader->GetSubresourceData(0).Stride, File.Desc.RowStride);

            std::fill(File.Bytes.begin(), File.Bytes.end(), Uint8{0});
            pImage.Release();
            EXPECT_EQ(Allocator.FreeCount, 0u);
            ExpectSTBBlobContents(*pPixels, File.Pixels);
            pPixels.Release();
            EXPECT_EQ(Allocator.FreeCount, 0u);
            const Uint8* pRetainedPixels = static_cast<const Uint8*>(pLoader->GetSubresourceData(0).pData);
            ASSERT_EQ(pRetainedPixels, pPixelData);
            EXPECT_TRUE(std::equal(File.Pixels.begin(), File.Pixels.end(), pRetainedPixels));
            pLoader.Release();
            EXPECT_EQ(Allocator.FreeCount, Allocator.AllocationCount);
        }
    }
}

TEST(Tools_TextureLoader, PremultipliesDecodedTGAAlpha)
{
    const STBTestImage       File = MakeSTBTestImage(IMAGE_FILE_FORMAT_TGA);
    const std::vector<Uint8> Expected{
        64, 32, 16, 128, 255, 127, 31, 255,
        0, 0, 0, 0, 8, 4, 2, 85};
    for (bool UseCustomAllocator : {false, true})
    {
        SCOPED_TRACE(UseCustomAllocator);
        CountingImageAllocator Allocator;
        ImageLoadInfo          LoadInfo;
        LoadInfo.Format           = IMAGE_FILE_FORMAT_TGA;
        LoadInfo.PermultiplyAlpha = true;
        LoadInfo.pAllocator       = UseCustomAllocator ? &Allocator : nullptr;
        RefCntAutoPtr<Image> pImage;
        Image::CreateFromMemory(File.Bytes.data(), File.Bytes.size(), LoadInfo, &pImage);
        ASSERT_NE(pImage, nullptr);
        ASSERT_EQ(pImage->GetData()->GetSize(), Expected.size());
        ExpectSTBBlobContents(*pImage->GetData(), Expected);
        pImage.Release();
        if (UseCustomAllocator)
        {
            EXPECT_GT(Allocator.AllocationCount, 0u);
        }
        else
        {
            EXPECT_EQ(Allocator.AllocationCount, 0u);
        }
        EXPECT_EQ(Allocator.FreeCount, Allocator.AllocationCount);
    }
}
