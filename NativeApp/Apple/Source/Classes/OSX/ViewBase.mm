/*     Copyright 2025 Diligent Graphics LLC
 *     Copyright 2015-2019 Egor Yusov
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

#include <memory>
#include <string>
#include <vector>

#import "ViewBase.h"

@implementation ViewBase
{
    std::unique_ptr<Diligent::NativeAppBase> _theApp;
    std::string _error;
    NSRecursiveLock* appLock;
}

@synthesize displayLink;

// Prepares the receiver for service after it has been loaded
// from an Interface Builder archive, or nib file.
- (void) awakeFromNib
{
    [super awakeFromNib];

    std::vector<const char*> Args;
    std::vector<std::string> ArgStr;
    @autoreleasepool
    {
        NSArray<NSString*>* arguments = [[NSProcessInfo processInfo] arguments]; // Autoreleased
        const auto ArgCount = arguments.count;
        Args.resize(ArgCount);
        ArgStr.resize(ArgCount);
        for(size_t i = 0; i < ArgCount; ++i)
        {
            ArgStr[i] = [arguments[i] cStringUsingEncoding:NSUTF8StringEncoding]; // Autoreleased
            Args[i]   = ArgStr[i].c_str();
        }
   }

    _theApp.reset(Diligent::CreateApplication());
    _theApp->ProcessCommandLine(static_cast<int>(Args.size()), Args.data());

    // [self window] is nil here
    auto* mainWindow = [[NSApplication sharedApplication] mainWindow];
    // Register to be notified when the main window closes so we can stop the displaylink
    [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(windowWillClose:)
                                                 name:NSWindowWillCloseNotification
                                               object:mainWindow];
    mainWindow.minSize = NSSize{320, 240};
}

-(void)initApp:(NSView*) view
{
    // Init the application.
    try
    {
        _theApp->Initialize((__bridge void*)view, self.renderMode);
    }
    catch(std::runtime_error &err)
    {
        _error = err.what();
        _theApp.reset();
    }
}

-(Diligent::NativeAppBase*)lockApp
{
    [appLock lock];
    return _theApp.get();
}

-(void)unlockApp
{
    [appLock unlock];
}

- (BOOL)acceptsFirstResponder
{
    return YES; // To make keyboard events work
}

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"

-(void)destroyApp
{
    // Stop the display link BEFORE releasing anything in the view
    // otherwise the display link thread may call into the view and crash
    // when it encounters something that has been released
    if (displayLink)
    {
        CVDisplayLinkStop(displayLink);
    }

    [appLock lock];
    _theApp.reset();
    [appLock unlock];
}

-(void) dealloc
{
    [self destroyApp];

    CVDisplayLinkRelease(displayLink);

    [appLock release];

    [super dealloc];
}

-(NSString*)getError
{
    return _error.empty() ? nil : [NSString stringWithFormat:@"%s", _error.c_str()];
}


- (void)stopDisplayLink
{
    if (displayLink)
    {
        CVDisplayLinkStop(displayLink);
    }
}

- (void)startDisplayLink
{
    if (displayLink)
    {
        CVDisplayLinkStart(displayLink);
    }
}

#pragma clang diagnostic pop

- (void) windowWillClose:(NSNotification*)notification
{
    [self destroyApp];
}

-(NSString*)getAppName
{
    auto* theApp = [self lockApp];
    auto Title = [NSString stringWithFormat:@"%s", theApp ? theApp->GetAppTitle() : ""];
    [self unlockApp];
    return Title;
}

@end
