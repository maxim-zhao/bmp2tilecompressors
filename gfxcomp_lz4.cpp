#include "lz4/lz4.h"
#include <string>

extern "C" __declspec(dllexport) const char* getName()
{
    // A pretty name for this compression type
    // Generally, the name of the game it was REd from
    static std::string name;
    if (name.empty())
    {
        name = "LZ4 (raw, v";
        name += LZ4_versionString();
        name += ")";
    }
    return name.c_str();
}

extern "C" __declspec(dllexport) const char* getExt()
{
    // A string suitable for use as a file extension
    return "lz4";
}

extern "C" __declspec(dllexport) int compressTiles(char* source, int numTiles, char* dest, int destLength)
{
    const int sourceLength = numTiles * 32;
    if (destLength < LZ4_compressBound(sourceLength))
    {
        return 0;
    }
    return LZ4_compress_default(source, dest, sourceLength, destLength);
}

extern "C" __declspec(dllexport) int compressTilemap(char* source, int width, int height, char* dest, int destLen)
{
    const int sourceLen = width * height * 2;
    if (destLen < LZ4_compressBound(sourceLen))
    {
        return 0;
    }
    return LZ4_compress_default(source, dest, sourceLen, destLen);
}
