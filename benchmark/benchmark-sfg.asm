; { "technology": "Shining Force Gaiden", "extension": "sfg" }

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
  ; We first decompress to RAM...
  ld a, %1000
  ld ($fffc),a
  ld bc,data
  ld de,$8000
  call sfg_decompress
  ; Then copy to VRAM. Compute bc = ix-8000
  ld a,ixh
  sub $80
  ld b,a
  ld c,ixl
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
.include "../decompressors/shining-force-gaiden-decompressor.asm"
.endb

data:
.incbin "data.sfg"
