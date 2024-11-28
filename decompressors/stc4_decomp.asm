;~ STC4 simple tile compressor four (based on STC0)
;~   [by sverx, 28/11/2022]
;~
;~ uses 4 bytes RAM buffer
;~
;~ one byte holds information on how to create a group of 4 bytes, providing two bits for each:
;~
;~     D7 D6          D5 D4            D3 D2          D1 D0
;~
;~    1st byte       2nd byte         3rd byte       4th byte
;~
;~ 0b10 -> byte has value 0x00
;~ 0b11 -> byte has value 0xFF
;~ 0b01 -> byte is a different value, uncompressed byte follows
;~ 0b00 -> repeat last uncompressed value from same group of 4 bytes
;~
;~
;~ note: if D7,D6 are both zero it means this whole group of 4 bytes is based on the previous group so:
;~
;~       if D5 = 0 then we need to repeat the previous group again, up to 31 times (counter is value in D4-D0) or EOD if counter is 0
;~
;~       else if D5 = 1 then we need to use the last group of 4 bytes replacing each value that has a 1 in D3-D0 with a raw value that follows (up to 3)
;~          [ and if D4 is 1 then it means bytes from the previous group of 4 bytes needs to be complemented, so for instance a value of $02 becomes $FD and viceversa ]
;~
;~  IN:
;~      HL (data source address)
;~      DE (destination in VRAM w/flags)
;~  clobbers:
;~      AF,BC,DE,HL
;~


; comment the following line to create a faster but VRAM unsafe version of this algorithm
.define VRAM_SAFE

.define VDP_CTRL_PORT    $bf
.define VDP_DATA_PORT    $be

.ramsection "stc4_buffer" slot 3 free
  stc4_buffer dsb 4                   ; the 4 bytes buffer for the decompression
.ends

.section "stc4_decompress" free
stc4_decompress:
  ld c,VDP_CTRL_PORT                  ; set VRAM destination address
  di
  out (c),e
  out (c),d
  ei

_stc4_decompress_outer_loop:
  ld a,(hl)
  cp $20                              ; if value less than $20 it's a rerun or an end-of-data marker
  jp c,_reruns_or_leave               ; too far for JR

  ld de,stc4_buffer

; ************************************
.rept 4 index COUNT
_bitmask_{COUNT}:
  rla
  jr c,@_compressed_00_or_FF          ; if 1X found, write $00 or $FF
  rla
.if COUNT == 0
  jp nc,_diff                         ; if 00 found on D7,D6 it's a diff for sure as reruns have aleady been addressed (also too far for JR)
.else
  jr nc,@_same_byte                   ; if 00 found NOT on D7,D6 it's a same raw byte as previous one
.endif
.if COUNT != 3
  ld b,a                              ; preserve A
.endif
  inc hl
  ld a,(hl)
  out (VDP_DATA_PORT),a               ; write byte to VRAM
  ld (de),a                           ; and to buffer too
.if COUNT != 3
  ld a,b                              ; restore A
.endif
  jp @_next_bitmask

@_compressed_00_or_FF:
  rla
.if COUNT != 3
  ld b,a                              ; preserve A
.endif
  sbc a                               ; this turns the CF into $00 or $FF
  out (VDP_DATA_PORT),a               ; write $00 or $FF to VRAM
  ld (de),a                           ; and to buffer too
.if COUNT != 3
  ld a,b                              ; restore A
.endif
.if COUNT != 0
  jp @_next_bitmask
.endif

.if COUNT != 0
@_same_byte:
.if COUNT != 3
  ld b,a                              ; preserve A
.endif
  ld a,(hl)                           ; we won't [inc hl] here because we're loading the same value as before so we are already on that
  out (VDP_DATA_PORT),a               ; write byte to VRAM
  ld (de),a                           ; and to buffer too
.if COUNT != 3
  ld a,b                              ; restore A
.endif
.endif

@_next_bitmask
.if COUNT != 3
  inc de                              ; advance buffer pointer
.endif
.endr
  inc hl
  jp _stc4_decompress_outer_loop

; ************************************
_diff:
  rla                                 ; skip D5
  ld c,a                              ; save D4 in C's MSB
  rla                                 ; skip D4

.rept 4 index COUNT
_diff_bit_{COUNT}:
  rla
.if COUNT != 3
  ld b,a                              ; preserve A
.endif
  jr c,@_diff
  ld a,(de)
  bit 7,c
  jr z,+
  cpl                                 ; invert data if D4 was set
  ld (de),a                           ; and save it back into the buffer
+:
  out (VDP_DATA_PORT),a               ; write byte from buffer to VRAM
  jp @next_bit
@_diff:
  inc hl
  ld a,(hl)
  out (VDP_DATA_PORT),a               ; write byte from stream to VRAM
  ld (de),a                           ; and to buffer too
@next_bit:
.if COUNT != 3
  inc de
  ld a,b                              ; restore A
.endif
.endr
  inc hl
  jp _stc4_decompress_outer_loop

; ************************************
_reruns_or_leave:
  and $1F
  ret z                               ; if value is zero, the EOD marker has been found, so leave

  ld b,a                              ; save reruns counter in B

_transfer_whole_buffer_B_times:
  ld de,stc4_buffer
.rept 4 index COUNT
  ld a,(de)                           ; 7
  out (VDP_DATA_PORT),a               ; 11
.if COUNT != 3
.ifdef VRAM_SAFE
  nop                                 ; 4
.endif
  inc de                              ; 6  = 28
.endif
.endr
  djnz _transfer_whole_buffer_B_times
  inc hl
  jp _stc4_decompress_outer_loop
.ends

.undef VDP_CTRL_PORT
.undef VDP_DATA_PORT
