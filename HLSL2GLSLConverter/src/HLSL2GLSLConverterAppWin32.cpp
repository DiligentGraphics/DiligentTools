// HLSL2GLSLConverterImpl.cpp : Defines the entry point for the console application.
//

#include "HLSL2GLSLConverterApp.h"
#include "Errors.hpp"
#include "RenderDevice.h"
#include "EngineFactoryOpenGL.h"
#include "RefCntAutoPtr.hpp"

#include <Windows.h>
#include <crtdbg.h>

using namespace Diligent;

// Called every time the application receives a message
LRESULT CALLBACK MessageProc(HWND wnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    // Send event message to AntTweakBar
    switch (message)
    {
        case WM_PAINT:
        {
            PAINTSTRUCT ps;
            BeginPaint(wnd, &ps);
            EndPaint(wnd, &ps);
            return 0;
        }
        case WM_SIZE: // Window size has been changed
            return 0;

        case WM_CHAR:
            if (wParam == VK_ESCAPE)
                PostQuitMessage(0);
            return 0;

        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;

        default:
            return DefWindowProc(wnd, message, wParam, lParam);
    }
}

// Main
int main(int argc, char** argv)
{
#if defined(_DEBUG) || defined(DEBUG)
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

    HLSL2GLSLConverterApp Converter;

    if (argc == 1)
    {
        Converter.PrintHelp();
        return 0;
    }

    {
        auto ret = Converter.ParseCmdLine(argc, argv);
        if (ret != 0)
            return ret;
    }

    RefCntAutoPtr<IRenderDevice>  pDevice;
    RefCntAutoPtr<IDeviceContext> pContext;
    RefCntAutoPtr<ISwapChain>     pSwapChain;

    if (Converter.NeedsCompileShader())
    {
        // Register window cla  ss
        WNDCLASSEX wcex = {sizeof(WNDCLASSEX), CS_HREDRAW | CS_VREDRAW, MessageProc,
                           0L, 0L, GetModuleHandle(NULL), NULL, NULL, NULL, NULL, L"HLSL2GLSLConverter", NULL};
        RegisterClassEx(&wcex);

        // Create dummy window
        RECT rc = {0, 0, 512, 512};
        AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);
        HWND wnd = CreateWindow(L"HLSL2GLSLConverter", L"HLSL2GLSL Converter",
                                WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
                                rc.right - rc.left, rc.bottom - rc.top, NULL, NULL, wcex.hInstance, NULL);

        if (!wnd)
        {
            LOG_ERROR_MESSAGE("Failed to create window");
            return -1;
        }
        EngineGLCreateInfo EngineCI;
        SwapChainDesc      SCDesc;
        EngineCI.Window.hWnd = wnd;

        auto* pFactory = Converter.GetFactoryGL();
        pFactory->CreateDeviceAndSwapChainGL(
            EngineCI, &pDevice, &pContext, SCDesc, &pSwapChain);
        if (!pDevice)
        {
            LOG_ERROR_MESSAGE("Failed to create render device");
            return -1;
        }
    }

    return Converter.Convert(pDevice);
}
