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
#include "Image.h"
#include "Errors.h"

#include "tiffio.h"
#include "png.h"
#include "jpeglib.h"

#include "DataBlobImpl.h"
#include "DebugUtilities.h"
#include "RefCntAutoPtr.h"

using namespace Diligent;
namespace
{
    template <typename T>
    inline T Align(T v, T align)
    {
        VERIFY( (align & (align-1)) == 0, "Alignment must be power of 2");
        return (v + (align-1)) & ~(align-1);
    }
}

namespace Diligent
{
    class TIFFClientOpenWrapper
    {
    public:
        TIFFClientOpenWrapper( IDataBlob *pData ) :
            m_Offset( 0 ),
            m_Size( pData->GetSize() ),
            m_pData( pData )
        {
        }

        static tmsize_t TIFFReadProc( thandle_t pClientData, void* pBuffer, tmsize_t Size )
        {
            auto *pThis = reinterpret_cast<TIFFClientOpenWrapper*>(pClientData);
            auto *pSrcPtr = reinterpret_cast<Uint8*>(pThis->m_pData->GetDataPtr()) + pThis->m_Offset;
            memcpy( pBuffer, pSrcPtr, Size );
            pThis->m_Offset += Size;
            return Size;
        }

        static tmsize_t TIFFWriteProc( thandle_t pClientData, void* pBuffer, tmsize_t Size )
        {
            auto *pThis = reinterpret_cast<TIFFClientOpenWrapper*>(pClientData);
            if( pThis->m_Offset + Size > pThis->m_Size )
            {
                pThis->m_Size = pThis->m_Offset + Size;
                pThis->m_pData->Resize( pThis->m_Size );
            }
            auto *pDstPtr = reinterpret_cast<Uint8*>(pThis->m_pData->GetDataPtr()) + pThis->m_Offset;
            memcpy( pDstPtr, pBuffer, Size );
            pThis->m_Offset += Size;
            return Size;
        }

        static toff_t TIFFSeekProc( thandle_t pClientData, toff_t Offset, int Whence )
        {
            auto *pThis = reinterpret_cast<TIFFClientOpenWrapper*>(pClientData);
            switch( Whence )
            {
                case SEEK_SET: pThis->m_Offset = static_cast<size_t>(Offset); break;
                case SEEK_CUR: pThis->m_Offset += static_cast<size_t>(Offset); break;
                case SEEK_END: pThis->m_Offset = pThis->m_Size + static_cast<size_t>(Offset); break;
                default: UNEXPECTED( "Unexpected whence" );
            }

            return pThis->m_Offset;
        }

        static int TIFFCloseProc( thandle_t pClientData )
        {
            auto *pThis = reinterpret_cast<TIFFClientOpenWrapper*>(pClientData);
            pThis->m_pData.Release();
            pThis->m_Size = 0;
            pThis->m_Offset = 0;
            return 0;
        }

        static toff_t TIFFSizeProc( thandle_t pClientData )
        {
            auto *pThis = reinterpret_cast<TIFFClientOpenWrapper*>(pClientData);
            return pThis->m_Size;
        }

        static int TIFFMapFileProc( thandle_t pClientData, void** base, toff_t* size )
        {
            UNEXPECTED( "Client file mapping is not implemented. Use \'m\' when opening TIFF file to disable file mapping." );
            return 0;
        }

        static void TIFFUnmapFileProc( thandle_t pClientData, void* base, toff_t size )
        {
            UNEXPECTED( "Client file mapping is not implemented. Use \'m\' when opening TIFF file to disable file mapping." );
        }

    private:
        size_t m_Offset;
        size_t m_Size;
        RefCntAutoPtr<IDataBlob> m_pData;
    };

    void Image::LoadTiffFile( IDataBlob *pFileData, const ImageLoadInfo& LoadInfo )
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
        m_Desc.BitsPerPixel = m_Desc.NumComponents * BitsPerSample;

        auto ScanlineSize = TIFFScanlineSize(TiffFile);
        m_Desc.RowStride = Align(static_cast<Uint32>( ScanlineSize ), 4u);
        m_pData->Resize(m_Desc.Height * m_Desc.RowStride );
        auto *pDataPtr = reinterpret_cast<Uint8*>( m_pData->GetDataPtr() );
        for (Uint32 row = 0; row < m_Desc.Height; row++, pDataPtr += m_Desc.RowStride)
        {
            TIFFReadScanline(TiffFile, pDataPtr, row);
        }
        TIFFClose(TiffFile);
    }


    class PNGReadFnHelper
    {
    public:
        PNGReadFnHelper( IDataBlob *pData ) :
            m_pData( pData ),
            m_Offset( 0 )
        {
        }

        static void ReadData( png_structp pngPtr, png_bytep data, png_size_t length )
        {
            auto pThis = reinterpret_cast<PNGReadFnHelper*>( png_get_io_ptr(pngPtr) );
            memcpy( data, reinterpret_cast<Uint8*>(pThis->m_pData->GetDataPtr()) + pThis->m_Offset, length );
            pThis->m_Offset += length;
        }

    private:
        RefCntAutoPtr<IDataBlob> m_pData;
        size_t m_Offset;
    };

    void Image::LoadPngFile( IDataBlob *pFileData, const ImageLoadInfo& LoadInfo )
    {
        // http://www.piko3d.net/tutorials/libpng-tutorial-loading-png-files-from-streams/
        // http://www.libpng.org/pub/png/book/chapter13.html#png.ch13.div.10
        // https://gist.github.com/niw/5963798\

        PNGReadFnHelper ReadFnHelper(pFileData);

        const size_t PngSigSize = 8;
        png_const_bytep pngsig = reinterpret_cast<png_const_bytep>(pFileData->GetDataPtr());
        //Let LibPNG check the signature. If this function returns 0, everything is OK.
        if( png_sig_cmp( pngsig, 0, PngSigSize ) != 0 )
        {
            LOG_ERROR_AND_THROW( "Invalid png signature" );
        }

        png_structp png = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
        VERIFY(png, "png_create_read_struct() failed");
 
        png_infop info = png_create_info_struct(png);
        VERIFY(info, "png_create_info_struct() failed");
           
        if( setjmp( png_jmpbuf( png ) ) )
        {
            // When an error occurs during parsing, libPNG will jump to here
            png_destroy_read_struct(&png, &info, (png_infopp)0);
            LOG_ERROR_AND_THROW( "Failed to read png file" );
        }

        png_set_read_fn(png, (png_voidp)&ReadFnHelper, PNGReadFnHelper::ReadData);

        png_read_info(png, info);

        m_Desc.Width  = png_get_image_width(png, info);
        m_Desc.Height = png_get_image_height(png, info);
        m_Desc.NumComponents = png_get_channels(png, info);
        auto bit_depth  = png_get_bit_depth(png, info);
        m_Desc.BitsPerPixel = bit_depth * m_Desc.NumComponents;
        
        // PNG files store 16-bit pixels in network byte order (big-endian, ie 
        // most significant bytes first). png_set_swap() shall switch the byte-order 
        // to little-endian (ie, least significant bits first).
        if( bit_depth == 16 )
            png_set_swap(png);

#if 0
        auto color_type = png_get_color_type(png, info);
        // Read any color_type into 8bit depth, RGBA format.
        // See http://www.libpng.org/pub/png/libpng-manual.txt

        if( bit_depth == 16 )
            png_set_strip_16( png );

        if( color_type == PNG_COLOR_TYPE_PALETTE )
            png_set_palette_to_rgb( png );

        // PNG_COLOR_TYPE_GRAY_ALPHA is always 8 or 16bit depth.
        if( color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8 )
            png_set_expand_gray_1_2_4_to_8( png );

        if( png_get_valid( png, info, PNG_INFO_tRNS ) )
            png_set_tRNS_to_alpha( png );

        // These color_type don't have an alpha channel then fill it with 0xff.
        if( color_type == PNG_COLOR_TYPE_RGB ||
            color_type == PNG_COLOR_TYPE_GRAY ||
            color_type == PNG_COLOR_TYPE_PALETTE )
            png_set_filler( png, 0xFF, PNG_FILLER_AFTER );

        if( color_type == PNG_COLOR_TYPE_GRAY ||
            color_type == PNG_COLOR_TYPE_GRAY_ALPHA )
            png_set_gray_to_rgb( png );
        
        png_read_update_info( png, info );
#endif
    
        //Array of row pointers. One for every row.
        std::vector<png_bytep> rowPtrs(m_Desc.Height);

        //Alocate a buffer with enough space. Align stride to 4 bytes
        m_Desc.RowStride = Align(m_Desc.Width * bit_depth * m_Desc.NumComponents / 8, 4u);
        m_pData->Resize( m_Desc.Height * m_Desc.RowStride );
        for( size_t i = 0; i < m_Desc.Height; i++ )
            rowPtrs[i] = reinterpret_cast<png_bytep>(m_pData->GetDataPtr()) + i * m_Desc.RowStride;

        //Read the imagedata and write it to the adresses pointed to
        //by rowptrs (in other words: our image databuffer)
        png_read_image( png, rowPtrs.data() );

        png_destroy_read_struct(&png, &info, (png_infopp)0);
    }



    struct my_jpeg_error_mgr {
      jpeg_error_mgr pub;
      jmp_buf setjmp_buffer;// for return to caller
    };

    // Here's the routine that will replace the standard error_exit method:
    METHODDEF(void)
    my_error_exit (j_common_ptr cinfo)
    {
      // cinfo->err really points to a my_jpeg_error_mgr struct, so coerce pointer
      my_jpeg_error_mgr* myerr = (my_jpeg_error_mgr*) cinfo->err;

      /* Always display the message. */
      /* We could postpone this until after returning, if we chose. */
      (*cinfo->err->output_message) (cinfo);

      // Return control to the setjmp point
      longjmp(myerr->setjmp_buffer, 1);
    }

    void Image::LoadJpegFile( IDataBlob *pFileData, const ImageLoadInfo& LoadInfo )
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
        cinfo.err = jpeg_std_error( &jerr.pub );
        jerr.pub.error_exit = my_error_exit;
        // Establish the setjmp return context for my_error_exit to use.
        if( setjmp( jerr.setjmp_buffer ) )
        {
            // If we get here, the JPEG code has signaled an error.
            // We need to clean up the JPEG object, close the input file, and return.
            jpeg_destroy_decompress( &cinfo );
            LOG_ERROR_AND_THROW( "Failed to decompress JPEG image" );
        }
        // Now we can initialize the JPEG decompression object.
        jpeg_create_decompress( &cinfo );

        // Step 2: specify data source
        jpeg_mem_src( &cinfo, reinterpret_cast<unsigned char*>(pFileData->GetDataPtr()), static_cast<unsigned long>(pFileData->GetSize()) );

        // Step 3: read file parameters with jpeg_read_header()
        jpeg_read_header( &cinfo, TRUE );
        // We can ignore the return value from jpeg_read_header since
        //   (a) suspension is not possible with the stdio data source, and
        //   (b) we passed TRUE to reject a tables-only JPEG file as an error.
        // See libjpeg.txt for more info.


        // Step 4: set parameters for decompression

        // In this example, we don't need to change any of the defaults set by
        // jpeg_read_header(), so we do nothing here.


        // Step 5: Start decompressor

        jpeg_start_decompress( &cinfo );
        // We can ignore the return value since suspension is not possible
        // with the stdio data source.

        // We may need to do some setup of our own at this point before reading
        // the data.  After jpeg_start_decompress() we have the correct scaled
        // output image dimensions available, as well as the output colormap
        // if we asked for color quantization.

        m_Desc.Width = cinfo.output_width;
        m_Desc.Height = cinfo.output_height;
        m_Desc.NumComponents = cinfo.output_components;
        m_Desc.RowStride = Align(m_Desc.Width * m_Desc.NumComponents, 4u);
        m_Desc.BitsPerPixel = 8 * m_Desc.NumComponents;

        m_pData->Resize(m_Desc.RowStride * m_Desc.Height);
        // Step 6: while (scan lines remain to be read)
        //           jpeg_read_scanlines(...);

        // Here we use the library's state variable cinfo.output_scanline as the
        // loop counter, so that we don't have to keep track ourselves.
        while( cinfo.output_scanline < cinfo.output_height ) {
            // jpeg_read_scanlines expects an array of pointers to scanlines.
            // Here the array is only one element long, but you could ask for
            // more than one scanline at a time if that's more convenient.


            auto *pDstScanline = reinterpret_cast<Uint8*>( m_pData->GetDataPtr() ) + cinfo.output_scanline * m_Desc.RowStride;
            JSAMPROW RowPtrs[] = { reinterpret_cast<JSAMPROW>(pDstScanline) };
            jpeg_read_scanlines( &cinfo, RowPtrs, 1 );
        }

        // Step 7: Finish decompression

        jpeg_finish_decompress( &cinfo );
        // We can ignore the return value since suspension is not possible
        // with the stdio data source.

        // Step 8: Release JPEG decompression object

        // This is an important step since it will release a good deal of memory.
        jpeg_destroy_decompress( &cinfo );

        // At this point you may want to check to see whether any corrupt-data
        // warnings occurred (test whether jerr.pub.num_warnings is nonzero).
    }

    Image::Image( IReferenceCounters *pRefCounters,
                  IFileStream *pSrcFile, 
                  const ImageLoadInfo& LoadInfo ) : 
        TBase(pRefCounters),
        m_pData( MakeNewRCObj<DataBlobImpl>()(0) )
    {
        RefCntAutoPtr<IDataBlob> pFileData( MakeNewRCObj<DataBlobImpl>()(0) );
        pSrcFile->Read(pFileData);

        if( LoadInfo.Format == EImageFileFormat::tiff )
        {
            LoadTiffFile(pFileData, LoadInfo );
        }
        else if( LoadInfo.Format == EImageFileFormat::png )
        {
            LoadPngFile(pFileData, LoadInfo );
        }
        else if( LoadInfo.Format == EImageFileFormat::jpeg )
        {
            LoadJpegFile(pFileData, LoadInfo );
        }
    }
}
