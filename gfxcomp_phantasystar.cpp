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
	return "Phantasy Star RLE";
}

extern "C" __declspec(dllexport) char* getExt()
{
	// A string suitable for use as a file extension
	return "pscompr";
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
/*
extern "C" __declspec(dllexport) int decompressTiles(char* source, int sourceLen, char* dest, int destLen)
{
	return decompress(source, sourceLen, dest, destLen, 4);
}

extern "C" __declspec(dllexport) int decompressTilemap(char* source, int sourceLen, char* dest, int destLen)
{
	return decompress(source, sourceLen, dest, destLen, 4);
}
*/
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

	// Compress each plane in turn
	int bitplanesize = sourceLen / interleaving;
	for (buffer::const_iterator it = bufSource.begin(); it != bufSource.end(); it += bitplanesize)
	{
		compress_plane(bufDest, it, it + bitplanesize);
	}

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
/*
int decompress(char* source, int sourceLen, char* dest, int destLen, int interleaving)
{
	// Decompress into a buffer
	buffer buf;

	// decompress it one bitplane at a time
	int lastBitplaneLength = 0;
	for (int bitplane = 0; bitplane < interleaving; ++bitplane)
	{
		int bitplaneLength = decompress_plane(&buf, &source);
		if (bitplaneLength % 32 != 0)
		{
			// tile data should always be a multiple of 32 bytes
			return -1;
		}
		if (lastBitplaneLength != 0 && lastBitplaneLength != bitplaneLength)
		{
			// bitplanes must be equal length
			return -1;
		}
		lastBitplaneLength = bitplaneLength;
	}

	// check destLen
	int length = buf.length;
	if (length > destLen)
	{
		return 0;
	}

	// interleave the data
	interleave(&buf, interleaving);

	// copy to dest
	memcpy(dest, buf.buf, length);
	
	return length;
}

int decompress_plane(buffer* dest, char** src)
{
	int destLenBefore = dest->length;
	for (;;)
	{
		// look at byte and increment
		char b = *(*src)++;
		if ((b & RAW_MASK) == RAW_MASK)
		{
			// raw bytes
			// copy to dest
			buffer_write(dest, *src, b);
			// increment src pointer
			(*src) += b;
		}
		else if (b == 0)
		{
			// end of bitplane
			break;
		}
		else
		{
			// RLE
			// Expand run
			for (int i = 0; i < b; ++i)
			{
				buffer_write(dest, **src);
			}
			// Increment source pointer
			++(*src);
		}
	}

	// return number of bytes produced
	return dest->length - destLenBefore;
}
*/
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
/*
void interleave(buffer* buf, int interleaving)
{
	char* tempbuf = (char*)malloc(buf->length);

	// Interleave into tempbuf
	int bitplanesize = buf->length / interleaving;
	for (int src = 0; src < buf->length; ++src)
	{
		// If interleaving is 4 I want to turn
		// ABCDEFghijklmnopqrstuvwx
		// into
		// AgmsBhntCiouDjpvEkqwFlr
		// so for a char at position x,
		// x div bitplanesize = offset within this section
		// x mod bitplanesize = which section
		// final position = (x div bitplanesize) + (x mod bitplanesize) * (section size)
		int dest = src / bitplanesize + (src % bitplanesize) * interleaving;
		tempbuf[dest] = buf->buf[src];
	}

	// Copy results over the original data
	memcpy(buf->buf, tempbuf, buf->length);

	free(tempbuf);
}
*/
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

void compress_plane(buffer& dest, buffer::const_iterator src, buffer::const_iterator srcEnd)
{
	buffer::const_iterator rawStart = src;
	for (buffer::const_iterator it = src; it != srcEnd; /* increment in loop */)
	{
		int runlength = getrunlength(it, srcEnd);
		int runlengthneeded = 3; // normally need a run of 3 to be worth breaking a raw block
		if (it == src || it + runlength == srcEnd) --runlengthneeded; // but at the beginning or end, 2 is enough
		if (runlength < runlengthneeded)
		{
			// Not good enough; keep looking for a run
			it += runlength;
			continue;
		}

		// We found a good enough run. Write the raw (if any) and then the run
		write_raw(dest, rawStart, it);
		write_run(dest, *it, runlength);
		it += runlength;
		rawStart = it;
	}
	// We may have a final run of raw bytes
	write_raw(dest, rawStart, srcEnd);
	// Zero terminator
	dest.push_back(0);
}
