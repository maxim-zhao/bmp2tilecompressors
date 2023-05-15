; { "technology": "ZX7", "extension": "zx7" }

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
	call zx7_decompress
	ret ; ends the test
	
.define ZX7ToVRAM
.block "decompressor"
.include "../decompressors/ZX7 decompressor.asm"
.endb

data: .incbin "data.zx7"
