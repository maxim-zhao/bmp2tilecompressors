#include <vector>
#include <cstdint>

using namespace std;

typedef vector<uint8_t> buffer;

#define MAX_RUN_SIZE 0x7f
#define RLE_MASK 0x00
#define RAW_MASK 0x80

// Forward declares
uint32_t compress(uint8_t* source, unsigned int sourceLen, uint8_t* dest, unsigned int destLen, unsigned int interleaving);
//int decompress(uint8_t* source, int sourceLen, uint8_t* dest, int destLen, int interleaving);
void deinterleave(buffer& buf, uint32_t interleaving);
//void interleave(buffer* buf, uint32_t interleaving);
//int decompress_plane(buffer* buf, uint8_t** src);
void compress_plane(buffer& dest, buffer::const_iterator src, buffer::const_iterator srcEnd);

extern "C" __declspec(dllexport) const char* getName()
{
	// A pretty name for this compression type
	// Generally, the name of the game it was REd from
	return "Phantasy Star RLE";
}

extern "C" __declspec(dllexport) const char* getExt()
{
	// A string suitable for use as a file extension
	return "pscompr";
}

extern "C" __declspec(dllexport) int compressTiles(uint8_t* source, uint32_t numTiles, uint8_t* dest, uint32_t destLen)
{
	// Compress tiles
	return (int)compress(source, numTiles * 32, dest, destLen, 4);
}

extern "C" __declspec(dllexport) int compressTilemap(uint8_t* source, uint32_t width, uint32_t height, uint8_t* dest, uint32_t destLen)
{
	// Compress tilemap
	return (int)compress(source, width * height * 2, dest, destLen, 2);
}
/*
extern "C" __declspec(dllexport) int decompressTiles(uint8_t* source, uint32_t sourceLen, uint8_t* dest, uint32_t destLen)
{
	return decompress(source, sourceLen, dest, destLen, 4);
}

extern "C" __declspec(dllexport) int decompressTilemap(uint8_t* source, uint32_t sourceLen, uint8_t* dest, uint32_t destLen)
{
	return decompress(source, sourceLen, dest, destLen, 4);
}
*/

uint32_t compress(uint8_t* source, uint32_t sourceLen, uint8_t* dest, uint32_t destLen, uint32_t interleaving)
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
	int32_t bitplanesize = (int32_t)(sourceLen / interleaving);
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
int decompress(uint8_t* source, uint32_t sourceLen, uint8_t* dest, uint32_t destLen, uint32_t interleaving)
{
	// Decompress into a buffer
	buffer buf;

	// decompress it one bitplane at a time
	uint32_t lastBitplaneLength = 0;
	for (uint32_t bitplane = 0; bitplane < interleaving; ++bitplane)
	{
		uint32_t bitplaneLength = decompress_plane(&buf, &source);
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
	uint32_t length = buf.length;
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

int decompress_plane(buffer* dest, uint8_t** src)
{
	int destLenBefore = dest->length;
	for (;;)
	{
		// look at byte and increment
		uint8_t b = *(*src)++;
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

void deinterleave(buffer& buf, uint32_t interleaving)
{
	buffer tempbuf(buf);

	// Deinterleave into tempbuf
	uint32_t bitplanesize = buf.size() / interleaving;
	for (buffer::size_type src = 0; src < buf.size(); ++src)
	{
		// If interleaving is 4 I want to turn
		// AbcdEfghIjklMnopQrstUvwx
		// into
		// AEIMQUbfjnrvcgkoswdhlptx
		// so for a byte at position x
		// x div 4 = offset within this section
		// x mod 4 = which section
		// final position = (x div 4) + (x mod 4) * (section size)
		uint32_t dest = src / interleaving + (src % interleaving) * bitplanesize;
		tempbuf[dest] = buf[src];
	}

	// Copy results over the original data
	std::copy(tempbuf.begin(), tempbuf.end(), buf.begin());
}
/*
void interleave(buffer* buf, int interleaving)
{
	uint8_t* tempbuf = (uint8_t*)malloc(buf->length);

	// Interleave into tempbuf
	int bitplanesize = buf->length / interleaving;
	for (int src = 0; src < buf->length; ++src)
	{
		// If interleaving is 4 I want to turn
		// ABCDEFghijklmnopqrstuvwx
		// into
		// AgmsBhntCiouDjpvEkqwFlr
		// so for a byte at position x,
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
		dest.push_back((uint8_t)(RAW_MASK | blocklen));
		for (int i = 0; i < blocklen; ++i)
		{
			dest.push_back(*begin++);
		}
	}
}

void write_run(buffer& dest, uint8_t val, uint32_t len)
{
	if (len == 0)
	{
		return;
	}

	while (len > 0)
	{
		uint32_t blocklen = len;
		if (blocklen > MAX_RUN_SIZE) 
		{
			blocklen = MAX_RUN_SIZE;
		}
		dest.push_back((uint8_t)(RLE_MASK | blocklen));
		dest.push_back(val);
		len -= blocklen;
	}
}

uint32_t getrunlength(buffer::const_iterator src, buffer::const_iterator end)
{
	// find the number of consecutive identical values
	uint8_t c = *src;
	buffer::const_iterator it = src;
	for (++it; it != end && *it == c; ++it);
	return it - src;
}

void compress_plane(buffer& dest, buffer::const_iterator src, buffer::const_iterator srcEnd)
{
	buffer::const_iterator rawStart = src;
	for (buffer::const_iterator it = src; it != srcEnd; /* increment in loop */)
	{
		uint32_t runlength = getrunlength(it, srcEnd);
		uint32_t runlengthneeded = 3; // normally need a run of 3 to be worth breaking a raw block
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
