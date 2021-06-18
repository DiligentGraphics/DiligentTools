/*
See LICENSE folder for this sample’s licensing information.
*/

#import "MainUIView.h"

@implementation MainUIView
{
    CADisplayLink *_displayLink;
}

+ (Class) layerClass
{
    return [CAMetalLayer class];
}

- (void)didMoveToWindow
{
    [super didMoveToWindow];

    if(self.window == nil)
    {
        // If moving off of a window destroy the display link.
        [_displayLink invalidate];
        _displayLink = nil;
        return;
    }

    [self setupCADisplayLinkForScreen:self.window.screen];

    // CADisplayLink callbacks are associated with an 'NSRunLoop'. The currentRunLoop is the
    // the main run loop (since 'didMoveToWindow' is always executed from the main thread.
    [_displayLink addToRunLoop:[NSRunLoop currentRunLoop] forMode:NSRunLoopCommonModes];

    // Perform any actions which need to know the size and scale of the drawable.  When UIKit calls
    // didMoveToWindow after the view initialization, this is the first opportunity to notify
    // components of the drawable's size
    [self resizeDrawable:self.window.screen.nativeScale];

    self.layer.opaque = YES;
    self.opaque = YES;
    self.window.opaque = YES;
}



- (void)setPaused:(BOOL)paused
{
    super.paused = paused;

    _displayLink.paused = paused;
}

- (void)setupCADisplayLinkForScreen:(UIScreen*)screen
{
    [self stopRenderLoop];

    _displayLink = [screen displayLinkWithTarget:self selector:@selector(render)];

    _displayLink.paused = self.paused;

    _displayLink.preferredFramesPerSecond = 60;
}

- (void)didEnterBackground:(NSNotification*)notification
{
    self.paused = YES;
}

- (void)willEnterForeground:(NSNotification*)notification
{
    self.paused = NO;
}

- (void)stopRenderLoop
{
    [_displayLink invalidate];
}



// Override all methods which indicate the view's size has changed

- (void)setContentScaleFactor:(CGFloat)contentScaleFactor
{
    [super setContentScaleFactor:contentScaleFactor];
    [self resizeDrawable:self.window.screen.nativeScale];
}

- (void)layoutSubviews
{
    [super layoutSubviews];
    [self resizeDrawable:self.window.screen.nativeScale];
}

- (void)setFrame:(CGRect)frame
{
    [super setFrame:frame];
    [self resizeDrawable:self.window.screen.nativeScale];
}

- (void)setBounds:(CGRect)bounds
{
    [super setBounds:bounds];
    [self resizeDrawable:self.window.screen.nativeScale];
}

@end
