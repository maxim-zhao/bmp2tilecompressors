; { "technology": "LZSA2", "extension": "lzsa2" }

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
	call DecompressLZSA2
	ret ; ends the test
	
.define LZSAToVRAM
.block "decompressor"
.include "../decompressors/unlzsa2_fast.asm"
.endb

.org $1000
data: .incbin "data.lzsa2"
