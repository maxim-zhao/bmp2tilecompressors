WBMWDecompressTiles:  
  ld b, 4 ; bitplanes
---:
  push bc
    ; Set VRAM address
    ld c, $bf
    out (c), e
    out (c), d
    dec c ; so we can use it later to write
    
    jp _➰
    
_Raw:  
    ; Marker byte is the negated count -1..-128
    neg
    ld b, a
-:  ; Write a byte
    outi ; out (c), (hl); inc hl; dec b
    in a, ($be) ; Skip 3 bytes. We use this opcode to preserve the flags.
    in a, ($be) ; (in r,(c) affects z.)
    in a, ($be)
    jp nz, - ; Repeat until done
    ; fall through
  
    ; Decompress
_➰:ld a, (hl) ; Read a byte
    inc hl
    or a
    jp m, _Raw
    jr z, _Terminator
    ; fall through

_RLE:
    ; a = number of bytes
    ld b, a
    ; Get value to write
    ld a, (hl)
    inc hl
-:  ; Output byte to VRAM
    out (c), a
    in f, (c) ; Skip 3 bytes
    in f, (c)
    in f, (c)
    djnz - ; Repeat until done
    jp _➰ ; Then get next byte

_Terminator:
    ; Zero means end of bitplane
    inc de ; Move to next bitplane
  pop bc
  djnz ---
  ret
    