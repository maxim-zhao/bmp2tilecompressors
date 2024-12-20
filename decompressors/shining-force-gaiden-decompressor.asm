; Tile decompression algorithm from Shining Force Gaiden
; Bitstream interleaved with data
; Bitstream is right to left
; 1 = copy literal, 0 = LZ
; If literal, copy one byte from data stream
; If LZ, read two bytes: %llllllll %hhhccccc
; Copy c+3 bytes (range 3..34) from offset hhhllllllll as a signed number with implicit high bits set
; 0s => %1111100000000000 = -2048
; 1s => %1111111111111111 = -1
; Data is interleaved after decompression. LZ references act on the chunky data so it can't be done a tile at a time.
; Thus at minimum a 2KB buffer is needed. Shining Force Gaiden uses a 16KB (?) buffer in cart SRAM.

; Various speed optimisations from the original code...
; First working: speed          15.75% of ldir (including tome to copy to VRAM)
; Deinterleave to VRAM:         15.06% (worse)
; Deinterleave not using ix:    23.14%
; Optimize unrolled loop:       23.87%
; Unroll further:               24.36%
; Simplify counter:             24.71%

; bc = data
; de = write address

sfg_decompress: 
  ; hl = VRAM address
  ld a, l
  out ($bf),a
  ld a, h
  out ($bf),a

    push de
_nextControlByte: 
      ; Get next byte into h
      ld a, (bc)
      inc bc
      ld h, a
      ; Rotate a bit in
      scf
      rr h
      jr nc, _lz ; 0 = LZ
      ; 1
_copyLiteral: 
      ; *de++ = *bc++
      ld a, (bc)
      inc bc
      ld (de), a
      inc de
--: 
      ; Next bit
      srl h
      jr z, _nextControlByte ; unlikely
      jp c, _copyLiteral ; So %01111... = copy n literal bytes, where n = the number of 1s before a 0 is seen (within that control byte)
      ; 0 => lz
_lz:
      push hl
        ; Read word into hl
        ld a, (bc)
        inc bc
        ld l, a
        ld a, (bc)
        inc bc
        ld h, a
        ; If zero, we are (probably) done
        or l
        jr z, _interleave
-:      push bc
          ; bc = low 5 bits of h plus 1 = copy length minus 2
          ld b, $00
          ld a, h
          and %00011111
          ld c, a
          inc c
          ; high 3 bits of h, sign-extended to be negative
          ld a, h
          rlca
          rlca
          rlca
          or %11111000
          ld h, a
          ; l stays as it is
          ; offset from write pointer
          add hl, de
          ; copy n+2 bytes
          ldir
          ldi
          ldi
        pop bc
      pop hl
      jp --
   
_interleave: 
        ; We had a zero LZ word
      pop hl
      ; Save de = write address in hl
      ex de, hl
    pop de
    ; de = start address
    ; hl = end address
    ; compute bc = length
    or a
    sbc hl, de
    ex de,hl ; now de = data length, hl = data address
    ; Divide by 32
    .repeat 5
    srl d
    rr e
    .endr
  
-:;push de
    ; Copy one tile from de
    .repeat 8
      ld bc, 8 ; stride
--:   push hl
        ; Emit every 8th byte, 4 times
        .repeat 4 index m
          ld a,(hl)
          out ($be),a
          .if m != 3
            add hl,bc
          .endif
        .endr
      pop hl
      inc hl
    .endr
  ;pop de
  ; Add 24 to hl
  ld bc,24
  add hl,bc
  dec de
  ; Check if 0
  ld a,d
  or e
  jp nz,-
  ret
