#include <cstdint>
#include <algorithm>

// Forward declares for oapack stuff. It uses globals so we have to poke into them to make it work.
int FindOptimalSolution();
int EmitCompressed();
extern unsigned char* packedData;
extern unsigned char* data;
extern int packedBits;
extern int size;

int compress(uint8_t* source, uint32_t sourceLen, uint8_t* dest, uint32_t destLen)
{
	data = static_cast<unsigned char*>(source);
	size = static_cast<int>(sourceLen);

	packedBits = FindOptimalSolution();
    const auto compressedSize = static_cast<uint32_t>(EmitCompressed());

	// Check size
	if (compressedSize > destLen)
	{
		return 0;
	}

	// Copy to dest
	memcpy_s(dest, destLen, &packedData, compressedSize);

	// Done
	return compressedSize;
}

extern "C" __declspec(dllexport) const char* getName()
{
	// A pretty name for this compression type
	// Generally, the name of the game it was REd from
	return "aPLib (oapack)";
}

extern "C" __declspec(dllexport) const char* getExt()
{
	// A string suitable for use as a file extension
	return "oapack";
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

