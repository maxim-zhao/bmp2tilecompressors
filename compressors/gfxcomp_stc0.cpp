#include <cstdint>

#include "utils.h"

extern "C" __declspec(dllexport) const char* getName()
{
    return "Simple Tile Compressor Zero";
}

extern "C" __declspec(dllexport) const char* getExt()
{
    return "stc0compr";
}

extern "C" __declspec(dllexport) int32_t compressTiles(
    const uint8_t* pSource,
    const uint32_t numTiles,
    uint8_t* pDestination,
    const uint32_t destinationLength)
{
    if (destinationLength < numTiles * 40 + 1)
    {
        return ReturnValues::BufferTooSmall;
    }
    // please give me more space for the data (up to 40 bytes per tile needed worst case scenario, plus 1 byte terminator)

    uint8_t lastValue = 0;
    uint8_t values[4];
    int finalSize = 0;

    for (unsigned int tileNumber = 0; tileNumber < numTiles; tileNumber++)
    {
        // loop over all tiles
        for (unsigned char i = 0; i < 8; i++)
        {
            // loop over all the 8 rows of the tile
            unsigned char mask = 0b00000000;
            bool lastValid = false;
            unsigned char valuesCount = 0;

            if (pSource[0] == 0x00)
            {
                mask |= 0b10000000;
            }
            else if (pSource[0] == 0xFF)
            {
                mask |= 0b11000000;
            }
            else
            {
                mask |= 0b01000000; // raw
                lastValid = true;
                lastValue = pSource[0];
                values[valuesCount++] = pSource[0];
            }

            if (pSource[1] == 0x00)
            {
                mask |= 0b00100000;
            }
            else if (pSource[1] == 0xFF)
            {
                mask |= 0b00110000;
            }
            else if (!lastValid ||
                pSource[1] != lastValue)
            {
                mask |= 0b00010000; // raw
                lastValid = true;
                lastValue = pSource[1];
                values[valuesCount++] = pSource[1];
            }

            if (pSource[2] == 0x00)
            {
                mask |= 0b00001000;
            }
            else if (pSource[2] == 0xFF)
            {
                mask |= 0b00001100;
            }
            else if (!lastValid ||
                pSource[2] != lastValue)
            {
                mask |= 0b00000100; // raw
                lastValid = true;
                lastValue = pSource[2];
                values[valuesCount++] = pSource[2];
            }

            if (pSource[3] == 0x00)
            {
                mask |= 0b00000010;
            }
            else if (pSource[3] == 0xFF)
            {
                mask |= 0b00000011;
            }
            else if (!lastValid ||
                pSource[3] != lastValue)
            {
                mask |= 0b00000001; // raw
                lastValid = true;
                lastValue = pSource[3];
                values[valuesCount++] = pSource[3];
            }

            *pDestination++ = mask; // write mask byte

            for (unsigned char j = 0; j < valuesCount; j++)
            {
                *pDestination++ = values[j]; // dump uncompressed values
            }

            finalSize += 1 + valuesCount; // add this to finalSize

            pSource += 4; // skip to next 4 bytes
        }
    }

    *pDestination = 0x00; // end of data
    finalSize++;

    return finalSize; // report size to caller
}
