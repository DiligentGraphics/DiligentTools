/*     Copyright 2015-2019 Egor Yusov
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

#include "pch.h"
#include "TextureUtilities.h"
#include "Errors.h"
#include "TextureLoader.h"
#include "Image.h"
#include "BasicFileStream.h"
#include "RefCntAutoPtr.h"
#include "DataBlobImpl.h"
#include "StringTools.h"

using namespace Diligent;

namespace Diligent
{

EImageFileFormat CreateImageFromFile(const Char*  FilePath, 
                                     Image**      ppImage,
                                     IDataBlob**  ppRawData)
{
    EImageFileFormat ImgFileFormat = EImageFileFormat::unknown;
    try
    {
        RefCntAutoPtr<BasicFileStream> pFileStream(MakeNewRCObj<BasicFileStream>()(FilePath, EFileAccessMode::Read));
        if(!pFileStream->IsValid())
            LOG_ERROR_AND_THROW("Failed to open image file \"", FilePath, '\"');

        RefCntAutoPtr<IDataBlob> pFileData(MakeNewRCObj<DataBlobImpl>()(0));
        pFileStream->Read(pFileData);

        ImgFileFormat = Image::GetFileFormat(reinterpret_cast<Uint8*>(pFileData->GetDataPtr()), pFileData->GetSize());
        if (ImgFileFormat == EImageFileFormat::unknown)
        {
            LOG_WARNING_MESSAGE("Unable to derive image format from the header for file \"", FilePath, "\". Trying to analyze extension.");

            // Try to use extension to derive format
            auto *pDotPos = strrchr(FilePath, '.');
            if (pDotPos == nullptr)
                LOG_ERROR_AND_THROW("Unable to recognize file format: file name \"", FilePath, "\" does not contain extension");

            auto *pExtension = pDotPos + 1;
            if (*pExtension == 0)
                LOG_ERROR_AND_THROW("Unable to recognize file format: file name \"", FilePath, "\" contain empty extension");

            String Extension = StrToLower(pExtension);
            if (Extension == "png")
                ImgFileFormat = EImageFileFormat::png;
            else if (Extension == "jpeg" || Extension == "jpg")
                ImgFileFormat = EImageFileFormat::jpeg;
            else if (Extension == "tiff" || Extension == "tif")
                ImgFileFormat = EImageFileFormat::tiff;
            else if (Extension == "dds")
                ImgFileFormat = EImageFileFormat::dds;
            else if (Extension == "ktx")
                ImgFileFormat = EImageFileFormat::ktx;
            else
                LOG_ERROR_AND_THROW("Unsupported file format ", Extension);
        }

        if (ImgFileFormat == EImageFileFormat::png  ||
            ImgFileFormat == EImageFileFormat::jpeg ||
            ImgFileFormat == EImageFileFormat::tiff)
        {
            ImageLoadInfo ImgLoadInfo;
            ImgLoadInfo.Format = ImgFileFormat;
            Image::CreateFromDataBlob(pFileData, ImgLoadInfo, ppImage);
        }
        else
        {
            *ppRawData = pFileData.Detach();
        }
    }
    catch (std::runtime_error &err)
    {
        LOG_ERROR("Failed to create image from file: ", err.what());
    }

    return ImgFileFormat;
}

void CreateTextureFromFile(const Char*             FilePath, 
                           const TextureLoadInfo&  TexLoadInfo, 
                           IRenderDevice*          pDevice, 
                           ITexture**              ppTexture )
{
    RefCntAutoPtr<Image>     pImage;
    RefCntAutoPtr<IDataBlob> pRawData;
    auto ImgFmt = CreateImageFromFile( FilePath, &pImage, &pRawData );

    if (pImage)
        CreateTextureFromImage( pImage, TexLoadInfo, pDevice, ppTexture );
    else if (pRawData)
    {
        if (ImgFmt == EImageFileFormat::dds)
            CreateTextureFromDDS( pRawData, TexLoadInfo, pDevice, ppTexture );
        else if (ImgFmt == EImageFileFormat::ktx)
            CreateTextureFromKTX( pRawData, TexLoadInfo, pDevice, ppTexture );
        else
            UNEXPECTED("Unexpected format");
    }
}

}
