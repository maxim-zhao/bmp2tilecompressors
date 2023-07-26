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
    return "lemmings";
}

extern "C" __declspec(dllexport) int32_t compressTiles(
    const uint8_t* pSource,
    const uint32_t numTiles,
    uint8_t* pDestination,
    const uint32_t destinationLength)
{
    // Transfer to a vector
    const uint32_t sourceLength = numTiles * 32;
    std::vector<uint8_t> source = Utils::toVector(pSource, sourceLength);

    // Add extra 0s for the multiple of 8 tiles/256 bytes
    source.resize(source.size() + sourceLength % 256, 0);

    // Deinterleave
    Utils::deinterleave(source, 4);

    std::vector<uint8_t> result;
    // First byte is the tile count divided by 8
    result.push_back(static_cast<uint8_t>(numTiles / 8));

    // Compress in chunks of 256 bytes
    for (int i = 0; i < static_cast<int>(source.size()); i += 256)
    {
        // Decompose to blocks
        auto blocks = rle::process(source.begin() + i, source.begin() + i + 256);

        // Optimize
        rle::optimize(blocks, 1, 0x80, 0x80);

        // Emit
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
    }

    return Utils::copyToDestination(result, pDestination, destinationLength);
}
