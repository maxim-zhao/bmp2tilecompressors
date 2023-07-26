.enum HighSchoolKimengumiDecompressorMemory ; memory used: 4 bytes
    rowCount dw
    rowCountTotal dw
.ende

; Usage:
; bc = source data
; de = destination (VRAM address ORed with $4000)
decode:
    call _decode

    ; clean up
    xor a
    ld (rowCount),a
    ld (rowCount+1),a
    ld (rowCountTotal),a
    ld (rowCountTotal+1),a
    ret

_decode:
    ld a,(bc)       ; hl = (bc)
    ld l,a
    inc bc
    ld a,(bc)
    ld h,a
    dec bc

    ld (rowCount),hl ; load row count into RAM
    ld (rowCountTotal),hl

    ld h,b          ; hl = bc
    ld l,c

    inc hl          ; hl += 2
    inc hl

_nextBlock:
    ld a,(hl)       ; read control byte
    or a            ; 0 = terminator
    ret z

    bit 7,a         ; if bit 7 = 1 then it's raw
    jr nz,+

    ; RLE
    ld b,a          ; run length
    inc hl          ; run value
-:  call _outputByte
    djnz -
    inc hl          ; next control byte
    jp _nextBlock

+:  ; Raw
    and $7f         ; length
    ld b,a
    inc hl          ; value
-:  call _outputByte
    inc hl
    djnz -
    jp _nextBlock

_outputByte:
    ; writes a byte from hl to VRAM address de with interleaving 4

    ; set VRAM address
    ld a,e
    out ($bf),a
    ld a,d
    out ($bf),a

    ; write value
    ld a,(hl)
    out ($be),a

    ; decrement row counter
    push hl
        ld hl,(rowCount)
        dec hl
        ld a,h      ; if zero, move to next bitplane
        or l
        jr z,_setNextBitplane
        ld (rowCount),hl

        inc de      ; de += 4 (interleaving)
        inc de
        inc de
        inc de
    pop hl
    ret

_setNextBitplane:
        ld hl,(rowCountTotal) ; restore the counter
        ld (rowCount),hl

        ; now:
        ; - hl = number of tile rows
        ; - de = last byte written in previous bitplane
        ; so we want to set de = de - hl * 4 + 5

        ; first calculate hl = hl * 4
        xor a           ; clear carry
        rl l            ; shift hl left by 1
        rl h
        xor a           ; repeat
        rl l
        rl h

        ; subtract hl and add 5
        ex de,hl
            xor a       ; clear carry
            sbc hl,de
            inc hl
            inc hl
            inc hl
            inc hl
            inc hl
        ex de,hl
    pop hl
    ret
