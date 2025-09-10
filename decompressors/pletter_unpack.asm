; pletter v0.5c SMS unpacker

; call unpack with hl pointing to some pletter5 data, and de pointing to the destination.
; changes all registers

; Modified by Maxim in 2025 for use with WLA DX
; and to target SMS VRAM

.macro GETBIT
  add a,a
  call z,_getbit
.endm

.macro GETBITEXX
  add a,a
  call z,_getbitexx
.endm

pletter_unpack:
.ifdef pletter_to_vram
  ; Set VRAM address
  ld a, e
  out ($bf), a
  ld a, d
  out ($bf), a
.endif
  ld a,(hl)
  inc hl
  exx
  ld de,0
  add a,a
  inc a
  rl e
  add a,a
  rl e
  add a,a
  rl e
  rl e
  ld hl,_modes
  add hl,de
  ld e,(hl)
  ld ixl,e
  inc hl
  ld e,(hl)
  ld ixh,e
  ld e,1
  exx
  ld iy,_loop
_literal:
.ifdef pletter_to_vram
  push af
    ld a, (hl)
    inc hl
    inc de
    out ($be), a
  pop af
.else
  ldi
.endif
_loop:
  GETBIT
  jr nc,_literal
  exx
  ld h,d
  ld l,e
_getlen:
  GETBITEXX
  jr nc,_lenok
_lus
  GETBITEXX
  adc hl,hl
  ret c
  GETBITEXX
  jr nc,_lenok
  GETBITEXX
  adc hl,hl
  ret c
  GETBITEXX
  jp c,_lus
_lenok
  inc hl
  exx
  ld c,(hl)
  inc hl
  ld b,0
  bit 7,c
  jp z,_offsok
  jp ix

_mode6:
  GETBIT
  rl b
_mode5:
  GETBIT
  rl b
_mode4:
  GETBIT
  rl b
_mode3:
  GETBIT
  rl b
_mode2:
  GETBIT
  rl b
  GETBIT
  jr nc,_offsok
  or a
  inc b
  res 7,c
_offsok:
  inc bc
  push hl
  exx
  push hl
  exx
  ld l,e
  ld h,d
  sbc hl,bc
  pop bc
.ifdef pletter_to_vram
  call _ldir_vram_to_vram
.else
  ldir
.endif
  pop hl
  jp iy

_getbit:
  ld a,(hl)
  inc hl
  rla
  ret

_getbitexx:
  exx
  ld a,(hl)
  inc hl
  exx
  rla
  ret

_modes:
.dw _offsok
.dw _mode2
.dw _mode3
.dw _mode4
.dw _mode5
.dw _mode6

.ifdef pletter_to_vram
_ldir_vram_to_vram:
  ; Copy bc bytes from VRAM address hl to VRAM address de
  ; Both hl and de are "write" addresses ($4xxx)
  push af
    ; Make hl a read address
    ld a,h
    xor $40
    ld h,a
    ; Check if the count is below 256
    ld a,b
    or a
    jr z,_below256
    ; Else emit 256*b bytes
-:  push bc
      ld c,$bf
      ld b,0
      call +
    pop bc
    djnz -
    ; Then fall through for the rest - if c>0
    ld a,c
    or a
    jr z,_done
_below256:
    ; By emitting 256 at a time, we can use the out (c),r opcode
    ; for address setting, which then relieves pressure on a
    ; and saves some push/pops; and we can use djnz for the loop.
    ld b,c
    ld c,$bf
    call +
_done:
  pop af
  ld c,0
  ret

+:
-:out (c),l
  out (c),h
  in a,($be)
  out (c),e
  out (c),d
  out ($be),a
  inc hl
  inc de
  djnz -
  ret
.endif