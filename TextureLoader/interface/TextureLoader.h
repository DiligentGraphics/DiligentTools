/*     Copyright 2015-2017 Egor Yusov
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
#include "RenderDevice.h"
#include "Texture.h"
#include "Image.h"

namespace Diligent
{
    struct TextureLoadInfo
    {
        const Diligent::Char *Name;
        Diligent::USAGE Usage;
        Diligent::Uint32 BindFlags;
        Diligent::Uint32 MipLevels;
        Diligent::Uint32 CPUAccessFlags;
        Diligent::Bool IsSRGB;
        Diligent::Bool GenerateMips;
        Diligent::TEXTURE_FORMAT Format;

        TextureLoadInfo() :
            Name(""),
            Usage( Diligent::USAGE_STATIC ),
            BindFlags( Diligent::BIND_SHADER_RESOURCE ),
            MipLevels(0),
            CPUAccessFlags(0),
            IsSRGB(false),
            GenerateMips(true),
            Format(Diligent::TEX_FORMAT_UNKNOWN)
        {}
    };

    void CreateTextureFromImage( Image *pSrcImage,
                                 const TextureLoadInfo& TexLoadInfo, 
                                 Diligent::IRenderDevice *pDevice, 
                                 Diligent::ITexture **ppTexture );

    void CreateTextureFromDDS( IDataBlob *pDDSData,
                               const TextureLoadInfo& TexLoadInfo, 
                               Diligent::IRenderDevice *pDevice, 
                               Diligent::ITexture **ppTexture );
};
