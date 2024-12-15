#include <cstdint> // uint8_t, etc
#include <cstdlib> // free
#include <memory>
#include <filesystem>

#include "utils.h"

// Things in lzee.c
extern "C"
{
    extern void encode();
    extern FILE* infile;
    extern FILE* outfile;
}

extern "C" __declspec(dllexport) const char* getName()
{
    // A pretty name for this compression type
    return "lzee";
}

extern "C" __declspec(dllexport) const char* getExt()
{
    // A string suitable for use as a file extension
    return "lzee";
}

// The actual compressor function, calling into the lzee code
static int32_t compress(
    const uint8_t* pSource,
    const size_t sourceLength,
    uint8_t* pDestination,
    const size_t destinationLength)
{
    try
    {
        // LZEE doesn't have an in-memory compressor, so let's just make temp files
        if (tmpfile_s(&infile) != 0)
        {
            throw std::runtime_error("Failed to make infile");
        }
        if (fwrite(pSource, 1, sourceLength, infile) != sourceLength)
        {
            throw std::runtime_error("Failed to write infile");
        }
        if (fseek(infile, 0, SEEK_SET) != 0)
        {
            throw std::runtime_error("Failed to seek infile");
        }
        if (tmpfile_s(&outfile) != 0)
        {
            throw std::runtime_error("Failed to make outfile");
        }
        encode();
        if (fclose(infile) != 0)
        {
            throw std::runtime_error("Failed to close input file");
        }
        const auto outSize = ftell(outfile);
        std::vector<uint8_t> result(outSize);
        if (fseek(outfile, 0, SEEK_SET) != 0)
        {
            throw std::runtime_error("Failed to seek outfile");
        }
        if (fread_s(result.data(), result.size(), 1, result.size(), outfile) != result.size())
        {
            throw std::runtime_error("Failed to read outfile");
        }
        if (fclose(outfile) != 0)
        {
            throw std::runtime_error("Failed to close output file");
        }
        return Utils::copyToDestination(result, pDestination, destinationLength);
    }
    catch (const std::exception& e)
    {
        printf("%s\n", e.what());
        return ReturnValues::CannotCompress;
    }
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
