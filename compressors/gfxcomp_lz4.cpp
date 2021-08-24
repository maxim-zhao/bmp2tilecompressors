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

extern "C" __declspec(dllexport) int compressTiles(const uint8_t* pSource, const uint32_t numTiles, uint8_t* pDestination, const uint32_t destinationLength)
{
    const auto sourceLength = numTiles * 32;
    if (destinationLength < static_cast<uint32_t>(LZ4_compressBound(sourceLength)))
    {
        return 0;
    }
    return LZ4_compress_default(reinterpret_cast<const char*>(pSource), reinterpret_cast<char*>(pDestination), sourceLength, destinationLength);
}

extern "C" __declspec(dllexport) int compressTilemap(const uint8_t* pSource, const uint32_t width, const uint32_t height, uint8_t* pDestination, const uint32_t destinationLength)
{
    const auto sourceLen = width * height * 2;
    if (destinationLength < static_cast<uint32_t>(LZ4_compressBound(sourceLen)))
    {
        return 0;
    }
    return LZ4_compress_default(reinterpret_cast<const char*>(pSource), reinterpret_cast<char*>(pDestination), sourceLen, destinationLength);
}
