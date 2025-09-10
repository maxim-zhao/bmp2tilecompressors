; { "technology": "Pletter", "extension": "pletter" }

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
	call pletter_unpack
	ret ; ends the test

.block "decompressor"
.define pletter_to_vram
.include "../decompressors/pletter_unpack.asm"
.endb

data:
.incbin "data.pletter"