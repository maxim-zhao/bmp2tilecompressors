; Magic Knight Rayearth 2 tile decompressor
; No RAM used apart from stack

.block "TileLoaderMKRE2"
.section "Tile loader (Magic Knight Rayearth 2)" free

MKRE2_Decompress:
; Arguments:
; hl = data address
; de = VDP address

    set 6, d   ; Make de a VRAM write address (if not already)
    ld c, $bf
    ld a, (hl) ; First byte is zero if using RLE, else LZ (typically 1)
    inc hl
    or a
    jr nz, _lz

_rle:
    ; If 0, RLE on bitplanes. Loop 3 times then fall through for fourth.
    ld b, 3
-:  push bc
    push de
      call _f
    pop de
    pop bc
    inc de
    djnz -
    ; fall through for fourth

__:
    ; Read byte. 0 means end of data.
    ld a, (hl)
    inc hl
    or a
    ret z
    jp p, + ; High bit not set -> RLE

    ; High bit set -> raw, count in remaining 7 bits
    ; Mask to low bits
    and $7F
    ld b, a
-:
    ld a, (hl) ; Read byte
    inc hl
    di
      ; Set VRAM address
      out (c), e
      out (c), d
      ; Emit byte
      out ($be), a
    ei
    ; Move on to next row of the bitplane
    inc de
    inc de
    inc de
    inc de
    djnz -
    jr _b

+:  ; RLE, count in A
    ld b, a
    ld a, (hl)
    inc hl
-:
    di
      out (c), e
      out (c), d
      out ($be), a
    ei
    inc de
    inc de
    inc de
    inc de
    djnz -
    jr _b

_lz:
    ; Initial byte non-zero
    ; Set VRAM address
    di
      out (c), e
      out (c), d
    ei
--:
    ld c, (hl) ; Read byte
    inc hl
    ld b, 8 ; Bit count = 8
-:
    rr c ; rotate a bit out of c
    jr nc, +
    ; Bit was 1:
    ; Emit a raw byte
    ld a, (hl)
    out ($be), a
    inc hl
    inc de
    djnz -
    jr --

+:  ; Bit was 0:
    call +
    djnz -
    jr --

+:  push bc
      ; read word into bc
      ld c, (hl)
      inc hl
      ld b, (hl)
      inc hl
      ; if 0, end
      ld a, c
      or b
      jr z, +
      ; Else:
      push hl
        ; Get low 12 bits of bc as a signed 16-bit number and add to de
        ; This is the relative offset of the match, e.g. $fff means -1,
        ; $000 means -4096
        ; hl = (bc | $F000) + de
        ld a, b
        or $F0
        ld h, a
        ld l, c
        add hl, de
        ; Then clear the write bit so it's a read address
        res 6, h
        ; Retrieve remaining 4 bits as count-3
        ld a, b
        and $F0
        rrca
        rrca
        rrca
        rrca
        add a, 3
        ld b, a
        ld c, $bf
-:
        di
          ; Read byte
          out (c), l
          out (c), h
          inc hl
          push hl ; Delay
          pop hl
          in a, ($be)
          push hl ; Delay
          pop hl
          ; Write byte
          out (c), e
          out (c), d
          out ($be), a ; No delay here
        ei
        inc de
        djnz -
        pop hl
      pop bc
    ret

+:
        pop af
      pop af
    ret


.ends
.endb
