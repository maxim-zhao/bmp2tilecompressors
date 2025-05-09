; { "technology": "SIMS", "extension": "sims" }

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
	call SIMSDecompress
	ret ; ends the test
	
.define SIMSDecompressorMemory $d000

.block "decompressor"
.include "../decompressors/sims_decompressor.asm"
.endb

data: .incbin "data.sims"
