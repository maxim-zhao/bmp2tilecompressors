#include <algorithm>
#include <vector>
#include <cstdint>

#include "InterleavedBitStream.h"
#include "utils.h"

// Compression format:
// - Data is deinterleaved on a per-tile basis
// - Then encoded as literals or runs
// - There is a bitstream interleaved with the data
// - Literals are a 1 in the bitstream and a byte of data -> 9 bits
// - LZ matches have a 5-bit length and 11-bit offset encoded as two bytes in the data

struct Match
{
    enum class Type: uint8_t
    {
        Invalid,
        Raw,
        Lz
    };

    Type type{};
    int costToEndInBits{};
    int offset{};
    int count{};
};

static int32_t compress(
    const uint8_t* pSource,
    const size_t sourceLength,
    uint8_t* pDestination,
    const size_t destinationLength)
{
    // Copy the data into a buffer
    auto source = Utils::toVector(pSource, sourceLength);

    // Deinterleave it
    for (size_t i = 0; i < sourceLength; i += 32)
    {
        // We extract 32 bytes
        std::vector tileBytes(source.begin() + i, source.begin() + i + 32);
        // Deinterleave them
        Utils::deinterleave(tileBytes, 4);
        // And replace
        std::ranges::copy(tileBytes, source.begin() + i);
    }

    // At each offset from the end, compute the cheapest option
    std::vector<Match> bestMatches(sourceLength + 1);
    for (int position = static_cast<int>(sourceLength) - 1; position >= 0; --position)
    {
        // We can always do a raw match to position + 1
        bestMatches[position].type = Match::Type::Raw;
        bestMatches[position].costToEndInBits = 9 + bestMatches[position + 1].costToEndInBits;

        // Then look for LZ matches...
        const auto maxMatchLength = std::min(static_cast<int>(sourceLength) - position, 34);
        for (auto matchLength = 3; matchLength <= maxMatchLength; ++matchLength)
        {
            // Look for a match
            // The relative haystack range is from -2048 to -1 + length
            // The relative needle range is from 0 to length
            const auto& r = std::ranges::find_end(
                source.begin() + std::max(0, position - 2048),
                source.begin() + position + matchLength - 1,
                source.begin() + position,
                source.begin() + position + matchLength);
            if (r.begin() != r.end())
            {
                // We have a match
                // Cost is 2 bytes + 1 bit
                if (const auto costToEnd = 17 + bestMatches[position + matchLength].costToEndInBits; 
                    costToEnd < bestMatches[position].costToEndInBits)
                {
                    // Zero encoding is a sentinel, so we have to skip it.
                    // That corresponds to matchLength == 3 and offset == 2048
                    auto offset = position - std::distance(source.begin(), r.begin());
                    if (matchLength == 3 && offset == 2048)
                    {
                        continue;
                    }
                    bestMatches[position].type = Match::Type::Lz;
                    bestMatches[position].offset = offset;
                    bestMatches[position].count = matchLength;
                    bestMatches[position].costToEndInBits = costToEnd;
                }
            }
            else
            {
                // No match, so longer ones won't work either
                break;
            }
        }
    }

    // Choose the best route to the end
    InterleavedBitStream b(true);
    for (size_t position = 0; position < sourceLength; /* increment in loop */)
    {
        const auto& match = bestMatches[position];
        switch (match.type)
        {
        case Match::Type::Raw:
            b.addBit(1);
            b.addByte(source[position]);
            position += 1;
            break;
        case Match::Type::Lz:
            {
                b.addBit(0);
                const auto hl = static_cast<uint16_t>(
                    (((match.count - 3) & 0b11111) << 8) |      // %---ccccc--------
                    (((-match.offset) & 0b11100000000) << 5) |  // %hhh-------------
                    ((-match.offset) & 0b11111111)              // %--------llllllll
                );
                b.addByte((hl >> 0) & 0xff);
                b.addByte((hl >> 8) & 0xff);
                position += match.count;
            }
            break;
        case Match::Type::Invalid: [[fallthrough]];
        default:
            // should not happen
            break;
        }
    }

    // Then add the terminator
    b.addBit(0);
    b.addByte(0);
    b.addByte(0);

    return Utils::copyToDestination(b.buffer(), pDestination, destinationLength);
}

extern "C" __declspec(dllexport) const char* getName()
{
    // A pretty name for this compression type
    // Generally, the name of the game it was REd from
    return "Shining Force Gaiden LZ";
}

extern "C" __declspec(dllexport) const char* getExt()
{
    // A string suitable for use as a file extension
    // ReSharper disable once StringLiteralTypo
    return "sfg";
}

extern "C" __declspec(dllexport) int32_t compressTiles(
    const uint8_t* pSource,
    const uint32_t numTiles,
    uint8_t* pDestination,
    const uint32_t destinationLength)
{
    // Compress tiles
    return compress(pSource, numTiles * 32, pDestination, destinationLength);
}
