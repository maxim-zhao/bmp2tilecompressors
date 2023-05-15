; { "technology": "LZSA1", "extension": "lzsa1" }

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
	call DecompressLZSA1
	ret ; ends the test
	
.define LZSAToVRAM
.block "decompressor"
.include "../decompressors/unlzsa1_fast.asm"
.endb

data: .incbin "data.lzsa1"
