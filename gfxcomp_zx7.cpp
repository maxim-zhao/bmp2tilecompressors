#include <cstdint> // uint8_t, etc
#include <cstring> // memcpy
#include <cstdlib> // free

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
uint32_t compress(uint8_t* pSource, uint32_t sourceLength, uint8_t* pDestination, uint32_t destinationLength)
{
    // The compressor allocates using calloc
    size_t outputSize;
    long delta; // we don't care about this
    auto optimised = optimize(pSource, sourceLength, 0);
    auto pOutputData = compress(optimised, pSource, sourceLength, 0, &outputSize, &delta);
    free(optimised);

    if (outputSize > destinationLength)
    {
        free(pOutputData);
        return 0;
    }

    memcpy_s(pDestination, destinationLength, pOutputData, outputSize);
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
