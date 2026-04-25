#import "SceneDelegate.h"
#import "BaseView.h"
#import "MetalView.h"

@implementation SceneDelegate

@synthesize window = _window;

- (void) scene:(UIScene*)scene
    willConnectToSession:(UISceneSession*)session
                 options:(UISceneConnectionOptions*)connectionOptions
{
    if (![scene isKindOfClass:[UIWindowScene class]])
        return;

    UIWindow* window = [[UIWindow alloc] initWithWindowScene:(UIWindowScene*)scene];

    UIViewController* viewController = [[UIViewController alloc] init];
    MetalView*        view           = [[MetalView alloc] initWithFrame:window.bounds];
    view.autoresizingMask           = UIViewAutoresizingFlexibleWidth | UIViewAutoresizingFlexibleHeight;
    viewController.view             = view;

    window.rootViewController = viewController;
    self.window               = window;
    [window makeKeyAndVisible];
    [view startAnimation];

    [view release];
    [viewController release];
    [window release];
}

- (void) sceneDidDisconnect:(UIScene*)scene
{
    [(BaseView*)self.window.rootViewController.view stopAnimation];
    [(BaseView*)self.window.rootViewController.view terminate];
    self.window = nil;
}

- (void) sceneWillResignActive:(UIScene*)scene
{
    [(BaseView*)self.window.rootViewController.view stopAnimation];
}

- (void) sceneDidEnterBackground:(UIScene*)scene
{
    [(BaseView*)self.window.rootViewController.view stopAnimation];
}

- (void) sceneWillEnterForeground:(UIScene*)scene
{
    [(BaseView*)self.window.rootViewController.view startAnimation];
}

- (void) sceneDidBecomeActive:(UIScene*)scene
{
    [(BaseView*)self.window.rootViewController.view startAnimation];
}

- (void) dealloc
{
    [_window release];
    [super dealloc];
}

@end
