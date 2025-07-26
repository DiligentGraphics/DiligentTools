// dear imgui: Platform Backend for macOS / Cocoa  (legacy threading-friendly)
// -------------------------------------------------------------------------
//  - Based on the 1.85 backend, updated to Dear ImGui >= 1.92 new-IO API
//  - Keeps ImGui_ImplOSX_HandleEvent()
// -------------------------------------------------------------------------

#include "imgui_impl_osx_v1.85.h"
#import "imgui.h"
#import "imgui_internal.h"
#import <Cocoa/Cocoa.h>
#import <Carbon/Carbon.h>
#include <mach/mach_time.h>

// ---- Undocumented cursor selectors ----------------------------------------
@interface NSCursor ()
+ (NSCursor*)_windowResizeNorthSouthCursor;
+ (NSCursor*)_windowResizeEastWestCursor;
+ (NSCursor*)_windowResizeNorthEastSouthWestCursor;
+ (NSCursor*)_windowResizeNorthWestSouthEastCursor;
@end

// ---------------------------------------------------------------------------
// Data & helpers
// ---------------------------------------------------------------------------
static double    g_Time = 0.0;
static NSCursor* g_MouseCursors[ImGuiMouseCursor_COUNT] = {};
static bool      g_MouseCursorHidden = false;
static bool      g_MouseDown[ImGuiMouseButton_COUNT] = {};   // only for cursor

static inline double AbsoluteSeconds()
{ return clock_gettime_nsec_np(CLOCK_UPTIME_RAW) / 1e9; }

static ImGuiKey KeycodeToImGuiKey(unsigned key_code)
{
    switch (key_code)
    {
        case kVK_ANSI_A: return ImGuiKey_A;
        case kVK_ANSI_S: return ImGuiKey_S;
        case kVK_ANSI_D: return ImGuiKey_D;
        case kVK_ANSI_F: return ImGuiKey_F;
        case kVK_ANSI_H: return ImGuiKey_H;
        case kVK_ANSI_G: return ImGuiKey_G;
        case kVK_ANSI_Z: return ImGuiKey_Z;
        case kVK_ANSI_X: return ImGuiKey_X;
        case kVK_ANSI_C: return ImGuiKey_C;
        case kVK_ANSI_V: return ImGuiKey_V;
        case kVK_ANSI_B: return ImGuiKey_B;
        case kVK_ANSI_Q: return ImGuiKey_Q;
        case kVK_ANSI_W: return ImGuiKey_W;
        case kVK_ANSI_E: return ImGuiKey_E;
        case kVK_ANSI_R: return ImGuiKey_R;
        case kVK_ANSI_Y: return ImGuiKey_Y;
        case kVK_ANSI_T: return ImGuiKey_T;
        case kVK_ANSI_1: return ImGuiKey_1;
        case kVK_ANSI_2: return ImGuiKey_2;
        case kVK_ANSI_3: return ImGuiKey_3;
        case kVK_ANSI_4: return ImGuiKey_4;
        case kVK_ANSI_6: return ImGuiKey_6;
        case kVK_ANSI_5: return ImGuiKey_5;
        case kVK_ANSI_Equal: return ImGuiKey_Equal;
        case kVK_ANSI_9: return ImGuiKey_9;
        case kVK_ANSI_7: return ImGuiKey_7;
        case kVK_ANSI_Minus: return ImGuiKey_Minus;
        case kVK_ANSI_8: return ImGuiKey_8;
        case kVK_ANSI_0: return ImGuiKey_0;
        case kVK_ANSI_RightBracket: return ImGuiKey_RightBracket;
        case kVK_ANSI_O: return ImGuiKey_O;
        case kVK_ANSI_U: return ImGuiKey_U;
        case kVK_ANSI_LeftBracket: return ImGuiKey_LeftBracket;
        case kVK_ANSI_I: return ImGuiKey_I;
        case kVK_ANSI_P: return ImGuiKey_P;
        case kVK_ANSI_L: return ImGuiKey_L;
        case kVK_ANSI_J: return ImGuiKey_J;
        case kVK_ANSI_Quote: return ImGuiKey_Apostrophe;
        case kVK_ANSI_K: return ImGuiKey_K;
        case kVK_ANSI_Semicolon: return ImGuiKey_Semicolon;
        case kVK_ANSI_Backslash: return ImGuiKey_Backslash;
        case kVK_ANSI_Comma: return ImGuiKey_Comma;
        case kVK_ANSI_Slash: return ImGuiKey_Slash;
        case kVK_ANSI_N: return ImGuiKey_N;
        case kVK_ANSI_M: return ImGuiKey_M;
        case kVK_ANSI_Period: return ImGuiKey_Period;
        case kVK_ANSI_Grave: return ImGuiKey_GraveAccent;
        case kVK_ANSI_KeypadDecimal: return ImGuiKey_KeypadDecimal;
        case kVK_ANSI_KeypadMultiply: return ImGuiKey_KeypadMultiply;
        case kVK_ANSI_KeypadPlus: return ImGuiKey_KeypadAdd;
        case kVK_ANSI_KeypadClear: return ImGuiKey_NumLock;
        case kVK_ANSI_KeypadDivide: return ImGuiKey_KeypadDivide;
        case kVK_ANSI_KeypadEnter: return ImGuiKey_KeypadEnter;
        case kVK_ANSI_KeypadMinus: return ImGuiKey_KeypadSubtract;
        case kVK_ANSI_KeypadEquals: return ImGuiKey_KeypadEqual;
        case kVK_ANSI_Keypad0: return ImGuiKey_Keypad0;
        case kVK_ANSI_Keypad1: return ImGuiKey_Keypad1;
        case kVK_ANSI_Keypad2: return ImGuiKey_Keypad2;
        case kVK_ANSI_Keypad3: return ImGuiKey_Keypad3;
        case kVK_ANSI_Keypad4: return ImGuiKey_Keypad4;
        case kVK_ANSI_Keypad5: return ImGuiKey_Keypad5;
        case kVK_ANSI_Keypad6: return ImGuiKey_Keypad6;
        case kVK_ANSI_Keypad7: return ImGuiKey_Keypad7;
        case kVK_ANSI_Keypad8: return ImGuiKey_Keypad8;
        case kVK_ANSI_Keypad9: return ImGuiKey_Keypad9;
        case kVK_Return: return ImGuiKey_Enter;
        case kVK_Tab: return ImGuiKey_Tab;
        case kVK_Space: return ImGuiKey_Space;
        case kVK_Delete: return ImGuiKey_Backspace;
        case kVK_Escape: return ImGuiKey_Escape;
        case kVK_CapsLock: return ImGuiKey_CapsLock;
        case kVK_Control: return ImGuiKey_LeftCtrl;
        case kVK_Shift: return ImGuiKey_LeftShift;
        case kVK_Option: return ImGuiKey_LeftAlt;
        case kVK_Command: return ImGuiKey_LeftSuper;
        case kVK_RightControl: return ImGuiKey_RightCtrl;
        case kVK_RightShift: return ImGuiKey_RightShift;
        case kVK_RightOption: return ImGuiKey_RightAlt;
        case kVK_RightCommand: return ImGuiKey_RightSuper;
//      case kVK_Function: return ImGuiKey_;
//      case kVK_VolumeUp: return ImGuiKey_;
//      case kVK_VolumeDown: return ImGuiKey_;
//      case kVK_Mute: return ImGuiKey_;
        case kVK_F1: return ImGuiKey_F1;
        case kVK_F2: return ImGuiKey_F2;
        case kVK_F3: return ImGuiKey_F3;
        case kVK_F4: return ImGuiKey_F4;
        case kVK_F5: return ImGuiKey_F5;
        case kVK_F6: return ImGuiKey_F6;
        case kVK_F7: return ImGuiKey_F7;
        case kVK_F8: return ImGuiKey_F8;
        case kVK_F9: return ImGuiKey_F9;
        case kVK_F10: return ImGuiKey_F10;
        case kVK_F11: return ImGuiKey_F11;
        case kVK_F12: return ImGuiKey_F12;
        case kVK_F13: return ImGuiKey_F13;
        case kVK_F14: return ImGuiKey_F14;
        case kVK_F15: return ImGuiKey_F15;
        case kVK_F16: return ImGuiKey_F16;
        case kVK_F17: return ImGuiKey_F17;
        case kVK_F18: return ImGuiKey_F18;
        case kVK_F19: return ImGuiKey_F19;
        case kVK_F20: return ImGuiKey_F20;
        case 0x6E: return ImGuiKey_Menu;
        case kVK_Help: return ImGuiKey_Insert;
        case kVK_Home: return ImGuiKey_Home;
        case kVK_PageUp: return ImGuiKey_PageUp;
        case kVK_ForwardDelete: return ImGuiKey_Delete;
        case kVK_End: return ImGuiKey_End;
        case kVK_PageDown: return ImGuiKey_PageDown;
        case kVK_LeftArrow: return ImGuiKey_LeftArrow;
        case kVK_RightArrow: return ImGuiKey_RightArrow;
        case kVK_DownArrow: return ImGuiKey_DownArrow;
        case kVK_UpArrow: return ImGuiKey_UpArrow;
        default: return ImGuiKey_None;
    }
}

static void ResetAllKeys()
{
    ImGuiIO& io = ImGui::GetIO();

    // Only release genuine (non-alias) named keys
    for (int k = (int)ImGuiKey_NamedKey_BEGIN; k < (int)ImGuiKey_COUNT; ++k)
    {
        ImGuiKey key = (ImGuiKey)k;
        if (!ImGui::IsAliasKey(key))          // filter out MouseXXX, etc.
            io.AddKeyEvent(key, false);
    }

    // Explicitly release modifiers
    io.AddKeyEvent(ImGuiMod_Ctrl,  false);
    io.AddKeyEvent(ImGuiMod_Shift, false);
    io.AddKeyEvent(ImGuiMod_Alt,   false);
    io.AddKeyEvent(ImGuiMod_Super, false);
}

// ---------------------------------------------------------------------------
// Focus observer – release stuck keys on deactivate
// ---------------------------------------------------------------------------
@interface ImFocusObserver : NSObject
- (void)appDidBecomeActive:(NSNotification*)n;
- (void)appDidResignActive:(NSNotification*)n;
@end
@implementation ImFocusObserver
- (void)appDidBecomeActive:(NSNotification*)n { ImGui::GetIO().AddFocusEvent(true);  }
- (void)appDidResignActive:(NSNotification*)n { ImGui::GetIO().AddFocusEvent(false); ResetAllKeys(); }
@end
static ImFocusObserver* g_FocusObserver = nil;

// ---------------------------------------------------------------------------
// Backend API
// ---------------------------------------------------------------------------
bool ImGui_ImplOSX_Init()
{
    ImGuiIO& io = ImGui::GetIO();
    io.BackendPlatformName = "imgui_impl_osx (1.85 compat, modern IO)";
    io.BackendFlags |= ImGuiBackendFlags_HasMouseCursors;

    // Cursors ----------------------------------------------------------------
    g_MouseCursors[ImGuiMouseCursor_Arrow]      = [NSCursor arrowCursor];
    g_MouseCursors[ImGuiMouseCursor_TextInput]  = [NSCursor IBeamCursor];
    g_MouseCursors[ImGuiMouseCursor_ResizeAll]  = [NSCursor closedHandCursor];
    g_MouseCursors[ImGuiMouseCursor_Hand]       = [NSCursor pointingHandCursor];
    g_MouseCursors[ImGuiMouseCursor_NotAllowed] = [NSCursor operationNotAllowedCursor];
    g_MouseCursors[ImGuiMouseCursor_ResizeNS]   = [NSCursor respondsToSelector:@selector(_windowResizeNorthSouthCursor)]
                                                  ? [NSCursor _windowResizeNorthSouthCursor]
                                                  : [NSCursor resizeUpDownCursor];
    g_MouseCursors[ImGuiMouseCursor_ResizeEW]   = [NSCursor respondsToSelector:@selector(_windowResizeEastWestCursor)]
                                                  ? [NSCursor _windowResizeEastWestCursor]
                                                  : [NSCursor resizeLeftRightCursor];
    g_MouseCursors[ImGuiMouseCursor_ResizeNESW] = [NSCursor respondsToSelector:@selector(_windowResizeNorthEastSouthWestCursor)]
                                                  ? [NSCursor _windowResizeNorthEastSouthWestCursor]
                                                  : [NSCursor closedHandCursor];
    g_MouseCursors[ImGuiMouseCursor_ResizeNWSE] = [NSCursor respondsToSelector:@selector(_windowResizeNorthWestSouthEastCursor)]
                                                  ? [NSCursor _windowResizeNorthWestSouthEastCursor]
                                                  : [NSCursor closedHandCursor];

    // Clipboard --------------------------------------------------------------
    io.SetClipboardTextFn = [](void*, const char* s)
    {
        NSPasteboard* pb = [NSPasteboard generalPasteboard];
        [pb declareTypes:[NSArray arrayWithObject:NSPasteboardTypeString] owner:nil];
        [pb setString:[NSString stringWithUTF8String:s] forType:NSPasteboardTypeString];
    };
    io.GetClipboardTextFn = [](void*) -> const char*
    {
        NSPasteboard* pb = [NSPasteboard generalPasteboard];
        NSString* typ = [pb availableTypeFromArray:[NSArray arrayWithObject:NSPasteboardTypeString]];
        if (![typ isEqualToString:NSPasteboardTypeString]) return nullptr;
        NSString* str = [pb stringForType:NSPasteboardTypeString]; if (!str) return nullptr;
        static ImVector<char> buf; const char* c = str.UTF8String; buf.resize((int)strlen(c)+1); strcpy(buf.Data,c); return buf.Data;
    };

    // Focus notifications ----------------------------------------------------
    g_FocusObserver = [[ImFocusObserver alloc] init];
    NSNotificationCenter* nc = [NSNotificationCenter defaultCenter];
    [nc addObserver:g_FocusObserver selector:@selector(appDidBecomeActive:) name:NSApplicationDidBecomeActiveNotification  object:nil];
    [nc addObserver:g_FocusObserver selector:@selector(appDidResignActive:) name:NSApplicationDidResignActiveNotification object:nil];
    return true;
}

void ImGui_ImplOSX_Shutdown() { g_FocusObserver = nil; }

static void UpdateMouseCursor()
{
    ImGuiIO& io = ImGui::GetIO();
    if (io.ConfigFlags & ImGuiConfigFlags_NoMouseCursorChange)
        return;

    ImGuiMouseCursor cur = ImGui::GetMouseCursor();
    if (io.MouseDrawCursor || cur == ImGuiMouseCursor_None)
    {
        if (!g_MouseCursorHidden) { g_MouseCursorHidden = true; [NSCursor hide]; }
    }
    else
    {
        NSCursor* c = g_MouseCursors[cur] ? g_MouseCursors[cur]
                                          : g_MouseCursors[ImGuiMouseCursor_Arrow];
        [c set];                               // <— standard C++14, no “?:” extension
        if (g_MouseCursorHidden) { g_MouseCursorHidden = false; [NSCursor unhide]; }
    }
}

void ImGui_ImplOSX_NewFrame(NSView* view)
{
    ImGuiIO& io = ImGui::GetIO();

    if (view)
    {
        float dpi = view.window.backingScaleFactor;
        io.DisplaySize            = ImVec2(view.bounds.size.width,  view.bounds.size.height);
        io.DisplayFramebufferScale= ImVec2(dpi, dpi);
    }

    double now = AbsoluteSeconds();
    io.DeltaTime = (g_Time > 0.0) ? (float)(now - g_Time) : 1.0f/60.0f;
    g_Time = now;

    UpdateMouseCursor();
}

// ---------------------------------------------------------------------------
// Event translation
// ---------------------------------------------------------------------------
bool ImGui_ImplOSX_HandleEvent(NSEvent* e, NSView* view)
{
    ImGuiIO& io = ImGui::GetIO();

    auto PushMods = [&](NSEventModifierFlags f)
    {
        io.AddKeyEvent(ImGuiMod_Ctrl,  (f & NSEventModifierFlagControl)  != 0);
        io.AddKeyEvent(ImGuiMod_Shift, (f & NSEventModifierFlagShift)    != 0);
        io.AddKeyEvent(ImGuiMod_Alt,   (f & NSEventModifierFlagOption)   != 0);
        io.AddKeyEvent(ImGuiMod_Super, (f & NSEventModifierFlagCommand)  != 0);
    };
    auto PushMousePos = [&](NSPoint p)
    {
        if (!view) return;
        p = [view convertPoint:p fromView:nil];
        p.y = view.bounds.size.height - p.y;
        io.AddMousePosEvent((float)p.x, (float)p.y);
    };
    auto PushMouseBtn = [&](int b,bool d)
    {
        io.AddMouseButtonEvent(b,d); if (b>=0&&b<IM_ARRAYSIZE(g_MouseDown)) g_MouseDown[b]=d;
    };

    switch (e.type)
    {
        case NSEventTypeLeftMouseDown:
        case NSEventTypeRightMouseDown:
        case NSEventTypeOtherMouseDown:  PushMouseBtn((int)e.buttonNumber,true);  return io.WantCaptureMouse;
        case NSEventTypeLeftMouseUp:
        case NSEventTypeRightMouseUp:
        case NSEventTypeOtherMouseUp:    PushMouseBtn((int)e.buttonNumber,false); return io.WantCaptureMouse;

        case NSEventTypeMouseMoved:
        case NSEventTypeLeftMouseDragged:
        case NSEventTypeRightMouseDragged:
        case NSEventTypeOtherMouseDragged: PushMousePos(e.locationInWindow); break;

        case NSEventTypeScrollWheel:
        {
            double dx=0,dy=0;
#if MAC_OS_X_VERSION_MAX_ALLOWED >= 1070
            if (floor(NSAppKitVersionNumber) > NSAppKitVersionNumber10_6)
            { dx=e.scrollingDeltaX; dy=e.scrollingDeltaY; if (e.hasPreciseScrollingDeltas){dx*=0.1; dy*=0.1;} }
            else
#endif
            { dx=e.deltaX; dy=e.deltaY; }
            if (fabs(dx)||fabs(dy)) io.AddMouseWheelEvent((float)dx*0.1f,(float)dy*0.1f);
            return io.WantCaptureMouse;
        }

        case NSEventTypeKeyDown:
            PushMods(e.modifierFlags);
            if (ImGuiKey k = KeycodeToImGuiKey((unsigned)e.keyCode)) io.AddKeyEvent(k,true);
            for (NSUInteger i = 0; i < e.charactersIgnoringModifiers.length; ++i)
            {
                unsigned short ch = [e.charactersIgnoringModifiers characterAtIndex:i];
                if (ch != 0x7F && (ch < 0xF700 || ch > 0xFFFF))   // skip DEL + function-key range
                    io.AddInputCharacter(ch);
            }
            return io.WantCaptureKeyboard;

        case NSEventTypeKeyUp:
            PushMods(e.modifierFlags);
            if (ImGuiKey k = KeycodeToImGuiKey((unsigned)e.keyCode)) io.AddKeyEvent(k,false);
            return io.WantCaptureKeyboard;

        case NSEventTypeFlagsChanged: PushMods(e.modifierFlags); return io.WantCaptureKeyboard;
        default: break;
    }
    return false;
}
