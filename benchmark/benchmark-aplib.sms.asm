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

.define aPLibMemory $c000
.define aPLibToVRAM
.include "../decompressors/aPLib decompressor.asm"

.bank 0 slot 0

.org 0
	ld hl,$4000
	ld de,$4000
	call aPLib_decompress
	ret ; ends the test