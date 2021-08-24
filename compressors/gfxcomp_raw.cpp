#include <algorithm>

extern "C" __declspec(dllexport) const char* getName()
{
    // A pretty name for this compression type
    // Generally, the name of the game it was REd from
    return "Raw (uncompressed) binary";
}

extern "C" __declspec(dllexport) const char* getExt()
{
    // A string suitable for use as a file extension
    return "bin";
}

extern "C" __declspec(dllexport) int compressTiles(const uint8_t* pSource, const uint32_t numTiles, uint8_t* pDestination, const uint32_t destinationLength)
{
    const uint32_t sourceLength = numTiles * 32;
    if (sourceLength > destinationLength)
    {
        return 0;
    }
    memcpy_s(pDestination, destinationLength, pSource, sourceLength);
    return sourceLength;
}

extern "C" __declspec(dllexport) int compressTilemap(const uint8_t* pSource, const uint32_t width, uint32_t height, uint8_t* pDestination, const uint32_t destinationLength)
{
    const uint32_t sourceLength = width * height * 2;
    if (sourceLength > destinationLength)
    {
        return 0;
    }
    memcpy_s(pDestination, destinationLength, pSource, sourceLength);
    return sourceLength;
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
