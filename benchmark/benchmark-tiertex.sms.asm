; { "technology": "Tiertex", "extension": "tiertexcompr" }

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
	call decompress_tiertex
	ret ; ends the test

.block "decompressor"
.define TiertexDecompressorMemory $c000
.include "../decompressors/tiertex_decomp.asm"
.endb

data:
.db >foo, <foo, >_sizeof_foo, <_sizeof_foo, 0, 0, 0, 1

foo:
.incbin "data.tiertexcompr"