#include <algorithm>
#include <vector>
#include <cstdint>

extern "C" __declspec(dllexport) const char* getName()
{
    // A pretty name for this compression type
    // Generally, the name of the game it was REd from
    return "Sonic 1";
}

extern "C" __declspec(dllexport) const char* getExt()
{
    // A string suitable for use as a file extension
    // ReSharper disable once StringLiteralTypo
    return "soniccompr";
}

void writeWord(std::vector<uint8_t>& buffer, const uint16_t word)
{
    buffer.push_back((word >> 0) & 0xff);
    buffer.push_back((word >> 8) & 0xff);
}

uint32_t getRow(const uint8_t*& source)
{
    // We just need to be consistent with addRow...
    // So we read it as little-endian.
    const uint8_t b0 = *source++;
    const uint8_t b1 = *source++;
    const uint8_t b2 = *source++;
    const uint8_t b3 = *source++;
    return
        (b0 << 0) |
        (b1 << 8) |
        (b2 << 16) |
        (b3 << 24);
}

void addRow(std::vector<uint8_t>& buf, const uint32_t row)
{
    // We write it back as little-endian.
    buf.push_back((row >> 0) & 0xff);
    buf.push_back((row >> 8) & 0xff);
    buf.push_back((row >> 16) & 0xff);
    buf.push_back((row >> 24) & 0xff);
}

extern "C" __declspec(dllexport) int compressTiles(
    const uint8_t* pSource,
    const uint32_t numTiles,
    uint8_t* pDestination,
    const uint32_t destinationLength)
{
    if (numTiles > 0xffff)
    {
        return -1; // error
    }
    std::vector<uint8_t> destination; // the output
    destination.reserve(destinationLength); // avoid reallocation

    // The format:
    // Header:
    //   Magic         dw ; Always $5948
    //   DuplicateRows dw ; Offset (from start of header) of duplicate rows data
    //   ArtData       dw ; Offset (from start of header) of art data
    //   RowCount      dw ; Number of rows of data
    // Unique rows list
    //   Each byte is a bitmask for each tile. 0 = new data, 1 = repeated data.
    //   If 0, the value is the next 4 bytes from ArtData.
    //   If 1, the next byte from the "duplicate rows data" is an index 
    //     into the ArtData, considered as an array of 4-byte entries.

    // We will do this in the dumbest way possible... brute force.
    // First we make the header
    writeWord(destination, 0x5948);
    const auto duplicateRowsOffset = numTiles + 8;
    writeWord(destination, static_cast<uint16_t>(duplicateRowsOffset));
    writeWord(destination, 0); // Art pointer will be filled in later
    const auto rowCount = numTiles * 8;
    writeWord(destination, static_cast<uint16_t>(rowCount));

    // We make a buffer for the art data...
    std::vector<uint32_t> artData;

    // One for the "duplicate rows data"
    std::vector<uint8_t> duplicateRows;

    // Then walk the tiles...
    for (uint32_t i = 0; i < numTiles; ++i)
    {
        uint8_t bitmask = 0;
        // For each row in the tile...
        for (int rowIndex = 0; rowIndex < 8; ++rowIndex)
        {
            bitmask >>= 1; // Shift the bitmask

            // Get the "row". This moves the pointer on...
            uint32_t row = getRow(pSource);

            // Look for it in the art data
            auto it = std::find(artData.begin(), artData.end(), row);

            if (it == artData.end())
            {
                // New art data
                artData.push_back(row);
            }
            else
            {
                // Repeated art data
                bitmask |= 0x80;
                const uint16_t index = static_cast<uint16_t>(std::distance(artData.begin(), it));
                if (index >= 0xF0)
                {
                    duplicateRows.push_back(static_cast<uint8_t>(index >> 8) | 0xf0);
                }
                duplicateRows.push_back(static_cast<uint8_t>(index));
            }
        }

        // Push the bitmask into the buffer
        destination.push_back(bitmask);
    }

    // Then we copy in the "duplicate rows" data...
    destination.insert(destination.end(), duplicateRows.begin(), duplicateRows.end());

    // Now we know the art data offset...
    const auto artDataOffset = destination.size();
    destination[4] = (artDataOffset >> 0) & 0xff;
    destination[5] = (artDataOffset >> 8) & 0xff;

    // Copy over the art data
    for (const unsigned int row : artData)
    {
        addRow(destination, row);
    }

    // check length
    if (destination.size() > destinationLength)
    {
        return 0;
    }
    // copy to dest
    memcpy_s(pDestination, destinationLength, &destination[0], destination.size());
    // return length
    return static_cast<int>(destination.size());
}
