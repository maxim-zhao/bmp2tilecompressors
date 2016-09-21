#include <vector>
#include <cstdint>

namespace gfxcomp_phantasystar
{
	const uint8_t MAX_RUN_SIZE = 0x7f;
	const uint8_t RLE_MASK = 0x00;
	const uint8_t RAW_MASK = 0x80;

	// Forward declares
	uint32_t compress(uint8_t* source, uint32_t sourceLen, uint8_t* dest, uint32_t destLen, uint32_t interleaving);
	void deinterleave(std::vector<uint8_t>& buf, uint32_t interleaving);
	void compressPlane(std::vector<uint8_t>& dest, std::vector<uint8_t>::const_iterator src, std::vector<uint8_t>::const_iterator srcEnd);

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
		return static_cast<int>(compress(source, numTiles * 32, dest, destLen, 4));
	}

	extern "C" __declspec(dllexport) int compressTilemap(uint8_t* source, uint32_t width, uint32_t height, uint8_t* dest, uint32_t destLen)
	{
		// Compress tilemap
		return static_cast<int>(compress(source, width * height * 2, dest, destLen, 2));
	}

	uint32_t compress(uint8_t* source, uint32_t sourceLen, uint8_t* dest, uint32_t destLen, uint32_t interleaving)
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

		// Compress each plane in turn
		int32_t bitplanesize = static_cast<int32_t>(sourceLen / interleaving);
		for (std::vector<uint8_t>::const_iterator it = bufSource.begin(); it != bufSource.end(); it += bitplanesize)
		{
			compressPlane(bufDest, it, it + bitplanesize);
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

	void deinterleave(std::vector<uint8_t>& buf, uint32_t interleaving)
	{
		std::vector<uint8_t> tempbuf(buf);

		// Deinterleave into tempbuf
		uint32_t bitplanesize = buf.size() / interleaving;
		for (std::vector<uint8_t>::size_type src = 0; src < buf.size(); ++src)
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

	void writeRaw(std::vector<uint8_t>& dest, std::vector<uint8_t>::const_iterator begin, std::vector<uint8_t>::const_iterator end)
	{
		while (begin < end)
		{
			auto blockLength = end - begin;
			if (blockLength > MAX_RUN_SIZE)
			{
				blockLength = MAX_RUN_SIZE;
			}
			dest.push_back(static_cast<uint8_t>(RAW_MASK | blockLength));
			for (auto i = 0; i < blockLength; ++i)
			{
				dest.push_back(*begin++);
			}
		}
	}

	void writeRun(std::vector<uint8_t>& destination, uint8_t value, uint32_t count)
	{
		if (count == 0)
		{
			return;
		}

		while (count > 0)
		{
			uint8_t blockLength = count > MAX_RUN_SIZE ? MAX_RUN_SIZE : static_cast<uint8_t>(count);
			destination.push_back(RLE_MASK | blockLength);
			destination.push_back(value);
			count -= blockLength;
		}
	}

	uint32_t getRunLength(std::vector<uint8_t>::const_iterator begin, std::vector<uint8_t>::const_iterator end)
	{
		// find the number of consecutive identical values
		uint8_t c = *begin;
		std::vector<uint8_t>::const_iterator it = begin;
		for (++it; it != end && *it == c; ++it) {};
		return it - begin;
	}

	void compressPlane(std::vector<uint8_t>& destination, std::vector<uint8_t>::const_iterator source, std::vector<uint8_t>::const_iterator sourceEnd)
	{
		std::vector<uint8_t>::const_iterator rawStart = source;
		for (std::vector<uint8_t>::const_iterator it = source; it != sourceEnd; /* increment in loop */)
		{
			uint32_t runlength = getRunLength(it, sourceEnd);
			uint32_t runlengthneeded = 3; // normally need a run of 3 to be worth breaking a raw block
			if (it == source || it + runlength == sourceEnd) --runlengthneeded; // but at the beginning or end, 2 is enough
			if (runlength < runlengthneeded)
			{
				// Not good enough; keep looking for a run
				it += runlength;
				continue;
			}

			// We found a good enough run. Write the raw (if any) and then the run
			writeRaw(destination, rawStart, it);
			writeRun(destination, *it, runlength);
			it += runlength;
			rawStart = it;
		}
		// We may have a final run of raw bytes
		writeRaw(destination, rawStart, sourceEnd);
		// Zero terminator
		destination.push_back(0);
	}
}
