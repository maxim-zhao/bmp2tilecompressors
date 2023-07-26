#include "rle.h"

#include <ranges>

#include "utils.h"

rle::Block::Block(const std::vector<uint8_t>::const_iterator& begin, const std::vector<uint8_t>::const_iterator& end):
    _type(Type::Raw)
{
    // Copy to our vector
    _data.insert(_data.end(), begin, end);
}

rle::Block::Block(const uint32_t runLength, const uint8_t value):
    _type(Type::Run)
{
    // Add to our vector as the only entry
    _data.resize(runLength, value);
}

void rle::Block::mergeWith(const Block& other)
{
    // Merging blocks forces them to be raw.
    _type = Type::Raw;
    std::ranges::copy(other._data, std::back_inserter(_data));
}

rle::Block rle::Block::split(const std::size_t length)
{
    // We need to return the data that's been removed
    // Copy to a new raw block
    Block result(_data.begin() + static_cast<int>(length), _data.end());
    // Change its type to Run if we are a Run and it's longer than 1 byte
    if (_type == Type::Run && result.getSize() >= 2)
    {
        result._type = Type::Run;
    }
    // Then truncate our data
    _data.resize(length);
    // And return the new block
    return result;
}

auto getRunLength(
    const std::vector<uint8_t>::const_iterator& begin,
    const std::vector<uint8_t>::const_iterator& end)
{
    // Find the number of consecutive identical values
    const auto c = *begin;
    auto it = begin;
    for (++it; it != end && *it == c; ++it) {}
    return it - begin;
}

std::vector<rle::Block> rle::process(
    const std::vector<uint8_t>::const_iterator& start,
    const std::vector<uint8_t>::const_iterator& end)
{
    // First we decompose into blocks
    std::vector<Block> blocks;
    auto rawStart = start;
    for (auto it = start; it != end; /* increment in loop */)
    {
        const auto runLength = getRunLength(it, end);
        if (runLength < 2)
        {
            // Not good enough; keep looking for a run
            it += runLength;
            continue;
        }

        // We found a good enough run. Write the raw (if any) and then the run
        if (rawStart != it)
        {
            blocks.emplace_back(rawStart, it);
        }
        blocks.emplace_back(runLength, *it);

        it += runLength;
        rawStart = it;
    }

    // We may have a final run of raw bytes
    if (rawStart != end)
    {
        blocks.emplace_back(rawStart, end);
    }

    return blocks;
}

void rle::optimize(std::vector<Block>& blocks, const std::size_t rleBlockCost, const std::size_t maxRawLength, const std::size_t maxRleLength)
{
    // First we split any oversized blocks
    for (auto it = blocks.begin(); it != blocks.end(); /* increment in loop */)
    {
        if (it->getType() == Block::Type::Run && it->getSize() > maxRleLength)
        {
            const auto& data = it->split(maxRleLength);
            ++it;
            it = blocks.insert(it, data);
        }
        else if (it->getType() == Block::Type::Raw && it->getSize() > maxRawLength)
        {
            const auto& data = it->split(maxRawLength);
            ++it;
            it = blocks.insert(it, data);
        }
        else
        {
            ++it;
        }
    }

    // Go through and optimise any instances of these to a single raw block:
    //                    Bytes before                          Bytes after             Requirement
    // * raw(n) + rle(m): rawBlockCost + n + rleBlockCost + 1   rawBlockCost + m + n    m <= rleBlockCost + 1, m + n <= maxRawLength
    // * rle(m) + raw(n): (same as above)
    // * raw(n) + raw(m): rawBlockCost + n + rawBlockCost + m   rawBlockCost + m + n    m + n <= maxRawLength
    auto previous = blocks.begin();
    const auto rleSizeMergeLimit = rleBlockCost + 1;
    for (auto current = previous + 1; current != blocks.end(); /* increment in loop */)
    {
        if (
            (previous->getType() == Block::Type::Raw && current->getType() == Block::Type::Run && current->getSize() <= rleSizeMergeLimit && previous->getSize() + current->getSize() <= maxRawLength) ||
            (current->getType() == Block::Type::Raw && previous->getType() == Block::Type::Run && previous->getSize() <= rleSizeMergeLimit && previous->getSize() + current->getSize() <= maxRawLength) ||
            (previous->getType() == Block::Type::Raw && current->getType() == Block::Type::Raw && previous->getSize() + current->getSize() <= maxRawLength)
        )
        {
            // Combine the data
            previous->mergeWith(*current);
            // Knock out the dead block (slow for a vector, probably not a big deal)
            blocks.erase(current);
            // Fix up the iterator
            current = previous + 1;
            // We go around again pointing at the new current
        }
        else
        {
            // Move on
            previous = current;
            ++current;
        }
    }
}
