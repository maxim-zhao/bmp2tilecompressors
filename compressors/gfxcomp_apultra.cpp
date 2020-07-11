#include <cstdint>
#include <algorithm>
#include "libapultra.h"

int compress(uint8_t* source, uint32_t sourceLen, uint8_t* dest, uint32_t destLen)
{
	// Allocate memory
	auto packedSize = apultra_get_max_compressed_size(sourceLen);
	auto* pPacked = new uint8_t[packedSize];

	const auto size = apultra_compress(source, pPacked, sourceLen, packedSize, 0, 0, 0, nullptr, nullptr);

	// Check size
	if (size > destLen)
	{
		delete [] pPacked;
		return 0;
	}

	// Copy to dest
	std::copy(pPacked, pPacked + size, dest);

	// Free other memory
	delete [] pPacked;

	// Done
	return size;
}

extern "C" __declspec(dllexport) const char* getName()
{
	// A pretty name for this compression type
	// Generally, the name of the game it was REd from
	return "aPLib (apultra)";
}

extern "C" __declspec(dllexport) const char* getExt()
{
	// A string suitable for use as a file extension
	return "apultra";
}

extern "C" __declspec(dllexport) uint32_t compressTiles(uint8_t* source, uint32_t numTiles, uint8_t* dest, uint32_t destinationLength)
{
	// Compress tiles
	return compress(source, numTiles * 32, dest, destinationLength);
}

extern "C" __declspec(dllexport) uint32_t compressTilemap(uint8_t* source, uint32_t width, uint32_t height, uint8_t* dest, uint32_t destLen)
{
	// Compress tilemap
	return compress(source, width * height * 2, dest, destLen);
}

