#include <windows.h>
#include <vector>
#include <map>
#include <cstdint>
#include <algorithm>
using namespace::std;

typedef vector<uint8_t> buffer;

// Forward declares
int compress(char* source, int sourceLen, char* dest, int destLen, int interleaving);
void deinterleave(buffer& buf, int interleaving);
void compress_tile(const buffer& src, buffer& dest);

extern "C" __declspec(dllexport) char* getName()
{
	// A pretty name for this compression type
	// Generally, the name of the game it was REd from
	return "Sonic 1";
}

extern "C" __declspec(dllexport) char* getExt()
{
	// A string suitable for use as a file extension
	return "soniccompr";
}

void writeWord(buffer& buf, uint16_t word)
{
	buf.push_back((word >> 0) & 0xff);
	buf.push_back((word >> 8) & 0xff);
}

uint32_t getRow(uint8_t*& source)
{
	// We just need to be consistent with addRow...
	// So we read it as little-endian.
	uint8_t b0 = *source++;
	uint8_t b1 = *source++;
	uint8_t b2 = *source++;
	uint8_t b3 = *source++;
	return
		(b0 <<  0) |
		(b1 <<  8) |
		(b2 << 16) |
		(b3 << 24);
}

void addRow(buffer& buf, uint32_t row)
{
	// We write it back as little-endian.
	buf.push_back((row >>  0) & 0xff);
	buf.push_back((row >>  8) & 0xff);
	buf.push_back((row >> 16) & 0xff);
	buf.push_back((row >> 24) & 0xff);
}

extern "C" __declspec(dllexport) int compressTiles(uint8_t* source, int numTiles, uint8_t* dest, int destLen)
{
	if (numTiles > 0xffff)
	{
		return -1; // error
	}
	buffer buf; // the output
	buf.reserve(destLen); // avoid reallocation

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
	writeWord(buf, 0x5948);
	int duplicateRowsOffset = numTiles + 8;
	writeWord(buf, (uint16_t)duplicateRowsOffset);
	writeWord(buf, 0); // Art pointer will be filled in later
	int rowCount = numTiles * 8;
	writeWord(buf, (uint16_t)rowCount);

	// We make a buffer for the art data...
	std::vector<uint32_t> artData;

	// One for the "duplicate rows data"
	std::vector<uint8_t> duplicateRows;

	// Then walk the tiles...
	for (int i = 0; i < numTiles; ++i)
	{
		uint8_t bitmask = 0;
		// For each row in the tile...
		for (int rowIndex = 0; rowIndex < 8; ++rowIndex)
		{
			bitmask >>= 1; // Shift the bitmask

			// Get the "row". This moves the pointer on...
			uint32_t row = getRow(source);

			// Look for it in the art data
			std::vector<uint32_t>::iterator it = std::find(artData.begin(), artData.end(), row);

			if (it == artData.end())
			{
				// New art data
				artData.push_back(row);
			}
			else
			{
				// Repeated art data
				bitmask |= 0x80;
				duplicateRows.push_back((uint8_t)std::distance(artData.begin(), it));
			}
		}

		// Push the bitmask into the buffer
		buf.push_back(bitmask);
	}

	// Then we copy in the "duplicate rows" data...
	buf.insert(buf.end(), duplicateRows.begin(), duplicateRows.end());

	// Now we know the art data offset...
	int artDataOffset = buf.size();
	buf[4] = (artDataOffset >> 0) & 0xff;
	buf[5] = (artDataOffset >> 8) & 0xff;

	// Copy over the art data
	// Not sure if there's an STL-y way to do this
	for (std::vector<uint32_t>::const_iterator it = artData.begin(); it != artData.end(); ++it)
	{
		uint32_t row = *it;
		addRow(buf, row);
	}

	// check length
	int resultlen = (int)buf.size();
	if (resultlen > destLen)
	{
		return 0;
	}
	// copy to dest
	memcpy(dest, &buf[0], resultlen);
	// return length
	return resultlen;
}
