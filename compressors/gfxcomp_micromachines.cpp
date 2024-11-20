#include <cstdint>
#include <iterator>
#include <stdexcept>
#include <iostream>
#include <format>
#include <algorithm>

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
        LZSmall,        // %1nnooooo        Copy n+2 bytes from relative offset -(n+o+2)
        RawSmall,       // $0n              Copy x+8 bytes to destination. n is 0..14
        RawMedium,      // $0f $nn          Copy n+30 bytes to destination. n is 0..254
        RawLarge,       // $0f $ff $nnnn    Raw run. Copy n bytes to destination. n is 0..65535
        RleSmall,       // $1n              Repeat the previous byte n+2 times. n is 0..14
        RleLarge,       // $1f $nn          Repeat the previous byte n+17 times. n is 0..255
        LZ2,            // $2n $oo          Copy n+3 bytes from offset -(o+2)
        LZ3,            // $3n $oo          Copy n+3 bytes from offset -(o+258)
        LZ4,            // $4n $oo          Copy n+3 bytes from offset -(o+514)
        LZ5,            // $5x $oo $nn      Copy n+4 bytes from relative offset -($oo-1)
        LZ5f,           // $5f $OO $oo $nn  Copy n+4 bytes from relative offset -($OOoo-1)
        LZReverse,      // $6n $oo          Copy x+3 bytes from -(o+1) to -(o+1+n+3-1) inclusive, i.e. a reversed run
        CountingShort,  // $7n              Output n+2 bytes incrementing from last value written
        CountingLong,   // $7f $nn          Output n+17 bytes incrementing from last value written
    };

    Types type;
    int costInBits;
    int costToEnd;
    int n;
    int o;
};

static void tryRaw(
    const int position, //Current position
    const size_t sourceLength, // Length of source data
    const std::vector<Match>& matches, // Current state
    Match& bestMatch, // Current best match
    const int maxN, // maximum value for count in data (0 to this inclusive)
    const int nOffset, // value added to count
    const int bits, // bits to encode it, not counting the raw bytes
    const Match::Types type // type to encode as
)
{
    for (int n = 0; n <= maxN; ++n)
    {
        int byteCount = std::max(n + nOffset, 1);
        if (position + byteCount > static_cast<int>(sourceLength))
        {
            // Past the end, ignore (and any longer)
            return;
        }
        // Cost is 1 bit plus one byte plus the data
        int costInBits = bits + 8 * byteCount;
        int costToEnd = costInBits + matches[position + byteCount].costToEnd;
        if (costToEnd < bestMatch.costToEnd)
        {
            bestMatch.type = type;
            bestMatch.costInBits = costInBits;
            bestMatch.costToEnd = costToEnd;
            bestMatch.n = n;
        }
    }
}

static void tryLz(
    const int position,
    const std::vector<uint8_t>& source,
    const std::vector<Match>& matches,
    Match& match,
    const int maxN,
    const int nOffset,
    const int maxO,
    const int oOffset,
    [[maybe_unused]] const bool addNToO,
    const int costInBits,
    const Match::Types type)
{
    // We look for LZ matches in the uncompressed data to the left.
    // We first try the possible offsets, then check for the longest valid match (if any).
    // Closer or further offsets cost the same, so iteration order doesn't matter.
    for (int o = 0; o <= maxO; ++o)
    {
        int offset = o + oOffset;
        if (offset > position)
        {
            // Past the start, cancel the loop (longer will not be OK)
            break;
        }
        // Count how many bytes match.
        int minByteCount = std::max(nOffset, 1);
        int maxByteCount = std::min(maxN + nOffset, static_cast<int>(source.size() - position));
        const auto mismatch = std::mismatch(
            source.begin() + position, 
            source.begin() + position + maxByteCount,
            source.begin() + position - offset, 
            source.begin() + position - offset + maxByteCount
        );
        const auto byteCount = mismatch.first - source.begin() - position;
        if (byteCount >= minByteCount)
        {
            // Compute the cost
            int costToEnd = costInBits + matches[position + byteCount].costToEnd;
            if (costToEnd < match.costToEnd)
            {
                match.type = type;
                match.costInBits = costInBits;
                match.costToEnd = costToEnd;
                match.n = byteCount - nOffset;
                match.o = o;
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

    void addBytes(const std::vector<uint8_t>& source, size_t offset, int count)
    {
        std::ranges::copy(
            source.begin() + static_cast<int>(offset), 
            source.begin() + static_cast<int>(offset) + count, 
            std::back_inserter(m_buffer));
    }
};

static int32_t compress(const uint8_t* pSource, const size_t sourceLength, uint8_t* pDestination, const size_t destinationLength)
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
        // Always possible... one bit in the stream and one in 
        Match bestMatch
        {
            .type = Match::Types::RawSingle,
            .costInBits = 9, // Cost is one bit plus the byte
            .costToEnd = 9 + matches[position + 1].costToEnd,
            .n = 1,
            .o = 0
        };

        // Raw small
        tryRaw(position, sourceLength, matches, bestMatch, 0xe, 8, 9, Match::Types::RawSmall);

        // 0x1200 -> 0x114b (8.4 bits per byte)

        // Raw medium
        tryRaw(position, sourceLength, matches, bestMatch, 0xfe, 30, 17, Match::Types::RawMedium);

        // 0x1200 -> 0x1226 (8.1 bits per byte)

        // Raw large
        tryRaw(position, sourceLength, matches, bestMatch, 0xffff, 0, 33, Match::Types::RawLarge);

        // 0x1200 -> 0x1206 (8.01 bits per byte)

        // LZSmall,        // %1nnooooo        Copy n+2 bytes from relative offset -(n+o+2)
        tryLz(position, source, matches, bestMatch, 0b11, 2, 0b11111, 2, true, 9, Match::Types::LZSmall);

        // 0x1200 -> 0x0fb1 (6.97 bits per byte)

        // LZ2,            // $2n $oo          Copy n+3 bytes from offset -(o+2)
        tryLz(position, source, matches, bestMatch, 0xf, 3, 0xff, 2, false, 17, Match::Types::LZ2);

        // 0x1200 -> 0x0564 (6.83 bits per byte)

        // LZ3,            // $3n $oo          Copy n+3 bytes from offset -(o+258)
        tryLz(position, source, matches, bestMatch, 0xf, 3, 0xff, 258, false, 17, Match::Types::LZ3);

        // 0x1200 -> 0x0f53 (6.81 bits per byte)

        // LZ4,            // $4n $oo          Copy n+3 bytes from offset -(o+514)
        tryLz(position, source, matches, bestMatch, 0xf, 3, 0xff, 514, false, 17, Match::Types::LZ4);

        // 0x1200 -> 0x0f53 (6.81 bits per byte) (no change)

        // LZ5,            // $5x $oo $nn      Copy n+4 bytes from relative offset -($xoo+1)
        tryLz(position, source, matches, bestMatch, 0xff, 4, 0xeff, 1, false, 25, Match::Types::LZ5);

        // 0x1200 -> 0xf49 (6.79 bits per byte)

        // LZ5f,           // $5f $OO $oo $nn  Copy n+4 bytes from relative offset -($OOoo+1)
        tryLz(position, source, matches, bestMatch, 0xff, 4, 0xffff, 1, false, 33, Match::Types::LZ5f);

        // 0x1200 -> 0xf49 (6.79 bits per byte) (no change)

        // RleSmall,       // $1n              Repeat the previous byte n+2 times. n is 0..14
        // RleLarge,       // $1f $nn          Repeat the previous byte n+17 times. n is 0..255
        // LZReverse,      // $6n $oo          Copy x+3 bytes from -(o+1) to -(o+1+n+3-1) inclusive, i.e. a reversed run
        // CountingShort,  // $7n              Output n+2 bytes incrementing from last value written
        // CountingLong,   // $7f $nn          Output n+17 bytes incrementing from last value written
        // TODO:
        // - Other match types
        // - AddNToO behaviour
        // - Maybe debug each in turn?

        // Store the best match
        matches[position] = bestMatch;

        //std::cout << std::format("{:04X}: best cost to end is {}\n", position, bestMatch.costToEnd);
    }

    //std::cout << std::format("{} bits = {}%\n", matches[0].costToEnd, matches[0].costToEnd * 100.0 / sourceLength / 8);

    // And now we can trace the best path by working through the matches in turn.
    BitWriter b;
    for (size_t offset = 0; offset < sourceLength; /* increment in loop */)
    {
        switch (const auto& match = matches[offset]; match.type)
        {
        case Match::Types::Invalid:
            throw std::runtime_error("Impossible!");
        case Match::Types::RawSingle:      //                  Copy one byte to destination
            b.addBit(0);
            b.addByte(source[offset]);
            ++offset;
            break;
        case Match::Types::LZSmall:        // %1nnooooo        Copy n+2 bytes from relative offset -(n+o+2)
            b.addBit(1);
            b.addByte((1 << 7) | (match.n << 5) | (match.o));
            offset += match.n + 2;
            break;
        case Match::Types::RawSmall:       // $0n              Copy n+8 bytes to destination. n is 0..14
            b.addBit(1);
            b.addByte(match.n);
            b.addBytes(source, offset, match.n + 8);
            offset += match.n + 8;
            break;
        case Match::Types::RawMedium:      // $0f $nn          Copy n+30 bytes to destination. n is 0..254
            b.addBit(1);
            b.addByte(0x0f);
            b.addByte(match.n);
            b.addBytes(source, offset, match.n + 30);
            offset += match.n + 30;
            break;
        case Match::Types::RawLarge:       // $0f $ff $nnnn    Raw run. Copy n bytes to destination. n is 0..65535
            b.addBit(1);
            b.addByte(0x0f);
            b.addByte(0xff);
            b.addByte(match.n & 0xff);
            b.addByte(match.n >> 8);
            b.addBytes(source, offset, match.n);
            offset += match.n;
            break;
        case Match::Types::RleSmall:       // $1n              Repeat the previous byte n+2 times. n is 0..14
            b.addBit(1);
            b.addByte(0x10 | match.n);
            offset += match.n + 2;
            break;
        case Match::Types::RleLarge:       // $1f $nn          Repeat the previous byte n+17 times. n is 0..255
            b.addBit(1);
            b.addByte(0x1f);
            b.addByte(match.n);
            offset += match.n + 17;
            break;
        case Match::Types::LZ2:            // $2n $oo          Copy n+3 bytes from offset -(o+2)
            b.addBit(1);
            b.addByte(0x20 | match.n);
            b.addByte(match.o);
            offset += match.n + 3;
            break;
        case Match::Types::LZ3:            // $3n $oo          Copy n+3 bytes from offset -(o+258)
            b.addBit(1);
            b.addByte(0x30 | match.n);
            b.addByte(match.o);
            offset += match.n + 3;
            break;
        case Match::Types::LZ4:            // $4n $oo          Copy n+3 bytes from offset -(o+514)
            b.addBit(1);
            b.addByte(0x40 | match.n);
            b.addByte(match.o);
            offset += match.n + 3;
            break;
        case Match::Types::LZ5:            // $5O $oo $nn      Copy n+4 bytes from relative offset -($Ooo-1)
            b.addBit(1);
            b.addByte(0x50 | (match.o >> 8));
            b.addByte(match.o & 0xff);
            b.addByte(match.n);
            offset += match.n + 4;
            break;
        case Match::Types::LZ5f:           // $5f $OO $oo $nn  Copy n+4 bytes from relative offset -($OOoo-1)
            b.addBit(1);
            b.addByte(0x5f);
            b.addByte(match.o >> 8);
            b.addByte(match.o & 0xff);
            b.addByte(match.n);
            offset += match.n + 4;
            break;
        case Match::Types::LZReverse:      // $6n $oo          Copy n+3 bytes from -(o+1) to -(o+1+n+3-1) inclusive: i.e. a reversed run
            b.addBit(1);
            b.addByte(0x60 | match.n);
            b.addByte(match.o);
            offset += match.n + 3;
            break;
        case Match::Types::CountingShort:  // $7n              Output n+2 bytes incrementing from last value written
            b.addBit(1);
            b.addByte(0x70 | match.n);
            offset += match.n + 2;
            break;
        case Match::Types::CountingLong:   // $7f $nn          Output n+17 bytes incrementing from last value written
            b.addBit(1);
            b.addByte(0x7f);
            b.addByte(match.n);
            offset += match.n + 17;
            break;
        }
    }

    // Add the terminator
    b.addBit(1);
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
