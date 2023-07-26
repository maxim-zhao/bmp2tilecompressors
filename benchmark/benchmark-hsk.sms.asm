; { "technology": "High School Kimengumi", "extension": "hskcompr" }

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
	ld bc,data
	ld de,$4000
	call decode
	ret ; ends the test
	
.define HighSchoolKimengumiDecompressorMemory $c100
.block "decompressor"
.include "../decompressors/High School Kimengumi decompressor.asm"
.endb

data: .incbin "data.hskcompr"
