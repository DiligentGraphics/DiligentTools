/*
See LICENSE folder for this sample’s licensing information.
*/

#import "MetalViewController.h"
#import "MainUIView.h"

#include "NativeAppBase.hpp"
#include "GraphicsTypes.h"

#import <QuartzCore/CAMetalLayer.h>

#include <memory>

@implementation MetalViewController
{
    std::unique_ptr<Diligent::NativeAppBase> _theApp;
}

- (void)viewDidLoad
{
    [super viewDidLoad];

    MetalView *view = (MetalView *)self.view;

    // Set this class as the delegate to receive resize and render callbacks.
    view.delegate = self;

    _theApp.reset(Diligent::CreateApplication());

    // Init the application.
    try
    {
        Diligent::RENDER_DEVICE_TYPE DeviceType = Diligent::RENDER_DEVICE_TYPE_UNDEFINED;
#if METAL_SUPPORTED
        DeviceType = Diligent::RENDER_DEVICE_TYPE_METAL;
#elif VULKAN_SUPPORTED
        DeviceType = Diligent::RENDER_DEVICE_TYPE_VULKAN;
#else
        #error No supported API
#endif
        _theApp->Initialize(DeviceType, (__bridge void*)view.metalLayer);
    }
    catch(std::runtime_error &err)
    {
        _theApp.reset();
    }
}

- (void)drawableResize:(CGSize)size
{
    if (_theApp)
    {
        _theApp->WindowResize(size.width, size.height);
    }
}

- (void)renderToMetalLayer:(nonnull CAMetalLayer *)layer
{
    if (_theApp)
    {
        _theApp->Update();
        _theApp->Render();
        _theApp->Present();
    }
}

@end
