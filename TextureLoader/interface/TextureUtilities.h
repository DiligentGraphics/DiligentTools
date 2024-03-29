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

#pragma once

/// \file
/// Defines texture utilities

#include "../../../DiligentCore/Graphics/GraphicsEngine/interface/GraphicsTypes.h"
#include "../../../DiligentCore/Graphics/GraphicsEngine/interface/Texture.h"
#include "../../../DiligentCore/Graphics/GraphicsEngine/interface/RenderDevice.h"
#include "TextureLoader.h"

DILIGENT_BEGIN_NAMESPACE(Diligent)


#include "../../../DiligentCore/Primitives/interface/DefineGlobalFuncHelperMacros.h"

/// Parameters of the CopyPixels function.
struct CopyPixelsAttribs
{
    /// Texture width.
    Uint32 Width DEFAULT_INITIALIZER(0);

    /// Texture height.
    Uint32 Height DEFAULT_INITIALIZER(0);

    /// Source component size in bytes.
    Uint32 SrcComponentSize DEFAULT_INITIALIZER(0);

    /// A pointer to source pixels.
    const void* pSrcPixels DEFAULT_INITIALIZER(nullptr);

    /// Source stride in bytes.
    Uint32 SrcStride DEFAULT_INITIALIZER(0);

    /// Source component count.
    Uint32 SrcCompCount DEFAULT_INITIALIZER(0);

    /// A pointer to destination pixels.
    void* pDstPixels DEFAULT_INITIALIZER(nullptr);

    /// Destination component size in bytes.
    Uint32 DstComponentSize DEFAULT_INITIALIZER(0);

    /// Destination stride in bytes.
    Uint32 DstStride DEFAULT_INITIALIZER(0);

    /// Destination component count.
    Uint32 DstCompCount DEFAULT_INITIALIZER(0);

    /// If true, flip the image vertically.
    bool FlipVertically DEFAULT_INITIALIZER(false);

    /// Texture component swizzle.
    TextureComponentMapping Swizzle DEFAULT_INITIALIZER(TextureComponentMapping::Identity());
};
typedef struct CopyPixelsAttribs CopyPixelsAttribs;

/// Copies texture pixels allowing changing the number of components.
void DILIGENT_GLOBAL_FUNCTION(CopyPixels)(const CopyPixelsAttribs REF Attribs);


/// Parameters of the ExpandPixels function.
struct ExpandPixelsAttribs
{
    /// Source texture width.
    Uint32 SrcWidth DEFAULT_INITIALIZER(0);

    /// Source texture height.
    Uint32 SrcHeight DEFAULT_INITIALIZER(0);

    /// Texture component size in bytes.
    Uint32 ComponentSize DEFAULT_INITIALIZER(0);

    /// Component count.
    Uint32 ComponentCount DEFAULT_INITIALIZER(0);

    /// A pointer to source pixels.
    const void* pSrcPixels DEFAULT_INITIALIZER(nullptr);

    /// Source stride in bytes.
    Uint32 SrcStride DEFAULT_INITIALIZER(0);

    /// Destination texture width.
    Uint32 DstWidth DEFAULT_INITIALIZER(0);

    /// Destination texture height.
    Uint32 DstHeight DEFAULT_INITIALIZER(0);

    /// A pointer to destination pixels.
    void* pDstPixels DEFAULT_INITIALIZER(nullptr);

    /// Destination stride in bytes.
    Uint32 DstStride DEFAULT_INITIALIZER(0);
};
typedef struct ExpandPixelsAttribs ExpandPixelsAttribs;

/// Expands the texture pixels by repeating the last row and column.
void DILIGENT_GLOBAL_FUNCTION(ExpandPixels)(const ExpandPixelsAttribs REF Attribs);


/// Parameters of the PremultiplyAlpha function.
struct PremultiplyAlphaAttribs
{
    /// Texture width.
    Uint32 Width DEFAULT_INITIALIZER(0);

    /// Texture height.
    Uint32 Height DEFAULT_INITIALIZER(0);

    /// A pointer to pixels.
    void* pPixels DEFAULT_INITIALIZER(nullptr);

    /// Stride in bytes.
    Uint32 Stride DEFAULT_INITIALIZER(0);

    /// Component count.
    Uint32 ComponentCount DEFAULT_INITIALIZER(0);

    /// Component type.
    VALUE_TYPE ComponentType DEFAULT_INITIALIZER(VT_UINT8);

    /// If true, the texture is in sRGB format.
    bool IsSRGB DEFAULT_INITIALIZER(false);
};
typedef struct PremultiplyAlphaAttribs PremultiplyAlphaAttribs;

/// Premultiplies image components with alpha in place.
/// \note Alpha is assumed to be the last component.
void DILIGENT_GLOBAL_FUNCTION(PremultiplyAlpha)(const PremultiplyAlphaAttribs REF Attribs);


/// Creates a texture from file.

/// \param [in] FilePath    - Source file path.
/// \param [in] TexLoadInfo - Texture loading information.
/// \param [in] pDevice     - Render device that will be used to create the texture.
/// \param [out] ppTexture  - Memory location where pointer to the created texture will be written.
void DILIGENT_GLOBAL_FUNCTION(CreateTextureFromFile)(const Char*               FilePath,
                                                     const TextureLoadInfo REF TexLoadInfo,
                                                     IRenderDevice*            pDevice,
                                                     ITexture**                ppTexture);

#include "../../../DiligentCore/Primitives/interface/UndefGlobalFuncHelperMacros.h"

DILIGENT_END_NAMESPACE // namespace Diligent
