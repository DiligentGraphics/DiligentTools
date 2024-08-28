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

#include "imgui.h"
#include "ImGuiImplEmscripten.hpp"

#include <emscripten.h>
#include <emscripten/html5.h>
#include <emscripten/key_codes.h>

namespace Diligent
{

struct KeyMapping
{
    ImGuiKey KeyCode;
    bool     IsConsume;
};

static KeyMapping ImGuiKeyMap[256] = {
    {ImGuiKey_None, false}};

static void InitializeImGuiKeyMap()
{
    ImGuiKeyMap[DOM_VK_BACK_SPACE]    = {ImGuiKey_Backspace, true};
    ImGuiKeyMap[DOM_VK_TAB]           = {ImGuiKey_Tab, true};
    ImGuiKeyMap[DOM_VK_RETURN]        = {ImGuiKey_Enter, true};
    ImGuiKeyMap[DOM_VK_SHIFT]         = {ImGuiKey_LeftShift, true};
    ImGuiKeyMap[DOM_VK_CONTROL]       = {ImGuiKey_LeftCtrl, true};
    ImGuiKeyMap[DOM_VK_ALT]           = {ImGuiKey_LeftAlt, true};
    ImGuiKeyMap[DOM_VK_CAPS_LOCK]     = {ImGuiKey_CapsLock, true};
    ImGuiKeyMap[DOM_VK_ESCAPE]        = {ImGuiKey_Escape, true};
    ImGuiKeyMap[DOM_VK_SPACE]         = {ImGuiKey_Space, false};
    ImGuiKeyMap[DOM_VK_PAGE_UP]       = {ImGuiKey_PageUp, true};
    ImGuiKeyMap[DOM_VK_PAGE_DOWN]     = {ImGuiKey_PageDown, true};
    ImGuiKeyMap[DOM_VK_END]           = {ImGuiKey_End, true};
    ImGuiKeyMap[DOM_VK_HOME]          = {ImGuiKey_Home, true};
    ImGuiKeyMap[DOM_VK_LEFT]          = {ImGuiKey_LeftArrow, true};
    ImGuiKeyMap[DOM_VK_UP]            = {ImGuiKey_UpArrow, true};
    ImGuiKeyMap[DOM_VK_RIGHT]         = {ImGuiKey_RightArrow, true};
    ImGuiKeyMap[DOM_VK_DOWN]          = {ImGuiKey_DownArrow, true};
    ImGuiKeyMap[DOM_VK_INSERT]        = {ImGuiKey_Insert, true};
    ImGuiKeyMap[DOM_VK_DELETE]        = {ImGuiKey_Delete, true};
    ImGuiKeyMap[DOM_VK_0]             = {ImGuiKey_0, false};
    ImGuiKeyMap[DOM_VK_1]             = {ImGuiKey_1, false};
    ImGuiKeyMap[DOM_VK_2]             = {ImGuiKey_2, false};
    ImGuiKeyMap[DOM_VK_3]             = {ImGuiKey_3, false};
    ImGuiKeyMap[DOM_VK_4]             = {ImGuiKey_4, false};
    ImGuiKeyMap[DOM_VK_5]             = {ImGuiKey_5, false};
    ImGuiKeyMap[DOM_VK_6]             = {ImGuiKey_6, false};
    ImGuiKeyMap[DOM_VK_7]             = {ImGuiKey_7, false};
    ImGuiKeyMap[DOM_VK_8]             = {ImGuiKey_8, false};
    ImGuiKeyMap[DOM_VK_9]             = {ImGuiKey_9, false};
    ImGuiKeyMap[DOM_VK_SEMICOLON]     = {ImGuiKey_Semicolon, false};
    ImGuiKeyMap[DOM_VK_EQUALS]        = {ImGuiKey_Equal, false};
    ImGuiKeyMap[DOM_VK_A]             = {ImGuiKey_A, false};
    ImGuiKeyMap[DOM_VK_B]             = {ImGuiKey_B, false};
    ImGuiKeyMap[DOM_VK_C]             = {ImGuiKey_C, false};
    ImGuiKeyMap[DOM_VK_D]             = {ImGuiKey_D, false};
    ImGuiKeyMap[DOM_VK_E]             = {ImGuiKey_E, false};
    ImGuiKeyMap[DOM_VK_F]             = {ImGuiKey_F, false};
    ImGuiKeyMap[DOM_VK_G]             = {ImGuiKey_G, false};
    ImGuiKeyMap[DOM_VK_H]             = {ImGuiKey_H, false};
    ImGuiKeyMap[DOM_VK_I]             = {ImGuiKey_I, false};
    ImGuiKeyMap[DOM_VK_J]             = {ImGuiKey_J, false};
    ImGuiKeyMap[DOM_VK_K]             = {ImGuiKey_K, false};
    ImGuiKeyMap[DOM_VK_L]             = {ImGuiKey_L, false};
    ImGuiKeyMap[DOM_VK_M]             = {ImGuiKey_M, false};
    ImGuiKeyMap[DOM_VK_N]             = {ImGuiKey_N, false};
    ImGuiKeyMap[DOM_VK_O]             = {ImGuiKey_O, false};
    ImGuiKeyMap[DOM_VK_P]             = {ImGuiKey_P, false};
    ImGuiKeyMap[DOM_VK_Q]             = {ImGuiKey_Q, false};
    ImGuiKeyMap[DOM_VK_R]             = {ImGuiKey_R, false};
    ImGuiKeyMap[DOM_VK_S]             = {ImGuiKey_S, false};
    ImGuiKeyMap[DOM_VK_T]             = {ImGuiKey_T, false};
    ImGuiKeyMap[DOM_VK_U]             = {ImGuiKey_U, false};
    ImGuiKeyMap[DOM_VK_V]             = {ImGuiKey_V, false};
    ImGuiKeyMap[DOM_VK_W]             = {ImGuiKey_W, false};
    ImGuiKeyMap[DOM_VK_X]             = {ImGuiKey_X, false};
    ImGuiKeyMap[DOM_VK_Y]             = {ImGuiKey_Y, false};
    ImGuiKeyMap[DOM_VK_Z]             = {ImGuiKey_Z, false};
    ImGuiKeyMap[DOM_VK_WIN]           = {ImGuiKey_LeftSuper, true};
    ImGuiKeyMap[DOM_VK_CONTEXT_MENU]  = {ImGuiKey_Menu, true};
    ImGuiKeyMap[DOM_VK_NUMPAD0]       = {ImGuiKey_Keypad0, false};
    ImGuiKeyMap[DOM_VK_NUMPAD1]       = {ImGuiKey_Keypad1, false};
    ImGuiKeyMap[DOM_VK_NUMPAD2]       = {ImGuiKey_Keypad2, false};
    ImGuiKeyMap[DOM_VK_NUMPAD3]       = {ImGuiKey_Keypad3, false};
    ImGuiKeyMap[DOM_VK_NUMPAD4]       = {ImGuiKey_Keypad4, false};
    ImGuiKeyMap[DOM_VK_NUMPAD5]       = {ImGuiKey_Keypad5, false};
    ImGuiKeyMap[DOM_VK_NUMPAD6]       = {ImGuiKey_Keypad6, false};
    ImGuiKeyMap[DOM_VK_NUMPAD7]       = {ImGuiKey_Keypad7, false};
    ImGuiKeyMap[DOM_VK_NUMPAD8]       = {ImGuiKey_Keypad8, false};
    ImGuiKeyMap[DOM_VK_NUMPAD9]       = {ImGuiKey_Keypad9, false};
    ImGuiKeyMap[DOM_VK_MULTIPLY]      = {ImGuiKey_KeypadMultiply, false};
    ImGuiKeyMap[DOM_VK_ADD]           = {ImGuiKey_KeypadAdd, false};
    ImGuiKeyMap[DOM_VK_SUBTRACT]      = {ImGuiKey_KeypadSubtract, false};
    ImGuiKeyMap[DOM_VK_DECIMAL]       = {ImGuiKey_KeypadDecimal, false};
    ImGuiKeyMap[DOM_VK_DIVIDE]        = {ImGuiKey_KeypadDivide, false};
    ImGuiKeyMap[DOM_VK_F1]            = {ImGuiKey_F1, true};
    ImGuiKeyMap[DOM_VK_F2]            = {ImGuiKey_F2, true};
    ImGuiKeyMap[DOM_VK_F3]            = {ImGuiKey_F3, true};
    ImGuiKeyMap[DOM_VK_F4]            = {ImGuiKey_F4, true};
    ImGuiKeyMap[DOM_VK_F5]            = {ImGuiKey_F5, true};
    ImGuiKeyMap[DOM_VK_F6]            = {ImGuiKey_F6, true};
    ImGuiKeyMap[DOM_VK_F7]            = {ImGuiKey_F7, true};
    ImGuiKeyMap[DOM_VK_F8]            = {ImGuiKey_F8, true};
    ImGuiKeyMap[DOM_VK_F9]            = {ImGuiKey_F9, true};
    ImGuiKeyMap[DOM_VK_F10]           = {ImGuiKey_F10, true};
    ImGuiKeyMap[DOM_VK_F11]           = {ImGuiKey_F11, false};
    ImGuiKeyMap[DOM_VK_F12]           = {ImGuiKey_F12, false};
    ImGuiKeyMap[DOM_VK_NUM_LOCK]      = {ImGuiKey_NumLock, true};
    ImGuiKeyMap[DOM_VK_SCROLL_LOCK]   = {ImGuiKey_ScrollLock, true};
    ImGuiKeyMap[DOM_VK_HYPHEN_MINUS]  = {ImGuiKey_Minus, false};
    ImGuiKeyMap[DOM_VK_SEMICOLON]     = {ImGuiKey_Semicolon, false};
    ImGuiKeyMap[DOM_VK_EQUALS]        = {ImGuiKey_Equal, false};
    ImGuiKeyMap[DOM_VK_COMMA]         = {ImGuiKey_Comma, false};
    ImGuiKeyMap[DOM_VK_HYPHEN_MINUS]  = {ImGuiKey_Minus, false};
    ImGuiKeyMap[DOM_VK_PERIOD]        = {ImGuiKey_Period, false};
    ImGuiKeyMap[DOM_VK_SLASH]         = {ImGuiKey_Slash, false};
    ImGuiKeyMap[DOM_VK_BACK_QUOTE]    = {ImGuiKey_GraveAccent, false};
    ImGuiKeyMap[DOM_VK_OPEN_BRACKET]  = {ImGuiKey_LeftBracket, false};
    ImGuiKeyMap[DOM_VK_BACK_SLASH]    = {ImGuiKey_Backslash, false};
    ImGuiKeyMap[DOM_VK_CLOSE_BRACKET] = {ImGuiKey_RightBracket, false};
    ImGuiKeyMap[DOM_VK_QUOTE]         = {ImGuiKey_Apostrophe, false};
    ImGuiKeyMap[DOM_VK_META]          = {ImGuiKey_LeftSuper, true};
}

static ImGuiKey RemapKeyCodeToImGuiKey(Int32 KeyCode, bool* IsConsume)
{
    if (KeyCode < 0 || KeyCode >= 256)
    {
        *IsConsume = false;
        return ImGuiKey_None;
    }
    *IsConsume = ImGuiKeyMap[KeyCode].IsConsume;
    return ImGuiKeyMap[KeyCode].KeyCode;
}

std::unique_ptr<ImGuiImplEmscripten> ImGuiImplEmscripten::Create(const ImGuiDiligentCreateInfo& CI)
{
    return std::make_unique<ImGuiImplEmscripten>(CI);
}

ImGuiImplEmscripten::ImGuiImplEmscripten(const ImGuiDiligentCreateInfo& CI) :
    ImGuiImplDiligent{CI}
{
    ImGuiIO& io            = ImGui::GetIO();
    io.BackendPlatformName = "Diligent-ImGuiImplEmscripten";
    m_LastTimestamp        = std::chrono::high_resolution_clock::now();
    InitializeImGuiKeyMap();
}

ImGuiImplEmscripten::~ImGuiImplEmscripten()
{
}

void ImGuiImplEmscripten::NewFrame(Uint32            RenderSurfaceWidth,
                                   Uint32            RenderSurfaceHeight,
                                   SURFACE_TRANSFORM SurfacePreTransform)
{
    auto now        = std::chrono::high_resolution_clock::now();
    auto elapsed_ns = std::chrono::duration_cast<std::chrono::duration<float>>(now - m_LastTimestamp);
    m_LastTimestamp = now;
    auto& io        = ImGui::GetIO();
    io.DeltaTime    = elapsed_ns.count();
    io.DisplaySize  = ImVec2(RenderSurfaceWidth / io.DisplayFramebufferScale.x, RenderSurfaceHeight / io.DisplayFramebufferScale.y);
    ImGuiImplDiligent::NewFrame(RenderSurfaceWidth, RenderSurfaceHeight, SurfacePreTransform);
}

void ImGuiImplEmscripten::Render(IDeviceContext* pCtx)
{
    ImGuiImplDiligent::Render(pCtx);
}

bool ImGuiImplEmscripten::OnMouseEvent(int32_t EventType, const EmscriptenMouseEvent* Event)
{
    auto& io = ImGui::GetIO();
    io.AddMousePosEvent(Event->targetX, Event->targetY);
    io.AddMouseButtonEvent(0, Event->buttons & 1);
    io.AddMouseButtonEvent(1, Event->buttons & 2);
    io.AddMouseButtonEvent(2, Event->buttons & 4);
    return io.WantCaptureMouse;
}

bool ImGuiImplEmscripten::OnWheelEvent(int32_t EventType, const EmscriptenWheelEvent* Event)
{
    auto& io = ImGui::GetIO();
    io.AddMouseWheelEvent(float(Event->deltaX / +120.0f), float(Event->deltaY / -120.0f));
    return io.WantCaptureMouse;
}

bool ImGuiImplEmscripten::OnKeyEvent(int32_t EventType, const EmscriptenKeyboardEvent* Event)
{
    auto& io = ImGui::GetIO();
    io.AddKeyEvent(ImGuiKey_ModCtrl, Event->ctrlKey);
    io.AddKeyEvent(ImGuiKey_ModShift, Event->shiftKey);
    io.AddKeyEvent(ImGuiKey_ModAlt, Event->altKey);
    io.AddKeyEvent(ImGuiKey_ModSuper, Event->metaKey);

    switch (EventType)
    {
        case EMSCRIPTEN_EVENT_KEYDOWN:
        {
            bool IsConsume = false;
            io.AddKeyEvent(RemapKeyCodeToImGuiKey(Event->keyCode, &IsConsume), true);
            if (!IsConsume)
                io.AddInputCharactersUTF8(Event->key);
            break;
        }
        case EMSCRIPTEN_EVENT_KEYUP:
        {
            bool IsConsume = false;
            io.AddKeyEvent(RemapKeyCodeToImGuiKey(Event->keyCode, &IsConsume), false);
            break;
        }
        default:
            break;
    }

    return io.WantCaptureKeyboard;
}
} // namespace Diligent
