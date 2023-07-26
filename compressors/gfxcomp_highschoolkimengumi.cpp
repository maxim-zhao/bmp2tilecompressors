#include <vector>
#include <cstdint>
#include <iterator>

#include "rle.h"
#include "utils.h"

constexpr int MAX_RUN_SIZE = 0x7f;
constexpr int RLE_MASK = 0x00;
constexpr int RAW_MASK = 0x80;

extern "C" __declspec(dllexport) const char* getName()
{
    // A pretty name for this compression type
    // Generally, the name of the game it was REd from
    // ReSharper disable once StringLiteralTypo
    return "High School Kimengumi RLE";
}

extern "C" __declspec(dllexport) const char* getExt()
{
    // A string suitable for use as a file extension
    // ReSharper disable once StringLiteralTypo
    return "hskcompr";
}

int32_t compress(
    const uint8_t* pSource,
    const uint32_t sourceLength,
    uint8_t* pDestination,
    const uint32_t destinationLength,
    const int interleaving)
{
    // Copy the data into a buffer
    auto source = Utils::toVector(pSource, sourceLength);

    // Deinterleave it
    Utils::deinterleave(source, interleaving);

    // Make a buffer to hold the result
    std::vector<uint8_t> result;

    // Header is the size divided by the interleaving
    const auto size = sourceLength / interleaving;
    result.push_back((size >> 0) & 0xff);
    result.push_back((size >> 8) & 0xff);

    // Compress everything in one go
    auto blocks = rle::process(source.cbegin(), source.cend());
    rle::optimize(blocks, 1, MAX_RUN_SIZE, MAX_RUN_SIZE);

    for (const auto& block : blocks)
    {
        const auto mask = block.getType() == rle::Block::Type::Raw ? 0x80 : 0x00;
        result.push_back(static_cast<uint8_t>(mask | block.getSize()));
        switch (block.getType())
        {
        case rle::Block::Type::Raw:
            std::ranges::copy(block.getData(), std::back_inserter(result));
            break;
        case rle::Block::Type::Run:
            result.push_back(block.getData()[0]);
            break;
        }
    }

    // Zero terminator
    result.push_back(0);

    return Utils::copyToDestination(result, pDestination, destinationLength);
}

extern "C" __declspec(dllexport) int32_t compressTiles(
    const uint8_t* pSource,
    const uint32_t numTiles,
    uint8_t* pDestination,
    const uint32_t destinationLength)
{
    // Compress tiles
    return compress(pSource, numTiles * 32, pDestination, destinationLength, 4);
}

extern "C" __declspec(dllexport) int32_t compressTilemap(
    const uint8_t* pSource,
    const uint32_t width,
    const uint32_t height,
    uint8_t* pDestination,
    const uint32_t destinationLength)
{
    // Compress tilemap
    return compress(pSource, width * height * 2, pDestination, destinationLength, 2);
}
