#if PLATFORM_WIN32 || PLATFORM_UNIVERSAL_WINDOWS

#   include "tiffconf.vc.h"

#elif PLATFORM_ANDROID || PLATFORM_LINUX || PLATFORM_MACOS || PLATFORM_IOS

#   include "tiffconf.linux.h"

#else

#   error Unsupported platform

#endif
