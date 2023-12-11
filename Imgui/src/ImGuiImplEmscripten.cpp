/*
 *  Copyright 2019-2023 Diligent Graphics LLC
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

#include "imgui.h"
#include "ImGuiImplEmscripten.hpp"

#include <emscripten.h>
#include <emscripten/html5.h>
#include <emscripten/key_codes.h>

namespace Diligent
{

std::unique_ptr<ImGuiImplEmscripten> ImGuiImplEmscripten::Create(const ImGuiDiligentCreateInfo& CI)
{
    return std::make_unique<ImGuiImplEmscripten>(CI);
}

ImGuiImplEmscripten::ImGuiImplEmscripten(const ImGuiDiligentCreateInfo& CI) :
    ImGuiImplDiligent{CI}
{
    ImGuiIO& io            = ImGui::GetIO();
    io.BackendPlatformName = "Diligent-ImGuiImplEmscripten";

    io.KeyMap[ImGuiKey_Tab]        = DOM_VK_TAB;
    io.KeyMap[ImGuiKey_LeftArrow]  = DOM_VK_LEFT;
    io.KeyMap[ImGuiKey_RightArrow] = DOM_VK_RIGHT;
    io.KeyMap[ImGuiKey_UpArrow]    = DOM_VK_UP;
    io.KeyMap[ImGuiKey_DownArrow]  = DOM_VK_DOWN;
    io.KeyMap[ImGuiKey_PageUp]     = DOM_VK_PAGE_UP;
    io.KeyMap[ImGuiKey_PageDown]   = DOM_VK_PAGE_DOWN;
    io.KeyMap[ImGuiKey_Home]       = DOM_VK_HOME;
    io.KeyMap[ImGuiKey_End]        = DOM_VK_END;
    io.KeyMap[ImGuiKey_Delete]     = DOM_VK_DELETE;
    io.KeyMap[ImGuiKey_Backspace]  = DOM_VK_BACK_SPACE;
    io.KeyMap[ImGuiKey_Enter]      = DOM_VK_ENTER;
    io.KeyMap[ImGuiKey_Escape]     = DOM_VK_ESCAPE;
    io.KeyMap[ImGuiKey_A]          = DOM_VK_A;
    io.KeyMap[ImGuiKey_C]          = DOM_VK_C;
    io.KeyMap[ImGuiKey_V]          = DOM_VK_V;
    io.KeyMap[ImGuiKey_X]          = DOM_VK_X;
    io.KeyMap[ImGuiKey_Y]          = DOM_VK_Y;
    io.KeyMap[ImGuiKey_Z]          = DOM_VK_Z;
}

ImGuiImplEmscripten::~ImGuiImplEmscripten()
{
}

void ImGuiImplEmscripten::NewFrame(Uint32            RenderSurfaceWidth,
                                   Uint32            RenderSurfaceHeight,
                                   SURFACE_TRANSFORM SurfacePreTransform)
{
    auto now        = std::chrono::high_resolution_clock::now();
    auto elapsed_ns = now - m_LastTimestamp;
    m_LastTimestamp = now;
    auto& io        = ImGui::GetIO();
    io.DeltaTime    = static_cast<float>(elapsed_ns.count() / 1e+9);
    io.DisplaySize  = ImVec2(RenderSurfaceWidth / io.DisplayFramebufferScale.x, RenderSurfaceHeight / io.DisplayFramebufferScale.y);
    ImGuiImplDiligent::NewFrame(RenderSurfaceWidth, RenderSurfaceHeight, SurfacePreTransform);
}

void ImGuiImplEmscripten::Render(IDeviceContext* pCtx)
{
    ImGuiImplDiligent::Render(pCtx);
}

bool ImGuiImplEmscripten::OnMouseEvent(int32_t EventType, const EmscriptenMouseEvent* Event)
{
    auto& io        = ImGui::GetIO();
    io.MousePos     = ImVec2(Event->targetX, Event->targetY);
    io.MouseDown[0] = Event->buttons & 1;
    io.MouseDown[1] = Event->buttons & 2;
    io.MouseDown[2] = Event->buttons & 4;
    return io.WantCaptureMouse;
}

bool ImGuiImplEmscripten::OnWheelEvent(int32_t EventType, const EmscriptenWheelEvent* Event)
{
    auto& io = ImGui::GetIO();
    io.MouseWheelH += -0.1f * Event->deltaX;
    io.MouseWheel += -0.1f * Event->deltaY;
    return io.WantCaptureMouse;
}

bool ImGuiImplEmscripten::OnKeyEvent(int32_t EventType, const EmscriptenKeyboardEvent* Event)
{
    auto& io    = ImGui::GetIO();
    io.KeyCtrl  = Event->ctrlKey;
    io.KeyShift = Event->shiftKey;
    io.KeyAlt   = Event->altKey;
    io.KeySuper = Event->metaKey;

    switch (EventType)
    {
        case EMSCRIPTEN_EVENT_KEYDOWN:
        case EMSCRIPTEN_EVENT_KEYUP:
        {
            bool IsPressed            = EventType == EMSCRIPTEN_EVENT_KEYDOWN;
            io.KeysDown[Event->which] = IsPressed;
            break;
        }
        case EMSCRIPTEN_EVENT_KEYPRESS:
        {
            io.AddInputCharactersUTF8(Event->key);
            break;
        }
        default:
            break;
    }
    return io.WantCaptureKeyboard;
}
} // namespace Diligent
