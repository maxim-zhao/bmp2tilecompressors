;~
;~ STC0 simple tile compression zero (uses no RAM)
;~
;~ one byte holds information on how to create the next 4 bytes, two bits for each:
;~
;~     D7 D6          D5 D4            D3 D2          D1 D0
;~
;~    1st byte       2nd byte         3rd byte       4th byte
;~
;~
;~ 0b10 -> write value 0x00
;~ 0b11 -> value 0xFF
;~ 0b01 -> uncompressed (data byte follows)
;~ 0b00 -> same uncompressed byte as the previous one (in this set of 4) OR end of stream if this is the first two bits (setting the whole byte to 0x00 works fine as EOD)
;~

;~  IN:
;~      HL (data source address)
;~      DE (destination in VRAM w/flags)
;~  clobbers:
;~      AF,BC,DE,HL
;~

stc0_decompress:
  ld c,$bf                            ; VDP_CTRL_PORT
  di
  out (c),e
  out (c),d
  ei
  dec c                               ; same as ld c,VDP_DATA_PORT_ADDRESS
  ld de,$00FF                         ; D = $00, E = $FF

_stc0_decompress_outer_loop:
  ld b,4
  ld a,(hl)
  inc hl

_stc0_decompress_inner_loop:
  rla
  jr c,_compressed_byte               ; if 1X found, write $00 or $FF
  rla
  jr nc,_same_byte_or_leave
  outi                                ; write raw byte
  jr nz,_stc0_decompress_inner_loop
  jp _stc0_decompress_outer_loop

_same_byte_or_leave:
  bit 2,b
  ret nz                              ; 00 found in first 2 bits -> end of data, leave
  dec hl
  outi                                ; write same raw byte again
  jr nz,_stc0_decompress_inner_loop
  jp _stc0_decompress_outer_loop

_compressed_byte:
  rla
  jr c,_compressed_FF
  out (c),d                           ; write $00
  djnz _stc0_decompress_inner_loop
  jp _stc0_decompress_outer_loop

_compressed_FF:
  out (c),e                           ; write $FF
  djnz _stc0_decompress_inner_loop
  jp _stc0_decompress_outer_loop

