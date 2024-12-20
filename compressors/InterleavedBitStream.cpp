#include "InterleavedBitStream.h"

#include <iterator>

InterleavedBitStream::InterleavedBitStream(bool rightToLeft)
: m_rightToLeft(rightToLeft)
{
}

void InterleavedBitStream::addBit(const int bit)
{
    if (m_bitsLeft == 0)
    {
        m_buffer.push_back(0);
        m_bitsLeft = 8;
        m_currentOffset = m_buffer.size() - 1;
    }
    // Assign bits from left to right
    --m_bitsLeft;
    if (m_rightToLeft)
    {
        m_buffer[m_currentOffset] |= bit << (7 - m_bitsLeft);
    }
    else
    {
        m_buffer[m_currentOffset] |= bit << m_bitsLeft;
    }
}

void InterleavedBitStream::addByte(const int b)
{
    m_buffer.push_back(static_cast<uint8_t>(b));
}

void InterleavedBitStream::addBytes(const std::vector<uint8_t>& source, const size_t offset, const int count)
{
    std::ranges::copy(
        source.begin() + static_cast<int>(offset),
        source.begin() + static_cast<int>(offset) + count,
        std::back_inserter(m_buffer));
}
