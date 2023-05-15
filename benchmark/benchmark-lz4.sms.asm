; { "technology": "LZ4", "extension": "lz4" }

.memorymap
defaultslot 0
slotsize $8000
slot 0 $0000
.endme

.rombankmap
bankstotal 1
banksize $8000
banks 1
.endro

.bank 0 slot 0

.org 0
	ld hl,data
	ld de,$4000
	ld ix,dataend
	call lz4_decompress
	ret ; ends the test

.block "decompressor"
.include "../decompressors/LZ4 decompressor.asm"
.endb

data: .incbin "data.lz4"
dataend: