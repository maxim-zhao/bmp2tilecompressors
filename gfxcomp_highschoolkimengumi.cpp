#include <vector>
#include <cstdint>

#define MAX_RUN_SIZE 0x7f
#define RLE_MASK 0x00
#define RAW_MASK 0x80

// Forward declares
int compress(uint8_t* source, uint32_t sourceLen, uint8_t* dest, uint32_t destLen, unsigned int interleaving);
void deinterleave(std::vector<uint8_t>& buf, int interleaving);

extern "C" __declspec(dllexport) const char* getName()
{
	// A pretty name for this compression type
	// Generally, the name of the game it was REd from
	// ReSharper disable once StringLiteralTypo
	return "High School Kimengumi RLE";
}

extern "C" __declspec(dllexport) const char* getExt()
{
	// A string suitable for use as a file extension
	// ReSharper disable once StringLiteralTypo
	return "hskcompr";
}

extern "C" __declspec(dllexport) int compressTiles(uint8_t* source, uint32_t numTiles, uint8_t* dest, uint32_t destLen)
{
	// Compress tiles
	return compress(source, numTiles * 32, dest, destLen, 4);
}

extern "C" __declspec(dllexport) int compressTilemap(uint8_t* source, uint32_t width, uint32_t height, uint8_t* dest, uint32_t destLen)
{
	// Compress tilemap
	return compress(source, width * height * 2, dest, destLen, 2);
}

void deinterleave(std::vector<uint8_t>& buf, const int interleaving)
{
	std::vector<uint8_t> tempBuffer(buf);

	// Deinterleave into tempBuffer
	const size_t bitplaneSize = buf.size() / interleaving;
	for (size_t src = 0; src < buf.size(); ++src)
	{
		// ReSharper disable CommentTypo
		// If interleaving is 4 I want to turn
		// AbcdEfghIjklMnopQrstUvwx
		// into
		// AEIMQUbfjnrvcgkoswdhlptx
		// so for a byte at position x
		// x div 4 = offset within this section
		// x mod 4 = which section
		// final position = (x div 4) + (x mod 4) * (section size)
		// ReSharper restore CommentTypo
		const size_t dest = src / interleaving + (src % interleaving) * bitplaneSize;
		tempBuffer[dest] = buf[src];
	}

	// Copy results over the original data
	std::copy(tempBuffer.begin(), tempBuffer.end(), buf.begin());
}

void writeRaw(std::vector<uint8_t>& dest, std::vector<uint8_t>::const_iterator begin, std::vector<uint8_t>::const_iterator end)
{
	while (begin < end)
	{
		size_t blocklen = end - begin;
		if (blocklen > MAX_RUN_SIZE)
		{
			blocklen = MAX_RUN_SIZE;
		}
		dest.push_back(static_cast<uint8_t>(RAW_MASK | blocklen));
		for (size_t i = 0; i < blocklen; ++i)
		{
			dest.push_back(*begin++);
		}
	}
}

void writeRun(std::vector<uint8_t>& dest, uint8_t val, uint32_t len)
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
		dest.push_back(static_cast<uint8_t>(RLE_MASK | blocklen));
		dest.push_back(val);
		len -= blocklen;
	}
}

int getRunLength(std::vector<uint8_t>::const_iterator src, std::vector<uint8_t>::const_iterator end)
{
	// find the number of consecutive identical values
	const uint8_t c = *src;
	auto it = src;
	for (++it; it != end && *it == c; ++it) {};
	return it - src;
}

int compress(uint8_t* source, uint32_t sourceLen, uint8_t* dest, uint32_t destLen, uint32_t interleaving)
{
	// Compress sourceLen bytes from source to dest;
	// return length, or 0 if destLen is too small, or -1 if there is an error

	// Copy the data into a buffer
	std::vector<uint8_t> bufSource(source, source + sourceLen);

	// Deinterleave it
	deinterleave(bufSource, interleaving);

	// Make a buffer to hold the result
	std::vector<uint8_t> bufDest;
	bufDest.reserve(destLen);

	// Compress everything in one go
	auto rawStart = bufSource.cbegin();
	for (auto it = bufSource.cbegin(); it != bufSource.end(); /* increment in loop */)
	{
		const int runLength = getRunLength(it, bufSource.end());
		int runLengthNeeded = 3; // normally need a run of 3 to be worth breaking a raw block
		if (it == bufSource.begin() || it + runLength == bufSource.end()) --runLengthNeeded; // but at the beginning or end, 2 is enough
		if (runLength < runLengthNeeded)
		{
			// Not good enough; keep looking for a run
			it += runLength;
			continue;
		}

		// We found a good enough run. Write the raw (if any) and then the run
		writeRaw(bufDest, rawStart, it);
		writeRun(bufDest, *it, runLength);
		it += runLength;
		rawStart = it;
	}
	// We may have a final run of raw bytes
	writeRaw(bufDest, rawStart, bufSource.end());
	// Zero terminator
	bufDest.push_back(0);

	// check length
	if (bufDest.size() > destLen)
	{
		return 0;
	}

	// copy to dest
	memcpy_s(dest, destLen, &bufDest[0], bufDest.size());

	// return length
	return bufDest.size();
}
