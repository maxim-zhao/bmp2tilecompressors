#include <algorithm>
#include <cstdint>
#include <iterator>
#include <stdexcept>

#include "utils.h"

extern "C" __declspec(dllexport) const char* getName()
{
    // A pretty name for this compression type
    // Generally, the name of the game it was REd from
    return "SIMS compression";
}

extern "C" __declspec(dllexport) const char* getExt()
{
    // A string suitable for use as a file extension
    return "sims";
}

namespace
{
    struct Match
    {
        enum class Modes: uint8_t
        {
            Default,
            Rle,
            Lz,
            Raw
        };

        explicit Match(const Modes mode) :
            mode(mode),
            offset(-1),
            costToEnd(std::numeric_limits<int>::max()),
            length(-1),
            extra(-1)
        {
        }

        // Match type
        Modes mode;
        // Location in source data
        int offset;
        // Encoding cost to end of data
        int costToEnd;
        // Other parameters specific to the match type
        int length;
        int extra;

        int writeTo(std::vector<uint8_t>& vector, const std::vector<uint8_t>& data) const
        {
            switch (mode)
            {
            case Modes::Rle:
                if (extra <= 9)
                {
                    vector.push_back(static_cast<uint8_t>(0b11000000 | ((length - 1) << 3) | (extra - 2)));
                }
                else
                {
                    vector.push_back(static_cast<uint8_t>(0b11100000 | ((length - 1) << 3) | ((extra - 2) >> 8)));
                    vector.push_back((extra - 2) & 0xff);
                }
                for (int i = 0; i < length; ++i)
                {
                    vector.push_back(data[offset + i]);
                }
                return length * extra;
            case Modes::Lz:
                vector.push_back(static_cast<uint8_t>(extra >> 4));
                vector.push_back(static_cast<uint8_t>(((extra & 0b1111) << 4) | (length - 2)));
                return length;
            case Modes::Raw:
                vector.push_back(static_cast<uint8_t>(0b10000000 | (length - 1)));
                for (int i = 0; i < length; ++i)
                {
                    vector.push_back(data[offset + i]);
                }
                return length;
            case Modes::Default:
                throw std::runtime_error("Unexpected mode=default");
            }
            throw std::runtime_error("Unexpected mode=default");
        }
    };

    std::vector<uint8_t> compress(const std::vector<uint8_t>& data)
    {
        // Best matches found so far
        std::vector bestMatches(data.size(), Match(Match::Modes::Default));

        auto lenData = static_cast<int>(data.size());

        // Work through the file backwards
        for (int position = lenData - 1; position >= 0; --position)
        {
            // Find the best option at this point, that either gets to the end of the file or to a position in the
            // sequences that sums to the lowest byte length

            // First, we look for LZ matches. They tend to dominate the results (when possible, they are very cheap)
            // so doing them first helps avoid some wasted effort.
            // LZ run length is in the range 2..17 but also bounded by the number of bytes to the left and right
            for (int length = 2; length < std::min(lenData - position, 17) + 1; ++length)
            {
                // Look for a match
                const auto& r = std::ranges::find_end(
                    data.begin() + std::max(0, position - 2047),
                    data.begin() + position,
                    data.begin() + position,
                    data.begin() + position + length);
                if (r.begin() != r.end())
                {
                    // We have a match
                    // LZ matches are always 2 bytes
                    auto cost = 2;
                    if (position + length < lenData)
                    {
                        cost += bestMatches[position + length].costToEnd;
                    }
                    if (cost < bestMatches[position].costToEnd)
                    {
                        bestMatches[position].costToEnd = cost;
                        bestMatches[position].mode = Match::Modes::Lz;
                        bestMatches[position].length = length;
                        bestMatches[position].extra = position - std::distance(data.begin(), r.begin());
                    }
                }
                else
                {
                    // We try short lengths because they might have a cheaper total cost, but if we failed to find any 
                    // match at length n, length n+1 will fail too
                    break;
                }
            }

            // Next an RLE match...
            for (int rleMatchLength = 1; rleMatchLength < 5; ++rleMatchLength)
            {
                // We can't do RLE unless there's at least rle_match_length*2 bytes left in the data
                if (lenData - position - 1 < rleMatchLength * 2)
                {
                    // Won't be true for higher rleMatchLength values either
                    break;
                }
                // Grab the sequence
                const auto& rleSequence = std::ranges::subrange(
                    data.begin() + position,
                    data.begin() + position + rleMatchLength);
                auto rleCount = 1;
                // Check for matches
                for (int matchPosition = position + rleMatchLength;
                     matchPosition < std::min(lenData - rleMatchLength, position + rleMatchLength * 2047);
                     matchPosition += rleMatchLength)
                {
                    // Does it match?
                    if (std::ranges::equal(
                        data.begin() + matchPosition,
                        data.begin() + matchPosition + rleMatchLength,
                        rleSequence.begin(),
                        rleSequence.end()))
                    {
                        // Yes, so what's our cost?
                        ++rleCount;
                        // RLE costs 1 byte + the sequence for counts up to 9, and 2 bytes + the sequence for counts
                        // up to 2050 - but the decompressor only wants up to 2047
                        int cost;
                        if (rleCount <= 9)
                        {
                            cost = 1 + rleMatchLength;
                        }
                        else if (rleCount <= 2047)
                        {
                            cost = 2 + rleMatchLength;
                        }
                        else
                        {
                            // Over the max length
                            continue;
                        }
                        if (matchPosition + rleMatchLength < lenData)
                        {
                            cost += bestMatches[matchPosition + rleMatchLength].costToEnd;
                        }
                        if (cost < bestMatches[position].costToEnd)
                        {
                            bestMatches[position].costToEnd = cost;
                            bestMatches[position].mode = Match::Modes::Rle;
                            bestMatches[position].length = rleMatchLength;
                            bestMatches[position].extra = rleCount;
                            bestMatches[position].offset = position;
                        }
                    }
                    else
                    {
                        // Not a match, do not continue
                        break;
                    }
                }
            }

            // Finally raw matches...
            for (int n = 1; n < std::min(64, lenData - position) + 1; ++n)
            {
                // A raw match will cost n+1 bytes
                int cost = n + 1;
                if (position + n < lenData)
                {
                    cost += bestMatches[position + n].costToEnd;
                }
                if (cost < bestMatches[position].costToEnd)
                {
                    bestMatches[position].costToEnd = cost;
                    bestMatches[position].mode = Match::Modes::Raw;
                    bestMatches[position].length = n;
                    bestMatches[position].offset = position;
                }
            }
        }

        // Now we have filled our vector, and we can walk through the matches to populate our data.
        // First two bytes are the compressed data length, which is the same as the first cost.
        std::vector<uint8_t> result;
        result.push_back((bestMatches[0].costToEnd >> 0) & 0xff);
        result.push_back((bestMatches[0].costToEnd >> 8) & 0xff);

        auto position = 0;
        while (position < lenData)
        {
            const auto& match = bestMatches[position];
            position += match.writeTo(result, data);
        }

        return result;
    }
}

extern "C" __declspec(dllexport) int32_t compressTiles(
    const uint8_t* pSource,
    const uint32_t numTiles,
    uint8_t* pDestination,
    const uint32_t destinationLength)
{
    const auto& data = Utils::toVector(pSource, numTiles * 32);
    const auto& compressed = compress(data);
    return Utils::copyToDestination(compressed, pDestination, destinationLength);
}
