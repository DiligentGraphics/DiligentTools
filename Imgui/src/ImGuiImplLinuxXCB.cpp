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

#include "ImGuiImplLinuxXCB.hpp"

#include "imgui.h"

#include <X11/keysym.h>
#include "xcb_keysyms/xcb_keysyms.h"

#include "DebugUtilities.hpp"

namespace Diligent
{

std::unique_ptr<ImGuiImplLinuxXCB> ImGuiImplLinuxXCB::Create(const ImGuiDiligentCreateInfo& CI,
                                                             xcb_connection_t*              connection,
                                                             Uint32                         DisplayWidth,
                                                             Uint32                         DisplayHeight)
{
    return std::make_unique<ImGuiImplLinuxXCB>(CI, connection, DisplayWidth, DisplayHeight);
}

ImGuiImplLinuxXCB::ImGuiImplLinuxXCB(const ImGuiDiligentCreateInfo& CI,
                                     xcb_connection_t*              connection,
                                     Uint32                         DisplayWidth,
                                     Uint32                         DisplayHeight) :
    ImGuiImplDiligent{CI}
{
    m_syms = xcb_key_symbols_alloc((xcb_connection_t*)connection);

    ImGuiIO& io    = ImGui::GetIO();
    io.DisplaySize = ImVec2(DisplayWidth, DisplayHeight);

    io.BackendPlatformName = "Diligent-ImGuiImplLinuxXCB";

    m_LastTimestamp = std::chrono::high_resolution_clock::now();
}

ImGuiImplLinuxXCB::~ImGuiImplLinuxXCB()
{
    if (m_syms)
    {
        xcb_key_symbols_free(m_syms);
    }
}

void ImGuiImplLinuxXCB::NewFrame(Uint32            RenderSurfaceWidth,
                                 Uint32            RenderSurfaceHeight,
                                 SURFACE_TRANSFORM SurfacePreTransform)
{
    auto now        = std::chrono::high_resolution_clock::now();
    auto elapsed_ns = now - m_LastTimestamp;
    m_LastTimestamp = now;
    ImGuiIO& io     = ImGui::GetIO();
    io.DeltaTime    = static_cast<float>(elapsed_ns.count() / 1e+9);

    VERIFY(io.DisplaySize.x == 0 || io.DisplaySize.x == static_cast<float>(RenderSurfaceWidth), "io.DisplaySize.x (",
           io.DisplaySize.x, " does not match RenderSurfaceWidth (", RenderSurfaceWidth, ")");
    VERIFY(io.DisplaySize.y == 0 || io.DisplaySize.y == static_cast<float>(RenderSurfaceHeight), "io.DisplaySize.y (",
           io.DisplaySize.y, " does not match RenderSurfaceHeight (", RenderSurfaceHeight, ")");

    ImGuiImplDiligent::NewFrame(RenderSurfaceWidth, RenderSurfaceHeight, SurfacePreTransform);
}

// ----------------------------------------------------------------------
// ----------------------------------------------------------------------
void ImGuiImplLinuxXCB::HandleKeyEvent(xcb_key_release_event_t* event)
{
    bool IsKeyPressed = (event->response_type & 0x7f) == XCB_KEY_PRESS;

    ImGuiIO& io = ImGui::GetIO();

    // Handle modifier keys
    bool IsCtrlPressed  = (event->state & XCB_MOD_MASK_CONTROL) != 0;
    bool IsShiftPressed = (event->state & XCB_MOD_MASK_SHIFT) != 0;
    bool IsAltPressed   = (event->state & XCB_MOD_MASK_1) != 0;
    io.AddKeyEvent(ImGuiKey_ModCtrl, IsCtrlPressed);
    io.AddKeyEvent(ImGuiKey_ModShift, IsShiftPressed);
    io.AddKeyEvent(ImGuiKey_ModAlt, IsAltPressed);

    // Lookup the keysym
    xcb_keysym_t keysym = xcb_key_press_lookup_keysym(m_syms, event, 0);

    ImGuiKey k = ImGuiKey_None;
    switch (keysym)
    {
            // clang-format off
        case XK_Tab:       k = ImGuiKey_Tab;        break;
        case XK_Left:      k = ImGuiKey_LeftArrow;  break;
        case XK_Right:     k = ImGuiKey_RightArrow; break;
        case XK_Up:        k = ImGuiKey_UpArrow;    break;
        case XK_Down:      k = ImGuiKey_DownArrow;  break;
        case XK_Page_Up:   k = ImGuiKey_PageUp;     break;
        case XK_Page_Down: k = ImGuiKey_PageDown;   break;
        case XK_Home:      k = ImGuiKey_Home;       break;
        case XK_End:       k = ImGuiKey_End;        break;
        case XK_Insert:    k = ImGuiKey_Insert;     break;
        case XK_Delete:    k = ImGuiKey_Delete;     break;
        case XK_BackSpace: k = ImGuiKey_Backspace;  break;
        case XK_Return:    k = ImGuiKey_Enter;      break;
        case XK_Escape:    k = ImGuiKey_Escape;     break;
        case XK_KP_Enter:  k = ImGuiKey_Enter;      break;
            // clang-format on

        default:
            if (keysym >= 'a' && keysym <= 'z')
                k = static_cast<ImGuiKey>(ImGuiKey_A + (keysym - 'a'));
            else if (keysym >= 'A' && keysym <= 'Z')
                k = static_cast<ImGuiKey>(ImGuiKey_A + (keysym - 'A'));
    }

    if (k != ImGuiKey_None)
        io.AddKeyEvent(k, IsKeyPressed);

    // Add input characters for pressed keys
    if (IsKeyPressed && keysym > 12 && keysym < 127)
    {
        if (IsShiftPressed)
        {
            if (keysym >= 'a' && keysym <= 'z')
                keysym += (int)'A' - (int)'a';
            else
            {
                switch (keysym)
                {
                    case '`': keysym = '~'; break;
                    case '1': keysym = '!'; break;
                    case '2': keysym = '@'; break;
                    case '3': keysym = '#'; break;
                    case '4': keysym = '$'; break;
                    case '5': keysym = '%'; break;
                    case '6': keysym = '^'; break;
                    case '7': keysym = '&'; break;
                    case '8': keysym = '*'; break;
                    case '9': keysym = '('; break;
                    case '0': keysym = ')'; break;
                    case '-': keysym = '_'; break;
                    case '=': keysym = '+'; break;
                    case '[': keysym = '{'; break;
                    case ']': keysym = '}'; break;
                    case '\\': keysym = '|'; break;
                    case ';': keysym = ':'; break;
                    case '\'': keysym = '\"'; break;
                    case ',': keysym = '<'; break;
                    case '.': keysym = '>'; break;
                    case '/': keysym = '?'; break;
                }
            }
        }
        io.AddInputCharacter(keysym);
    }
}

bool ImGuiImplLinuxXCB::HandleXCBEvent(xcb_generic_event_t* event)
{
    ImGuiIO& io = ImGui::GetIO();
    switch (event->response_type & 0x7f)
    {
        case XCB_MOTION_NOTIFY:
        {
            xcb_motion_notify_event_t* motion = (xcb_motion_notify_event_t*)event;
            io.MousePos                       = ImVec2(motion->event_x, motion->event_y);
            return io.WantCaptureMouse;
        }
        break;

        case XCB_BUTTON_PRESS:
        {
            xcb_button_press_event_t* press = (xcb_button_press_event_t*)event;
            switch (press->detail)
            {
                case XCB_BUTTON_INDEX_1: io.MouseDown[0] = true; break; // left
                case XCB_BUTTON_INDEX_2: io.MouseDown[2] = true; break; // middle
                case XCB_BUTTON_INDEX_3: io.MouseDown[1] = true; break; // right
                case XCB_BUTTON_INDEX_4: io.MouseWheel += 1; break;
                case XCB_BUTTON_INDEX_5: io.MouseWheel -= 1; break;
            }

            return io.WantCaptureMouse;
        }
        break;

        case XCB_BUTTON_RELEASE:
        {
            xcb_button_release_event_t* press = (xcb_button_release_event_t*)event;
            switch (press->detail)
            {
                case XCB_BUTTON_INDEX_1: io.MouseDown[0] = false; break; // left
                case XCB_BUTTON_INDEX_2: io.MouseDown[2] = false; break; // middle
                case XCB_BUTTON_INDEX_3: io.MouseDown[1] = false; break; // right
            }

            return io.WantCaptureMouse;
        }
        break;

        case XCB_KEY_RELEASE:
        case XCB_KEY_PRESS:
        {
            xcb_key_press_event_t* keyEvent = (xcb_key_press_event_t*)event;
            HandleKeyEvent(keyEvent);
            return io.WantCaptureKeyboard;
        }
        break;

        case XCB_CONFIGURE_NOTIFY:
        {
            const xcb_configure_notify_event_t* cfgEvent = (const xcb_configure_notify_event_t*)event;

            io.DisplaySize = ImVec2(cfgEvent->width, cfgEvent->height);
            return false;
        }
        break;

        default:
            break;
    }

    return false;
}

} // namespace Diligent
