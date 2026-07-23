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

#include "GLTFDocument.hpp"

#include <cstring>
#include <limits>
#include <mutex>
#include <shared_mutex>
#include <utility>

#include "FileSystem.hpp"
#include "FileWrapper.hpp"
#include "GLTFResourceManager.hpp"
#include "GraphicsAccessories.hpp"
#include "Image.h"
#include "StringTools.hpp"
#include "Texture.h"
#include "TextureUtilities.h"

#define TINYGLTF_IMPLEMENTATION
#define TINYGLTF_NO_STB_IMAGE
#define TINYGLTF_NO_STB_IMAGE_WRITE

#if defined(_MSC_VER) && defined(TINYGLTF_ENABLE_DRACO)
#    pragma warning(disable : 4127) // warning C4127: conditional expression is constant
#endif
#include "../../ThirdParty/tinygltf/tiny_gltf.h"

namespace Diligent
{

namespace GLTF
{

namespace
{

std::string DecodeURI(const std::string& URI)
{
    std::string DecodedURI;
    if (tinygltf::URIDecode(URI, &DecodedURI, nullptr))
        return DecodedURI;

    return URI;
}

std::string GetImagePath(const std::string& BaseDir, const std::string& URI)
{
    return !URI.empty() ? FileSystem::SimplifyPath((BaseDir + DecodeURI(URI)).c_str()) : "";
}

} // namespace

namespace MSFTTextureDDS
{

constexpr const char* MSFTTextureDDSExtension = "MSFT_texture_dds";

bool IsValidImageSource(const tinygltf::Model& gltf_model, int Source)
{
    return Source >= 0 && Source < static_cast<int>(gltf_model.images.size());
}

bool IsDDSImage(const tinygltf::Image& gltf_image)
{
    return (gltf_image.width < 0 && gltf_image.height < 0 &&
            static_cast<IMAGE_FILE_FORMAT>(gltf_image.pixel_type) == IMAGE_FILE_FORMAT_DDS &&
            !gltf_image.image.empty());
}

int GetSource(const tinygltf::Texture& gltf_tex,
              const tinygltf::Model&   gltf_model)
{
    const auto ext_it = gltf_tex.extensions.find(MSFTTextureDDSExtension);
    if (ext_it == gltf_tex.extensions.end() || !ext_it->second.IsObject())
        return -1;

    const tinygltf::Value& SourceValue = ext_it->second.Get("source");
    if (!SourceValue.IsInt())
        return -1;

    const int DDSSource = SourceValue.GetNumberAsInt();
    if (!IsValidImageSource(gltf_model, DDSSource))
        return -1;

    const tinygltf::Image& DDSImage = gltf_model.images[DDSSource];
    if (!DDSImage.image.empty() && !IsDDSImage(DDSImage))
        return -1;

    return DDSSource;
}

} // namespace MSFTTextureDDS

int GetTextureImageIndex(const tinygltf::Model&   gltf_model,
                         const tinygltf::Texture& gltf_tex)
{
    const int DDSSource = MSFTTextureDDS::GetSource(gltf_tex, gltf_model);
    return DDSSource >= 0 ? DDSSource : gltf_tex.source;
}

Uint32 Document::GetTextureCount() const
{
    const tinygltf::Model& gltf_model = GetModel();
    DEV_CHECK_ERR(gltf_model.textures.size() <= (std::numeric_limits<Uint32>::max)(),
                  "Too many textures in GLTF document");
    return static_cast<Uint32>(gltf_model.textures.size());
}

Uint32 Document::GetMaterialCount() const
{
    const tinygltf::Model& gltf_model = GetModel();
    DEV_CHECK_ERR(gltf_model.materials.size() <= (std::numeric_limits<Uint32>::max)(),
                  "Too many materials in GLTF document");
    return static_cast<Uint32>(gltf_model.materials.size());
}

bool Document::GetTextureSourceInfo(Uint32 TextureIndex, TextureSourceInfo& Source) const
{
    const tinygltf::Model& gltf_model = GetModel();
    if (TextureIndex >= gltf_model.textures.size())
        return false;

    const tinygltf::Texture& gltf_tex = gltf_model.textures[TextureIndex];
    const int                ImageIdx = GetTextureImageIndex(gltf_model, gltf_tex);
    if (ImageIdx < 0 || ImageIdx >= static_cast<int>(gltf_model.images.size()))
        return false;

    const tinygltf::Image& gltf_image = gltf_model.images[ImageIdx];

    Source              = {};
    Source.TextureIndex = TextureIndex;
    Source.ImageIndex   = ImageIdx;
    Source.SamplerIndex = gltf_tex.sampler;

    if (gltf_image.bufferView >= 0)
    {
        if (gltf_image.bufferView >= static_cast<int>(gltf_model.bufferViews.size()))
            return false;

        const tinygltf::BufferView& BufferView = gltf_model.bufferViews[static_cast<size_t>(gltf_image.bufferView)];
        if (BufferView.buffer < 0 || static_cast<size_t>(BufferView.buffer) >= gltf_model.buffers.size())
            return false;

        const tinygltf::Buffer& Buffer = gltf_model.buffers[static_cast<size_t>(BufferView.buffer)];
        if (BufferView.byteOffset > Buffer.data.size() ||
            BufferView.byteLength > Buffer.data.size() - BufferView.byteOffset)
        {
            return false;
        }

        Source.pData    = Buffer.data.data() + BufferView.byteOffset;
        Source.DataSize = static_cast<Uint64>(BufferView.byteLength);
        return true;
    }

    if (!gltf_image.image.empty())
    {
        Source.pData    = gltf_image.image.data();
        Source.DataSize = static_cast<Uint64>(gltf_image.image.size());
        return true;
    }

    if (!gltf_image.uri.empty())
    {
        Source.URI = GetImagePath(GetBaseDir(), gltf_image.uri);
        return !Source.URI.empty();
    }

    return false;
}

namespace Callbacks
{

namespace
{

struct LoaderData
{
    TextureCacheType* const pTextureCache;
    ResourceManager* const  pResourceMgr;

    std::vector<RefCntAutoPtr<IObject>> TexturesHold = {};

    std::string BaseDir      = {};
    bool        DecodeImages = true;

    DocumentLoadInfo::FileExistsCallbackType    FileExists    = nullptr;
    DocumentLoadInfo::ReadWholeFileCallbackType ReadWholeFile = nullptr;
};

bool IsImageFilePath(const std::string& FilePath)
{
    const size_t DotPos = FilePath.find_last_of('.');
    if (DotPos == std::string::npos || DotPos + 1 >= FilePath.length())
        return false;

    const char* Ext = &FilePath[DotPos + 1];

    static constexpr const char* ImageExtensions[] =
        {
            "png",
            "jpg",
            "jpeg",
            "dds",
            "ktx",
            "ktx2",
            "bmp",
            "gif",
            "tga",
            "hdr",
            "pic",
            "pnm",
            "ppm",
            "pgm",
        };
    for (const char* ImageExt : ImageExtensions)
    {
        if (StrCmpNoCase(Ext, ImageExt) == 0)
            return true;
    }

    return false;
}

bool LoadImageData(tinygltf::Image*     gltf_image,
                   const int            gltf_image_idx,
                   std::string*         error,
                   std::string*         warning,
                   int                  req_width,
                   int                  req_height,
                   const unsigned char* image_data,
                   int                  size,
                   void*                user_data)
{
    (void)warning;

    LoaderData* pLoaderData = static_cast<LoaderData*>(user_data);
    if (pLoaderData != nullptr)
    {
        const auto CacheId = GetImagePath(pLoaderData->BaseDir, gltf_image->uri);

        if (pLoaderData->pResourceMgr != nullptr)
        {
            if (RefCntAutoPtr<ITextureAtlasSuballocation> pAllocation = pLoaderData->pResourceMgr->FindTextureAllocation(CacheId.c_str()))
            {
                const TextureDesc           TexDesc    = pAllocation->GetAtlas()->GetAtlasDesc();
                const TextureFormatAttribs& FmtAttribs = GetTextureFormatAttribs(TexDesc.Format);
                const uint2                 Size       = pAllocation->GetSize();

                gltf_image->width      = Size.x;
                gltf_image->height     = Size.y;
                gltf_image->component  = FmtAttribs.NumComponents;
                gltf_image->bits       = FmtAttribs.ComponentSize * 8;
                gltf_image->pixel_type = TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE;

                // Keep strong reference to ensure the allocation is alive (second time, but that's fine).
                pLoaderData->TexturesHold.emplace_back(std::move(pAllocation));

                return true;
            }
        }
        else if (pLoaderData->pTextureCache != nullptr)
        {
            TextureCacheType& TexCache = *pLoaderData->pTextureCache;

            RefCntAutoPtr<ITexture> pTexture;
            bool                    TextureExpired = false;

            // Try with shared lock first
            {
                std::shared_lock<Threading::SharedMutex> SharedLock{TexCache.TexturesMtx};

                auto it = TexCache.Textures.find(CacheId);
                if (it != TexCache.Textures.end())
                {
                    pTexture = it->second.Lock();
                    if (!pTexture)
                    {
                        // Texture is stale
                        TextureExpired = true;
                    }
                }
            }

            if (TextureExpired)
            {
                // Upgrade to exclusive lock to remove stale texture
                std::unique_lock<Threading::SharedMutex> UniqueLock{TexCache.TexturesMtx};

                auto it = TexCache.Textures.find(CacheId);
                if (it != TexCache.Textures.end())
                {
                    pTexture = it->second.Lock();
                    if (!pTexture)
                    {
                        // Remove stale texture from the cache
                        TexCache.Textures.erase(it);
                    }
                }
            }

            if (pTexture)
            {
                const TextureDesc&          TexDesc    = pTexture->GetDesc();
                const TextureFormatAttribs& FmtAttribs = GetTextureFormatAttribs(TexDesc.Format);

                gltf_image->width      = TexDesc.Width;
                gltf_image->height     = TexDesc.Height;
                gltf_image->component  = FmtAttribs.NumComponents;
                gltf_image->bits       = FmtAttribs.ComponentSize * 8;
                gltf_image->pixel_type = TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE;

                // Keep strong reference to ensure the texture is alive (second time, but that's fine).
                pLoaderData->TexturesHold.emplace_back(std::move(pTexture));

                return true;
            }
        }
    }

    VERIFY(size != 1, "The texture was previously cached, but was not found in the cache now");

    ImageLoadInfo LoadInfo;
    LoadInfo.Format = Image::GetFileFormat(image_data, size);
    if (LoadInfo.Format == IMAGE_FILE_FORMAT_UNKNOWN)
    {
        if (error != nullptr)
        {
            *error += FormatString("Unknown format for image[", gltf_image_idx, "] name = '", gltf_image->name, "'");
        }
        return false;
    }

    if (LoadInfo.Format == IMAGE_FILE_FORMAT_DDS || LoadInfo.Format == IMAGE_FILE_FORMAT_KTX)
    {
        // Store binary data directly
        gltf_image->image.resize(size);
        std::memcpy(gltf_image->image.data(), image_data, size);
        // Use pixel_type field to indicate the file format
        gltf_image->pixel_type = LoadInfo.Format;
    }
    else
    {
        RefCntAutoPtr<Image> pImage;
        Image::CreateFromMemory(image_data, size, LoadInfo, &pImage);
        if (!pImage)
        {
            if (error != nullptr)
            {
                *error += FormatString("Failed to load image[", gltf_image_idx, "] name = '", gltf_image->name, "'");
            }
            return false;
        }
        const ImageDesc& ImgDesc = pImage->GetDesc();

        if (req_width > 0)
        {
            if (static_cast<Uint32>(req_width) != ImgDesc.Width)
            {
                if (error != nullptr)
                {
                    (*error) += FormatString("Image width mismatch for image[",
                                             gltf_image_idx, "] name = '", gltf_image->name,
                                             "': requested width: ",
                                             req_width, ", actual width: ",
                                             ImgDesc.Width);
                }
                return false;
            }
        }

        if (req_height > 0)
        {
            if (static_cast<Uint32>(req_height) != ImgDesc.Height)
            {
                if (error != nullptr)
                {
                    (*error) += FormatString("Image height mismatch for image[",
                                             gltf_image_idx, "] name = '", gltf_image->name,
                                             "': requested height: ",
                                             req_height, ", actual height: ",
                                             ImgDesc.Height);
                }
                return false;
            }
        }

        gltf_image->width      = ImgDesc.Width;
        gltf_image->height     = ImgDesc.Height;
        gltf_image->component  = 4;
        gltf_image->bits       = GetValueSize(ImgDesc.ComponentType) * 8;
        gltf_image->pixel_type = TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE;
        size_t DstRowSize      = static_cast<size_t>(gltf_image->width) * gltf_image->component * (gltf_image->bits / 8);
        gltf_image->image.resize(static_cast<size_t>(gltf_image->height) * DstRowSize);

        CopyPixelsAttribs CopyAttribs;
        CopyAttribs.Width            = ImgDesc.Width;
        CopyAttribs.Height           = ImgDesc.Height;
        CopyAttribs.SrcComponentSize = gltf_image->bits / 8;
        CopyAttribs.pSrcPixels       = pImage->GetData()->GetDataPtr();
        CopyAttribs.SrcStride        = ImgDesc.RowStride;
        CopyAttribs.SrcCompCount     = ImgDesc.NumComponents;
        CopyAttribs.pDstPixels       = gltf_image->image.data();
        CopyAttribs.DstComponentSize = gltf_image->bits / 8;
        CopyAttribs.DstStride        = static_cast<Uint32>(DstRowSize);
        CopyAttribs.DstCompCount     = gltf_image->component;
        if (CopyAttribs.SrcCompCount < 4)
        {
            // Always set alpha to 1
            CopyAttribs.Swizzle.A = TEXTURE_COMPONENT_SWIZZLE_ONE;
            if (CopyAttribs.SrcCompCount == 1)
            {
                // Expand R to RGB
                CopyAttribs.Swizzle.R = TEXTURE_COMPONENT_SWIZZLE_R;
                CopyAttribs.Swizzle.G = TEXTURE_COMPONENT_SWIZZLE_R;
                CopyAttribs.Swizzle.B = TEXTURE_COMPONENT_SWIZZLE_R;
            }
            else if (CopyAttribs.SrcCompCount == 2)
            {
                // RG -> RG01
                CopyAttribs.Swizzle.B = TEXTURE_COMPONENT_SWIZZLE_ZERO;
            }
            else
            {
                VERIFY(CopyAttribs.SrcCompCount == 3, "Unexpected number of components");
            }
        }
        CopyPixels(CopyAttribs);
    }

    return true;
}

bool LoadImageDataNoDecode(tinygltf::Image*     gltf_image,
                           const int            gltf_image_idx,
                           std::string*         error,
                           std::string*         warning,
                           int                  req_width,
                           int                  req_height,
                           const unsigned char* image_data,
                           int                  size,
                           void*                user_data)
{
    (void)warning;
    (void)req_width;
    (void)req_height;
    (void)user_data;

    if (gltf_image == nullptr)
        return false;

    gltf_image->image.clear();

    if (gltf_image->bufferView >= 0)
    {
        // Embedded buffer-view image data is owned by tinygltf::Model::buffers.
        // Keep the image as a view into that data and avoid duplicating it in image.image.
        return true;
    }

    // At this point, a non-empty URI means an external image file. Leave it
    // as URI-only metadata so higher-level loaders can fetch and decode it.
    if (!gltf_image->uri.empty())
        return true;

    if (image_data == nullptr || size <= 0)
    {
        if (error != nullptr)
            *error += FormatString("Missing encoded image data for image[", gltf_image_idx, "] name = '", gltf_image->name, "'");
        return false;
    }

    // The remaining no-decode path is an embedded data URI. TinyGLTF supplies
    // these encoded image bytes through temporary callback storage, so keep a copy.
    gltf_image->image.assign(image_data, image_data + size);

    return true;
}

bool FileExists(const std::string& abs_filename, void* user_data)
{
    // FileSystem::FileExists() is a pretty slow function.
    // Try to find the file in the cache first to avoid calling it.
    if (LoaderData* pLoaderData = static_cast<LoaderData*>(user_data))
    {
        const std::string CacheId = FileSystem::SimplifyPath(abs_filename.c_str());

        if (!pLoaderData->DecodeImages && IsImageFilePath(CacheId))
            return true;

        if (pLoaderData->pResourceMgr != nullptr)
        {
            if (pLoaderData->pResourceMgr->FindTextureAllocation(CacheId.c_str()) != nullptr)
                return true;
        }
        else if (pLoaderData->pTextureCache != nullptr)
        {
            std::shared_lock<Threading::SharedMutex> SharedLock{pLoaderData->pTextureCache->TexturesMtx};

            auto it = pLoaderData->pTextureCache->Textures.find(CacheId);
            if (it != pLoaderData->pTextureCache->Textures.end())
                return true;
        }

        if (pLoaderData->FileExists)
            return pLoaderData->FileExists(abs_filename.c_str());
    }

    return FileSystem::FileExists(abs_filename.c_str());
}

bool ReadWholeFile(std::vector<unsigned char>* out,
                   std::string*                err,
                   const std::string&          filepath,
                   void*                       user_data)
{
    VERIFY_EXPR(out != nullptr);
    VERIFY_EXPR(err != nullptr);

    // Try to find the file in the texture cache to avoid reading it
    if (LoaderData* pLoaderData = static_cast<LoaderData*>(user_data))
    {
        const std::string CacheId = FileSystem::SimplifyPath(filepath.c_str());

        if (!pLoaderData->DecodeImages && IsImageFilePath(CacheId))
        {
            // TinyGLTF requires non-empty data before it calls the image
            // loader. The no-decode image callback will leave URI images empty.
            out->assign(1, 0);
            return true;
        }

        if (pLoaderData->pResourceMgr != nullptr)
        {
            if (RefCntAutoPtr<ITextureAtlasSuballocation> pAllocation = pLoaderData->pResourceMgr->FindTextureAllocation(CacheId.c_str()))
            {
                // Keep strong reference to ensure the allocation is alive.
                pLoaderData->TexturesHold.emplace_back(std::move(pAllocation));
                // Tiny GLTF checks the size of 'out', it can't be empty
                out->resize(1);
                return true;
            }
        }
        else if (pLoaderData->pTextureCache != nullptr)
        {
            std::shared_lock<Threading::SharedMutex> SharedLock{pLoaderData->pTextureCache->TexturesMtx};

            auto it = pLoaderData->pTextureCache->Textures.find(CacheId);
            if (it != pLoaderData->pTextureCache->Textures.end())
            {
                if (RefCntAutoPtr<ITexture> pTexture = it->second.Lock())
                {
                    // Keep strong reference to ensure the texture is alive.
                    pLoaderData->TexturesHold.emplace_back(std::move(pTexture));
                    // Tiny GLTF checks the size of 'out', it can't be empty
                    out->resize(1);
                    return true;
                }
            }
        }

        if (pLoaderData->ReadWholeFile)
            return pLoaderData->ReadWholeFile(filepath.c_str(), *out, *err);
    }

    FileWrapper pFile{filepath.c_str(), EFileAccessMode::Read};
    if (!pFile)
    {
        if (err)
        {
            (*err) += FormatString("Unable to open file ", filepath, "\n");
        }
        return false;
    }

    size_t size = pFile->GetSize();
    if (size == 0)
    {
        if (err)
        {
            (*err) += FormatString("File is empty: ", filepath, "\n");
        }
        return false;
    }

    out->resize(size);
    pFile->Read(out->data(), size);

    return true;
}

} // namespace

} // namespace Callbacks

Document::Document(const DocumentLoadInfo& LoadInfo) :
    m_pModel{std::make_unique<tinygltf::Model>()}
{
    if (LoadInfo.FileName == nullptr || *LoadInfo.FileName == 0)
        LOG_ERROR_AND_THROW("File path must not be empty");

    m_FileName = LoadInfo.FileName;
    if (m_FileName.find_last_of("/\\") != std::string::npos)
        m_BaseDir = m_FileName.substr(0, m_FileName.find_last_of("/\\"));
    m_BaseDir += '/';

    if (LoadInfo.pTextureCache != nullptr && LoadInfo.pResourceManager != nullptr)
        LOG_WARNING_MESSAGE("Texture cache is ignored when resource manager is used");

    Callbacks::LoaderData LoaderData{LoadInfo.pTextureCache, LoadInfo.pResourceManager};
    LoaderData.BaseDir       = m_BaseDir;
    LoaderData.DecodeImages  = LoadInfo.DecodeImages;
    LoaderData.FileExists    = LoadInfo.FileExistsCallback;
    LoaderData.ReadWholeFile = LoadInfo.ReadWholeFileCallback;

    tinygltf::TinyGLTF gltf_context;
    gltf_context.SetImageLoader(LoadInfo.DecodeImages ? Callbacks::LoadImageData : Callbacks::LoadImageDataNoDecode, &LoaderData);
    tinygltf::FsCallbacks fsCallbacks = {};
    fsCallbacks.ExpandFilePath        = tinygltf::ExpandFilePath;
    fsCallbacks.FileExists            = Callbacks::FileExists;
    fsCallbacks.ReadWholeFile         = Callbacks::ReadWholeFile;
    fsCallbacks.WriteWholeFile        = tinygltf::WriteWholeFile;
    fsCallbacks.user_data             = &LoaderData;
    gltf_context.SetFsCallbacks(fsCallbacks);

    bool   binary = false;
    size_t extpos = m_FileName.rfind('.', m_FileName.length());
    if (extpos != std::string::npos)
    {
        binary = (m_FileName.substr(extpos + 1, m_FileName.length() - extpos) == "glb");
    }

    std::string error;
    std::string warning;

    bool fileLoaded = false;
    if (binary)
        fileLoaded = gltf_context.LoadBinaryFromFile(m_pModel.get(), &error, &warning, m_FileName.c_str());
    else
        fileLoaded = gltf_context.LoadASCIIFromFile(m_pModel.get(), &error, &warning, m_FileName.c_str());
    if (!fileLoaded)
    {
        LOG_ERROR_AND_THROW("Failed to load gltf file ", m_FileName, ": ", error);
    }
    if (!warning.empty())
    {
        LOG_WARNING_MESSAGE("Loaded gltf file ", m_FileName, " with the following warning:", warning);
    }

    m_TexturesHold = std::move(LoaderData.TexturesHold);
}

Document::~Document() = default;

const tinygltf::Model& Document::GetModel() const noexcept
{
    VERIFY_EXPR(m_pModel != nullptr);
    return *m_pModel;
}

const std::string& Document::GetBaseDir() const noexcept
{
    return m_BaseDir;
}

} // namespace GLTF

} // namespace Diligent
