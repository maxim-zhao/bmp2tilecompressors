#include <cstdint>
#include <iterator>
#include <stdexcept>
#include <format>
#include <algorithm>
#include <iostream>

#include "utils.h"

extern "C" __declspec(dllexport) const char* getName()
{
    // A pretty name for this compression type
    // Generally, the name of the game it was REd from
    return "Micro Machines compression";
}

extern "C" __declspec(dllexport) const char* getExt()
{
    // A string suitable for use as a file extension
    return "mmcompr";
}

struct Match
{
    enum class Types
    {
        Invalid,
        RawSingle,      //                  Copy one byte to destination
        LZTiny,         // %1nnooooo        Copy n+2 bytes from relative offset -(n+o+2)
        RawSmall,       // $0n              Copy x+8 bytes to destination. n is 0..14
        RawMedium,      // $0f $nn          Copy n+30 bytes to destination. n is 0..254
        RawLarge,       // $0f $ff $nnnn    Raw run. Copy n bytes to destination. n is 0..65535
        RleSmall,       // $1n              Repeat the previous byte n+2 times. n is 0..14
        RleLarge,       // $1f $nn          Repeat the previous byte n+17 times. n is 0..255
        LZSmallNear,    // $2n $oo          Copy n+3 bytes from offset -(o+2)
        LZSmallMid,     // $3n $oo          Copy n+3 bytes from offset -(o+258)
        LZSmallFar,     // $4n $oo          Copy n+3 bytes from offset -(o+514)
        LZLargeNear,    // $5x $oo $nn      Copy n+4 bytes from relative offset -($oo-1)
        LZLargeFar,     // $5f $OO $oo $nn  Copy n+4 bytes from relative offset -($OOoo-1)
        LZReverse,      // $6n $oo          Copy x+3 bytes from -(o+1) to -(o+1+n+3-1) inclusive, i.e. a reversed run
        CountingShort,  // $7n              Output n+2 bytes incrementing from last value written
        CountingLong,   // $7f $nn          Output n+17 bytes incrementing from last value written
    };

    Types type;
    int costToEnd;
    int n;
    int o;
};

static void tryRaw(
    const Match::Types type,
    const int maxN,
    const int nOffset,
    const int bits, 
    const int position,
    const size_t sourceLength,
    const std::vector<Match>& matches,
    Match& bestMatch)
{
    for (int n = 0; n <= maxN; ++n)
    {
        const int byteCount = std::max(n + nOffset, 1);
        if (position + byteCount > static_cast<int>(sourceLength))
        {
            // Past the end, ignore (and any longer)
            return;
        }
        // Cost is 1 bit plus one byte plus the data
        const int costInBits = bits + 8 * byteCount;
        const int costToEnd = costInBits + matches[position + byteCount].costToEnd;
        if (costToEnd < bestMatch.costToEnd)
        {
            bestMatch.type = type;
            bestMatch.costToEnd = costToEnd;
            bestMatch.n = n;
        }
    }
}

static void tryLz(
    const Match::Types type,
    const int maxN,
    const int nOffset,
    const int maxO,
    const int oOffset,
    const int position,
    const bool addNToO,
    const bool backwards,
    const int costInBits,
    const std::vector<uint8_t>& source,
    const std::vector<Match>& matches,
    Match& match)
{
    // Look for LZ matches of length 0+nOffset to maxN+nOffset
    // at offsets -(0+oOffset) to -(maxO+oOffset) (if addNToO is false)
    // or at offsets -(0+oOffset+n) to -(maxO+oOffset+n) (if addNToO is true).
    // Since it is more efficient to iterate over o and then n,
    // we need to select from the minimum range to the maximum and 
    // limit the valid values of n.
    // e.g. with addNToO = false,
    //      maxN = 3, nOffset = 3, we can match lengths 3..6 inclusive.
    //      maxO = 8, oOffset = 2, we can match at distances 2..10 inclusive
    // ABCDEFGHIJ.|
    //  ^^^^^^  ^^^^^^
    //  ^^^     ^^^
    // i.e. any match of length 3..6 at any distance from 2..10
    // OR:
    // e.g. with addNToO = true,
    //      maxN = 3, nOffset = 3, we can match lengths 3..6 inclusive.
    //      maxO = 8, oOffset = 2, we can match at right-side distances 2..10 inclusive
    // ABCDEFGHIJ......|
    //    ^^^           3@10
    //   ^^^^           4@10
    //  ^^^^^           5@10
    // ^^^^^^           6@10 is not allowed! It encodes to %11111111 which is the terminator.
    //     ^^^          3@9
    //    ^^^^          4@9
    //   ^^^^^          5@9
    //  ^^^^^^          6@9
    //            ^^^   3@2
    //           ^^^^   4@2
    //          ^^^^^   5@2
    //         ^^^^^^   6@2
    //           ^^^    3@3
    //          ^^^^    4@3
    //         ^^^^^    5@3
    //        ^^^^^^    6@3
    // ↑↑↑↑↑↑↑↑↑↑↑↑
    // │││││││││││└ length 3..3 real offset 5
    // ││││││││││└─ length 3..4 real offset 6
    // │││││││││└── length 3..5 real offset 7
    // ││││││││└─── length 3..6 real offset 8
    // │││││││└──── length 3..6 real offset 9
    // ││││││└───── length 3..6 real offset 10
    // │││││└────── length 3..6 real offset 11
    // ││││└─────── length 3..6 real offset 12
    // │││└──────── length 3..6 real offset 13
    // ││└───────── length 4..6 real offset 14
    // │└────────── length 5..6 real offset 15
    // └─────────── Not allowed real offset 16
    // We also need to have room after the current position to emit the data, i.e. limited by (length - position).

    // First we calculate the range of offsets of starts of runs to consider...
    auto minOffset = oOffset;
    if (addNToO)
    {
        // Add the smallest n
        minOffset += nOffset;
    }
    int maxOffset = oOffset + maxO;
    if (addNToO)
    {
        // Add the largest n, minus 1 because that would leave all bits set which is not allowed
        maxOffset += maxN - 1;
    }
    // The offset is also limited by how far we are from the start of the data.
    maxOffset = std::min(maxOffset, position);

    // Then we iterate over all offsets. The order doesn't matter - close and far cost the same.
    for (int offset = minOffset; offset <= maxOffset; ++offset)
    {
        // Now figure out how long a run is valid here.
        // First the simple case...
        // - Any count must be no greater than the space available after the current position
        int spaceAvailable = static_cast<int>(source.size()) - position - 1;
        // - The minimum count is that represented by n = 0. We can't encode shorter.
        int minByteCount = nOffset;
        // - The maximum count is that represented by n = maxN
        int maxByteCount = std::min(maxN + nOffset, spaceAvailable);

        if (backwards)
        {
            // If we are counting backwards, the count is limited to the amount we can count to the left
            auto countToLeft = position - offset;
            maxByteCount = std::min(maxByteCount, countToLeft);
        }

        if (addNToO)
        {
            // If we are adding n to o, the numbers are tricky :)
            minByteCount = std::max(minByteCount, offset - maxO);
            maxByteCount = std::min(maxByteCount, offset - minOffset + oOffset);
        }
        // Count matching bytes
        if (maxByteCount < minByteCount)
        {
            // Can't match anything
            continue;
        }
        for (int i = 0; i < maxByteCount; ++i)
        {
            if (backwards)
            {
                if (source[position + i] != source[position - offset - i])
                {
                    // Mismatch; try a different length
                    break;
                }
            }
            else
            {
                if (source[position + i] != source[position - offset + i])
                {
                    // Mismatch; try a different length
                    break;
                }
            }
            const auto byteCount = i + 1;
            if (byteCount >= minByteCount)
            {
                const auto costToEnd = costInBits + matches[position + byteCount].costToEnd;
                if (costToEnd < match.costToEnd)
                {
                    match.type = type;
                    match.costToEnd = costToEnd;
                    match.n = byteCount - nOffset;
                    if (addNToO)
                    {
                        match.o = offset - oOffset - match.n;
//                        if (match.o > maxO || match.o < 0)
//                        {
//                            std::cout << "oh no\n";
//                        }
                    }
                    else
                    {
                        match.o = offset - oOffset;
                    }
                }
            }
        }
    }
}

class BitWriter
{
    std::vector<unsigned char> m_buffer;
    std::size_t m_currentOffset = 0;
    int m_bitsLeft = 0;
public:
    void addBit(const int bit)
    {
        if (m_bitsLeft == 0)
        {
            m_buffer.push_back(0);
            m_bitsLeft = 8;
            m_currentOffset = m_buffer.size() - 1;
        }
        // Assign bits from left to right
        --m_bitsLeft;
        m_buffer[m_currentOffset] |= bit << m_bitsLeft;
    }
    void addByte(const int b)
    {
        m_buffer.push_back(static_cast<uint8_t>(b));
    }

    [[nodiscard]] const std::vector<unsigned char>& buffer() const
    {
        return m_buffer;
    }

    void addBytes(const std::vector<uint8_t>& source, const size_t offset, const int count)
    {
        std::ranges::copy(
            source.begin() + static_cast<int>(offset), 
            source.begin() + static_cast<int>(offset) + count, 
            std::back_inserter(m_buffer));
    }
};

static void tryRLE(
    const Match::Types type, 
    const int maxN, 
    const int nOffset, 
    const int costInBits, 
    const int increment, 
    const int position, 
    const std::vector<uint8_t>& source, 
    const std::vector<Match>& matches, 
    Match& match)
{
    if (position == 0)
    {
        // Impossible
        return;
    }
    // We check for (0..maxN)+nOffset bytes equal to source[position-1]
    const auto minCount = nOffset;
    const auto maxCount = std::min(maxN + nOffset, static_cast<int>(source.size()) - position);
    if (maxCount < minCount)
    {
        // Not enough space for a match at this position
        return;
    }
    auto b = source[position - 1];
    for (int i = 0; i <= maxCount; ++i)
    {
        b = (b + increment) & 0xff;
        if (source[position + i] != b)
        {
            // No need to check for higher n
            return;
        }
        if (i < minCount)
        {
            // Keep going
            continue;
        }
        // Compute cost
        if (const auto costToEnd = costInBits + matches[position + i + 1].costToEnd; 
            costToEnd < match.costToEnd)
        {
            match.costToEnd = costToEnd;
            match.n = i - nOffset;
            match.type = type;
        }
    }
}

static int32_t compress(
    const uint8_t* pSource, 
    const size_t sourceLength, 
    uint8_t* pDestination, 
    const size_t destinationLength)
{
    auto source = Utils::toVector(pSource, sourceLength);

    // We try to implement optimal compression. This means:
    // Make a vector holding info for each location saying what kind of match it was,
    // and its cost to the end. We make it sized one greater to allow references to
    // "the end".
    std::vector<Match> matches(sourceLength + 1);
    // Starting at the end of the data and working backwards...
    for (int position = static_cast<int>(sourceLength - 1); position >= 0; --position)
    {
        // Select the optimal match at this point
        // Micro Machines has a lot of possibilities...

        // Raw single
        // Always possible...
        Match bestMatch
        {
            .type = Match::Types::RawSingle,
            // Cost is one bit in the bitstream plus the byte
            .costToEnd = 1 + 8 + matches[position + 1].costToEnd,
            .n = 1,
            .o = 0,
        };

        // Raw runs are "free" in the bitstream. This is a bit confusing...
        // - A raw run is still signalled by a 1 bit in the bitstream
        // - But then the next thing in the byte stream is consumed immediately
        // - So they effectively "borrow" the bitstream bit from the following thing,
        //   so we can treat them as not emitting a 1 bit
        // RawSmall,       // $0n              Copy x+8 bytes to destination. n is 0..$e
        tryRaw(Match::Types::RawSmall, 0xe, 8, 8, position, sourceLength, matches, bestMatch);

        // RawMedium,      // $0f $nn          Copy n+30 bytes to destination. n is 0..$fe
        tryRaw(Match::Types::RawMedium, 0xfe, 30, 8+8, position, sourceLength, matches, bestMatch);

        // RawLarge,       // $0f $ff $nnnn    Raw run. Copy n bytes to destination. n is 0..$ffff
        tryRaw(Match::Types::RawLarge, 0xffff, 0, 8+8+16, position, sourceLength, matches, bestMatch);

            // LZTiny,        // %1nnooooo        Copy n+2 bytes from relative offset -(n+o+2)
        tryLz(Match::Types::LZTiny, 0b11, 2, 0b11111, 2, position, true, false, 8 + 1, source, matches, bestMatch);

        // LZSmallNear,            // $2n $oo          Copy n+3 bytes from offset -(o+2)
        tryLz(Match::Types::LZSmallNear, 0xf, 3, 0xff, 2, position, false, false, 16 + 1, source, matches, bestMatch);

        // LZSmallMid,            // $3n $oo          Copy n+3 bytes from offset -(o+258)
        tryLz(Match::Types::LZSmallMid, 0xf, 3, 0xff, 258, position, false, false, 16 + 1, source, matches, bestMatch);

        // LZSmallFar,            // $4n $oo          Copy n+3 bytes from offset -(o+514)
        tryLz(Match::Types::LZSmallFar, 0xf, 3, 0xff, 514, position, false, false, 16 + 1, source, matches, bestMatch);

        // LZLargeNear,            // $5O $oo $nn      Copy n+4 bytes from relative offset -($Ooo+1)
        tryLz(Match::Types::LZLargeNear, 0xff, 4, 0xeff, 1, position, false, false, 24 + 1, source, matches, bestMatch);

        // LZLargeFar,           // $5f $OO $oo $nn  Copy n+4 bytes from relative offset -($OOoo+1)
        tryLz(Match::Types::LZLargeFar, 0xff, 4, 0xffff, 1, position, false, false, 32 + 1, source, matches, bestMatch);

        // RleSmall,       // $1n              Repeat the previous byte n+2 times. n is 0..$e
        tryRLE(Match::Types::RleSmall, 0xe, 2, 8 + 1, 0, position, source, matches, bestMatch);

        // RleLarge,       // $1f $nn          Repeat the previous byte n+17 times. n is 0..$ff
        tryRLE(Match::Types::RleLarge, 255, 17, 16 + 1, 0, position, source, matches, bestMatch);

        // LZReverse,      // $6n $oo          Copy n+3 bytes from -(o+1) to -(o+1+n+3-1) inclusive, i.e. a reversed run
        tryLz(Match::Types::LZReverse, 0xf, 3, 0xff, 1, position, false, true, 16 + 1, source, matches, bestMatch);

        // CountingShort,  // $7n              Output n+2 bytes incrementing from last value written
        tryRLE(Match::Types::CountingShort, 0xe, 2, 8 + 1, 1, position, source, matches, bestMatch);

        // CountingLong,   // $7f $nn          Output n+17 bytes incrementing from last value written
        tryRLE(Match::Types::CountingLong, 0xff, 17, 16 + 1, 1, position, source, matches, bestMatch);

        // TODO:
        // - Maybe reorder things to make it go faster? RLE or LZ first?
        //   - Counting still takes time no matter what, we'd just save on assignments

        // Store the best match
        matches[position] = bestMatch;
    }

    // And now we can trace the best path by working through the matches in turn.
    BitWriter b;
    bool needBitstreamBit = true;
    for (size_t offset = 0; offset < sourceLength; /* increment in loop */)
    {
        switch (const auto& match = matches[offset]; match.type)
        {
        case Match::Types::Invalid:
            throw std::runtime_error("Impossible!");
        case Match::Types::RawSingle:      //                  Copy one byte to destination
            //std::cout << std::format("@{:05x}: {:02x} Raw\n", offset, source[offset]);
            b.addBit(0);
            b.addByte(source[offset]);
            ++offset;
            needBitstreamBit = true;
            break;
        case Match::Types::LZTiny:        // %1nnooooo        Copy n+2 bytes from relative offset -(n+o+2)
            if (needBitstreamBit)
            {
                b.addBit(1);
            }
            //std::cout << std::format("@{:05x}: {:02x} LZ tiny: n={} o={}\n", offset, ((1 << 7) | (match.n << 5) | (match.o)), match.n, match.o);
            b.addByte((1 << 7) | (match.n << 5) | (match.o));
            offset += match.n + 2;
            needBitstreamBit = true;
            break;
        case Match::Types::RawSmall:       // $0n              Copy n+8 bytes to destination. n is 0..14
            if (needBitstreamBit)
            {
                b.addBit(1);
            }
            b.addByte(match.n);
            b.addBytes(source, offset, match.n + 8);
            offset += match.n + 8;
            needBitstreamBit = false;
            break;
        case Match::Types::RawMedium:      // $0f $nn          Copy n+30 bytes to destination. n is 0..254
            if (needBitstreamBit)
            {
                b.addBit(1);
            }
            b.addByte(0x0f);
            b.addByte(match.n);
            b.addBytes(source, offset, match.n + 30);
            offset += match.n + 30;
            needBitstreamBit = false;
            break;
        case Match::Types::RawLarge:       // $0f $ff $nnnn    Raw run. Copy n bytes to destination. n is 0..65535
            if (needBitstreamBit)
            {
                b.addBit(1);
            }
            b.addByte(0x0f);
            b.addByte(0xff);
            b.addByte(match.n & 0xff);
            b.addByte(match.n >> 8);
            b.addBytes(source, offset, match.n);
            offset += match.n;
            needBitstreamBit = false;
            break;
        case Match::Types::RleSmall:       // $1n              Repeat the previous byte n+2 times. n is 0..14
            if (needBitstreamBit)
            {
                b.addBit(1);
            }
            b.addByte(0x10 | match.n);
            offset += match.n + 2;
            needBitstreamBit = true;
            break;
        case Match::Types::RleLarge:       // $1f $nn          Repeat the previous byte n+17 times. n is 0..255
            if (needBitstreamBit)
            {
                b.addBit(1);
            }
            b.addByte(0x1f);
            b.addByte(match.n);
            offset += match.n + 17;
            needBitstreamBit = true;
            break;
        case Match::Types::LZSmallNear:            // $2n $oo          Copy n+3 bytes from offset -(o+2)
            if (needBitstreamBit)
            {
                b.addBit(1);
            }
            //std::cout << std::format("@{:05x}: {:02x} LZ small near: n={} o={}\n", offset, 0x20 | match.n, match.n, match.o);
            b.addByte(0x20 | match.n);
            b.addByte(match.o);
            offset += match.n + 3;
            needBitstreamBit = true;
            break;
        case Match::Types::LZSmallMid:            // $3n $oo          Copy n+3 bytes from offset -(o+258)
            if (needBitstreamBit)
            {
                b.addBit(1);
            }
            //std::cout << std::format("@{:05x}: {:02x} LZ small mid: n={} o={}\n", offset, 0x30 | match.n, match.n, match.o);
            b.addByte(0x30 | match.n);
            b.addByte(match.o);
            offset += match.n + 3;
            needBitstreamBit = true;
            break;
        case Match::Types::LZSmallFar:            // $4n $oo          Copy n+3 bytes from offset -(o+514)
            if (needBitstreamBit)
            {
                b.addBit(1);
            }
            //std::cout << std::format("@{:05x}: {:02x} LZ small far: n={} o={}\n", offset, 0x40 | match.n, match.n, match.o);
            b.addByte(0x40 | match.n);
            b.addByte(match.o);
            offset += match.n + 3;
            needBitstreamBit = true;
            break;
        case Match::Types::LZLargeNear:            // $5O $oo $nn      Copy n+4 bytes from relative offset -($Ooo-1)
            if (needBitstreamBit)
            {
                b.addBit(1);
            }
            b.addByte(0x50 | (match.o >> 8));
            b.addByte(match.o & 0xff);
            b.addByte(match.n);
            offset += match.n + 4;
            needBitstreamBit = true;
            break;
        case Match::Types::LZLargeFar:           // $5f $OO $oo $nn  Copy n+4 bytes from relative offset -($OOoo-1)
            if (needBitstreamBit)
            {
                b.addBit(1);
            }
            b.addByte(0x5f);
            b.addByte(match.o >> 8);
            b.addByte(match.o & 0xff);
            b.addByte(match.n);
            offset += match.n + 4;
            needBitstreamBit = true;
            break;
        case Match::Types::LZReverse:      // $6n $oo          Copy n+3 bytes from -(o+1) to -(o+1+n+3-1) inclusive: i.e. a reversed run
            if (needBitstreamBit)
            {
                b.addBit(1);
            }
            b.addByte(0x60 | match.n);
            b.addByte(match.o);
            offset += match.n + 3;
            needBitstreamBit = true;
            break;
        case Match::Types::CountingShort:  // $7n              Output n+2 bytes incrementing from last value written
            if (needBitstreamBit)
            {
                b.addBit(1);
            }
            b.addByte(0x70 | match.n);
            offset += match.n + 2;
            needBitstreamBit = true;
            break;
        case Match::Types::CountingLong:   // $7f $nn          Output n+17 bytes incrementing from last value written
            if (needBitstreamBit)
            {
                b.addBit(1);
            }
            b.addByte(0x7f);
            b.addByte(match.n);
            offset += match.n + 17;
            needBitstreamBit = true;
            break;
        }
    }

    // Add the terminator
    if (needBitstreamBit)
    {
        b.addBit(1);
    }
    b.addByte(0xff);

    // Copy to the destination buffer if it fits
    return Utils::copyToDestination(b.buffer(), pDestination, destinationLength);
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
