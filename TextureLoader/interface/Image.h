/*     Copyright 2015 Egor Yusov
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

#include "FileStream.h"
#include "DataBlob.h"
#include "RefCntAutoPtr.h"
#include "ObjectBase.h"

namespace Diligent
{
    enum class EImageFileFormat
    {
        unknown = 0,
        jpeg,
        png,
        tiff
    };

    struct ImageLoadInfo
    {
        EImageFileFormat Format;
        ImageLoadInfo() : 
            Format(EImageFileFormat::unknown)
        {}
    };

    struct ImageDesc
    {
        Diligent::Uint32 Width;
        Diligent::Uint32 Height;
        Diligent::Uint32 BitsPerPixel;
        Diligent::Uint32 NumComponents;
        Diligent::Uint32 RowStride; // In bytes
        ImageDesc() : 
            Width(0),
            Height(0),
            BitsPerPixel(0),
            NumComponents(0),
            RowStride(0)
        {}
    };

    class Image : public Diligent::ObjectBase<Diligent::IObject>
    {
    public:
        Image( Diligent::IFileStream *pSrcFile, 
               const ImageLoadInfo& LoadInfo );
    
        const ImageDesc &GetDesc(){ return m_Desc; }
        IDataBlob *GetData(){ return m_pData; }

    private:
        void LoadPngFile( IDataBlob *pFileData, const ImageLoadInfo& LoadInfo );
        void LoadTiffFile( IDataBlob *pFileData,const ImageLoadInfo& LoadInfo );
        void LoadJpegFile( IDataBlob *pFileData,const ImageLoadInfo& LoadInfo );
    
        ImageDesc m_Desc;
        RefCntAutoPtr<IDataBlob> m_pData;
    };
}
