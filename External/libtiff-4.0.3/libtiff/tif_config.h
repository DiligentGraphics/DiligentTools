#if PLATFORM_WIN32 || PLATFORM_UNIVERSAL_WINDOWS

#   include "tif_config.vc.h"

#elif PLATFORM_ANDROID

#   include "tif_config.android.h"

#elif PLATFORM_LINUX || PLATFORM_MACOS || PLATFORM_IOS

#   include "tif_config.linux.h"

#else

#   error "Unknown platform"

#endif
