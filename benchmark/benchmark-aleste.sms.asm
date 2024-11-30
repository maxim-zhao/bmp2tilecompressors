; { "technology": "Aleste", "extension": "aleste" }

.memorymap
defaultslot 0
slotsize $8000
slot 0 $0000
.endme

.rombankmap
bankstotal 2
banksize $8000
banks 2
.endro

.bank 0 slot 0

.org 0
	ld hl,data
	ld de,$4000
	call aleste_decompress
	ret ; ends the test

.block "decompressor"
.define aleste_decompress_memory $c000
.include "../decompressors/aleste_decompressor.asm"
.endb

data:
.incbin "data.aleste"