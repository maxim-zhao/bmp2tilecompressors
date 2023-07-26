#include <cstdint>

#include "utils.h"
#include "lzsa/src/lib.h"

int32_t compress(const uint8_t* pSource, const size_t sourceLength, uint8_t* pDestination, const size_t destinationLength)
{
    const auto compressedSize = lzsa_compress_inmem(
        const_cast<unsigned char*>(pSource),
        pDestination,
        sourceLength,
        destinationLength,
        LZSA_FLAG_RAW_BLOCK | LZSA_FLAG_FAVOR_RATIO,
        0,
        2);

    if (compressedSize == static_cast<size_t>(-1))
    {
        return ReturnValues::CannotCompress;
    }

    if (compressedSize > destinationLength)
    {
        return ReturnValues::BufferTooSmall;
    }

    return static_cast<int32_t>(compressedSize);
}

extern "C" __declspec(dllexport) const char* getName()
{
    // A pretty name for this compression type
    return "LZSA2";
}

extern "C" __declspec(dllexport) const char* getExt()
{
    // A string suitable for use as a file extension
    return "lzsa2";
}

extern "C" __declspec(dllexport) int32_t compressTiles(
    const uint8_t* pSource,
    const uint32_t numTiles,
    uint8_t* pDestination,
    const uint32_t destinationLength)
{
    // Compress tiles
    return compress(pSource, numTiles * 32, pDestination, destinationLength);
}

extern "C" __declspec(dllexport) int32_t compressTilemap(
    const uint8_t* pSource,
    const uint32_t width,
    uint32_t height,
    uint8_t* pDestination,
    const uint32_t destinationLength)
{
    // Compress tilemap
    return compress(pSource, width * height * 2, pDestination, destinationLength);
}
