#include <cstdint>

extern "C" __declspec(dllexport) const char* getName()
{
	// A pretty name for this compression type
	// Generally, the name of the game it was REd from
	return "LSB-only tilemap";
}

extern "C" __declspec(dllexport) const char* getExt()
{
	// A string suitable for use as a file extension
	// ReSharper disable once StringLiteralTypo
	return "lsbtilemap";
}

extern "C" __declspec(dllexport) uint32_t compressTilemap(uint8_t* source, uint32_t width, uint32_t height, uint8_t* dest, uint32_t destSize)
{
	const size_t outputSize = width * height;
	if (outputSize > destSize)
	{
		return 0;
	}

	for (size_t i = 0; i < outputSize; ++i)
	{
		// Copy one...
		*dest++ = *source++;
		// ...skip one
		++source;
	}
	return outputSize;
}
