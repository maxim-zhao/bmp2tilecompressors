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
	ld ix,$4000
	ld hl,$4000
	call PSG_decompress
	ret ; ends the test
	
.define PSGDecoderBuffer $c000
.include "../decompressors/Phantasy Star Gaiden decompressor.asm"
