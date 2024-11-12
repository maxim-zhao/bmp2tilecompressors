; { "technology": "RNC1", "extension": "rnc1" }

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
  ; We enable SRAM
  ld a, %1000
  ld ($fffc),a
  ; Then use it as our buffer
	ld hl,data
	ld de,$8000
	call Unpack
  ; Then copy to VRAM. We have to read the size from the header.
  ld ix,data
  ld b,(ix+6)
  ld c,(ix+7)
  xor a
  out ($bf),a
  ld a,$40
  out ($bf),a
  ld hl,$8000
-:ld a,(hl)
  inc hl
  out ($be),a
  dec bc
  ld a,b
  or c
  jp nz,-
	ret ; ends the test
	
.block "decompressor"
.define RNC_MEMORY $c000
.include "../decompressors/rnc1.asm"
.endb

data: .incbin "data.rnc1"
