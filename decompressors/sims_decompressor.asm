; Format:
; %0nnnnnnn %NNNNcccc   LZ copy: distance nN (0..2047) count c+2 (2..17)
; %10nnnnnn $dd...      Raw copy: n+1 bytes (1..64)
; %110llnnn $dd...      Short semi-RLE: n+2 repetitions (2..9) of length l+1 (1..4)
; %111llnnn $NN $dd...  Long semi-RLE: nN+2 repetitions (2..2047) of length l+1 (1..4)

.enum SIMSDecompressorMemory
  LZ_WINDOW            dsb 2048 ; Lookback for LZ
  LZ_WINDOW_CURSOR     dw ; Index into it
  COMPRESSED_DATA_END  dw ; Pointer to end of input data
  LZ_COPY_LENGTH       db ; Another counter
  LZ_COPY_TEMP_BUFFER  dsb 17 ; LZ runs are put there before being appended to the window
.ende

SIMSDecompress:
  ; Parameters:
  ; de = VRAM address, with write bit set
  ; hl = pointer to data

  ; Initialise window to zeros
  exx
    xor a
    ld hl, LZ_WINDOW
    ld bc, $0820 ; 2080 = all the memory and then some
    dec bc
    ld (hl), a
    ld d, h
    ld e, l
    inc de
    ldir
  exx

  ; Set VRAM address
  ld c, $bf
  out (c), e
  out (c), d
  ; Read the data size
  ld c, (hl)
  inc hl
  ld b, (hl)
  inc hl
  ; Compute the end address
  add hl, bc
  ld (COMPRESSED_DATA_END), hl
  ; Restore HL
  or a
  sbc hl, bc

DECOMPRESSOR_LOOP:
  ; Check if we got to the end
  ld bc, (COMPRESSED_DATA_END)
  or a
  sbc hl, bc
  ret nc
  add hl, bc
  ; If not, do a chunk
  call _CHECK_DECOMPRESSION_TYPE
  jp DECOMPRESSOR_LOOP


_CHECK_DECOMPRESSION_TYPE:
  ; Read the next byte
  ld a, (hl)
  inc hl
  ; Dispatch based on the upper bits
  bit 7, a
  jp z, _LZ           ; %0------- = LZ
  bit 6, a
  jp z, _RAW          ; %10------ = raw
  push af
    bit 5, a
    jr z, _RLE_SHORT  ; %110----- = few repeats

_RLE_LONG:
    ; Else it is      ; %111----- = many repeats
    ; Mask to the high distance bits, put them in b
    and %111
    ld b, a
    ; Next byte is additional distance bits, put them in c
    ld a, (hl)
    inc hl
    ld c, a
    
    ; Then jump into the RLE copy code
    jp +

_RLE_SHORT:
    ; a is the distance and repeat length
    ; Mask to the distance bits, put them in bc
    and %111
    ld c, a
    ld b, 0
    ; fall through
+:  ; bc += 2
    inc bc
    inc bc
  pop af ; restore command byte
  
  ; Repeat length is in bits 3-4, minus 1
  .repeat 3
    rra
  .endr
  and %11
  inc a
  ld e, a
  
  ; Now do the copy
--:
  push bc ; repeat count
  push hl ; data pointer
    ld d, e ; run length
-:  ; Copy a byte from (hl++)
    ld a, (hl)
    out ($be), a
    call _STORE_IN_LZ_WINDOW ; also increments hl
    dec d
    jr nz, - ; repeat for the run length (e)
  pop hl
  pop bc
  dec bc ; repeat for the required count of repeats
  ld a, b
  or c
  jr nz, --
  ; Finally, move hl on by the run length
  ld d, 0
  add hl, de
  ret

_LZ:
  ; Distance = low 7 bits in a followed by high 4 bits in (hl)
  ; Compute that in bc
  push hl
    ld l, a
    ld h, 0
    .repeat 4
      add hl, hl
    .endr
    ld b, h
    ld c, l
  pop hl
  ; OR high 4 bits of next byte into c
  ld a, (hl)
  .repeat 4
    rra
  .endr
  and $f
  or c
  ld c, a
  
  ; Now subtract that from LZ_WINDOW_CURSOR, wrapping at 2K so the buffer acts circularly.
  ; This is still an offset relative to the start of LZ_WINDOW.
  push hl
    ld hl, (LZ_WINDOW_CURSOR)
    or a
    sbc hl, bc
    ld a, h
    and $07
    ld b, a
    ld c, l
  pop hl

  ; Re-read byte and get the length (minus 2) in the low 4 bits
  ld a, (hl)
  inc hl
  and $0F
  add a, 2
  ld (LZ_COPY_LENGTH), a

  push hl ; preserve read address
    push af ; preserve LZ run length
      push de
        ; We 
        ld de, LZ_COPY_TEMP_BUFFER
-:      ; Compute hl = LZ_WINDOW + bc
        ld hl, LZ_WINDOW
        add hl, bc
        ; Copy a byte from there to LZ_COPY_TEMP_BUFFER and to VRAM
        ld a, (hl)
        ld (de), a
        out ($be), a
        ; Increment dest and index
        inc de
        inc bc
        ; Wrap bc at 2K
        ld a, b
        and $07
        ld b, a
        ; Decrement the counter
        ld hl, LZ_COPY_LENGTH
        dec (hl)
        ; Loop over the run length
        jr nz, -
      pop hl ; Unmatched pop: seemingly doesn't matter? Can be removed?
    pop af
    ; Then copy all the bytes from LZ_COPY_TEMP_BUFFER into the window - only after the copy is done
    ld hl, LZ_COPY_TEMP_BUFFER
    ld d, a
-:  call _STORE_IN_LZ_WINDOW ; also increments hl
    dec d
    jr nz, - ; djnz would be faster
  pop hl
  ret

_RAW:
  ; Low 6 bits of a = length - 1
  ; Copy bytes from 
  and $3F
  inc a
  ld d, a
-:; Copy a byte to VRAM
  ld a, (hl)
  out ($be), a
  ; And remember it
  call _STORE_IN_LZ_WINDOW ; also increments hl
  dec d ; djnz would be faster
  jr nz, -
  ret

_STORE_IN_LZ_WINDOW:
  ; Copies (hl++) to the "window", wrapping at 2K
  ; Get byte
  ld a, (hl)
  inc hl
  push hl
    ; Write to LZ_WINDOW[LZ_WINDOW_CURSOR]
    ld hl, LZ_WINDOW
    ld bc, (LZ_WINDOW_CURSOR)
    add hl, bc
    ld (hl), a
    ; Increment and wrap offset
    inc bc
    res 3, b
    ;  And store for next time
    ld (LZ_WINDOW_CURSOR), bc
  pop hl
  ret
        