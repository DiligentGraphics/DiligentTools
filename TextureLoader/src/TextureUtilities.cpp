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

#include "TextureUtilities.h"

#include <algorithm>
#include <vector>
#include <limits>

#include "TextureLoader.h"
#include "RefCntAutoPtr.hpp"
#include "ColorConversion.h"
#include "GraphicsAccessories.hpp"

namespace Diligent
{

template <typename SrcChannelType, typename DstChannelType>
DstChannelType ConvertChannel(SrcChannelType Val)
{
    return Val;
}

template <>
Uint16 ConvertChannel<Uint8, Uint16>(Uint8 Val)
{
    return static_cast<Uint16>(Val) << 8u;
}
template <>
Uint32 ConvertChannel<Uint8, Uint32>(Uint8 Val)
{
    return static_cast<Uint32>(Val) << 24u;
}

template <>
Uint8 ConvertChannel<Uint16, Uint8>(Uint16 Val)
{
    return static_cast<Uint8>(Val >> 8u);
}
template <>
Uint32 ConvertChannel<Uint16, Uint32>(Uint16 Val)
{
    return static_cast<Uint32>(Val) << 16u;
}

template <>
Uint8 ConvertChannel<Uint32, Uint8>(Uint32 Val)
{
    return static_cast<Uint8>(Val >> 24u);
}
template <>
Uint16 ConvertChannel<Uint32, Uint16>(Uint32 Val)
{
    return static_cast<Uint16>(Val >> 16u);
}

template <typename SrcChannelType, typename DstChannelType>
void CopyPixelsImpl(const CopyPixelsAttribs& Attribs)
{
    VERIFY_EXPR(sizeof(SrcChannelType) == Attribs.SrcComponentSize);
    VERIFY_EXPR(sizeof(DstChannelType) == Attribs.DstComponentSize);

    auto ProcessRows = [&Attribs](auto&& Handler) {
        for (size_t row = 0; row < size_t{Attribs.Height}; ++row)
        {
            size_t src_row = Attribs.FlipVertically ? size_t{Attribs.Height} - row - 1 : row;
            // clang-format off
            const auto* pSrcRow = reinterpret_cast<const SrcChannelType*>((static_cast<const Uint8*>(Attribs.pSrcPixels) + size_t{Attribs.SrcStride} * src_row));
            auto*       pDstRow = reinterpret_cast<      DstChannelType*>((static_cast<      Uint8*>(Attribs.pDstPixels) + size_t{Attribs.DstStride} * row));
            // clang-format on
            Handler(pSrcRow, pDstRow);
        }
    };

    const bool SwizzleRequired =
        (Attribs.DstCompCount >= 1 && Attribs.Swizzle.R != TEXTURE_COMPONENT_SWIZZLE_IDENTITY && Attribs.Swizzle.R != TEXTURE_COMPONENT_SWIZZLE_R) ||
        (Attribs.DstCompCount >= 2 && Attribs.Swizzle.G != TEXTURE_COMPONENT_SWIZZLE_IDENTITY && Attribs.Swizzle.G != TEXTURE_COMPONENT_SWIZZLE_G) ||
        (Attribs.DstCompCount >= 3 && Attribs.Swizzle.B != TEXTURE_COMPONENT_SWIZZLE_IDENTITY && Attribs.Swizzle.B != TEXTURE_COMPONENT_SWIZZLE_B) ||
        (Attribs.DstCompCount >= 4 && Attribs.Swizzle.A != TEXTURE_COMPONENT_SWIZZLE_IDENTITY && Attribs.Swizzle.A != TEXTURE_COMPONENT_SWIZZLE_A);

    const auto SrcRowSize = Attribs.Width * Attribs.SrcComponentSize * Attribs.SrcCompCount;
    const auto DstRowSize = Attribs.Width * Attribs.DstComponentSize * Attribs.DstCompCount;
    if (SrcRowSize == DstRowSize && !SwizzleRequired)
    {
        if (SrcRowSize == Attribs.SrcStride &&
            DstRowSize == Attribs.DstStride &&
            !Attribs.FlipVertically)
        {
            memcpy(Attribs.pDstPixels, Attribs.pSrcPixels, size_t{SrcRowSize} * size_t{Attribs.Height});
        }
        else
        {
            ProcessRows([SrcRowSize](auto* pSrcRow, auto* pDstRow) {
                memcpy(pDstRow, pSrcRow, SrcRowSize);
            });
        }
    }
    else if (Attribs.DstCompCount < Attribs.SrcCompCount && !SwizzleRequired)
    {
        ProcessRows([&Attribs](auto* pSrcRow, auto* pDstRow) {
            for (size_t col = 0; col < size_t{Attribs.Width}; ++col)
            {
                auto*       pDst = pDstRow + col * Attribs.DstCompCount;
                const auto* pSrc = pSrcRow + col * Attribs.SrcCompCount;
                for (size_t c = 0; c < Attribs.DstCompCount; ++c)
                    pDst[c] = ConvertChannel<SrcChannelType, DstChannelType>(pSrc[c]);
            }
        });
    }
    else
    {
        static constexpr int SrcCompOffset_ZERO = -1;
        static constexpr int SrcCompOffset_ONE  = -2;

        auto GetSrcCompOffset = [&Attribs](TEXTURE_COMPONENT_SWIZZLE Swizzle, int IdentityOffset) {
            int SrcCompOffset = SrcCompOffset_ZERO;
            switch (Swizzle)
            {
                // clang-format off
                case TEXTURE_COMPONENT_SWIZZLE_IDENTITY: SrcCompOffset = IdentityOffset;     break;
                case TEXTURE_COMPONENT_SWIZZLE_ZERO:     SrcCompOffset = SrcCompOffset_ZERO; break;
                case TEXTURE_COMPONENT_SWIZZLE_ONE:      SrcCompOffset = SrcCompOffset_ONE;  break;
                case TEXTURE_COMPONENT_SWIZZLE_R:        SrcCompOffset = 0;                  break;
                case TEXTURE_COMPONENT_SWIZZLE_G:        SrcCompOffset = 1;                  break;
                case TEXTURE_COMPONENT_SWIZZLE_B:        SrcCompOffset = 2;                  break;
                case TEXTURE_COMPONENT_SWIZZLE_A:        SrcCompOffset = 3;                  break;
                // clang-format on
                default:
                    UNEXPECTED("Unexpected swizzle value");
            }
            if (SrcCompOffset >= static_cast<int>(Attribs.SrcCompCount))
                SrcCompOffset = SrcCompOffset_ZERO;
            return SrcCompOffset;
        };

        const int SrcCompOffsets[4] = {
            GetSrcCompOffset(Attribs.Swizzle.R, 0),
            GetSrcCompOffset(Attribs.Swizzle.G, 1),
            GetSrcCompOffset(Attribs.Swizzle.B, 2),
            GetSrcCompOffset(Attribs.Swizzle.A, 3)};

        ProcessRows([&Attribs, &SrcCompOffsets](auto* pSrcRow, auto* pDstRow) {
            for (size_t col = 0; col < size_t{Attribs.Width}; ++col)
            {
                auto*       pDst = pDstRow + col * Attribs.DstCompCount;
                const auto* pSrc = pSrcRow + col * Attribs.SrcCompCount;

                for (size_t c = 0; c < Attribs.DstCompCount; ++c)
                {
                    const int SrcCompOffset = SrcCompOffsets[c];

                    pDst[c] = (SrcCompOffset >= 0) ?
                        ConvertChannel<SrcChannelType, DstChannelType>(pSrc[SrcCompOffset]) :
                        (SrcCompOffset == SrcCompOffset_ZERO ? 0 : std::numeric_limits<DstChannelType>::max());
                }
            }
        });
    }
}

void CopyPixels(const CopyPixelsAttribs& Attribs)
{
    DEV_CHECK_ERR(Attribs.Width > 0, "Width must not be zero");
    DEV_CHECK_ERR(Attribs.Height > 0, "Height must not be zero");
    DEV_CHECK_ERR(Attribs.SrcComponentSize > 0, "Source component size must not be zero");
    DEV_CHECK_ERR(Attribs.pSrcPixels != nullptr, "Source pixels pointer must not be null");
    DEV_CHECK_ERR(Attribs.SrcStride != 0 || Attribs.Height == 1, "Source stride must not be null");
    DEV_CHECK_ERR(Attribs.SrcCompCount != 0, "Source component count must not be zero");
    DEV_CHECK_ERR(Attribs.pDstPixels != nullptr, "Destination pixels pointer must not be null");
    DEV_CHECK_ERR(Attribs.DstComponentSize > 0, "Destination component size must not be zero");
    DEV_CHECK_ERR(Attribs.DstStride != 0 || Attribs.Height == 1, "Destination stride must not be null");
    DEV_CHECK_ERR(Attribs.DstCompCount != 0, "Destination component count must not be zero");
    DEV_CHECK_ERR(Attribs.SrcStride >= Attribs.Width * Attribs.SrcComponentSize * Attribs.SrcCompCount || Attribs.Height == 1, "Source stride is too small");
    DEV_CHECK_ERR(Attribs.DstStride >= Attribs.Width * Attribs.DstComponentSize * Attribs.DstCompCount || Attribs.Height == 1, "Destination stride is too small");


    switch (Attribs.SrcComponentSize)
    {
#define CASE_SRC_COMPONENT_SIZE(SRC_TYPE)                                                               \
    case sizeof(SRC_TYPE):                                                                              \
        switch (Attribs.DstComponentSize)                                                               \
        {                                                                                               \
            case 1: CopyPixelsImpl<SRC_TYPE, Uint8>(Attribs); break;                                    \
            case 2: CopyPixelsImpl<SRC_TYPE, Uint16>(Attribs); break;                                   \
            case 4: CopyPixelsImpl<SRC_TYPE, Uint32>(Attribs); break;                                   \
            default: UNSUPPORTED("Unsupported destination component size: ", Attribs.DstComponentSize); \
        }                                                                                               \
        break

        CASE_SRC_COMPONENT_SIZE(Uint8);
        CASE_SRC_COMPONENT_SIZE(Uint16);
        CASE_SRC_COMPONENT_SIZE(Uint32);
#undef CASE_SRC_COMPONENT_SIZE

        default:
            UNSUPPORTED("Unsupported source component size: ", Attribs.SrcComponentSize);
    }
}

void ExpandPixels(const ExpandPixelsAttribs& Attribs)
{
    DEV_CHECK_ERR(Attribs.SrcWidth > 0, "Source width must not be zero");
    DEV_CHECK_ERR(Attribs.SrcHeight > 0, "Source height must not be zero");
    DEV_CHECK_ERR(Attribs.ComponentSize > 0, "Component size must not be zero");
    DEV_CHECK_ERR(Attribs.ComponentCount != 0, "Component count must not be zero");
    DEV_CHECK_ERR(Attribs.pSrcPixels != nullptr, "Source pixels pointer must not be null");
    DEV_CHECK_ERR(Attribs.SrcStride != 0 || Attribs.SrcHeight == 1, "Source stride must not be null");

    DEV_CHECK_ERR(Attribs.DstWidth > 0, "Destination width must not be zero");
    DEV_CHECK_ERR(Attribs.DstHeight > 0, "Destination height must not be zero");
    DEV_CHECK_ERR(Attribs.pDstPixels != nullptr, "Destination pixels pointer must not be null");
    DEV_CHECK_ERR(Attribs.DstStride != 0 || Attribs.DstHeight == 1, "Destination stride must not be null");
    DEV_CHECK_ERR(Attribs.SrcStride >= Attribs.SrcWidth * Attribs.ComponentSize * Attribs.ComponentCount || Attribs.SrcHeight == 1, "Source stride is too small");
    DEV_CHECK_ERR(Attribs.DstStride >= Attribs.DstWidth * Attribs.ComponentSize * Attribs.ComponentCount || Attribs.DstHeight == 1, "Destination stride is too small");

    const auto NumRowsToCopy = std::min(Attribs.SrcHeight, Attribs.DstHeight);
    const auto NumColsToCopy = std::min(Attribs.SrcWidth, Attribs.DstWidth);

    auto ExpandRow = [&Attribs, NumColsToCopy](size_t row, Uint8* pDstRow) {
        const auto* pSrcRow = reinterpret_cast<const Uint8*>(Attribs.pSrcPixels) + row * size_t{Attribs.SrcStride};
        memcpy(pDstRow, pSrcRow, size_t{NumColsToCopy} * size_t{Attribs.ComponentSize} * size_t{Attribs.ComponentCount});

        // Expand the row by repeating the last pixel
        const auto* pLastPixel = pSrcRow + size_t{NumColsToCopy - 1u} * size_t{Attribs.ComponentSize} * size_t{Attribs.ComponentCount};
        for (size_t col = NumColsToCopy; col < Attribs.DstWidth; ++col)
        {
            memcpy(pDstRow + col * Attribs.ComponentSize * Attribs.ComponentCount, pLastPixel, size_t{Attribs.ComponentSize} * size_t{Attribs.ComponentCount});
        }
    };

    for (size_t row = 0; row < NumRowsToCopy; ++row)
    {
        auto* pDstRow = reinterpret_cast<Uint8*>(Attribs.pDstPixels) + row * Attribs.DstStride;
        ExpandRow(row, pDstRow);
    }

    if (NumRowsToCopy < Attribs.DstHeight)
    {
        std::vector<Uint8> LastRow(size_t{Attribs.DstWidth} * size_t{Attribs.ComponentSize} * size_t{Attribs.ComponentCount});
        ExpandRow(NumRowsToCopy - 1, LastRow.data());
        for (size_t row = NumRowsToCopy - 1; row < Attribs.DstHeight; ++row)
        {
            auto* pDstRow = reinterpret_cast<Uint8*>(Attribs.pDstPixels) + row * Attribs.DstStride;
            memcpy(pDstRow, LastRow.data(), LastRow.size());
        }
    }
}

template <typename Type>
struct PremultiplyAlphaImplHelper;

template <>
struct PremultiplyAlphaImplHelper<Uint8>
{
    using IntermediateType = Uint32;
};

template <>
struct PremultiplyAlphaImplHelper<Int8>
{
    using IntermediateType = Int32;
};

template <>
struct PremultiplyAlphaImplHelper<Uint16>
{
    using IntermediateType = Uint32;
};

template <>
struct PremultiplyAlphaImplHelper<Int16>
{
    using IntermediateType = Int32;
};

template <>
struct PremultiplyAlphaImplHelper<Uint32>
{
    using IntermediateType = Uint64;
};

template <>
struct PremultiplyAlphaImplHelper<Int32>
{
    using IntermediateType = Int64;
};

template <typename Type, typename PremultiplyComponentType>
void PremultiplyComponents(const PremultiplyAlphaAttribs& Attribs, PremultiplyComponentType&& PremultiplyComponent)
{
    for (Uint32 row = 0; row < Attribs.Height; ++row)
    {
        auto* pRow = reinterpret_cast<Type*>(reinterpret_cast<Uint8*>(Attribs.pPixels) + row * Attribs.Stride);
        for (Uint32 col = 0; col < Attribs.Width; ++col)
        {
            auto* pPixel = pRow + col * Attribs.ComponentCount;
            auto  A      = pPixel[Attribs.ComponentCount - 1];
            for (Uint32 c = 0; c < Attribs.ComponentCount - 1; ++c)
                PremultiplyComponent(pPixel[c], A);
        }
    }
}

template <typename Type>
void PremultiplyAlphaImpl(const PremultiplyAlphaAttribs& Attribs)
{
    if (Attribs.IsSRGB)
    {
        PremultiplyComponents<Type>(
            Attribs,
            [](auto& C, auto A) {
                constexpr auto MaxValue = static_cast<float>(std::numeric_limits<Type>::max());

                float Linear = FastGammaToLinear(static_cast<float>(C) / MaxValue);
                Linear *= static_cast<float>(A) / MaxValue;
                float Gamma = FastLinearToGamma(Linear);

                C = static_cast<Type>(Gamma * MaxValue + 0.5f);
            });
    }
    else
    {
        PremultiplyComponents<Type>(
            Attribs,
            [](auto& C, auto A) {
                using IntermediateType = typename PremultiplyAlphaImplHelper<Type>::IntermediateType;

                constexpr auto MaxValue = static_cast<IntermediateType>(std::numeric_limits<Type>::max());

                C = static_cast<Type>((static_cast<IntermediateType>(C) * A + MaxValue / 2) / MaxValue);
            });
    }
}

template <>
void PremultiplyAlphaImpl<float>(const PremultiplyAlphaAttribs& Attribs)
{
    using Type = float;
    if (Attribs.IsSRGB)
    {
        PremultiplyComponents<Type>(
            Attribs,
            [](auto& C, auto A) {
                float Linear = FastGammaToLinear(C);
                Linear *= A;
                C = FastLinearToGamma(Linear);
            });
    }
    else
    {
        PremultiplyComponents<Type>(
            Attribs,
            [](auto& C, auto A) {
                C *= A;
            });
    }
}

void PremultiplyAlpha(const PremultiplyAlphaAttribs& Attribs)
{
    const auto ValueSize = GetValueSize(Attribs.ComponentType);

    DEV_CHECK_ERR(Attribs.Width > 0, "Eidth must not be zero");
    DEV_CHECK_ERR(Attribs.Height > 0, "Height must not be zero");
    DEV_CHECK_ERR(Attribs.ComponentCount >= 2, "The number of components must be at least two");
    DEV_CHECK_ERR(Attribs.pPixels != nullptr, "Pixels pointer must not be null");
    DEV_CHECK_ERR(Attribs.Stride != 0 || Attribs.Height == 1, "Source stride must not be null");
    DEV_CHECK_ERR(Attribs.Stride >= Attribs.Width * ValueSize * Attribs.ComponentCount || Attribs.Height == 1, "Source stride is too small");

    switch (Attribs.ComponentType)
    {
        case VT_UINT8: PremultiplyAlphaImpl<Uint8>(Attribs); break;
        case VT_UINT16: PremultiplyAlphaImpl<Uint16>(Attribs); break;
        case VT_UINT32: PremultiplyAlphaImpl<Uint32>(Attribs); break;

        case VT_INT8: PremultiplyAlphaImpl<Int8>(Attribs); break;
        case VT_INT16: PremultiplyAlphaImpl<Int16>(Attribs); break;
        case VT_INT32: PremultiplyAlphaImpl<Int32>(Attribs); break;

        case VT_FLOAT32: PremultiplyAlphaImpl<float>(Attribs); break;

        default:
            UNSUPPORTED("Unsupported component type ", GetValueTypeString(Attribs.ComponentType));
    }
}

void CreateTextureFromFile(const Char*            FilePath,
                           const TextureLoadInfo& TexLoadInfo,
                           IRenderDevice*         pDevice,
                           ITexture**             ppTexture)
{
    RefCntAutoPtr<ITextureLoader> pTexLoader;
    CreateTextureLoaderFromFile(FilePath, IMAGE_FILE_FORMAT_UNKNOWN, TexLoadInfo, &pTexLoader);
    if (!pTexLoader)
        return;

    pTexLoader->CreateTexture(pDevice, ppTexture);
}

} // namespace Diligent

extern "C"
{
    void Diligent_CreateTextureFromFile(const Diligent::Char*            FilePath,
                                        const Diligent::TextureLoadInfo& TexLoadInfo,
                                        Diligent::IRenderDevice*         pDevice,
                                        Diligent::ITexture**             ppTexture)
    {
        Diligent::CreateTextureFromFile(FilePath, TexLoadInfo, pDevice, ppTexture);
    }
}
