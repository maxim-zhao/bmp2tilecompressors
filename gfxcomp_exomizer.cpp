#define _SCL_SECURE_NO_WARNINGS

#include <cstdint>
#include <algorithm>

extern "C" {
#include "exomizer/src/membuf.h"
#include "exomizer/src/exo_helper.h"
}

extern "C" __declspec(dllexport) const char* getName()
{
	// A pretty name for this compression type
	return "Exomizer";
}

extern "C" __declspec(dllexport) const char* getExt()
{
	// A string suitable for use as a file extension
	return "exomizer";
}

// The actual compressor function
uint32_t compress(uint8_t* pSource, uint32_t sourceLength, uint8_t* pDestination, uint32_t destinationLength)
{
	struct membuf inbuf;
	membuf_init(&inbuf);
	membuf_append(&inbuf, pSource, sourceLength);

	struct membuf outbuf;
	membuf_init(&outbuf);

	crunch(&inbuf, &outbuf, nullptr, nullptr);

	membuf_free(&inbuf);

	uint32_t length = membuf_memlen(&outbuf);
	if (length > destinationLength)
	{
		membuf_free(&outbuf);
		return 0;
	}

	const uint8_t* pBegin = static_cast<const uint8_t*>(membuf_get(&outbuf));

	memcpy_s(pDestination, destinationLength, pBegin, length);

	membuf_free(&outbuf);
	return length;
}

extern "C" __declspec(dllexport) uint32_t compressTiles(uint8_t* pSource, uint32_t numTiles, uint8_t* pDestination, uint32_t destinationLength)
{
	return compress(pSource, numTiles * 32, pDestination, destinationLength);
}

extern "C" __declspec(dllexport) uint32_t compressTilemap(uint8_t* pSource, uint32_t width, uint32_t height, uint8_t* pDestination, uint32_t destinationLength)
{
	// Compress tilemap
	return compress(pSource, width * height * 2, pDestination, destinationLength);
}
