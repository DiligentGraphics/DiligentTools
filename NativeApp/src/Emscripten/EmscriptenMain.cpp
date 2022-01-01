/*
 *  Copyright 2019-2022 Diligent Graphics LLC
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
#include <string>

#include "NativeAppBase.hpp"
#include "Timer.hpp"

std::unique_ptr<Diligent::NativeAppBase> g_pTheApp  = nullptr;
Diligent::Timer                          g_Timer    = {};
double                                   g_PrevTime = 0.0;

void EventLoopCallback()
{
    auto CurrTime    = g_Timer.GetElapsedTime();
    auto ElapsedTime = CurrTime - g_PrevTime;
    g_PrevTime       = CurrTime;

    if (g_pTheApp->IsReady())
    {
        g_pTheApp->Update(CurrTime, ElapsedTime);
        g_pTheApp->Render();
    }
}

EM_BOOL EventResizeCallback(int32_t EventType, const EmscriptenUiEvent* Event, void* pUserData)
{
    if (g_pTheApp->IsReady())
        g_pTheApp->WindowResize(Event->documentBodyClientWidth, Event->documentBodyClientHeight);
    return true;
}

EM_BOOL EventMouseCallback(int32_t EventType, const EmscriptenMouseEvent* Event, void* pUserData)
{
    g_pTheApp->OnMouseEvent(EventType, Event);
    return true;
}

EM_BOOL EventWheelCallback(int32_t EventType, const EmscriptenWheelEvent* Event, void* pUserData)
{
    g_pTheApp->OnWheelEvent(EventType, Event);
    return true;
}

EM_BOOL EventKeyCallback(int32_t EventType, const EmscriptenKeyboardEvent* Event, void* pUserData)
{
    g_pTheApp->OnKeyEvent(EventType, Event);
    return true;
}


int main(int argc, char* argv[])
{
    g_pTheApp.reset(Diligent::CreateApplication());

    int32_t     CanvasWidth  = 0;
    int32_t     CanvasHeight = 0;
    const char* CanvasID     = "#canvas";

    emscripten_get_canvas_element_size(CanvasID, &CanvasWidth, &CanvasHeight);
    emscripten_set_mousedown_callback(CanvasID, nullptr, true, EventMouseCallback);
    emscripten_set_mouseup_callback(CanvasID, nullptr, true, EventMouseCallback);
    emscripten_set_mousemove_callback(CanvasID, nullptr, true, EventMouseCallback);
    emscripten_set_wheel_callback(CanvasID, nullptr, true, EventWheelCallback);
    emscripten_set_keydown_callback(CanvasID, nullptr, true, EventKeyCallback);
    emscripten_set_keyup_callback(CanvasID, nullptr, true, EventKeyCallback);
    emscripten_set_keypress_callback(CanvasID, nullptr, true, EventKeyCallback);
    emscripten_set_resize_callback(CanvasID, nullptr, true, EventResizeCallback);

    g_pTheApp->OnWindowCreated(CanvasID, CanvasWidth, CanvasHeight);
    emscripten_set_main_loop(EventLoopCallback, 0, true);

    g_pTheApp.reset();
}
