#pragma once
#include <vector>

// Implements a buffer where there are bytes containing a bitstream interleaved with a byte stream.
class InterleavedBitStream
{
    std::vector<unsigned char> m_buffer;
    std::size_t m_currentOffset = 0;
    int m_bitsLeft = 0;

public:
    void addBit(int bit);

    void addByte(int b);

    void addBytes(const std::vector<uint8_t>& source, size_t offset, int count);

    [[nodiscard]] const std::vector<unsigned char>& buffer() const
    {
        return m_buffer;
    }

};
