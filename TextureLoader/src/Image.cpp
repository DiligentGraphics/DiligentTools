/*
 *  Copyright 2019-2020 Diligent Graphics LLC
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

#include "pch.h"

#include <algorithm>
#include <array>

#include "Image.hpp"
#include "Errors.hpp"

#include "tiffio.h"
#include "png.h"
#include "jpeglib.h"

#include "DataBlobImpl.hpp"
#include "DebugUtilities.hpp"
#include "RefCntAutoPtr.hpp"
#include "Align.hpp"
#include "GraphicsAccessories.hpp"
#include "BasicFileStream.hpp"
#include "StringTools.hpp"

namespace Diligent
{

class TIFFClientOpenWrapper
{
public:
    TIFFClientOpenWrapper(IDataBlob* pData) :
        m_Offset{0},
        m_Size{pData->GetSize()},
        m_pData{pData}
    {
    }

    static tmsize_t TIFFReadProc(thandle_t pClientData, void* pBuffer, tmsize_t Size)
    {
        auto* pThis   = reinterpret_cast<TIFFClientOpenWrapper*>(pClientData);
        auto* pSrcPtr = reinterpret_cast<Uint8*>(pThis->m_pData->GetDataPtr()) + pThis->m_Offset;
        memcpy(pBuffer, pSrcPtr, Size);
        pThis->m_Offset += Size;
        return Size;
    }

    static tmsize_t TIFFWriteProc(thandle_t pClientData, void* pBuffer, tmsize_t Size)
    {
        auto* pThis = reinterpret_cast<TIFFClientOpenWrapper*>(pClientData);
        if (pThis->m_Offset + Size > pThis->m_Size)
        {
            pThis->m_Size = pThis->m_Offset + Size;
            pThis->m_pData->Resize(pThis->m_Size);
        }
        auto* pDstPtr = reinterpret_cast<Uint8*>(pThis->m_pData->GetDataPtr()) + pThis->m_Offset;
        memcpy(pDstPtr, pBuffer, Size);
        pThis->m_Offset += Size;
        return Size;
    }

    static toff_t TIFFSeekProc(thandle_t pClientData, toff_t Offset, int Whence)
    {
        auto* pThis = reinterpret_cast<TIFFClientOpenWrapper*>(pClientData);
        switch (Whence)
        {
            case SEEK_SET: pThis->m_Offset = static_cast<size_t>(Offset); break;
            case SEEK_CUR: pThis->m_Offset += static_cast<size_t>(Offset); break;
            case SEEK_END: pThis->m_Offset = pThis->m_Size + static_cast<size_t>(Offset); break;
            default: UNEXPECTED("Unexpected whence");
        }

        return pThis->m_Offset;
    }

    static int TIFFCloseProc(thandle_t pClientData)
    {
        auto* pThis = reinterpret_cast<TIFFClientOpenWrapper*>(pClientData);
        pThis->m_pData.Release();
        pThis->m_Size   = 0;
        pThis->m_Offset = 0;
        return 0;
    }

    static toff_t TIFFSizeProc(thandle_t pClientData)
    {
        auto* pThis = reinterpret_cast<TIFFClientOpenWrapper*>(pClientData);
        return pThis->m_Size;
    }

    static int TIFFMapFileProc(thandle_t pClientData, void** base, toff_t* size)
    {
        UNEXPECTED("Client file mapping is not implemented. Use \'m\' when opening TIFF file to disable file mapping.");
        return 0;
    }

    static void TIFFUnmapFileProc(thandle_t pClientData, void* base, toff_t size)
    {
        UNEXPECTED("Client file mapping is not implemented. Use \'m\' when opening TIFF file to disable file mapping.");
    }

private:
    size_t                   m_Offset;
    size_t                   m_Size;
    RefCntAutoPtr<IDataBlob> m_pData;
};

void Image::LoadTiffFile(IDataBlob* pFileData, const ImageLoadInfo& LoadInfo)
{
    TIFFClientOpenWrapper TiffClientOpenWrpr(pFileData);

    auto TiffFile = TIFFClientOpen("", "rm", &TiffClientOpenWrpr,
                                   TIFFClientOpenWrapper::TIFFReadProc,
                                   TIFFClientOpenWrapper::TIFFWriteProc,
                                   TIFFClientOpenWrapper::TIFFSeekProc,
                                   TIFFClientOpenWrapper::TIFFCloseProc,
                                   TIFFClientOpenWrapper::TIFFSizeProc,
                                   TIFFClientOpenWrapper::TIFFMapFileProc,
                                   TIFFClientOpenWrapper::TIFFUnmapFileProc);

    TIFFGetField(TiffFile, TIFFTAG_IMAGEWIDTH, &m_Desc.Width);
    TIFFGetField(TiffFile, TIFFTAG_IMAGELENGTH, &m_Desc.Height);

    Uint16 SamplesPerPixel = 0;
    // SamplesPerPixel is usually 1 for bilevel, grayscale, and palette-color images.
    // SamplesPerPixel is usually 3 for RGB images. If this value is higher, ExtraSamples
    // should give an indication of the meaning of the additional channels.
    TIFFGetField(TiffFile, TIFFTAG_SAMPLESPERPIXEL, &SamplesPerPixel);
    m_Desc.NumComponents = SamplesPerPixel;

    Uint16 BitsPerSample = 0;
    TIFFGetField(TiffFile, TIFFTAG_BITSPERSAMPLE, &BitsPerSample);

    Uint16 SampleFormat = 0;
    TIFFGetField(TiffFile, TIFFTAG_SAMPLEFORMAT, &SampleFormat);
    if (SampleFormat == 0)
        SampleFormat = SAMPLEFORMAT_UINT;

    switch (SampleFormat)
    {
        case SAMPLEFORMAT_UINT:
            switch (BitsPerSample)
            {
                case 8: m_Desc.ComponentType = VT_UINT8; break;
                case 16: m_Desc.ComponentType = VT_UINT16; break;
                case 32: m_Desc.ComponentType = VT_UINT32; break;
                default: LOG_ERROR_AND_THROW(BitsPerSample, " is not a valid UINT component bit depth. Only 8, 16 and 32 are allowed");
            }
            break;

        case SAMPLEFORMAT_INT:
            switch (BitsPerSample)
            {
                case 8: m_Desc.ComponentType = VT_INT8; break;
                case 16: m_Desc.ComponentType = VT_INT16; break;
                case 32: m_Desc.ComponentType = VT_INT32; break;
                default: LOG_ERROR_AND_THROW(BitsPerSample, " is not a valid INT component bit depth. Only 8, 16 and 32 are allowed");
            }
            break;

        case SAMPLEFORMAT_IEEEFP:
            switch (BitsPerSample)
            {
                case 16: m_Desc.ComponentType = VT_FLOAT16; break;
                case 32: m_Desc.ComponentType = VT_FLOAT32; break;
                default: LOG_ERROR_AND_THROW(BitsPerSample, " is not a valid FLOAT component bit depth. Only 16 and 32 are allowed");
            }
            break;

        case SAMPLEFORMAT_VOID:
            LOG_ERROR_AND_THROW("Untyped tif images are not supported");
            break;

        case SAMPLEFORMAT_COMPLEXINT:
            LOG_ERROR_AND_THROW("Complex int tif images are not supported");
            break;

        case SAMPLEFORMAT_COMPLEXIEEEFP:
            LOG_ERROR_AND_THROW("Complex floating point tif images are not supported");
            break;

        default:
            LOG_ERROR_AND_THROW("Unknown sample format: ", Uint32{SampleFormat});
    }

    auto ScanlineSize = TIFFScanlineSize(TiffFile);
    m_Desc.RowStride  = Align(static_cast<Uint32>(ScanlineSize), 4u);
    m_pData->Resize(size_t{m_Desc.Height} * size_t{m_Desc.RowStride});
    auto* pDataPtr = reinterpret_cast<Uint8*>(m_pData->GetDataPtr());
    for (Uint32 row = 0; row < m_Desc.Height; row++, pDataPtr += m_Desc.RowStride)
    {
        TIFFReadScanline(TiffFile, pDataPtr, row);
    }
    TIFFClose(TiffFile);
}


class PNGReadFnHelper
{
public:
    PNGReadFnHelper(IDataBlob* pData) :
        m_pData{pData},
        m_Offset{0}
    {
    }

    static void ReadData(png_structp pngPtr, png_bytep data, png_size_t length)
    {
        auto pThis = reinterpret_cast<PNGReadFnHelper*>(png_get_io_ptr(pngPtr));
        memcpy(data, reinterpret_cast<Uint8*>(pThis->m_pData->GetDataPtr()) + pThis->m_Offset, length);
        pThis->m_Offset += length;
    }

private:
    RefCntAutoPtr<IDataBlob> m_pData;
    size_t                   m_Offset;
};

void Image::LoadPngFile(IDataBlob* pFileData, const ImageLoadInfo& LoadInfo)
{
    // http://www.piko3d.net/tutorials/libpng-tutorial-loading-png-files-from-streams/
    // http://www.libpng.org/pub/png/book/chapter13.html#png.ch13.div.10
    // https://gist.github.com/niw/5963798

    PNGReadFnHelper ReadFnHelper(pFileData);

    const size_t    PngSigSize = 8;
    png_const_bytep pngsig     = reinterpret_cast<png_const_bytep>(pFileData->GetDataPtr());
    //Let LibPNG check the signature. If this function returns 0, everything is OK.
    if (png_sig_cmp(pngsig, 0, PngSigSize) != 0)
    {
        LOG_ERROR_AND_THROW("Invalid png signature");
    }

    png_structp png = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    VERIFY(png, "png_create_read_struct() failed");

    png_infop info = png_create_info_struct(png);
    VERIFY(info, "png_create_info_struct() failed");

    if (setjmp(png_jmpbuf(png)))
    {
        // When an error occurs during parsing, libPNG will jump to here
        png_destroy_read_struct(&png, &info, (png_infopp)0);
        LOG_ERROR_AND_THROW("Failed to read png file");
    }

    png_set_read_fn(png, (png_voidp)&ReadFnHelper, PNGReadFnHelper::ReadData);

    png_read_info(png, info);

    auto bit_depth = png_get_bit_depth(png, info);

    // PNG files store 16-bit pixels in network byte order (big-endian, ie
    // most significant bytes first). png_set_swap() shall switch the byte-order
    // to little-endian (ie, least significant bits first).
    if (bit_depth == 16)
        png_set_swap(png);

    auto color_type = png_get_color_type(png, info);

    // See http://www.libpng.org/pub/png/libpng-manual.txt
    if (color_type == PNG_COLOR_TYPE_PALETTE)
    {
        // Transform paletted images into 8-bit rgba
        png_set_palette_to_rgb(png);
        png_set_filler(png, 0xFF, PNG_FILLER_AFTER);
    }

    // PNG_COLOR_TYPE_GRAY_ALPHA is always 8 or 16bit depth.
    if (color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8)
    {
        // Expand 1, 2, or 4-bit images to 8-bit
        png_set_expand_gray_1_2_4_to_8(png);
    }

    if (png_get_valid(png, info, PNG_INFO_tRNS))
        png_set_tRNS_to_alpha(png);

#if 0
    // These color_type don't have an alpha channel then fill it with 0xff.
    if( color_type == PNG_COLOR_TYPE_RGB ||
        color_type == PNG_COLOR_TYPE_GRAY ||
        color_type == PNG_COLOR_TYPE_PALETTE )
        png_set_filler( png, 0xFF, PNG_FILLER_AFTER );

    if( color_type == PNG_COLOR_TYPE_GRAY ||
        color_type == PNG_COLOR_TYPE_GRAY_ALPHA )
        png_set_gray_to_rgb( png );
#endif

    png_read_update_info(png, info);

    bit_depth            = png_get_bit_depth(png, info);
    m_Desc.Width         = png_get_image_width(png, info);
    m_Desc.Height        = png_get_image_height(png, info);
    m_Desc.NumComponents = png_get_channels(png, info);
    switch (bit_depth)
    {
        case 8: m_Desc.ComponentType = VT_UINT8; break;
        case 16: m_Desc.ComponentType = VT_UINT16; break;
        case 32: m_Desc.ComponentType = VT_UINT32; break;
        default: LOG_ERROR_AND_THROW("Unsupported component bit depth: ", bit_depth, ". Only 8, 16 and 32-bit components are supported");
    }

    //Array of row pointers. One for every row.
    std::vector<png_bytep> rowPtrs(m_Desc.Height);

    //Alocate a buffer with enough space. Align stride to 4 bytes
    m_Desc.RowStride = Align(m_Desc.Width * static_cast<Uint32>(bit_depth) * m_Desc.NumComponents / 8u, 4u);
    m_pData->Resize(size_t{m_Desc.Height} * size_t{m_Desc.RowStride});
    for (size_t i = 0; i < m_Desc.Height; i++)
        rowPtrs[i] = reinterpret_cast<png_bytep>(m_pData->GetDataPtr()) + i * m_Desc.RowStride;

    //Read the imagedata and write it to the adresses pointed to
    //by rowptrs (in other words: our image databuffer)
    png_read_image(png, rowPtrs.data());

    png_destroy_read_struct(&png, &info, (png_infopp)0);
}



struct my_jpeg_error_mgr
{
    jpeg_error_mgr pub;
    char           padding[8];
    jmp_buf        setjmp_buffer; // for return to caller
};

// Here's the routine that will replace the standard error_exit method:
METHODDEF(void)
my_error_exit(j_common_ptr cinfo)
{
    // cinfo->err really points to a my_jpeg_error_mgr struct, so coerce pointer
    my_jpeg_error_mgr* myerr = (my_jpeg_error_mgr*)cinfo->err;

    /* Always display the message. */
    /* We could postpone this until after returning, if we chose. */
    (*cinfo->err->output_message)(cinfo);

    // Return control to the setjmp point
    longjmp(myerr->setjmp_buffer, 1);
}

void Image::LoadJpegFile(IDataBlob* pFileData, const ImageLoadInfo& LoadInfo)
{
    // https://github.com/LuaDist/libjpeg/blob/master/example.c

    // This struct contains the JPEG decompression parameters and pointers to
    // working space (which is allocated as needed by the JPEG library).
    jpeg_decompress_struct cinfo;

    // We use our private extension JPEG error handler.
    // Note that this struct must live as long as the main JPEG parameter
    // struct, to avoid dangling-pointer problems.
    my_jpeg_error_mgr jerr;

    // Step 1: allocate and initialize JPEG decompression object

    // We set up the normal JPEG error routines, then override error_exit.
    cinfo.err           = jpeg_std_error(&jerr.pub);
    jerr.pub.error_exit = my_error_exit;
    // Establish the setjmp return context for my_error_exit to use.
    if (setjmp(jerr.setjmp_buffer))
    {
        // If we get here, the JPEG code has signaled an error.
        // We need to clean up the JPEG object, close the input file, and return.
        jpeg_destroy_decompress(&cinfo);
        LOG_ERROR_AND_THROW("Failed to decompress JPEG image");
    }
    // Now we can initialize the JPEG decompression object.
    jpeg_create_decompress(&cinfo);

    // Step 2: specify data source
    jpeg_mem_src(&cinfo, reinterpret_cast<unsigned char*>(pFileData->GetDataPtr()), static_cast<unsigned long>(pFileData->GetSize()));

    // Step 3: read file parameters with jpeg_read_header()
    jpeg_read_header(&cinfo, TRUE);
    // We can ignore the return value from jpeg_read_header since
    //   (a) suspension is not possible with the stdio data source, and
    //   (b) we passed TRUE to reject a tables-only JPEG file as an error.
    // See libjpeg.txt for more info.


    // Step 4: set parameters for decompression

    // In this example, we don't need to change any of the defaults set by
    // jpeg_read_header(), so we do nothing here.


    // Step 5: Start decompressor

    jpeg_start_decompress(&cinfo);
    // We can ignore the return value since suspension is not possible
    // with the stdio data source.

    // We may need to do some setup of our own at this point before reading
    // the data.  After jpeg_start_decompress() we have the correct scaled
    // output image dimensions available, as well as the output colormap
    // if we asked for color quantization.

    m_Desc.Width         = cinfo.output_width;
    m_Desc.Height        = cinfo.output_height;
    m_Desc.ComponentType = VT_UINT8;
    m_Desc.NumComponents = cinfo.output_components;
    m_Desc.RowStride     = Align(m_Desc.Width * m_Desc.NumComponents, 4u);

    m_pData->Resize(size_t{m_Desc.RowStride} * size_t{m_Desc.Height});
    // Step 6: while (scan lines remain to be read)
    //           jpeg_read_scanlines(...);

    // Here we use the library's state variable cinfo.output_scanline as the
    // loop counter, so that we don't have to keep track ourselves.
    while (cinfo.output_scanline < cinfo.output_height)
    {
        // jpeg_read_scanlines expects an array of pointers to scanlines.
        // Here the array is only one element long, but you could ask for
        // more than one scanline at a time if that's more convenient.


        auto*    pDstScanline = reinterpret_cast<Uint8*>(m_pData->GetDataPtr()) + size_t{cinfo.output_scanline} * size_t{m_Desc.RowStride};
        JSAMPROW RowPtrs[]    = {reinterpret_cast<JSAMPROW>(pDstScanline)};
        jpeg_read_scanlines(&cinfo, RowPtrs, 1);
    }

    // Step 7: Finish decompression

    jpeg_finish_decompress(&cinfo);
    // We can ignore the return value since suspension is not possible
    // with the stdio data source.

    // Step 8: Release JPEG decompression object

    // This is an important step since it will release a good deal of memory.
    jpeg_destroy_decompress(&cinfo);

    // At this point you may want to check to see whether any corrupt-data
    // warnings occurred (test whether jerr.pub.num_warnings is nonzero).
}

Image::Image(IReferenceCounters*  pRefCounters,
             IDataBlob*           pFileData,
             const ImageLoadInfo& LoadInfo) :
    TBase{pRefCounters},
    m_pData{MakeNewRCObj<DataBlobImpl>()(0)}
{
    if (LoadInfo.Format == EImageFileFormat::tiff)
    {
        LoadTiffFile(pFileData, LoadInfo);
    }
    else if (LoadInfo.Format == EImageFileFormat::png)
    {
        LoadPngFile(pFileData, LoadInfo);
    }
    else if (LoadInfo.Format == EImageFileFormat::jpeg)
    {
        LoadJpegFile(pFileData, LoadInfo);
    }
    else if (LoadInfo.Format == EImageFileFormat::dds)
    {
        LOG_ERROR_MESSAGE("An image can't be created from DDS file. Use CreateTextureFromFile() or CreateTextureFromDDS() functions.");
    }
    else if (LoadInfo.Format == EImageFileFormat::ktx)
    {
        LOG_ERROR_MESSAGE("An image can't be created from KTX file. Use CreateTextureFromFile() or CreateTextureFromKTX() functions.");
    }
    else
    {
        LOG_ERROR_MESSAGE("Unknown image format.");
    }
}

void Image::CreateFromDataBlob(IDataBlob*           pFileData,
                               const ImageLoadInfo& LoadInfo,
                               Image**              ppImage)
{
    *ppImage = MakeNewRCObj<Image>()(pFileData, LoadInfo);
    (*ppImage)->AddRef();
}


static void WritePng(const Uint8* pData, Uint32 Width, Uint32 Height, Uint32 Stride, int PngColorType, IDataBlob* pEncodedData)
{
    struct PngWrapper
    {
        PngWrapper()
        {
            strct = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
            VERIFY(strct, "png_create_write_struct() failed");
            info = png_create_info_struct(strct);
            VERIFY(info, "png_create_info_struct() failed");
        }

        ~PngWrapper()
        {
            if (strct)
            {
                png_destroy_write_struct(&strct, &info);
            }
        }

        png_struct* strct = nullptr;
        png_info*   info  = nullptr;
    } Png;

    VERIFY(setjmp(png_jmpbuf(Png.strct)) == 0, "setjmp(png_jmpbuf(p) failed");
    png_set_IHDR(Png.strct, Png.info, Width, Height, 8,
                 PngColorType,
                 PNG_INTERLACE_NONE,
                 PNG_COMPRESSION_TYPE_DEFAULT,
                 PNG_FILTER_TYPE_DEFAULT);
    //png_set_compression_level(p, 1);
    std::vector<Uint8*> rows(Height);
    for (size_t y = 0; y < Height; ++y)
        rows[y] = const_cast<Uint8*>(pData) + y * Stride;
    png_set_rows(Png.strct, Png.info, rows.data());

    auto PngWriteCallback = [](png_structp png_ptr, png_bytep data, png_size_t length) {
        auto* pEncodedData = reinterpret_cast<IDataBlob*>(png_get_io_ptr(png_ptr));
        pEncodedData->Resize(pEncodedData->GetSize() + length);
        auto* pBytes = reinterpret_cast<Uint8*>(pEncodedData->GetDataPtr());
        memcpy(pBytes + pEncodedData->GetSize() - length, data, length);
    };

    png_set_write_fn(Png.strct, pEncodedData, PngWriteCallback, NULL);
    png_write_png(Png.strct, Png.info, PNG_TRANSFORM_IDENTITY, NULL);
}

static void WriteJPEG(JSAMPLE* pRGBData, Uint32 Width, Uint32 Height, int quality, IDataBlob* pEncodedData)
{
    /* This struct contains the JPEG compression parameters and pointers to
     * working space (which is allocated as needed by the JPEG library).
     * It is possible to have several such structures, representing multiple
     * compression/decompression processes, in existence at once.  We refer
     * to any one struct (and its associated working data) as a "JPEG object".
     */
    jpeg_compress_struct cinfo;

    /* This struct represents a JPEG error handler.  It is declared separately
     * because applications often want to supply a specialized error handler
     * (see the second half of this file for an example).  But here we just
     * take the easy way out and use the standard error handler, which will
     * print a message on stderr and call exit() if compression fails.
     * Note that this struct must live as long as the main JPEG parameter
     * struct, to avoid dangling-pointer problems.
     */
    jpeg_error_mgr jerr;

    /* Step 1: allocate and initialize JPEG compression object */

    /* We have to set up the error handler first, in case the initialization
     * step fails.  (Unlikely, but it could happen if you are out of memory.)
     * This routine fills in the contents of struct jerr, and returns jerr's
     * address which we place into the link field in cinfo.
     */
    cinfo.err = jpeg_std_error(&jerr);
    /* Now we can initialize the JPEG compression object. */
    jpeg_create_compress(&cinfo);

    /* Step 2: specify data destination (memory) */
    /* Note: steps 2 and 3 can be done in either order. */
    unsigned char* mem      = NULL;
    unsigned long  mem_size = 0;
    jpeg_mem_dest(&cinfo, &mem, &mem_size);

    /* Step 3: set parameters for compression */

    /* First we supply a description of the input image.
     * Four fields of the cinfo struct must be filled in:
     */
    cinfo.image_width      = Width; /* image width and height, in pixels */
    cinfo.image_height     = Height;
    cinfo.input_components = 3;       /* # of color components per pixel */
    cinfo.in_color_space   = JCS_RGB; /* colorspace of input image */
    /* Now use the library's routine to set default compression parameters.
     * (You must set at least cinfo.in_color_space before calling this,
     * since the defaults depend on the source color space.)
     */
    jpeg_set_defaults(&cinfo);
    /* Now you can set any non-default parameters you wish to.
     * Here we just illustrate the use of quality (quantization table) scaling:
     */
    jpeg_set_quality(&cinfo, quality, TRUE /* limit to baseline-JPEG values */);

    /* Step 4: Start compressor */

    /* TRUE ensures that we will write a complete interchange-JPEG file.
     * Pass TRUE unless you are very sure of what you're doing.
     */
    jpeg_start_compress(&cinfo, TRUE);

    /* Step 5: while (scan lines remain to be written) */
    /*           jpeg_write_scanlines(...); */

    /* Here we use the library's state variable cinfo.next_scanline as the
     * loop counter, so that we don't have to keep track ourselves.
     * To keep things simple, we pass one scanline per call; you can pass
     * more if you wish, though.
     */
    auto row_stride = Width * 3; /* JSAMPLEs per row in image_buffer */

    while (cinfo.next_scanline < cinfo.image_height)
    {
        /* jpeg_write_scanlines expects an array of pointers to scanlines.
         * Here the array is only one element long, but you could pass
         * more than one scanline at a time if that's more convenient.
         */
        JSAMPROW row_pointer[1] = {&pRGBData[cinfo.next_scanline * row_stride]};
        jpeg_write_scanlines(&cinfo, row_pointer, 1);
    }

    /* Step 6: Finish compression */
    jpeg_finish_compress(&cinfo);

    pEncodedData->Resize(mem_size);
    memcpy(pEncodedData->GetDataPtr(), mem, mem_size);

    /* After finish_compress, we can free memory buffer. */
    free(mem);

    /* Step 7: release JPEG compression object */

    /* This is an important step since it will release a good deal of memory. */
    jpeg_destroy_compress(&cinfo);
}

static const std::array<Uint8, 4> GetRGBAOffsets(TEXTURE_FORMAT Format)
{
    switch (Format)
    {
        case TEX_FORMAT_BGRA8_TYPELESS:
        case TEX_FORMAT_BGRA8_UNORM:
        case TEX_FORMAT_BGRA8_UNORM_SRGB:
            return {{2, 1, 0, 3}};
        default:
            return {{0, 1, 2, 3}};
    }
}

std::vector<Uint8> Image::ConvertImageData(Uint32         Width,
                                           Uint32         Height,
                                           const Uint8*   pData,
                                           Uint32         Stride,
                                           TEXTURE_FORMAT SrcFormat,
                                           TEXTURE_FORMAT DstFormat,
                                           bool           KeepAlpha)
{
    const auto& SrcFmtAttribs = GetTextureFormatAttribs(SrcFormat);
    const auto& DstFmtAttribs = GetTextureFormatAttribs(DstFormat);
    VERIFY(SrcFmtAttribs.ComponentSize == 1, "Only 8-bit formats are currently supported");
    VERIFY(DstFmtAttribs.ComponentSize == 1, "Only 8-bit formats are currently supported");

    auto NumDstComponents = SrcFmtAttribs.NumComponents;
    if (!KeepAlpha)
        NumDstComponents = std::min(NumDstComponents, Uint8{3});

    auto SrcOffsets = GetRGBAOffsets(SrcFormat);
    auto DstOffsets = GetRGBAOffsets(DstFormat);

    std::vector<Uint8> ConvertedData(DstFmtAttribs.ComponentSize * NumDstComponents * Width * Height);

    for (Uint32 j = 0; j < Height; ++j)
    {
        for (Uint32 i = 0; i < Width; ++i)
        {
            for (Uint32 c = 0; c < NumDstComponents; ++c)
            {
                ConvertedData[j * Width * NumDstComponents + i * NumDstComponents + DstOffsets[c]] =
                    pData[j * Stride + i * SrcFmtAttribs.NumComponents + SrcOffsets[c]];
            }
        }
    }

    return ConvertedData;
}


void Image::Encode(const EncodeInfo& Info, IDataBlob** ppEncodedData)
{
    RefCntAutoPtr<IDataBlob> pEncodedData(MakeNewRCObj<DataBlobImpl>()(0));
    if (Info.FileFormat == EImageFileFormat::jpeg)
    {
        auto RGBData = ConvertImageData(Info.Width, Info.Height, reinterpret_cast<const Uint8*>(Info.pData), Info.Stride, Info.TexFormat, TEX_FORMAT_RGBA8_UNORM, false);
        WriteJPEG(RGBData.data(), Info.Width, Info.Height, Info.JpegQuality, pEncodedData);
    }
    else if (Info.FileFormat == EImageFileFormat::png)
    {
        const auto*        pData  = reinterpret_cast<const Uint8*>(Info.pData);
        auto               Stride = Info.Stride;
        std::vector<Uint8> ConvertedData;
        if (!((Info.TexFormat == TEX_FORMAT_RGBA8_UNORM || Info.TexFormat == TEX_FORMAT_RGBA8_UNORM_SRGB) && Info.KeepAlpha))
        {
            ConvertedData = ConvertImageData(Info.Width, Info.Height, reinterpret_cast<const Uint8*>(Info.pData), Info.Stride, Info.TexFormat, TEX_FORMAT_RGBA8_UNORM, Info.KeepAlpha);
            pData         = ConvertedData.data();
            Stride        = Info.Width * (Info.KeepAlpha ? 4 : 3);
        }

        WritePng(pData, Info.Width, Info.Height, Stride, Info.KeepAlpha ? PNG_COLOR_TYPE_RGBA : PNG_COLOR_TYPE_RGB, pEncodedData);
    }
    else
    {
        UNSUPPORTED("Unsupported image file format");
    }
    pEncodedData->QueryInterface(IID_DataBlob, reinterpret_cast<IObject**>(ppEncodedData));
}

EImageFileFormat Image::GetFileFormat(const Uint8* pData, size_t Size)
{
    if (Size >= 3 && pData[0] == 0xFF && pData[1] == 0xD8 && pData[2] == 0xFF)
        return EImageFileFormat::jpeg;

    if (Size >= 8 &&
        pData[0] == 0x89 && pData[1] == 0x50 && pData[2] == 0x4E && pData[3] == 0x47 &&
        pData[4] == 0x0D && pData[5] == 0x0A && pData[6] == 0x1A && pData[7] == 0x0A)
        return EImageFileFormat::png;

    if (Size >= 4 &&
        ((pData[0] == 0x49 && pData[1] == 0x20 && pData[2] == 0x49) ||
         (pData[0] == 0x49 && pData[1] == 0x49 && pData[2] == 0x2A && pData[3] == 0x00) ||
         (pData[0] == 0x4D && pData[1] == 0x4D && pData[2] == 0x00 && pData[3] == 0x2A) ||
         (pData[0] == 0x4D && pData[1] == 0x4D && pData[2] == 0x00 && pData[3] == 0x2B)))
        return EImageFileFormat::tiff;

    if (Size >= 4 && pData[0] == 0x44 && pData[1] == 0x44 && pData[2] == 0x53 && pData[3] == 0x20)
        return EImageFileFormat::dds;

    static constexpr Uint8 KTX10FileIdentifier[12] = {0xAB, 0x4B, 0x54, 0x58, 0x20, 0x31, 0x31, 0xBB, 0x0D, 0x0A, 0x1A, 0x0A};
    static constexpr Uint8 KTX20FileIdentifier[12] = {0xAB, 0x4B, 0x54, 0x58, 0x20, 0x32, 0x30, 0xBB, 0x0D, 0x0A, 0x1A, 0x0A};
    if (Size >= 12 &&
        (memcmp(pData, KTX10FileIdentifier, sizeof(KTX10FileIdentifier)) == 0 ||
         memcmp(pData, KTX20FileIdentifier, sizeof(KTX20FileIdentifier)) == 0))
        return EImageFileFormat::ktx;

    return EImageFileFormat::unknown;
}


EImageFileFormat CreateImageFromFile(const Char* FilePath,
                                     Image**     ppImage,
                                     IDataBlob** ppRawData)
{
    EImageFileFormat ImgFileFormat = EImageFileFormat::unknown;
    try
    {
        RefCntAutoPtr<BasicFileStream> pFileStream(MakeNewRCObj<BasicFileStream>()(FilePath, EFileAccessMode::Read));
        if (!pFileStream->IsValid())
            LOG_ERROR_AND_THROW("Failed to open image file \"", FilePath, '\"');

        RefCntAutoPtr<IDataBlob> pFileData(MakeNewRCObj<DataBlobImpl>()(0));
        pFileStream->ReadBlob(pFileData);

        ImgFileFormat = Image::GetFileFormat(reinterpret_cast<Uint8*>(pFileData->GetDataPtr()), pFileData->GetSize());
        if (ImgFileFormat == EImageFileFormat::unknown)
        {
            LOG_WARNING_MESSAGE("Unable to derive image format from the header for file \"", FilePath, "\". Trying to analyze extension.");

            // Try to use extension to derive format
            auto* pDotPos = strrchr(FilePath, '.');
            if (pDotPos == nullptr)
                LOG_ERROR_AND_THROW("Unable to recognize file format: file name \"", FilePath, "\" does not contain extension");

            auto* pExtension = pDotPos + 1;
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

        if (ImgFileFormat == EImageFileFormat::png ||
            ImgFileFormat == EImageFileFormat::jpeg ||
            ImgFileFormat == EImageFileFormat::tiff)
        {
            ImageLoadInfo ImgLoadInfo;
            ImgLoadInfo.Format = ImgFileFormat;
            Image::CreateFromDataBlob(pFileData, ImgLoadInfo, ppImage);
        }
        else if (ppRawData != nullptr)
        {
            *ppRawData = pFileData.Detach();
        }
    }
    catch (std::runtime_error& err)
    {
        LOG_ERROR("Failed to create image from file: ", err.what());
    }

    return ImgFileFormat;
}

} // namespace Diligent
