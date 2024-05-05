/*
 Copyright (C) 2015 Apple Inc. All Rights Reserved.
 See LICENSE.txt for this sample’s licensing information
 
 Abstract:
 Standard AppKit entry point.
 */

#if PLATFORM_IOS || PLATFORM_TVOS
#import "AppDelegate.h"
#else // OS X
#import <Cocoa/Cocoa.h>
#endif

int main(int argc, char * argv[]) {

#if PLATFORM_IOS || PLATFORM_TVOS
    @autoreleasepool {
        return UIApplicationMain(argc, argv, nil, NSStringFromClass([AppDelegate class]));
    }
#else
    return NSApplicationMain(argc, (const char**)argv);
#endif
}
