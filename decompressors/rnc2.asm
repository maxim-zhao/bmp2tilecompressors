dernc2:
; Args: 
; de = dest: <8000 = VRAM, >8000 = RAM
; hl = source
; Define RNC_RAM_BUFFER pointing to RAM big enough to hold anything decompressed to VRAM
; Originally ripped from The Smurfs Travel The World, but the assembly
; decompression routine is a port of the code at 
; https://github.com/tobiasvl/rnc_propack/blob/master/SOURCE/GAMEBOY/RNC_2.S
; from GB-Z80 to Z80.
  bit 7, d
  jr nz, _deRNCToRAM
  ; Else this is VRAM decompression.
  ; First get the output data size into bc
  ; by pulling it from the header
  push hl
    inc hl ; Skip "RNC",1,0,0
    inc hl
    inc hl
    inc hl
    inc hl
    inc hl
    ld a, (hl)
    inc hl
    ld b, a
    ld c, (hl)
  pop hl
  push bc
  push de
    ; Then decompress to RAM
    ld de, RNC_RAM_BUFFER ; Buffer for VRAM decompression
    call _deRNCToRAM
  pop de
  pop bc
  ; Then write to VRAM address de
  ld a, e
  out ($bf), a
  ld a, d
  or $40
  out ($bf), a
  ld hl, RNC_RAM_BUFFER
-:ld a, (hl)
  inc hl
  out ($be), a
  dec bc
  ld a, b
  or c
  jr nz, -
  ret
 
_deRNCToRAM:
  ; Add 18 bytes to skip header
  ld bc, 18
  add hl, bc
  ; Carry flag rotates through a to signal when sata is needed
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
    ld (de), a
    inc de
    ld a, (hl)
    inc hl
    ld (de), a
    inc de
    dec c
    jr nz, _RAWLPB
  pop af
  jp _XLOOP
 
_FETCH0: 
  ld a, (hl)
  inc hl
  adc a, a
  jr c, _SMALLS
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
-:    ld a, (hl)
      inc hl
      ld (de), a
      inc de
      dec c
      jr nz, -
    pop hl
    inc hl
  pop af
  jr _XLOOP
 
_GETBITS:
  ; Get next byte
  ld a, (hl)
  inc hl
  ; Rotate a bit out
  adc a, a
  jr c, _STRING
_XBYTE:
  push af
    ; Literal byte: copy to dest
    ld a, (hl)
    inc hl
    ld (de), a
    inc de
  pop af
_XLOOP:
  ; Rotate a bit out
  add a, a
  jr c, +
  push af
    ; Literal byte: copy to dest
    ld a, (hl)
    inc hl
    ld (de), a
    inc de
  pop af
  add a, a
  jr nc, _XBYTE
+: 
  jr z, _GETBITS
_STRING: 
  ld bc, 2
  add a, a
  jr z, _FETCH0
  jr nc, _GETLEN
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
