#include <cstdint>
#include <iterator>
#include <vector>
#include <ranges>

#include "rle.h"
#include "utils.h"

extern "C" __declspec(dllexport) const char* getName()
{
    // A pretty name for this compression type
    // Generally, the name of the game it was REd from
    return "Lemmings RLE";
}

extern "C" __declspec(dllexport) const char* getExt()
{
    // A string suitable for use as a file extension
    return "lemmingscompr";
}

extern "C" __declspec(dllexport) int32_t compressTiles(
    const uint8_t* pSource,
    const uint32_t numTiles,
    uint8_t* pDestination,
    const uint32_t destinationLength)
{
    // Transfer to a vector
    const auto sourceLength = numTiles * 32;
    std::vector<uint8_t> source = Utils::toVector(pSource, sourceLength);

    // Add extra 0s for the multiple of 8 tiles/256 bytes
    const auto extraTileCount = (8 - (numTiles % 8)) & 7;
    source.resize(source.size() + extraTileCount * 32, 0);

    std::vector<uint8_t> result;
    // First byte is the number of 256-bytes (8 tile) chunks
    result.push_back(static_cast<uint8_t>(source.size() / 256));

    // Compress in chunks of 256 bytes
    for (auto it = source.begin(); it != source.end(); it += 256)
    {
        // Get a 256-byte buffer
        std::vector chunk(it, it + 256);

        // Deinterleave
        Utils::deinterleave(chunk, 4);

        // Decompose to blocks
        auto blocks = rle::process(chunk.begin(), chunk.end());

        // Optimize
        rle::optimize(blocks, 1, 0x80, 0x80);

        // Emit
        for (const auto& block : blocks)
        {
            const auto mask = block.getType() == rle::Block::Type::Raw ? 0x00 : 0x80;
            result.push_back(static_cast<uint8_t>(mask | (block.getSize() - 1)));
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
    }

    return Utils::copyToDestination(result, pDestination, destinationLength);
}
