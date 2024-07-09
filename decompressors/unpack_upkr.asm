;; https://github.com/exoticorn/upkr/blob/z80/c_unpacker/unpack.c - original C implementation
;; C source in comments ahead of asm - the C macros are removed to keep only bitstream variant
;;
;; initial version by Peter "Ped" Helcmanovsky (C) 2022, licensed same as upkr project ("unlicensed")
;; to assemble use z00m's sjasmplus: https://github.com/z00m128/sjasmplus
;;
;; you can define UPKR_PROBS_ORIGIN to specific 256 byte aligned address for probs array (320 bytes),
;; otherwise it will be positioned after the unpacker code (256 aligned)
;;
;; public API:
;;
;;     upkr.unpack
;;         IN: IX = packed data, DE' (shadow DE) = destination
;;         OUT: IX = after packed data
;;         modifies: all registers except IY, requires 10 bytes of stack space
;;

; Modifications 2024 by Maxim for compatibility with WLA DX, ROM and VRAM
; Define upkr_memory for a 321 bytes region of memory to use. Must be aligned to 256 bytes.
.enum upkr_memory
	probs dsb 320
	upkr_offset	db
.ende

.define NUMBER_BITS 16+15       ; context-bits per offset/length (16+15 for 16bit offsets/pointers)
    ; numbers (offsets/lengths) are encoded like: 1a1b1c1d1e0 = 0000'0000'001e'dbca

; IN: IX = compressed_data, DE' = destination
unpack:
	; Set VRAM write address
	exx
		ld a,e
		out ($bf),a
		ld a,d
		out ($bf),a
	exx

  ; ** reset probs to 0x80, also reset HL (state) to zero, and set BC to probs+context 0
    ld      hl,probs_c>>1
    ld      bc,probs_e
    ld      a,$80
_reset_probs:
    dec     bc
    ld      (bc),a              ; will overwrite one extra byte after the array because of odd length
    dec     bc
    ld      (bc),a
    dec     l
    jr      nz,_reset_probs
    ex af,af'
    ; BC = probs (context_index 0), state HL = 0, A' = 0x80 (no source bits left in upkr_current_byte)

  ; ** main loop to decompress data
    ; D = prev_was_match = uninitialised, literal is expected first => will reset D to "false"
    ; values for false/true of prev_was_match are: false = high(probs), true = 1 + high(probs)
_decompress_data:
    ld      c,0
    call    _decode_bit          ; if(upkr_decode_bit(0))
    jr      c,_copy_chunk

  ; * extract byte from compressed data (literal)
    inc     c                   ; C = byte = 1 (and also context_index)
_decode_byte:
    call    _decode_bit          ; bit = upkr_decode_bit(byte);
    rl      c                   ; byte = (byte << 1) + bit;
    jr      nc,_decode_byte     ; while(byte < 256)
    ld      a,c
    exx
.ifdef upkr_vram
	out ($be),a
.else
    ld      (de),a              ; *write_ptr++ = byte;
.endif
	inc de
    exx
    ld      d,b                 ; prev_was_match = false
    jr      _decompress_data

  ; * copy chunk of already decompressed data (match)
_copy_chunk:
    ld      a,b
    inc     b                   ; context_index = 256
        ;             if(prev_was_match || upkr_decode_bit(256)) {
        ;                 offset = upkr_decode_length(257) - 1;
        ;                 if (0 == offset) break;
        ;             }
    cp      d                   ; CF = prev_was_match
    call    nc,_decode_bit       ; if not prev_was_match, then upkr_decode_bit(256)
    jr      nc,_keep_offset     ; if neither, keep old offset
    call    _decode_number       ; context_index is already 257-1 as needed by decode_number
    dec     de                  ; offset = upkr_decode_length(257) - 1;
    ld      a,d
    or      e
    ret     z                   ; if(offset == 0) break
    ld      (upkr_offset),de
_keep_offset:
        ;             int length = upkr_decode_length(257 + 64);
        ;             while(length--) {
        ;                 *write_ptr = write_ptr[-offset];
        ;                 ++write_ptr;
        ;             }
        ;             prev_was_match = 1;
    ld      c,(257 + NUMBER_BITS - 1)&0xff ;low(257 + NUMBER_BITS - 1)    ; context_index to second "number" set for lengths decoding
    call    _decode_number       ; length = upkr_decode_length(257 + 64);
    push    de
    exx
        ; forward unpack (write_ptr++, upkr_data_ptr++)
        ld      h,d             ; DE = write_ptr
        ld      l,e
		ld  bc,(upkr_offset)
        sbc     hl,bc           ; CF=0 from decode_number ; HL = write_ptr - offset
        pop     bc              ; BC = length
.ifdef upkr_vram
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
    exx
    ld      d,b                 ; prev_was_match = true
    djnz    _decompress_data    ; adjust context_index back to 0..255 range, go to main loop

_inc_c_decode_bit:
  ; ++low(context_index) before decode_bit (to get -1B by two calls in decode_number)
    inc     c
_decode_bit:
  ; HL = upkr_state
  ; IX = upkr_data_ptr
  ; BC = probs+context_index
  ; A' = upkr_current_byte (!!! init to 0x80 at start, not 0x00)
  ; preserves DE
  ; ** while (state < 32768) - initial check
    push    de
    bit     7,h
    jr      nz,_state_b15_set
    ex af,af'
  ; ** while body
_state_b15_zero:
  ; HL = upkr_state
  ; IX = upkr_data_ptr
  ; A = upkr_current_byte (init to 0x80 at start, not 0x00)
    add     a,a                     ; upkr_current_byte <<= 1; // and testing if(upkr_bits_left == 0)
    jr      nz,_has_bit             ; CF=data, ZF=0 -> some bits + stop bit still available
  ; CF=1 (by stop bit)
    ld      a,(ix+0)
	inc ix
    adc     a,a                     ; CF=data, b0=1 as new stop bit
_has_bit:
    adc     hl,hl                   ; upkr_state = (upkr_state << 1) + (upkr_current_byte >> 7);
    jp      p,_state_b15_zero       ; while (state < 32768)
    ex af,af'
  ; ** set "bit"
_state_b15_set:
    ld      a,(bc)                  ; A = upkr_probs[context_index]
    dec     a                       ; prob is in ~7..249 range, never zero, safe to -1
    cp      l                       ; CF = bit = prob-1 < (upkr_state & 255) <=> prob <= (upkr_state & 255)
    inc     a
  ; ** adjust state
    push    bc
    ld      c,l                     ; C = (upkr_state & 255); (preserving the value)
    push    af
    jr      nc,_bit_is_0
    neg                             ; A = -prob == (256-prob), CF=1 preserved
_bit_is_0:
    ld      d,0
    ld      e,a                     ; DE = state_scale ; prob || (256-prob)
    ld      l,d                     ; H:L = (upkr_state>>8) : 0

.IFNDEF UPKR_UNPACK_SPEED

    ;; looped MUL for minimum unpack size
    ld      b,8                     ; counter
_mulLoop:
    add     hl,hl
    jr      nc,_mul0
    add     hl,de
_mul0:
    djnz    _mulLoop                ; until HL = state_scale * (upkr_state>>8), also BC becomes (upkr_state & 255)

.ELSE

    ;;; unrolled MUL for better performance, +25 bytes unpack size
    ld      b,d
    .repeat 8
        add     hl,hl
        jr      nc,+
        add     hl,de
+:
    .endr

.ENDIF

    add     hl,bc                   ; HL = state_scale * (upkr_state >> 8) + (upkr_state & 255)
    pop     af                      ; restore prob and CF=bit
    jr      nc,_bit_is_0_2
    dec     d                       ; DE = -prob (also D = bit ? $FF : $00)
    add     hl,de                   ; HL += -prob
    ; ^ this always preserves CF=1, because (state>>8) >= 128, state_scale: 7..250, prob: 7..250,
    ; so 7*128 > 250 and thus edge case `ADD hl=(7*128+0),de=(-250)` => CF=1
_bit_is_0_2:
 ; *** adjust probs[context_index]
    rra                             ; + (bit<<4) ; part of -prob_offset, needs another -16
    and     $FC                     ; clear/keep correct bits to get desired (prob>>4) + extras, CF=0
    rra
    rra
    rra                             ; A = (bit<<4) + (prob>>4), CF=(prob & 8)
    adc     a,-16                   ; A = (bit<<4) - 16 + ((prob + 8)>>4) ; -prob_offset = (bit<<4) - 16
    ld      e,a
    pop     bc
    ld      a,(bc)                  ; A = prob (cheaper + shorter to re-read again from memory)
    sub     e                       ; A = 16 - (bit<<4) + prob - ((prob + 8)>>4) ; = prob_offset + prob - ((prob + 8)>>4)
    ld      (bc),a                  ; probs[context_index] = prob_offset + prob - ((prob + 8) >> 4);
    add     a,d                     ; restore CF = bit (D = bit ? $FF : $00 && A > 0)
    pop     de
    ret

_decode_number:
  ; HL = upkr_state
  ; IX = upkr_data_ptr
  ; BC = probs+context_index-1
  ; A' = upkr_current_byte (!!! init to 0x80 at start, not 0x00)
  ; return length in DE, CF=0
    ld      de,$FFFF            ; length = 0 with positional-stop-bit
    or      a                   ; CF=0 to skip getting data bit and use only `rr d : rr e` to fix init DE
_loop:
    call    c,_inc_c_decode_bit  ; get data bit, context_index + 1 / if CF=0 just add stop bit into DE init
    rr      d
    rr      e                   ; DE = length = (length >> 1) | (bit << 15);
    call    _inc_c_decode_bit    ; context_index += 2
    jr      c,_loop
_fix_bit_pos:
    ccf                         ; NC will become this final `| (1 << bit_pos)` bit
    rr      d
    rr      e
    jr      c,_fix_bit_pos      ; until stop bit is reached (all bits did land to correct position)
    ret                         ; return with CF=0 (important for unpack routine)

;    DISPLAY "upkr.unpack total size: ",/D,$-unpack

    ; reserve space for probs array without emitting any machine code (using only EQU)

.define real_c 1 + 255 + 1 + 2*NUMBER_BITS             ; real size of probs array
.define probs_c (real_c + 1) & -2                      ; padding to even size (required by init code)
.define probs_e probs + probs_c

;    DISPLAY "upkr.unpack probs array placed at: ",/A,probs,",\tsize: ",/A,probs.c

.ifdef upkr_vram
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
