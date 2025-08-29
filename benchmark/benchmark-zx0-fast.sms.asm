; { "technology": "ZX0 (fast)", "extension": "zx0" }

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
  call DecompressZX0
  ret ; ends the test
  
.define ZX0ToVRAM
.define ZX0Memory $c010
.block "decompressor"
.include "../decompressors/dzx0_fast_sms.asm"
.endb

data: .incbin "data.zx0"
