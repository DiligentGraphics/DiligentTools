/*     Copyright 2015-2017 Egor Yusov
 *  
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 * 
 *     http://www.apache.org/licenses/LICENSE-2.0
 * 
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT OF ANY PROPRIETARY RIGHTS.
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

#include "Errors.h"
#include "FileWrapper.h"
#include "ScriptParser.h"
#include "DataBlobImpl.h"
#ifdef _WIN32
#   ifndef NOMINMAX
#       define NOMINMAX
#   endif
#   include <Windows.h>
#endif

inline void SetGlobalVarsStub( Diligent::ScriptParser * ){}

template<typename TSetGlobalVars>
Diligent::RefCntAutoPtr<Diligent::ScriptParser> CreateRenderScriptFromFile( const Diligent::Char *FilePath, Diligent::IRenderDevice *pRenderDevice, Diligent::IDeviceContext *pContext, TSetGlobalVars SetGlobalVars )
{
    bool bSuccess = true;
    Diligent::RefCntAutoPtr<Diligent::ScriptParser> pScriptParser;
#ifdef PLATFORM_WIN32
    do
    {
#endif
        std::string ErrorMsg;
        bSuccess = true;
        try
        {
            Diligent::FileWrapper ScriptFile(FilePath);
            if( !ScriptFile )
                LOG_ERROR_AND_THROW( "Failed to open Lua source file" );
            Diligent::RefCntAutoPtr<Diligent::IDataBlob> pFileData( Diligent::MakeNewRCObj<Diligent::DataBlobImpl>()(0) );
            ScriptFile->Read( pFileData );
            
            // Null-terminator is not read from the stream
            pFileData->Resize(pFileData->GetSize() + 1);
            auto *ScriptText = reinterpret_cast<char*>(pFileData->GetDataPtr());
            ScriptText[pFileData->GetSize() - 1] = 0;
            
            pScriptParser = Diligent::MakeNewRCObj<Diligent::ScriptParser>()( pRenderDevice );
            pScriptParser->Parse( ScriptText );
            SetGlobalVars( pScriptParser );
            pScriptParser->Run( pContext );
        }
        catch( const std::runtime_error &err )
        {
            bSuccess = false;
            ErrorMsg = err.what();
        }

#ifdef PLATFORM_WIN32
        if( !bSuccess )
        {
            if( IDRETRY != MessageBoxA( NULL, "Failed to parse the script. Retry?", "Lua parser error", MB_ICONERROR | MB_ABORTRETRYIGNORE ) )
            {
                abort();
            }
        }
    } while( !bSuccess );
#endif

    return pScriptParser;
}
