/*
 *  Copyright 2019-2022 Diligent Graphics LLC
 *  Copyright 2015-2019 Egor Yusov
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

//--------------------------------------------------------------------------------------
// File: DDSLoader.cpp
//
// Functions for loading a DDS texture and creating a Direct3D 11 runtime resource for it
//
// Note these functions are useful as a light-weight runtime loader for DDS files. For
// a full-featured DDS file reader, writer, and texture processing pipeline see
// the 'Texconv' sample and the 'DirectXTex' library.
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//
// http://go.microsoft.com/fwlink/?LinkId=248926
// http://go.microsoft.com/fwlink/?LinkId=248929
//--------------------------------------------------------------------------------------

// clang-format off

#include "TextureLoaderImpl.hpp"
#include "FileWrapper.hpp"
#include "GraphicsAccessories.hpp"

#include "dxgiformat.h"

#include <memory>
#include <algorithm>
#include <array>

#ifndef _In_
#   define _In_
#endif

#ifndef _Out_
#   define _Out_
#endif

#ifndef _Out_opt_
#   define _Out_opt_
#endif

#ifndef _Outptr_opt_
#   define _Outptr_opt_
#endif

using namespace Diligent;

namespace
{

// D3D11 definitions

enum D3D11_RESOURCE_DIMENSION
{
    D3D11_RESOURCE_DIMENSION_UNKNOWN	= 0,
    D3D11_RESOURCE_DIMENSION_BUFFER	    = 1,
    D3D11_RESOURCE_DIMENSION_TEXTURE1D	= 2,
    D3D11_RESOURCE_DIMENSION_TEXTURE2D	= 3,
    D3D11_RESOURCE_DIMENSION_TEXTURE3D	= 4
};

enum D3D11_RESOURCE_MISC_FLAG
{
    D3D11_RESOURCE_MISC_TEXTURECUBE	= 0x4L,
};

#ifndef D3D11_REQ_MIP_LEVELS
#   define	D3D11_REQ_MIP_LEVELS	( 15 )
#endif

#ifndef D3D11_REQ_TEXTURE1D_ARRAY_AXIS_DIMENSION
#   define	D3D11_REQ_TEXTURE1D_ARRAY_AXIS_DIMENSION	( 2048 )
#endif

#ifndef D3D11_REQ_TEXTURE1D_U_DIMENSION
#   define	D3D11_REQ_TEXTURE1D_U_DIMENSION	( 16384 )
#endif

#ifndef D3D11_REQ_TEXTURE2D_ARRAY_AXIS_DIMENSION
#   define	D3D11_REQ_TEXTURE2D_ARRAY_AXIS_DIMENSION	( 2048 )
#endif

#ifndef D3D11_REQ_TEXTURE2D_U_OR_V_DIMENSION
#   define	D3D11_REQ_TEXTURE2D_U_OR_V_DIMENSION	( 16384 )
#endif

#ifndef D3D11_REQ_TEXTURE3D_U_V_OR_W_DIMENSION
#   define	D3D11_REQ_TEXTURE3D_U_V_OR_W_DIMENSION	( 2048 )
#endif

#ifndef D3D11_REQ_TEXTURECUBE_DIMENSION
#   define	D3D11_REQ_TEXTURECUBE_DIMENSION	( 16384 )
#endif


//--------------------------------------------------------------------------------------
// Macros
//--------------------------------------------------------------------------------------
#ifndef MAKEFOURCC
    #define MAKEFOURCC(ch0, ch1, ch2, ch3)                                      \
                ((Uint32)(Uint8)(ch0)        | ((Uint32)(Uint8)(ch1) << 8) |    \
                ((Uint32)(Uint8)(ch2) << 16) | ((Uint32)(Uint8)(ch3) << 24))
#endif /* defined(MAKEFOURCC) */

//--------------------------------------------------------------------------------------
// DDS file structure definitions
//
// See DDS.h in the 'Texconv' sample and the 'DirectXTex' library
//--------------------------------------------------------------------------------------
#pragma pack(push, 1)

#define DDS_MAGIC 0x20534444 // "DDS "

struct DDS_PIXELFORMAT
{
    Uint32  size;
    Uint32  flags;
    Uint32  fourCC;
    Uint32  RGBBitCount;
    Uint32  RBitMask;
    Uint32  GBitMask;
    Uint32  BBitMask;
    Uint32  ABitMask;
};

#define DDS_FOURCC      0x00000004  // DDPF_FOURCC
#define DDS_RGB         0x00000040  // DDPF_RGB
#define DDS_RGBA        0x00000041  // DDPF_RGB | DDPF_ALPHAPIXELS
#define DDS_LUMINANCE   0x00020000  // DDPF_LUMINANCE
#define DDS_LUMINANCEA  0x00020001  // DDPF_LUMINANCE | DDPF_ALPHAPIXELS
#define DDS_ALPHA       0x00000002  // DDPF_ALPHA
#define DDS_PAL8        0x00000020  // DDPF_PALETTEINDEXED8

#define DDS_HEADER_FLAGS_TEXTURE        0x00001007  // DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_PIXELFORMAT
#define DDS_HEADER_FLAGS_MIPMAP         0x00020000  // DDSD_MIPMAPCOUNT
#define DDS_HEADER_FLAGS_VOLUME         0x00800000  // DDSD_DEPTH
#define DDS_HEADER_FLAGS_PITCH          0x00000008  // DDSD_PITCH
#define DDS_HEADER_FLAGS_LINEARSIZE     0x00080000  // DDSD_LINEARSIZE

#define DDS_HEIGHT 0x00000002 // DDSD_HEIGHT
#define DDS_WIDTH  0x00000004 // DDSD_WIDTH

#define DDS_SURFACE_FLAGS_TEXTURE 0x00001000 // DDSCAPS_TEXTURE
#define DDS_SURFACE_FLAGS_MIPMAP  0x00400008 // DDSCAPS_COMPLEX | DDSCAPS_MIPMAP
#define DDS_SURFACE_FLAGS_CUBEMAP 0x00000008 // DDSCAPS_COMPLEX

#define DDS_CUBEMAP_POSITIVEX 0x00000600 // DDSCAPS2_CUBEMAP | DDSCAPS2_CUBEMAP_POSITIVEX
#define DDS_CUBEMAP_NEGATIVEX 0x00000a00 // DDSCAPS2_CUBEMAP | DDSCAPS2_CUBEMAP_NEGATIVEX
#define DDS_CUBEMAP_POSITIVEY 0x00001200 // DDSCAPS2_CUBEMAP | DDSCAPS2_CUBEMAP_POSITIVEY
#define DDS_CUBEMAP_NEGATIVEY 0x00002200 // DDSCAPS2_CUBEMAP | DDSCAPS2_CUBEMAP_NEGATIVEY
#define DDS_CUBEMAP_POSITIVEZ 0x00004200 // DDSCAPS2_CUBEMAP | DDSCAPS2_CUBEMAP_POSITIVEZ
#define DDS_CUBEMAP_NEGATIVEZ 0x00008200 // DDSCAPS2_CUBEMAP | DDSCAPS2_CUBEMAP_NEGATIVEZ

#define DDS_CUBEMAP_ALLFACES (DDS_CUBEMAP_POSITIVEX | DDS_CUBEMAP_NEGATIVEX |\
                              DDS_CUBEMAP_POSITIVEY | DDS_CUBEMAP_NEGATIVEY |\
                              DDS_CUBEMAP_POSITIVEZ | DDS_CUBEMAP_NEGATIVEZ)

#define DDS_CUBEMAP 0x00000200 // DDSCAPS2_CUBEMAP

#define DDS_FLAGS_VOLUME 0x00200000 // DDSCAPS2_VOLUME

enum DDS_MISC_FLAGS2
{
    DDS_MISC_FLAGS2_ALPHA_MODE_MASK = 0x7L,
};

enum DDS_ALPHA_MODE
{
    DDS_ALPHA_MODE_UNKNOWN       = 0,
    DDS_ALPHA_MODE_STRAIGHT      = 1,
    DDS_ALPHA_MODE_PREMULTIPLIED = 2,
    DDS_ALPHA_MODE_OPAQUE        = 3,
    DDS_ALPHA_MODE_CUSTOM        = 4,
};

typedef struct
{
    Uint32          size;
    Uint32          flags;
    Uint32          height;
    Uint32          width;
    Uint32          pitchOrLinearSize;
    Uint32          depth; // only if DDS_HEADER_FLAGS_VOLUME is set in flags
    Uint32          mipMapCount;
    Uint32          reserved1[11];
    DDS_PIXELFORMAT ddspf;
    Uint32          caps;
    Uint32          caps2;
    Uint32          caps3;
    Uint32          caps4;
    Uint32          reserved2;
} DDS_HEADER;


typedef struct
{
    DXGI_FORMAT dxgiFormat;
    Uint32      resourceDimension;
    Uint32      miscFlag; // see D3D11_RESOURCE_MISC_FLAG
    Uint32      arraySize;
    Uint32      miscFlags2;
} DDS_HEADER_DXT10;

#pragma pack(pop)



//--------------------------------------------------------------------------------------
// Return the BPP for a particular format
//--------------------------------------------------------------------------------------
static size_t BitsPerPixel(_In_ DXGI_FORMAT fmt)
{
    switch (fmt)
    {
    case DXGI_FORMAT_R32G32B32A32_TYPELESS:
    case DXGI_FORMAT_R32G32B32A32_FLOAT:
    case DXGI_FORMAT_R32G32B32A32_UINT:
    case DXGI_FORMAT_R32G32B32A32_SINT:
        return 128;

    case DXGI_FORMAT_R32G32B32_TYPELESS:
    case DXGI_FORMAT_R32G32B32_FLOAT:
    case DXGI_FORMAT_R32G32B32_UINT:
    case DXGI_FORMAT_R32G32B32_SINT:
        return 96;

    case DXGI_FORMAT_R16G16B16A16_TYPELESS:
    case DXGI_FORMAT_R16G16B16A16_FLOAT:
    case DXGI_FORMAT_R16G16B16A16_UNORM:
    case DXGI_FORMAT_R16G16B16A16_UINT:
    case DXGI_FORMAT_R16G16B16A16_SNORM:
    case DXGI_FORMAT_R16G16B16A16_SINT:
    case DXGI_FORMAT_R32G32_TYPELESS:
    case DXGI_FORMAT_R32G32_FLOAT:
    case DXGI_FORMAT_R32G32_UINT:
    case DXGI_FORMAT_R32G32_SINT:
    case DXGI_FORMAT_R32G8X24_TYPELESS:
    case DXGI_FORMAT_D32_FLOAT_S8X24_UINT:
    case DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS:
    case DXGI_FORMAT_X32_TYPELESS_G8X24_UINT:
        return 64;

    case DXGI_FORMAT_R10G10B10A2_TYPELESS:
    case DXGI_FORMAT_R10G10B10A2_UNORM:
    case DXGI_FORMAT_R10G10B10A2_UINT:
    case DXGI_FORMAT_R11G11B10_FLOAT:
    case DXGI_FORMAT_R8G8B8A8_TYPELESS:
    case DXGI_FORMAT_R8G8B8A8_UNORM:
    case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
    case DXGI_FORMAT_R8G8B8A8_UINT:
    case DXGI_FORMAT_R8G8B8A8_SNORM:
    case DXGI_FORMAT_R8G8B8A8_SINT:
    case DXGI_FORMAT_R16G16_TYPELESS:
    case DXGI_FORMAT_R16G16_FLOAT:
    case DXGI_FORMAT_R16G16_UNORM:
    case DXGI_FORMAT_R16G16_UINT:
    case DXGI_FORMAT_R16G16_SNORM:
    case DXGI_FORMAT_R16G16_SINT:
    case DXGI_FORMAT_R32_TYPELESS:
    case DXGI_FORMAT_D32_FLOAT:
    case DXGI_FORMAT_R32_FLOAT:
    case DXGI_FORMAT_R32_UINT:
    case DXGI_FORMAT_R32_SINT:
    case DXGI_FORMAT_R24G8_TYPELESS:
    case DXGI_FORMAT_D24_UNORM_S8_UINT:
    case DXGI_FORMAT_R24_UNORM_X8_TYPELESS:
    case DXGI_FORMAT_X24_TYPELESS_G8_UINT:
    case DXGI_FORMAT_R9G9B9E5_SHAREDEXP:
    case DXGI_FORMAT_R8G8_B8G8_UNORM:
    case DXGI_FORMAT_G8R8_G8B8_UNORM:
    case DXGI_FORMAT_B8G8R8A8_UNORM:
    case DXGI_FORMAT_B8G8R8X8_UNORM:
    case DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM:
    case DXGI_FORMAT_B8G8R8A8_TYPELESS:
    case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB:
    case DXGI_FORMAT_B8G8R8X8_TYPELESS:
    case DXGI_FORMAT_B8G8R8X8_UNORM_SRGB:
        return 32;

    case DXGI_FORMAT_R8G8_TYPELESS:
    case DXGI_FORMAT_R8G8_UNORM:
    case DXGI_FORMAT_R8G8_UINT:
    case DXGI_FORMAT_R8G8_SNORM:
    case DXGI_FORMAT_R8G8_SINT:
    case DXGI_FORMAT_R16_TYPELESS:
    case DXGI_FORMAT_R16_FLOAT:
    case DXGI_FORMAT_D16_UNORM:
    case DXGI_FORMAT_R16_UNORM:
    case DXGI_FORMAT_R16_UINT:
    case DXGI_FORMAT_R16_SNORM:
    case DXGI_FORMAT_R16_SINT:
    case DXGI_FORMAT_B5G6R5_UNORM:
    case DXGI_FORMAT_B5G5R5A1_UNORM:
    case DXGI_FORMAT_B4G4R4A4_UNORM:
        return 16;

    case DXGI_FORMAT_R8_TYPELESS:
    case DXGI_FORMAT_R8_UNORM:
    case DXGI_FORMAT_R8_UINT:
    case DXGI_FORMAT_R8_SNORM:
    case DXGI_FORMAT_R8_SINT:
    case DXGI_FORMAT_A8_UNORM:
        return 8;

    case DXGI_FORMAT_R1_UNORM:
        return 1;

    case DXGI_FORMAT_BC1_TYPELESS:
    case DXGI_FORMAT_BC1_UNORM:
    case DXGI_FORMAT_BC1_UNORM_SRGB:
    case DXGI_FORMAT_BC4_TYPELESS:
    case DXGI_FORMAT_BC4_UNORM:
    case DXGI_FORMAT_BC4_SNORM:
        return 4;

    case DXGI_FORMAT_BC2_TYPELESS:
    case DXGI_FORMAT_BC2_UNORM:
    case DXGI_FORMAT_BC2_UNORM_SRGB:
    case DXGI_FORMAT_BC3_TYPELESS:
    case DXGI_FORMAT_BC3_UNORM:
    case DXGI_FORMAT_BC3_UNORM_SRGB:
    case DXGI_FORMAT_BC5_TYPELESS:
    case DXGI_FORMAT_BC5_UNORM:
    case DXGI_FORMAT_BC5_SNORM:
    case DXGI_FORMAT_BC6H_TYPELESS:
    case DXGI_FORMAT_BC6H_UF16:
    case DXGI_FORMAT_BC6H_SF16:
    case DXGI_FORMAT_BC7_TYPELESS:
    case DXGI_FORMAT_BC7_UNORM:
    case DXGI_FORMAT_BC7_UNORM_SRGB:
        return 8;

    default:
        return 0;
    }
}


static TEXTURE_FORMAT DXGIFormatToTexFormat( DXGI_FORMAT TexFormat )
{
    switch(TexFormat)
    {
        case DXGI_FORMAT_UNKNOWN:                       return TEX_FORMAT_UNKNOWN;

        case DXGI_FORMAT_R32G32B32A32_TYPELESS:         return TEX_FORMAT_RGBA32_TYPELESS; 
        case DXGI_FORMAT_R32G32B32A32_FLOAT:            return TEX_FORMAT_RGBA32_FLOAT; 
        case DXGI_FORMAT_R32G32B32A32_UINT:             return TEX_FORMAT_RGBA32_UINT; 
        case DXGI_FORMAT_R32G32B32A32_SINT:             return TEX_FORMAT_RGBA32_SINT; 

        case DXGI_FORMAT_R32G32B32_TYPELESS:            return TEX_FORMAT_RGB32_TYPELESS; 
        case DXGI_FORMAT_R32G32B32_FLOAT:               return TEX_FORMAT_RGB32_FLOAT; 
        case DXGI_FORMAT_R32G32B32_UINT:                return TEX_FORMAT_RGB32_UINT; 
        case DXGI_FORMAT_R32G32B32_SINT:                return TEX_FORMAT_RGB32_SINT; 

        case DXGI_FORMAT_R16G16B16A16_TYPELESS:         return TEX_FORMAT_RGBA16_TYPELESS;
        case DXGI_FORMAT_R16G16B16A16_FLOAT:            return TEX_FORMAT_RGBA16_FLOAT;
        case DXGI_FORMAT_R16G16B16A16_UNORM:            return TEX_FORMAT_RGBA16_UNORM;
        case DXGI_FORMAT_R16G16B16A16_UINT:             return TEX_FORMAT_RGBA16_UINT;
        case DXGI_FORMAT_R16G16B16A16_SNORM:            return TEX_FORMAT_RGBA16_SNORM;
        case DXGI_FORMAT_R16G16B16A16_SINT:             return TEX_FORMAT_RGBA16_SINT;

        case DXGI_FORMAT_R32G32_TYPELESS:               return TEX_FORMAT_RG32_TYPELESS; 
        case DXGI_FORMAT_R32G32_FLOAT:                  return TEX_FORMAT_RG32_FLOAT; 
        case DXGI_FORMAT_R32G32_UINT:                   return TEX_FORMAT_RG32_UINT; 
        case DXGI_FORMAT_R32G32_SINT:                   return TEX_FORMAT_RG32_SINT; 

        case DXGI_FORMAT_R32G8X24_TYPELESS:             return TEX_FORMAT_R32G8X24_TYPELESS; 
        case DXGI_FORMAT_D32_FLOAT_S8X24_UINT:          return TEX_FORMAT_D32_FLOAT_S8X24_UINT; 
        case  DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS:     return TEX_FORMAT_R32_FLOAT_X8X24_TYPELESS;
        case DXGI_FORMAT_X32_TYPELESS_G8X24_UINT:       return TEX_FORMAT_X32_TYPELESS_G8X24_UINT; 

        case DXGI_FORMAT_R10G10B10A2_TYPELESS:          return TEX_FORMAT_RGB10A2_TYPELESS; 
        case DXGI_FORMAT_R10G10B10A2_UNORM:             return TEX_FORMAT_RGB10A2_UNORM; 
        case DXGI_FORMAT_R10G10B10A2_UINT:              return TEX_FORMAT_RGB10A2_UINT; 

        case DXGI_FORMAT_R11G11B10_FLOAT:               return TEX_FORMAT_R11G11B10_FLOAT; 

        case DXGI_FORMAT_R8G8B8A8_TYPELESS:             return TEX_FORMAT_RGBA8_TYPELESS; 
        case DXGI_FORMAT_R8G8B8A8_UNORM:                return TEX_FORMAT_RGBA8_UNORM; 
        case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:           return TEX_FORMAT_RGBA8_UNORM_SRGB; 
        case DXGI_FORMAT_R8G8B8A8_UINT:                 return TEX_FORMAT_RGBA8_UINT; 
        case DXGI_FORMAT_R8G8B8A8_SNORM:                return TEX_FORMAT_RGBA8_SNORM; 
        case DXGI_FORMAT_R8G8B8A8_SINT:                 return TEX_FORMAT_RGBA8_SINT; 

        case DXGI_FORMAT_R16G16_TYPELESS:               return TEX_FORMAT_RG16_TYPELESS; 
        case DXGI_FORMAT_R16G16_FLOAT:                  return TEX_FORMAT_RG16_FLOAT; 
        case DXGI_FORMAT_R16G16_UNORM:                  return TEX_FORMAT_RG16_UNORM; 
        case DXGI_FORMAT_R16G16_UINT:                   return TEX_FORMAT_RG16_UINT; 
        case DXGI_FORMAT_R16G16_SNORM:                  return TEX_FORMAT_RG16_SNORM; 
        case DXGI_FORMAT_R16G16_SINT:                   return TEX_FORMAT_RG16_SINT; 

        case DXGI_FORMAT_R32_TYPELESS:                  return TEX_FORMAT_R32_TYPELESS; 
        case DXGI_FORMAT_D32_FLOAT:                     return TEX_FORMAT_D32_FLOAT; 
        case DXGI_FORMAT_R32_FLOAT:                     return TEX_FORMAT_R32_FLOAT; 
        case DXGI_FORMAT_R32_UINT:                      return TEX_FORMAT_R32_UINT; 
        case DXGI_FORMAT_R32_SINT:                      return TEX_FORMAT_R32_SINT; 

        case DXGI_FORMAT_R24G8_TYPELESS:                return TEX_FORMAT_R24G8_TYPELESS; 
        case DXGI_FORMAT_D24_UNORM_S8_UINT:             return TEX_FORMAT_D24_UNORM_S8_UINT; 
        case DXGI_FORMAT_R24_UNORM_X8_TYPELESS:         return TEX_FORMAT_R24_UNORM_X8_TYPELESS; 
        case DXGI_FORMAT_X24_TYPELESS_G8_UINT:          return TEX_FORMAT_X24_TYPELESS_G8_UINT; 

        case DXGI_FORMAT_R8G8_TYPELESS:                 return TEX_FORMAT_RG8_TYPELESS; 
        case DXGI_FORMAT_R8G8_UNORM:                    return TEX_FORMAT_RG8_UNORM; 
        case DXGI_FORMAT_R8G8_UINT:                     return TEX_FORMAT_RG8_UINT; 
        case DXGI_FORMAT_R8G8_SNORM:                    return TEX_FORMAT_RG8_SNORM; 
        case DXGI_FORMAT_R8G8_SINT:                     return TEX_FORMAT_RG8_SINT; 

        case DXGI_FORMAT_R16_TYPELESS:                  return TEX_FORMAT_R16_TYPELESS; 
        case DXGI_FORMAT_R16_FLOAT:                     return TEX_FORMAT_R16_FLOAT; 
        case DXGI_FORMAT_D16_UNORM:                     return TEX_FORMAT_D16_UNORM; 
        case DXGI_FORMAT_R16_UNORM:                     return TEX_FORMAT_R16_UNORM; 
        case DXGI_FORMAT_R16_UINT:                      return TEX_FORMAT_R16_UINT; 
        case DXGI_FORMAT_R16_SNORM:                     return TEX_FORMAT_R16_SNORM; 
        case DXGI_FORMAT_R16_SINT:                      return TEX_FORMAT_R16_SINT; 

        case DXGI_FORMAT_R8_TYPELESS:                   return TEX_FORMAT_R8_TYPELESS; 
        case DXGI_FORMAT_R8_UNORM:                      return TEX_FORMAT_R8_UNORM; 
        case DXGI_FORMAT_R8_UINT:                       return TEX_FORMAT_R8_UINT; 
        case DXGI_FORMAT_R8_SNORM:                      return TEX_FORMAT_R8_SNORM; 
        case DXGI_FORMAT_R8_SINT:                       return TEX_FORMAT_R8_SINT; 
        case DXGI_FORMAT_A8_UNORM:                      return TEX_FORMAT_A8_UNORM; 

        case DXGI_FORMAT_R1_UNORM :                     return TEX_FORMAT_R1_UNORM; 
        case DXGI_FORMAT_R9G9B9E5_SHAREDEXP:            return TEX_FORMAT_RGB9E5_SHAREDEXP; 
        case DXGI_FORMAT_R8G8_B8G8_UNORM:               return TEX_FORMAT_RG8_B8G8_UNORM; 
        case DXGI_FORMAT_G8R8_G8B8_UNORM:               return TEX_FORMAT_G8R8_G8B8_UNORM; 

        case DXGI_FORMAT_BC1_TYPELESS:                  return TEX_FORMAT_BC1_TYPELESS; 
        case DXGI_FORMAT_BC1_UNORM:                     return TEX_FORMAT_BC1_UNORM; 
        case DXGI_FORMAT_BC1_UNORM_SRGB:                return TEX_FORMAT_BC1_UNORM_SRGB; 
        case DXGI_FORMAT_BC2_TYPELESS:                  return TEX_FORMAT_BC2_TYPELESS; 
        case DXGI_FORMAT_BC2_UNORM:                     return TEX_FORMAT_BC2_UNORM; 
        case DXGI_FORMAT_BC2_UNORM_SRGB:                return TEX_FORMAT_BC2_UNORM_SRGB; 
        case DXGI_FORMAT_BC3_TYPELESS:                  return TEX_FORMAT_BC3_TYPELESS; 
        case DXGI_FORMAT_BC3_UNORM:                     return TEX_FORMAT_BC3_UNORM; 
        case DXGI_FORMAT_BC3_UNORM_SRGB:                return TEX_FORMAT_BC3_UNORM_SRGB; 
        case DXGI_FORMAT_BC4_TYPELESS:                  return TEX_FORMAT_BC4_TYPELESS; 
        case DXGI_FORMAT_BC4_UNORM:                     return TEX_FORMAT_BC4_UNORM; 
        case DXGI_FORMAT_BC4_SNORM:                     return TEX_FORMAT_BC4_SNORM; 
        case DXGI_FORMAT_BC5_TYPELESS:                  return TEX_FORMAT_BC5_TYPELESS; 
        case DXGI_FORMAT_BC5_UNORM:                     return TEX_FORMAT_BC5_UNORM; 
        case DXGI_FORMAT_BC5_SNORM:                     return TEX_FORMAT_BC5_SNORM; 

        case DXGI_FORMAT_B5G6R5_UNORM:                  return TEX_FORMAT_B5G6R5_UNORM; 
        case DXGI_FORMAT_B5G5R5A1_UNORM:                return TEX_FORMAT_B5G5R5A1_UNORM; 
        case DXGI_FORMAT_B8G8R8A8_UNORM:                return TEX_FORMAT_BGRA8_UNORM; 
        case DXGI_FORMAT_B8G8R8X8_UNORM:                return TEX_FORMAT_BGRX8_UNORM; 

        case DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM:    return TEX_FORMAT_R10G10B10_XR_BIAS_A2_UNORM;

        case DXGI_FORMAT_B8G8R8A8_TYPELESS:             return TEX_FORMAT_BGRA8_TYPELESS; 
        case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB:           return TEX_FORMAT_BGRA8_UNORM_SRGB; 
        case DXGI_FORMAT_B8G8R8X8_TYPELESS:             return TEX_FORMAT_BGRX8_TYPELESS; 
        case DXGI_FORMAT_B8G8R8X8_UNORM_SRGB:           return TEX_FORMAT_BGRX8_UNORM_SRGB; 

        case DXGI_FORMAT_BC6H_TYPELESS:                 return TEX_FORMAT_BC6H_TYPELESS; 
        case DXGI_FORMAT_BC6H_UF16:                     return TEX_FORMAT_BC6H_UF16; 
        case DXGI_FORMAT_BC6H_SF16:                     return TEX_FORMAT_BC6H_SF16; 
        case DXGI_FORMAT_BC7_TYPELESS :                 return TEX_FORMAT_BC7_TYPELESS; 
        case DXGI_FORMAT_BC7_UNORM:                     return TEX_FORMAT_BC7_UNORM; 
        case DXGI_FORMAT_BC7_UNORM_SRGB:                return TEX_FORMAT_BC7_UNORM_SRGB; 

        default:                                        return TEX_FORMAT_UNKNOWN;
    }
}


struct TexFormatToDXGIFormatMap
{
    TexFormatToDXGIFormatMap()
    {
        for (int DXGIFmt = int{DXGI_FORMAT_UNKNOWN} + 1; DXGIFmt < int{DXGI_FORMAT_COUNT}; ++DXGIFmt)
        {
            auto TexFmt = DXGIFormatToTexFormat(static_cast<DXGI_FORMAT>(DXGIFmt));
            if (TexFmt  != TEX_FORMAT_UNKNOWN)
                FmtMap[TexFmt] = static_cast<DXGI_FORMAT>(DXGIFmt);
        }
    }

    DXGI_FORMAT operator()(TEXTURE_FORMAT Fmt)const
    {
        VERIFY_EXPR(Fmt < FmtMap.size());
        return Fmt < FmtMap.size() ? FmtMap[Fmt] : DXGI_FORMAT_UNKNOWN;
    }

private:
    std::array<DXGI_FORMAT, TEX_FORMAT_NUM_FORMATS> FmtMap{};
};
TexFormatToDXGIFormatMap TexFormatToDXGIFormat;


//--------------------------------------------------------------------------------------
// Get surface information for a particular format
//--------------------------------------------------------------------------------------
static void GetSurfaceInfo(
    _In_ size_t width,
    _In_ size_t height,
    _In_ DXGI_FORMAT fmt,
    _Out_opt_ size_t* outNumBytes,
    _Out_opt_ size_t* outRowBytes,
    _Out_opt_ size_t* outNumRows
    )
{
    size_t numBytes = 0;
    size_t rowBytes = 0;
    size_t numRows = 0;

    bool bc = false;
    bool packed  = false;
    size_t bcnumBytesPerBlock = 0;
    switch (fmt)
    {
    case DXGI_FORMAT_BC1_TYPELESS:
    case DXGI_FORMAT_BC1_UNORM:
    case DXGI_FORMAT_BC1_UNORM_SRGB:
    case DXGI_FORMAT_BC4_TYPELESS:
    case DXGI_FORMAT_BC4_UNORM:
    case DXGI_FORMAT_BC4_SNORM:
        bc = true;
        bcnumBytesPerBlock = 8;
        break;

    case DXGI_FORMAT_BC2_TYPELESS:
    case DXGI_FORMAT_BC2_UNORM:
    case DXGI_FORMAT_BC2_UNORM_SRGB:
    case DXGI_FORMAT_BC3_TYPELESS:
    case DXGI_FORMAT_BC3_UNORM:
    case DXGI_FORMAT_BC3_UNORM_SRGB:
    case DXGI_FORMAT_BC5_TYPELESS:
    case DXGI_FORMAT_BC5_UNORM:
    case DXGI_FORMAT_BC5_SNORM:
    case DXGI_FORMAT_BC6H_TYPELESS:
    case DXGI_FORMAT_BC6H_UF16:
    case DXGI_FORMAT_BC6H_SF16:
    case DXGI_FORMAT_BC7_TYPELESS:
    case DXGI_FORMAT_BC7_UNORM:
    case DXGI_FORMAT_BC7_UNORM_SRGB:
        bc = true;
        bcnumBytesPerBlock = 16;
        break;

    case DXGI_FORMAT_R8G8_B8G8_UNORM:
    case DXGI_FORMAT_G8R8_G8B8_UNORM:
        packed = true;
        break;
    default: break;
    }

    if (bc)
    {
        size_t numBlocksWide = 0;
        if (width > 0)
        {
            numBlocksWide = std::max<size_t>(1, (width + 3) / 4);
        }
        size_t numBlocksHigh = 0;
        if (height > 0)
        {
            numBlocksHigh = std::max<size_t>(1, (height + 3) / 4);
        }
        rowBytes = numBlocksWide * bcnumBytesPerBlock;
        numRows = numBlocksHigh;
    }
    else if (packed)
    {
        rowBytes = ((width + 1) >> 1) * 4;
        numRows = height;
    }
    else
    {
        size_t bpp = BitsPerPixel(fmt);
        rowBytes = (width * bpp + 7) / 8; // round up to nearest byte
        numRows = height;
    }

    numBytes = rowBytes * numRows;
    if (outNumBytes)
    {
        *outNumBytes = numBytes;
    }
    if (outRowBytes)
    {
        *outRowBytes = rowBytes;
    }
    if (outNumRows)
    {
        *outNumRows = numRows;
    }
}


//--------------------------------------------------------------------------------------
#define ISBITMASK(r, g, b, a) (ddpf.RBitMask == r && ddpf.GBitMask == g && ddpf.BBitMask == b && ddpf.ABitMask == a)

static DXGI_FORMAT GetDXGIFormat(const DDS_PIXELFORMAT& ddpf)
{
    if (ddpf.flags & DDS_RGB)
    {
        // Note that sRGB formats are written using the "DX10" extended header

        switch (ddpf.RGBBitCount)
        {
        case 32:
            if (ISBITMASK(0x000000ff, 0x0000ff00, 0x00ff0000, 0xff000000))
            {
                return DXGI_FORMAT_R8G8B8A8_UNORM;
            }

            if (ISBITMASK(0x00ff0000, 0x0000ff00, 0x000000ff, 0xff000000))
            {
                return DXGI_FORMAT_B8G8R8A8_UNORM;
            }

            if (ISBITMASK(0x00ff0000, 0x0000ff00, 0x000000ff, 0x00000000))
            {
                return DXGI_FORMAT_B8G8R8X8_UNORM;
            }

            // No DXGI format maps to ISBITMASK(0x000000ff, 0x0000ff00, 0x00ff0000, 0x00000000) aka D3DFMT_X8B8G8R8

            // Note that many common DDS reader/writers (including D3DX) swap the
            // the RED/BLUE masks for 10:10:10:2 formats. We assume
            // below that the 'backwards' header mask is being used since it is most
            // likely written by D3DX. The more robust solution is to use the 'DX10'
            // header extension and specify the DXGI_FORMAT_R10G10B10A2_UNORM format directly

            // For 'correct' writers, this should be 0x000003ff, 0x000ffc00, 0x3ff00000 for RGB data
            if (ISBITMASK(0x3ff00000, 0x000ffc00, 0x000003ff, 0xc0000000))
            {
                return DXGI_FORMAT_R10G10B10A2_UNORM;
            }

            // No DXGI format maps to ISBITMASK(0x000003ff, 0x000ffc00, 0x3ff00000, 0xc0000000) aka D3DFMT_A2R10G10B10

            if (ISBITMASK(0x0000ffff, 0xffff0000, 0x00000000, 0x00000000))
            {
                return DXGI_FORMAT_R16G16_UNORM;
            }

            if (ISBITMASK(0xffffffff, 0x00000000, 0x00000000, 0x00000000))
            {
                // Only 32-bit color channel format in D3D9 was R32F
                return DXGI_FORMAT_R32_FLOAT; // D3DX writes this out as a FourCC of 114
            }
            break;

        case 24:
            // No 24bpp DXGI formats aka D3DFMT_R8G8B8
            break;

        case 16:
            if (ISBITMASK(0x7c00, 0x03e0, 0x001f, 0x8000))
            {
                return DXGI_FORMAT_B5G5R5A1_UNORM;
            }
            if (ISBITMASK(0xf800, 0x07e0, 0x001f, 0x0000))
            {
                return DXGI_FORMAT_B5G6R5_UNORM;
            }

            // No DXGI format maps to ISBITMASK(0x7c00, 0x03e0, 0x001f, 0x0000) aka D3DFMT_X1R5G5B5
            if (ISBITMASK(0x0f00, 0x00f0, 0x000f, 0xf000))
            {
                return DXGI_FORMAT_B4G4R4A4_UNORM;
            }

            // No DXGI format maps to ISBITMASK(0x0f00, 0x00f0, 0x000f, 0x0000) aka D3DFMT_X4R4G4B4

            // No 3:3:2, 3:3:2:8, or paletted DXGI formats aka D3DFMT_A8R3G3B2, D3DFMT_R3G3B2, D3DFMT_P8, D3DFMT_A8P8, etc.
            break;
        }
    }
    else if (ddpf.flags & DDS_LUMINANCE)
    {
        if (8 == ddpf.RGBBitCount)
        {
            if (ISBITMASK(0x000000ff, 0x00000000, 0x00000000, 0x00000000))
            {
                return DXGI_FORMAT_R8_UNORM; // D3DX10/11 writes this out as DX10 extension
            }

            // No DXGI format maps to ISBITMASK(0x0f, 0x00, 0x00, 0xf0) aka D3DFMT_A4L4
        }

        if (16 == ddpf.RGBBitCount)
        {
            if (ISBITMASK(0x0000ffff, 0x00000000, 0x00000000, 0x00000000))
            {
                return DXGI_FORMAT_R16_UNORM; // D3DX10/11 writes this out as DX10 extension
            }
            if (ISBITMASK(0x000000ff, 0x00000000, 0x00000000, 0x0000ff00))
            {
                return DXGI_FORMAT_R8G8_UNORM; // D3DX10/11 writes this out as DX10 extension
            }
        }
    }
    else if (ddpf.flags & DDS_ALPHA)
    {
        if (8 == ddpf.RGBBitCount)
        {
            return DXGI_FORMAT_A8_UNORM;
        }
    }
    else if (ddpf.flags & DDS_FOURCC)
    {
        if (MAKEFOURCC('D', 'X', 'T', '1') == ddpf.fourCC)
        {
            return DXGI_FORMAT_BC1_UNORM;
        }
        if (MAKEFOURCC('D', 'X', 'T', '3') == ddpf.fourCC)
        {
            return DXGI_FORMAT_BC2_UNORM;
        }
        if (MAKEFOURCC('D', 'X', 'T', '5') == ddpf.fourCC)
        {
            return DXGI_FORMAT_BC3_UNORM;
        }

        // While pre-mulitplied alpha isn't directly supported by the DXGI formats,
        // they are basically the same as these BC formats so they can be mapped
        if (MAKEFOURCC('D', 'X', 'T', '2') == ddpf.fourCC)
        {
            return DXGI_FORMAT_BC2_UNORM;
        }
        if (MAKEFOURCC('D', 'X', 'T', '4') == ddpf.fourCC)
        {
            return DXGI_FORMAT_BC3_UNORM;
        }

        if (MAKEFOURCC('A', 'T', 'I', '1') == ddpf.fourCC)
        {
            return DXGI_FORMAT_BC4_UNORM;
        }
        if (MAKEFOURCC('B', 'C', '4', 'U') == ddpf.fourCC)
        {
            return DXGI_FORMAT_BC4_UNORM;
        }
        if (MAKEFOURCC('B', 'C', '4', 'S') == ddpf.fourCC)
        {
            return DXGI_FORMAT_BC4_SNORM;
        }

        if (MAKEFOURCC('A', 'T', 'I', '2') == ddpf.fourCC)
        {
            return DXGI_FORMAT_BC5_UNORM;
        }
        if (MAKEFOURCC('B', 'C', '5', 'U') == ddpf.fourCC)
        {
            return DXGI_FORMAT_BC5_UNORM;
        }
        if (MAKEFOURCC('B', 'C', '5', 'S') == ddpf.fourCC)
        {
            return DXGI_FORMAT_BC5_SNORM;
        }

        // BC6H and BC7 are written using the "DX10" extended header

        if (MAKEFOURCC('R', 'G', 'B', 'G') == ddpf.fourCC)
        {
            return DXGI_FORMAT_R8G8_B8G8_UNORM;
        }
        if (MAKEFOURCC('G', 'R', 'G', 'B') == ddpf.fourCC)
        {
            return DXGI_FORMAT_G8R8_G8B8_UNORM;
        }

        // Check for D3DFORMAT enums being set here
        switch (ddpf.fourCC)
        {
        case 36: // D3DFMT_A16B16G16R16
            return DXGI_FORMAT_R16G16B16A16_UNORM;

        case 110: // D3DFMT_Q16W16V16U16
            return DXGI_FORMAT_R16G16B16A16_SNORM;

        case 111: // D3DFMT_R16F
            return DXGI_FORMAT_R16_FLOAT;

        case 112: // D3DFMT_G16R16F
            return DXGI_FORMAT_R16G16_FLOAT;

        case 113: // D3DFMT_A16B16G16R16F
            return DXGI_FORMAT_R16G16B16A16_FLOAT;

        case 114: // D3DFMT_R32F
            return DXGI_FORMAT_R32_FLOAT;

        case 115: // D3DFMT_G32R32F
            return DXGI_FORMAT_R32G32_FLOAT;

        case 116: // D3DFMT_A32B32G32R32F
            return DXGI_FORMAT_R32G32B32A32_FLOAT;
        }
    }

    return DXGI_FORMAT_UNKNOWN;
}

// clang-format on

//--------------------------------------------------------------------------------------
static void FillInitData(
    _In_ Uint32      width,
    _In_ Uint32      height,
    _In_ Uint32      depth,
    _In_ Uint32      srcMipCount,
    _In_ Uint32      dstMipCount,
    _In_ Uint32      arraySize,
    _In_ DXGI_FORMAT format,
    _In_ size_t      bitSize,
    _In_ const Uint8* bitData,
    _Out_ TextureSubResData* initData)
{
    VERIFY_EXPR(bitData != nullptr && initData != nullptr);

    const Uint8* pSrcBits = bitData;
    const Uint8* pEndBits = bitData + bitSize;

    size_t index = 0;
    for (size_t slice = 0; slice < arraySize; slice++)
    {
        for (size_t mip = 0; mip < srcMipCount; mip++)
        {
            const auto w = std::max(width >> mip, 1u);
            const auto h = std::max(height >> mip, 1u);
            const auto d = std::max(depth >> mip, 1u);

            size_t NumBytes = 0;
            size_t RowBytes = 0;
            size_t NumRows  = 0;
            GetSurfaceInfo(w, h, format, &NumBytes, &RowBytes, &NumRows);

            if (mip < dstMipCount)
            {
                VERIFY_EXPR(index < size_t{dstMipCount} * size_t{arraySize});
                initData[index].pData       = reinterpret_cast<const void*>(pSrcBits);
                initData[index].Stride      = static_cast<Uint32>(RowBytes);
                initData[index].DepthStride = static_cast<Uint32>(NumBytes);
                ++index;
            }

            pSrcBits += NumBytes * d;
            if (pSrcBits > pEndBits)
            {
                LOG_ERROR_AND_THROW("Out of bounds");
            }
        }
    }

    if (!index)
    {
        LOG_ERROR_AND_THROW("Unknown error");
    }
}


#if 0
//--------------------------------------------------------------------------------------
static D2D1_ALPHA_MODE GetAlphaMode(_In_ const DDS_HEADER* header)
{
    if (header->ddspf.flags & DDS_FOURCC)
    {
        if (MAKEFOURCC('D', 'X', '1', '0') == header->ddspf.fourCC)
        {
            auto d3d10ext = reinterpret_cast<const DDS_HEADER_DXT10*>((const char*)header + sizeof(DDS_HEADER));
            switch (d3d10ext->miscFlags2 & DDS_MISC_FLAGS2_ALPHA_MODE_MASK)
            {
            case DDS_ALPHA_MODE_STRAIGHT:
                return D2D1_ALPHA_MODE_STRAIGHT;

            case DDS_ALPHA_MODE_PREMULTIPLIED:
                return D2D1_ALPHA_MODE_PREMULTIPLIED;

            case DDS_ALPHA_MODE_OPAQUE:
            case DDS_ALPHA_MODE_CUSTOM:
                // No D2D1_ALPHA_MODE equivalent, so return "Ignore" for now
                return D2D1_ALPHA_MODE_IGNORE;
            }
        }
        else if ((MAKEFOURCC('D', 'X', 'T', '2') == header->ddspf.fourCC)
                  || (MAKEFOURCC('D', 'X', 'T', '4') == header->ddspf.fourCC))
        {
            return D2D1_ALPHA_MODE_PREMULTIPLIED;
        }
        // DXT1, DXT3, and DXT5 legacy files could be straight alpha or something else, so return "Unknown" to leave it up to the app
    }

    return D2D1_ALPHA_MODE_UNKNOWN;
}

#endif

} // namespace

namespace Diligent
{

void TextureLoaderImpl::LoadFromDDS(const TextureLoadInfo& TexLoadInfo, const Uint8* pData, size_t DataSize)
{
    // Validate DDS file in memory
    if (DataSize < (sizeof(Uint32) + sizeof(DDS_HEADER)))
    {
        LOG_ERROR_AND_THROW("DDS data size (", DataSize, ") is too small");
    }

    Uint32 dwMagicNumber = *(const Uint32*)(pData);
    if (dwMagicNumber != DDS_MAGIC)
    {
        LOG_ERROR_AND_THROW("Invalid dds magic number (", dwMagicNumber, "). ", DDS_MAGIC, " is expected.");
    }

    const auto* header = reinterpret_cast<const DDS_HEADER*>(pData + sizeof(Uint32));

    // Verify header to validate DDS file
    if (header->size != sizeof(DDS_HEADER) ||
        header->ddspf.size != sizeof(DDS_PIXELFORMAT))
    {
        LOG_ERROR_AND_THROW("Invalid dds file header");
    }

    // Check for DX10 extension
    bool bDXT10Header = false;
    if ((header->ddspf.flags & DDS_FOURCC) &&
        (MAKEFOURCC('D', 'X', '1', '0') == header->ddspf.fourCC))
    {
        // Must be long enough for both headers and magic value
        if (DataSize < (sizeof(DDS_HEADER) + sizeof(Uint32) + sizeof(DDS_HEADER_DXT10)))
        {
            LOG_ERROR_AND_THROW("Invalid DX10 extension");
        }

        bDXT10Header = true;
    }

    m_TexDesc.Width  = header->width;
    m_TexDesc.Height = header->height;
    Uint32 Depth     = header->depth;
    Uint32 ArraySize = 1;

    ptrdiff_t SubResDataOffset = sizeof(Uint32) + sizeof(DDS_HEADER) + (bDXT10Header ? sizeof(DDS_HEADER_DXT10) : 0);

    bool        IsCubeMap   = false;
    Uint32      d3d11ResDim = D3D11_RESOURCE_DIMENSION_UNKNOWN;
    DXGI_FORMAT dxgiFormat  = DXGI_FORMAT_UNKNOWN;

    const auto SrcMipCount = std::max(header->mipMapCount, 1u);
    m_TexDesc.MipLevels    = SrcMipCount;
    if (TexLoadInfo.MipLevels > 0)
        m_TexDesc.MipLevels = std::min(m_TexDesc.MipLevels, TexLoadInfo.MipLevels);

    if ((header->ddspf.flags & DDS_FOURCC) &&
        (MAKEFOURCC('D', 'X', '1', '0') == header->ddspf.fourCC))
    {
        auto d3d10ext = reinterpret_cast<const DDS_HEADER_DXT10*>((const char*)header + sizeof(DDS_HEADER));

        ArraySize = d3d10ext->arraySize;
        if (ArraySize == 0)
        {
            LOG_ERROR_AND_THROW("Array size is zero");
        }

        if (BitsPerPixel(d3d10ext->dxgiFormat) == 0)
        {
            LOG_ERROR_AND_THROW("Undefined DXGI format");
        }

        dxgiFormat = d3d10ext->dxgiFormat;

        switch (d3d10ext->resourceDimension)
        {
            case D3D11_RESOURCE_DIMENSION_TEXTURE1D:
            {
                // D3DX writes 1D textures with a fixed Height of 1
                if ((header->flags & DDS_HEIGHT) && m_TexDesc.Height != 1)
                {
                    LOG_ERROR_AND_THROW("Unexpected height (", m_TexDesc.Height, ") for texture 1D");
                }
            }
            break;

            case D3D11_RESOURCE_DIMENSION_TEXTURE2D:
            {
                if ((d3d10ext->miscFlag & D3D11_RESOURCE_MISC_TEXTURECUBE) != 0)
                {
                    IsCubeMap = true;
                    ArraySize *= 6;
                }
            }
            break;

            case D3D11_RESOURCE_DIMENSION_TEXTURE3D:
            {
                if (!(header->flags & DDS_HEADER_FLAGS_VOLUME))
                {
                    LOG_ERROR_AND_THROW("DDS_HEADER_FLAGS_VOLUME flag is not set");
                }
            }
            break;

            default:
                LOG_ERROR_AND_THROW("Unknown resource dimension");
        }

        d3d11ResDim = d3d10ext->resourceDimension;
    }
    else
    {
        dxgiFormat = GetDXGIFormat(header->ddspf);
        if (dxgiFormat == DXGI_FORMAT_UNKNOWN)
        {
            LOG_ERROR_AND_THROW("Unknown DXGIF format");
        }

        if (header->flags & DDS_HEADER_FLAGS_VOLUME)
        {
            d3d11ResDim = D3D11_RESOURCE_DIMENSION_TEXTURE3D;
        }
        else
        {
            if (header->caps2 & DDS_CUBEMAP)
            {
                // We require all six faces to be defined
                if ((header->caps2 & DDS_CUBEMAP_ALLFACES) != DDS_CUBEMAP_ALLFACES)
                {
                    LOG_ERROR_AND_THROW("All six faces of a cubemap must be defined");
                }

                ArraySize = 6;
                IsCubeMap = true;
            }

            d3d11ResDim = D3D11_RESOURCE_DIMENSION_TEXTURE2D;
        }

        VERIFY_EXPR(BitsPerPixel(dxgiFormat) != 0);
    }

    // Bound sizes (for security purposes we don't trust DDS file metadata larger than the D3D 11.x hardware requirements)
    if (m_TexDesc.MipLevels > D3D11_REQ_MIP_LEVELS)
    {
        LOG_ERROR_AND_THROW("Too many mip levels specified (", m_TexDesc.MipLevels, ")");
    }

    switch (d3d11ResDim)
    {
        case D3D11_RESOURCE_DIMENSION_TEXTURE1D:
        {
            m_TexDesc.ArraySize = ArraySize; // ArraySize is aliased with Depth
            if ((m_TexDesc.ArraySize > D3D11_REQ_TEXTURE1D_ARRAY_AXIS_DIMENSION) ||
                (m_TexDesc.Width > D3D11_REQ_TEXTURE1D_U_DIMENSION))
            {
                LOG_ERROR_AND_THROW("Texture1D dimensions are out of bounds");
            }

            m_TexDesc.Height = 1;
            m_TexDesc.Type   = m_TexDesc.ArraySize > 1 ? RESOURCE_DIM_TEX_1D_ARRAY : RESOURCE_DIM_TEX_1D;
        }
        break;

        case D3D11_RESOURCE_DIMENSION_TEXTURE2D:
        {
            m_TexDesc.ArraySize = ArraySize; // ArraySize is aliased with Depth
            if ((m_TexDesc.ArraySize > D3D11_REQ_TEXTURE2D_ARRAY_AXIS_DIMENSION) ||
                (m_TexDesc.Width > D3D11_REQ_TEXTURECUBE_DIMENSION) ||
                (m_TexDesc.Height > D3D11_REQ_TEXTURECUBE_DIMENSION))
            {
                LOG_ERROR_AND_THROW((IsCubeMap ? "TextureCube" : "Texture2D"), " dimensions are out of bounds");
            }

            m_TexDesc.Type = IsCubeMap ?
                (m_TexDesc.ArraySize > 6 ? RESOURCE_DIM_TEX_CUBE_ARRAY : RESOURCE_DIM_TEX_CUBE) :
                (m_TexDesc.ArraySize > 1 ? RESOURCE_DIM_TEX_2D_ARRAY : RESOURCE_DIM_TEX_2D);
        }
        break;

        case D3D11_RESOURCE_DIMENSION_TEXTURE3D:
        {
            m_TexDesc.Depth = Depth; // Depth is aliased with ArraySize
            if ((m_TexDesc.Width > D3D11_REQ_TEXTURE3D_U_V_OR_W_DIMENSION) ||
                (m_TexDesc.Height > D3D11_REQ_TEXTURE3D_U_V_OR_W_DIMENSION) ||
                (m_TexDesc.Depth > D3D11_REQ_TEXTURE3D_U_V_OR_W_DIMENSION))
            {
                LOG_ERROR_AND_THROW("Texture3D dimensions are out of bounds");
            }

            m_TexDesc.Type = RESOURCE_DIM_TEX_3D;
        }
        break;
    }
    m_TexDesc.Format = DXGIFormatToTexFormat(dxgiFormat);

    m_SubResources.resize(size_t{ArraySize} * size_t{m_TexDesc.MipLevels});
    FillInitData(m_TexDesc.Width, m_TexDesc.Height, Depth, SrcMipCount, m_TexDesc.MipLevels, ArraySize, dxgiFormat,
                 DataSize - SubResDataOffset, pData + SubResDataOffset, m_SubResources.data());
}


bool SaveTextureAsDDS(const char*        FilePath,
                      const TextureDesc& Desc,
                      const TextureData& TexData)
{
    const auto ArraySize = Desc.GetArraySize();
    VERIFY(TexData.NumSubresources == Desc.MipLevels * ArraySize, "Incorrect number of subresources");
    VERIFY_EXPR(TexData.pSubResources != nullptr);

    Uint32 Magic = MAKEFOURCC('D', 'D', 'S', ' ');

    DDS_HEADER Header{};
    Header.size         = sizeof(Header);
    Header.flags        = DDS_HEADER_FLAGS_TEXTURE | DDS_HEADER_FLAGS_MIPMAP;
    Header.ddspf.size   = sizeof(Header.ddspf);
    Header.ddspf.fourCC = MAKEFOURCC('D', 'X', '1', '0');
    Header.ddspf.flags  = DDS_FOURCC;
    Header.width        = Desc.Width;
    Header.height       = Desc.Height;
    Header.mipMapCount  = Desc.MipLevels;

    DDS_HEADER_DXT10 Header10{};
    Header10.dxgiFormat = TexFormatToDXGIFormat(Desc.Format);
    Header10.arraySize  = ArraySize;
    switch (Desc.Type)
    {
        case RESOURCE_DIM_TEX_1D:
        case RESOURCE_DIM_TEX_1D_ARRAY:
            Header10.resourceDimension = D3D11_RESOURCE_DIMENSION_TEXTURE1D;
            break;

        case RESOURCE_DIM_TEX_2D:
        case RESOURCE_DIM_TEX_2D_ARRAY:
        case RESOURCE_DIM_TEX_CUBE:
        case RESOURCE_DIM_TEX_CUBE_ARRAY:
            Header10.resourceDimension = D3D11_RESOURCE_DIMENSION_TEXTURE2D;
            break;

        case RESOURCE_DIM_TEX_3D:
            Header10.resourceDimension = D3D11_RESOURCE_DIMENSION_TEXTURE3D;
            break;

        default:
            UNEXPECTED("Unexpected texture dimension");
            return false;
    }

    FileWrapper File{FilePath, EFileAccessMode::Overwrite};
    if (!File)
    {
        LOG_ERROR_MESSAGE("Failed to open file '", FilePath, "'.");
        return false;
    }

    if (!File->Write(&Magic, sizeof(Magic)))
        return false;

    if (!File->Write(&Header, sizeof(Header)))
        return false;

    if (!File->Write(&Header10, sizeof(Header10)))
        return false;

    const auto& FmtAttribs = GetTextureFormatAttribs(Desc.Format);
    for (Uint32 Slice = 0; Slice < ArraySize; ++Slice)
    {
        for (Uint32 Mip = 0; Mip < Desc.MipLevels; ++Mip)
        {
            const auto& MipProps = GetMipLevelProperties(Desc, Mip);
            const auto& SubRes   = TexData.pSubResources[Slice * Desc.MipLevels + Mip];
            VERIFY_EXPR(SubRes.pData != nullptr);
            const auto* pData  = reinterpret_cast<const Uint8*>(SubRes.pData);
            const auto  Stride = SubRes.Stride;
            VERIFY(Stride >= MipProps.RowSize, "Row stride is too small");
            for (Uint32 row = 0; row < MipProps.StorageHeight / FmtAttribs.BlockHeight; ++row)
            {
                const auto* pRowData = pData + Stride * row;
                if (!File->Write(pRowData, StaticCast<size_t>(MipProps.RowSize)))
                    return false;
            }
        }
    }

    return true;
}

} // namespace Diligent

extern "C"
{
    void Diligent_SaveTextureAsDDS(const char*                  FilePath,
                                   const Diligent::TextureDesc& Desc,
                                   const Diligent::TextureData& TexData)
    {
        Diligent::SaveTextureAsDDS(FilePath, Desc, TexData);
    }
}
