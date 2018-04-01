/*     Copyright 2015-2016 Egor Yusov
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

/// \file
/// Defines texture utilities

#include "../../../DiligentCore/Graphics/GraphicsEngine/interface/GraphicsTypes.h"
#include "../../../DiligentCore/Graphics/GraphicsEngine/interface/Texture.h"
#include "../../../DiligentCore/Graphics/GraphicsEngine/interface/RenderDevice.h"
#include "TextureLoader.h"

namespace Diligent
{

/// Creates an image from file

/// \param [in] FilePath - Source file path
/// \param [out] ppImage - Memory location where pointer to the created image will be stored
/// \param [out] ppDDSData - If the file is a dds file, this will contain the pointer to the blob
///                          containing dds data. This parameter can be null.
void CreateImageFromFile( const Char *FilePath, 
                          Image **ppImage,
                          IDataBlob **ppDDSData = nullptr);


/// Creates a texture from file

/// \param [in] FilePath - Source file path
/// \param [in] TexLoadInfo - Texture loading information
/// \param [in] pDevice - Render device that will be used to create the texture
/// \param [out] ppTexture - Memory location where pointer to the created texture will be stored
void CreateTextureFromFile( const Char *FilePath, 
                            const TextureLoadInfo& TexLoadInfo, 
                            IRenderDevice *pDevice, 
                            ITexture **ppTexture );

}
