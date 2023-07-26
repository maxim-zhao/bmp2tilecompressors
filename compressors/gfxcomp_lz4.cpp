#pragma warning(push,0)
#include "smallz4/smallz4.h"
#pragma warning(pop)
#include <iterator>
#include <string>

#include "utils.h"

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

int32_t compress(
    const uint8_t* pSource,
    const uint32_t sourceLength,
    uint8_t* pDestination,
    const uint32_t destinationLength)
{
    // We put vectors in a little struct for passing around
    auto source = Utils::toVector(pSource, sourceLength);
    std::vector<uint8_t> result;
    struct State
    {
        std::vector<uint8_t>::iterator it;
        std::vector<uint8_t>::iterator end;
        std::back_insert_iterator<std::vector<uint8_t>> dest;
    };
    State state
    {
        source.begin(),
        source.end(),
        std::back_inserter(result)
    };

    // Pass lambdas for the function pointers it wants. These have to be non-capturing so we pass some state too.
    smallz4::lz4(
        [](void* data, size_t numBytes, void* userPtr)
        {
            // return min(remaining
            auto& state = *static_cast<State*>(userPtr);
            const int toCopy = std::min(state.end - state.it, static_cast<int>(numBytes));
            std::copy_n(state.it, toCopy, static_cast<uint8_t*>(data));
            state.it += toCopy;
            return static_cast<size_t>(toCopy);
        },
        [](const void* data, size_t numBytes, void* userPtr)
        {
            const auto& state = *static_cast<State*>(userPtr);
            std::copy_n(static_cast<const int8_t*>(data), numBytes, state.dest);
        },
        65535,
        true,
        &state);

    // Remove framing
    result.erase(result.begin(), result.begin() + 8); // Header + block size = 8
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
    return compress(pSource, width * height * 2, pDestination, destinationLength);
}
