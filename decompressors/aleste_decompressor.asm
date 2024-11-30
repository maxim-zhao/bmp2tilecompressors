; Format:
; %00000000     End
; %0nnnnnnn     n bytes of raw data
; %1nnnnnnn $oo n+3 bytes of LZ data from offset o+1, may overlap itself

.enum aleste_decompress_memory ; Must be 256-aligned! 258 bytes used
aleste_decompress_buffer  dsb 256
aleste_decompress_pointer dw
.ende

_lz: 
  inc hl
  ; Count = (a & $7f) + 3
  and $7F
  add a, $03
  ld b, a
  ld de, (aleste_decompress_pointer) ; Get VRAM address
  ld a, e
  dec a
  sub (hl) ; Next byte is the offset minus 1
  inc hl
  push hl
    ; point to history buffer
    ld l, a
    ld h, >aleste_decompress_buffer
    ld c, $bf
.ifdef aleste_decompress_interrupt_safe
    di
.endif
      ; Set VRAM address
      out (c), e
      out (c), d
      ld c, d
      ld d, >aleste_decompress_buffer
-:    ld a, (hl)
      out ($be), a
      ld (de), a
      inc l
      inc e
      jp nz, +
      inc c
+:    djnz - ; Emit bytes from history window
.ifdef aleste_decompress_interrupt_safe
    ei
.endif
    ld d, c ; Save VRAM address
    ld (aleste_decompress_pointer), de
  pop hl
  ; Fall through

aleste_decompress:
; de = VRAM address
; hl = data
  ld (aleste_decompress_pointer), de
  ld a, (hl) ; Read byte
  and a
  jp m, _lz ; High bit = LZ
  ret z ; 0 = end

_raw:
  ; Emit a bytes of raw data
  inc hl
  ld b, a
.ifdef aleste_decompress_interrupt_safe
  di
.endif
    ld de, (aleste_decompress_pointer) ; Read dest address
    ld c, $bf
    out (c), e
    out (c), d
    ld c, d
    ld d, >aleste_decompress_buffer ; Then convert to a RAM address
-:  ld a, (hl)  ; Copy byte to VRAM
    out ($be), a
    inc hl
    ld (de), a ; And buffer
    inc e
    jp nz, +
    inc c
+:  djnz -
.ifdef aleste_decompress_interrupt_safe
  ei
.endif
  ; Update VRAM address in RAM
  ld d, c
  ld (aleste_decompress_pointer), de
  jp aleste_decompress
 