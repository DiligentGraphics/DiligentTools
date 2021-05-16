#import "MVKView.h"

#import <QuartzCore/CAMetalLayer.h>

#include "GraphicsTypes.h"

@implementation MVKView

+ (Class) layerClass
{
    return [CAMetalLayer class];
}

- (instancetype) initWithCoder:(NSCoder*)coder
{
    if ((self = [super initWithCoder:coder]))
    {
        [self initApp:(int)Diligent::RENDER_DEVICE_TYPE_VULKAN];
    }

    return self;
}

- (void) drawView:(id)sender
{
    // There is no autorelease pool when this method is called
    // because it will be called from a background thread.
    // It's important to create one or app can leak objects.
    @autoreleasepool
    {
        [self render];
    }
}

- (void) dealloc
{
    [self terminate];
    [super dealloc];
}

@end
