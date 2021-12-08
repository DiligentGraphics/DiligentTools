/*
 *  Copyright 2019-2021 Diligent Graphics LLC
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

#include "json.hpp"
#include "BasicMath.hpp"
#include "DynamicLinearAllocator.hpp"
#include "FileWrapper.hpp"
#include "DataBlobImpl.hpp"
#include "DefaultRawMemoryAllocator.hpp"
#include "RenderDevice.h"
#include "CommonParser.hpp"
#include "GraphicsTypesParser.hpp"
#include "BlendStateParser.hpp"
#include "RasterizerStateParser.hpp"
#include "DepthStencilStateParser.hpp"
#include "InputLayoutParser.hpp"
#include "SamplerParser.hpp"
#include "ShaderParser.hpp"
#include "ShaderResourceVariableParser.hpp"
#include "PipelineResourceSignatureParser.hpp"
#include "RenderPassParser.hpp"
#include "PipelineStateParser.hpp"

namespace Diligent
{

inline nlohmann::json LoadDRSNFromFile(const Char* FilePath)
{
    try
    {
        FileWrapper File{FilePath, EFileAccessMode::Read};
        if (!File)
            LOG_ERROR_AND_THROW("Failed to open file '", FilePath, "'.");

        auto pFileData = DataBlobImpl::Create();
        File->Read(pFileData);

        String Source{reinterpret_cast<const char*>(pFileData->GetConstDataPtr()), pFileData->GetSize()};
        return nlohmann::json::parse(Source);
    }
    catch (std::runtime_error& err)
    {
        LOG_FATAL_ERROR_AND_THROW(err.what());
    }
}

template <typename Type, typename Counter>
bool TestEnum(DynamicLinearAllocator& Allocator, Type ValueBegin, Type ValueEnd)
{
    for (Counter i = static_cast<Counter>(ValueBegin); i < static_cast<Counter>(ValueEnd) + 1; i++)
    {
        nlohmann::json Json;
        Type           EnumReference = static_cast<Type>(i);
        Serialize(Json, EnumReference, Allocator);

        Type Enum = {};
        Deserialize(Json, Enum, Allocator);
        if (Enum != EnumReference)
            return false;
    }
    return true;
}

template <typename Type, typename Counter>
bool TestBitwiseEnum(DynamicLinearAllocator& Allocator, Type Value)
{
    for (Counter Bits = static_cast<Type>(Value | (Value - 1u)); Bits != 0;)
    {
        nlohmann::json Json;
        Type           EnumReference = static_cast<Type>(ExtractLSB(Bits));
        SerializeBitwiseEnum(Json, EnumReference, Allocator);

        Type Enum = {};
        DeserializeBitwiseEnum(Json, Enum, Allocator);
        if (Enum != EnumReference)
            return false;
    }
    return true;
}

} // namespace Diligent
