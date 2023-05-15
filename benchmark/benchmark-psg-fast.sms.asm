; { "technology": "Phantasy Star Gaiden (fast)", "extension": "psgcompr" }

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
	call PSGaiden_tile_decompr
	ret ; ends the test
	
.define PSGaiden_decomp_buffer $c000
.block "decompressor"
.include "../decompressors/Phantasy Star Gaiden decompressor (fast).asm"
.endb

data: .incbin "data.psgcompr"
