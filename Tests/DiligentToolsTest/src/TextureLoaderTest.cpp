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
    CreateTextureLoaderFromTextureData(Desc, TexData, false, &pLoader);
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
    CreateTextureLoaderFromTextureData(Desc, TexData, true, &pLoader);
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
    CreateTextureLoaderFromTextureData(Desc, TexData, false, &pLoader);
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

TEST(Tools_TextureLoader, RejectsInvalidTextureData)
{
    TextureDesc Desc = MakeRGBA8TextureDesc();
    TextureData TexData{};

    Testing::TestingEnvironment::ErrorScope ExpectedErrors{
        "Failed to create texture loader from texture data",
        "Texture data must contain exactly one entry"};

    RefCntAutoPtr<ITextureLoader> pLoader;
    CreateTextureLoaderFromTextureData(Desc, TexData, false, &pLoader);
    EXPECT_EQ(pLoader, nullptr);
}
