/*
 *  Copyright 2019-2024 Diligent Graphics LLC
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

#include <chrono>
#include "ImGuiImplDiligent.hpp"

struct EmscriptenMouseEvent;
struct EmscriptenWheelEvent;
struct EmscriptenKeyboardEvent;

namespace Diligent
{

class ImGuiImplEmscripten final : public ImGuiImplDiligent
{
public:
    static std::unique_ptr<ImGuiImplEmscripten> Create(const ImGuiDiligentCreateInfo& CI);

    ImGuiImplEmscripten(const ImGuiDiligentCreateInfo& CI);
    ~ImGuiImplEmscripten();

    // clang-format off
    ImGuiImplEmscripten             (const ImGuiImplEmscripten&)  = delete;
    ImGuiImplEmscripten             (      ImGuiImplEmscripten&&) = delete;
    ImGuiImplEmscripten& operator = (const ImGuiImplEmscripten&)  = delete;
    ImGuiImplEmscripten& operator = (      ImGuiImplEmscripten&&) = delete;
    // clang-format on

    virtual void NewFrame(Uint32            RenderSurfaceWidth,
                          Uint32            RenderSurfaceHeight,
                          SURFACE_TRANSFORM SurfacePreTransform) override final;
    virtual void Render(IDeviceContext* pCtx) override final;

    bool OnMouseEvent(int32_t EventType, const EmscriptenMouseEvent* Event);

    bool OnWheelEvent(int32_t EventType, const EmscriptenWheelEvent* Event);

    bool OnKeyEvent(int32_t EventType, const EmscriptenKeyboardEvent* Event);


private:
    std::chrono::high_resolution_clock::time_point m_LastTimestamp = {};
};

} // namespace Diligent
