#include <algorithm>
#include <cstdint>
#include <vector>

extern "C" __declspec(dllexport) const char* getName()
{
    return "Legend of Illusion (Aspect) compression";
}

extern "C" __declspec(dllexport) const char* getExt()
{
    return "aspectcompr";
}

int countZeroes(const uint8_t* pSource)
{
    int zc = 0;
    for (uint8_t i = 0; i < 32; i++)
        if (*pSource++ == 0)
            zc++;
    return (zc);
}

extern "C" __declspec(dllexport) int compressTiles(
    const uint8_t* pSource,
    const uint32_t numTiles,
    uint8_t* pDestination,
    const uint32_t destinationLength)
{
    uint32_t final_size = 0;
    uint32_t dst_index = 0;
    uint8_t xored_buffer[32];
    uint32_t mode_table_len = (numTiles + 3) / 4;
    // this is an easy way to make sure there's space available for all the tiles even when they're not exact multiple of 4
    std::vector<uint8_t> mode_table(mode_table_len);

    if (destinationLength < (numTiles * 32 + 6 + mode_table_len))
        return (0);
    // please give me more space for the data (32 bytes per tile + 6 bytes header + the mode table, in the worst case)

    for (uint8_t i = 0; i < mode_table_len; i++) // set mode_table to all zeroes
        mode_table[i] = 0;

    pDestination[dst_index++] = 0x01; // write version
    pDestination[dst_index++] = 0x00;
    pDestination[dst_index++] = static_cast<uint8_t>(numTiles % 256); // write the number of tiles (little endian)
    pDestination[dst_index++] = static_cast<uint8_t>(numTiles / 256);
    dst_index += 2; // skip two bytes, we'll fill the offset for the mode table later
    final_size = 6;

    for (uint32_t tile_num = 0; tile_num < numTiles; tile_num++)
    {
        int zc = countZeroes(&pSource[tile_num * 32]);
        if (zc != 32)
        {
            // otherwise if zc==32, mode for this tile is 0b00 and thus nothing needs to be written at all for this tile

            xored_buffer[0] = pSource[tile_num * 32]; // create the xored_buffer
            xored_buffer[1] = pSource[tile_num * 32 + 1];
            xored_buffer[0 + 16] = pSource[tile_num * 32 + 16];
            xored_buffer[1 + 16] = pSource[tile_num * 32 + 1 + 16];
            for (uint8_t i = 0; i < 7; i++)
            {
                xored_buffer[2 + i * 2] = pSource[tile_num * 32 + i * 2] ^ pSource[tile_num * 32 + i * 2 + 2];
                xored_buffer[3 + i * 2] = pSource[tile_num * 32 + i * 2 + 1] ^ pSource[tile_num * 32 + i * 2 + 2 + 1];
                xored_buffer[2 + i * 2 + 16] = pSource[tile_num * 32 + i * 2 + 16] ^ pSource[tile_num * 32 + i * 2 + 2 + 16];
                xored_buffer[3 + i * 2 + 16] = pSource[tile_num * 32 + i * 2 + 16 + 1] ^ pSource[tile_num * 32 + i * 2 + 2 + 16 + 1];
            }

            int zc_xored = countZeroes(xored_buffer);

            if (std::max(zc, zc_xored) > 4)
            {
                // if there aren't at least 5 zeroes, there's no point in compression

                mode_table[tile_num / 4] |= ((zc >= zc_xored) ? 0x02 : 0x03) << ((tile_num % 4) * 2);
                // mode 0b10 (compressed) or mode 0b11 (xored then compressed)
                const uint8_t* src = (zc >= zc_xored) ? &pSource[tile_num * 32] : xored_buffer;
                // compress the tile or the XORED tile (if more convenient)

                uint32_t tile_header = 0;
                uint32_t tile_header_loc = dst_index;

                dst_index += 4; // skip the 4 bytes tile header
                final_size += 4;

                for (uint8_t i = 0; i < 32; i++)
                {
                    if (src[i] != 0)
                    {
                        // if the data is not zero
                        tile_header |= (0x01 << i); // set the corresponding bit
                        pDestination[dst_index++] = src[i]; // and append the byte
                        final_size++;
                    }
                }

                pDestination[tile_header_loc++] = tile_header & 0xFF; // write this compressed tile's header
                pDestination[tile_header_loc++] = (tile_header >> 8) & 0xFF;
                pDestination[tile_header_loc++] = (tile_header >> 16) & 0xFF;
                pDestination[tile_header_loc++] = tile_header >> 24;
            }
            else
            {
                // not enough zeroes to be worthy to use tile compression, dump raw data

                mode_table[tile_num / 4] |= 0x01 << ((tile_num % 4) * 2); // mode 0b01 (raw data)

                for (uint8_t i = 0; i < 32; i++)
                    pDestination[dst_index++] = pSource[tile_num * 32 + i]; // append the bytes

                final_size += 32;
            }
        }
    }

    pDestination[4] = static_cast<uint8_t>(final_size % 256); // update the offset for the mode table (little endian)
    pDestination[5] = static_cast<uint8_t>(final_size / 256);

    for (uint32_t i = 0; i < mode_table_len; i++) // append the mode table
        pDestination[dst_index++] = mode_table[i];

    final_size += mode_table_len;

    return (final_size); // report size to caller
}
