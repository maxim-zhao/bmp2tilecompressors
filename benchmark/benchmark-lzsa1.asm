; { "technology": "LZSA1", "extension": "lzsa1" }

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
	call DecompressLZSA1
	ret ; ends the test
	
.define LZSAToVRAM
.include "../decompressors/unlzsa1_fast.asm"

data: .incbin "data.lzsa1"
