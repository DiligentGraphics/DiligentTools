/*
 *  Copyright 2019-2025 Diligent Graphics LLC
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
/// Defines Diligent::ImGuiImplDiligent class implementing ImGui renderer for Diligent Engine.

#include <memory>
#include "../../../DiligentCore/Primitives/interface/BasicTypes.h"

namespace Diligent
{

struct IRenderDevice;
struct IDeviceContext;
struct SwapChainDesc;
enum TEXTURE_FORMAT : Uint16;
enum SURFACE_TRANSFORM : Uint32;

class ImGuiDiligentRenderer;

/// Conversion mode to apply to imgui colors.
///
/// Imgui colors are defined in sRGB space.
/// Depending on the use case, they may need
/// to be converted to linear space.
enum IMGUI_COLOR_CONVERSION_MODE : Uint8
{
    /// Select the color conversion mode automatically:
    /// * Use SRGB_TO_LINEAR mode for sRGB framebuffers
    /// * Use NONE mode for non-sRGB framebuffers
    IMGUI_COLOR_CONVERSION_MODE_AUTO = 0,

    /// Always perform srgb-to-linear conversion.
    IMGUI_COLOR_CONVERSION_MODE_SRGB_TO_LINEAR,

    /// Do not perform any color conversion.
    IMGUI_COLOR_CONVERSION_MODE_NONE
};

struct ImGuiDiligentCreateInfo
{
    static constexpr Uint32 DefaultInitialVBSize = 1024;
    static constexpr Uint32 DefaultInitialIBSize = 2048;

    IRenderDevice* pDevice = nullptr;

    TEXTURE_FORMAT BackBufferFmt  = {};
    TEXTURE_FORMAT DepthBufferFmt = {};

    IMGUI_COLOR_CONVERSION_MODE ColorConversion = IMGUI_COLOR_CONVERSION_MODE_AUTO;

    Uint32 InitialVertexBufferSize = DefaultInitialVBSize;
    Uint32 InitialIndexBufferSize  = DefaultInitialIBSize;

    ImGuiDiligentCreateInfo() noexcept {}
    ImGuiDiligentCreateInfo(IRenderDevice* _pDevice,
                            TEXTURE_FORMAT _BackBufferFmt,
                            TEXTURE_FORMAT _DepthBufferFmt) noexcept;
    ImGuiDiligentCreateInfo(IRenderDevice*       _pDevice,
                            const SwapChainDesc& _SCDesc) noexcept;
};

/// ImGui renderer for Diligent Engine.
class ImGuiImplDiligent
{
public:
    ImGuiImplDiligent(const ImGuiDiligentCreateInfo& CI);
    virtual ~ImGuiImplDiligent();

    // clang-format off
    ImGuiImplDiligent             (const ImGuiImplDiligent&)  = delete;
    ImGuiImplDiligent             (      ImGuiImplDiligent&&) = delete;
    ImGuiImplDiligent& operator = (const ImGuiImplDiligent&)  = delete;
    ImGuiImplDiligent& operator = (      ImGuiImplDiligent&&) = delete;
    // clang-format on


    /// Begins new frame

    /// \param [in] RenderSurfaceWidth  - Render surface width not accounting for pre-transform.
    ///                                   Most of the time this is the actual swap chain width.
    /// \param [in] RenderSurfaceHeight - Render surface height not accounting for pre-transform.
    ///                                   Most of the time this is the actual swap chain height.
    /// \param [in] SurfacePreTransform - Render surface pre-transform.
    ///                                   Most of the time this is the swap chain pre-transform.
    virtual void NewFrame(Uint32            RenderSurfaceWidth,
                          Uint32            RenderSurfaceHeight,
                          SURFACE_TRANSFORM SurfacePreTransform);

    virtual void EndFrame();
    virtual void Render(IDeviceContext* pCtx);

    // Use if you want to reset your rendering device without losing ImGui state.
    void InvalidateDeviceObjects();
    void CreateDeviceObjects();

    void UpdateFontsTexture();

protected:
    std::unique_ptr<ImGuiDiligentRenderer> m_pRenderer;
};

} // namespace Diligent
