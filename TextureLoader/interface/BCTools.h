/*
 *  Copyright 2019-2021 Diligent Graphics LLC
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

#pragma once

#include "../../../DiligentCore/Primitives/interface/BasicTypes.h"

DILIGENT_BEGIN_NAMESPACE(Diligent)

// clang-format off

/// Decompresses BC1 block (4x4 RGB).

/// \param[in]  Bits        - Compressed block bits.
/// \param[out] DstBuffer   - Pointer to the output 4x4 RGB or RGBA buffer.
/// \param[in]  DstChannels - The number of components in the output buffer.
///                           Must be 3 (RGB) or 4 (RGBA).
void DecompressBC1Block(const Uint8* Bits,
                        Uint8*       DstBuffer,
                        Uint32       DstChannels DEFAULT_VALUE(4));


/// Decompresses BC3 block (4x4 RGB+A).

/// \param[in]  Bits      - Compressed block bits.
/// \param[out] DstBuffer - Pointer to the output 4x4 RGBA buffer.
void DecompressBC3Block(const Uint8* Bits,
                        Uint8*       DstBuffer);


/// Decompresses BC4 block (4x4 R).

/// \param[in]  Bits        - Compressed block bits.
/// \param[out] DstBuffer   - Pointer to the output 4x4 pixel buffer.
/// \param[in]  DstChannels - The number of components in the output buffer.
void DecompressBC4Block(const Uint8* Bits,
                        Uint8*       DstBuffer,
                        Uint32       DstChannels DEFAULT_VALUE(1));


/// Decompresses BC5 block (4x4 R+G).

/// \param[in]  Bits        - Compressed block bits.
/// \param[out] DstBuffer   - Pointer to the output 4x4 pixel buffer.
/// \param[in]  DstChannels - The number of components in the output buffer.
///                           Must be greater than 2.
void DecompressBC5Block(const Uint8* Bits,
                        Uint8*       DstBuffer,
                        Uint32       DstChannels DEFAULT_VALUE(2));

// clang-format on

DILIGENT_END_NAMESPACE // namespace Diligent
