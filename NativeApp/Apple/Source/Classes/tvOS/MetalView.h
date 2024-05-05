/*
See LICENSE folder for this sample’s licensing information.
*/

#import <QuartzCore/CAMetalLayer.h>
#import <Metal/Metal.h>
#import <UIKit/UIView.h>

// Protocol to provide resize and redraw callbacks to a delegate
@protocol MetalViewDelegate <NSObject>

- (void)drawableResize:(CGSize)size;

- (void)renderToMetalLayer:(nonnull CAMetalLayer *)metalLayer;

@end

// Metal view base class
@interface MetalView : UIView <CALayerDelegate>

@property (nonatomic, nonnull, readonly) CAMetalLayer *metalLayer;

@property (nonatomic, getter=isPaused) BOOL paused;

@property (nonatomic, nullable, retain) id<MetalViewDelegate> delegate;

- (void)initCommon;

- (void)resizeDrawable:(CGFloat)scaleFactor;
- (void)stopRenderLoop;
- (void)render;

@end
