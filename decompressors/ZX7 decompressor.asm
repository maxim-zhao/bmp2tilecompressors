
; ===============================================================
; Dec 2012 by Einar Saukas, Antonio Villena & Metalbrain
; "Standard" version (70 bytes)
; modified for sms vram by aralbrec
; modified for asm by Maxim :)
; ===============================================================
; 
; Usage:
; ld hl, <data>
; ld de, <VRAM address OR $4000>
; call zx7_decompress_vram
;
; This version only supports match lengths up to 255. This enables 
; it to be smaller and faster, but it is not 100% compatible.
;
; Uses af, bc, de, hl
; ===============================================================

zx7_decompress_vram:
  ; Set VRAM address
  ld c,$bf
  out (c),e
  out (c),d
  dec c ; data port

  ld a, 1<<7 ; Signal bit for flags byte
  ; This is a trick whereby we can cycle a flags byte in a through the carry flag,
  ; and a will never be zero until it is time to read a new flags byte because we
  ; shift a bit to the right of the data to keep it non-zero until they are all
  ; consumed.
  ; Each time we want a bit, we do:
  ; add a, a ; get MSB into carry
  ; jr z, _nextFlagsByte ; get new flags byte if necessary (and shift it into carry)
  ; <use the bit in carry>

-:; First byte is always literal
  outi ; increments hl
  inc de

--: ; Main loop

  add a, a
  call z, _nextFlagsByte
  jr nc, - ; next bit indicates either literal or sequence
  ; 0 bit = literal byte
  ; 1 bit = sequence:
  ; - length is encoded using a variable number of flags bits, encoding
  ;   the number of bits and the value
  ;   length 2 is encoded as %0
  ;                           ^--- indicates 0 bit number, value 0
  ;   length 3 is encoded as %101
  ;                           ^^-- indicates 1 bit number, value 1
  ;   length 4 is encoded as %11010
  ;                           ^^^- indicates 2 bit number, value 2
  ;   ...etc
  ; - offsets are encoded as either 7 or 11 bits plus a flag:
  ;   - 0oooooooo for offset ooooooo
  ;   - 1oooooooo plus bitstream bits abcd for offset abcdooooooo
  
  push de
    ; determine number of bits used for length (Elias gamma coding)
    ld b, 1 ; length result
    ld d, 0 ; d = 0
    
-:  ; Count how many 0 bits we have in the flags sequence
    inc d
    add a, a
    call z, _nextFlagsByte
    jr nc, -
    jp +

    ; determine length
-:  add a, a
    call z, _nextFlagsByte
    rl b
    jp c, _done ; check end marker
+:  dec d
    jr nz, -
    inc b      ; adjust length

    ; determine offset
    ld e, (hl) ; load offset flag (1 bit) + offset value (7 bits)
    inc hl
    sll e ; Undocumented instruction! Shifts into carry, inserts 1 in LSB
    jr nc, + ; if offset flag is set, load 4 extra bits

    add a, a
    call z, _nextFlagsByte
    rl d

    add a, a
    call z, _nextFlagsByte
    rl d

    add a, a
    call z, _nextFlagsByte
    rl d

    add a, a
    call z, _nextFlagsByte
    ccf
    jr c, +
    inc d
    
+:  rr e       ; insert inverted fourth bit into E

    ; copy previous sequence
    ex (sp), hl   ; store source, restore destination
    push hl       ; store destination
      sbc hl, de  ; HL = destination - offset - 1
    pop de        ; DE = destination

    ; ldir vram -> vram
    push af ; need to preserve carry
      ; Make hl a read address
      res 6, h
      inc c ; ld c, $bf
-:    out (c),l
      out (c),h
      in a,($be)
      out (c),e
      out (c),d
      out ($be),a
      inc hl
      inc de
      djnz -
    pop af

    dec c ; ld c, $be ; restore VRAM write port

  pop hl      ; restore source address (compressed data)
  jp nc, --

_nextFlagsByte:
  ld a, (hl)  ; Else load the next byte
  inc hl
  rla         ; And push that into the carry bit
  ret

_done:
  pop hl
  ret
