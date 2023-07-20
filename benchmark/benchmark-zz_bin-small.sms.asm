; { "technology": "Uncompressed (otir)", "extension": "bin" }

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
    ld bc,dataSize
    call copy
    ret ; ends the test
    
; This is a space-optimised routine for emitting bc bytes from hl to VRAM address de.
copy:
    ld a,b
    push bc
      ld c,$bf
      out (c),e
      out (c),d
      or a
      jr z,+
      ld bc,$00be
-:    otir
      dec a
      jp nz,-
+:  pop de
    ld a,e
    or a
    ret z
    ld b,a
    otir
    ret

data: .incbin "data.bin" fsize dataSize
