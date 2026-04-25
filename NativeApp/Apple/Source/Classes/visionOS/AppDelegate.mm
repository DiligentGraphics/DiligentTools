#import "AppDelegate.h"
#import "SceneDelegate.h"

@implementation AppDelegate

- (UISceneConfiguration*) application:(UIApplication*)application
    configurationForConnectingSceneSession:(UISceneSession*)connectingSceneSession
                                 options:(UISceneConnectionOptions*)options
{
    UISceneConfiguration* configuration =
        [[[UISceneConfiguration alloc] initWithName:@"Default Configuration"
                                        sessionRole:connectingSceneSession.role] autorelease];
    configuration.delegateClass = [SceneDelegate class];
    return configuration;
}

@end
