#include <cstdint>

#include "utils.h"

extern "C"
{
    #include "exomizer/src/membuf.h"
    #include "exomizer/src/exo_helper.h"
}

extern "C" __declspec(dllexport) const char* getName()
{
    // A pretty name for this compression type
    return "Exomizer v2";
}

extern "C" __declspec(dllexport) const char* getExt()
{
    // A string suitable for use as a file extension
    return "exomizer";
}

// The actual compressor function
int32_t compress(
    const uint8_t* pSource,
    const size_t sourceLength,
    uint8_t* pDestination,
    const size_t destinationLength)
{
    struct membuf sourceBuffer{};
    membuf_init(&sourceBuffer);
    membuf_append(&sourceBuffer, pSource, static_cast<int>(sourceLength));

    struct membuf destinationBuffer{};
    membuf_init(&destinationBuffer);

    crunch(&sourceBuffer, &destinationBuffer, nullptr, nullptr);

    membuf_free(&sourceBuffer);

    auto result = Utils::toVector(static_cast<const uint8_t*>(membuf_get(&destinationBuffer)), membuf_memlen(&destinationBuffer));
    membuf_free(&destinationBuffer);

    return Utils::copyToDestination(result, pDestination, destinationLength);
}

extern "C" __declspec(dllexport) int32_t compressTiles(
    const uint8_t* pSource,
    const uint32_t numTiles,
    uint8_t* pDestination,
    const uint32_t destinationLength)
{
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
