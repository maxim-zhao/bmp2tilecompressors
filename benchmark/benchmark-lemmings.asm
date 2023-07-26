; { "technology": "Lemmings", "extension": "lemmingscompr" }

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
	ld de,data
	ld ix,$4000
	call lemmings_decompress
	ret ; ends the test
	
.define LemmingsDecompressorMemory $c100
.block "decompressor"
.include "../decompressors/Lemmings decompressor.asm"
.endb

data: .incbin "data.lemmingscompr"
