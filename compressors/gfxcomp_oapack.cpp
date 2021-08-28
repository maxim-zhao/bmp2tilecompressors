#include <cstdint>
#include <algorithm>

// Forward declares for oapack stuff. It uses globals so we have to poke into them to make it work.
int FindOptimalSolution();
int EmitCompressed();
extern unsigned char* packedData;
extern unsigned char* data;
extern int packedBits;
extern int size;

int compress(const uint8_t* pSource, const size_t sourceLength, uint8_t* pDestination, const size_t destinationLength)
{
    data = const_cast<unsigned char*>(pSource);
    size = static_cast<int>(sourceLength);

    packedBits = FindOptimalSolution();
    const auto compressedSize = static_cast<size_t>(EmitCompressed());

    // Check size
    if (compressedSize > destinationLength)
    {
        return 0;
    }

    // Copy to pDestination
    memcpy_s(pDestination, destinationLength, &packedData, compressedSize);

    // Done
    return static_cast<int>(compressedSize);
}

extern "C" __declspec(dllexport) const char* getName()
{
    // A pretty name for this compression type
    // Generally, the name of the game it was REd from
    return "aPLib (oapack)";
}

extern "C" __declspec(dllexport) const char* getExt()
{
    // A string suitable for use as a file extension
    return "oapack";
}

extern "C" __declspec(dllexport) int compressTiles(
    const uint8_t* pSource,
    const uint32_t numTiles,
    uint8_t* pDestination,
    const uint32_t destinationLength)
{
    // Compress tiles
    return compress(pSource, numTiles * 32, pDestination, destinationLength);
}

extern "C" __declspec(dllexport) int compressTilemap(
    const uint8_t* pSource,
    const uint32_t width,
    uint32_t height,
    uint8_t* pDestination,
    const uint32_t destinationLength)
{
    // Compress tilemap
    return compress(pSource, width * height * 2, pDestination, destinationLength);
}
