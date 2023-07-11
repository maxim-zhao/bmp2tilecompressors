; { "technology": "Phantasy Star Gaiden", "extension": "psgcompr" }

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
	ld ix,data
	ld hl,$4000
	call PSG_decompress
	ret ; ends the test
	
.define PSGDecoderBuffer $c000
.block "decompressor"
.include "../decompressors/Phantasy Star Gaiden decompressor.asm"
.endb

data: .incbin "data.psgcompr"
