/*
 *  Copyright 2019-2026 Diligent Graphics LLC
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

#include <emscripten.h>
#include <emscripten/html5.h>

#include <memory>
#include <utility>

#include "NativeAppBase.hpp"

namespace
{

struct NativeAppCallbackData
{
    Diligent::NativeAppBase* pApplication = nullptr;
    const char*              CanvasID     = nullptr;
};

std::pair<int32_t, int32_t> ComputeCanvasSize(int32_t Width, int32_t Height)
{
    double  PixelRatio   = emscripten_get_device_pixel_ratio();
    int32_t CanvasWidth  = static_cast<int32_t>(std::ceil(PixelRatio * static_cast<double>(Width)));
    int32_t CanvasHeight = static_cast<int32_t>(std::ceil(PixelRatio * static_cast<double>(Height)));
    return {CanvasWidth, CanvasHeight};
}

void EventLoopCallback(void* pUserData)
{
    auto pAppUserData = static_cast<NativeAppCallbackData*>(pUserData);

    if (pAppUserData->pApplication->IsReady())
    {
        pAppUserData->pApplication->Update();
        pAppUserData->pApplication->Render();
        pAppUserData->pApplication->Present();
    }
}

EM_BOOL EventResizeCallback(int32_t EventType, const EmscriptenUiEvent* Event, void* pUserData)
{
    auto pAppUserData = static_cast<NativeAppCallbackData*>(pUserData);

    auto [CanvasWidth, CanvasHeight] = ComputeCanvasSize(Event->windowInnerWidth, Event->windowInnerHeight);
    emscripten_set_canvas_element_size(pAppUserData->CanvasID, CanvasWidth, CanvasHeight);
    if (pAppUserData->pApplication->IsReady())
        pAppUserData->pApplication->WindowResize(CanvasWidth, CanvasHeight);
    return true;
}

EM_BOOL EventMouseCallback(int32_t EventType, const EmscriptenMouseEvent* Event, void* pUserData)
{
    auto pAppUserData = static_cast<NativeAppCallbackData*>(pUserData);
    pAppUserData->pApplication->OnMouseEvent(EventType, Event);
    return true;
}

EM_BOOL EventWheelCallback(int32_t EventType, const EmscriptenWheelEvent* Event, void* pUserData)
{
    auto pAppUserData = static_cast<NativeAppCallbackData*>(pUserData);
    pAppUserData->pApplication->OnWheelEvent(EventType, Event);
    return true;
}

EM_BOOL EventKeyCallback(int32_t EventType, const EmscriptenKeyboardEvent* Event, void* pUserData)
{
    auto pAppUserData = static_cast<NativeAppCallbackData*>(pUserData);
    pAppUserData->pApplication->OnKeyEvent(EventType, Event);
    return true;
}

} // namespace

int main(int argc, char* argv[])
{
    std::unique_ptr<Diligent::NativeAppBase> pApplication{Diligent::CreateApplication()};

    if (argc > 0 && argv != nullptr)
    {
        auto CmdLineStatus = pApplication->ProcessCommandLine(argc, argv);
        if (CmdLineStatus == Diligent::AppBase::CommandLineStatus::Help)
            return 0;
        else if (CmdLineStatus == Diligent::AppBase::CommandLineStatus::Error)
            return 1;
    }

    NativeAppCallbackData AppUserData{pApplication.get(), "#canvas"};

    int32_t WindowWidth  = EM_ASM_INT({ return window.innerWidth; });
    int32_t WindowHeight = EM_ASM_INT({ return window.innerHeight; });

    auto [CanvasWidth, CanvasHeight] = ComputeCanvasSize(WindowWidth, WindowHeight);
    emscripten_set_canvas_element_size(AppUserData.CanvasID, CanvasWidth, CanvasHeight);

    emscripten_set_mousedown_callback(AppUserData.CanvasID, &AppUserData, true, EventMouseCallback);
    emscripten_set_mouseup_callback(AppUserData.CanvasID, &AppUserData, true, EventMouseCallback);
    emscripten_set_mousemove_callback(AppUserData.CanvasID, &AppUserData, true, EventMouseCallback);
    emscripten_set_wheel_callback(AppUserData.CanvasID, &AppUserData, true, EventWheelCallback);
    emscripten_set_keydown_callback(AppUserData.CanvasID, &AppUserData, true, EventKeyCallback);
    emscripten_set_keyup_callback(AppUserData.CanvasID, &AppUserData, true, EventKeyCallback);
    emscripten_set_keypress_callback(AppUserData.CanvasID, &AppUserData, true, EventKeyCallback);
    emscripten_set_resize_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, &AppUserData, true, EventResizeCallback);
    pApplication->OnWindowCreated(AppUserData.CanvasID, CanvasWidth, CanvasHeight);

    emscripten_set_main_loop_arg(EventLoopCallback, &AppUserData, 0, true);
}
