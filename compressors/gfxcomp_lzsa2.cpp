#include <cstdint>
#include <algorithm>
#include <cstdio>

#include "lzsa/src/lib.h"

int compress(const uint8_t* pSource, const size_t sourceLength, uint8_t* pDestination, const size_t destinationLength)
{
    // LZSA only likes to work on files so we have to write them to disk...
    char inFilename[L_tmpnam_s];
    if (tmpnam_s(inFilename))
    {
        return -1;
    }
    FILE* f;
    if (fopen_s(&f, inFilename, "wb") != 0 || f == nullptr)
    {
        return -1;
    }
    fwrite(pSource, 1, sourceLength, f);
    fclose(f);

    char outFilename[L_tmpnam_s];
    if (tmpnam_s(outFilename))
    {
        remove(inFilename);
        return -1;
    }

    long long compressedSize;

    const auto status = lzsa_compress_file(
        inFilename,
        outFilename,
        nullptr,
        LZSA_FLAG_RAW_BLOCK | LZSA_FLAG_FAVOR_RATIO, // TODO: make this optional (without it favours speed a bit)
        0,
        2,
        nullptr,
        nullptr,
        &compressedSize,
        nullptr,
        nullptr,
        nullptr);

    remove(inFilename);

    if (status != LZSA_OK)
    {
        remove(outFilename);
        return -1;
    }

    if (compressedSize > destinationLength)
    {
        remove(outFilename);
        return 0; // Buffer too small
    }

    // Read the file back in again
    if (fopen_s(&f, outFilename, "rb") != 0 || f == nullptr)
    {
        remove(outFilename);
        return -1;
    }

    if (fread_s(pDestination, destinationLength, 1, static_cast<size_t>(compressedSize), f) != compressedSize)
    {
        fclose(f);
        remove(outFilename);
        return -1;
    }

    fclose(f);
    return static_cast<int>(compressedSize);
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

extern "C" __declspec(dllexport) int compressTiles(const uint8_t* pSource, const uint32_t numTiles, uint8_t* pDestination, const uint32_t destinationLength)
{
    // Compress tiles
    return compress(pSource, numTiles * 32, pDestination, destinationLength);
}

extern "C" __declspec(dllexport) int compressTilemap(const uint8_t* pSource, const uint32_t width, uint32_t height, uint8_t* pDestination, const uint32_t destinationLength)
{
    // Compress tilemap
    return compress(pSource, width * height * 2, pDestination, destinationLength);
}
