#include <cstdint>
#include <algorithm>
#include "aPLib/aplib.h"

int compress(const uint8_t* pSource, const size_t sourceLength, uint8_t* pDestination, const size_t destinationLength)
{
	// Allocate memory
	auto* pPacked = new uint8_t[aP_max_packed_size(sourceLength)];
	auto* pWorkMemory = new uint8_t[aP_workmem_size(sourceLength)];

	const auto size = aP_pack(pSource, pPacked, sourceLength, pWorkMemory, nullptr, nullptr);

	// Free work memory
	delete [] pWorkMemory;

	// Check size
	if (size > destinationLength)
	{
		delete [] pPacked;
		return 0;
	}

	// Copy to pDestination
	memcpy_s(pDestination, destinationLength, pPacked, size);

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

extern "C" __declspec(dllexport) int compressTiles(const uint8_t* pSource, const uint32_t numTiles, uint8_t* pDestination, const uint32_t destinationLength)
{
	// Compress tiles
	return compress(pSource, numTiles * 32, pDestination, destinationLength);
}

extern "C" __declspec(dllexport) int compressTilemap(const uint8_t* pSource, const uint32_t width, const uint32_t height, uint8_t* pDestination, const uint32_t destinationLength)
{
	// Compress tilemap
	return compress(pSource, width * height * 2, pDestination, destinationLength);
}

