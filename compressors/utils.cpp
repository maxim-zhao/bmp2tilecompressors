#include "utils.h"
#include <algorithm>
#include <iterator>
#include <vector>

void Utils::deinterleave(std::vector<uint8_t>& buffer, const int interleaving)
{
    std::vector tempBuffer(buffer);

    // Deinterleave into tempBuffer
    const size_t bitplaneSize = buffer.size() / interleaving;
    for (size_t src = 0; src < buffer.size(); ++src)
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
        tempBuffer[dest] = buffer[src];
    }

    // Copy results over the original data
    std::ranges::copy(tempBuffer, buffer.begin());
}

std::vector<uint8_t> Utils::toVector(const uint8_t* pBuffer, const uint32_t length)
{
    return {pBuffer, pBuffer + length};
}

int32_t Utils::copyToDestination(const std::vector<uint8_t>& source, uint8_t* pDestination, const uint32_t destinationLength)
{
    if (source.size() > destinationLength)
    {
        return ReturnValues::BufferTooSmall;
    }
    std::ranges::copy(source, pDestination);
    return static_cast<int32_t>(source.size());
}

int32_t Utils::copyToDestination(const uint8_t* pSource, const uint32_t sourceLength, uint8_t* pDestination, const uint32_t destinationLength)
{
    if (sourceLength > destinationLength)
    {
        return ReturnValues::BufferTooSmall;
    }
    std::copy_n(pSource, sourceLength, pDestination);
    return static_cast<int32_t>(sourceLength);
}

