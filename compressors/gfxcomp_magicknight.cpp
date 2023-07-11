#include <cstdint>
#include <iterator>
#include <vector>

extern "C" __declspec(dllexport) const char* getName()
{
    // A pretty name for this compression type
    // Generally, the name of the game it was REd from
    return "Magic Knight Rayearth 2 RLE or LZ";
}

extern "C" __declspec(dllexport) const char* getExt()
{
    // A string suitable for use as a file extension
    return "mkre2";
}

void rle_emit_raw(std::vector<uint8_t>& result, std::vector<uint8_t>& raw_bytes)
{
    // Emits raw_bytes to result as raw runs, and clears it
    while (!raw_bytes.empty())
    {
        const auto raw_length = std::min(0x7f, static_cast<int>(raw_bytes.size()));
        result.push_back(static_cast<uint8_t>(raw_length));
        std::copy_n(raw_bytes.begin(), raw_length, std::back_inserter(result));
        raw_bytes.erase(raw_bytes.begin(), raw_bytes.begin() + raw_length);
    }
}

std::vector<uint8_t> compress_rle(const std::vector<uint8_t>& data)
{
    std::vector<uint8_t> result;
    result.push_back(0); // RLE marker

    // First we deinterleave the data
    std::vector<std::vector<uint8_t>> bitplanes(4);
    for (std::size_t initial_offset = 0; initial_offset < 4; ++initial_offset)
    {
        auto& bitplane = bitplanes[initial_offset];
        for (std::size_t offset = initial_offset; offset < data.size(); offset += 4)
        {
            bitplane.push_back(data[offset]);
        }
    }

    // Then we compress each in turn
    for (const auto& bitplane : bitplanes)
    {
        // At each position in the data...
        std::vector<uint8_t> raw_bytes;
        for (std::size_t offset = 0; offset < bitplane.size(); /* increment in loop */)
        {
            // A run costs a byte to encode so a run of 3 or more is worth doing.
            // 2 would encode no better than a raw run.
            auto b = bitplane[offset];
            auto run_length = 1;
            for (std::size_t other = offset + 1; other < bitplane.size(); ++other)
            {
                if (bitplane[other] != b)
                {
                    break;
                }
                ++run_length;
            }
            if (run_length > 2)
            {
                // Emit any raw we have stored up
                rle_emit_raw(result, raw_bytes);

                // Truncate if needed
                run_length = std::min(0x7f, run_length);
                // Emit it
                result.push_back(static_cast<uint8_t>(0x80 | run_length));
                result.push_back(b);
                offset += run_length;
            }
            else
            {
                // No RLE. Store in a raw buffer.
                raw_bytes.push_back(b);
                ++offset;
            }
        }
        // Emit any trailing raw run
        rle_emit_raw(result, raw_bytes);

        // Emit a 0 for end of data
        result.push_back(0);
    }

    return result;
}

void add_bitstream_bit(std::vector<uint8_t>& result, int& currentOffset, int& currentBitCount, const int bit)
{
    // If we do not have a byte to use, make one
    if (currentOffset == -1)
    {
        currentOffset = static_cast<int>(result.size());
        result.push_back(0);
        currentBitCount = 0;
    }

    // Add it in
    auto b = result[currentOffset];
    b <<= 1;
    b |= bit;
    result[currentOffset] = b;
    ++currentBitCount;

    // If it's full, then it's time to forget it and allocate a new one next time
    if (currentBitCount == 8)
    {
        currentOffset = -1;
    }
}

std::vector<uint8_t> compress_lz(const std::vector<uint8_t>& data)
{
    std::vector<uint8_t> result;
    auto current_bitmask_offset = -1;
    auto current_bitmask_bit_count = 0;

    // We split the data into either raw bytes or LZ references.
    // LZ runs must be between 3 and 18 bytes long, at a max distance of 0xf000 = -4096 bytes from the current address.
    for (int offset = 0; offset < static_cast<int>(data.size()); /* increment in loop */)
    {
        // Look for the longest run in data before the current offset which matches the current data
        auto best_lz_offset = -1;
        auto best_lz_length = -1;

        // Minimum distance is -1
        // Maximum distance is -4096
        const auto min_lz_offset = std::max(0, offset - 4096);
        const auto max_lz_offset = offset;
        for (auto i = min_lz_offset; i < max_lz_offset; ++i)
        {
            // Check for the match length at i compared to offset
            const auto maxMatchLength = std::min(19, static_cast<int>(data.size()) - offset);
            for (auto matchLength = 0; matchLength < maxMatchLength; ++matchLength)
            {
                if (data[i + matchLength] != data[offset + matchLength])
                {
                    // Mismatch; stop looking here
                    break;
                }
                // Else see if it's a new record
                if (matchLength > best_lz_length)
                {
                    best_lz_length = matchLength;
                    best_lz_offset = i;
                }
            }
            if (best_lz_length >= 18)
            {
                // No point finding anything longer
                break;
            }
        }

        // Did we find anything?
        if (best_lz_length > 2)
        {
            // Yes: emit an LZ reference
            // A 1 bit in the bitstream
            add_bitstream_bit(result, current_bitmask_offset, current_bitmask_bit_count, 1);
            // Then the length - 3 in the high 4 bits
            const auto length = std::min(18, best_lz_length);
            auto lz_word = (length - 3) << 12;
            // And the relative offset in the remaining 12 bits, as 2's complement
            const auto relative_offset = best_lz_offset - offset;
            lz_word |= relative_offset & 0xfff;
            // And then emit it
            result.push_back((lz_word >> 0) & 0xff);
            result.push_back((lz_word >> 8) & 0xff);
            // And move on
            offset += length;
        }
        else
        {
            // No: emit a raw byte
            // A 0 bit in the bitstream
            add_bitstream_bit(result, current_bitmask_offset, current_bitmask_bit_count, 0);
            // Then the raw byte
            result.push_back(data[offset]);
            // And move on
            ++offset;
        }
    }
    // Then terminate with a 0 LZ reference
    add_bitstream_bit(result, current_bitmask_offset, current_bitmask_bit_count, 1);
    result.push_back(0);
    result.push_back(0);

    return result;
}

extern "C" __declspec(dllexport) int compressTiles(
    const uint8_t* pSource,
    const uint32_t numTiles,
    uint8_t* pDestination,
    const uint32_t destinationLength)
{
    // We try RLE or LZ and pick the winner
    // First get bytes into a vector
    std::vector<uint8_t> source;
    std::copy_n(pSource, numTiles * 32, std::back_inserter(source));
    const auto& rle = compress_rle(source);
    const auto& lz = compress_lz(source);

    printf("Raw: %d bytes. RLE: %d bytes. LZ: %d bytes.\n", source.size(), rle.size(), lz.size());

    const auto& smaller = rle.size() < lz.size() ? rle : lz;

    if (smaller.size() >= destinationLength)
    {
        return 0; // Need a bigger buffer
    }

    // Else emit the smaller
    std::ranges::copy(smaller, std::bit_cast<uint8_t*>(pDestination));

    return static_cast<int>(smaller.size() + 1);
}
