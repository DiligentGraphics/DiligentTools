/*     Copyright 2019-2023 Diligent Graphics LLC
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

#include "imgui.h"
#include "ImGuiImplMacOS.hpp"
#include "backends/imgui_impl_osx.h"
#import <Cocoa/Cocoa.h>

ImGuiKey ImGui_ImplOSX_KeyCodeToImGuiKey(int key_code);

namespace Diligent
{

std::unique_ptr<ImGuiImplMacOS> ImGuiImplMacOS::Create(const ImGuiDiligentCreateInfo& CI, void* _Nullable view)
{
    return std::make_unique<ImGuiImplMacOS>(CI, view);
}

ImGuiImplMacOS::ImGuiImplMacOS(const ImGuiDiligentCreateInfo& CI, void* _Nullable view) :
    ImGuiImplDiligent{CI}
{
    ImGui_ImplOSX_Init((NSView*)view);
    ImGuiIO& io = ImGui::GetIO();
    io.BackendPlatformName = "Diligent-ImGuiImplMacOS";

    const auto framebufferScale = ((NSView*)view).window.screen.backingScaleFactor ?: NSScreen.mainScreen.backingScaleFactor;;
    io.DisplayFramebufferScale = ImVec2(framebufferScale, framebufferScale);
}

ImGuiImplMacOS::~ImGuiImplMacOS()
{
    ImGui_ImplOSX_Shutdown();
}

void ImGuiImplMacOS::NewFrame(Uint32            RenderSurfaceWidth,
                              Uint32            RenderSurfaceHeight,
                              SURFACE_TRANSFORM SurfacePreTransform)
{
    std::lock_guard<std::mutex> Lock(m_Mtx);
    ImGuiIO& io = ImGui::GetIO();
    io.DisplaySize = ImVec2(RenderSurfaceWidth / io.DisplayFramebufferScale.x, RenderSurfaceHeight / io.DisplayFramebufferScale.y);
    ImGui_ImplOSX_NewFrame(nil);
    ImGuiImplDiligent::NewFrame(RenderSurfaceWidth, RenderSurfaceHeight, SurfacePreTransform);
}


// Must only be called for a mouse event, otherwise an exception occurs
// (Note that NSEventTypeScrollWheel is considered "other input". Oddly enough an exception does not occur with it, but the value will sometimes be wrong!)
static ImGuiMouseSource GetMouseSource(NSEvent* event)
{
    switch (event.subtype)
    {
        case NSEventSubtypeTabletPoint:
            return ImGuiMouseSource_Pen;
        // macOS considers input from relative touch devices (like the trackpad or Apple Magic Mouse) to be touch input.
        // This doesn't really make sense for Dear ImGui, which expects absolute touch devices only.
        // There does not seem to be a simple way to disambiguate things here so we consider NSEventSubtypeTouch events to always come from mice.
        // See https://developer.apple.com/library/archive/documentation/Cocoa/Conceptual/EventOverview/HandlingTouchEvents/HandlingTouchEvents.html#//apple_ref/doc/uid/10000060i-CH13-SW24
        //case NSEventSubtypeTouch:
        //    return ImGuiMouseSource_TouchScreen;
        case NSEventSubtypeMouseEvent:
        default:
            return ImGuiMouseSource_Mouse;
    }
}

static bool ImGui_ImplOSX_HandleEvent(NSEvent* event, NSView* view)
{
    // Only process events from the window containing ImGui view
    if (event.window != view.window)
        return false;
    ImGuiIO& io = ImGui::GetIO();

    if (event.type == NSEventTypeLeftMouseDown || event.type == NSEventTypeRightMouseDown || event.type == NSEventTypeOtherMouseDown)
    {
        int button = (int)[event buttonNumber];
        if (button >= 0 && button < ImGuiMouseButton_COUNT)
        {
            io.AddMouseSourceEvent(GetMouseSource(event));
            io.AddMouseButtonEvent(button, true);
        }
        return io.WantCaptureMouse;
    }

    if (event.type == NSEventTypeLeftMouseUp || event.type == NSEventTypeRightMouseUp || event.type == NSEventTypeOtherMouseUp)
    {
        int button = (int)[event buttonNumber];
        if (button >= 0 && button < ImGuiMouseButton_COUNT)
        {
            io.AddMouseSourceEvent(GetMouseSource(event));
            io.AddMouseButtonEvent(button, false);
        }
        return io.WantCaptureMouse;
    }

    if (event.type == NSEventTypeMouseMoved || event.type == NSEventTypeLeftMouseDragged || event.type == NSEventTypeRightMouseDragged || event.type == NSEventTypeOtherMouseDragged)
    {
        NSPoint mousePoint = event.locationInWindow;
        if (event.window == nil)
            mousePoint = [[view window] convertPointFromScreen:mousePoint];
        mousePoint = [view convertPoint:mousePoint fromView:nil];
        if ([view isFlipped])
            mousePoint = NSMakePoint(mousePoint.x, mousePoint.y);
        else
            mousePoint = NSMakePoint(mousePoint.x, view.bounds.size.height - mousePoint.y);
        io.AddMouseSourceEvent(GetMouseSource(event));
        io.AddMousePosEvent((float)mousePoint.x, (float)mousePoint.y);
        return io.WantCaptureMouse;
    }

    if (event.type == NSEventTypeScrollWheel)
    {
        // Ignore canceled events.
        //
        // From macOS 12.1, scrolling with two fingers and then decelerating
        // by tapping two fingers results in two events appearing:
        //
        // 1. A scroll wheel NSEvent, with a phase == NSEventPhaseMayBegin, when the user taps
        // two fingers to decelerate or stop the scroll events.
        //
        // 2. A scroll wheel NSEvent, with a phase == NSEventPhaseCancelled, when the user releases the
        // two-finger tap. It is this event that sometimes contains large values for scrollingDeltaX and
        // scrollingDeltaY. When these are added to the current x and y positions of the scrolling view,
        // it appears to jump up or down. It can be observed in Preview, various JetBrains IDEs and here.
        if (event.phase == NSEventPhaseCancelled)
            return false;

        double wheel_dx = 0.0;
        double wheel_dy = 0.0;

        #if MAC_OS_X_VERSION_MAX_ALLOWED >= 1070
        if (floor(NSAppKitVersionNumber) > NSAppKitVersionNumber10_6)
        {
            wheel_dx = [event scrollingDeltaX];
            wheel_dy = [event scrollingDeltaY];
            if ([event hasPreciseScrollingDeltas])
            {
                wheel_dx *= 0.01;
                wheel_dy *= 0.01;
            }
        }
        else
        #endif // MAC_OS_X_VERSION_MAX_ALLOWED
        {
            wheel_dx = [event deltaX] * 0.1;
            wheel_dy = [event deltaY] * 0.1;
        }
        if (wheel_dx != 0.0 || wheel_dy != 0.0)
            io.AddMouseWheelEvent((float)wheel_dx, (float)wheel_dy);

        return io.WantCaptureMouse;
    }

    if (event.type == NSEventTypeKeyDown || event.type == NSEventTypeKeyUp)
    {
        if ([event isARepeat])
            return io.WantCaptureKeyboard;

        int key_code = (int)[event keyCode];
        ImGuiKey key = ImGui_ImplOSX_KeyCodeToImGuiKey(key_code);
        io.AddKeyEvent(key, event.type == NSEventTypeKeyDown);
        io.SetKeyEventNativeData(key, key_code, -1); // To support legacy indexing (<1.87 user code)

        return io.WantCaptureKeyboard;
    }

    if (event.type == NSEventTypeFlagsChanged)
    {
        unsigned short key_code = [event keyCode];
        NSEventModifierFlags modifier_flags = [event modifierFlags];

        io.AddKeyEvent(ImGuiMod_Shift, (modifier_flags & NSEventModifierFlagShift)   != 0);
        io.AddKeyEvent(ImGuiMod_Ctrl,  (modifier_flags & NSEventModifierFlagControl) != 0);
        io.AddKeyEvent(ImGuiMod_Alt,   (modifier_flags & NSEventModifierFlagOption)  != 0);
        io.AddKeyEvent(ImGuiMod_Super, (modifier_flags & NSEventModifierFlagCommand) != 0);

        ImGuiKey key = ImGui_ImplOSX_KeyCodeToImGuiKey(key_code);
        if (key != ImGuiKey_None)
        {
            // macOS does not generate down/up event for modifiers. We're trying
            // to use hardware dependent masks to extract that information.
            // 'imgui_mask' is left as a fallback.
            NSEventModifierFlags mask = 0;
            switch (key)
            {
                case ImGuiKey_LeftCtrl:   mask = 0x0001; break;
                case ImGuiKey_RightCtrl:  mask = 0x2000; break;
                case ImGuiKey_LeftShift:  mask = 0x0002; break;
                case ImGuiKey_RightShift: mask = 0x0004; break;
                case ImGuiKey_LeftSuper:  mask = 0x0008; break;
                case ImGuiKey_RightSuper: mask = 0x0010; break;
                case ImGuiKey_LeftAlt:    mask = 0x0020; break;
                case ImGuiKey_RightAlt:   mask = 0x0040; break;
                default:
                    return io.WantCaptureKeyboard;
            }
            io.AddKeyEvent(key, (modifier_flags & mask) != 0);
            io.SetKeyEventNativeData(key, key_code, -1); // To support legacy indexing (<1.87 user code)
        }

        return io.WantCaptureKeyboard;
    }

    return false;
}

bool ImGuiImplMacOS::HandleOSXEvent(NSEvent *_Nonnull event, NSView *_Nonnull view)
{
    std::lock_guard<std::mutex> Lock(m_Mtx);
    ImGuiIO& io = ImGui::GetIO();
    if (event.type == NSEventTypeMouseMoved || event.type == NSEventTypeLeftMouseDragged)
    {
        NSRect  viewRectPoints = [view bounds];
        NSPoint curPoint       = [view convertPoint:[event locationInWindow] fromView:nil];
        io.MousePos.x = curPoint.x;
        io.MousePos.y = viewRectPoints.size.height-1 - curPoint.y;
        return io.WantCaptureMouse;
    }

    return ImGui_ImplOSX_HandleEvent((NSEvent*)event, (NSView*)view);
}

void ImGuiImplMacOS::Render(IDeviceContext* pCtx)
{
    std::lock_guard<std::mutex> Lock(m_Mtx);
    ImGuiImplDiligent::Render(pCtx);
}

}
