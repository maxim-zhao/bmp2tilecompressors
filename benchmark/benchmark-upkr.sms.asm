; { "technology": "upkr", "extension": "upkr" }

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
	exx
	ld de,$4000
	exx
	call unpack
	ret ; ends the test
	
.define upkr_memory $c100
.define upkr_vram
.block "decompressor"
.include "../decompressors/unpack_upkr.asm"
.endb

data: .incbin "data.upkr"
