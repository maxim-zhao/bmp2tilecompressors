; Decompression fails, benchmark disabled
; { "technology": "Pucrunch", "extension": "pucrunch" }

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
	ld de,$4000
	ld hl,data
	call Uncrunch
	ret ; ends the test

.define PucrunchVars $c000
.define PuCrunchToVRAM
.block "decompressor"
.include "../decompressors/PuCrunch decompressor.asm"
.endb

data: .incbin "data.pucrunch"
