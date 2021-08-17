#include <cstdint> // uint8_t, etc
#include <cstdlib> // free
#include <iterator>

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
uint32_t compress(uint8_t* pSource, uint32_t sourceLength, uint8_t* pDestination, uint32_t destinationLength)
{
    int outputSize;
    int delta; // we don't care about this
    const auto optimised = optimize(pSource, sourceLength, 0, sourceLength);
    // ZX0 leaks data by design, we can't free this
    const auto pOutputData = compress(optimised, pSource, sourceLength, 0, 0, &outputSize, &delta);

    if (static_cast<uint32_t>(outputSize) > destinationLength)
    {
        free(pOutputData);
        return 0;
    }

    // Copy to the provided destination buffer
    std::copy_n(pOutputData, outputSize, stdext::checked_array_iterator<uint8_t*>(pDestination, destinationLength));
    free(pOutputData);

    return outputSize;
}

extern "C" __declspec(dllexport) uint32_t compressTiles(uint8_t* pSource, uint32_t numTiles, uint8_t* pDestination, uint32_t destinationLength)
{
    return compress(pSource, numTiles * 32, pDestination, destinationLength);
}

extern "C" __declspec(dllexport) uint32_t compressTilemap(uint8_t* pSource, uint32_t width, uint32_t height, uint8_t* pDestination, uint32_t destinationLength)
{
    return compress(pSource, width * height * 2, pDestination, destinationLength);
}
