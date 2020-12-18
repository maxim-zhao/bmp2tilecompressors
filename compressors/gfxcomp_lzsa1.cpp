#include <cstdint>
#include <algorithm>
#include <cstdio>

#include "lzsa/src/lib.h"

int compress(uint8_t* source, uint32_t sourceLen, uint8_t* dest, uint32_t destLen)
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
    fwrite(source, 1, sourceLen, f);
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
        0, // TODO: make this optional
        1, // TODO: make this optional?
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

    if (compressedSize > destLen)
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

    if (fread_s(dest, destLen, 1, static_cast<size_t>(compressedSize), f) != compressedSize)
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
    return "lzsa1";
}

extern "C" __declspec(dllexport) const char* getExt()
{
    // A string suitable for use as a file extension
    return "lzsa1";
}

extern "C" __declspec(dllexport) uint32_t compressTiles(uint8_t* source, uint32_t numTiles, uint8_t* dest, uint32_t destinationLength)
{
    // Compress tiles
    return compress(source, numTiles * 32, dest, destinationLength);
}

extern "C" __declspec(dllexport) uint32_t compressTilemap(uint8_t* source, uint32_t width, uint32_t height, uint8_t* dest, uint32_t destLen)
{
    // Compress tilemap
    return compress(source, width * height * 2, dest, destLen);
}
