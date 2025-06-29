/*
 *  Copyright 2019-2023 Diligent Graphics LLC
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

#include "ImGuiImplLinuxX11.hpp"

#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <X11/Xutil.h>

// Defined by XLib
#ifdef Bool
#    undef Bool
#endif

#ifdef True
#    undef True
#endif

#ifdef False
#    undef False
#endif


#include "imgui.h"

#include "DebugUtilities.hpp"

namespace Diligent
{

std::unique_ptr<ImGuiImplLinuxX11> ImGuiImplLinuxX11::Create(const ImGuiDiligentCreateInfo& CI,
                                                             Uint32                         DisplayWidth,
                                                             Uint32                         DisplayHeight)
{
    return std::make_unique<ImGuiImplLinuxX11>(CI, DisplayWidth, DisplayHeight);
}

ImGuiImplLinuxX11::ImGuiImplLinuxX11(const ImGuiDiligentCreateInfo& CI,
                                     Uint32                         DisplayWidth,
                                     Uint32                         DisplayHeight) :
    ImGuiImplDiligent{CI}
{

    auto& io       = ImGui::GetIO();
    io.DisplaySize = ImVec2(DisplayWidth, DisplayHeight);

    io.BackendPlatformName = "Diligent-ImGuiImplLinuxX11";

    m_LastTimestamp = std::chrono::high_resolution_clock::now();
}

ImGuiImplLinuxX11::~ImGuiImplLinuxX11()
{
}

void ImGuiImplLinuxX11::NewFrame(Uint32            RenderSurfaceWidth,
                                 Uint32            RenderSurfaceHeight,
                                 SURFACE_TRANSFORM SurfacePreTransform)
{
    auto now        = std::chrono::high_resolution_clock::now();
    auto elapsed_ns = now - m_LastTimestamp;
    m_LastTimestamp = now;
    auto& io        = ImGui::GetIO();
    io.DeltaTime    = static_cast<float>(elapsed_ns.count() / 1e+9);

    VERIFY(io.DisplaySize.x == 0 || io.DisplaySize.x == static_cast<float>(RenderSurfaceWidth), "io.DisplaySize.x (",
           io.DisplaySize.x, " does not match RenderSurfaceWidth (", RenderSurfaceWidth, ")");
    VERIFY(io.DisplaySize.y == 0 || io.DisplaySize.y == static_cast<float>(RenderSurfaceHeight), "io.DisplaySize.y (",
           io.DisplaySize.y, " does not match RenderSurfaceHeight (", RenderSurfaceHeight, ")");

    ImGuiImplDiligent::NewFrame(RenderSurfaceWidth, RenderSurfaceHeight, SurfacePreTransform);
}


bool ImGuiImplLinuxX11::HandleXEvent(XEvent* event)
{
    auto& io = ImGui::GetIO();
    switch (event->type)
    {
        case ButtonPress:
        case ButtonRelease:
        {
            bool  IsPressed = event->type == ButtonPress;
            auto* xbe       = reinterpret_cast<XButtonEvent*>(event);
            switch (xbe->button)
            {
                case Button1: io.MouseDown[0] = IsPressed; break; // Left
                case Button2: io.MouseDown[2] = IsPressed; break; // Middle
                case Button3: io.MouseDown[1] = IsPressed; break; // Right
                case Button4: io.MouseWheel += 1; break;
                case Button5: io.MouseWheel -= 1; break;
            }
            return io.WantCaptureMouse;
        }

        case MotionNotify:
        {
            XMotionEvent* xme = (XMotionEvent*)event;
            io.MousePos       = ImVec2(xme->x, xme->y);
            return io.WantCaptureMouse;
        }

        case ConfigureNotify:
        {
            XConfigureEvent* xce = (XConfigureEvent*)event;
            io.DisplaySize       = ImVec2(xce->width, xce->height);
            return false;
        }

        case KeyPress:
        case KeyRelease:
        {
            bool IsPressed = event->type == KeyPress;
            io.KeyCtrl     = (event->xkey.state & ControlMask) != 0;
            io.KeyShift    = (event->xkey.state & ShiftMask) != 0;
            io.KeyAlt      = (event->xkey.state & Mod1Mask) != 0;

            KeySym        keysym  = 0;
            constexpr int buff_sz = 80;
            char          buffer[buff_sz];
            int           num_char = XLookupString((XKeyEvent*)event, buffer, buff_sz, &keysym, 0);
            
            ImGuiKey imgui_key = ImGuiKey_None;
            switch (keysym)
            {
                // clang-format off
                case XK_Tab:       imgui_key = ImGuiKey_Tab;        break;
                case XK_Left:      imgui_key = ImGuiKey_LeftArrow;  break;
                case XK_Right:     imgui_key = ImGuiKey_RightArrow; break;
                case XK_Up:        imgui_key = ImGuiKey_UpArrow;    break;
                case XK_Down:      imgui_key = ImGuiKey_DownArrow;  break;
                case XK_Page_Up:   imgui_key = ImGuiKey_PageUp;     break;
                case XK_Page_Down: imgui_key = ImGuiKey_PageDown;   break;
                case XK_Home:      imgui_key = ImGuiKey_Home;       break;
                case XK_End:       imgui_key = ImGuiKey_End;        break;
                case XK_Insert:    imgui_key = ImGuiKey_Insert;     break;
                case XK_Delete:    imgui_key = ImGuiKey_Delete;     break;
                case XK_BackSpace: imgui_key = ImGuiKey_Backspace;  break;
                case XK_space:     imgui_key = ImGuiKey_Space;      break;
                case XK_Return:    imgui_key = ImGuiKey_Enter;      break;
                case XK_Escape:    imgui_key = ImGuiKey_Escape;     break;
                case XK_KP_Enter:  imgui_key = ImGuiKey_KeypadEnter;break;
                    // clang-format on
            }

            // Handle character keys
            if (imgui_key == ImGuiKey_None && keysym >= 'A' && keysym <= 'Z')
                imgui_key = (ImGuiKey)(ImGuiKey_A + (keysym - 'A'));
            else if (imgui_key == ImGuiKey_None && keysym >= 'a' && keysym <= 'z')
                imgui_key = (ImGuiKey)(ImGuiKey_A + (keysym - 'a'));
            
            if (imgui_key != ImGuiKey_None)
                io.AddKeyEvent(imgui_key, IsPressed);

            if (IsPressed && num_char > 0)
            {
                for (int i = 0; i < num_char; ++i)
                    io.AddInputCharacter(buffer[i]);
            }

            return io.WantCaptureKeyboard;
        }

        default:
            break;
    }
    return false;
}

} // namespace Diligent
