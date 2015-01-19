#include <windows.h>
#include <vector>
#include <map>
using namespace::std;

typedef vector<unsigned char> buffer;

// Forward declares
int compress(char* source, int sourceLen, char* dest, int destLen, int interleaving);
void deinterleave(buffer& buf, int interleaving);
void compress_tile(const buffer& src, buffer& dest);

// avoid CRT. Evil. Big. Bloated.
BOOL WINAPI _DllMainCRTStartup(HANDLE, ULONG, LPVOID)
{
	return TRUE;
}

extern "C" __declspec(dllexport) char* getName()
{
	// A pretty name for this compression type
	// Generally, the name of the game it was REd from
	return "PS Gaiden";
}

extern "C" __declspec(dllexport) char* getExt()
{
	// A string suitable for use as a file extension
	return "psgcompr";
}

extern "C" __declspec(dllexport) int compressTiles(char* source, int numTiles, char* dest, int destLen)
{
	if (numTiles > 0xffff)
	{
		return -1; // error
	}
	buffer buf; // the output
	buf.reserve(destLen); // avoid reallocation

	// Write number of tiles
	buf.push_back((numTiles >> 0) & 0xff);
	buf.push_back((numTiles >> 8) & 0xff);

	buffer tile;
	tile.resize(32); // zero fill
	for (int i = 0; i < numTiles; ++i)
	{
		// Get tile into buffer
		memcpy(&tile[0], source, 32);
		source += 32;
		// Deinterleave it
		deinterleave(tile, 4);
		// Compress it to dest
		compress_tile(tile, buf);
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

void deinterleave(buffer& buf, int interleaving)
{
	buffer tempbuf(buf);

	// Deinterleave into tempbuf
	int bitplanesize = buf.size() / interleaving;
	for (unsigned int src = 0; src < buf.size(); ++src)
	{
		// If interleaving is 4 I want to turn
		// AbcdEfghIjklMnopQrstUvwx
		// into
		// AEIMQUbfjnrvcgkoswdhlptx
		// so for a char at position x
		// x div 4 = offset within this section
		// x mod 4 = which section
		// final position = (x div 4) + (x mod 4) * (section size)
		unsigned int dest = src / interleaving + (src % interleaving) * bitplanesize;
		tempbuf[dest] = buf[src];
	}

	// Copy results over the original data
	std::copy(tempbuf.begin(), tempbuf.end(), buf.begin());
}

void findMostCommonValue(buffer::const_iterator data, int& value, int& count)
{
	map<int,int> counts;
	// count occurences of each value
	for (int i = 0; i < 8; ++i)
	{
		int val= *data++;
		counts[val]++;
	}
	// find the highest count and its value
	count = 0;
	for (map<int,int>::const_iterator it = counts.begin(); it != counts.end(); ++it)
	{
		if (it->second > count)
		{
			value = it->first;
			count = it->second;
		}
	}
}

int countMatches(buffer::const_iterator me, buffer::const_iterator other, bool invert)
{
	int count = 0;
	int mask = invert ? 0xff : 0x00;
	for (int i = 0; i < 8; ++i)
	{
		if (*me++ == (*other++ ^ mask))
		{
			++count;
		}
	}
	return count;
}

void compress_tile(const buffer& src, buffer& dest)
{
	int bitplaneMethods = 0;

	// compress the tile data into a temporary buffer
	// because we need to build the method byte before outputting to dest
	buffer tiledata;

	// for each bitplane
	for(int bitplane = 0; bitplane < 4; ++bitplane)
	{
		// Find the most common value in this bitplane
		int mostCommonByte;
		int mostCommonByteCount;
		buffer::const_iterator itBitplane = src.begin() + bitplane * 8;
		findMostCommonValue(itBitplane, mostCommonByte, mostCommonByteCount);

		// Find how much it matches previous bitplanes
		int otherBitplaneMatch = 0; // which bitplane
		int otherBitplaneMatchCount = 0; // how many matched
		bool otherBitplaneMatchInverse = false; // whether it was an inverted match
		for (int otherbitplane = 0; otherbitplane < bitplane; ++otherbitplane)
		{
			// Compare
			buffer::const_iterator itOtherBitplane = src.begin() + otherbitplane * 8;
			int count = countMatches(itBitplane, itOtherBitplane, false);
			if (count > otherBitplaneMatchCount)
			{
				otherBitplaneMatch = otherbitplane;
				otherBitplaneMatchCount = count;
				otherBitplaneMatchInverse = false;
			}
			count = countMatches(itBitplane, itOtherBitplane, true);
			if (count > otherBitplaneMatchCount)
			{
				otherBitplaneMatch = otherbitplane;
				otherBitplaneMatchCount = count;
				otherBitplaneMatchInverse = true;
			}
		}

		// Choose method and shift into bitplaneMethods
		bitplaneMethods <<= 2;

		if (mostCommonByteCount == 8 && mostCommonByte == 0x00)
		{
			// all 0 = %00
			bitplaneMethods |= 0x00;
			// no data
		}
		else if (mostCommonByteCount == 8 && mostCommonByte == 0xff) 
		{
			// all ff = %01
			bitplaneMethods |= 0x01;
			// no data
		}
		else if (mostCommonByteCount <= 2 && otherBitplaneMatchCount <= 2 ) 
		{
			// raw = %11
			bitplaneMethods |= 0x03;
			// output raw data
			tiledata.insert(tiledata.end(), itBitplane, itBitplane + 8);
		}
		else
		{
			// compressed = %10
			bitplaneMethods |= 0x02;

			// select compression type
			if (otherBitplaneMatchCount == 8)
			{
				/// %000f00nn = whole bitplane duplicate
				if (otherBitplaneMatchInverse) 
				{
					tiledata.push_back((unsigned char)(0x10 | otherBitplaneMatch));
				}
				else
				{
					tiledata.push_back((unsigned char)(0x00 | otherBitplaneMatch));
				}
			}
			else if (otherBitplaneMatchCount > mostCommonByteCount)
			{
				/// %001000nn = copy bytes
				/// %010000nn = copy and invert bytes
				if (otherBitplaneMatchInverse) 
				{
					tiledata.push_back((unsigned char)(0x40 | otherBitplaneMatch));
				}
				else
				{
					tiledata.push_back((unsigned char)(0x20 | otherBitplaneMatch));
				}

				// Build bitmask
				int bitmask = 0;
				int mask = otherBitplaneMatchInverse ? 0xff : 0x00;
				buffer::const_iterator it1 = itBitplane;
				buffer::const_iterator it2 = src.begin() + otherBitplaneMatch * 8;
				for (int i = 0; i < 8; ++i, ++it1, ++it2)
				{
					bitmask <<= 1;
					if (*it1 == (*it2 ^ mask))
					{
						bitmask |= 1;
					}
				}

				// output byte
				tiledata.push_back((unsigned char)(bitmask));

				// output non-matching bytes
				it1 = itBitplane;
				it2 = src.begin() + otherBitplaneMatch * 8;
				for (int i = 0; i < 8; ++i, ++it1, ++it2)
				{
					bitmask <<= 1;
					if (*it1 != (*it2 ^ mask))
					{
						tiledata.push_back(*it1);
					}
				}
			}
			else 
			{
				// common byte
				// build bitmask
				int bitmask = 0;
				buffer::const_iterator it = itBitplane;
				for (int i = 0; i < 8; ++i, ++it)
				{
					bitmask <<= 1;
					if (*it == mostCommonByte)
					{
						bitmask |= 1;
					}
				}
				tiledata.push_back((unsigned char)(bitmask));
				tiledata.push_back((unsigned char)(mostCommonByte));

				// output non-matching bytes
				it = itBitplane;
				for (int i = 0; i < 8; ++i, ++it)
				{
					bitmask <<= 1;
					if (*it != mostCommonByte)
					{
						tiledata.push_back(*it);
					}
				}
			} // compression method selection
		} // method selection
	} // bitplane loop

	// output
	dest.push_back((unsigned char)(bitplaneMethods));
	dest.insert(dest.end(), tiledata.begin(), tiledata.end());
}
