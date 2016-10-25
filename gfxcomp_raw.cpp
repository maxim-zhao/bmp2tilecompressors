#include <algorithm>

extern "C" __declspec(dllexport) char* getName()
{
	// A pretty name for this compression type
	// Generally, the name of the game it was REd from
	return "Raw (uncompressed) binary";
}

extern "C" __declspec(dllexport) char* getExt()
{
	// A string suitable for use as a file extension
	return "bin";
}

extern "C" __declspec(dllexport) int compressTiles(char* source, int numTiles, char* dest, int destLen)
{
	int sourceLen = numTiles * 32;
	if (sourceLen > destLen)
	{
		return 0;
	}
	memcpy_s(dest, destLen, source, sourceLen);
	return sourceLen;
}

extern "C" __declspec(dllexport) int compressTilemap(char* source, int width, int height, char* dest, int destLen)
{
	int sourceLen = width * height * 2;
	if (sourceLen > destLen)
	{
		return 0;
	}
	memcpy_s(dest, destLen, source, sourceLen);
	return sourceLen;
}

extern "C" __declspec(dllexport) int decompressTiles(char* source, int sourceLen, char* dest, int destLen)
{
	if (sourceLen > destLen)
	{
		return 0;
	}
	memcpy_s(dest, destLen, source, sourceLen);
	return sourceLen;
}

extern "C" __declspec(dllexport) int decompressTilemap(char* source, int sourceLen, char* dest, int destLen)
{
	if (sourceLen > destLen)
	{
		return 0;
	}
	memcpy_s(dest, destLen, source, sourceLen);
	return sourceLen;
}
