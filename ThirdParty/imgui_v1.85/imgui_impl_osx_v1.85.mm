// dear imgui: Platform Backend for OSX / Cocoa
// This needs to be used along with a Renderer (e.g. OpenGL2, OpenGL3, Vulkan, Metal..)
// [ALPHA] Early backend, not well tested. If you want a portable application, prefer using the GLFW or SDL platform Backends on Mac.

// Implemented features:
//  [X] Platform: Mouse cursor shape and visibility. Disable with 'io.ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange'.
//  [X] Platform: OSX clipboard is supported within core Dear ImGui (no specific code in this backend).
// Issues:
//  [ ] Platform: Keys are all generally very broken. Best using [event keycode] and not [event characters]..

// You can use unmodified imgui_impl_* files in your project. See examples/ folder for examples of using this.
// Prefer including the entire imgui/ repository into your project (either as a copy or as a submodule), and only build the backends you need.
// If you are new to Dear ImGui, read documentation from the docs/ folder + read the top of imgui.cpp.
// Read online: https://github.com/ocornut/imgui/tree/master/docs

#include "imgui.h"
#include "imgui_impl_osx_v1.85.h"
#import <Cocoa/Cocoa.h>
#include <mach/mach_time.h>
#include <vector>

// CHANGELOG
// (minor and older changes stripped away, please see git history for details)
//  2021-09-21: Use mach_absolute_time as CFAbsoluteTimeGetCurrent can jump backwards.
//  2021-08-17: Calling io.AddFocusEvent() on NSApplicationDidBecomeActiveNotification/NSApplicationDidResignActiveNotification events.
//  2021-06-23: Inputs: Added a fix for shortcuts using CTRL key instead of CMD key.
//  2021-04-19: Inputs: Added a fix for keys remaining stuck in pressed state when CMD-tabbing into different application.
//  2021-01-27: Inputs: Added a fix for mouse position not being reported when mouse buttons other than left one are down.
//  2020-10-28: Inputs: Added a fix for handling keypad-enter key.
//  2020-05-25: Inputs: Added a fix for missing trackpad clicks when done with "soft tap".
//  2019-12-05: Inputs: Added support for ImGuiMouseCursor_NotAllowed mouse cursor.
//  2019-10-11: Inputs:  Fix using Backspace key.
//  2019-07-21: Re-added clipboard handlers as they are not enabled by default in core imgui.cpp (reverted 2019-05-18 change).
//  2019-05-28: Inputs: Added mouse cursor shape and visibility support.
//  2019-05-18: Misc: Removed clipboard handlers as they are now supported by core imgui.cpp.
//  2019-05-11: Inputs: Don't filter character values before calling AddInputCharacter() apart from 0xF700..0xFFFF range.
//  2018-11-30: Misc: Setting up io.BackendPlatformName so it can be displayed in the About Window.
//  2018-07-07: Initial version.

@class ImFocusObserver;

// Data
static double    g_Time = 0.0;
static NSCursor* g_MouseCursors[ImGuiMouseCursor_COUNT] = {};
static bool      g_MouseCursorHidden = false;
static bool      g_MouseJustPressed[ImGuiMouseButton_COUNT] = {};
static bool      g_MouseDown[ImGuiMouseButton_COUNT] = {};

static ImFocusObserver* g_FocusObserver = NULL;

static std::vector<ImGuiKey> g_KeysPressedWithCmd;

// Undocumented methods for creating cursors.
@interface NSCursor()
+ (id)_windowResizeNorthWestSouthEastCursor;
+ (id)_windowResizeNorthEastSouthWestCursor;
+ (id)_windowResizeNorthSouthCursor;
+ (id)_windowResizeEastWestCursor;
@end

static CFTimeInterval GetMachAbsoluteTimeInSeconds()
{
    return (CFTimeInterval)(double)(clock_gettime_nsec_np(CLOCK_UPTIME_RAW) / 1e9);;
}

static void resetKeys()
{
    ImGuiIO& io = ImGui::GetIO();
    io.ClearInputKeys();
    io.KeyCtrl = io.KeyShift = io.KeyAlt = io.KeySuper = false;
    g_KeysPressedWithCmd.clear();
}

@interface ImFocusObserver : NSObject

- (void)onApplicationBecomeActive:(NSNotification*)aNotification;
- (void)onApplicationBecomeInactive:(NSNotification*)aNotification;

@end

@implementation ImFocusObserver

- (void)onApplicationBecomeActive:(NSNotification*)aNotification
{
    ImGuiIO& io = ImGui::GetIO();
    io.AddFocusEvent(true);
}

- (void)onApplicationBecomeInactive:(NSNotification*)aNotification
{
    ImGuiIO& io = ImGui::GetIO();
    io.AddFocusEvent(false);

    // Unfocused applications do not receive input events, therefore we must manually
    // release any pressed keys when application loses focus, otherwise they would remain
    // stuck in a pressed state. https://github.com/ocornut/imgui/issues/3832
    resetKeys();
}

@end

// Functions
bool ImGui_ImplOSX_Init()
{
    ImGuiIO& io = ImGui::GetIO();

    // Setup backend capabilities flags
    io.BackendFlags |= ImGuiBackendFlags_HasMouseCursors;           // We can honor GetMouseCursor() values (optional)
    //io.BackendFlags |= ImGuiBackendFlags_HasSetMousePos;          // We can honor io.WantSetMousePos requests (optional, rarely used)
    //io.BackendFlags |= ImGuiBackendFlags_PlatformHasViewports;    // We can create multi-viewports on the Platform side (optional)
    //io.BackendFlags |= ImGuiBackendFlags_HasMouseHoveredViewport; // We can set io.MouseHoveredViewport correctly (optional, not easy)
    io.BackendPlatformName = "imgui_impl_osx";

    // Load cursors. Some of them are undocumented.
    g_MouseCursorHidden = false;
    g_MouseCursors[ImGuiMouseCursor_Arrow] = [NSCursor arrowCursor];
    g_MouseCursors[ImGuiMouseCursor_TextInput] = [NSCursor IBeamCursor];
    g_MouseCursors[ImGuiMouseCursor_ResizeAll] = [NSCursor closedHandCursor];
    g_MouseCursors[ImGuiMouseCursor_Hand] = [NSCursor pointingHandCursor];
    g_MouseCursors[ImGuiMouseCursor_NotAllowed] = [NSCursor operationNotAllowedCursor];
    g_MouseCursors[ImGuiMouseCursor_ResizeNS] = [NSCursor respondsToSelector:@selector(_windowResizeNorthSouthCursor)] ? [NSCursor _windowResizeNorthSouthCursor] : [NSCursor resizeUpDownCursor];
    g_MouseCursors[ImGuiMouseCursor_ResizeEW] = [NSCursor respondsToSelector:@selector(_windowResizeEastWestCursor)] ? [NSCursor _windowResizeEastWestCursor] : [NSCursor resizeLeftRightCursor];
    g_MouseCursors[ImGuiMouseCursor_ResizeNESW] = [NSCursor respondsToSelector:@selector(_windowResizeNorthEastSouthWestCursor)] ? [NSCursor _windowResizeNorthEastSouthWestCursor] : [NSCursor closedHandCursor];
    g_MouseCursors[ImGuiMouseCursor_ResizeNWSE] = [NSCursor respondsToSelector:@selector(_windowResizeNorthWestSouthEastCursor)] ? [NSCursor _windowResizeNorthWestSouthEastCursor] : [NSCursor closedHandCursor];

    // Note that imgui.cpp also include default OSX clipboard handlers which can be enabled
    // by adding '#define IMGUI_ENABLE_OSX_DEFAULT_CLIPBOARD_FUNCTIONS' in imconfig.h and adding '-framework ApplicationServices' to your linker command-line.
    // Since we are already in ObjC land here, it is easy for us to add a clipboard handler using the NSPasteboard api.
    io.SetClipboardTextFn = [](void*, const char* str) -> void
    {
        NSPasteboard* pasteboard = [NSPasteboard generalPasteboard];
        [pasteboard declareTypes:[NSArray arrayWithObject:NSPasteboardTypeString] owner:nil];
        [pasteboard setString:[NSString stringWithUTF8String:str] forType:NSPasteboardTypeString];
    };

    io.GetClipboardTextFn = [](void*) -> const char*
    {
        NSPasteboard* pasteboard = [NSPasteboard generalPasteboard];
        NSString* available = [pasteboard availableTypeFromArray: [NSArray arrayWithObject:NSPasteboardTypeString]];
        if (![available isEqualToString:NSPasteboardTypeString])
            return NULL;

        NSString* string = [pasteboard stringForType:NSPasteboardTypeString];
        if (string == nil)
            return NULL;

        const char* string_c = (const char*)[string UTF8String];
        size_t string_len = strlen(string_c);
        static ImVector<char> s_clipboard;
        s_clipboard.resize((int)string_len + 1);
        strcpy(s_clipboard.Data, string_c);
        return s_clipboard.Data;
    };

    g_FocusObserver = [[ImFocusObserver alloc] init];
    [[NSNotificationCenter defaultCenter] addObserver:g_FocusObserver
                                             selector:@selector(onApplicationBecomeActive:)
                                                 name:NSApplicationDidBecomeActiveNotification
                                               object:nil];
    [[NSNotificationCenter defaultCenter] addObserver:g_FocusObserver
                                             selector:@selector(onApplicationBecomeInactive:)
                                                 name:NSApplicationDidResignActiveNotification
                                               object:nil];

    return true;
}

void ImGui_ImplOSX_Shutdown()
{
    g_FocusObserver = NULL;
}

static void ImGui_ImplOSX_UpdateMouseCursorAndButtons()
{
    // Update buttons
    ImGuiIO& io = ImGui::GetIO();
    for (int i = 0; i < IM_ARRAYSIZE(io.MouseDown); i++)
    {
        // If a mouse press event came, always pass it as "mouse held this frame", so we don't miss click-release events that are shorter than 1 frame.
        io.MouseDown[i] = g_MouseJustPressed[i] || g_MouseDown[i];
        g_MouseJustPressed[i] = false;
    }

    if (io.ConfigFlags & ImGuiConfigFlags_NoMouseCursorChange)
        return;

    ImGuiMouseCursor imgui_cursor = ImGui::GetMouseCursor();
    if (io.MouseDrawCursor || imgui_cursor == ImGuiMouseCursor_None)
    {
        // Hide OS mouse cursor if imgui is drawing it or if it wants no cursor
        if (!g_MouseCursorHidden)
        {
            g_MouseCursorHidden = true;
            [NSCursor hide];
        }
    }
    else
    {
        // Show OS mouse cursor
        [g_MouseCursors[g_MouseCursors[imgui_cursor] ? imgui_cursor : ImGuiMouseCursor_Arrow] set];
        if (g_MouseCursorHidden)
        {
            g_MouseCursorHidden = false;
            [NSCursor unhide];
        }
    }
}

void ImGui_ImplOSX_NewFrame(NSView* view)
{
    // Setup display size
    ImGuiIO& io = ImGui::GetIO();
    if (view)
    {
        const float dpi = (float)[view.window backingScaleFactor];
        io.DisplaySize = ImVec2((float)view.bounds.size.width, (float)view.bounds.size.height);
        io.DisplayFramebufferScale = ImVec2(dpi, dpi);
    }

    // Setup time step
    if (g_Time == 0.0)
    {
        g_Time = GetMachAbsoluteTimeInSeconds();
    }
    double current_time = GetMachAbsoluteTimeInSeconds();
    io.DeltaTime = (float)(current_time - g_Time);
    g_Time = current_time;

    ImGui_ImplOSX_UpdateMouseCursorAndButtons();
    
    // Generate KeyUp event for all keys pressed while Cmd was pressed.
    for (ImGuiKey key : g_KeysPressedWithCmd)
    {
        io.AddKeyEvent(key, false);
    }
    g_KeysPressedWithCmd.clear();
}

static ImGuiKey mapCharacterToKey(int c)
{
    if (c >= 'a' && c <= 'z')
        return (ImGuiKey)(ImGuiKey_A + (c - 'a'));
    if (c >= 'A' && c <= 'Z')
        return (ImGuiKey)(ImGuiKey_A + (c - 'A'));
    if (c >= '0' && c <= '9')
        return (ImGuiKey)(ImGuiKey_0 + (c - '0'));

    switch (c)
    {
        case 9:    return ImGuiKey_Tab;
        case 13:   return ImGuiKey_Enter;
        case 27:   return ImGuiKey_Escape;
        case 127:  return ImGuiKey_Backspace;
        case 25:   return ImGuiKey_Tab; // Shift+Tab workaround
    }

    // macOS NSEvent function keys (0xF700+)
    switch (c)
    {
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

bool ImGui_ImplOSX_HandleEvent(NSEvent* event, NSView* view)
{
    ImGuiIO& io = ImGui::GetIO();

    if (event.type == NSEventTypeLeftMouseDown || event.type == NSEventTypeRightMouseDown || event.type == NSEventTypeOtherMouseDown)
    {
        int button = (int)[event buttonNumber];
        if (button >= 0 && button < IM_ARRAYSIZE(g_MouseDown))
            g_MouseDown[button] = g_MouseJustPressed[button] = true;
        return io.WantCaptureMouse;
    }

    if (event.type == NSEventTypeLeftMouseUp || event.type == NSEventTypeRightMouseUp || event.type == NSEventTypeOtherMouseUp)
    {
        int button = (int)[event buttonNumber];
        if (button >= 0 && button < IM_ARRAYSIZE(g_MouseDown))
            g_MouseDown[button] = false;
        return io.WantCaptureMouse;
    }

    if (event.type == NSEventTypeMouseMoved || event.type == NSEventTypeLeftMouseDragged || event.type == NSEventTypeRightMouseDragged || event.type == NSEventTypeOtherMouseDragged)
    {
        NSPoint mousePoint = event.locationInWindow;
        mousePoint = [view convertPoint:mousePoint fromView:nil];
        mousePoint = NSMakePoint(mousePoint.x, view.bounds.size.height - mousePoint.y);
        io.MousePos = ImVec2((float)mousePoint.x, (float)mousePoint.y);
    }

    if (event.type == NSEventTypeScrollWheel)
    {
        double wheel_dx = 0.0;
        double wheel_dy = 0.0;

        #if MAC_OS_X_VERSION_MAX_ALLOWED >= 1070
        if (floor(NSAppKitVersionNumber) > NSAppKitVersionNumber10_6)
        {
            wheel_dx = [event scrollingDeltaX];
            wheel_dy = [event scrollingDeltaY];
            if ([event hasPreciseScrollingDeltas])
            {
                wheel_dx *= 0.1;
                wheel_dy *= 0.1;
            }
        }
        else
        #endif // MAC_OS_X_VERSION_MAX_ALLOWED
        {
            wheel_dx = [event deltaX];
            wheel_dy = [event deltaY];
        }

        if (fabs(wheel_dx) > 0.0)
            io.MouseWheelH += (float)wheel_dx * 0.1f;
        if (fabs(wheel_dy) > 0.0)
            io.MouseWheel += (float)wheel_dy * 0.1f;
        return io.WantCaptureMouse;
    }

    // FIXME: All the key handling is wrong and broken. Refer to GLFW's cocoa_init.mm and cocoa_window.mm.
    if (event.type == NSEventTypeKeyDown)
    {
        NSString* str = [event characters];
        NSUInteger len = [str length];
        for (NSUInteger i = 0; i < len; i++)
        {
            int c = [str characterAtIndex:i];
            if (!io.KeySuper && !(c >= 0xF700 && c <= 0xFFFF) && c != 127)
                io.AddInputCharacter((unsigned int)c);

            ImGuiKey key = mapCharacterToKey(c);
            if (key != ImGuiKey_None)
            {
                io.AddKeyEvent(key, true);
                
                // NB: AddKeyEvent swaps Ctrl and Cmd (aka Super) on Mac
                if (io.KeyCtrl)
                {
                    // MacOS does not generate KeyUp event when Cmd is pressed.
                    g_KeysPressedWithCmd.push_back(key);
                }
            }
        }
        
        return io.WantCaptureKeyboard;
    }

    if (event.type == NSEventTypeKeyUp)
    {
        NSString* str = [event characters];
        NSUInteger len = [str length];
        for (NSUInteger i = 0; i < len; i++)
        {
            int c = [str characterAtIndex:i];
            ImGuiKey key = mapCharacterToKey(c);
            if (key != ImGuiKey_None)
            {
                io.AddKeyEvent(key, false);
            }
        }
        return io.WantCaptureKeyboard;
    }

    if (event.type == NSEventTypeFlagsChanged)
    {
        unsigned int flags = [event modifierFlags] & NSEventModifierFlagDeviceIndependentFlagsMask;

        bool oldKeyCtrl  = io.KeyCtrl;
        bool oldKeyShift = io.KeyShift;
        bool oldKeyAlt   = io.KeyAlt;
        bool oldKeySuper = io.KeySuper;
        
        bool KeyCtrl  = flags & NSEventModifierFlagControl;
        bool KeyShift = flags & NSEventModifierFlagShift;
        bool KeyAlt   = flags & NSEventModifierFlagOption;
        bool KeySuper = flags & NSEventModifierFlagCommand;

        // NB: AddKeyEvent swaps Ctrl and Cmd (aka Super) on Mac
        io.AddKeyEvent(ImGuiKey_ModCtrl, KeyCtrl);
        io.AddKeyEvent(ImGuiKey_ModShift, KeyShift);
        io.AddKeyEvent(ImGuiKey_ModAlt, KeyAlt);
        io.AddKeyEvent(ImGuiKey_ModSuper, KeySuper);
        
        // We must reset keys as we will not receive any keyUp event if they where pressed with a modifier
        if ((oldKeyShift && !io.KeyShift) || (oldKeyCtrl && !io.KeyCtrl) || (oldKeyAlt && !io.KeyAlt) || (oldKeySuper && !io.KeySuper))
            resetKeys();
        
        return io.WantCaptureKeyboard;
    }

    return false;
}
