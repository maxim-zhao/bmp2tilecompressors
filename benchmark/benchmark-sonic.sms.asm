; { "technology": "Sonic 1", "extension": "soniccompr" }

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
	call Sonic1TileLoader_Decompress
	ret ; ends the test

.define Sonic1TileLoaderMemory $c000
.block "decompressor"
.include "../decompressors/Sonic decompressor.asm"
.endb

data: .incbin "data.soniccompr"
