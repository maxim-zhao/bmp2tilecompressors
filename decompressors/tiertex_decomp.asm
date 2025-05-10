; Tiertex art decompressor
; Usage:
; ld hl, data_header
; ld de, vram_address
; call decompress_tiertex
; where data_header points to bytes:
; $hh $ll $CC $cc $DD $dd $bb $tt
; where
; $hh $ll = big-endian location of data
; $CC $cc = big-endian compressed size of data in first bank
; $DD $dd = big-endian compressed size of data in second bank
; $bb = bank number
; $tt = compression type (0..3).
; If data is in a single bank, $dd $dd = 0.
; If it is across banks, it tells the decompressor where to split.
; Type 0: RLE
; Type 1: RLE and 4-interleaved data
; Type 2: raw
; Type 3: RLE and packed 2x12-bit data to 3 bytes
; Types 0 and 2 are disabled here because they are not supported by the compressor.
; It always uses 1 for tiles and 3 for tilemap.

.enum TiertexDecompressorMemory
  TiertexDecompressorHeaderCopy  dsb 8
  TiertexDecompressorBuffer      dsb 32
  TiertexDecompressorSpecialByte db
.ende

decompress_tiertex:
  ; Set VRAM address
  ld a, e
  out ($BF), a
  ld a, d
  out ($BF), a

  ; Copy 8 bytes table entry to RAM
  ld de, TiertexDecompressorHeaderCopy
  ld bc, 8
  ldir
  ; Point ix to it
  ld ix, TiertexDecompressorHeaderCopy
  ; Read first four bytes to to hl, bc
  ld h, (ix+0) ; Data pointer
  ld l, (ix+1)
  ld b, (ix+2) ; Byte count
  ld c, (ix+3)
  dec bc ; because it's one too many?
  
  ld e, 32 ; Buffer size
  ld iy, TiertexDecompressorBuffer ; iy points to a buffer
  
  ; 7th byte is the bank number, relative to the table bank
  ld a, (ix+6)
  ld ($ffff), a
  ; Read a byte from hl to TiertexDecompressorSpecialByte
  ld a, (hl)
  ld (TiertexDecompressorSpecialByte), a
  inc hl
  dec bc
  ; Check the 7th byte in the table entry
  ld a, (ix+7)
;  cp 2
;  jp z, _Type2 ; Type 2
;  cp 3
;  jp z, _Type3 ; Type 3
;  and a
;  jp nz, _Type1 ; Type 1
;  jp _Type0 ; Type 0
  dec a
  jr nz, _Type3
  ; else fall through

_Type1:
; Type 1
; Data is:
; BB          Most common byte, not emitted (stored to TiertexDecompressorSpecialByte before getting here)
; BB NN       Emit byte BB NN times
; XX          Emit byte XX
; Bytes are emitted into a 32 bytes buffer; when it is full, it is emitted to VRAM
; deinterleaved by 8 bytes, so:
; abcdefghijklmnopqrstuvwxyz012345
; is emitted as
; aiqybjrz...

  ; Read first byte
  ld d, (hl)
  inc hl
  dec bc
  ; Compare to TiertexDecompressorSpecialByte
  ld a, (TiertexDecompressorSpecialByte)
  cp d
  jr z, +
  ; It is different: load to buffer
  ld (iy+0), d
  inc iy
  dec e
  call z, _EmitBuffer32 ; If we have done this 32 times
---:
  ; Check for bank overflow (see comments elsewhere)
  ld a, b
  inc a
  jr nz, _Type1
  ld b, (ix+4)
  ld c, (ix+5)
  ld a, b
  or c
  ret z
  dec bc
  ld a, (ix+6)
  inc a
  ld ($ffff), a
  ld (ix+4), $00
  ld (ix+5), $00
  ld hl, $8000
  jp _Type1

--:
  inc hl
  dec bc
  inc hl
  dec bc
  jr ---

+:
  ; Byte is same as the lead marker
  ; Next byte is the repeat count
  inc hl
  ld d, (hl)
  inc d
  dec hl
-:
  dec d
  jr z, --
  ld a, (hl)
  ld (iy+0), a
  inc iy
  dec e
  call z, _EmitBuffer32
  jr -
  
_EmitBuffer32:
  ; Point to buffer
  ld iy, TiertexDecompressorBuffer
  push bc
    ld b, $08
-:  ; Deinterleave by 8 as we go
    ld a, (iy+0)
    out ($BE), a
    ld a, (iy+8)
    out ($BE), a
    ld a, (iy+16)
    out ($BE), a
    ld a, (iy+24)
    out ($BE), a
    inc iy
    djnz -
  pop bc
  ; Reset counter and pointer
  ld e, 32
  ld iy, TiertexDecompressorBuffer
  ret

_Type3:
; Type 3
; Data is:
; BB          Most common byte, not emitted (stored to TiertexDecompressorSpecialByte before getting here)
; BB NN       Emit byte BB NN times
; XX          Emit byte XX
; Bytes are emitted into a 3 bytes buffer; when it is full, it is emitted to VRAM in the form
; AaBbCc
; =>
; Aa0cBb0C
; This is because it is two name table entries, with the upper bits sharing a byte.
; This means those entries are 12 bits, so the priority bit cannot be used.
  ld e, $03 ; 3 bytes buffer
----:
  ; Read a byte
  ld d, (hl)
  inc hl
  dec bc
  ; Compare to marker
  ld a, (TiertexDecompressorSpecialByte)
  cp d
  jr z, +
  ; Byte is different: emit to buffer
  ld (iy+0), d
  inc iy
  dec e
  call z, _EmitBuffer3
---:
  ; Check for bank overflow
  ld a, b
  inc a
  jr nz, ----
  ld b, (ix+4)
  ld c, (ix+5)
  ld a, b
  or c
  ret z
  dec bc
  ld a, (ix+6)
  inc a
  ld ($ffff), a
  ld (ix+4), $00
  ld (ix+5), $00
  ld hl, $8000
  jp ----

--:
  inc hl
  dec bc
  inc hl
  dec bc
  jr ---

+:
  ; Byte is the same: next byte is repeat count
  inc hl
  ld d, (hl)
  inc d
  dec hl
-:
  dec d
  jr z, --
  ld a, (hl)
  ld (iy+0), a
  inc iy
  dec e
  call z, _EmitBuffer3
  jr -

_EmitBuffer3:
  ; Emit buffer. Split byte 2 across the 2nd and 4th bytes emitted.
  ld iy, TiertexDecompressorBuffer
  ld a, (iy+0)
  out ($BE), a
  ld a, (iy+2)
  and $0F
  out ($BE), a
  ld a, (iy+1)
  out ($BE), a
  ld a, (iy+2)
  srl a
  srl a
  srl a
  srl a
  out ($BE), a
  ; Reset counter and pointer
  ld e, 3
  ld iy, TiertexDecompressorBuffer
  ret
/*
_Type0:
; Type 0
; Data is:
; BB          Most common byte, not emitted (stored to TiertexDecompressorSpecialByte before getting here)
; BB NN       Emit byte BB NN times
; XX          Emit byte XX
  ; Check next byte
  ld d, (hl)
  inc hl
  dec bc
  ; Compare to first byte of data
  ld a, (TiertexDecompressorSpecialByte)
  cp d
  jr z, +
  ; If different, output to VRAM
  ld a, d
  out ($BE), a
---:
  ; Check if bc has underflowed
  ld a, b
  inc a
  jr nz, _Type0 ; Loop if not
  ; Check if we have bytes in the next bank
  ld b, (ix+4)
  ld c, (ix+5)
  ld a, b
  or c
  ret z ; Finished if not
  ; Else page it in
  ld a, (ix+6)
  inc a
  ld ($ffff), a
  ; Clear RAM marker so we don't do it again
  ld (ix+4), $00
  ld (ix+5), $00
  ; Point to start of bank
  ld hl, $8000
  jp _Type0 ; And carry on

--:
  ; Skip two bytes
  inc hl
  dec bc
  inc hl
  dec bc
  jr ---

+:
  ; First byte of chunk is same as first byte of data
  ; Read next byte, and emit current byte that many times
  inc hl
  ld d, (hl)
  inc d
  dec hl
-:
  dec d
  jr z, --
  ld a, (hl)
  out ($BE), a
  jr -

_Type2:
; Type 2
; Just raw data
  ; Emit byte
  ld a, (hl)
  inc hl
  dec bc
  out ($BE), a
  ; Check for bank overflow
  ld a, b
  inc a
  jr nz, _Type2
  ld b, (ix+4)
  ld c, (ix+5)
  ld a, b
  or c
  ret z
  ld a, (ix+6)
  inc a
  ld ($ffff), a
  ld (ix+4), $00
  ld (ix+5), $00
  ld hl, $8000
  jp _Type2
*/