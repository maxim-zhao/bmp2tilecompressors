#include <cstdint>
#include <iterator>
#include <stdexcept>
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
    const Match::Types type,
    const int maxN,
    const int nOffset,
    const int maxO,
    const int oOffset,
    const int position,
    [[maybe_unused]] const bool addNToO,
    const bool backwards,
    const int costInBits,
    const std::vector<uint8_t>& source,
    const std::vector<Match>& matches,
    Match& match)
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
        // Count matching bytes
        int minByteCount = std::max(nOffset, 1);
        int maxByteCount = std::min(maxN + nOffset, static_cast<int>(source.size() - position));
        for (int i = 0; i <= maxByteCount; ++i)
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
            if (i >= minByteCount)
            {
                auto costToEnd = costInBits + matches[position + i].costToEnd;
                if (costToEnd < match.costToEnd)
                {
                    match.type = type;
                    match.costInBits = costInBits;
                    match.costToEnd = costToEnd;
                    match.n = i - nOffset;
                    match.o = o;
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

void tryRLE(
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
    auto b = source[position - 1];
    for (int n = 0; n <= maxN; ++n)
    {
        auto offset = n + nOffset;
        if (source[position + offset] != b)
        {
            // No need to check for higher n
            return;
        }
        // Compute cost
        if (auto costToEnd = costInBits + matches[position + offset].costToEnd; 
            costToEnd < match.costToEnd)
        {
            match.costToEnd = costToEnd;
            match.costInBits = costInBits;
            match.n = n;
            match.type = type;
        }
        b = (b + increment) & 0xff;
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
        // Always possible... one bit in the stream and one in 
        Match bestMatch
        {
            .type = Match::Types::RawSingle,
            .costInBits = 9, // Cost is one bit plus the byte
            .costToEnd = 9 + matches[position + 1].costToEnd,
            .n = 1,
            .o = 0,
        };

        // RawSmall,       // $0n              Copy x+8 bytes to destination. n is 0..$e
        tryRaw(Match::Types::RawSmall, 0xe, 8, 1+8, position, sourceLength, matches, bestMatch);

        // RawMedium,      // $0f $nn          Copy n+30 bytes to destination. n is 0..$fe
        tryRaw(Match::Types::RawMedium, 0xfe, 30, 1+8+8, position, sourceLength, matches, bestMatch);

        // RawLarge,       // $0f $ff $nnnn    Raw run. Copy n bytes to destination. n is 0..$ffff
        tryRaw(Match::Types::RawLarge, 0xffff, 0, 1+8+8+16, position, sourceLength, matches, bestMatch);

        // LZSmall,        // %1nnooooo        Copy n+2 bytes from relative offset -(n+o+2)
        tryLz(Match::Types::LZSmall, 0b11, 2, 0b11111, 2, position, true, false, 9, source, matches, bestMatch);

        // LZ2,            // $2n $oo          Copy n+3 bytes from offset -(o+2)
        tryLz(Match::Types::LZ2, 0xf, 3, 0xff, 2, position, false, false, 17, source, matches, bestMatch);

        // LZ3,            // $3n $oo          Copy n+3 bytes from offset -(o+258)
        tryLz(Match::Types::LZ3, 0xf, 3, 0xff, 258, position, false, false, 17, source, matches, bestMatch);

        // LZ4,            // $4n $oo          Copy n+3 bytes from offset -(o+514)
        tryLz(Match::Types::LZ4, 0xf, 3, 0xff, 514, position, false, false, 17, source, matches, bestMatch);

        // LZ5,            // $5O $oo $nn      Copy n+4 bytes from relative offset -($Ooo+1)
        tryLz(Match::Types::LZ5, 0xff, 4, 0xeff, 1, position, false, false, 25, source, matches, bestMatch);

        // LZ5f,           // $5f $OO $oo $nn  Copy n+4 bytes from relative offset -($OOoo+1)
        tryLz(Match::Types::LZ5f, 0xff, 4, 0xffff, 1, position, false, false, 33, source, matches, bestMatch);

        // RleSmall,       // $1n              Repeat the previous byte n+2 times. n is 0..$e
        tryRLE(Match::Types::RleSmall, 0xe, 2, 9, 0, position, source, matches, bestMatch);

        // RleLarge,       // $1f $nn          Repeat the previous byte n+17 times. n is 0..$ff
        tryRLE(Match::Types::RleLarge, 255, 17, 17, 0, position, source, matches, bestMatch);

        // LZReverse,      // $6n $oo          Copy n+3 bytes from -(o+1) to -(o+1+n+3-1) inclusive, i.e. a reversed run
        tryLz(Match::Types::LZReverse, 0xf, 3, 0xff, 1, position, false, true, 17, source, matches, bestMatch);

        // CountingShort,  // $7n              Output n+2 bytes incrementing from last value written
        tryRLE(Match::Types::CountingShort, 0xe, 2, 9, 1, position, source, matches, bestMatch);

        // CountingLong,   // $7f $nn          Output n+17 bytes incrementing from last value written
        tryRLE(Match::Types::CountingLong, 0xff, 17, 17, 1, position, source, matches, bestMatch);

        // TODO:
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
