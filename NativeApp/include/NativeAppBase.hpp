/*
 *  Copyright 2019-2022 Diligent Graphics LLC
 *  Copyright 2015-2019 Egor Yusov
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 *  In no event and under no legal theory, whether in tort (including negligence),
 *  contract, or otherwise, unless required by applicable law (such as deliberate
 *  and grossly negligent acts) or agreed to in writing, shall any Contributor be
 *  liable for any damages, including any direct, indirect, special, incidental,
 *  or consequential damages of any character arising as a result of this License or
 *  out of the use or inability to use the software (including but not limited to damages
 *  for loss of goodwill, work stoppage, computer failure or malfunction, or any and
 *  all other commercial damages or losses), even if such Contributor has been advised
 *  of the possibility of such damages.
 */
#pragma once

// clang-format off

#if PLATFORM_WIN32

    #include "Win32/Win32AppBase.hpp"
    namespace Diligent
    {
        using NativeAppBase = Win32AppBase;
    }

#elif PLATFORM_UNIVERSAL_WINDOWS

    #include "UWP/UWPAppBase.hpp"
    namespace Diligent
    {
        using NativeAppBase = UWPAppBase;
    }

#elif PLATFORM_LINUX

    #include "Linux/LinuxAppBase.hpp"
    namespace Diligent
    {
        using NativeAppBase = LinuxAppBase;
    }

#elif PLATFORM_ANDROID

    #include "Android/AndroidAppBase.hpp"
    namespace Diligent
    {
        using NativeAppBase = AndroidAppBase;
    }

#elif PLATFORM_MACOS

    #include "MacOS/MacOSAppBase.hpp"
    namespace Diligent
    {
        using NativeAppBase = MacOSAppBase;
    }

#elif PLATFORM_IOS

    #include "IOS/IOSAppBase.hpp"
    namespace Diligent
    {
        using NativeAppBase = IOSAppBase;
    }

#elif PLATFORM_TVOS

    #include "TVOS/TVOSAppBase.hpp"
    namespace Diligent
    {
        using NativeAppBase = TVOSAppBase;
    }

#elif PLATFORM_WEB

    #include "Emscripten/EmscriptenAppBase.hpp"
    namespace Diligent 
    {
        using NativeAppBase = EmscriptenAppBase;
    }

#else

#   error Usnupported platform

#endif

namespace Diligent
{
    extern NativeAppBase* CreateApplication();
}
