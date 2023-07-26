#include <cstdint>

#include "utils.h"

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

extern "C" __declspec(dllexport) int32_t compressTilemap(
    const uint8_t* pSource,
    const uint32_t width,
    const uint32_t height,
    uint8_t* pDestination,
    const uint32_t destinationSize)
{
    const uint32_t outputSize = width * height;
    if (outputSize > destinationSize)
    {
        return ReturnValues::BufferTooSmall;
    }

    for (auto i = 0u; i < outputSize; ++i)
    {
        // Copy one...
        *pDestination++ = *pSource++;
        // ...skip one
        ++pSource;
    }
    return static_cast<int32_t>(outputSize);
}
