; { "technology": "Simple Tile Compression 4", "extension": "stc4compr" }

.memorymap
defaultslot 0
slotsize $8000
slot 0 $0000
slot 1 $0000
slot 2 $0000
slotsize $2000
slot 3 $c000
.endme

.rombankmap
bankstotal 1
banksize $8000
banks 1
.endro

.rambankmap

.bank 0 slot 0

.org 0
	ld hl,data
	ld de,$4000
	call stc4_decompress
	ret ; ends the test
  
.org $38
  reti ; It enables interrupts!

.block "decompressor"
.include "../decompressors/stc4_decomp.asm"
.endb

data: .incbin "data.stc4compr"
