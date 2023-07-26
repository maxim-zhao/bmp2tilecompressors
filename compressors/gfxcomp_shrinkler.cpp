#include <cstdint>
#include <iterator>
#include <string>
#include <ranges>

#pragma warning(push, 0)
#include "utils.h"
#include "shrinkler/cruncher/HunkFile.h"
#include "shrinkler/cruncher/Pack.h"
#pragma warning(pop)

int32_t compress(const uint8_t* pSource, const size_t sourceLength, uint8_t* pDestination, const size_t destinationLength)
{
    PackParams params
    {
        .parity_context = false, // "Disable parity context - better on byte-oriented data"
        .iterations = 3, // 1..9, default 3. Seems to get better, then worse, if you use more?
        .length_margin = 3, // 1..100, default 3
        .skip_length = 3000, // 2..100000, default 3000
        .match_patience = 300, // 0..100000, default 300
        .max_same_length = 30, // 1..100000, default 30
    };

    RefEdgeFactory edgeFactory(10000); // 1000..100000000, default 100000

    vector<unsigned char> packBuffer;
    RangeCoder rangeCoder(LZEncoder::NUM_CONTEXTS + NUM_RELOC_CONTEXTS, packBuffer);

    // Crunch the data
    rangeCoder.reset();
    packData(
        const_cast<unsigned char*>(pSource),
        static_cast<int>(sourceLength),
        0,
        &params,
        &rangeCoder,
        &edgeFactory,
        false);
    // It prints stuff to the screen, so we need to draw a newline at the end
    printf("\n");
    rangeCoder.finish();

    return Utils::copyToDestination(packBuffer, pDestination, destinationLength);
}

extern "C" __declspec(dllexport) const char* getName()
{
    // A pretty name for this compression type
    return "Shrinkler";
}

extern "C" __declspec(dllexport) const char* getExt()
{
    // A string suitable for use as a file extension
    return "shrinkler";
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
    uint32_t height,
    uint8_t* pDestination,
    const uint32_t destinationLength)
{
    // Compress tilemap
    return compress(pSource, width * height * 2, pDestination, destinationLength);
}
