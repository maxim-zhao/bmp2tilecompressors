#include <cstdint>

#include "libapultra.h"
#include "utils.h"

int32_t compress(const uint8_t* pSource, const size_t sourceLength, uint8_t* pDestination, const size_t destinationLength)
{
    // Allocate oversized memory buffer
    std::vector<unsigned char> packed(apultra_get_max_compressed_size(sourceLength));

    const auto size = apultra_compress(pSource, packed.data(), sourceLength, packed.size(), 0, 0, 0, nullptr, nullptr);

    // It returns size = -1 on failure
    if (size == static_cast<size_t>(-1))
    {
        return ReturnValues::CannotCompress;
    }

    // Trim the vector to the right size
    packed.resize(size);

    return Utils::copyToDestination(packed, pDestination, destinationLength);
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
    const uint32_t height,
    uint8_t* pDestination,
    const uint32_t destinationLength)
{
    // Compress tilemap
    return compress(pSource, width * height * 2, pDestination, destinationLength);
}
