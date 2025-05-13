#include <cstdint>
#include <iterator>
#include <ranges>

#include "utils.h"

extern "C" __declspec(dllexport) const char* getName()
{
    // A pretty name for this compression type
    // Generally, the name of the game it was REd from
    return "Tiertex compression";
}

extern "C" __declspec(dllexport) const char* getExt()
{
    // A string suitable for use as a file extension
    return "tiertexcompr";
}

namespace
{
    std::vector<uint8_t> compress(const std::vector<uint8_t>& source, const int rleByte)
    {
        std::vector<uint8_t> result;
        result.push_back(static_cast<uint8_t>(rleByte));
        for (size_t i = 0; i < source.size(); ++i)
        {
            const auto b = source[i];
            result.push_back(b);
            if (b == rleByte)
            {
                // Count the run length
                auto runLength = 1;
                while (i + 1 < source.size() && source[i+1] == rleByte && runLength < 255)
                {
                    ++runLength;
                    ++i;
                }
                // Emit the data
                result.push_back(b);
                result.push_back(static_cast<uint8_t>(runLength));
            }
        }
        return result;
    }
}

extern "C"
__declspec(dllexport)
int32_t compressTiles(
    const uint8_t* pSource,
    const uint32_t numTiles,
    uint8_t* pDestination,
    const uint32_t destinationLength)
{
    // Put data into a vector
    std::vector<uint8_t> buffer;
    for (size_t tileNumber = 0; tileNumber < numTiles; ++tileNumber)
    {
        // Extract a sub-buffer and interleave to match what teh code does on decompression
        auto tile = std::vector<uint8_t>(32);
        for (int i = 0; i < 8; ++i)
        {
            tile[i+0] = *pSource++;
            tile[i+8] = *pSource++;
            tile[i+16] = *pSource++;
            tile[i+24] = *pSource++;
        }
        std::ranges::copy(tile, std::back_inserter(buffer));
    }

    // Try all "magic bytes" and see which compresses best
    std::vector<uint8_t> result;
    for (auto i = 0; i < 256; ++i)
    {
        auto vec = compress(buffer, i);
        if (vec.size() < result.size() || result.empty())
        {
            // Swapping is cheaper than copying...
            std::swap(result, vec);
        }
    }
    return Utils::copyToDestination(result, pDestination, destinationLength);
}

extern "C"
__declspec(dllexport)
int32_t compressTilemap(
    const uint8_t* pSource,
    const uint32_t width,
    const uint32_t height, 
    uint8_t* pDestination,
    const uint32_t destinationLength)
{
    size_t sourceLength = width * height * 2;
    if (sourceLength % 4 != 0)
    {
        return ReturnValues::CannotCompress;
    }
    // Put data into a vector, packing the high bits
    std::vector<uint8_t> buffer;
    for (auto* p = pSource; p < pSource + sourceLength; /* increment in loop */)
    {
        buffer.push_back(*pSource++);
        auto highBits = *pSource++ & 0xf;
        buffer.push_back(*pSource++);
        highBits |= (*pSource++ & 0xf) << 4;
        buffer.push_back(static_cast<uint8_t>(highBits));
    }
    // Try all "magic bytes" and see which compresses best
    std::vector<uint8_t> result;
    for (auto i = 0; i < 256; ++i)
    {
        auto vec = compress(buffer, i);
        if (vec.size() < result.size() || result.empty())
        {
            // Swapping is cheaper than copying...
            std::swap(result, vec);
        }
    }
    return Utils::copyToDestination(result, pDestination, destinationLength);
}
