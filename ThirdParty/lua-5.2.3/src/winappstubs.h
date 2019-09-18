/*
** $Id: winappstubs.h, Egor Yusov
** Stubs for functions unavailable
** for Windows Store Apps
*/

#ifndef winappstubs_h
#define winappstubs_h

#include <stdio.h>
#include <cassert>

// Do NOT include windows headers! They #define CreateDirectory,
// DeleteFile, min, max, and do other evil.
// This file is included by luaconf.h, which is in turn included by
// lua.h. So including windows headers here will propagate to every
// file that uses lua.h.
// #define NOMINMAX
// #include <Windows.h>

#define Unsupported(Message) assert(Message && false);

inline int _pclose( FILE * )
{
    Unsupported("_pclose() is not available for Windows App Store applications");
    return 0;
}

inline FILE *_popen( const char *, const char * )
{
    Unsupported("_popen() is not available for Windows App Store applications");
    return nullptr;
}

inline int system( const char* command )
{
    Unsupported("system() is not available for Windows App Store applications");
    return 0;
}

inline char* getenv( const char* name )
{
    Unsupported("getenv() is not available for Windows App Store applications");
    return nullptr;
}

// Definitions of windows types are not available for us as we want to avoid
// inclusion of windows headers
template<typename HMODULE, typename LPSTR, typename DWORD>
inline DWORD GetModuleFileNameA(HMODULE hModule, LPSTR lpFilename, DWORD nSize)
{
    Unsupported("GetModuleFileNameA() is not available for Windows App Store applications");
    return 0;
}


template<typename HMODULE, typename LPCSTR, typename HANDLE, typename DWORD>
inline HMODULE LoadLibraryExA_stub(LPCSTR lpFileName, HANDLE hFile, DWORD dwFlags)
{
    Unsupported("LoadLibraryExA() is not available for Windows App Store applications");
    return 0;
}
// HMODULE will be defined where this macro is used, so we do not have to define it
#define LoadLibraryExA(lpFileName, hFile, dwFlags) LoadLibraryExA_stub<HMODULE>(lpFileName, hFile, dwFlags)

#endif
