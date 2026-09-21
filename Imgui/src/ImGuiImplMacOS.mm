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

#import <Cocoa/Cocoa.h>

#include <cstring>

// Undocumented methods for creating cursors.
@interface NSCursor ()
+ (id)_windowResizeNorthWestSouthEastCursor;
+ (id)_windowResizeNorthEastSouthWestCursor;
+ (id)busyButClickableCursor;
@end

@interface DgImGuiMacFocusObserver : NSObject
- (instancetype)initWithActive:(std::atomic<bool>*)pActive dirty:(std::atomic<bool>*)pDirty;
@end

@implementation DgImGuiMacFocusObserver
{
    std::atomic<bool>* _pActive;
    std::atomic<bool>* _pDirty;
}
- (instancetype)initWithActive:(std::atomic<bool>*)pActive dirty:(std::atomic<bool>*)pDirty
{
    if (self = [super init])
    {
        _pActive = pActive;
        _pDirty  = pDirty;
    }
    return self;
}
- (void)onApplicationBecomeActive:(NSNotification*)aNotification
{
    _pActive->store(true);
    _pDirty->store(true);
}
- (void)onApplicationBecomeInactive:(NSNotification*)aNotification
{
    _pActive->store(false);
    _pDirty->store(true);
}
@end

namespace Diligent
{

namespace
{

ImGuiKey MapCharacterToImGuiKey(int32_t c)
{
    if (c >= 'a' && c <= 'z')
        return static_cast<ImGuiKey>(ImGuiKey_A + (c - 'a'));
    if (c >= 'A' && c <= 'Z')
        return static_cast<ImGuiKey>(ImGuiKey_A + (c - 'A'));
    if (c >= '0' && c <= '9')
        return static_cast<ImGuiKey>(ImGuiKey_0 + (c - '0'));

    switch (c)
    {
        case 9:   return ImGuiKey_Tab;
        case 13:  return ImGuiKey_Enter;
        case 27:  return ImGuiKey_Escape;
        case 127: return ImGuiKey_Backspace;
        case 25:  return ImGuiKey_Tab;

        case 0xF700: return ImGuiKey_UpArrow;
        case 0xF701: return ImGuiKey_DownArrow;
        case 0xF702: return ImGuiKey_LeftArrow;
        case 0xF703: return ImGuiKey_RightArrow;
        case 0xF704: return ImGuiKey_F1;
        case 0xF705: return ImGuiKey_F2;
        case 0xF706: return ImGuiKey_F3;
        case 0xF707: return ImGuiKey_F4;
        case 0xF708: return ImGuiKey_F5;
        case 0xF709: return ImGuiKey_F6;
        case 0xF70A: return ImGuiKey_F7;
        case 0xF70B: return ImGuiKey_F8;
        case 0xF70C: return ImGuiKey_F9;
        case 0xF70D: return ImGuiKey_F10;
        case 0xF70E: return ImGuiKey_F11;
        case 0xF70F: return ImGuiKey_F12;
        case 0xF729: return ImGuiKey_Home;
        case 0xF72B: return ImGuiKey_End;
        case 0xF72C: return ImGuiKey_PageUp;
        case 0xF72D: return ImGuiKey_PageDown;
    }

    return ImGuiKey_None;
}

// 4-way move cursor matching CSS "move"; macOS has no built-in equivalent.
NSCursor* CreateMoveCursor()
{
    const CGFloat S = 24.0;
    const CGFloat c = S * 0.5;
    const CGFloat H = 10.0;
    const CGFloat h = 5.0;
    const CGFloat a = 4.0;
    const CGFloat w = 1.5;

    NSImage* image = [NSImage imageWithSize:NSMakeSize(S, S)
                                    flipped:NO
                             drawingHandler:^BOOL(NSRect dstRect) {
        NSBezierPath* glyph = [NSBezierPath bezierPath];

        [glyph moveToPoint:NSMakePoint(c,     c + H)];
        [glyph lineToPoint:NSMakePoint(c + a, c + H - h)];
        [glyph lineToPoint:NSMakePoint(c + w, c + H - h)];
        [glyph lineToPoint:NSMakePoint(c + w, c - H + h)];
        [glyph lineToPoint:NSMakePoint(c + a, c - H + h)];
        [glyph lineToPoint:NSMakePoint(c,     c - H)];
        [glyph lineToPoint:NSMakePoint(c - a, c - H + h)];
        [glyph lineToPoint:NSMakePoint(c - w, c - H + h)];
        [glyph lineToPoint:NSMakePoint(c - w, c + H - h)];
        [glyph lineToPoint:NSMakePoint(c - a, c + H - h)];
        [glyph closePath];

        [glyph moveToPoint:NSMakePoint(c + H,     c)];
        [glyph lineToPoint:NSMakePoint(c + H - h, c + a)];
        [glyph lineToPoint:NSMakePoint(c + H - h, c + w)];
        [glyph lineToPoint:NSMakePoint(c - H + h, c + w)];
        [glyph lineToPoint:NSMakePoint(c - H + h, c + a)];
        [glyph lineToPoint:NSMakePoint(c - H,     c)];
        [glyph lineToPoint:NSMakePoint(c - H + h, c - a)];
        [glyph lineToPoint:NSMakePoint(c - H + h, c - w)];
        [glyph lineToPoint:NSMakePoint(c + H - h, c - w)];
        [glyph lineToPoint:NSMakePoint(c + H - h, c - a)];
        [glyph closePath];

        [[NSColor whiteColor] setStroke];
        glyph.lineWidth  = 2.0;
        glyph.miterLimit = 2.0;
        [glyph stroke];
        [[NSColor blackColor] setFill];
        [glyph fill];
        return YES;
    }];

    return [[[NSCursor alloc] initWithImage:image hotSpot:NSMakePoint(c, c)] autorelease];
}

} // namespace

std::unique_ptr<ImGuiImplMacOS> ImGuiImplMacOS::Create(const ImGuiDiligentCreateInfo& CI, void* _Nullable view)
{
    return std::make_unique<ImGuiImplMacOS>(CI, view);
}

ImGuiImplMacOS::ImGuiImplMacOS(const ImGuiDiligentCreateInfo& CI, void* _Nullable view) :
    ImGuiImplDiligent{CI}
{
    ImGuiIO& io            = ImGui::GetIO();
    io.BackendPlatformName = "Diligent-ImGuiImplMacOS";
    io.BackendFlags |= ImGuiBackendFlags_HasMouseCursors;

    const auto framebufferScale = ((NSView*)view).window.screen.backingScaleFactor ?: NSScreen.mainScreen.backingScaleFactor;
    io.DisplayFramebufferScale  = ImVec2(framebufferScale, framebufferScale);

    InitMouseCursors();

    io.SetClipboardTextFn = [](void*, const char* str) {
        NSPasteboard* pasteboard = [NSPasteboard generalPasteboard];
        [pasteboard declareTypes:[NSArray arrayWithObject:NSPasteboardTypeString] owner:nil];
        [pasteboard setString:[NSString stringWithUTF8String:str] forType:NSPasteboardTypeString];
    };
    io.GetClipboardTextFn = [](void*) -> const char* {
        NSPasteboard* pasteboard = [NSPasteboard generalPasteboard];
        NSString*     available  = [pasteboard availableTypeFromArray:[NSArray arrayWithObject:NSPasteboardTypeString]];
        if (![available isEqualToString:NSPasteboardTypeString])
            return nullptr;
        NSString* string = [pasteboard stringForType:NSPasteboardTypeString];
        if (string == nil)
            return nullptr;
        const char*           string_c = [string UTF8String];
        static ImVector<char> s_clipboard;
        s_clipboard.resize(static_cast<int>(strlen(string_c)) + 1);
        strcpy(s_clipboard.Data, string_c);
        return s_clipboard.Data;
    };

    m_FocusObserver = [[DgImGuiMacFocusObserver alloc] initWithActive:&m_AppActive dirty:&m_FocusDirty];
    [[NSNotificationCenter defaultCenter] addObserver:m_FocusObserver
                                             selector:@selector(onApplicationBecomeActive:)
                                                 name:NSApplicationDidBecomeActiveNotification
                                               object:nil];
    [[NSNotificationCenter defaultCenter] addObserver:m_FocusObserver
                                             selector:@selector(onApplicationBecomeInactive:)
                                                 name:NSApplicationDidResignActiveNotification
                                               object:nil];

    m_LastTime = std::chrono::high_resolution_clock::now();
}

ImGuiImplMacOS::~ImGuiImplMacOS()
{
    if (m_FocusObserver != nullptr)
    {
        [[NSNotificationCenter defaultCenter] removeObserver:m_FocusObserver];
        [m_FocusObserver release];
        m_FocusObserver = nullptr;
    }
    for (NSCursor* Cursor : m_MouseCursors)
        [Cursor release];
}

void ImGuiImplMacOS::InitMouseCursors()
{
    m_MouseCursors[ImGuiMouseCursor_Arrow]      = [NSCursor arrowCursor];
    m_MouseCursors[ImGuiMouseCursor_TextInput]  = [NSCursor IBeamCursor];
    m_MouseCursors[ImGuiMouseCursor_Hand]       = [NSCursor pointingHandCursor];
    m_MouseCursors[ImGuiMouseCursor_NotAllowed] = [NSCursor operationNotAllowedCursor];
    m_MouseCursors[ImGuiMouseCursor_ResizeNS]   = [NSCursor resizeUpDownCursor];
    m_MouseCursors[ImGuiMouseCursor_ResizeEW]   = [NSCursor resizeLeftRightCursor];
    m_MouseCursors[ImGuiMouseCursor_ResizeNESW] = [NSCursor respondsToSelector:@selector(_windowResizeNorthEastSouthWestCursor)]
        ? [NSCursor _windowResizeNorthEastSouthWestCursor] : [NSCursor closedHandCursor];
    m_MouseCursors[ImGuiMouseCursor_ResizeNWSE] = [NSCursor respondsToSelector:@selector(_windowResizeNorthWestSouthEastCursor)]
        ? [NSCursor _windowResizeNorthWestSouthEastCursor] : [NSCursor closedHandCursor];
    m_MouseCursors[ImGuiMouseCursor_ResizeAll]  = CreateMoveCursor() ?: [NSCursor closedHandCursor];
    m_MouseCursors[ImGuiMouseCursor_Wait] = m_MouseCursors[ImGuiMouseCursor_Progress] =
        [NSCursor respondsToSelector:@selector(busyButClickableCursor)] ? [NSCursor busyButClickableCursor] : [NSCursor arrowCursor];

    for (NSCursor* Cursor : m_MouseCursors)
        [Cursor retain];
}

void ImGuiImplMacOS::UpdateMouseCursor()
{
    ImGuiIO& io = ImGui::GetIO();
    if (io.ConfigFlags & ImGuiConfigFlags_NoMouseCursorChange)
        return;

    const ImGuiMouseCursor Cursor = io.MouseDrawCursor ? ImGuiMouseCursor_None : ImGui::GetMouseCursor();
    if (m_LastMouseCursor == static_cast<int32_t>(Cursor))
        return;
    const bool WasNone = m_LastMouseCursor == ImGuiMouseCursor_None;
    m_LastMouseCursor  = static_cast<int32_t>(Cursor);

    // NSCursor is main-thread state; Render() runs on the display link thread.
    if (Cursor == ImGuiMouseCursor_None)
    {
        dispatch_async(dispatch_get_main_queue(), ^{
            [NSCursor hide];
        });
    }
    else
    {
        NSCursor* NewCursor = m_MouseCursors[Cursor] ?: m_MouseCursors[ImGuiMouseCursor_Arrow];
        dispatch_async(dispatch_get_main_queue(), ^{
            [NewCursor set];
            if (WasNone)
                [NSCursor unhide];
        });
    }
}

void ImGuiImplMacOS::NewFrame(Uint32            RenderSurfaceWidth,
                              Uint32            RenderSurfaceHeight,
                              SURFACE_TRANSFORM SurfacePreTransform)
{
    std::lock_guard<std::mutex> Lock(m_Mtx);
    ImGuiIO& io = ImGui::GetIO();

    if (m_FocusDirty.exchange(false))
    {
        const bool Active = m_AppActive.load();
        io.AddFocusEvent(Active);
        if (!Active)
        {
            io.ClearInputKeys();
            m_KeysPressedWithCmd.clear();
        }
    }

    // macOS sends no key-up for keys pressed while Cmd is held; release them here.
    for (ImGuiKey Key : m_KeysPressedWithCmd)
        io.AddKeyEvent(Key, false);
    m_KeysPressedWithCmd.clear();

    const auto  Now     = std::chrono::high_resolution_clock::now();
    const float Elapsed = std::chrono::duration<float>(Now - m_LastTime).count();
    m_LastTime          = Now;
    io.DeltaTime        = Elapsed > 0.0f ? Elapsed : 1.0f / 60.0f;

    io.DisplaySize = ImVec2(RenderSurfaceWidth / io.DisplayFramebufferScale.x,
                            RenderSurfaceHeight / io.DisplayFramebufferScale.y);

    ImGuiImplDiligent::NewFrame(RenderSurfaceWidth, RenderSurfaceHeight, SurfacePreTransform);
}

void ImGuiImplMacOS::Render(IDeviceContext* pCtx)
{
    std::lock_guard<std::mutex> Lock(m_Mtx);
    UpdateMouseCursor();
    ImGuiImplDiligent::Render(pCtx);
}

bool ImGuiImplMacOS::HandleOSXEvent(NSEvent* _Nonnull event, NSView* _Nonnull view)
{
    std::lock_guard<std::mutex> Lock(m_Mtx);
    ImGuiIO& io = ImGui::GetIO();

    switch (event.type)
    {
        case NSEventTypeLeftMouseDown:
        case NSEventTypeRightMouseDown:
        case NSEventTypeOtherMouseDown:
        {
            const int Button = static_cast<int>(event.buttonNumber);
            if (Button >= 0 && Button < ImGuiMouseButton_COUNT)
                io.AddMouseButtonEvent(Button, true);
            return io.WantCaptureMouse;
        }

        case NSEventTypeLeftMouseUp:
        case NSEventTypeRightMouseUp:
        case NSEventTypeOtherMouseUp:
        {
            const int Button = static_cast<int>(event.buttonNumber);
            if (Button >= 0 && Button < ImGuiMouseButton_COUNT)
                io.AddMouseButtonEvent(Button, false);
            return io.WantCaptureMouse;
        }

        case NSEventTypeMouseMoved:
        case NSEventTypeLeftMouseDragged:
        case NSEventTypeRightMouseDragged:
        case NSEventTypeOtherMouseDragged:
        {
            const NSPoint Pos = [view convertPoint:event.locationInWindow fromView:nil];
            io.AddMousePosEvent(static_cast<float>(Pos.x),
                                static_cast<float>(view.bounds.size.height - Pos.y));
            return io.WantCaptureMouse;
        }

        case NSEventTypeScrollWheel:
        {
            double dx = event.scrollingDeltaX;
            double dy = event.scrollingDeltaY;
            if (event.hasPreciseScrollingDeltas)
            {
                dx *= 0.01;
                dy *= 0.01;
            }
            if (dx != 0.0 || dy != 0.0)
                io.AddMouseWheelEvent(static_cast<float>(dx), static_cast<float>(dy));
            return io.WantCaptureMouse;
        }

        case NSEventTypeKeyDown:
        case NSEventTypeKeyUp:
        {
            const bool Down = event.type == NSEventTypeKeyDown;
            NSString*  str  = event.characters;
            for (NSUInteger i = 0; i < str.length; ++i)
            {
                const int c = [str characterAtIndex:i];
                if (Down && !io.KeySuper && !(c >= 0xF700 && c <= 0xFFFF) && c != 127)
                    io.AddInputCharacter(static_cast<unsigned int>(c));

                const ImGuiKey Key = MapCharacterToImGuiKey(c);
                if (Key != ImGuiKey_None)
                {
                    io.AddKeyEvent(Key, Down);
                    if (Down && io.KeySuper)
                        m_KeysPressedWithCmd.push_back(Key);
                }
            }
            return io.WantCaptureKeyboard;
        }

        case NSEventTypeFlagsChanged:
        {
            const NSEventModifierFlags flags = event.modifierFlags;
            io.AddKeyEvent(ImGuiKey_ModCtrl, (flags & NSEventModifierFlagControl) != 0);
            io.AddKeyEvent(ImGuiKey_ModShift, (flags & NSEventModifierFlagShift) != 0);
            io.AddKeyEvent(ImGuiKey_ModAlt, (flags & NSEventModifierFlagOption) != 0);
            io.AddKeyEvent(ImGuiKey_ModSuper, (flags & NSEventModifierFlagCommand) != 0);
            return io.WantCaptureKeyboard;
        }

        default:
            return false;
    }
}

} // namespace Diligent
