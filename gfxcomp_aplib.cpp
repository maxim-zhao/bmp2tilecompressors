#include <cstdint>
#include <algorithm>
#include "aPLib/aplib.h"

int compress(uint8_t* source, uint32_t sourceLen, uint8_t* dest, uint32_t destLen)
{
	// Allocate memory
	auto* pPacked = new uint8_t[aP_max_packed_size(sourceLen)];
	auto* pWorkMemory = new uint8_t[aP_workmem_size(sourceLen)];

	unsigned int size = aP_pack(source, pPacked, sourceLen, pWorkMemory, nullptr, nullptr);

	// Free work memory
	delete [] pWorkMemory;

	// Check size
	if (size > destLen)
	{
		delete [] pPacked;
		return 0;
	}

	// Copy to dest
	memcpy_s(dest, destLen, pPacked, size);

	// Free other memory
	delete [] pPacked;

	// Done
	return size;
}

extern "C" __declspec(dllexport) const char* getName()
{
	// A pretty name for this compression type
	// Generally, the name of the game it was REd from
	return "aPLib";
}

extern "C" __declspec(dllexport) const char* getExt()
{
	// A string suitable for use as a file extension
	return "aPLib";
}

extern "C" __declspec(dllexport) uint32_t compressTiles(uint8_t* source, uint32_t numTiles, uint8_t* dest, uint32_t destLen)
{
	// Compress tiles
	return compress(source, numTiles * 32, dest, destLen);
}

extern "C" __declspec(dllexport) uint32_t compressTilemap(uint8_t* source, uint32_t width, uint32_t height, uint8_t* dest, uint32_t destLen)
{
	// Compress tilemap
	return compress(source, width * height * 2, dest, destLen);
}

