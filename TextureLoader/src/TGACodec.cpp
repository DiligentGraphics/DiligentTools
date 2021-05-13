#include "TGACodec.hpp"

#include <cstring>

/*
 * This Decoder is made from source at https://stackoverflow.com/a/20596072/6278261
 */

typedef union PixelInfo
{
	std::uint32_t Colour;
	struct
	{
		std::uint8_t R, G, B, A;
	};
} *PPixelInfo;

DILIGENT_BEGIN_NAMESPACE(Diligent)

	DECODE_TGA_RESULT DecodeTGA(IDataBlob* pSrcTgaBits,
										   IDataBlob* pDstPixels,
										   ImageDesc* pDstImgDesc)
	{
		if (!pSrcTgaBits || !pDstPixels || !pDstImgDesc)
			return DECODE_TGA_RESULT_INVALID_ARGUMENTS;

		const Uint8* pData = reinterpret_cast<const Uint8*>(pSrcTgaBits->GetDataPtr());
		const auto DataSize = pSrcTgaBits->GetSize();
		std::uint8_t dataOffset = 0;

		bool ImageCompressed;
		std::uint32_t width, height, size, BitsPerPixel;


		std::uint8_t Header[18] = {0};
		std::vector<std::uint8_t> ImageData;
		static std::uint8_t DeCompressed[12] = {0x0, 0x0, 0x2, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0};
		static std::uint8_t IsCompressed[12] = {0x0, 0x0, 0xA, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0};

		if(!std::memcpy(Header, pData, sizeof(Header))) {
			LOG_ERROR_AND_THROW("Could not decode TGA header");
			return DECODE_TGA_RESULT_DECODING_ERROR;
		}

		dataOffset += sizeof(Header);

		if (!std::memcmp(DeCompressed, &Header, sizeof(DeCompressed))) // File is not compressed
		{
			BitsPerPixel = Header[16];
			width  = Header[13] * 256 + Header[12];
			height = Header[15] * 256 + Header[14];
			size  = ((width * BitsPerPixel + 31) / 32) * 4 * height;

			if ((BitsPerPixel != 24) && (BitsPerPixel != 32))
			{
				LOG_ERROR_MESSAGE("Invalid File Format. Required: 24 or 32 Bit Image.");
			}

			ImageData.resize(size);
			ImageCompressed = false;
			std::memcpy(ImageData.data(), pData, size);
		} else if (!std::memcmp(IsCompressed, &Header, sizeof(IsCompressed))) // Image is compressed
		{
			BitsPerPixel = Header[16];
			width  = Header[13] * 256 + Header[12];
			height = Header[15] * 256 + Header[14];
			size  = ((width * BitsPerPixel + 31) / 32) * 4 * height;

			if ((BitsPerPixel != 24) && (BitsPerPixel != 32))
			{
				LOG_ERROR_MESSAGE("Invalid File Format. Required: 24 or 32 Bit Image.");
			}

			PixelInfo Pixel = {0};
			int CurrentByte = 0;
			std::size_t CurrentPixel = 0;
			ImageCompressed = true;
			std::uint8_t ChunkHeader = {0};
			std::uint32_t BytesPerPixel = (BitsPerPixel / 8);
			ImageData.resize(width * height * sizeof(PixelInfo));

			do
			{
				std::memcpy(&ChunkHeader, pData + dataOffset, sizeof(ChunkHeader));
				dataOffset += sizeof(ChunkHeader);

				if(ChunkHeader < 128)
				{
					++ChunkHeader;
					for(int I = 0; I < ChunkHeader; ++I, ++CurrentPixel)
					{
						std::memcpy(&Pixel, pData + dataOffset, BytesPerPixel);
						dataOffset += BytesPerPixel;

						ImageData[CurrentByte++] = Pixel.B;
						ImageData[CurrentByte++] = Pixel.G;
						ImageData[CurrentByte++] = Pixel.R;
						if (BitsPerPixel > 24) ImageData[CurrentByte++] = Pixel.A;
					}
				}
				else
				{
					ChunkHeader -= 127;
					std::memcpy(&Pixel, pData + dataOffset, BytesPerPixel);
					dataOffset += BytesPerPixel;

					for(int I = 0; I < ChunkHeader; ++I, ++CurrentPixel)
					{
						ImageData[CurrentByte++] = Pixel.B;
						ImageData[CurrentByte++] = Pixel.G;
						ImageData[CurrentByte++] = Pixel.R;
						if (BitsPerPixel > 24) ImageData[CurrentByte++] = Pixel.A;
					}
				}
			} while(CurrentPixel < (width * height));
		}

		pDstImgDesc->Width         = width;
		pDstImgDesc->Height        = height;
		pDstImgDesc->ComponentType = VT_UINT8;
		pDstImgDesc->NumComponents = BitsPerPixel / 8;
		pDstImgDesc->RowStride     = pDstImgDesc->Width * pDstImgDesc->NumComponents;
		pDstImgDesc->RowStride     = (pDstImgDesc->RowStride + 3u) & ~3u;

		pDstPixels->Resize(pDstImgDesc->RowStride * pDstImgDesc->Height);

		std::memcpy(pDstPixels->GetDataPtr(), ImageData.data(), pDstImgDesc->RowStride * pDstImgDesc->Height);

		LOG_INFO_MESSAGE("tga header loaded");

		return DECODE_TGA_RESULT_OK;
	}

DILIGENT_END_NAMESPACE // namespace Diligent