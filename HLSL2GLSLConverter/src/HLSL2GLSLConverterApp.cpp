// HLSL2GLSLConverterImpl.cpp : Defines the entry point for the console application.
//

#include <Windows.h>
#include <crtdbg.h>

#include "Errors.hpp"
#include "HLSL2GLSLConverterImpl.hpp"
#include "RefCntAutoPtr.hpp"
#include "Errors.hpp"
#include "EngineFactoryOpenGL.h"
#include "RefCntAutoPtr.hpp"
#include "DataBlobImpl.hpp"
#include "FileWrapper.hpp"

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

void PrintHelp()
{
    LOG_INFO_MESSAGE("Command line arguments:\n");
    LOG_INFO_MESSAGE("-h             Print help message\n");
    LOG_INFO_MESSAGE("-i <filename>  Input file path (relative to the search directories)\n");
    LOG_INFO_MESSAGE("-d <dirname>   Search directory to look for input file path as well as all #include files");
    LOG_INFO_MESSAGE("               Every search directory should be specified using -d argument\n");
    LOG_INFO_MESSAGE("-o <filename>  Output file to write converted GLSL source to\n");
    LOG_INFO_MESSAGE("-e <funcname>  Shader entry point\n");
    LOG_INFO_MESSAGE("-c             Compile converted GLSL shader\n");
    LOG_INFO_MESSAGE("-t <type>      Shader type. Allowed values:");
    LOG_INFO_MESSAGE("                 vs - vertex shader");
    LOG_INFO_MESSAGE("                 ps - pixel shader");
    LOG_INFO_MESSAGE("                 gs - geometry shader");
    LOG_INFO_MESSAGE("                 ds - domain shader");
    LOG_INFO_MESSAGE("                 hs - domain shader");
    LOG_INFO_MESSAGE("                 cs - domain shader\n");
    LOG_INFO_MESSAGE("-noglsldef     Do not include glsl definitions into the converted source\n");
    LOG_INFO_MESSAGE("-nolocations   Do not use shader input/output locations qualifiers.\n"
                     "               Shader stage interface linking will rely on exact name matching.\n");
}

// Main
int main(int argc, char** argv)
{
#if defined(_DEBUG) || defined(DEBUG)
    _CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
#endif

    if (argc == 1)
    {
        PrintHelp();
        return 0;
    }

    std::string InputPath, OutputPath, SearchDirectories, EntryPoint("main");
    SHADER_TYPE ShaderType = SHADER_TYPE_UNKNOWN;
    bool CompileShader = false;
    bool IncludeGLSLDefintions = true;
    bool UseInOutLocations = true;
    for (int a = 1; a < argc; ++a) {
        if (_stricmp(argv[a], "-h") == 0) 
        {
            PrintHelp();
        } 
        else if (_stricmp(argv[a], "-i") == 0 && a + 1 < argc) 
        {
            InputPath = argv[++a];
        } 
        else if (_stricmp(argv[a], "-o") == 0 && a + 1 < argc) 
        {
            OutputPath = argv[++a];
        } 
        else if (_stricmp(argv[a], "-d") == 0 && a + 1 < argc) 
        {
            if(!SearchDirectories.empty())
                SearchDirectories.push_back(';');
            SearchDirectories += argv[++a];
        } 
        else if (_stricmp(argv[a], "-e") == 0 && a + 1 < argc) 
        {
            EntryPoint = argv[++a];
        } 
        else if (_stricmp(argv[a], "-c") == 0) 
        {
            CompileShader = true;
        }
        else if (_stricmp(argv[a], "-t") == 0 && a + 1 < argc) 
        {
            ++a;
            if (_stricmp(argv[a], "vs")==0)ShaderType = SHADER_TYPE_VERTEX;
            else if (_stricmp(argv[a], "gs")==0)ShaderType = SHADER_TYPE_GEOMETRY;
            else if (_stricmp(argv[a], "ps")==0)ShaderType = SHADER_TYPE_PIXEL;
            else if (_stricmp(argv[a], "hs")==0)ShaderType = SHADER_TYPE_HULL;
            else if (_stricmp(argv[a], "ds")==0)ShaderType = SHADER_TYPE_DOMAIN;
            else if (_stricmp(argv[a], "cs")==0)ShaderType = SHADER_TYPE_COMPUTE;
            else
            {
                LOG_ERROR_MESSAGE("Unknow shader type ", argv[a],"; Allowed values: vs,gs,ps,ds,hs,cs");
                return -1;
            }
        }
        else if (_stricmp(argv[a], "-noglsldef") == 0)
        {
            IncludeGLSLDefintions = false;
        }
        else if (_stricmp(argv[a], "-nolocations") == 0)
        {
            UseInOutLocations = false;
        }
        else
        {
            LOG_ERROR_MESSAGE("Unknow command line option ", argv[a]);
            return -1;
        }
    }

    if (InputPath.length() == 0)
    {
        LOG_ERROR_MESSAGE("Input file path not specified; use -i command line option");
        return -1;
    }

    if(ShaderType == SHADER_TYPE_UNKNOWN)
    {
        LOG_ERROR_MESSAGE("Shader type not specified; use -t [vs;ps;gs;ds;hs;cs] command line option");
        return -1;
    }

    LOG_INFO_MESSAGE("Converting \'", InputPath, "\' to GLSL...");

#if EXPLICITLY_LOAD_ENGINE_GL_DLL
    // Declare function pointer
    auto GetEngineFactoryOpenGL = LoadGraphicsEngineOpenGL();
    if (GetEngineFactoryOpenGL == nullptr)
    {
        LOG_ERROR_MESSAGE("Failed to load OpenGL engine implementation");
        return -1;
    }
#endif
    IEngineFactoryOpenGL *pFactory = GetEngineFactoryOpenGL();

    RefCntAutoPtr<IShaderSourceInputStreamFactory> pShaderSourceFactory;
    pFactory->CreateDefaultShaderSourceStreamFactory(SearchDirectories.c_str(), &pShaderSourceFactory);

    RefCntAutoPtr<IFileStream> pInputFileStream;
    pShaderSourceFactory->CreateInputStream(InputPath.c_str(), &pInputFileStream);
    if (!pInputFileStream)
    {
        return -1;
    }
    RefCntAutoPtr<Diligent::IDataBlob> pHLSLSourceBlob( MakeNewRCObj<DataBlobImpl>()(0) );
    pInputFileStream->ReadBlob(pHLSLSourceBlob);
    auto *HLSLSource = reinterpret_cast<char*>(pHLSLSourceBlob->GetDataPtr());
    auto SourceLen = static_cast<Int32>( pHLSLSourceBlob->GetSize() );

    const auto &Converter = HLSL2GLSLConverterImpl::GetInstance();
    RefCntAutoPtr<IHLSL2GLSLConversionStream> pStream;
    Converter.CreateStream(InputPath.c_str(), pShaderSourceFactory, HLSLSource, SourceLen, &pStream);
    RefCntAutoPtr<Diligent::IDataBlob> pGLSLSourceBlob;
    pStream->Convert(EntryPoint.c_str(), ShaderType, IncludeGLSLDefintions, "_sampler", UseInOutLocations, &pGLSLSourceBlob);
    if(!pGLSLSourceBlob)return -1;

    LOG_INFO_MESSAGE("Done");

    if (OutputPath.length() != 0)
    {
        FileWrapper pOutputFile( OutputPath.c_str(), EFileAccessMode::Overwrite );
        if (pOutputFile != nullptr)
        {
            if( !pOutputFile->Write(pGLSLSourceBlob->GetDataPtr(), pGLSLSourceBlob->GetSize()) )
            {
                LOG_ERROR_MESSAGE("Failed to write converted source to output file ", OutputPath);
            }
        }
        else
        {
            LOG_ERROR_MESSAGE("Failed to open output file ", OutputPath);
        }
    }

    if(CompileShader)
    {
        LOG_INFO_MESSAGE("Compiling entry point \'", EntryPoint, "\' in converted file \'", InputPath, '\'');
        // Register window cla  ss
        WNDCLASSEX wcex = { sizeof(WNDCLASSEX), CS_HREDRAW|CS_VREDRAW, MessageProc,
                            0L, 0L, GetModuleHandle(NULL), NULL, NULL, NULL, NULL, L"HLSL2GLSLConverter", NULL };
        RegisterClassEx(&wcex);

        // Create dummy window
        RECT rc = { 0, 0, 512, 512 };
        AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);
        HWND wnd = CreateWindow(L"HLSL2GLSLConverter", L"HLSL2GLSL Converter", 
                                WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 
                                rc.right-rc.left, rc.bottom-rc.top, NULL, NULL, wcex.hInstance, NULL);

        if (!wnd)
        {
            LOG_ERROR_MESSAGE("Failed to create window");
            return -1;
        }
        RefCntAutoPtr<IRenderDevice> pDevice;
        RefCntAutoPtr<IDeviceContext> pContext;
        RefCntAutoPtr<ISwapChain> pSwapChain;
        EngineGLCreateInfo EngineCI;
        SwapChainDesc SCDesc;
        EngineCI.Window.hWnd = wnd;
        pFactory->CreateDeviceAndSwapChainGL(
            EngineCI, &pDevice, &pContext, SCDesc, &pSwapChain );
        if (!pDevice)
        {
            LOG_ERROR_MESSAGE("Failed to create render device");
            return -1;
        }

        ShaderCreateInfo ShaderCI;
        ShaderCI.EntryPoint = EntryPoint.c_str();
        ShaderCI.Desc.ShaderType = ShaderType;
        ShaderCI.Desc.Name = "Test shader";
        ShaderCI.Source = reinterpret_cast<char*>(pGLSLSourceBlob->GetDataPtr());
        ShaderCI.SourceLanguage = SHADER_SOURCE_LANGUAGE_GLSL;
        ShaderCI.UseCombinedTextureSamplers = true;
        RefCntAutoPtr<IShader> pTestShader;
        pDevice->CreateShader(ShaderCI, &pTestShader);
        if(!pTestShader)
        {
            LOG_ERROR_MESSAGE("Failed to compile converted source \'", InputPath, '\'');
            return -1;
        }
        LOG_INFO_MESSAGE("Done");
    }

    return 0;
}
