; { "technology": "Aspect (Legend of Illusion)", "extension": "aspectcompr" }

.memorymap
defaultslot 0
slotsize $7ff0
slot 0 $0000
slotsize $0010
slot 1 $7ff0
slotsize $4000
slot 2 $8000
slotsize $2000
slot 3 $c000
.endme

.rombankmap
bankstotal 2
banksize $7ff0
banks 1
banksize $0010
banks 1
.endro

.bank 0 slot 0

.org 0
	ld bc,data
	ld de,$4000
	call aspect_decomp
	ret ; ends the test
	
.block "decompressor"
.include "../decompressors/aspect_decomp.asm"
.endb

data: .incbin "data.aspectcompr"
