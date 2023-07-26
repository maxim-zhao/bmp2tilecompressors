#include <cstdint>

#include "utils.h"

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

extern "C" __declspec(dllexport) int32_t compressTiles(
    const uint8_t* pSource,
    const uint32_t numTiles,
    uint8_t* pDestination,
    const uint32_t destinationLength)
{
    const uint32_t sourceLength = numTiles * 32;
    const uint32_t outputLength = sourceLength / 2;
    if (outputLength > destinationLength)
    {
        return ReturnValues::BufferTooSmall;
    }

    for (uint32_t i = 0; i < outputLength; i += 2)
    {
        // Copy two bitplanes
        *pDestination++ = *pSource++;
        *pDestination++ = *pSource++;
        // And copy two
        pSource += 2;
    }
    return static_cast<int>(outputLength);
}
