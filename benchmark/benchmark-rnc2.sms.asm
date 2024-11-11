; { "technology": "RNC2", "extension": "rnc2" }

.memorymap
defaultslot 0
slotsize $8000
slot 0 $0000
.endme

.rombankmap
bankstotal 2
banksize $8000
banks 2 ; To enable a mapper
.endro

.bank 0 slot 0

.org 0
  ; We enable SRAM
  ld a, %1000
  ld ($fffc),a
  ; Then use it as our buffer
	ld hl,data
	ld de,$0000
	call dernc2
	ret ; ends the test
	
.block "decompressor"
.define RNC_RAM_BUFFER $8000
.include "../decompressors/rnc2.asm"
.endb

data: .incbin "data.rnc2"
