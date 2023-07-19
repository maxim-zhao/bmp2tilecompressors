; { "technology": "Shrinkler", "extension": "shrinkler" }

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
	ld de,$4000
	call shrinkler_decrunch
	ret ; ends the test
	
.define ShrinklerToVRAM
.define ShrinklerMemory $c000
.block "decompressor"
.include "../decompressors/Shrinkler decompressor.asm"
.endb

data: .incbin "data.shrinkler"
