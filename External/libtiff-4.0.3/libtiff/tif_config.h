#if defined(PLATFORM_WIN32) || defined(PLATFORM_UNIVERSAL_WINDOWS)

#   include "tif_config.vc.h"

#elif defined(PLATFORM_ANDROID)

#   include "tif_config.android.h"

#elif defined(PLATFORM_LINUX) || defined(PLATFORM_MACOS)

#   include "tif_config.linux.h"

#else

#   error "Unknown platform"

#endif
