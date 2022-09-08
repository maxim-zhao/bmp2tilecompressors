; { "technology": "ZX0", "extension": "zx0" }

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
	call dzx0_standard
	ret ; ends the test
	
.define ZX0ToVRAM
.block "decompressor"
.include "../decompressors/dzx0_standard_sms.asm"
.endb

data: .incbin "data.zx0"