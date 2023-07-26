#pragma once
#include <cstdint>
#include <memory>
#include <vector>

class Utils
{
public:
    // Deinterleave a buffer in place
    static void deinterleave(std::vector<uint8_t>& buffer, int interleaving);

    // Convert pointer + length to a vector
    static std::vector<uint8_t> toVector(const uint8_t* pBuffer, uint32_t length);

    // Emit vector to pointer, safely
    [[nodiscard]]
    static int32_t copyToDestination(const std::vector<uint8_t>& source, uint8_t* pDestination, uint32_t destinationLength);
    [[nodiscard]]
    static int32_t copyToDestination(const uint8_t* pSource, uint32_t sourceLength, uint8_t* pDestination, uint32_t destinationLength);

    template <typename T>
    static std::unique_ptr<T, void(*)(void*)> makeUniqueForMalloc(T* pData)
    {
        return std::unique_ptr<T, void(*)(void*)>(pData, free);
    }
};

// API defined return values
class ReturnValues
{
public:
    static constexpr int32_t CannotCompress = -1;
    static constexpr int32_t BufferTooSmall = 0;
};
