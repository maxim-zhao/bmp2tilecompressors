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
  ld a, %1000
  ld ($fffc),a
  ld bc,data
  ld de,$8000
  ld hl,$4000
  call sfg_decompress
  ret ; ends the test

.block "decompressor"
.include "../decompressors/shining-force-gaiden-decompressor.asm"
.endb

data:
.incbin "data.sfg"
