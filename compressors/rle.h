#pragma once
#include <cstdint>
#include <vector>

class rle
{
public:
    class Block
    {
    public:
        enum class Type
        {
            Raw,
            Run
        };

        // Raw block constructor
        Block(const std::vector<uint8_t>::const_iterator& begin, const std::vector<uint8_t>::const_iterator& end);

        // RLE block constructor
        Block(uint32_t runLength, uint8_t value);

        [[nodiscard]]
        Type getType() const
        {
            return _type;
        }

        void mergeWith(const Block& other);

        [[nodiscard]]
        std::size_t getSize() const
        {
            return _data.size();
        }

        [[nodiscard]]
        const std::vector<uint8_t>& getData() const
        {
            return _data;
        }

        Block split(std::size_t length);

    private:
        Type _type;
        std::vector<uint8_t> _data;
    };

    static std::vector<Block> process(
        const std::vector<uint8_t>::const_iterator& start,
        const std::vector<uint8_t>::const_iterator& end);

    static void optimize(
        std::vector<Block>& blocks,
        std::size_t rleBlockCost,
        std::size_t maxRawLength,
        std::size_t maxRleLength);
};
