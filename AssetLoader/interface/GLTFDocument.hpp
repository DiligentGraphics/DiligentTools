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

/// \file
/// GLTF document loading and metadata queries.

#include <functional>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "../../../DiligentCore/Platforms/interface/PlatformMisc.hpp"
#include "../../../DiligentCore/Common/interface/RefCntAutoPtr.hpp"
#include "../../../DiligentCore/Common/interface/SharedMutex.hpp"

namespace tinygltf
{

class Model;
struct Texture;

} // namespace tinygltf

namespace Diligent
{

struct ITexture;

namespace GLTF
{

class ResourceManager;

/// Texture cache used by the GLTF loader.
struct TextureCacheType
{
    Threading::SharedMutex TexturesMtx;

    std::unordered_map<std::string, RefCntWeakPtr<ITexture>> Textures;
};

/// GLTF document load information.
struct DocumentLoadInfo
{
    using FileExistsCallbackType    = std::function<bool(const char* FilePath)>;
    using ReadWholeFileCallbackType = std::function<bool(const char* FilePath, std::vector<unsigned char>& Data, std::string& Error)>;

    /// File name.
    const char* FileName = nullptr;

    /// Optional callback function that will be called by the loader to check if the file exists.
    FileExistsCallbackType FileExistsCallback = nullptr;

    /// Optional callback function that will be called by the loader to read the whole file.
    ReadWholeFileCallbackType ReadWholeFileCallback = nullptr;

    /// Optional texture cache to use when loading images referenced by the document.
    TextureCacheType* pTextureCache = nullptr;

    /// Optional resource manager to use when loading images referenced by the document.
    ResourceManager* pResourceManager = nullptr;

    /// Whether image data should be decoded into pixels while loading the document.
    ///
    /// When this is false, the document still parses image and texture metadata.
    /// Images stored in buffer views remain addressable through tinygltf::Image::bufferView
    /// without duplicating their bytes in tinygltf::Image::image. External image files with
    /// known image extensions are not read by the document loader and remain addressable
    /// through tinygltf::Image::uri. Embedded data URI images are copied into
    /// tinygltf::Image::image as encoded image bytes because tinygltf supplies them
    /// through temporary callback storage.
    bool DecodeImages = true;
};

/// Resolved texture source referenced by a GLTF texture.
struct TextureSourceInfo
{
    /// Index in tinygltf::Model::textures.
    Uint32 TextureIndex = 0;

    /// Index in tinygltf::Model::images.
    int ImageIndex = -1;

    /// Index in tinygltf::Model::samplers.
    int SamplerIndex = -1;

    /// Resolved external image URI. Empty for embedded image data.
    std::string URI;

    /// Pointer to encoded image data for embedded buffer-view or data URI images.
    const void* pData = nullptr;

    /// Size of encoded image data in bytes.
    Uint64 DataSize = 0;
};

/// Parsed GLTF document.
class Document
{
public:
    explicit Document(const DocumentLoadInfo& LoadInfo);
    ~Document();

    // clang-format off
    Document           (const Document&) = delete;
    Document& operator=(const Document&) = delete;
    // clang-format on

    const tinygltf::Model& GetModel() const noexcept;
    const std::string&     GetBaseDir() const noexcept;

    /// Returns the number of textures in the document.
    Uint32 GetTextureCount() const;

    /// Returns the number of materials in the document.
    Uint32 GetMaterialCount() const;

    /// Resolves a GLTF texture to either an external URI or an embedded encoded-data span.
    ///
    /// Embedded buffer-view spans are owned by the document buffers. Embedded data URI
    /// spans are owned by tinygltf::Image::image. In both cases the returned pointer
    /// remains valid only while the document is alive and unchanged.
    bool GetTextureSourceInfo(Uint32 TextureIndex, TextureSourceInfo& Source) const;

private:
    std::string m_FileName;
    std::string m_BaseDir;

    std::vector<RefCntAutoPtr<IObject>> m_TexturesHold;
    std::unique_ptr<tinygltf::Model>    m_pModel;
};

int GetTextureImageIndex(const tinygltf::Model&   GltfModel,
                         const tinygltf::Texture& GltfTexture);

} // namespace GLTF

} // namespace Diligent
