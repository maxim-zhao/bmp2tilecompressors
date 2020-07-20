; { "technology": "Phantasy Star", "extension": "pscompr" }

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
	call LoadTiles4BitRLENoDI
	ret ; ends the test
	
.include "../decompressors/Phantasy Star decompressor.asm"

data: .incbin "data.pscompr"
