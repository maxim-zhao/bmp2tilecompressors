#include <cstdint>

#include "utils.h"

extern "C" __declspec(dllexport) const char* getName()
{
    return "Wonder Boy RLE";
}

extern "C" __declspec(dllexport) const char* getExt()
{
    return "wbcompr";
}

extern "C" __declspec(dllexport) int compressTiles(
    const uint8_t* pSource,
    const uint32_t numTiles,
    uint8_t* pDestination,
    const uint32_t destinationLength)
{
    if (destinationLength < numTiles * 32 * 3 + 2 * 4)
    {
        return ReturnValues::BufferTooSmall;
        // Please give me more space for the data (up to 3 times the data size plus 2 bytes terminator per each bitplane)
    }

    int finalSize = 0;

    for (auto bp = 0; bp < 4; bp++)
    {
        // loop over all the 4 bitplanes
        uint32_t srcIndex = bp;

        while (srcIndex < numTiles * 32)
        {
            // find the run
            uint32_t runSize = 1;
            const uint8_t runValue = pSource[srcIndex];
            srcIndex += 4;

            while (srcIndex < numTiles * 32 && pSource[srcIndex] == runValue)
            {
                runSize++;
                srcIndex += 4;
            }

            // dump the run
            while (runSize >= 255)
            {
                if ((run_size==256) && ((run_value==0x00) || (run_value==0xFF))) {       // handle this corner case saving one byte by storing one 254 bytes run and one 2 bytes run
                    *pDestination++=0x00;
                    *pDestination++=254;
                    *pDestination++=run_value;
                    *pDestination++=0xFF;
                    *pDestination++=run_value;
                    final_size+=5;
                    run_size-=256;
                } else {
                    *pDestination++ = 0x00;
                    *pDestination++ = 255;
                    *pDestination++ = runValue;
                    finalSize += 3;
                    runSize -= 255;
                }
            }

            if (runSize >= 3)
            {
                *pDestination++ = 0x00;
                *pDestination++ = static_cast<uint8_t>(runSize);
                *pDestination++ = runValue;
                finalSize += 3;
            }
            else if (runSize == 2)
            {
                *pDestination++ = 0xFF;
                *pDestination++ = runValue;
                finalSize += 2;
            }
            else if (runSize == 1)
            {
                if (runValue == 0x00 || runValue == 0xFF)
                {
                    *pDestination++ = 0x00;
                    *pDestination++ = 1;
                    *pDestination++ = runValue;
                    finalSize += 3;
                }
                else
                {
                    *pDestination++ = runValue;
                    finalSize += 1;
                }
            }
        }

        // end of bitplane
        *pDestination++ = 0x00;
        *pDestination++ = 0x00;
        finalSize += 2;
    }

    return finalSize; // report size to caller
}
