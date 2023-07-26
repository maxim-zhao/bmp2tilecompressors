#include <cstdint> // uint8_t, etc
#include <cstdlib> // free
#include <memory>

#include "utils.h"

extern "C"
{
    #include "zx7/zx7.h"
}

extern "C" __declspec(dllexport) const char* getName()
{
    // A pretty name for this compression type
    return "ZX7 (8-bit limited)";
}

extern "C" __declspec(dllexport) const char* getExt()
{
    // A string suitable for use as a file extension
    return "zx7";
}

// The actual compressor function, calling into the zx7 code
int32_t compress(
    const uint8_t* pSource,
    const size_t sourceLength,
    uint8_t* pDestination,
    const size_t destinationLength)
{
    const auto optimised = Utils::makeUniqueForMalloc(
        optimize(
            const_cast<unsigned char*>(pSource),
            sourceLength,
            0));
    std::size_t outputSize;
    long delta; // we don't care about this
    const auto pOutputData = Utils::makeUniqueForMalloc(
        compress(
            optimised.get(),
            const_cast<unsigned char*>(pSource),
            sourceLength,
            0,
            &outputSize,
            &delta));

    return Utils::copyToDestination(pOutputData.get(), outputSize, pDestination, destinationLength);
}

extern "C" __declspec(dllexport) int32_t compressTiles(
    const uint8_t* pSource,
    const uint32_t numTiles,
    uint8_t* pDestination,
    const uint32_t destinationLength)
{
    return compress(pSource, numTiles * 32, pDestination, destinationLength);
}

extern "C" __declspec(dllexport) int32_t compressTilemap(
    const uint8_t* pSource,
    const uint32_t width,
    const uint32_t height,
    uint8_t* pDestination,
    const uint32_t destinationLength)
{
    return compress(pSource, width * height * 2, pDestination, destinationLength);
}
