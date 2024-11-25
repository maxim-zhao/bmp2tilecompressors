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

.org 0
  ; We first decompress to RAM...
  ld a, %1000
  ld ($fffc),a
  ; Then use it as our buffer

	ld hl,data
	ld de,$8000
	call micromachines_decompress
  ; Then copy to VRAM. HC is the byte count (for now)
  ld b,h
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

;.define aPLibMemory $c000
;.define aPLibToVRAM
.block "decompressor"
;.define MicroMachinesDecompressToVRAM
.include "../decompressors/Micro Machines decompressor.asm"
.endb

data:
.incbin "data.mmcompr"