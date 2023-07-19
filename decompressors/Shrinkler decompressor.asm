; Copyright 1999-2015 Aske Simon Christensen.
; Decompress Shrinkler-compressed data produced with the -d option.
; Uses 3 kilobytes of buffer anywhere in the memory
; Decompression code may read one longword beyond compressed data.
; The contents of this longword does not matter.
;
; official Z80 release (recall) - 254 bytes
; Conversion by roudoudou
; size optimisations by Hicks, Antonio Villena & Urusergi
; Adapted for SMS + WLA DX + writing to VRAM by Maxim in 2023
;
; usage
;-------
; LD IX,source
; LD DE,destination
; CALL shrinkler_decrunch

.ifndef ShrinklerMemory
.fail "ShrinklerMemory is not defined! This must be the start of a 3082 bytes memory area for use during decompression."
.endif

; Shrinkler z80 decompressor Madram/OVL version v8
;
; Modified by Ivan Gorodetsky (2019-10-06)
; 1. Support of R800 with hardware multiplication added
; 2. Self-modifying code removed
; 3. Initialisation of d2 for reuse of decompressor added
; 4. Minor optimisations
; 5. No parity context
;
; 206 bytes - z80 version
; 193 bytes - R800 version
; Compile with The Telemark Assembler (TASM) 3.2
;
; input IX=source
; input DE=destination    - it has to be even!
; call shrinkler_decrunch

; you may change probs to point to any 256-byte aligned free buffer (size 2.0 Kilobytes)
.enum ShrinklerMemory
  probs dsb $400
  probs_ref .db
  probs_length dsb $200
  probs_offset dsb $200
.ende

shrinkler_decrunch:
.ifdef ShrinklerToVRAM
    ; Set VRAM address
    ld c,$bf
    out (c),e
    out (c),d
.endif
    exx
    ld hl,8*256+probs
    ld bc,8*256+80h
    xor a
--: dec h
-:  ld (hl),a
    inc l
    jr nz,-
    xor c
    djnz --
    ld d,b
    ld e,b
    push de
    pop iy      ; d2=0
    inc e
    ld a,c
    ex af,af'

_literal:
    scf
-:  call nc,_getbit
    rl l
    jr nc,-
    ld a,l
    exx

.ifdef ShrinklerToVRAM
    out ($be),a
.else
    ld (de),a
.endif

    inc de
    call _getkind
    jr nc,_literal
    ld h,probs_ref/256
    call _getbit
    jr nc,_readoffset
_readlength:
    ld h,probs_length/256
    call _getnumber
    push hl
    add hl,de

.ifdef ShrinklerToVRAM
    ; Copy bc bytes from VRAM address hl to VRAM address de
    ; Both hl and de are "write" addresses ($4xxx)
    push af
      ; Make hl a read address
      res 6,h
      ; Check if the count is below 256
      ld a,b
      or a
      call nz,ldir_vram_to_vram_above256 ; rare
      ; Then the rest - if c>0
      ld a,c
      or a
      jr z,_done ; unlikely
      ; By emitting 256 at a time, we can use the out (c),r opcode
      ; for address setting, which then relieves pressure on a
      ; and saves some push/pops; and we can use djnz for the loop.
      ld b,c
      ld c,$bf
      call _ldir_vram_to_vram_b
_done:
    pop af
; TODO needed?    ld c,0
.else
    ldir
.endif
    pop hl
    call _getkind
    jr nc,_literal
_readoffset:
    ld h,probs_offset/256
    call _getnumber
    ld hl,2
    sbc hl,bc
    exx
    jr nz,_readlength
    ret

_getnumber:
-:  inc l
    inc l
    call _getbit
    jr c,-
    exx
      ld bc,1
    exx
-:  dec l
    call _getbit
    exx
      rl c
      rl b
    exx
    dec l
    jr nz,-
    exx
    ret
    
_readbit:
    ex de,hl
    add hl,hl
    ex de,hl
    ex af,af'
    add a,a
    jr nz,+
    ld a,(ix+0)
    inc ix
    adc a,a
+:  ex af,af'
    add iy,iy
    ex af,af'
    jr nc,+
    inc iyl
+:  ex af,af'
    jr _getbit

_getkind:
    exx
    ld hl,probs
_getbit:
    ld a,d
    add a,a
    jr nc,_readbit
    ld b,(hl)
    inc h
    ld c,(hl)
    push hl
    ld l,c
    ld h,b
    push bc
    ld a,%11100001
-:  srl b
    rr c
    add a,a
    jr c,-
    sbc hl,bc
    pop bc
    push hl

    sbc hl,hl
-:  add hl,hl
    rl c
    rl b
    jr nc,+
    add hl,de
    jr nc,+
    inc bc
+:  dec a
    jr nz,-
    ld a,iyl;.db 0FDh,7Dh  ; ld a,yl
    sub c
    ld l,a
    ld a,iyh;.db 0FDh,7Ch  ; ld a,yh
    sbc a,b    
    jr nc,_zero
    ; one:
    ld e,c
    ld d,b

    pop bc
    ld a,b
    sub $F0
    ld b,a
    dec bc
    jr +

_zero:
    ld iyh,a;.db 0FDh,67h  ; ld yh,a
    ld a,l
    ld iyl,a;.db 0FDh,6Fh  ; ld yl,a

    ex de,hl
    sbc hl,bc
    
    ex de,hl
    pop bc
+:  pop hl
    ld (hl),c
    dec h
    ld (hl),b
    ret


    ;.end

/*
.enum ShrinklerMemory
  shrinkler_d5  dw
  shrinkler_d3  dw
  shrinkler_d2  dw
  shrinkler_d6  dw
  shrinkler_d4  dsw 2
  shrinkler_pr  dsb 3072 ; Yes, really!
.ende
        
shrinkler_decrunch:
.ifdef ShrinklerToVRAM
  ; Set VRAM address
  ld c,$bf
  out (c),e
  out (c),d
.endif
        ; Init range decoder state
        ld      hl, shrinkler_d3
        xor     a
        ex      af, af'
        xor     a
        ld      bc, $070d
        ld      (hl), 1
shrinkler_repeat:
        inc     hl
        ex      af, af'
        ld      (hl), a
        djnz    shrinkler_repeat
        ld      a, $80
        dec     c
        jr      nz, shrinkler_repeat

shrinkler_lit:
        ld      hl, shrinkler_d6
        inc     (hl)
shrinkler_getlit:
        call    shrinkler_getbit
        rl      (hl)
        jr      nc, shrinkler_getlit
.ifdef ShrinklerToVRAM
  ; bc value is not used. Need to protect hl, de, af
  push af
    ld a,(hl)
    inc hl
    inc de
    out ($be),a
  pop af
.else
        ldi
.endif
        ; After literal
        call    shrinkler_getkind
        jr      nc, shrinkler_lit
        ; Reference
        call    shrinkler_altgetbit
        jr      nc, shrinkler_readoffset
shrinkler_readlength:
        call    shrinkler_getnumber
        ld      hl, (shrinkler_d5)
        add     hl, de
.ifdef ShrinklerToVRAM
  ; Copy bc bytes from VRAM address hl to VRAM address de
  ; Both hl and de are "write" addresses ($4xxx)
  push af
    ; Make hl a read address
    res 6,h
    ; Check if the count is below 256
    ld a,b
    or a
    call nz,ldir_vram_to_vram_above256 ; rare
    ; Then the rest - if c>0
    ld a,c
    or a
    jr z,_done ; unlikely
    ; By emitting 256 at a time, we can use the out (c),r opcode
    ; for address setting, which then relieves pressure on a
    ; and saves some push/pops; and we can use djnz for the loop.
    ld b,c
    ld c,$bf
    call _ldir_vram_to_vram_b
_done:
  pop af
  set 6,h
  ld c,0
.else
        ldir
.endif
        ; After reference
        call    shrinkler_getkind
        jr      nc, shrinkler_lit
shrinkler_readoffset:
        dec     a
        call    shrinkler_getnumber
        ld      hl, 2
        sbc     hl, bc
        ld      (shrinkler_d5), hl
        jr      nz, shrinkler_readlength

shrinkler_getnumber:
        ; Out: Number in BC
        ld      bc, 1
        ld      hl, shrinkler_d6+1
        ld      (hl), a
        dec     hl
        ld      (hl), c
shrinkler_numberloop:
        inc     (hl)
        call    shrinkler_getbit
        inc     (hl)
        jr      c, shrinkler_numberloop
shrinkler_bitsloop:
        dec     (hl)
        dec     (hl)
        ret     m
        call    shrinkler_getbit
        rl      c
        rl      b
        jr      shrinkler_bitsloop

shrinkler_readbit:
        pop     de
        ld      (shrinkler_d3), hl
        ld      hl, (shrinkler_d4)
        ld      de, (shrinkler_d4+2)
shrinkler_rb0:
        adc     hl, hl
        ld      (shrinkler_d4), hl
        ex      de, hl
        jr      nz, shrinkler_rb2-1

shrinkler_rb4:
        adc     hl, hl
        jr      nz, shrinkler_rb3
        ; HL=DE=0
shrinkler_rb1:
        ccf
        ld      h, (ix+0)               ; DEHL=(a4) big endian value read!
        ld      l, (ix+1)
        inc     ix
        inc     ix
        jr      c, shrinkler_rb0
        ex      de, hl
        jr      shrinkler_rb1-3

shrinkler_rb2:
        ld      l, d                    ; adc hl, hl with $ed
shrinkler_rb3:
        ld      (shrinkler_d4+2), hl    ; written in little endian
        ld      hl, (shrinkler_d2)
        adc     hl, hl
        ld      (shrinkler_d2), hl
        jr      shrinkler_getbit1

shrinkler_getkind:
        ;Use parity as context
        ld      l, 1
shrinkler_altgetbit:
        ld      a, l
        and     e
        ld      h, a
        dec     hl
        ld      (shrinkler_d6), hl

shrinkler_getbit:
        exx
shrinkler_getbit1:
        ld      hl, (shrinkler_d3)
        push    hl
        add     hl, hl
        jr      nc, shrinkler_readbit
        ld      hl, (shrinkler_d6)
        add     hl, hl
        ld      de, shrinkler_pr+4      ; cause -1 context
        add     hl, de
        ld      e, (hl)
        inc     hl
        ld      d, (hl)
        ld      b, d
        ld      c, e                    ; bc=de=d1 / hl=a1
        ld      a, $eb
shrinkler_shift4:
        srl     b
        rr      c
        add     a, a
        jr      c, shrinkler_shift4-1
        sbc     hl, bc                  ; hl=d1-d1/16
        ex      de, hl
        ld      b, (hl)
        ld      (hl), d
        dec     hl
        ld      c, (hl)
        ld      (hl), e
        ex      (sp), hl
        ex      de, hl
        sbc     hl, hl
shrinkler_muluw:
        add     hl, hl
        rl      e
        rl      d
        jr      nc, shrinkler_cont
        add     hl, bc
        jr      nc, shrinkler_cont
        inc     de
shrinkler_cont:
        sub     $b
        jr      nz, shrinkler_muluw
        pop     bc                      ; bc=d1 initial
        ld      hl, (shrinkler_d2)
        sbc     hl, de
        jr      nc, shrinkler_zero

shrinkler_one:
        ; onebrob = 1 - (1 - oneprob) * (1 - adjust) = oneprob - oneprob * adjust + adjust
        ld      a, (bc)
        sub     1
        ld      (bc), a
        inc     bc
        ld      a, (bc)
        sbc     a, $f0                  ; (a1)+#FFF
        ld      (bc), a
        ex      de, hl
        jr      shrinkler_d3ret

shrinkler_zero:
        ; oneprob = oneprob * (1 - adjust) = oneprob - oneprob * adjust
        ld      (shrinkler_d2), hl
        ld      hl, (shrinkler_d3)
        sbc     hl, de
        ; oneprob*adjust < oneprob so carry is always cleared...
shrinkler_d3ret:
        ld      (shrinkler_d3), hl
        exx
        ld      a, 4
        ret
*/
.ifdef ShrinklerToVRAM
ldir_vram_to_vram_above256:
    ; Emit 256*b bytes
-:  push bc
      ld c,$bf
      ld b,0
      call _ldir_vram_to_vram_b
    pop bc
    djnz -
    ret
_ldir_vram_to_vram_b:
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
