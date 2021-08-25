; { "technology": "aPLib (fast, apultra)", "extension": "apultra" }

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
	call aPLib_decompress
	ret ; ends the test

.define aPLibMemory $c000
.define aPLibToVRAM
.block "decompressor"
.include "../decompressors/aPLib decompressor (fast).asm"
.endb

data:
.incbin "data.apultra"