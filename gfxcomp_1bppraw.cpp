#include <string>
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

extern "C" __declspec(dllexport) uint32_t compressTiles(uint8_t* source, uint32_t numTiles, uint8_t* dest, uint32_t destLen)
{
	uint32_t sourceLen = numTiles * 32;
	if (sourceLen / 4 > destLen)
	{
		return 0;
	}
	else
	{
		for (uint32_t i = 0; i < sourceLen; ++i)
		{
			*dest++ = *source;
            source += 4;
		}
		return sourceLen / 4;
	}
}
