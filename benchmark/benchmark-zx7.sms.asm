; { "technology": "ZX7", "extension": "zx7" }

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
	call zx7_decompress
	ret ; ends the test
	
.define ZX7ToVRAM
.include "../decompressors/ZX7 decompressor.asm"

data: .incbin "data.zx7"
