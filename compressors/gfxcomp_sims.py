import sys
import traceback

# Compression format:
# $nn Count
# Followed by n of:
# $pppp Pointer to data
# Each points to:
# $xx $yyyy $zzzz
# Where:
# $xx = page number
# $yyyy = VDP address
# $zzzz = data pointer
# Then in the data:
# $nnnn Byte count of following data
# Followed by one of:
# %0nnnnnnn %xxxxyyyy ... LZ
# - Distance = %nnnnnnnxxxx from cursor (range 0..2047)
# - Length = yyyy + 2 (range 2..17)
# Data is decompressed to RAM and then appended to the (circular) lookback buffer.
# %10nnnnnn ... Raw
# - n+1 bytes of raw data follow, range 1..64 inclusive
# %110nnxxx ... RLE
# - Repetitions = %xxx + 2, range 2..9 inclusive
# - Length = %nn + 1, range 1..4 inclusive
# Emit (length) bytes (repetitions) times, (length) bytes follow
# %111nnxxx %yyyyyyyy ... RLE
# - Repetitions = %xxxyyyyyyyy + 2, range 2..2050 inclusive (could be a bit more with more maths)
# - Length = %nn + 1, range 1..4 inclusive
# Emit (length) bytes (repetitions) times, (length) bytes follow
# Note that the decompressor does not allow you to have LZ runs that self-overlap: instead, it has a 2KB history buffer
# which is "static" for the lifetime of the copy. Thus a copy of length 17 from offset 1 means "copy the last byte,
# followed by 16 bytes from 2KB ago". While this might offer some compression opportunities, I won't try to implement it
# here, instead I'll only allow matches within the previous 2KB.

# These match classes have an implicit interface:
# .bytes_encoded = number of bytes they are encoding
# .encode() emits the compressed data

class RleMatch:
    def __init__(self, data, repetitions):
        self.data = data
        self.repetitions = repetitions
        self.bytes_encoded = len(data) * repetitions

    def encode(self):
        if self.repetitions <= 9:
            yield 0b11000000 | ((len(self.data) - 1) << 3) | (self.repetitions - 2)
        else:
            yield 0b11100000 | ((len(self.data) - 1) << 3) | ((self.repetitions - 2) >> 8)
            yield (self.repetitions - 2) & 0xff
        for b in self.data:
            yield b

    def __str__(self):
        return f"RLE: {self.repetitions} repeats of length {len(self.data)}"

class LzMatch:
    def __init__(self, offset, length):
        self.offset = offset
        self.length = length
        self.bytes_encoded = length

    def encode(self):
        yield self.offset >> 4
        yield ((self.offset & 0b1111) << 4) | (self.length - 2)

    def __str__(self):
        return f"LZ: offset {self.offset}, length {self.length}"

class RawMatch:
    def __init__(self, data):
        self.data = data
        self.bytes_encoded = len(data)

    def encode(self):
        yield 0b10000000 | (len(self.data) - 1)
        for b in self.data:
            yield b

    def __str__(self):
        return f"Raw: {len(self.data)} bytes"


def compress():
    try:
        # Read data in
        with open(sys.argv[1], "rb") as f:
            data = f.read()
        # For maybe optimal compression, we work backwards through the file, at each position finding the shortest
        # encoding to get from there to the end of the file. This list holds the best match found at each position.
        best_matches = [0 for x in range(len(data))]
        # This holds the total cost to the end of file at each position.
        costs_to_end = [0 for x in range(len(data))]
        for position in range(len(data) - 1, -1, -1):
            if position % 128 == 0:
              print(f"Position = {position}")
            # Find the best option at this point, that either gets to the end of the file or to a position in the
            # sequences that sums to the lowest byte length
            best_total_cost = len(data) * 2  # Start at a high number, this is high enough
            # First, we look for LZ matches. They tend to dominate the results (when possible, they are very cheap)
            # so doing them first helps avoid some wasted effort.
            # LZ run length is in the range 2..17 but also bounded by the number of bytes to the left and right
            for length in range(2, min(len(data) - position, position, 17) + 1):
                # Grab sequence
                lz_run = data[position : position + length]
                # Distance must be at least length - no overlapping allowed
                match_location = data.rfind(lz_run, max(0, position - 2047), position)
                if match_location != -1:
                    # We have a match
                    # LZ matches are always 2 bytes
                    cost = 2
                    if position + length < len(data):
                        cost += costs_to_end[position + length]
                    if cost < best_total_cost:
                        best_total_cost = cost
                        match = LzMatch(position - match_location, length)
                else:
                  # We try short lengths because they might have a cheaper total cost, but if we failed to find any 
                  # match at length n, length n+1 will fail too
                  break;

            # Next an RLE match...
            for rle_match_length in range(1, 4 + 1):
                # We can't do RLE unless there's at least rle_match_length*2 bytes left in the data
                if len(data) - position < rle_match_length * 2:
                    continue
                # Grab the sequence
                rle_sequence = data[position:position + rle_match_length]
                rle_count = 1
                # Check for matches
                for match_position in range(position + rle_match_length, min(len(data), position + rle_match_length * 2047), rle_match_length):
                    # Does it match?
                    if data.find(rle_sequence, match_position, match_position + rle_match_length) == match_position:
                        # Yes, so what's our cost?
                        rle_count += 1
                        # RLE costs 1 byte + the sequence for counts up to 9, and 2 bytes + the sequence for counts
                        # up to 2050 - but the decompressor only wants up to 2047
                        if rle_count <= 9:
                            cost = 1 + rle_match_length
                        elif rle_count <= 2047:
                            cost = 2 + rle_match_length
                        else:
                            # Over the max length
                            continue
                        if match_position + rle_match_length < len(data):
                            cost += costs_to_end[match_position + rle_match_length]
                        if cost < best_total_cost:
                            best_total_cost = cost
                            match = RleMatch(rle_sequence, rle_count)
                    else:
                        # Not a match, do not continue
                        break

            # Finally raw matches...
            for n in range(1, min(64, len(data) - position) + 1):
                # A raw match will cost n+1 bytes
                cost = n + 1
                if position + n < len(data):
                    cost += costs_to_end[position + n]
                if cost < best_total_cost:
                    best_total_cost = cost
                    match = RawMatch(data[position:position + n])

            # Now we store these in our lists
            best_matches[position] = match
            costs_to_end[position] = best_total_cost

        # Now we have filled our lists and we can walk through the matches to populate our data
        with open(sys.argv[2], "wb") as f:
            # First two bytes are the compressed data length, which is the same as the first cost
            f.write(costs_to_end[0].to_bytes(2, byteorder="little"))
            position = 0
            while position < len(data):
                match = best_matches[position]
                compressed_data = bytes(match.encode())
                f.write(compressed_data)
                position += match.bytes_encoded
                print(match)

    except Exception:
        print(traceback.format_exc())
        sys.exit(-1)  # Failure

compress()
