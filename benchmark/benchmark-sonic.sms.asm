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
	ld hl,$4000
	call Sonic1TileLoader_Decompress
	ret ; ends the test

.define Sonic1TileLoaderMemory $c000	
.include "../decompressors/Sonic decompressor.asm"
