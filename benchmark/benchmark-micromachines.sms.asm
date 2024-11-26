; { "technology": "Micro Machines", "extension": "mmcompr" }

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

.define MicroMachinesDecompressToVRAM

.org 0
.ifdef MicroMachinesDecompressToVRAM
	ld hl,data
	ld de,$4000
	call micromachines_decompress
.else
  ; We first decompress to RAM...
  ld a, %1000
  ld ($fffc),a
  ; Then use it as our buffer
	ld hl,data
	ld de,$8000
	call micromachines_decompress
  ; Then copy to VRAM. Compute bc = de-8000
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
.endif
	ret ; ends the test

.block "decompressor"
.include "../decompressors/Micro Machines decompressor.asm"
.endb

data:
.incbin "data.mmcompr"