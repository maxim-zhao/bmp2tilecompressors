#include <algorithm>
#include <vector>
#include <cstdint>
#include <ranges>

extern "C" __declspec(dllexport) const char* getName()
{
    // A pretty name for this compression type
    // Generally, the name of the game it was REd from
    return "Sonic 2";
}

extern "C" __declspec(dllexport) const char* getExt()
{
    // A string suitable for use as a file extension
    // ReSharper disable once StringLiteralTypo
    return "sonic2compr";
}

void writeWord(std::vector<uint8_t>& buffer, const uint16_t word)
{
    buffer.push_back((word >> 0) & 0xff);
    buffer.push_back((word >> 8) & 0xff);
}

bool allZero(const std::vector<uint8_t>& tile)
{
    return std::ranges::all_of(
        tile,
        [](auto x)
        {
            return x == 0;
        });
}

bool shouldCompress(const std::vector<uint8_t>& tile)
{
    int count = 0;
    for (const uint8_t value : tile)
    {
        if (value == 0)
        {
            ++count;
        }
    }
    return count > 4;
}

std::vector<uint8_t> xorTile(const std::vector<uint8_t>& tile)
{
    std::vector<uint8_t> copy(tile);
    for (int i = 0; i < 7; ++i)
    {
        copy[i + 2] ^= copy[i + 0];
        copy[i + 3] ^= copy[i + 1];
        copy[i + 18] ^= copy[i + 16];
        copy[i + 19] ^= copy[i + 17];
    }
    return copy;
}


std::vector<uint8_t> compress(const std::vector<uint8_t>& tile)
{
    std::vector<uint8_t> result;
    // Emit bitmask for zero bits
    uint32_t bitmask = 0;
    for (const uint8_t value : tile)
    {
        bitmask <<= 1;
        if (value != 0)
        {
            bitmask &= 1;
        }
    }
    result.push_back((bitmask >> 0) & 0xff);
    result.push_back((bitmask >> 8) & 0xff);
    result.push_back((bitmask >> 16) & 0xff);
    result.push_back((bitmask >> 24) & 0xff);
    // Then the non-zero bytes
    for (const uint8_t value : tile)
    {
        if (value != 0)
        {
            result.push_back(value);
        }
    }
    return result;
}

extern "C" __declspec(dllexport) int compressTiles(
    const uint8_t* pSource,
    const uint32_t numTiles,
    uint8_t* pDestination,
    const uint32_t destinationLength)
{
    if (numTiles > 0xffff)
    {
        return -1; // error
    }
    std::vector<uint8_t> destination; // the output
    destination.reserve(destinationLength); // avoid reallocation

    // Format is:
    // dw 0001 ; meaningless header (little-endian)
    // dw TileCount ; Tile count (little-endian)
    // dw OffsetToBitStream ; Offset of stream 2, relative to start of data
    // dsb n Data ; stream 1
    // dsb n CompressionData ; stream 2
    // Compression data holds 2 bits per tile, right-aligned in CompressionData:
    // 00 = all zeroes
    // 01 = raw, copy 32 bytes from Data
    // 02 = compressed tile, see below
    // 03 = XORed compressed data, see below
    // Compressed data then consists of a run of bytes in Data:
    // 4B: 32 bits, 1 = emit a byte from Data, 0 = emit a 0
    // So it makes sense to use this is more than 4 bytes of the 32 are 0.
    // XOR compressed data is the same except after decoding, the bytes are XORed against each other within bitplanes.
    // So we need to pre-process the same way to check if it yields some 0s.

    // First we make the header
    writeWord(destination, 0x0001);
    writeWord(destination, static_cast<uint16_t>(numTiles));
    writeWord(destination, 0x0000); // Offset to fill in later

    // Then we make a buffer for the compression bitstream. This will hold a byte per tile which we'll squash later.
    std::vector<uint8_t> bitStream;
    bitStream.reserve(numTiles);

    // Then for each tile...
    std::vector<uint8_t> tile(32);
    for (auto i = 0u; i < numTiles; ++i)
    {
        // Copy to temp buffer
        std::copy_n(pSource, 32, tile.begin());

        // If all 0, it's easy
        if (allZero(tile))
        {
            bitStream.push_back(0);
        }
        else if (shouldCompress(tile))
        {
            bitStream.push_back(2);
            const auto& compressed = compress(tile);
            std::ranges::copy(compressed, destination.end());
        }
        else
        {
            if (const auto& xored = xorTile(tile); shouldCompress(xored))
            {
                bitStream.push_back(3);
                const auto& compressed = compress(xored);
                std::ranges::copy(compressed, destination.end());
            }
            else
            {
                // Uncompressed
                bitStream.push_back(1);
                std::ranges::copy(tile, destination.end());
            }
        }
    }

    // Update the offset
    const auto offsetOfBitstream = static_cast<uint16_t>(destination.size());
    destination[4] = (offsetOfBitstream >> 0) & 0xff;
    destination[5] = (offsetOfBitstream >> 8) & 0xff;

    // And then append it
    int nibbleCount = 0;
    uint8_t accumulator = 0;
    for (const uint8_t b : bitStream)
    {
        accumulator <<= 2;
        accumulator |= b;
        if (++nibbleCount == 4)
        {
            destination.push_back(accumulator);
            accumulator = 0;
            nibbleCount = 0;
        }
    }
    if (nibbleCount != 0)
    {
        // Left over bits in accumulator
        destination.push_back(accumulator);
    }

    // check length
    if (destination.size() > destinationLength)
    {
        return 0;
    }
    // copy to dest
    memcpy_s(pDestination, destinationLength, destination.data(), destination.size());
    // return length
    return static_cast<int>(destination.size());
}
