#include <cstdint> // uint8_t, etc
#include <cstdlib> // free
#include <memory>

#include "utils.h"

extern "C"
{
    #include "ZX0/src/zx0.h"
}

extern "C" __declspec(dllexport) const char* getName()
{
    // A pretty name for this compression type
    return "zx0";
}

extern "C" __declspec(dllexport) const char* getExt()
{
    // A string suitable for use as a file extension
    return "zx0";
}

// The actual compressor function, calling into the zx0 code
int32_t compress(
    const uint8_t* pSource,
    const size_t sourceLength,
    uint8_t* pDestination,
    const size_t destinationLength)
{
    const auto optimised = optimize(
        const_cast<unsigned char*>(pSource),
        static_cast<int>(sourceLength),
        0,
        static_cast<int>(sourceLength));
    // ZX0 leaks data by design, we can't free this

    int outputSize;
    int delta; // we don't care about this

    // We auto-free this using a unique_ptr. We have to tell it to use free() on the memory though.
    const auto pOutputData = Utils::makeUniqueForMalloc(
        compress(
            optimised,
            const_cast<unsigned char*>(pSource),
            static_cast<int>(sourceLength),
            0,
            0,
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
    return static_cast<int>(compress(pSource, numTiles * 32, pDestination, destinationLength));
}

extern "C" __declspec(dllexport) int32_t compressTilemap(
    const uint8_t* pSource,
    const uint32_t width,
    const uint32_t height,
    uint8_t* pDestination,
    const uint32_t destinationLength)
{
    return static_cast<int>(compress(pSource, width * height * 2, pDestination, destinationLength));
}
