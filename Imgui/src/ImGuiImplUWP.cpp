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

#include "WinHPreface.h"
#include <wrl.h>
#include <wrl/client.h>
#include "WinHPostface.h"

#include "imgui.h"
#include "ImGuiImplUWP.hpp"


namespace Diligent
{

std::unique_ptr<ImGuiImplUWP> ImGuiImplUWP::Create(const ImGuiDiligentCreateInfo& CI)
{
    return std::make_unique<ImGuiImplUWP>(CI);
}

ImGuiImplUWP::ImGuiImplUWP(const ImGuiDiligentCreateInfo& CI) :
    ImGuiImplDiligent{CI}
{
    ::QueryPerformanceFrequency((LARGE_INTEGER*)&m_TicksPerSecond);
    ::QueryPerformanceCounter((LARGE_INTEGER*)&m_Time);

    // Setup back-end capabilities flags
    ImGuiIO& io            = ImGui::GetIO();
    io.BackendPlatformName = "imgui_impl_uwp";
}

ImGuiImplUWP::~ImGuiImplUWP()
{
}

void ImGuiImplUWP::NewFrame(Uint32            RenderSurfaceWidth,
                            Uint32            RenderSurfaceHeight,
                            SURFACE_TRANSFORM SurfacePreTransform)
{
    ImGuiIO& io = ImGui::GetIO();

    io.DisplaySize = ImVec2(static_cast<float>(RenderSurfaceWidth), static_cast<float>(RenderSurfaceHeight));

    // Setup time step
    INT64 current_time;
    ::QueryPerformanceCounter((LARGE_INTEGER*)&current_time);
    io.DeltaTime = (float)(current_time - m_Time) / m_TicksPerSecond;
    m_Time       = current_time;

    io.KeySuper = false;

    ImGuiImplDiligent::NewFrame(RenderSurfaceWidth, RenderSurfaceHeight, SurfacePreTransform);
}

} // namespace Diligent
