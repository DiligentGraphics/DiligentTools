/*     Copyright 2015-2018 Egor Yusov
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

void CreateImageFromFile( const Char *FilePath, 
                          Image **ppImage,
                          IDataBlob **ppDDSData)
{
    try
    {
        auto *pDotPos = strrchr(FilePath, '.');
        if (pDotPos == nullptr)
            LOG_ERROR_AND_THROW("File path \"", FilePath, "\" does not contain extension");

        auto *pExtension = pDotPos + 1;
        if (*pExtension == 0)
            LOG_ERROR_AND_THROW("File path \"", FilePath, "\" contains empty extension");

        String Extension = StrToLower(pExtension);
        RefCntAutoPtr<BasicFileStream> pFileStream(MakeNewRCObj<BasicFileStream>()(FilePath, EFileAccessMode::Read));
        if(!pFileStream->IsValid())
            LOG_ERROR_AND_THROW("Failed to open image file \"", FilePath, '\"');

        if (Extension == "dds")
        {
            VERIFY_EXPR(ppDDSData != nullptr);
            *ppDDSData = MakeNewRCObj<DataBlobImpl>()(0);
            pFileStream->Read(*ppDDSData);
            (*ppDDSData)->AddRef();
        }
        else
        {
            ImageLoadInfo ImgLoadInfo;
            if (Extension == "png")
                ImgLoadInfo.Format = EImageFileFormat::png;
            else if (Extension == "jpeg" || Extension == "jpg")
                ImgLoadInfo.Format = EImageFileFormat::jpeg;
            else if (Extension == "tiff" || Extension == "tif")
                ImgLoadInfo.Format = EImageFileFormat::tiff;
            else
                LOG_ERROR_AND_THROW("Unsupported file format ", Extension);

            RefCntAutoPtr<IDataBlob> pFileData(MakeNewRCObj<DataBlobImpl>()(0));
            pFileStream->Read(pFileData);

            Image::CreateFromDataBlob(pFileData, ImgLoadInfo, ppImage);
        }
    }
    catch (std::runtime_error &err)
    {
        LOG_ERROR("Failed to create image from file: ", err.what());
    }
}

void CreateTextureFromFile( const Char *FilePath, 
                            const TextureLoadInfo& TexLoadInfo, 
                            IRenderDevice *pDevice, 
                            ITexture **ppTexture )
{
    RefCntAutoPtr<Image> pImage;
    RefCntAutoPtr<IDataBlob> pDDSData;
    CreateImageFromFile( FilePath, &pImage, &pDDSData );

    if( pImage )
        CreateTextureFromImage( pImage, TexLoadInfo, pDevice, ppTexture );
    else if(pDDSData)
        CreateTextureFromDDS( pDDSData, TexLoadInfo, pDevice, ppTexture );
}

}
