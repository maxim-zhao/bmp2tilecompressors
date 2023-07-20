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

.ifndef ShrinklerMemory
.fail "ShrinklerMemory is not defined! This must be the start of a 2048 bytes memory area, aligned to a multiple of 256 bytes, for use during decompression."
.endif

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
    ld a,iyl
    sub c
    ld l,a
    ld a,iyh
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
    ld iyh,a
    ld a,l
    ld iyl,a

    ex de,hl
    sbc hl,bc
    
    ex de,hl
    pop bc
+:  pop hl
    ld (hl),c
    dec h
    ld (hl),b
    ret

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
