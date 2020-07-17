;Exomizer 2 Z80 decoder
;Copyright (C) 2008-2016 by Jaime Tejedor Gomez (Metalbrain)
;
;Optimized by Antonio Villena and Urusergi (169 bytes)
;Converted for SMS VRAM use by Maxim
;
;Compression algorithm by Magnus Lind
;
; This depacker is free software; you can redistribute it and/or
; modify it under the terms of the GNU Lesser General Public
; License as published by the Free Software Foundation; either
; version 2.1 of the License, or (at your option) any later version.
;
; This library is distributed in the hope that it will be useful,
; but WITHOUT ANY WARRANTY; without even the implied warranty of
; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
; Lesser General Public License for more details.
;
; You should have received a copy of the GNU Lesser General Public
; License along with this library; if not, write to the Free Software
; Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
;
;
;input: hl=compressed data start
; de=uncompressed destination start
;
; you may change exo_mapbasebits to point to any free buffer
;
;ATTENTION!
;A huge speed boost (around 14%) can be gained at the cost of only 5 bytes.
;If you want this, replace all instances of "call _getbit" with "srl a" followed by
;"call z,_getbit", and remove the first two instructions in _getbit routine.

deexo:
.ifdef ExomizerToVRAM
  ; Set VDP address
  ld c,$bf
  out (c),e
  out (c),d
.endif

  ld iy, exo_mapbasebits+11
  ld a, (hl)
  inc hl
  ld b, 52
  push de
    cp a
_initbits: 
    ld c, 16
    jr nz, _get4bits
    ld ixl, c
    ld de, 1 ;DE=b2
_get4bits:
    srl a
    call z, _getbit ;get one bit
    rl c
    jr nc, _get4bits
    inc c
    push hl
      ld hl, 1
      ld (iy+41), c ;bits[i]=b1 (and opcode 41 == add hl,hl)
_setbit: 
      dec c
      jr nz, _setbit-1 ;jump to add hl,hl instruction
      ld (iy-11), e
      ld (iy+93), d ;base[i]=b2
      add hl, de
      ex de, hl
      inc iy
    pop hl
    dec ixl
    djnz _initbits
  pop de
  jr _mainloop

_literalrun: 
    ld e, c ;DE=1
_getbits: 
    dec b
    ret z
_getbits1: 
    srl a
    call z, _getbit
    rl e
    rl d
    jr nc, _getbits
    ld b, d
    ld c, e
  pop de
_literalcopy:
ldir1:
.ifdef ExomizerToVRAM
  ; ROM to VRAM
  ld b,c
  ld c,$be
  outi
  inc de
  ld bc,0
.else
  ldir
.endif
_mainloop: 
  inc c
  srl a
  call z, _getbit ;literal?
  jr c, _literalcopy
  ld c, 239
_getindex: 
    srl a
    call z, _getbit
    inc c
    jr nc,_getindex
    ret z
    push de
      ld d, b
      jp p, _literalrun
      ld iy, exo_mapbasebits-229
      call _getpair
      push de
        rlc d
        jr nz, _dontgo
        dec e
        ld bc, 512+32 ;2 bits, 48 offset
        jr z, _goforit
        dec e ;2?
_dontgo: 
        ld bc, 1024+16 ;4 bits, 32 offset
        jr z, _goforit
        ld de, 0
        ld c, d ;16 offset
_goforit: 
        call _getbits1
        ld iy, exo_mapbasebits+27
        add iy, de
        call _getpair
      pop bc
      ex (sp), hl
      push hl
        sbc hl, de
      pop de
ldir2:
.ifdef ExomizerToVRAM
      ; Assume bc < 256
      ; We can modify hl
      ; Source address is in VRAM
      ; Destination address is in VRAM, we exit with it inremented by bc
      ; We must preserve af
      push af
        ld b,c
        ld c,$bf
        res 6,h
-:      out (c),l
        out (c),h
        in a,($be)
        out (c),e
        out (c),d
        out ($be),a
        inc hl
        inc de
        djnz -
      pop af
.else
      ldir
.endif
    pop hl
    jr _mainloop ;Next!
    
_getpair: 
  add iy, bc
  ld e, d
  ld b, (iy+41)
  call _getbits
  ex de, hl
  ld c, (iy-11)
  ld b, (iy+93)
  add hl, bc ;Always clear C flag
  ex de, hl
  ret

_getbit: 
  ld a, (hl)
  inc hl
  rra
  ret
  
;exo_mapbasebits:defs 156 ;tables for bits, baseL, baseH