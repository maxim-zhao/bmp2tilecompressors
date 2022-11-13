; Sonic 2 tile decompressor
;
; Needs 32 bytes of RAM for temporary storage. Define Sonic2TileLoaderMemory as the start address of the RAM to use.

.block "TileLoaderSonic2"
.section "Tile loader (Sonic 2)" free

; RAM usage
.enum Sonic2TileLoaderMemory export
Sonic2TileLoader_TileCount        dw
Sonic2TileLoader_DataPointer      dw
Sonic2TileLoader_BitStreamPointer dw
Sonic2TileLoader_EmptyTileBuffer       dsb 32
Sonic2TileLoader_DecompressedData dsb 32
Sonic2TileLoader_BitstreamTileCounter db
.ende

Sonic2TileLoader_Decompress:
  ; Set VDP address
  ld c,$bf
  out (c),e
  out (c),d
  
  push hl
    ; Skip header
    inc hl
    inc hl
    ; Read tile count
    ld a, (hl)
    ld (Sonic2TileLoader_TileCount), a
    inc hl
    ld a, (hl)
    ld (Sonic2TileLoader_TileCount+1), a
    inc hl
    ; Read bitstream pointer
      ld e, (hl)
      inc hl
      ld d, (hl)
      inc hl
    ; Data starts here
      ld (Sonic2TileLoader_DataPointer), hl
  pop hl
  add hl, de
  ld (Sonic2TileLoader_BitStreamPointer), hl

  ; Zero the buffer
  ld hl, Sonic2TileLoader_EmptyTileBuffer
  ld de, Sonic2TileLoader_EmptyTileBuffer + 1
  ld bc, 31
  ld (hl), 0
  ldir

  ; Reset the consumed tile counter
  xor a
  ld (Sonic2TileLoader_BitstreamTileCounter), a
  
-:call _getTileType
  cp 0
  jr nz, +
  ; Type 0 = all zero
  call _emitZeroBuffer
  jr ++

+:cp $02
  jr nz, +
  ; Type 2 = compressed
  call _decompress
  call _emitBuffer
  jr ++

+:cp $03
  jr nz, +
  ; Type 3 = compressed + XOR
  call _decompress
  call _xor
  call _emitBuffer
  jr ++

+:; Type 1 = raw
  call _raw
  call _emitBuffer
  ; fall through

++:
  ; Count down the tile count until done
  ld hl, (Sonic2TileLoader_TileCount)
  dec hl
  ld (Sonic2TileLoader_TileCount), hl
  ld a, l
  or h
  jr nz, -
  ret

_raw:
  ; Copy 32 bytes
  ld bc, 32
  ld hl, (Sonic2TileLoader_DataPointer)
  ld de, Sonic2TileLoader_DecompressedData
  ldir
  ld (Sonic2TileLoader_DataPointer), hl
  ret

_decompress:
  ; Output pointer
  ld ix, Sonic2TileLoader_DecompressedData
  ; Get 32 bits of flags
  ld hl, (Sonic2TileLoader_DataPointer)
  ld e, (hl)
  inc hl
  ld d, (hl)
  inc hl
  ld c, (hl)
  inc hl
  ld b, (hl)
  inc hl
  ld a, 32 ; bit counter
-:
  push af
    ; Rotate a bit out
    rr b
    rr c
    rr d
    rr e
    jr c, +
    ; 0 => emit a 0
    ld (ix+0), 0
    jr ++

+:  ; 1 => emit a byte
    ld a, (hl)
    ld (ix+0), a
    inc hl
    
++: inc ix
  pop af
  ; Repeat 32 times
  dec a
  jr nz, -
  ld (Sonic2TileLoader_DataPointer), hl
  ret

_xor:
  ld ix, Sonic2TileLoader_DecompressedData
  ld b, 7
-:; XOR bytes against each other across bitplanes on a few rows
  ld a, (ix+0)
  xor (ix+2)
  ld (ix+2), a
  ld a, (ix+1)
  xor (ix+3)
  ld (ix+3), a
  ld a, (ix+16)
  xor (ix+18)
  ld (ix+18), a
  ld a, (ix+17)
  xor (ix+19)
  ld (ix+19), a
  inc ix
  inc ix
  djnz -
  ret

_getTileType:
  ld a, (Sonic2TileLoader_BitstreamTileCounter)
  cp 4
  jr nz, +
  ; Read next byte from bitstream
  ld hl, (Sonic2TileLoader_BitStreamPointer)
  inc hl
  ld (Sonic2TileLoader_BitStreamPointer), hl
  xor a
  ld (Sonic2TileLoader_BitstreamTileCounter), a

+:; 
  ld b, a
  ld hl, (Sonic2TileLoader_BitStreamPointer)
  ld a, (hl)
  ; Get the right two bits in the low two according to the value of Sonic2TileLoader_BitstreamTileCounter
-:dec b
  jp m, +
  rrca
  rrca
  jp -

+:and %11 ; Mask to two bits
  push af
    ld a, (Sonic2TileLoader_BitstreamTileCounter)
    inc a
    ld (Sonic2TileLoader_BitstreamTileCounter), a
  pop af
  ret

_emitZeroBuffer:  
  ld hl, Sonic2TileLoader_EmptyTileBuffer
  ld de, Sonic2TileLoader_DecompressedData
  ld bc, 32
  ldir
  ; fall through
_emitBuffer: 
  ld hl, Sonic2TileLoader_DecompressedData
  ld b, 32
-:ld a, (hl)
  out ($be), a
  push hl ; delay
  pop hl
  inc hl
  djnz -
  ret
.ends
.endb



