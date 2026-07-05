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

#include "GLTFLoader.hpp"
#include "../../../ThirdParty/tinygltf/tiny_gltf.h"

#include "gtest/gtest.h"

#include "Image.h"

#include <string>
#include <unordered_map>
#include <vector>

namespace Diligent
{

namespace GLTF
{

namespace MSFTTextureDDS
{

int GetSource(const tinygltf::Texture& gltf_tex,
              const tinygltf::Model&   gltf_model);

} // namespace MSFTTextureDDS

} // namespace GLTF

} // namespace Diligent

using namespace Diligent;

namespace
{

std::vector<unsigned char> MakeBytes(const char* Str)
{
    return std::vector<unsigned char>{Str, Str + std::char_traits<char>::length(Str)};
}

bool HasSuffix(const std::string& Str, const char* Suffix)
{
    const size_t SuffixLen = std::char_traits<char>::length(Suffix);
    return Str.size() >= SuffixLen &&
        Str.compare(Str.size() - SuffixLen, SuffixLen, Suffix) == 0;
}

struct InMemoryGLTFFiles
{
    std::unordered_map<std::string, std::vector<unsigned char>> Files;
    std::vector<std::string>                                    FileExistsRequests;
    std::vector<std::string>                                    ReadRequests;

    bool FileExists(const char* FilePath)
    {
        FileExistsRequests.emplace_back(FilePath);
        return FindFile(FilePath) != nullptr;
    }

    bool ReadWholeFile(const char* FilePath, std::vector<unsigned char>& Data, std::string& Error)
    {
        ReadRequests.emplace_back(FilePath);

        if (const std::vector<unsigned char>* pData = FindFile(FilePath))
        {
            Data = *pData;
            return true;
        }

        Error = std::string{"Missing test file: "} + FilePath;
        return false;
    }

    bool WasRead(const char* Suffix) const
    {
        for (const std::string& Request : ReadRequests)
        {
            if (HasSuffix(Request, Suffix))
                return true;
        }
        return false;
    }

private:
    const std::vector<unsigned char>* FindFile(const char* FilePath) const
    {
        const std::string Path{FilePath};
        for (const auto& File : Files)
        {
            if (Path == File.first || HasSuffix(Path, File.first.c_str()))
                return &File.second;
        }

        return nullptr;
    }
};

tinygltf::Texture CreateDDSTexture(int Source)
{
    tinygltf::Value::Object Extension;
    Extension.emplace("source", tinygltf::Value{Source});

    tinygltf::Texture Texture;
    Texture.source = 0;
    Texture.extensions.emplace("MSFT_texture_dds", tinygltf::Value{std::move(Extension)});
    return Texture;
}

TEST(Tools_GLTFLoader, MSFTTextureDDSUsesRawDDSImageData)
{
    tinygltf::Image DDSImage;
    DDSImage.uri        = "texture.dds";
    DDSImage.pixel_type = IMAGE_FILE_FORMAT_DDS;
    DDSImage.image      = {'D', 'D', 'S', ' '};

    tinygltf::Model Model;
    Model.images.emplace_back(tinygltf::Image{});
    Model.images.emplace_back(std::move(DDSImage));

    const tinygltf::Texture Texture = CreateDDSTexture(1);

    EXPECT_EQ(GLTF::MSFTTextureDDS::GetSource(Texture, Model), 1);
}

TEST(Tools_GLTFLoader, MSFTTextureDDSRejectsLoadedImageMetadata)
{
    tinygltf::Image DDSImage;
    DDSImage.uri        = "cached.dds";
    DDSImage.width      = 4;
    DDSImage.height     = 4;
    DDSImage.component  = 4;
    DDSImage.bits       = 8;
    DDSImage.pixel_type = TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE;

    tinygltf::Model Model;
    Model.images.emplace_back(tinygltf::Image{});
    Model.images.emplace_back(std::move(DDSImage));

    const tinygltf::Texture Texture = CreateDDSTexture(1);

    EXPECT_EQ(GLTF::MSFTTextureDDS::GetSource(Texture, Model), -1);
}

TEST(Tools_GLTFLoader, MSFTTextureDDSRejectsUriOnlyImage)
{
    tinygltf::Image DDSImage;
    DDSImage.uri = "encoded%20texture.dds";

    tinygltf::Model Model;
    Model.images.emplace_back(tinygltf::Image{});
    Model.images.emplace_back(std::move(DDSImage));

    const tinygltf::Texture Texture = CreateDDSTexture(1);

    EXPECT_EQ(GLTF::MSFTTextureDDS::GetSource(Texture, Model), -1);
}

TEST(Tools_GLTFLoader, DocumentKeepsEmbeddedImageDataInBufferWhenDecodeImagesIsFalse)
{
    InMemoryGLTFFiles Files;
    Files.Files.emplace(
        "embedded.gltf",
        MakeBytes(R"({
            "asset": {"version": "2.0"},
            "buffers": [
                {
                    "uri": "data:application/octet-stream;base64,RERTIA==",
                    "byteLength": 4
                }
            ],
            "bufferViews": [
                {
                    "buffer": 0,
                    "byteOffset": 0,
                    "byteLength": 4
                }
            ],
            "images": [
                {
                    "name": "EmbeddedDDS",
                    "bufferView": 0,
                    "mimeType": "application/octet-stream"
                }
            ],
            "textures": [{"source": 0}]
        })"));

    GLTF::DocumentLoadInfo LoadInfo;
    LoadInfo.FileName           = "embedded.gltf";
    LoadInfo.DecodeImages       = false;
    LoadInfo.FileExistsCallback = [&Files](const char* FilePath) //
    {
        return Files.FileExists(FilePath);
    };
    LoadInfo.ReadWholeFileCallback = [&Files](const char* FilePath, std::vector<unsigned char>& Data, std::string& Error) //
    {
        return Files.ReadWholeFile(FilePath, Data, Error);
    };

    GLTF::Document Document{LoadInfo};

    const tinygltf::Model& Model = Document.GetModel();
    ASSERT_EQ(Model.images.size(), 1u);
    ASSERT_EQ(Model.buffers.size(), 1u);
    ASSERT_EQ(Model.bufferViews.size(), 1u);

    const tinygltf::Image& Image = Model.images[0];
    EXPECT_EQ(Image.name, "EmbeddedDDS");
    EXPECT_EQ(Image.bufferView, 0);
    EXPECT_TRUE(Image.image.empty());

    const tinygltf::BufferView& BufferView = Model.bufferViews[Image.bufferView];
    ASSERT_EQ(BufferView.buffer, 0);
    ASSERT_EQ(BufferView.byteLength, 4u);

    const std::vector<unsigned char>& BufferData = Model.buffers[BufferView.buffer].data;
    ASSERT_GE(BufferData.size(), BufferView.byteOffset + BufferView.byteLength);
    EXPECT_EQ(std::vector<unsigned char>(BufferData.begin() + BufferView.byteOffset,
                                         BufferData.begin() + BufferView.byteOffset + BufferView.byteLength),
              (std::vector<unsigned char>{'D', 'D', 'S', ' '}));
}

TEST(Tools_GLTFLoader, DocumentKeepsExternalImageUriWhenDecodeImagesIsFalse)
{
    InMemoryGLTFFiles Files;
    Files.Files.emplace(
        "external.gltf",
        MakeBytes(R"({
            "asset": {"version": "2.0"},
            "images": [
                {
                    "name": "ExternalTexture",
                    "uri": "external.png"
                }
            ],
            "textures": [{"source": 0}]
        })"));

    GLTF::DocumentLoadInfo LoadInfo;
    LoadInfo.FileName           = "external.gltf";
    LoadInfo.DecodeImages       = false;
    LoadInfo.FileExistsCallback = [&Files](const char* FilePath) //
    {
        return Files.FileExists(FilePath);
    };
    LoadInfo.ReadWholeFileCallback = [&Files](const char* FilePath, std::vector<unsigned char>& Data, std::string& Error) //
    {
        return Files.ReadWholeFile(FilePath, Data, Error);
    };

    GLTF::Document Document{LoadInfo};

    const tinygltf::Model& Model = Document.GetModel();
    ASSERT_EQ(Model.images.size(), 1u);

    const tinygltf::Image& Image = Model.images[0];
    EXPECT_EQ(Image.name, "ExternalTexture");
    EXPECT_EQ(Image.uri, "external.png");
    EXPECT_TRUE(Image.image.empty());
    EXPECT_EQ(Image.width, -1);
    EXPECT_EQ(Image.height, -1);
    EXPECT_FALSE(Files.WasRead("external.png"));
}

TEST(Tools_GLTFLoader, DocumentCopiesDataUriImageBytesWhenDecodeImagesIsFalse)
{
    InMemoryGLTFFiles Files;
    Files.Files.emplace(
        "data_uri.gltf",
        MakeBytes(R"({
            "asset": {"version": "2.0"},
            "images": [
                {
                    "name": "DataUriTexture",
                    "uri": "data:image/png;base64,iVBORw0KGgo="
                }
            ],
            "textures": [{"source": 0}]
        })"));

    GLTF::DocumentLoadInfo LoadInfo;
    LoadInfo.FileName           = "data_uri.gltf";
    LoadInfo.DecodeImages       = false;
    LoadInfo.FileExistsCallback = [&Files](const char* FilePath) //
    {
        return Files.FileExists(FilePath);
    };
    LoadInfo.ReadWholeFileCallback = [&Files](const char* FilePath, std::vector<unsigned char>& Data, std::string& Error) //
    {
        return Files.ReadWholeFile(FilePath, Data, Error);
    };

    GLTF::Document Document{LoadInfo};

    const tinygltf::Model& Model = Document.GetModel();
    ASSERT_EQ(Model.images.size(), 1u);

    const tinygltf::Image& Image = Model.images[0];
    EXPECT_EQ(Image.name, "DataUriTexture");
    EXPECT_EQ(Image.bufferView, -1);
    EXPECT_TRUE(Image.uri.empty());
    EXPECT_EQ(Image.image,
              (std::vector<unsigned char>{0x89u, 'P', 'N', 'G', '\r', '\n', 0x1Au, '\n'}));
}

} // namespace
