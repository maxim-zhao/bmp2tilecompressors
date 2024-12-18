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

; bc = data
; de = write address

sfg_decompress: 
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
    ; Compute current byte offset from start in hl
    or a
    sbc hl, de
    ld ixh, d
    ld ixl, e
    ld de, 32 ; One tile
-: 
    ; Rearrange all the bitplanes -> convert from chunky to interleaved
    ld c, (ix+4)
    ld b, (ix+1)
    ld (ix+4), b
    ld b, (ix+16)
    ld (ix+16), c
    ld c, (ix+2)
    ld (ix+2), b
    ld b, (ix+8)
    ld (ix+8), c
    ld (ix+1), b
    
    ld c, (ix+12)
    ld b, (ix+3)
    ld (ix+12), b
    ld b, (ix+17)
    ld (ix+17), c
    ld c, (ix+6)
    ld (ix+6), b
    ld b, (ix+24)
    ld (ix+24), c
    ld (ix+3), b
    
    ld c, (ix+20)
    ld b, (ix+5)
    ld (ix+20), b
    ld b, (ix+18)
    ld (ix+18), c
    ld c, (ix+10)
    ld (ix+10), b
    ld b, (ix+9)
    ld (ix+9), c
    ld (ix+5), b
    
    ld c, (ix+28)
    ld b, (ix+7)
    ld (ix+28), b
    ld b, (ix+19)
    ld (ix+19), c
    ld c, (ix+14)
    ld (ix+14), b
    ld b, (ix+25)
    ld (ix+25), c
    ld (ix+7), b
    
    ld c, (ix+13)
    ld b, (ix+11)
    ld (ix+13), b
    ld b, (ix+21)
    ld (ix+21), c
    ld c, (ix+22)
    ld (ix+22), b
    ld b, (ix+26)
    ld (ix+26), c
    ld (ix+11), b
    
    ld c, (ix+29)
    ld b, (ix+15)
    ld (ix+29), b
    ld b, (ix+23)
    ld (ix+23), c
    ld c, (ix+30)
    ld (ix+30), b
    ld b, (ix+27)
    ld (ix+27), c
    ld (ix+15), b
    
    add ix, de
    ld bc,-32
    add hl,bc
    ld a,h
    or l
    jp nz,-
    ret
