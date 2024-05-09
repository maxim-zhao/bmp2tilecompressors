
;-----------------------------------------------------------------
;           Master of Darknesss - Decompression Routine
;-----------------------------------------------------------------

.enum SIMSDecompressorMemory
  LZ_WINDOW            dsb 2048 ; Lookback for LZ
  LZ_WINDOW_CURSOR     dw ; Pointer into it
  COMPRESSED_DATA_SIZE dw ; Pointer to end of input data
  LZ_COPY_LENGTH       db ; Another counter
  LZ_COPY_TEMP_BUFFER  dsb 17 ; LZ runs are put there before being appended to the window
.ende

SIMSDecompress:
; Parameters:
; de = VRAM address, with write bit set
; hl = pointer to data
        ; Initialise window
        ex af, af'
        exx
          xor a
          ld hl, LZ_WINDOW
          ld bc, $0820
          dec bc
          ld (hl), a
          ld d, h
          ld e, l
          inc de
          ldir
        exx
        ex af, af'

        ld c, $bf
        out (c), e
        out (c), d
        ; Read DATA SIZE, first 2 bytes from COMPRESSED DATA
        ld c, (hl)
        inc hl
        ld b, (hl)
        inc hl
        add hl, bc
        ld (COMPRESSED_DATA_SIZE), hl
        or a
        sbc hl, bc

DECOMPRESSOR_LOOP:
        ld bc, (COMPRESSED_DATA_SIZE)
        or a
        sbc hl, bc
        ret nc
        add hl, bc
        call _CHECK_DECOMPRESSION_TYPE
        jp DECOMPRESSOR_LOOP


; CHECK DECOMPRESSION TYPE
; BIT 7 = LZ
; BIT 6 = RAW
; BIT 5 = RLE COPY
; NO BIT = RLE COPY DEFAULT REPETIONS
_CHECK_DECOMPRESSION_TYPE:
        ld a, (hl)                 ; Load byte from HL (CURSOR) in A
        inc hl                     ; Increment HL (CURSOR)

        bit 7, a                   ; Test 7th bit of A
        jp z, _LZ_COPY             ; If set do LZ COPY
        bit 6, a                   ; Test 6th bit of A
        jp z, _RAW_COPY            ; If set do RAW COPY
        push af                    
        bit 5, a                   ; Test 5th bit of A
        jr z, _RLE_COPY            ; If set do RLE COPY

                                   ; GET RLE REPETITIONS
                                   ; -------------------------------
                                   ; If all bit test fails
        and $07                    ; Get first 3 bits of Byte
        ld b, a
        ld a, (hl)                 ; Load next byte from HL (CURSOR) in A
        inc hl                     ; Increment HL (CURSOR)
        ld c, a

        jp _RLE_COPY_GET_LENGTH    ; Skip default GET RLE REPETITIONS
                                   ; and GET RLE LENGTH

; Performs RLE Copy
; BC  = REPETITIONS
; E  = LENGTH
_RLE_COPY:

                                    ; GET RLE REPETITIONS
                                    ; -------------------------------
        and $07                     ; Get first 3 bits of Byte
        ld c, a
        ld b, $00
_RLE_COPY_GET_LENGTH:
        inc bc                      ; Increment REPETITIONS
        inc bc                      ; Increment REPETITIONS
        pop af

                                    ; GET RLE LENGTH
                                    ; -------------------------------
        rra                         ; Bitshift a >> 3
        rra
        rra
        and $03                     ; Get first 2 bits of byte
        inc a                       ; Increment LENGTH
        ld e, a
--:                                 ;                          <<<<<<<<<<<<|
        push bc                     ;                                      |
        push hl                     ;                                      |
        ld d, e                     ;                                      |
                                    ;                                      |
                                    ;                                      |
-:                                  ;                            <<<<<<<<| |
        ld a, (hl)                  ; Load Byte from CURSOR (HL) in A    | |
        out ($be), a                ; Copy Byte to VRAM                  | |
        call _STORE_IN_LZ_WINDOW    ; Copy Byte do LZ_WINDOW             | |
        dec d                       ; Decrement D (LENGTH)               | |
        jr nz, -                    ; If NOT ZERO loop again     >>>>>>>>| |
                                    ;                                      |
        pop hl                      ;                                      |
        pop bc                      ;                                      |
        dec bc                      ; Decrement BC (REPETITIONS)           |
        ld a, b                     ;                                      |
        or c                        ;                                      |
        jr nz, --                   ; If NOT ZERO loop again   >>>>>>>>>>>>|
        ld d, $00
        add hl, de                  ; Add DE (LENGTH) to HL (CURSOR POSITION)
        ret

; PERFORMS LZ COPY
_LZ_COPY:
        push hl

                                    ; GET DISTANCE FROM LZ PAIR
                                    ; -------------------------------
        ld l, a
        ld h, $00
        add hl, hl                  ; Bitshift HL << 4
        add hl, hl
        add hl, hl
        add hl, hl
        ld b, h
        ld c, l

        pop hl

                                    ; GET DISTANCE IN LZ PAIR
                                    ; -------------------------------
        ld a, (hl)
        rra                         ; Bitshift A >> 4
        rra
        rra
        rra

        and $0F                     ; Get first 4 bits from Byte
        or c                        ; or C
        ld c, a
        push hl
        ld hl, (LZ_WINDOW_CURSOR)   ; Store LZ_WINDOW CURSOR position in HL
        or a                        ; or A
        sbc hl, bc                  ; Subtract BC (LZ_WINDOW CURSOR) from HL (DISTANCE)
        ld a, h
        and $07                     ; Get first 3 bits of A
        ld b, a
        ld c, l
        pop hl
                                    ; GET LENGTH IN LZ PAIR
                                    ; -------------------------------
        ld a, (hl)                  ; Load next byte from CURSOR (HL) in A
        inc hl                      ; Increment (HL) CURSOR
        and $0F                     ; Get first 4 bits of A (LENGTH)
        add a, $02                  ; Add 2 to A
        ld (LZ_COPY_LENGTH), a      ; Store A in LZ_COPY_LENGTH
        push hl
        push af
        push de
        ld de, LZ_COPY_TEMP_BUFFER
-:                                  ;                           <<<<<<<<<<<<|
        ld hl, LZ_WINDOW            ; Set HL (CURSOR) as LZ_WINDOW          |
        add hl, bc                  ; Add DISTANCE in HL                    |
        ld a, (hl)                  ; Load Byte from HL (CURSOR)            |
        ld (de), a                  ; Copy Byte to Temp Buffer              |
        out ($be), a       ; Copy Byte to VRAM                     |
        inc de                      ; Increment Temp Buffer                 |
        inc bc                      ; Increment DISTANCE                    |
        ld a, b                     ;                                       |
        and $07                     ; Get first 3 bits from Byte            |
        ld b, a                     ;                                       |
        ld hl, LZ_COPY_LENGTH       ;                                       |
        dec (hl)                    ; Decrement LENGTH                      |
        jr nz, -                    ; If NOT ZERO loop again    >>>>>>>>>>>>|
        pop hl
        pop af
        ld hl, LZ_COPY_TEMP_BUFFER  ; Set HL (CURSOR) as Temp Buffer position
        ld d, a
-:
                                    ; Copy Temp Buffer to LZ_WINDOW
                                    ; -------------------------------
                                    ;                             <<<<<<<<<<<<|
        call _STORE_IN_LZ_WINDOW    ; Copy Byte to LZ WINDOW                  |
        dec d                       ; Decrement LENGHT                        |
        jr nz, -                    ; If NOT ZERO loop again      >>>>>>>>>>>>|
        pop hl
        ret

; PERFORMS RAW COPY
; D  = LENGTH
_RAW_COPY:
                                    ; GET LENGTH
                                    ; -------------------------------
        and $3F                     ; Get first 6 bits from Byte
        inc a                       ; Increment LENGTH
        ld d, a

-:                                  ;                          <<<<<<<<|
        ld a, (hl)                  ; Load next byte in A              |
        out ($be), a       ; Copy Byte to VRAM                |
        call _STORE_IN_LZ_WINDOW    ; Copy ByteA to LZ_WINDOW          |
        dec d                       ; Decrement D                      |
        jr nz, -                    ; If NOT ZERO loop again   >>>>>>>>|
        ret

; STORE BYTE IN LZ WINDOW
_STORE_IN_LZ_WINDOW:
        ld a, (hl)                  ; Load byte from CURSOR (HL) in A
        inc hl                      ; Increment CURSOR (HL)
        push hl
        ld hl, LZ_WINDOW            ; Store LZ_WINDOW BASE address in HL
        ld bc, (LZ_WINDOW_CURSOR)   ; Store LZ_WINDOW CURSOR position in BC
        add hl, bc                  ; Add BC in HL (BASE address + CURSOR)
        ld (hl), a                  ; Store Byte in HL (LZ_WINDOW)
        inc bc                      ; Increment BC (LZ_WINDOW CURSOR position)
        res 3, b
        ld (LZ_WINDOW_CURSOR), bc   ; Store LZ_WINDOW CURSOR position
        pop hl
        ret
        