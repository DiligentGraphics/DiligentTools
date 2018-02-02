#if defined(PLATFORM_WIN32) || defined(PLATFORM_UNIVERSAL_WINDOWS)

#   include "tiffconf.vc.h"

#elif defined(PLATFORM_ANDROID) || defined(PLATFORM_LINUX) || defined(PLATFORM_MACOS) || defined(PLATFORM_IOS)

#   include "tiffconf.linux.h"

#else

#   error Unsupported platform

#endif
