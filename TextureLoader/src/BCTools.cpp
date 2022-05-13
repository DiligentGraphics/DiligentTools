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

#include "BCTools.h"
#include "DebugUtilities.hpp"

namespace Diligent
{

inline void DecompressColorBlock(const Uint8* Bits,
                                 Uint8*       DstBuffer,
                                 Uint32       DstChannels)
{
    VERIFY_EXPR(DstChannels >= 3);
    const Uint32 RGB[2] =
        {
            Uint32{Bits[0]} | ((Uint32{Bits[1]}) << 8),
            Uint32{Bits[2]} | ((Uint32{Bits[3]}) << 8) //
        };

    static constexpr Uint32 ROffset = 11;
    static constexpr Uint32 GOffset = 5;
    static constexpr Uint32 BOffset = 0;

    static constexpr Uint32 RMask = (1 << 5) - 1;
    static constexpr Uint32 GMask = (1 << 6) - 1;
    static constexpr Uint32 BMask = (1 << 5) - 1;

    Uint32 R[4] = {(RGB[0] >> ROffset) & RMask, (RGB[1] >> ROffset) & RMask, 0, 0};
    Uint32 G[4] = {(RGB[0] >> GOffset) & GMask, (RGB[1] >> GOffset) & GMask, 0, 0};
    Uint32 B[4] = {(RGB[0] >> BOffset) & BMask, (RGB[1] >> BOffset) & BMask, 0, 0};

    if (RGB[0] > RGB[1])
    {
        R[2] = (2 * R[0] + 1 * R[1]) / 3;
        G[2] = (2 * G[0] + 1 * G[1]) / 3;
        B[2] = (2 * B[0] + 1 * B[1]) / 3;

        R[3] = (1 * R[0] + 2 * R[1]) / 3;
        G[3] = (1 * G[0] + 2 * G[1]) / 3;
        B[3] = (1 * B[0] + 2 * B[1]) / 3;
    }
    else
    {
        R[2] = (2 * R[0] + 1 * R[1]) / 2;
        G[2] = (2 * G[0] + 1 * G[1]) / 2;
        B[2] = (2 * B[0] + 1 * B[1]) / 2;
    }

    const Uint8* Palette = Bits + 4;
    for (Uint32 i = 0; i < 16; ++i)
    {
        const auto Idx = (Palette[i / 4] >> ((i % 4) * 2)) & 0x03;

        DstBuffer[i * DstChannels + 0] = (R[Idx] << 3u) & 0xFFu;
        DstBuffer[i * DstChannels + 1] = (G[Idx] << 2u) & 0xFFu;
        DstBuffer[i * DstChannels + 2] = (B[Idx] << 3u) & 0xFFu;
    }
}

inline void DecompressAlphaBlock(const Uint8* Bits,
                                 Uint8*       DstBuffer,
                                 Uint32       DstChannels)
{
    Uint32 Alpha[8] = {Bits[0], Bits[1]};
    if (Alpha[0] > Alpha[1])
    {
        for (Uint32 i = 2; i < 8; ++i)
        {
            Alpha[i] = ((8 - i) * Alpha[0] + (i - 1) * Alpha[1]) / 7;
        }
    }
    else
    {
        for (Uint32 i = 2; i < 6; ++i)
        {
            Alpha[i] = ((6 - i) * Alpha[0] + (i - 1) * Alpha[1]) / 5;
        }
        Alpha[6] = 0;
        Alpha[7] = 255;
    }

    for (size_t p = 0; p < 2; ++p)
    {
        const Uint8* PaletteBits = Bits + 2 + p * 3;
        const Uint32 Palette0    = Uint32{PaletteBits[0]} | (Uint32{PaletteBits[1]} << 8) | (Uint32{PaletteBits[2]} << 16);
        for (Uint32 i = 0; i < 8; ++i)
        {
            Uint32 Idx = (Palette0 >> (i * 3)) & 0x07;

            DstBuffer[(p * 8 + i) * DstChannels] = Alpha[Idx] & 0xFFu;
        }
    }
}

void DecompressBC1Block(const Uint8* Bits,
                        Uint8*       DstBuffer,
                        Uint32       DstChannels)
{
    VERIFY_EXPR(DstChannels >= 3);
    DecompressColorBlock(Bits, DstBuffer, DstChannels);
}

void DecompressBC3Block(const Uint8* Bits,
                        Uint8*       DstBuffer)
{
    DecompressColorBlock(Bits + 8, DstBuffer, 4);
    DecompressAlphaBlock(Bits, DstBuffer + 3, 4);
}

void DecompressBC4Block(const Uint8* Bits,
                        Uint8*       DstBuffer,
                        Uint32       DstChannels)
{
    VERIFY_EXPR(DstChannels >= 1);
    DecompressAlphaBlock(Bits, DstBuffer, DstChannels);
}

void DecompressBC5lock(const Uint8* Bits,
                       Uint8*       DstBuffer,
                       Uint32       DstChannels)
{
    VERIFY_EXPR(DstChannels >= 2);
    DecompressAlphaBlock(Bits, DstBuffer, DstChannels);
    DecompressAlphaBlock(Bits + 8, DstBuffer + 1, DstChannels);
}

} // namespace Diligent
