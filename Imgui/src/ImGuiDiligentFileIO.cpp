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

#include <cstring>

#include "ImGuiDiligentConfig.h"
#include "imgui.h"
#include "imgui_internal.h"
#include "FileSystem.hpp"

using namespace Diligent;

ImFileHandle ImFileOpen(const char* pFileName, const char* pMode)
{
    if (!pFileName || !pMode || !pMode[0])
        return nullptr;

    const bool HasPlus = std::strchr(pMode, '+') != nullptr;

    EFileAccessMode AccessMode = EFileAccessMode::Read;

    switch (pMode[0])
    {
        case 'w': AccessMode = HasPlus ? EFileAccessMode::OverwriteUpdate : EFileAccessMode::Overwrite; break;
        case 'a': AccessMode = HasPlus ? EFileAccessMode::AppendUpdate : EFileAccessMode::Append; break;
        case 'r': AccessMode = HasPlus ? EFileAccessMode::ReadUpdate : EFileAccessMode::Read; break;
        default: AccessMode = HasPlus ? EFileAccessMode::ReadUpdate : EFileAccessMode::Read; break;
    }

    return static_cast<ImFileHandle>(FileSystem::OpenFile({pFileName, AccessMode}));
}

bool ImFileClose(ImFileHandle pFile)
{
    if (!pFile)
        return false;
    delete pFile;
    return true;
}

ImU64 ImFileGetSize(ImFileHandle pFile)
{
    if (!pFile)
        return static_cast<ImU64>(-1);
    return static_cast<ImU64>(pFile->GetSize());
}

ImU64 ImFileRead(void* pData, ImU64 ElemSize, ImU64 ElemCount, ImFileHandle pFile)
{
    if (!pFile || ElemSize == 0 || ElemCount == 0)
        return 0;
    const size_t TotalSize = static_cast<size_t>(ElemSize * ElemCount);
    return pFile->Read(pData, TotalSize) ? ElemCount : 0;
}

ImU64 ImFileWrite(const void* pData, ImU64 ElemSize, ImU64 ElemCount, ImFileHandle pFile)
{
    if (!pFile || ElemSize == 0 || ElemCount == 0)
        return 0;
    const size_t TotalSize = static_cast<size_t>(ElemSize * ElemCount);
    return pFile->Write(pData, TotalSize) ? ElemCount : 0;
}
