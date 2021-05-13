#pragma once

#include "../interface/Image.h"

DILIGENT_BEGIN_NAMESPACE(Diligent)

/// TGA image decoding result.
	DILIGENT_TYPED_ENUM(DECODE_TGA_RESULT, Uint32)
	{
		/// JPEG image was decoded successfully.
		DECODE_TGA_RESULT_OK = 0,

		/// Invalid arguments (e.g. null pointer).
		DECODE_TGA_RESULT_INVALID_ARGUMENTS,

		/// Failed to initialize the decoder.
		DECODE_TGA_RESULT_INITIALIZATION_FAILED,

		/// An unexpected error occurred while decoding the file.
		DECODE_TGA_RESULT_DECODING_ERROR
	};

/// Decodes jpeg image.

/// \param [in]  pSrcJpegBits - JPEG image encoded bits.
/// \param [out] pDstPixels   - Decoded pixels data blob. The pixels are always tightly packed
///                             (for instance, components of 3-channel image will be written as |r|g|b|r|g|b|r|g|b|...).
/// \param [out] pDstImgDesc  - Decoded image description.
/// \return                     Decoding result, see Diligent::DECODE_JPEG_RESULT.
	DECODE_TGA_RESULT DILIGENT_GLOBAL_FUNCTION(DecodeTGA)(IDataBlob* pSrcTgaBits,
															IDataBlob* pDstPixels,
															ImageDesc* pDstImgDesc);

DILIGENT_END_NAMESPACE // namespace Diligent