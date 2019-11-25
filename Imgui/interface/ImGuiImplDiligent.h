/*     Copyright 2019 Diligent Graphics LLC
 *  
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 * 
 *     http://www.apache.org/licenses/LICENSE-2.0
 * 
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT OF ANY PROPRIETARY RIGHTS.
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

#include <memory>
#include "../../../DiligentCore/Primitives/interface/BasicTypes.h"

namespace Diligent
{

class IRenderDevice;
class IDeviceContext;
enum TEXTURE_FORMAT : Uint16;

class ImGuiImplDiligent_Internal;

class ImGuiImplDiligent
{
public:
    static constexpr Uint32 DefaultInitialVBSize = 1024;
    static constexpr Uint32 DefaultInitialIBSize = 2048;

    ImGuiImplDiligent(IRenderDevice* pDevice,
                      TEXTURE_FORMAT BackBufferFmt,
                      TEXTURE_FORMAT DepthBufferFmt,
                      Uint32         InitialVertexBufferSize = DefaultInitialVBSize,
                      Uint32         InitialIndexBufferSize  = DefaultInitialIBSize);
    virtual ~ImGuiImplDiligent();

    // clang-format off
    ImGuiImplDiligent             (const ImGuiImplDiligent&)  = delete;
    ImGuiImplDiligent             (      ImGuiImplDiligent&&) = delete;
    ImGuiImplDiligent& operator = (const ImGuiImplDiligent&)  = delete;
    ImGuiImplDiligent& operator = (      ImGuiImplDiligent&&) = delete;
    // clang-format on

    virtual void NewFrame();
    virtual void Render(IDeviceContext* pCtx);

    // Use if you want to reset your rendering device without losing ImGui state.
    void InvalidateDeviceObjects();
    void CreateDeviceObjects();

private:
    std::unique_ptr<ImGuiImplDiligent_Internal> m_pImpl;
};

} // namespace Diligent
