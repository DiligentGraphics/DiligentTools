/*
 *  Copyright 2026 Diligent Graphics LLC
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

#include "GraphicsAccessories.hpp"

namespace Diligent
{

namespace GLTF
{

class VertexDataConverter final
{
public:
    struct WriteAttribs
    {
        const void* pSrc             = nullptr;
        VALUE_TYPE  SrcType          = VT_UNDEFINED;
        Uint32      NumSrcComponents = 0;
        Uint32      SrcElementStride = 0;

        void*      pDst             = nullptr;
        VALUE_TYPE DstType          = VT_UNDEFINED;
        Uint32     NumDstComponents = 0;
        Uint32     DstElementStride = 0;

        Uint32 NumElements  = 0;
        bool   IsNormalized = false;
    };

    static bool Write(const WriteAttribs& Attribs);

    struct WriteDefaultAttribs
    {
        const void* pDefaultValue    = nullptr;
        void*       pDst             = nullptr;
        VALUE_TYPE  DstType          = VT_UNDEFINED;
        Uint32      NumDstComponents = 0;
        Uint32      DstElementStride = 0;
        Uint32      NumElements      = 0;
    };

    static bool WriteDefault(const WriteDefaultAttribs& Attribs);
};

} // namespace GLTF

} // namespace Diligent
