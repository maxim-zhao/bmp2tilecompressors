; Disabled as the compressor crashes and sometimes doesn't decompress correctly
; { "technology": "Exomizer v2", "extension": "exomizer" }

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
	call deexo
	ret ; ends the test

.define exo_mapbasebits $c000
.define ExomizerToVRAM
.block "decompressor"
.include "../decompressors/Exomizer v2 decompressor.asm"
.endb

data: .incbin "data.exomizer"
