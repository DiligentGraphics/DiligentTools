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

#pragma once

#include "AppBase.hpp"
#include "Timer.hpp"

struct EmscriptenMouseEvent;
struct EmscriptenWheelEvent;
struct EmscriptenKeyboardEvent;

namespace Diligent
{

/// Base class for Emscripten applications.
class EmscriptenAppBase : public AppBase
{
public:
    using AppBase::Update;

    virtual void OnMouseEvent(int32_t EventType, const EmscriptenMouseEvent* Event) = 0;

    virtual void OnWheelEvent(int32_t EventType, const EmscriptenWheelEvent* Event) = 0;

    virtual void OnKeyEvent(int32_t EventType, const EmscriptenKeyboardEvent* Event) = 0;

    virtual void OnWindowCreated(const char* pCanvasID,
                                 int32_t     WindowWidth,
                                 int32_t     WindowHeight) = 0;
};

} // namespace Diligent
