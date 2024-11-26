; Parameters:
; hl = source
; de = dest
; Uses afbcdea'f'

; Via SRAM: 84.6% compression of zip, 22% speed of otir
; Direct to VRAM: same compression, 18% speed

micromachines_decompress:
.ifdef MicroMachinesDecompressToVRAM
  ld a,e
  out ($bf),a
  ld a,d
  out ($bf),a
.endif
  jr _get_next_bitstream_byte_set_carry

_large_lz_use_a:
      ld b, a
_large_lz_use_b:
      ld a, e     ; get dest low byte
      sub (hl)    ; subtract next byte
      inc hl
      ld c, (hl)  ; and get the next byte as a counter
      push hl
        ld l, a   ; Save subtracted low byte
        ld a, d   ; High byte
        sbc a, b  ; Subtract carry (from offset subtraction) and b
        ld h, a   ; That's our offset + 1
        dec hl
.ifdef MicroMachinesDecompressToVRAM
        ; set bc = c + 4
        push hl
          ld b,0
          ld hl,4
          add hl,bc
          ld c,l
          ld b,h
        pop hl
        jr _copybcbytes
.else
        ld a, c   ; save c
        ldi       ; copy 3 bytes
        ldi
        ldi
        ld c, a   ; restore c
        ld b, 0
        inc bc    ; increment counter
        jr _copybcbytes
.endif

_small_lz: ; LZ: copy x+3 bytes from offset -(*p++ + (high-2)*256 - 2)
; Examples:
; 21 ee
; de = c118
; Copy 4 bytes from $c118 - 0*256 - $ee - 2 = $c028
; 30 02
; de = c1d0
; Copy 3 bytes from $c1d0 - 1*256 - 2 - 2 = $c0cc
; 40 76
; de = c2a8
; Copy 3 bytes from $c2a8 - 2*256 - $76 - 2 = $c030

      ld b, a     ; save value - in range 0-$2f?
      and $0F     ; c = x+2
      add a, $02
      ld c, a
      ld a, b
      and $30     ; Mask to high bits
      rlca        ; Get high nibble (0..2)
      rlca
      rlca
      rlca
      cpl         ; Invert (-1..-3)
      ld b, a     ; -> B
      ld a, (hl)  ; Get next byte
      push hl
        cpl         ; Subtract from de
        add a, e
        ld l, a
        ld a, d
        adc a, b    ; Also subtract b*256
        ld h, a
        dec hl

_copycplusonebytes:
.ifdef MicroMachinesDecompressToVRAM
        ; Why does it do ldi; ldir?
        ; So 1 means 1, 0 means 256
        ; So let's make bc the right number
        ld b,1
        dec bc
        ld b,0
        inc bc
        inc bc
        ; yuck :(
.else
        ld b, $00 ; counter = c
        inc c
        ldi       ; copy from hl to de
.endif
_copybcbytes:
.ifdef MicroMachinesDecompressToVRAM
        call _ldir_vram_to_vram
.else
        ldir      ; 1-257 bytes?
.endif
      pop hl
      inc hl      ; Next byte
    ex af, af'
    jr _get_next_bitstream_bit

_large_lz: ; LZ: data bytes are offset, count.
; If x is $f, there is an extra offset-high byte first.
; Else, x is the high byte.
; Copy count+4 bytes from -(offset + 1).
; Examples:
; 5f 0f ff 06
; de = d180
; Copy 10 bytes from $d180 - $fff - 1 = $c180
; 53 26 01
; de = d1f2
; Copy 5 bytes from $d1f2 - $326 - 1 = $cecb
      cp $0F              ; Use x unless it's $f, when the next byte is the count
      jr nz, _large_lz_use_a
      ld b, (hl)          ; else b = next byte
      inc hl
      jr _large_lz_use_b

_tiny_lz: ; %1 nn ooooo Copy n+2 bytes from offset -(n+o+2)
; e.g. $ad = %1 01 01101 = 1, 13
; de = $c120
; So 3 bytes of data is written from $c120 which is copied from $c120 - $1 - $d - $2 = c110
      cp $FF        ; All the bits set?
      ret z         ; If so, done

      and %01100000 ; Mask to n bits
      rlca          ; Move to LSBs = n
      rlca
      rlca
      inc a         ; Add 1
      ld c, a

      ld a, (hl)    ; Get byte again
      push hl
        and %00011111 ; Mask to o bits
        add a, c    ; Add n+1
        cpl         ; Same as neg and then dec, i.e. now it's -(o+n+2)
        add a, e    ; Overall: hl = de - (o+n+2)
        ld l, a
        ld a, d
        adc a, -1
        ld h, a
        jp _copycplusonebytes

_copy_byte_and_get_next_bitstream_byte_with_carry:
.ifdef MicroMachinesDecompressToVRAM
    call _ldi_rom_to_vram
.else
    ldi
.endif
_get_next_bitstream_byte_set_carry:
    scf           ; Set carry, so as we shift we should not have zeroes in a even if it would otherwise
_get_next_bitstream_byte:
    ld a, (hl)    ; Get byte
    inc hl        ; Point at next

    adc a, a      ; High bit 1 = compressed, 0 = raw  $byte follows. Unrolled loop here, plus carry in
    jr c, _bitstream1
_raw_single:
.ifdef MicroMachinesDecompressToVRAM
    call _ldi_rom_to_vram
.else
    ldi
.endif
_get_next_bitstream_bit:
.repeat 6
    add a, a
    jr c, _bitstream1
.ifdef MicroMachinesDecompressToVRAM
    call _ldi_rom_to_vram
.else
    ldi
.endif
.endr
    add a, a
    jr nc, _copy_byte_and_get_next_bitstream_byte_with_carry
    ; else fall though

_bitstream1:
    jr z, _get_next_bitstream_byte      ; If no set bits left, we're done with our bitstream byte (we got to the terminating bit)
    ex af, af'                ; Save a
_byte_stream_next_byte:
      ld a, (hl)
      cp $80
      jr nc, _tiny_lz         ; $80+ = tiny LZ
      inc hl                  ; Point at next byte
      sub $70
      jr nc, _counting        ; $7x = counting RLE
      add a, $10
      jr c, _reverse_lz       ; $6x = reverse LZ
      add a, $10
      jr c, _large_lz         ; $5x = large LZ
      add a, $30
      jp c, _small_lz         ; $2x, $3x, $4x = small LZ
      add a, $10
      jp nc, _raw             ; $0x = raw
      ; Else it is $1x = RLE

_rle: ; Repeat previous byte x+2 times
      ld b, $00
      sub $0F                 ; Check for $f
      jr z, ++                ; $f means there's another data byte
      add a, $11              ; c = x + 2
_duplicate_previous_byte_ba_times:
      ld c, a
.ifdef MicroMachinesDecompressToVRAM
      ; We want c=0 to be 256
      ld b,1
      dec bc
      ld b,0
      inc bc
      ; then prepare it to iterate below
      ld a,c
      or a
      jr z,+
      inc b
+:      
      push bc
        ; First read the value in
        dec de
        res 6,d
        ld c,$bf
        out (c),e
        out (c),d
        in a,($be)
        ; Then set the write address again
        set 6,d
        inc de
        out (c),e
        out (c),d
      pop bc
      ; now output a bc times without changing a
-:    out ($be),a
      inc de
      dec c
      jr nz,-
      djnz -
      
.else
      push hl
        ld l, e               ; repeat previous byte x times
        ld h, d
        dec hl
        ldi
        ldir
      pop hl
.endif
    ex af, af'
    jr _get_next_bitstream_bit

++:   ld a, (hl)              ; next byte
      inc hl
      add a, $11              ; add 17
      jr nc, _duplicate_previous_byte_ba_times ; check for >255
      inc b                   ; if so, increment high counter byte
      jr _duplicate_previous_byte_ba_times

_counting: ; run of x+2 incrementing bytes, following last byte written. x = $f -> next byte is run length - 17.
; Examples:
; 70
; Run of 2 bytes following previous value
; 7f 03
; Run of 20 bytes following previous value
      sub $0F           ; $f means the next byte is the run length
      jr nz, +          ; else it is the run length (-2) itself
      ld a, (hl)
      inc hl
+:    add a, $11        ; a = x+2 (overflows!)
      ld b, a
.ifdef MicroMachinesDecompressToVRAM
      push bc
        ; First read the value in
        dec de
        res 6,d
        ld c,$bf
        out (c),e
        out (c),d
        in a,($be)
        ; Then set the write address again
        set 6,d
        inc de
        out (c),e
        out (c),d
      pop bc
      ; now output a b times, pre-incrementing a
-:    inc a
      out ($be),a
      inc de
      djnz -
.else
      dec de
      ld a, (de)        ; read previous byte
      inc de
-:    inc a             ; add 1
      ld (de), a        ; write
      inc de
      djnz -
.endif
    ex af, af'
--: jp _get_next_bitstream_bit

_reverse_lz: ; LZ run in reverse order
; 6x oo
; Copy x+3 bytes, starting from -(o+1) and working backwards
; Examples:
; 64 0a
; de = d229
; Copy 7 bytes from $d229 - $0a - 1 = $d21e
; down to $d21e - 6 = $d218 inclusive
      add a, $03            ; b = x+3
      ld b, a
      ld a, (hl)            ; get next byte o
      push hl
        cpl                 ; invert bits
        scf                 ; set carry
        adc a, e            ; hl = de - o
        ld l, a
        ld a, d
        adc a, $FF
        ld h, a
.ifdef MicroMachinesDecompressToVRAM
        ; Copy b bytes from --hl to de++
        res 6,h
        ld c,$bf
-:      dec hl
        out (c),l
        out (c),h
        in a,($be) ; Read
        out (c),e
        out (c),d
        out ($be),a ; Write
        inc de
        djnz -
.else
-:      dec hl              ; move back
        ld a, (hl)          ; read byte
        ld (de), a          ; copy to de
        inc de
        djnz -
.endif
      pop hl
      inc hl
    ex af, af'
    jr --

_raw: ; Raw run:
; if (x == $f)
;   x = next byte
;   if (x = $ff)
;     x = following 2 bytes
;   else
;     x += 30
; else
;   x += 8
; Copy x raw bytes
; Examples:
; 03 [data x 11]
; 0f 0c [data x 42]
; 0f ff 12 34 [data x 13330]
      ld b, $00
      inc a                 ; increment
      jr z, _0f             ; That'd be $0f originally
.ifdef MicroMachinesDecompressToVRAM
      push hl
        ; a is now $fx where x is the byte count minus 7
        ld c,a
        ld hl,$17 - $100
        add hl,bc
        ld c,l
        ld b,h
      pop hl
      call _ldir_rom_to_vram
.else
      add a, $17            ; a = x+8
      ld c, a               ; that's our count
      ldi
      ldi
      ldi
      ldi
      ldi
      ldi
      ldi
      ldir                  ; copy c bytes total, but allow range 8..263
.endif
      jp _byte_stream_next_byte

_0f:
      ld a, (hl)            ; read byte
      inc hl
      inc a                 ; $ff -> double-byte length
      jr z, _0fff
.ifdef MicroMachinesDecompressToVRAM
      ; We want to copy a+29 bytes
      push hl
        ld h,0
        ld l,a
        ld bc,29
        add hl,bc
        ld c,l
        ld b,h
      pop hl
      call _ldir_rom_to_vram
      jp _byte_stream_next_byte

_0fff:
      ld c, (hl)
      inc hl
      ld b, (hl)
      inc hl
      call _ldir_rom_to_vram
      jp _byte_stream_next_byte

.else      
      add a, $1D            ; add 29
      ld c, a
      ld a, $08
      jr nc, _f             ; deal with >255
      inc b
__:   ldi                   ; copy 8 bytes at a time until bc <= 8
      ldi                   ; This allows us to have bc in the range 9..65544 but that can't actually happen?
      ldi
      ldi
      ldi
      ldi
      ldi
      ldi
      cp c                  ; compare a to c
      jr c, _b
      dec b
      inc b
      jr nz, _b
      ldir
      jp _byte_stream_next_byte

_0fff:
      ld c, (hl)
      inc hl
      ld b, (hl)
      inc hl
      ld a, $08
      jr _b
.endif

.ifdef MicroMachinesDecompressToVRAM
_ldi_rom_to_vram:
  push af
    ; de++=hl++
    ld a,(hl)
    out ($be),a
    inc hl
    inc de
    ; We do not dec bc
  pop af
  ret

_ldir_rom_to_vram:
  ; copy bc bytes from hl (ROM) to de (VRAM)
  ; leaves hl, de increased by bc at the end
  ; leaves bc=0 at the end
  push af
  
    ; add bc to de to mimic ldir
    ex de,hl
      add hl,bc
    ex de,hl

    ; If b = 0, we can short-circuit. We optimise for this as we expect mostly short runs.
    ld a,b
    or a
    jr nz,+ ; unlikely

--: ld b,c
    ld c,$be
    otir
---:
  pop af
  ret

+:  push bc
      ; We copy b*256 bytes
      ld c,$be
      ld b,0
-:    otir
      dec a
      jp nz,-
    pop bc
    ; If we have remainder, process it
    ld a,c
    or a
    jr z,--- ; unlikely
    jp --

_ldir_vram_to_vram:
  ; Copy bc bytes from VRAM address hl to VRAM address de
  ; Both hl and de are "write" addresses ($4xxx)
  ; Make hl a read address
  res 6, h
  ; Check if the count is below 256
  ld a,b
  or a
  jp z,_below256 ; likely
  ; Else emit 256*b bytes
-:push bc
    ld b,0
    call +
  pop bc
  djnz -
  ; Then fall through for the rest  
_below256:
  ; By emitting <=256 at a time, we can use the out (c),r opcode
  ; for address setting, which then relieves pressure on a
  ; and saves some push/pops; and we can use djnz for the loop.
  ld b,c
--:
+:ld c,$bf
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

_ldir_vram_to_vram_b:
  ; Make hl a read address
  res 6, h
  jr --
.endif
; end of decompress code
