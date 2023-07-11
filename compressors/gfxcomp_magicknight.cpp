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

void rleEmitRaw(std::vector<uint8_t>& result, std::vector<uint8_t>& rawBytes)
{
    // Emits raw_bytes to result as raw runs, and clears it
    while (!rawBytes.empty())
    {
        const auto rawLength = std::min(0x7f, static_cast<int>(rawBytes.size()));
        result.push_back(static_cast<uint8_t>(rawLength | 0x80));
        std::copy_n(rawBytes.begin(), rawLength, std::back_inserter(result));
        rawBytes.erase(rawBytes.begin(), rawBytes.begin() + rawLength);
    }
}

std::vector<uint8_t> compressRle(const std::vector<uint8_t>& data)
{
    std::vector<uint8_t> result;
    result.push_back(0); // RLE marker

    // First we deinterleave the data
    std::vector<std::vector<uint8_t>> bitplanes(4);
    for (std::size_t initialOffset = 0; initialOffset < 4; ++initialOffset)
    {
        auto& bitplane = bitplanes[initialOffset];
        for (std::size_t offset = initialOffset; offset < data.size(); offset += 4)
        {
            bitplane.push_back(data[offset]);
        }
    }

    // Then we compress each in turn
    for (const auto& bitplane : bitplanes)
    {
        // At each position in the data...
        std::vector<uint8_t> rawBytes;
        for (std::size_t offset = 0; offset < bitplane.size(); /* increment in loop */)
        {
            // A run costs 2 bytes to encode so a run of 3 or more is worth doing.
            // 2 would encode no better than a raw run.
            auto b = bitplane[offset];
            auto runLength = 1;
            for (std::size_t other = offset + 1; other < bitplane.size(); ++other)
            {
                if (bitplane[other] != b)
                {
                    break;
                }
                ++runLength;
            }
            if (runLength > 2)
            {
                // Emit any raw we have stored up
                rleEmitRaw(result, rawBytes);

                // Truncate if needed
                runLength = std::min(0x7f, runLength);
                // Emit it
                result.push_back(static_cast<uint8_t>(runLength));
                result.push_back(b);
                offset += runLength;
            }
            else
            {
                // No RLE. Store in a raw buffer.
                rawBytes.push_back(b);
                ++offset;
            }
        }
        // Emit any trailing raw run
        rleEmitRaw(result, rawBytes);

        // Emit a 0 for end of data
        result.push_back(0);
    }

    return result;
}

void addBitstreamBit(std::vector<uint8_t>& result, int& currentOffset, int& currentBitCount, const int bit)
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
    b |= (bit << currentBitCount);
    result[currentOffset] = b;
    ++currentBitCount;

    // If it's full, then it's time to forget it and allocate a new one next time
    if (currentBitCount == 8)
    {
        currentOffset = -1;
    }
}

std::vector<uint8_t> compressLz(const std::vector<uint8_t>& data)
{
    std::vector<uint8_t> result;
    // Marker 1 for LZ
    result.push_back(1);
    auto currentBitmaskOffset = -1;
    auto currentBitmaskBitCount = 0;

    // We split the data into either raw bytes or LZ references.
    // LZ runs must be between 3 and 18 bytes long, at a max distance of 0xf000 = -4096 bytes from the current address.
    // However, a run of length 3 at offset -4096 is a sentinel for the end of the data, so must be avoided.
    for (int offset = 0; offset < static_cast<int>(data.size()); /* increment in loop */)
    {
        // Look for the longest run in data before the current offset which matches the current data
        auto bestLzOffset = -1;
        auto bestLzLength = -1;

        // Minimum distance is -1
        // Maximum distance is -4096
        const auto minLzOffset = std::max(0, offset - 4096);
        const auto maxLzOffset = offset - 1;
        for (auto i = minLzOffset; i <= maxLzOffset; ++i)
        {
            // Check for the match length at i compared to offset
            const auto maxMatchLength = std::min(18, static_cast<int>(data.size()) - offset);
            for (auto matchLength = 0; matchLength < maxMatchLength; ++matchLength)
            {
                if (data[i + matchLength] != data[offset + matchLength])
                {
                    // Mismatch; stop looking here
                    break;
                }
                // Else see if it's a new record
                // matchLength will be n here if we have matched n+1 bytes so far
                if (matchLength + 1 > bestLzLength && !(matchLength + 1 == 3 && bestLzOffset == offset - 4096))
                {
                    bestLzLength = matchLength + 1;
                    bestLzOffset = i;
                }
            }
            if (bestLzLength == maxMatchLength)
            {
                // No point searching for anything better, we already found the max
                break;
            }
        }

        // Did we find anything?
        if (bestLzLength > 2)
        {
            // Yes: emit an LZ reference
            // A 1 bit in the bitstream
            addBitstreamBit(result, currentBitmaskOffset, currentBitmaskBitCount, 0);
            // Then the length - 3 in the high 4 bits
            auto lzWord = (bestLzLength - 3) << 12;
            // And the relative offset in the remaining 12 bits, as 2's complement
            const auto relativeOffset = bestLzOffset - offset;
            lzWord |= relativeOffset & 0xfff;
            // And then emit it
            result.push_back((lzWord >> 0) & 0xff);
            result.push_back((lzWord >> 8) & 0xff);
            // And move on
            offset += bestLzLength;
        }
        else
        {
            // No: emit a raw byte
            // A 0 bit in the bitstream
            addBitstreamBit(result, currentBitmaskOffset, currentBitmaskBitCount, 1);
            // Then the raw byte
            result.push_back(data[offset]);
            // And move on
            ++offset;
        }
    }
    // Then terminate with a 0 LZ reference
    addBitstreamBit(result, currentBitmaskOffset, currentBitmaskBitCount, 0);
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
    const auto& rle = compressRle(source);
    const auto& lz = compressLz(source);

    const auto& smaller = rle.size() < lz.size() ? rle : lz;

    if (smaller.size() >= destinationLength)
    {
        return 0; // Need a bigger buffer
    }

    // Else emit the smaller
    std::ranges::copy(smaller, std::bit_cast<uint8_t*>(pDestination));

    return static_cast<int>(smaller.size());
}
