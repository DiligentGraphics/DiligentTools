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

#include "TestingEnvironment.hpp"
#include "gtest/gtest.h"

#include <algorithm>
#include <array>
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
