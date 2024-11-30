#include <algorithm>
#include <cstdint>
#include <iterator>
#include <stdexcept>

#include "utils.h"

extern "C" __declspec(dllexport) const char* getName()
{
    // A pretty name for this compression type
    // Generally, the name of the game it was REd from
    return "GG Aleste compression";
}

extern "C" __declspec(dllexport) const char* getExt()
{
    // A string suitable for use as a file extension
    return "aleste";
}

namespace
{
    struct Match
    {
        enum class Modes: uint8_t
        {
            Default,
            Lz,
            Raw
        };

        explicit Match(const Modes mode) :
            mode(mode),
            offset(-1),
            costToEnd(std::numeric_limits<int>::max()),
            length(-1),
            lzOffset(-1)
        {
        }

        // Match type
        Modes mode;
        // Location in data
        int offset;
        // Encoding cost to end of data
        int costToEnd;
        // Length
        int length;
        // LZ offset
        int lzOffset;

        int writeTo(std::vector<uint8_t>& vector, const std::vector<uint8_t>& data) const
        {
            switch (mode)
            {
            case Modes::Lz:
                vector.push_back(static_cast<uint8_t>(0b10000000 | (length - 3)));
                vector.push_back(static_cast<uint8_t>(lzOffset - 1));
                return length;
            case Modes::Raw:
                vector.push_back(static_cast<uint8_t>(length));
                std::ranges::copy_n(data.begin() + offset, length, std::back_inserter(vector));
                return length;
            case Modes::Default:
                throw std::runtime_error("Unexpected mode=default");
            }
            throw std::runtime_error("Unexpected mode=default");
        }
    };

    int32_t compress(
        const uint8_t* pSource,
        const size_t sourceLength,
        uint8_t* pDestination,
        const size_t destinationLength)
    {
        auto data = Utils::toVector(pSource, sourceLength);

        // Best matches found so far
        std::vector bestMatches(data.size(), Match(Match::Modes::Default));

        auto lenData = static_cast<int>(data.size());

        // Work through the file backwards
        for (int position = lenData - 1; position >= 0; --position)
        {
            // Find the best option at this point, that either gets to the end of the file or to a position in the
            // sequences that sums to the lowest byte length

            // First, we look for LZ matches.
            // LZ run length is in the range 3..130 but also bounded by the number of bytes to the left and right
            for (int length = 3; length < std::min(lenData - position, 130) + 1; ++length)
            {
                // Look for a match
                const auto& r = std::ranges::find_end(
                    // Haystack
                    data.begin() + std::max(0, position - 256),
                    data.begin() + position + length - 1,
                    // Needle
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
                        bestMatches[position].lzOffset = position - std::distance(data.begin(), r.begin());
                    }
                }
                else
                {
                    // We try short lengths because they might have a cheaper total cost, but if we failed to find any 
                    // match at length n, length n+1 will fail too
                    break;
                }
            }

            // Finally raw matches...
            for (int n = 1; n < std::min(127, lenData - position) + 1; ++n)
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

        auto position = 0;
        while (position < lenData)
        {
            const auto& match = bestMatches[position];
            position += match.writeTo(result, data);
        }

        result.push_back(0); // Terminator

        // Copy to the destination buffer if it fits
        return Utils::copyToDestination(result, pDestination, destinationLength);
    }
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

extern "C" __declspec(dllexport) int32_t compressTilemap(
    const uint8_t* pSource,
    const uint32_t width,
    const uint32_t height,
    uint8_t* pDestination,
    const uint32_t destinationLength)
{
    // Compress tilemap
    return compress(pSource, width * height * 2, pDestination, destinationLength);
}
