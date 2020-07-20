; { "technology": "LZ4", "extension": "lz4" }

.memorymap
defaultslot 0
slotsize $4000
slot 0 $0000
.endme

.rombankmap
bankstotal 1
banksize $4000
banks 1
.endro

.bank 0 slot 0

.org 0
	ld hl,data
	ld de,$4000
	ld ix,dataend
	call lz4_decompress
	ret ; ends the test

.include "../decompressors/LZ4 decompressor.asm"

data: .incbin "data.lz4"
dataend: