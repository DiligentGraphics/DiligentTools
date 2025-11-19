/*
 *  Copyright 2025 Diligent Graphics LLC
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

#include "ImGuiImplSDL3.hpp"

#include "Errors.hpp"
#include "RenderDevice.h"
#include "backends/imgui_impl_sdl3.h"

namespace Diligent
{

std::unique_ptr<ImGuiImplSDL3>
ImGuiImplSDL3::Create(const ImGuiDiligentCreateInfo& CI, SDL_Window* pWindow)
{
    return std::make_unique<ImGuiImplSDL3>(CI, pWindow);
}

ImGuiImplSDL3::ImGuiImplSDL3(const ImGuiDiligentCreateInfo& CI,
                             SDL_Window*                    pWindow) :
    ImGuiImplDiligent(CI)
{
    switch (CI.pDevice->GetDeviceInfo().Type)
    {
        case RENDER_DEVICE_TYPE_UNDEFINED:
            LOG_ERROR_AND_THROW("Undefined device type");
            break;
        case RENDER_DEVICE_TYPE_D3D11:
        case RENDER_DEVICE_TYPE_D3D12:
            ImGui_ImplSDL3_InitForD3D(pWindow);
            break;
        case RENDER_DEVICE_TYPE_GL:
        case RENDER_DEVICE_TYPE_GLES:
            ImGui_ImplSDL3_InitForOpenGL(pWindow, nullptr);
            break;
        case RENDER_DEVICE_TYPE_VULKAN:
            ImGui_ImplSDL3_InitForVulkan(pWindow);
            break;
        case RENDER_DEVICE_TYPE_METAL:
            ImGui_ImplSDL3_InitForMetal(pWindow);
            break;
        case RENDER_DEVICE_TYPE_WEBGPU:
            LOG_ERROR_AND_THROW("WebGPU not supported");
            break;
        case RENDER_DEVICE_TYPE_COUNT:
            LOG_ERROR_AND_THROW("Unsupported device type");
            break;
    }
}

ImGuiImplSDL3::~ImGuiImplSDL3() { ImGui_ImplSDL3_Shutdown(); }

void ImGuiImplSDL3::NewFrame(Uint32            RenderSurfaceWidth,
                             Uint32            RenderSurfaceHeight,
                             SURFACE_TRANSFORM SurfacePreTransform)
{
    ImGui_ImplSDL3_NewFrame();
    ImGuiImplDiligent::NewFrame(RenderSurfaceWidth, RenderSurfaceHeight,
                                SurfacePreTransform);
}

void ImGuiImplSDL3::Render(IDeviceContext* pCtx)
{
    ImGuiImplDiligent::Render(pCtx);
}

bool ImGuiImplSDL3::HandleSDLEvent(const SDL_Event* ev)
{
    return ImGui_ImplSDL3_ProcessEvent(ev);
}

void ImGuiImplSDL3::SetGamepadMode(GAMEPAD_MODE GamepadMode, SDL_Gamepad** ppManualGamepadsArray, int ManualGamepadsCount)
{
    ImGui_ImplSDL3_GamepadMode imgGamepadMode{};
    switch (GamepadMode)
    {
        case GAMEPAD_MODE_AUTO_FIRST:
            imgGamepadMode = ImGui_ImplSDL3_GamepadMode_AutoFirst;
            break;
        case GAMEPAD_MODE_AUTO_ALL:
            imgGamepadMode = ImGui_ImplSDL3_GamepadMode_AutoAll;
            break;
        case GAMEPAD_MODE_MANUAL:
            imgGamepadMode = ImGui_ImplSDL3_GamepadMode_Manual;
            break;
    }
    ImGui_ImplSDL3_SetGamepadMode(imgGamepadMode, ppManualGamepadsArray, ManualGamepadsCount);
}

} // namespace Diligent
