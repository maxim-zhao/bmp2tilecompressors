; lzee depacker for Z80 sjasm
;
; license:zlib license
;
; Copyright (c) 2019 uniabis
; Modified by Maxim to support VRAM and WLA DX
;
; This software is provided 'as-is', without any express or implied
; warranty. In no event will the authors be held liable for any damages
; arising from the use of this software.
;
; Permission is granted to anyone to use this software for any purpose,
; including commercial applications, and to alter it and redistribute it
; freely, subject to the following restrictions:
;
;   1. The origin of this software must not be misrepresented; you must not
;   claim that you wrote the original software. If you use this software
;   in a product, an acknowledgment in the product documentation would be
;   appreciated but is not required.
;
;   2. Altered source versions must be plainly marked as such, and must not be
;   misrepresented as being the original software.
;
;   3. This notice may not be removed or altered from any source
;   distribution.
;

dlze:
.ifdef LZEE_to_VRAM
  ; set VRAM address
  ld c,$bf
  out (c),e
  out (c),d
  ; Emit first literal
  dec c
  outi
  inc de
.else
  ldi
.endif
  scf

getbit1:
  ld a,(hl)
  inc hl
  adc a,a
  jr nc,dlze_lp1n

dlze_lp1:
.ifdef LZEE_to_VRAM
  ld c,$be
  outi
  inc de
.else
  ldi
.endif
dlze_lp2:
  add a
  jr z,getbit1
  jr c,dlze_lp1
dlze_lp1n:
  add a
  call z,getbit
  jr c,dlze_far
  ld bc,0

  add a
  call z,getbit
  rl c

  add a
  call z,getbit
  rl c

  push hl
    ld l,(hl)
    ld h,-1

dlze_copy:
    inc c
    add hl,de
.ifdef LZEE_to_VRAM
    push af
      inc bc
      call _ldir_vram_to_vram
    pop  af
.else
    ldir
    ldi
.endif
  pop hl
  inc hl
  jr dlze_lp2

dlze_far:
  ex af, af'
  ld a,(hl)
  inc hl
  push hl
    ld l,(hl)
    ld c,a
    or 7
    rrca
    rrca
    rrca
    ld h,a
    ld a,c
    and 7
    jr nz,dlze_skip

  pop bc
  inc bc
  ld a,(bc)
  sub 1
  ret c
  push bc

dlze_skip:
    ld b,0
    ld c,a
    ex af, af'
    jr dlze_copy

getbit:
  ld a,(hl)
  inc hl
  adc a
  ret

.ifdef LZEE_to_VRAM ; TODO inline this?
_ldir_vram_to_vram:
  ; Copy bc bytes from VRAM address hl to VRAM address de
  ; Both hl and de are "write" addresses ($4xxx)
  ; Make hl a read address
  res 6, h
  ; Check if the count is below 256
  ld a,b
  or a
  jr nz,_above256 ; unlikely
_below256:
  ; By emitting <=256 at a time, we can use the out (c),r opcode
  ; for address setting, which then relieves pressure on a
  ; and saves some push/pops; and we can use djnz for the loop.
  ld b,c
--:
  ld c,$bf
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
_above256:
  ; Else emit 256*b bytes
-:push bc
    ld b,0
    call --
  pop bc
  djnz -
  jp _below256
.endif