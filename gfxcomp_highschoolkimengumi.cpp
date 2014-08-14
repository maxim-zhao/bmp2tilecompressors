#include <windows.h>
#include <vector>

using namespace std;

typedef vector<unsigned char> buffer;

#define MAX_RUN_SIZE 0x7f
#define RLE_MASK 0x00
#define RAW_MASK 0x80

// Forward declares
int compress(char* source, unsigned int sourceLen, char* dest, unsigned int destLen, unsigned int interleaving);
//int decompress(char* source, int sourceLen, char* dest, int destLen, int interleaving);
void deinterleave(buffer& buf, int interleaving);
//void interleave(buffer* buf, int interleaving);
//int decompress_plane(buffer* buf, char** src);
void compress_plane(buffer& dest, buffer::const_iterator src, buffer::const_iterator srcEnd);

extern "C" __declspec(dllexport) char* getName()
{
	// A pretty name for this compression type
	// Generally, the name of the game it was REd from
	return "High School Kimengumi RLE";
}

extern "C" __declspec(dllexport) char* getExt()
{
	// A string suitable for use as a file extension
	return "hskcompr";
}

extern "C" __declspec(dllexport) int compressTiles(char* source, int numTiles, char* dest, int destLen)
{
	// Compress tiles
	return compress(source, numTiles * 32, dest, destLen, 4);
}

extern "C" __declspec(dllexport) int compressTilemap(char* source, int width, int height, char* dest, int destLen)
{
	// Compress tilemap
	return compress(source, width * height * 2, dest, destLen, 2);
}

void deinterleave(buffer& buf, int interleaving)
{
	buffer tempbuf(buf);

	// Deinterleave into tempbuf
	int bitplanesize = buf.size() / interleaving;
	for (buffer::size_type src = 0; src < buf.size(); ++src)
	{
		// If interleaving is 4 I want to turn
		// AbcdEfghIjklMnopQrstUvwx
		// into
		// AEIMQUbfjnrvcgkoswdhlptx
		// so for a char at position x
		// x div 4 = offset within this section
		// x mod 4 = which section
		// final position = (x div 4) + (x mod 4) * (section size)
		int dest = src / interleaving + (src % interleaving) * bitplanesize;
		tempbuf[dest] = buf[src];
	}

	// Copy results over the original data
	std::copy(tempbuf.begin(), tempbuf.end(), buf.begin());
}

void write_raw(buffer& dest, buffer::const_iterator begin, buffer::const_iterator end)
{
	while (begin < end)
	{
		int blocklen = end - begin;
		if (blocklen > MAX_RUN_SIZE)
		{
			blocklen = MAX_RUN_SIZE;
		}
		dest.push_back((unsigned char)(RAW_MASK | blocklen));
		for (int i = 0; i < blocklen; ++i)
		{
			dest.push_back(*begin++);
		}
	}
}

void write_run(buffer& dest, char val, int len)
{
	if (len == 0)
	{
		return;
	}

	while (len > 0)
	{
		int blocklen = len;
		if (blocklen > MAX_RUN_SIZE) 
		{
			blocklen = MAX_RUN_SIZE;
		}
		dest.push_back((unsigned char)(RLE_MASK | blocklen));
		dest.push_back(val);
		len -= blocklen;
	}
}

int getrunlength(buffer::const_iterator src, buffer::const_iterator end)
{
	// find the number of consecutive identical values
	unsigned char c = *src;
	buffer::const_iterator it = src;
	for (++it; it != end && *it == c; ++it);
	return it - src;
}

int compress(char* source, unsigned int sourceLen, char* dest, unsigned int destLen, unsigned int interleaving)
{
	// Compress sourceLen bytes from source to dest;
	// return length, or 0 if destLen is too small, or -1 if there is an error

	// Copy the data into a buffer
	buffer bufSource(source, source + sourceLen);

	// Deinterleave it
	deinterleave(bufSource, interleaving);

	// Make a buffer to hold the result
	buffer bufDest;
	bufDest.reserve(destLen);

	// Compress everything in one go
	buffer::const_iterator rawStart = bufSource.begin();
	for (buffer::const_iterator it = bufSource.begin(); it != bufSource.end(); /* increment in loop */)
	{
		int runlength = getrunlength(it, bufSource.end());
		int runlengthneeded = 3; // normally need a run of 3 to be worth breaking a raw block
		if (it == bufSource.begin() || it + runlength == bufSource.end()) --runlengthneeded; // but at the beginning or end, 2 is enough
		if (runlength < runlengthneeded)
		{
			// Not good enough; keep looking for a run
			it += runlength;
			continue;
		}

		// We found a good enough run. Write the raw (if any) and then the run
		write_raw(bufDest, rawStart, it);
		write_run(bufDest, *it, runlength);
		it += runlength;
		rawStart = it;
	}
	// We may have a final run of raw bytes
	write_raw(bufDest, rawStart, bufSource.end());
	// Zero terminator
	bufDest.push_back(0);

	// check length
	if (bufDest.size() > destLen)
	{
		return 0;
	}

	// copy to dest
	std::copy(bufDest.begin(), bufDest.end(), dest);

	// return length
	return bufDest.size();
}
