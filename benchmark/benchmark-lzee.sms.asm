; { "technology": "LZEe", "extension": "lzee" }

.memorymap
defaultslot 0
slotsize $8000
slot 0 $0000
.endme

.rombankmap
bankstotal 2 ; enable SRAM in Emulicious
banksize $8000
banks 2
.endro

.bank 0 slot 0

.define LZEE_to_VRAM

.org 0
.ifndef LZEE_to_VRAM
  ; We first decompress to RAM...
  ld a, %1000
  ld ($fffc),a
  ; Then use it as our buffer
  ld hl,data
  ld de,$8000
  call dlze
  ; Then copy to VRAM. Assume de points to the end of the decompressed data
  ld a,d
  sub $80
  ld b,a
  ld c,e
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
.else
  ld hl,data
  ld de,$4000
  call dlze
.endif
  ret ; ends the test
  
.block "decompressor"
.define mapbase $c000
.include "../decompressors/dlzee_fast.asm"
.endb

data: .incbin "data.lzee"
