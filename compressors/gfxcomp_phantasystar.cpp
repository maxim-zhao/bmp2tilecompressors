#include <vector>
#include <cstdint>

constexpr uint8_t MAX_RUN_SIZE = 0x7f;
constexpr uint8_t RLE_MASK = 0x00;
constexpr uint8_t RAW_MASK = 0x80;

class Block
{
public:
    enum Type
    {
        Raw,
        Run
    };

    Type type;
    std::vector<uint8_t> data;

    // Raw block constructor
    Block(std::vector<uint8_t>::const_iterator begin, std::vector<uint8_t>::const_iterator end)
        : type(Raw)
    {
        // Copy to our vector
        data.insert(data.end(), begin, end);
    }

    // RLE block constructor
    Block(uint32_t runLength, uint8_t value)
        : type(Run)
    {
        // Fill up our vector
        data.resize(runLength, value);
    }
};

void deinterleave(std::vector<uint8_t>& buffer, uint32_t interleaving)
{
    std::vector<uint8_t> tempBuf(buffer);

    // Deinterleave into tempBuf
    const uint32_t bitplaneSize = buffer.size() / interleaving;
    for (std::vector<uint8_t>::size_type src = 0; src < buffer.size(); ++src)
    {
        // ReSharper disable CommentTypo
        // If interleaving is 4 I want to turn
        // AbcdEfghIjklMnopQrstUvwx
        // into
        // AEIMQUbfjnrvcgkoswdhlptx
        // so for a byte at position x
        // x div 4 = offset within this section
        // x mod 4 = which section
        // final position = (x div 4) + (x mod 4) * (section size)
        // ReSharper restore CommentTypo
        const size_t dest = src / interleaving + (src % interleaving) * bitplaneSize;
        tempBuf[dest] = buffer[src];
    }

    // Copy results over the original data
    std::copy(tempBuf.begin(), tempBuf.end(), buffer.begin());
}

uint32_t getRunLength(std::vector<uint8_t>::const_iterator begin, std::vector<uint8_t>::const_iterator end)
{
    // find the number of consecutive identical values
    const uint8_t c = *begin;
    auto it = begin;
    for (++it; it != end && *it == c; ++it)
    {
    }
    return it - begin;
}

void writeRaw(
    std::vector<uint8_t>& dest,
    std::vector<uint8_t>::const_iterator begin,
    std::vector<uint8_t>::const_iterator end)
{
    while (begin < end)
    {
        auto blockLength = end - begin;
        if (blockLength > MAX_RUN_SIZE)
        {
            blockLength = MAX_RUN_SIZE;
        }
        dest.push_back(static_cast<uint8_t>(RAW_MASK | blockLength));
        for (auto i = 0; i < blockLength; ++i)
        {
            dest.push_back(*begin++);
        }
    }
}

void writeRun(std::vector<uint8_t>& destination, uint8_t value, uint32_t count)
{
    if (count == 0)
    {
        return;
    }

    while (count > 0)
    {
        const uint8_t blockLength = count > MAX_RUN_SIZE ? MAX_RUN_SIZE : static_cast<uint8_t>(count);
        destination.push_back(RLE_MASK | blockLength);
        destination.push_back(value);
        count -= blockLength;
    }
}

void compressPlane(
    std::vector<uint8_t>& destination,
    std::vector<uint8_t>::const_iterator source,
    std::vector<uint8_t>::const_iterator sourceEnd)
{
    // First we decompose into blocks
    std::vector<Block> blocks;
    auto rawStart = source;
    for (auto it = source; it != sourceEnd; /* increment in loop */)
    {
        const uint32_t runLength = getRunLength(it, sourceEnd);
        if (runLength < 2)
        {
            // Not good enough; keep looking for a run
            it += static_cast<int>(runLength);
            continue;
        }

        // We found a good enough run. Write the raw (if any) and then the run
        if (rawStart != it)
        {
            blocks.emplace_back(rawStart, it);
        }
        blocks.emplace_back(runLength, *it);

        it += static_cast<int>(runLength);
        rawStart = it;
    }

    // We may have a final run of raw bytes
    if (rawStart != sourceEnd)
    {
        blocks.emplace_back(rawStart, sourceEnd);
    }

    // Go through and optimise any instances of:
    // * raw + run[2] (n+1, 2 bytes) -> n+2+1 (same but more mergeable)
    // * run[2] + raw (2, n+1 bytes) -> 2+n+1 (same but more mergeable)
    // * raw + raw (n+1, m+1 bytes) -> n+m+1 (saves 1, merges results from above)
    // into a single raw block
    if (blocks.size() > 2)
    {
        auto previous = blocks.begin();
        for (auto current = previous + 1; current != blocks.end(); /* increment in loop */)
        {
            if ((previous->type == Block::Raw && current->type == Block::Run && current->data.size() == 2) ||
                (previous->type == Block::Raw && current->type == Block::Raw) ||
                (previous->type == Block::Run && previous->data.size() == 2 && current->type == Block::Raw))
            {
                // Combine the data
                previous->data.insert(previous->data.end(), current->data.begin(), current->data.end());
                previous->type = Block::Raw;
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

    // Now emit them
    for (auto& block : blocks)
    {
        switch (block.type)
        {
        case Block::Raw:
            writeRaw(destination, block.data.begin(), block.data.end());
            break;
        case Block::Run:
            writeRun(destination, block.data[0], block.data.size());
            break;
        }
    }

    // Zero terminator
    destination.push_back(0);
}

uint32_t compress(
    const uint8_t* pSource,
    const size_t sourceLength,
    uint8_t* pDestination,
    const size_t destinationLength,
    uint32_t interleaving)
{
    // Compress sourceLength bytes from pSource to pDestination;
    // return length, or 0 if destinationLength is too small, or -1 if there is an error

    // Copy the data into a buffer
    std::vector<uint8_t> source(pSource, pSource + sourceLength);

    // Deinterleave it
    deinterleave(source, interleaving);

    // Make a buffer to hold the result
    std::vector<uint8_t> destination;
    destination.reserve(destinationLength);

    // Compress each plane in turn
    const auto bitplaneSize = static_cast<int32_t>(sourceLength / interleaving);
    for (std::vector<uint8_t>::const_iterator it = source.begin(); it != source.end(); it += bitplaneSize)
    {
        compressPlane(destination, it, it + bitplaneSize);
    }

    // check length
    if (destination.size() > destinationLength)
    {
        return 0;
    }

    // copy to pDestination
    memcpy_s(pDestination, destinationLength, &destination[0], destination.size());

    // return length
    return destination.size();
}

extern "C" __declspec(dllexport) const char* getName()
{
    // A pretty name for this compression type
    // Generally, the name of the game it was REd from
    return "Phantasy Star RLE";
}

extern "C" __declspec(dllexport) const char* getExt()
{
    // A string suitable for use as a file extension
    // ReSharper disable once StringLiteralTypo
    return "pscompr";
}

extern "C" __declspec(dllexport) int compressTiles(
    const uint8_t* pSource,
    const uint32_t numTiles,
    uint8_t* pDestination,
    const uint32_t destinationLength)
{
    // Compress tiles
    return static_cast<int>(compress(pSource, numTiles * 32, pDestination, destinationLength, 4));
}

extern "C" __declspec(dllexport) int compressTilemap(
    const uint8_t* pSource,
    const uint32_t width,
    uint32_t height,
    uint8_t* pDestination,
    const uint32_t destinationLength)
{
    // Compress tilemap
    return static_cast<int>(compress(pSource, width * height * 2, pDestination, destinationLength, 2));
}
