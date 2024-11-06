/*
 *  Copyright 2019-2024 Diligent Graphics LLC
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

#include "Image.h"

DILIGENT_BEGIN_NAMESPACE(Diligent)

/// Loads an SGI image.

/// \param [in]  pSGIData    - SGI image data.
/// \param [in]  DataSize    - Size of the data.
/// \param [out] pDstPixels  - Destination pixels data blob. The pixels are always tightly packed
///                            (for instance, components of a 3-channel image will be written as |r|g|b|r|g|b|r|g|b|...).
/// \param [out] pDstImgDesc - Image description.
/// \return                    true if the image has been loaded successfully, and false otherwise.
///
/// \remarks    If pDstPixels is null, the function will only decode the image description and return true.
bool DILIGENT_GLOBAL_FUNCTION(LoadSGI)(const void* pSGIData,
                                       size_t      DataSize,
                                       IDataBlob*  pDstPixels,
                                       ImageDesc*  pDstImgDesc);

DILIGENT_END_NAMESPACE // namespace Diligent
