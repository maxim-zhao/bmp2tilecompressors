#include <cstdint>

extern "C" __declspec(dllexport) const char* getName()
{
	// A pretty name for this compression type
	// Generally, the name of the game it was REd from
	return "1bpp raw (uncompressed) binary";
}

extern "C" __declspec(dllexport) const char* getExt()
{
	// A string suitable for use as a file extension
	return "1bpp";
}

extern "C" __declspec(dllexport) uint32_t compressTiles(uint8_t* source, uint32_t numTiles, uint8_t* destination, uint32_t destinationLength)
{
	const uint32_t sourceLength = numTiles * 32;
	const uint32_t outputLength = sourceLength / 4;
	if (outputLength > destinationLength)
	{
		return 0;
	}

	for (uint32_t i = 0; i < outputLength; ++i)
	{
		*destination++ = *source++;
		source += 3;
	}
	return outputLength;
}
