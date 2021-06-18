/*
See LICENSE folder for this sample’s licensing information.
*/

#import "MetalView.h"

@implementation MetalView

- (instancetype) initWithFrame:(CGRect)frame
{
    self = [super initWithFrame:frame];
    if(self)
    {
        [self initCommon];
    }
    return self;
}

- (instancetype) initWithCoder:(NSCoder *)aDecoder
{
    self = [super initWithCoder:aDecoder];
    if(self)
    {
        [self initCommon];
    }
    return self;
}

- (void)initCommon
{
    _metalLayer = (CAMetalLayer*) self.layer;

    self.layer.delegate = self;
}


- (void)stopRenderLoop
{
    // Stubbed out method.  Subclasses need to implement this method.
}

- (void)dealloc
{
    [self stopRenderLoop];
    [super dealloc];
}


- (void)resizeDrawable:(CGFloat)scaleFactor
{
    CGSize newSize = self.bounds.size;
    newSize.width *= scaleFactor;
    newSize.height *= scaleFactor;

    if(newSize.width <= 0 || newSize.width <= 0)
    {
        return;
    }

    if(newSize.width == _metalLayer.drawableSize.width &&
       newSize.height == _metalLayer.drawableSize.height)
    {
        return;
    }

    _metalLayer.drawableSize = newSize;

    [_delegate drawableResize:newSize];
}


- (void)render
{
    [_delegate renderToMetalLayer:_metalLayer];
}

@end
