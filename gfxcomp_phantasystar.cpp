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
		memcpy_s(dest, destLen, &bufDest[0], bufDest.size());

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

	class Block
	{
	public:
		enum Type
		{
			Raw,
			Run
		};
		Type _type;
		std::vector<uint8_t> _data;

		// Raw block constructor
		Block(std::vector<uint8_t>::const_iterator begin, std::vector<uint8_t>::const_iterator end)
		: _type(Raw)
		{
			// Copy to our vector
			_data.insert(_data.end(), begin, end);
		}

		// RLE block constructor
		Block(uint32_t runLength, uint8_t value)
		: _type(Run)
		{
			// Fill up our vector
			_data.resize(runLength, value);
		}
	};

	void compressPlane(std::vector<uint8_t>& destination, std::vector<uint8_t>::const_iterator source, std::vector<uint8_t>::const_iterator sourceEnd)
	{
		// First we decompose into blocks
		std::vector<Block> blocks;
		auto rawStart = source;
		for (auto it = source; it != sourceEnd; /* increment in loop */)
		{
			uint32_t runlength = getRunLength(it, sourceEnd);
			if (runlength < 2)
			{
				// Not good enough; keep looking for a run
				it += runlength;
				continue;
			}

			// We found a good enough run. Write the raw (if any) and then the run
			if (rawStart != it)
			{
				blocks.push_back(Block(rawStart, it));
			}
			blocks.push_back(Block(runlength, *it));

			it += runlength;
			rawStart = it;
		}

		// We may have a final run of raw bytes
		if (rawStart != sourceEnd)
		{
			blocks.push_back(Block(rawStart, sourceEnd));
		}

		// Go through and optimise any instances of:
		// * raw + run[2] (n+1, 2 bytes) -> n+2+1 (same but more mergeable)
		// * run[2] + raw (2, n+1 bytes) -> 2+n+1 (same but more mergeable)
		// * raw + raw (n+1, m+1 bytes) -> n+m+1 (saves 1, merges results from above)
		// into a single raw block
		if (blocks.size() > 2)
		{
			std::vector<Block>::iterator previous = blocks.begin();
			for (auto current = previous + 1; current != blocks.end(); /* increment in loop */)
			{
				if ((previous->_type == Block::Raw && current->_type == Block::Run && current->_data.size() == 2) ||
					(previous->_type == Block::Raw && current->_type == Block::Raw) ||
					(previous->_type == Block::Run && previous->_data.size() == 2 && current->_type == Block::Raw))
				{
					// Combine the data
					previous->_data.insert(previous->_data.end(), current->_data.begin(), current->_data.end());
					previous->_type = Block::Raw;
					// Knock out the dead block (slow for a vector, probably not a big deal)
					blocks.erase(current);
					// Fix up the iterator
					current = previous + 1;
					// We go around again pointing at the new current
				}
				else
				{
					// Move on
					previous = current;
					++current;
				}
			}
		}

		// Now emit them
		for (auto it = blocks.begin(); it != blocks.end(); ++it)
		{
			switch (it->_type)
			{
			case Block::Raw:
				writeRaw(destination, it->_data.begin(), it->_data.end());
				break;
			case Block::Run:
				writeRun(destination, it->_data[0], it->_data.size());
				break;
			default:
				break;
			}
		}

		// Zero terminator
		destination.push_back(0);
	}
}
