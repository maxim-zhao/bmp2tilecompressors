; Sonic 2 tile decompressor
;
; Needs 39 bytes of RAM for temporary storage. Define Sonic2TileLoaderMemory as the start address of the RAM to use.

.section "Tile loader (Sonic 2)" free

; RAM usage
.enum Sonic2TileLoaderMemory export
Sonic2TileLoader_TileCount        dw
Sonic2TileLoader_DataPointer      dw
Sonic2TileLoader_BitStreamPointer dw
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

  ; Reset the consumed tile counter
  xor a
  ld (Sonic2TileLoader_BitstreamTileCounter), a
  
➿:
  ; Inlined "_getTileType" function >============================>
  ld a, (Sonic2TileLoader_BitstreamTileCounter)
  cp 4
  jr nz, +
  ; Read next byte from bitstream
  ld hl, (Sonic2TileLoader_BitStreamPointer)
  inc hl
  ld (Sonic2TileLoader_BitStreamPointer), hl
  xor a
  ld (Sonic2TileLoader_BitstreamTileCounter), a

+:; Get the current bitstream byte
  ld b, a
  ld hl, (Sonic2TileLoader_BitStreamPointer)
  ld a, (hl)
  ; Get the right two bits in the low two according to the value of Sonic2TileLoader_BitstreamTileCounter
-:dec b
  jp m, +
  rrca
  rrca
  jp -

+:ld hl,Sonic2TileLoader_BitstreamTileCounter
  inc (hl)
  and %11 ; Mask to two bits
  ; Inlined "_getTileType" function <============================<

  ; Type 0 = all zero
  jr z, _emitZeroBuffer
  
  dec a
  ; Type 1 = raw
  jr z,_raw

  push af
    ; Type 2 or 3 both need this
    
    ; Inlined "_decompress" function >============================>
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
-:  push af
      ; Rotate a bit out
      rr b
      rr c
      rr d
      rr e
      jr c, +
      ; 0 => emit a 0
      ld (ix+0), 0
      jr ++

+:    ; 1 => emit a byte
      ld a, (hl)
      ld (ix+0), a
      inc hl
      
++:   inc ix
    pop af
    ; Repeat 32 times
    dec a
    jr nz, -
    ld (Sonic2TileLoader_DataPointer), hl
    ; Inlined "_decompress" function <============================<
  pop af
  dec a
  ; Only XOR for type 3
  call nz, _xor
  ; Fall through

  ld hl, Sonic2TileLoader_DecompressedData
  ld b, 32
-:ld a, (hl)
  out ($be), a
  inc hl
  djnz -
  ; fall through

_tileDone:
  ; Count down the tile count until done
  ld hl, (Sonic2TileLoader_TileCount)
  dec hl
  ld (Sonic2TileLoader_TileCount), hl
  ld a, l
  or h
  ret z
  jp ➿

_raw:
  ; Emit 32 bytes
  ld hl, (Sonic2TileLoader_DataPointer)
  ld b, 32
-:ld a, (hl)
  out ($be), a
  inc hl
  djnz -
  ld (Sonic2TileLoader_DataPointer),hl
  jp _tileDone

_emitZeroBuffer:
  ; a must be 0 to get here
.repeat 32
  out ($be),a
.endr
  jp _tileDone

_xor:
  ld ix, Sonic2TileLoader_DecompressedData
  ld b, 7
-:; XOR bytes against each other across bitplanes on a few rows
  ; We end up using XORed bytes as values XORed later so this is not symmetric, it must be done in reverse order when compressing
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
.ends
