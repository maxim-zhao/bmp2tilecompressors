.enum LemmingsDecompressorMemory ; Must be 256-aligned
LemmingChunkyBuffer       dsb 256
LemmingsInterleavedBuffer dsb 256
.ende

; de = data
; ix = VRAM address
lemmings_decompress:
    ld a, (de) ; First byte = loop count (tile count / 8?)
    ld b, a
    inc de
-:  push bc
      call _decompress ; Decompress 8 tiles as chunky
      call _interleaveAndWriteToVDP
      ld bc, 256 ; + 8 tiles
      add ix, bc
    pop bc
    djnz -
    ret

_interleaveBuffer:
    push ix
      ld ix, LemmingsInterleavedBuffer ; Interleaved buffer
      ld hl, LemmingChunkyBuffer ; Chunky buffer
      call _copyAndInterleaveBitplane
      call _copyAndInterleaveBitplane
      call _copyAndInterleaveBitplane
      call _copyAndInterleaveBitplane
    pop ix
    ret

_interleaveAndWriteToVDP:
    call _interleaveBuffer
    push ix
    pop hl
    ; Set VRAM address
    ld c, $bf
    out (c), l
    out (c), h
    dec c ; c = data address
    ld hl, LemmingsInterleavedBuffer
    ; outi decrements b so to emit 256B we need to loop twice
    ld b, 0
-:  outi
    djnz -
    ld b, 0
-:  outi
    djnz -
    ret

_copyAndInterleaveBitplane:
    ; Deinterleaves 64 bytes from hl to ix, spaced every 4 bytes.
    ; Returns ix incremented by 1.
    push ix
      ld b, 64 ; Counter for 64 bytes = 8 tiles?!
-:    ld a, (hl)
      ld (ix+0), a
      inc hl
      inc ix
      inc ix
      inc ix
      inc ix
      djnz -
    pop ix
    inc ix
    ret

_decompress:
    ld hl, LemmingsDecompressorMemory
--:
    ld a, h
    cp (>LemmingsDecompressorMemory)+1 ; Quit if reached 256 bytes. Requires LemmingsDecompressorMemory to be 256-aligned.
    ret nc

    ld a, (de) ; Read source byte
    bit 7, a
    jr nz, +

    ; High bit is 0 => raw run
    ld b, a ; Value is loop count minus 1
    inc b
    inc de
-:
    ld a, (de) ; Read byte
    ld (hl), a ; Copy to dest
    inc de
    inc hl
    ld a, h ; Bounds check. Should not be necessary?
    cp (>LemmingsDecompressorMemory)+1
    ret nc
    djnz -
    jp --

+:  ; High bit is 1 => RLE
    ; Count is remaining bits plus 1:
    ; %10000000 = $80 -> $01
    ; %11111111 = $ff -> $80
    sub $7F
    ld b, a
    inc de
    ld a, (de)
    ld c, a
-:
    ld (hl), c
    inc hl
    ; Bounds check again, except now we continue the loop if it fails?!
    ld a, h
    cp (>LemmingsDecompressorMemory)+1
    jr nc, +
    djnz -
+:
    inc de
    jp --

