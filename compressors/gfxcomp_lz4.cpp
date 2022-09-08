#pragma warning(push,0)
#include "smallz4/smallz4.h"
#pragma warning(pop)
#include <string>

extern "C" __declspec(dllexport) const char* getName()
{
    // A pretty name for this compression type
    // Generally, the name of the game it was REd from
    static std::string name;
    if (name.empty())
    {
        name = "LZ4 (raw, v";
        name += smallz4::getVersion();
        name += ")";
    }
    return name.c_str();
}

extern "C" __declspec(dllexport) const char* getExt()
{
    // A string suitable for use as a file extension
    return "lz4";
}

const uint8_t* g_pSource;
uint32_t g_sourceRemaining;
uint8_t* g_pDestination;
uint32_t g_destinationRemaining;

size_t getBytes(void* data, size_t numBytes, void* /*userPtr*/)
{
    const auto count = std::min(g_sourceRemaining, numBytes);
    memcpy_s(data, numBytes, g_pSource, g_sourceRemaining);
    g_pSource += count;
    g_sourceRemaining -= count;
    return count;
}

void sendBytes(const void* data, size_t numBytes, void* /*userPtr*/)
{
    const auto count = std::min(g_destinationRemaining, numBytes);
    memcpy_s(g_pDestination, g_destinationRemaining, data, count);
    g_pDestination += count;
    g_destinationRemaining -= count;
}

extern "C" __declspec(dllexport) int compressTiles(
    const uint8_t* pSource,
    const uint32_t numTiles,
    uint8_t* pDestination,
    const uint32_t destinationLength)
{
    g_pSource = pSource;
    g_sourceRemaining = numTiles * 32;
    g_pDestination = pDestination;
    g_destinationRemaining = destinationLength;
    smallz4::lz4(getBytes, sendBytes);
    // Remove framing
    constexpr auto headerSize = 7 + 4; // Header + block size
    constexpr auto footerSize = 4; // Empty block terminator
    const auto size = destinationLength - g_destinationRemaining - headerSize - footerSize;
    memcpy_s(pDestination, destinationLength, pDestination + headerSize, size);
    return static_cast<int>(size);
}

extern "C" __declspec(dllexport) int compressTilemap(
    const uint8_t* pSource,
    const uint32_t width,
    const uint32_t height,
    uint8_t* pDestination,
    const uint32_t destinationLength)
{
    g_pSource = pSource;
    g_sourceRemaining = width * height * 2;
    g_pDestination = pDestination;
    g_destinationRemaining = destinationLength;
    smallz4::lz4(getBytes, sendBytes);
    // Remove framing
    constexpr auto headerSize = 7 + 4; // Header + block size
    constexpr auto footerSize = 4; // Empty block terminator
    const auto size = destinationLength - g_destinationRemaining - headerSize - footerSize;
    memcpy_s(pDestination, destinationLength, pDestination + headerSize, size);
    return static_cast<int>(size);
}
