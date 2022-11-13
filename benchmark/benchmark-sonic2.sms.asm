; { "technology": "Sonic 2", "extension": "sonic2compr" }

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
	call Sonic2TileLoader_Decompress
	ret ; ends the test

.define Sonic2TileLoaderMemory $c000
.block "decompressor"
.include "../decompressors/Sonic 2 decompressor.asm"
.endb

data: .incbin "data.sonic2compr"
