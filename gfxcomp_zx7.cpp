#include "zx7/zx7.h"
#include <cstdint>
#include <algorithm>

extern "C" __declspec(dllexport) const char* getName()
{
	// A pretty name for this compression type
	return "ZX7";
}

extern "C" __declspec(dllexport) const char* getExt()
{
	// A string suitable for use as a file extension
	return "zx7";
}

// The actual compressor function, calling into the zx7 code
uint32_t compress(uint8_t* pSource, uint32_t sourceLength, uint8_t* pDestination, uint32_t destinationLength)
{
	// The compressor allocates using malloc
	size_t outputSize;
	unsigned char* pOutputData = compress(optimize(pSource, sourceLength), pSource, sourceLength, &outputSize);

	if (outputSize > destinationLength)
	{
		free(pOutputData);
		return 0;
	}

	memcpy_s(pDestination, destinationLength, pOutputData, outputSize);
	free(pOutputData);

	return outputSize;
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
