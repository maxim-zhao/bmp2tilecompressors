; -----------------------------------------------------------------------------
; ZX0 decoder by Einar Saukas
; "Standard" version (69 bytes only)
; Modified by Maxim (20210817) to support decompressing to SMS VRAM
; - Inlining the ldir replacements may save a few cycles
; -----------------------------------------------------------------------------
; Parameters:
;   HL: source address (compressed data)
;   DE: destination address (decompressing)
; -----------------------------------------------------------------------------

dzx0_standard:
.ifdef ZX0ToVRAM
  ; Set VRAM address
  ld c,$bf
  out (c),e
  out (c),d
.endif
        ld      bc, $ffff               ; preserve default offset 1
        push    bc
        inc     bc
        ld      a, $80
dzx0s_literals:
        call    dzx0s_elias             ; obtain length
.ifdef ZX0ToVRAM
        call _ldir_rom_to_vram
.else
        ldir                            ; copy literals
.endif
        add     a, a                    ; copy from last offset or new offset?
        jr      c, dzx0s_new_offset
        call    dzx0s_elias             ; obtain length
dzx0s_copy:
        ex      (sp), hl                ; preserve source, restore offset
        push    hl                      ; preserve offset
        add     hl, de                  ; calculate destination - offset
.ifdef ZX0ToVRAM
        call _ldir_vram_to_vram
.else
        ldir                            ; copy from offset
.endif
        pop     hl                      ; restore offset
        ex      (sp), hl                ; preserve offset, restore source
        add     a, a                    ; copy from literals or new offset?
        jr      nc, dzx0s_literals
dzx0s_new_offset:
        call    dzx0s_elias             ; obtain offset MSB
        ex      af, af'
        pop     af                      ; discard last offset
        xor     a                       ; adjust for negative offset
        sub     c
        ret     z                       ; check end marker
        ld      b, a
        ex      af, af'
        ld      c, (hl)                 ; obtain offset LSB
        inc     hl
        rr      b                       ; last offset bit becomes first length bit
        rr      c
        push    bc                      ; preserve new offset
        ld      bc, 1                   ; obtain length
        call    nc, dzx0s_elias_backtrack
        inc     bc
        jr      dzx0s_copy
dzx0s_elias:
        inc     c                       ; interlaced Elias gamma coding
dzx0s_elias_loop:
        add     a, a
        jr      nz, dzx0s_elias_skip
        ld      a, (hl)                 ; load another group of 8 bits
        inc     hl
        rla
dzx0s_elias_skip:
        ret     c
dzx0s_elias_backtrack:
        add     a, a
        rl      c
        rl      b
        jr      dzx0s_elias_loop
; -----------------------------------------------------------------------------


.ifdef ZX0ToVRAM
_ldir_vram_to_vram:
  ; Copy bc bytes from VRAM address hl to VRAM address de
  ; Both hl and de are "write" addresses ($4xxx)
  push af
  ; Make hl a read address
  ld a,h
  xor $40
  ld h,a
  ; Check if the count is below 256
  ld a,b
  or a
  jr z,_below256
  ; Else emit 256*b bytes
-:push bc
    ld c,$bf
    ld b,0
    call +
  pop bc
  djnz -
  ; Then fall through for the rest  
_below256:
  ; By emitting 256 at a time, we can use the out (c),r opcode
  ; for address setting, which then relieves pressure on a
  ; and saves some push/pops; and we can use djnz for the loop.
  ld b,c
  ld c,$bf
+:
-:out (c),l
  out (c),h
  in a,($be)
  out (c),e
  out (c),d
  out ($be),a
  inc hl
  inc de
  djnz -
  ld c,0
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
  ld c,0
  ret

+:  push bc
      ; We copy b*256 bytes
      ld c,$be
      ld b,0
-:    otir
      inc d ; move de on by 256
      dec a
      jp nz,-
    pop bc
    ; If we have remainder, process it
    ld a,c
    or a
    jr z,--- ; unlikely
    jp --

.endif
