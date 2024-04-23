; This is taken verbatim from Wonder Boy.

WriteAToVRAMAtHL:
  push af
    ld a,l
    out ($bf),a
    ld a,h
    and $3f
    or $40
    out ($bf),a
  pop af
  out ($be),a
  ret
  
; hl = source
; de = dest
decompress:
  ex de,hl
  ld c,4                     ; 4 bitplanes
_OuterLoop:
  push hl
_Loop:
    ld a,(de)                ; get byte
    or a                     ; if zero, it is either RLE or the end of the bitplane
    jr z,_RLE
    inc a                    ; if $ff, write the following byte twice
    jr z,_WriteNextByteTwice
    dec a                    ; else write the byte itself

_WriteByteIncrementBothAndLoop:
    call WriteAToVRAMAtHL
    inc hl
    inc hl
    inc hl
    inc hl
_IncrementSrcAndLoop:
    inc de                   ; move to next byte and repeat
    jr _Loop

_WriteNextByteTwice:
  inc de                     ; get next byte
  ld a,(de)
  call WriteAToVRAMAtHL      ; write it to VRAM
  inc hl                     ; skip bitplane
  inc hl
  inc hl
  inc hl
  jr _WriteByteIncrementBothAndLoop ; then write it again and loop

_RLE:
  inc de                     ; get next byte = RLE count
  ld a,(de)
  inc de
  or a                       ; if it is 0, it is actually the end of the bitplane
  jr z,_EndOfBitplane
  ld b,a                     ; output the next byte repeatedly
  ld a,(de)
-:call WriteAToVRAMAtHL
  inc hl
  inc hl
  inc hl
  inc hl
  djnz -
  jr _IncrementSrcAndLoop    ; move to next byte and repeat

_EndOfBitplane:
  pop hl                     ; move original dest pointer on by 1
  inc hl
  dec c                      ; loop 4 times for 4 bitplanes
  jr nz,_OuterLoop
  ret                        ; done
