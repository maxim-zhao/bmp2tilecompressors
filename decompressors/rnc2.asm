dernc2:
; Args: 
; de = dest
; hl = source
; Originally ripped from The Smurfs Travel The World, but the assembly
; decompression routine is a port of the code at 
; https://github.com/tobiasvl/rnc_propack/blob/master/SOURCE/GAMEBOY/RNC_2.S
; from GB-Z80 to Z80, so I applied the (rather old-fashioned) labels from that. 
; Further modifications to support decompressing directly to VRAM by MAxim in 2024.

.ifdef RNCToVRAM
  ld a, e
  out ($bf), a
  ld a, d
  out ($bf), a
.endif
  ; Add 18 bytes to skip header
  ld bc, 18
  add hl, bc
  ; Carry flag rotates through a to signal when data is needed
  scf
  ; Read the first byte and skip the first two bits
  ld a, (hl)
  inc hl
  adc a, a
  add a, a
  jp _XLOOP
 
_FETCH3: 
  ld a, (hl)
  inc hl
  adc a, a
  jp _BACK3
 
_FETCH4: 
  ld a, (hl)
  inc hl
  adc a, a
  jp _BACK4
 
_FETCH5: 
  ld a, (hl)
  inc hl
  adc a, a
  jp _BACK5
 
_FETCH6: 
  ld a, (hl)
  inc hl
  adc a, a
  jp _BACK6
 
_FETCH7: 
  ld a, (hl)
  inc hl
  adc a, a
  jp _BACK7
 
_RAW: 
  ld c, $04
_X4BITS: 
  add a, a
  jr z, _FETCH7
_BACK7: 
  rl b
  dec c
  jr nz, _X4BITS
  push af
    ld a, 3
    add a, b
    add a, a
    ld c, a
_RAWLPB:
    ld a, (hl)
    inc hl
.ifdef RNCToVRAM
    out ($be),a
.else
    ld (de), a
.endif
    inc de
    ld a, (hl)
    inc hl
.ifdef RNCToVRAM
    out ($be),a
.else
    ld (de), a
.endif
    inc de
    dec c
    jr nz, _RAWLPB
  pop af
  jp _XLOOP
 
_FETCH0: 
  ld a, (hl)
  inc hl
  adc a, a
  jp c, _SMALLS
_GETLEN: 
  add a, a
  jr z, _FETCH3
_BACK3: 
  rl c
  add a, a
  jr z, _FETCH4
_BACK4: 
  jr nc, _COPY
  add a, a
  jr z, _FETCH5
_BACK5: 
  dec c
  push hl
    ld h, a
    ld a, c
    adc a, a
    ld c, a
    cp $09
    ld a, h
  pop hl
  jr z, _RAW
_COPY: 
  add a, a
  jr z, _FETCH6
_BACK6: 
  jr nc, _BYTEDISP
  add a, a
  jr nz, +
  ld a, (hl)
  inc hl
  adc a, a
+: 
  rl b
  add a, a
  jr nz, +
  ld a, (hl)
  inc hl
  adc a, a
+: 
  jr c, _BIGDISP
  inc b
  dec b
  jr nz, _BYTEDISP
  inc b
_ANOTHER: 
  add a, a
  jr nz, _DISPX
  ld a, (hl)
  inc hl
  adc a, a
_DISPX: 
  rl b
_BYTEDISP: 
  push af
    ld a, e
    sub (hl)
    push hl
      ld l, a
      ld a, d
      sbc a, b
      ld h, a
      dec hl
      ; Copy c bytes from hl to de
.ifdef RNCToVRAM
      res 6,h ; make a read address
      ld b,c
      ld c,$bf
-:    out (c),l
      out (c),h
      in a,($be)
      out (c),e
      out (c),d
      out ($be),a
      inc hl
      inc de
      djnz -
.else
-:    
      ld a, (hl)
      inc hl
      ld (de), a
      inc de
      dec c
      jr nz, -
.endif
    pop hl
    inc hl
  pop af
  jp _XLOOP
 
_GETBITS:
  ; Get next byte
  ld a, (hl)
  inc hl
  ; Rotate a bit out (and carry in)
  adc a, a
  jr c, _STRING
_XBYTE:
  push af
    ; Literal byte: copy to dest
    ld a, (hl)
    inc hl
.ifdef RNCToVRAM
    out ($be),a
.else
    ld (de), a
.endif
    inc de
  pop af
_XLOOP:
  ; Rotate a bit out
  add a, a
  jr c, +
  ; Zero bit = literal byte
  push af
    ; Literal byte: copy to dest
    ld a, (hl)
    inc hl
.ifdef RNCToVRAM
    out ($be),a
.else
    ld (de), a
.endif
    inc de
  pop af
  ; Rotate a bit out
  add a, a
  jr nc, _XBYTE ; Another zero -> another literal byte
+:; 1 bit 
  jr z, _GETBITS
_STRING: 
  ld bc, 2
  add a, a
  jp z, _FETCH0 ; Need more bits
  jp nc, _GETLEN ; %10 = LZ match
_SMALLS: 
  add a, a
  jr z, _FETCH1
_BACK1: 
  jr nc, _BYTEDISP
  inc c
  add a, a
  jr z, _FETCH2
_BACK2: 
  jr nc, _COPY
  ld c, (hl)
  inc hl
  inc c
  dec c
  jr z, _OVERNOUT
  push af
    ld a, c
    add a, $08
    ld c, a
  pop af
  jp _COPY
 
_BIGDISP: 
  add a, a
  jr nz, +
  ld a, (hl)
  inc hl
  adc a, a
+: 
  rl b
  set 2, b
  add a, a
  jr nz, +
  ld a, (hl)
  inc hl
  adc a, a
+: 
  jr c, _BYTEDISP
  jp _ANOTHER
 
_FETCH1: 
  ld a, (hl)
  inc hl
  adc a, a
  jp _BACK1
 
_FETCH2: 
  ld a, (hl)
  inc hl
  adc a, a
  jp _BACK2
 
_OVERNOUT: 
  add a, a
  jr nz, _CHECK4END
  ld a, (hl)
  inc hl
  adc a, a
_CHECK4END: 
  jr c, _XLOOP
  ret
