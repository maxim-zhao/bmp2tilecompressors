#include <cstdint>

#include "utils.h"

// Header uses reserved word, so we rename it
#define export foo  // NOLINT(clang-diagnostic-keyword-macro)
#include "exomizer/src/exo_helper.h"

extern "C" __declspec(dllexport) const char* getName()
{
    // A pretty name for this compression type
    return "Exomizer v3";
}

extern "C" __declspec(dllexport) const char* getExt()
{
    // A string suitable for use as a file extension
    return "exomizer3";
}

struct io_bufs_located
{
    struct io_bufs io;
    int write_location;
};

// The actual compressor function
static int32_t compress(
    const uint8_t* pSource,
    const size_t sourceLength,
    uint8_t* pDestination,
    const size_t destinationLength)
{
    buf sourceBuffer{};
    buf_init(&sourceBuffer);
    buf_append(&sourceBuffer, pSource, static_cast<int>(sourceLength));

    buf destinationBuffer{};
    buf_init(&destinationBuffer);

    crunch_options options = CRUNCH_OPTIONS_DEFAULT;
    options.flags_proto = 15; // -P
    options.flags_notrait = 1; // -T
    options.direction_forward = 1;

    crunch(
        &sourceBuffer, 
        0, 
        nullptr,
        &destinationBuffer, 
        &options, 
        nullptr);

    buf_free(&sourceBuffer);

    const auto result = Utils::toVector(static_cast<const uint8_t*>(destinationBuffer.data), destinationBuffer.size);
    buf_free(&destinationBuffer);

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
