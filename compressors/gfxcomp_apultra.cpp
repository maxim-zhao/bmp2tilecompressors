#include <cstdint>
#include <algorithm>
#include "libapultra.h"

int compress(const uint8_t* pSource, const size_t sourceLength, uint8_t* pDestination, const size_t destinationLength)
{
    // Allocate memory
    const auto packedSize = apultra_get_max_compressed_size(sourceLength);
    auto* pPacked = new uint8_t[packedSize];

    const auto size = apultra_compress(pSource, pPacked, sourceLength, packedSize, 0, 0, 0, nullptr, nullptr);

    // Check size
    if (size > destinationLength)
    {
        delete [] pPacked;
        return 0;
    }

    // Copy to pDestination
    std::copy_n(pPacked, size, pDestination);

    // Free other memory
    delete [] pPacked;

    // Done
    return static_cast<int>(size);
}

extern "C" __declspec(dllexport) const char* getName()
{
    // A pretty name for this compression type
    // Generally, the name of the game it was REd from
    return "aPLib (apultra)";
}

extern "C" __declspec(dllexport) const char* getExt()
{
    // A string suitable for use as a file extension
    return "apultra";
}

extern "C" __declspec(dllexport) int compressTiles(
    const uint8_t* pSource,
    const uint32_t numTiles,
    uint8_t* pDestination,
    const uint32_t destinationLength)
{
    // Compress tiles
    return compress(pSource, numTiles * 32, pDestination, destinationLength);
}

extern "C" __declspec(dllexport) int compressTilemap(
    const uint8_t* pSource,
    const uint32_t width,
    const uint32_t height,
    uint8_t* pDestination,
    const uint32_t destinationLength)
{
    // Compress tilemap
    return compress(pSource, width * height * 2, pDestination, destinationLength);
}
