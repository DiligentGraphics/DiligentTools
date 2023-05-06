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

#include "HLSL2GLSLConverterApp.h"

#include "Errors.hpp"
#include "HLSL2GLSLConverter.h"
#include "RefCntAutoPtr.hpp"
#include "Errors.hpp"
#include "EngineFactoryOpenGL.h"
#include "RefCntAutoPtr.hpp"
#include "DataBlobImpl.hpp"
#include "FileWrapper.hpp"
#include "args.hxx"

namespace Diligent
{

HLSL2GLSLConverterApp::HLSL2GLSLConverterApp()
{
#if EXPLICITLY_LOAD_ENGINE_GL_DLL
    // Declare function pointer
    auto GetEngineFactoryOpenGL = LoadGraphicsEngineOpenGL();
    if (GetEngineFactoryOpenGL == nullptr)
    {
        LOG_ERROR_MESSAGE("Failed to load OpenGL engine implementation");
        return -1;
    }
#endif
    m_pFactoryGL = GetEngineFactoryOpenGL();
}

int HLSL2GLSLConverterApp::ParseCmdLine(int argc, char** argv)
{
    args::ArgumentParser Parser{"HLSL->GLSL off-line converter"};

    args::HelpFlag Help{Parser, "help", "Show command line help", {'h', "help"}};

    args::ValueFlag<std::string>     InputArg{Parser, "filename", "Input file path", {'i', "in"}, ""};
    args::ValueFlag<std::string>     OutputArg{Parser, "filename", "Output file path where converted GLSL source will be saved", {'o', "out"}, ""};
    args::ValueFlagList<std::string> SearDirsArg{Parser, "dirname", "Search directories to look for input file as well as all includes", {'d', "dirs"}, {}};
    args::ValueFlag<std::string>     EntryArg{Parser, "funcname", "Shader entry point", {'e', "entry"}, "main"};

    std::unordered_map<std::string, SHADER_TYPE> ShaderTypeMap //
        {
            {"vs", SHADER_TYPE_VERTEX},
            {"gs", SHADER_TYPE_GEOMETRY},
            {"ds", SHADER_TYPE_DOMAIN},
            {"hs", SHADER_TYPE_HULL},
            {"ps", SHADER_TYPE_PIXEL},
            {"cs", SHADER_TYPE_COMPUTE} //
        };
    args::MapFlag<std::string, SHADER_TYPE> ShaderTypeArg{Parser, "shader_type", "Shader type. Allowed values:\n"
                                                                                 "  vs - vertex shader\n"
                                                                                 "  gs - geometry shader\n"
                                                                                 "  hs - hull (tess control) shader\n"
                                                                                 "  ds - domain (tess eval) shader\n"
                                                                                 "  ps - pixel shader\n"
                                                                                 "  cs - compute shader",
                                                          {'t', "type"},
                                                          ShaderTypeMap,
                                                          SHADER_TYPE_UNKNOWN};

    args::Flag CompileArg{Parser, "compile", "Compile converted GLSL shader", {'c', "compile"}};
    args::Flag NoGlslDefArg{Parser, "noglsldef", "Do not include glsl definitions into the converted source", {"no-glsl-definitions"}};
    args::Flag NoLocationsArg{Parser, "nolocations", "Do not use shader input/output locations qualifiers. Shader stage interface linking will rely on exact name matching.", {"no-locations"}};
    args::Flag PrintArg{Parser, "print", "Print resulting converted file to console.", {'p', "print"}};

    if (argc <= 1)
    {
        Parser.Help();
        return 1;
    }

    try
    {
        Parser.ParseCLI(argc, argv);
        if (!InputArg)
            throw args::Error{"Input file path is not specified"};
        if (!ShaderTypeArg)
            throw args::Error{"Shader type is not specified"};
    }
    catch (const args::Help&)
    {
        LOG_INFO_MESSAGE(Parser.Help());
        return 1;
    }
    catch (const args::Error& e)
    {
        LOG_ERROR_MESSAGE(e.what());
        LOG_INFO_MESSAGE(Parser.Help());
        return -1;
    }

    m_InputPath  = InputArg.Get();
    m_OutputPath = OutputArg.Get();
    for (const auto& Dir : SearDirsArg.Get())
    {
        if (!m_SearchDirectories.empty())
            m_SearchDirectories.push_back(';');
        m_SearchDirectories += Dir;
    }

    m_EntryPoint            = EntryArg.Get();
    m_ShaderType            = ShaderTypeArg.Get();
    m_CompileShader         = CompileArg.Get();
    m_IncludeGLSLDefintions = !NoGlslDefArg.Get();
    m_UseInOutLocations     = !NoLocationsArg.Get();
    m_PrintConvertedSource  = PrintArg.Get();

    return 0;
}

int HLSL2GLSLConverterApp::Convert(IRenderDevice* pDevice)
{
    if (m_InputPath.length() == 0)
    {
        LOG_ERROR_MESSAGE("Input file path not specified; use -i command line option");
        return -1;
    }

    if (m_ShaderType == SHADER_TYPE_UNKNOWN)
    {
        LOG_ERROR_MESSAGE("Shader type not specified; use -t [vs;ps;gs;ds;hs;cs] command line option");
        return -1;
    }

    LOG_INFO_MESSAGE("Converting \'", m_InputPath, "\' to GLSL...");

    RefCntAutoPtr<IShaderSourceInputStreamFactory> pShaderSourceFactory;
    m_pFactoryGL->CreateDefaultShaderSourceStreamFactory(m_SearchDirectories.c_str(), &pShaderSourceFactory);

    RefCntAutoPtr<IFileStream> pInputFileStream;
    pShaderSourceFactory->CreateInputStream(m_InputPath.c_str(), &pInputFileStream);
    if (!pInputFileStream)
    {
        return -1;
    }
    auto pHLSLSourceBlob = DataBlobImpl::Create();
    pInputFileStream->ReadBlob(pHLSLSourceBlob);
    auto* HLSLSource = reinterpret_cast<char*>(pHLSLSourceBlob->GetDataPtr());
    auto  SourceLen  = static_cast<Int32>(pHLSLSourceBlob->GetSize());

    RefCntAutoPtr<IHLSL2GLSLConverter> pConverter;
    CreateHLSL2GLSLConverter(&pConverter);
    if (!pConverter)
    {
        LOG_ERROR_MESSAGE("Failed to create HLSL2GLSL converter");
        return -1;
    }

    RefCntAutoPtr<IHLSL2GLSLConversionStream> pStream;
    pConverter->CreateStream(m_InputPath.c_str(), pShaderSourceFactory, HLSLSource, SourceLen, &pStream);
    RefCntAutoPtr<IDataBlob> pGLSLSourceBlob;
    pStream->Convert(m_EntryPoint.c_str(), m_ShaderType, m_IncludeGLSLDefintions, "_sampler", m_UseInOutLocations, &pGLSLSourceBlob);
    if (!pGLSLSourceBlob) return -1;

    LOG_INFO_MESSAGE("Done");

    if (m_OutputPath.length() != 0)
    {
        FileWrapper pOutputFile(m_OutputPath.c_str(), EFileAccessMode::Overwrite);
        if (pOutputFile != nullptr)
        {
            if (!pOutputFile->Write(pGLSLSourceBlob->GetDataPtr(), pGLSLSourceBlob->GetSize()))
            {
                LOG_ERROR_MESSAGE("Failed to write converted source to output file ", m_OutputPath);
                return -1;
            }
        }
        else
        {
            LOG_ERROR_MESSAGE("Failed to open output file ", m_OutputPath);
            return -1;
        }
    }

    if (pDevice != nullptr)
    {
        LOG_INFO_MESSAGE("Compiling entry point \'", m_EntryPoint, "\' in converted file \'", m_InputPath, '\'');

        ShaderCreateInfo ShaderCI;
        ShaderCI.EntryPoint     = m_EntryPoint.c_str();
        ShaderCI.Desc           = {"Test shader", m_ShaderType, true};
        ShaderCI.Source         = reinterpret_cast<const char*>(pGLSLSourceBlob->GetConstDataPtr());
        ShaderCI.SourceLanguage = SHADER_SOURCE_LANGUAGE_GLSL;
        RefCntAutoPtr<IShader> pTestShader;
        pDevice->CreateShader(ShaderCI, &pTestShader);
        if (!pTestShader)
        {
            LOG_ERROR_MESSAGE("Failed to compile converted source \'", m_InputPath, '\'');
            return -1;
        }
        LOG_INFO_MESSAGE("Done");
    }

    if (m_PrintConvertedSource)
    {
        LOG_INFO_MESSAGE("Converted GLSL:\n", reinterpret_cast<const char*>(pGLSLSourceBlob->GetConstDataPtr()));
    }

    return 0;
}

} // namespace Diligent
