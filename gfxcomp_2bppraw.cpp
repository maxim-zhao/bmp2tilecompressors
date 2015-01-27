#include <cstdint>

extern "C" __declspec(dllexport) const char* getName()
{
	// A pretty name for this compression type
	// Generally, the name of the game it was REd from
	return "2bpp raw (uncompressed) binary";
}

extern "C" __declspec(dllexport) const char* getExt()
{
	// A string suitable for use as a file extension
	return "2bpp";
}

extern "C" __declspec(dllexport) uint32_t compressTiles(uint8_t* source, uint32_t numTiles, uint8_t* dest, uint32_t destLen)
{
	uint32_t sourceLen = numTiles * 32;
	uint32_t outputSize = sourceLen / 2;
	if (outputSize > destLen)
	{
		return 0;
	}

	for (uint32_t i = 0; i < outputSize; i += 2)
	{
		*dest++ = *source++;
		*dest++ = *source++;
		source += 2;
	}
	return outputSize;
}
