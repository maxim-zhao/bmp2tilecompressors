#include <vector>
#include <cstdint>

constexpr int MAX_RUN_SIZE = 0x7f;
constexpr int RLE_MASK = 0x00;
constexpr int RAW_MASK = 0x80;

extern "C" __declspec(dllexport) const char* getName()
{
    // A pretty name for this compression type
    // Generally, the name of the game it was REd from
    // ReSharper disable once StringLiteralTypo
    return "High School Kimengumi RLE";
}

extern "C" __declspec(dllexport) const char* getExt()
{
    // A string suitable for use as a file extension
    // ReSharper disable once StringLiteralTypo
    return "hskcompr";
}

void deinterleave(std::vector<uint8_t>& buffer, const unsigned int interleaving)
{
    std::vector<uint8_t> tempBuffer(buffer);

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
    std::copy(tempBuffer.begin(), tempBuffer.end(), buffer.begin());
}

void writeRaw(
    std::vector<uint8_t>& buffer,
    std::vector<uint8_t>::const_iterator begin,
    const std::vector<uint8_t>::const_iterator end)
{
    while (begin < end)
    {
        size_t blockLength = end - begin;
        if (blockLength > MAX_RUN_SIZE)
        {
            blockLength = MAX_RUN_SIZE;
        }
        buffer.push_back(static_cast<uint8_t>(RAW_MASK | blockLength));
        for (size_t i = 0; i < blockLength; ++i)
        {
            buffer.push_back(*begin++);
        }
    }
}

void writeRun(std::vector<uint8_t>& buffer, const uint8_t value, size_t count)
{
    if (count == 0)
    {
        return;
    }

    while (count > 0)
    {
        uint32_t blockLength = count;
        if (blockLength > MAX_RUN_SIZE)
        {
            blockLength = MAX_RUN_SIZE;
        }
        buffer.push_back(static_cast<uint8_t>(RLE_MASK | blockLength));
        buffer.push_back(value);
        count -= blockLength;
    }
}

size_t getRunLength(const std::vector<uint8_t>::const_iterator buffer, const std::vector<uint8_t>::const_iterator end)
{
    // find the number of consecutive identical values
    const uint8_t c = *buffer;
    auto it = buffer;
    for (++it; it != end && *it == c; ++it)
    {
    }
    return it - buffer;
}

int compress(
    const uint8_t* pSource,
    const uint32_t sourceLength,
    uint8_t* pDestination,
    const uint32_t destinationLength,
    const uint32_t interleaving)
{
    // Compress sourceLength bytes from pSource to pDestination;
    // return length, or 0 if destinationLength is too small, or -1 if there is an error

    // Copy the data into a buffer
    std::vector<uint8_t> bufSource(pSource, pSource + sourceLength);

    // Deinterleave it
    deinterleave(bufSource, interleaving);

    // Make a buffer to hold the result
    std::vector<uint8_t> destinationBuffer;
    destinationBuffer.reserve(destinationLength);

    // Compress everything in one go
    auto rawStart = bufSource.cbegin();
    for (auto it = bufSource.cbegin(); it != bufSource.cend(); /* increment in loop */)
    {
        const int runLength = static_cast<int>(getRunLength(it, bufSource.cend()));
        int runLengthNeeded = 3; // normally need a run of 3 to be worth breaking a raw block
        if (it == bufSource.cbegin() || it + runLength == bufSource.cend()) --runLengthNeeded;
        // but at the beginning or end, 2 is enough
        if (runLength < runLengthNeeded)
        {
            // Not good enough; keep looking for a run
            it += runLength;
            continue;
        }

        // We found a good enough run. Write the raw (if any) and then the run
        writeRaw(destinationBuffer, rawStart, it);
        writeRun(destinationBuffer, *it, runLength);
        it += runLength;
        rawStart = it;
    }
    // We may have a final run of raw bytes
    writeRaw(destinationBuffer, rawStart, bufSource.cend());
    // Zero terminator
    destinationBuffer.push_back(0);

    // check length
    if (destinationBuffer.size() > destinationLength)
    {
        return 0;
    }

    // copy to pDestination
    std::copy(destinationBuffer.begin(), destinationBuffer.end(), pDestination);

    // return length
    return static_cast<int>(destinationBuffer.size());
}

extern "C" __declspec(dllexport) int compressTiles(
    const uint8_t* pSource,
    const uint32_t numTiles,
    uint8_t* pDestination,
    const uint32_t destinationLength)
{
    // Compress tiles
    return compress(pSource, numTiles * 32, pDestination, destinationLength, 4);
}

extern "C" __declspec(dllexport) int compressTilemap(
    const uint8_t* pSource,
    const uint32_t width,
    const uint32_t height,
    uint8_t* pDestination,
    const uint32_t destinationLength)
{
    // Compress tilemap
    return compress(pSource, width * height * 2, pDestination, destinationLength, 2);
}
