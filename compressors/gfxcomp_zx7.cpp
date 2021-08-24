#include <cstdint> // uint8_t, etc
#include <cstdlib> // free
#include <iterator>

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
uint32_t compress(const uint8_t* pSource, const size_t sourceLength, uint8_t* pDestination, const size_t destinationLength)
{
    // ReSharper disable once CommentTypo
    // The compressor allocates using calloc, so we have to free() the results
    std::size_t outputSize;
    long delta; // we don't care about this
    const auto optimised = optimize(const_cast<unsigned char*>(pSource), sourceLength, 0);
    const auto pOutputData = compress(optimised, const_cast<unsigned char*>(pSource), sourceLength, 0, &outputSize, &delta);
    free(optimised);

    if (outputSize > destinationLength)
    {
        free(pOutputData);
        return 0;
    }

    // Copy to the provided destination buffer
    std::copy_n(pOutputData, outputSize, stdext::checked_array_iterator<uint8_t*>(pDestination, destinationLength));
    free(pOutputData);

    return outputSize;
}

extern "C" __declspec(dllexport) int compressTiles(const uint8_t* pSource, const uint32_t numTiles, uint8_t* pDestination, const uint32_t destinationLength)
{
    return compress(pSource, numTiles * 32, pDestination, destinationLength);
}

extern "C" __declspec(dllexport) int compressTilemap(const uint8_t* pSource, const uint32_t width, const uint32_t height, uint8_t* pDestination, const uint32_t destinationLength)
{
    return compress(pSource, width * height * 2, pDestination, destinationLength);
}
