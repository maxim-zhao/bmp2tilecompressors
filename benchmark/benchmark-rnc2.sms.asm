; { "technology": "RNC2", "extension": "rnc2" }

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
	call dernc2
	ret ; ends the test
	
.block "decompressor"
.define RNCToVRAM
.include "../decompressors/rnc2.asm"
.endb

data: .incbin "data.rnc2"
