#include <cstdint>

#include "utils.h"

extern "C" __declspec(dllexport) const char* getName()
{
    // A pretty name for this compression type
    // Generally, the name of the game it was REd from
    return "Rob Northen compression 1";
}

extern "C" __declspec(dllexport) const char* getExt()
{
    // A string suitable for use as a file extension
    return "rnc1";
}

// "Includes" for C

extern "C"
{
    typedef struct huftable_s {
        uint32_t l1; // +0
        uint16_t l2; // +4
        uint32_t l3; // +6
        uint16_t bit_depth; // +A
    } huftable_t;

    typedef struct vars_s {
        uint16_t max_matches;
        uint16_t enc_key;
        uint32_t pack_block_size;
        uint16_t dict_size;
        uint32_t method;
        uint32_t puse_mode;
        uint32_t input_size;
        uint32_t file_size;

        // inner
        uint32_t bytes_left;
        uint32_t packed_size;
        uint32_t processed_size;
        uint32_t v7;
        uint32_t pack_block_pos;
        uint16_t pack_token, bit_count, v11;
        uint16_t last_min_offset;
        uint32_t v17;
        uint32_t pack_block_left_size;
        uint16_t match_count;
        uint16_t match_offset;
        uint32_t v20, v21;
        uint32_t bit_buffer;

        uint32_t unpacked_size;
        uint32_t rnc_data_size;
        uint16_t unpacked_crc, unpacked_crc_real;
        uint16_t packed_crc;
        uint32_t leeway;
        uint32_t chunks_count;

        uint8_t *mem1;
        uint8_t *pack_block_start;
        uint8_t *pack_block_max;
        uint8_t *pack_block_end;
        uint16_t *mem2;
        uint16_t *mem3;
        uint16_t *mem4;
        uint16_t *mem5;

        uint8_t *decoded;
        uint8_t *window;

        size_t read_start_offset, write_start_offset;
        uint8_t *input, *output, *temp;
        size_t input_offset, output_offset, temp_offset;

        uint8_t tmp_crc_data[2048];
        huftable_t raw_table[16];
        huftable_t pos_table[16];
        huftable_t len_table[16];
    } vars_t;

    vars_t *init_vars();
    int do_pack(vars_t *v);
}

int32_t compress(const uint8_t* pSource, const size_t sourceLength, uint8_t* pDestination, const size_t destinationLength)
{
    // Fill in vars struct, based on what we see in main.c in "p"ack mode
    const auto v = Utils::makeUniqueForMalloc(init_vars());
    v->puse_mode = 'p';
    v->method = 1;
    v->dict_size = 0x8000;
    v->max_matches = 0x1000;
    v->file_size = sourceLength;
    auto sourceCopy = Utils::toVector(pSource, sourceLength);
    v->input = sourceCopy.data();

    // We need to supply buffers of this size - 30MB!
    constexpr int MAX_BUF_SIZE = 0x1E00000;
    std::vector<uint8_t> output(MAX_BUF_SIZE);
    v->output = output.data();
    std::vector<uint8_t> temp(MAX_BUF_SIZE);
    v->temp = temp.data();

    // Then we call the packer...
    if (do_pack(v.get()) != 0)
    {
        return ReturnValues::CannotCompress;
    }

    output.resize(v->output_offset);

    if (output.size() == sourceLength)
    {
        // Seems to indicate a refusal to compress?
        return ReturnValues::CannotCompress;
    }

    // Copy to the destination buffer if it fits
    return Utils::copyToDestination(output, pDestination, destinationLength);
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
    const uint32_t height,
    uint8_t* pDestination,
    const uint32_t destinationLength)
{
    // Compress tilemap
    return compress(pSource, width * height * 2, pDestination, destinationLength);
}
