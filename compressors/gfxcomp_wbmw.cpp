#include <vector>
#include <cstdint>
#include <iterator>

#include "utils.h"
#include "rle.h"

void compressPlane(
    std::vector<uint8_t>& destination,
    const std::vector<uint8_t>::const_iterator& source,
    const std::vector<uint8_t>::const_iterator& sourceEnd)
{
    auto blocks = rle::process(source, sourceEnd);

    rle::optimize(blocks, 1, 0x7f, 0x7f);

    // Now emit them
    for (auto& block : blocks)
    {
        auto runLength = static_cast<int8_t>(block.getSize());
        if (block.getType() == rle::Block::Type::Raw)
        {
            // Negate the length for raw
            runLength *= -1;
        }
        destination.push_back(static_cast<uint8_t>(runLength));
        switch (block.getType())
        {
        case rle::Block::Type::Raw:
            std::ranges::copy(block.getData(), std::back_inserter(destination));
            break;
        case rle::Block::Type::Run:
            destination.push_back(block.getData()[0]);
            break;
        }
    }

    // Zero terminator
    destination.push_back(0);
}

int32_t compress(
    const uint8_t* pSource,
    const size_t sourceLength,
    uint8_t* pDestination,
    const size_t destinationLength,
    int interleaving)
{
    // Compress sourceLength bytes from pSource to pDestination;
    // return length, or 0 if destinationLength is too small, or -1 if there is an error

    // Copy the data into a buffer
    auto source = Utils::toVector(pSource, sourceLength);

    // Deinterleave it
    Utils::deinterleave(source, interleaving);

    // Make a buffer to hold the result
    std::vector<uint8_t> destination;

    // Compress each plane in turn
    const auto bitplaneSize = static_cast<int32_t>(sourceLength / interleaving);
    for (std::vector<uint8_t>::const_iterator it = source.begin(); it != source.end(); it += bitplaneSize)
    {
        compressPlane(destination, it, it + bitplaneSize);
    }
    return Utils::copyToDestination(destination, pDestination, destinationLength);
}

extern "C" __declspec(dllexport) const char* getName()
{
    // A pretty name for this compression type
    // Generally, the name of the game it was REd from
    return "Wonder Boy in Monster World RLE";
}

extern "C" __declspec(dllexport) const char* getExt()
{
    // A string suitable for use as a file extension
    // ReSharper disable once StringLiteralTypo
    return "wbmw";
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

/*
extern "C" __declspec(dllexport) int32_t compressTilemap(
    const uint8_t* pSource,
    const uint32_t width,
    uint32_t height,
    uint8_t* pDestination,
    const uint32_t destinationLength)
{
    // Compress tilemap
    return compress(pSource, width * height * 2, pDestination, destinationLength, 2);
}
*/