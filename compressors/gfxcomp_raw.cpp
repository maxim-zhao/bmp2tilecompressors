#include <algorithm>

extern "C" __declspec(dllexport) const char* getName()
{
    // A pretty name for this compression type
    // Generally, the name of the game it was REd from
    return "Raw (uncompressed) binary";
}

extern "C" __declspec(dllexport) const char* getExt()
{
    // A string suitable for use as a file extension
    return "bin";
}

extern "C" __declspec(dllexport) int compressTiles(
    const uint8_t* pSource,
    const uint32_t numTiles,
    uint8_t* pDestination,
    const uint32_t destinationLength)
{
    const uint32_t sourceLength = numTiles * 32;
    if (sourceLength > destinationLength)
    {
        return 0;
    }
    memcpy_s(pDestination, destinationLength, pSource, sourceLength);
    return static_cast<int>(sourceLength);
}

extern "C" __declspec(dllexport) int compressTilemap(
    const uint8_t* pSource,
    const uint32_t width,
    uint32_t height,
    uint8_t* pDestination,
    const uint32_t destinationLength)
{
    const uint32_t sourceLength = width * height * 2;
    if (sourceLength > destinationLength)
    {
        return 0;
    }
    memcpy_s(pDestination, destinationLength, pSource, sourceLength);
    return static_cast<int>(sourceLength);
}

extern "C" __declspec(dllexport) int decompressTiles(
    const uint8_t* source,
    const size_t sourceLength,
    uint8_t* destination,
    const size_t destinationLength)
{
    if (sourceLength > destinationLength)
    {
        return 0;
    }
    memcpy_s(destination, destinationLength, source, sourceLength);
    return static_cast<int>(sourceLength);
}

extern "C" __declspec(dllexport) int decompressTilemap(const uint8_t* source, const size_t sourceLength, uint8_t* destination, size_t destinationLength)
{
    if (sourceLength > destinationLength)
    {
        return 0;
    }
    memcpy_s(destination, destinationLength, source, sourceLength);
    return static_cast<int>(sourceLength);
}
