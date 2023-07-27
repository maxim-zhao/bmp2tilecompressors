.ifndef BerlinWallDecompressorMemory
.fail "Please define BerlinWallDecompressorMemory"
.endif

.if BerlinWallDecompressorMemory & $ff != 0
.fail "BerlinWallDecompressorMemory must be aligned to 256 bytes"
.endif

.enum BerlinWallDecompressorMemory
  BerlinWall_history_buffer              dsb 256  ; buffer of previous 256B emitted, must be aligned to 256B boundary
  BerlinWall_bitstream_next_byte_pointer dw       ; pointer to next byte for bitstream
  BerlinWall_buffer_output_pointer       dw       ; pointer to next byte for history buffer
  BerlinWall_LZ_counter                  db       ; counter for LZ bytes to emit
  BerlinWall_LZ_source                   dw       ; pointer to next byte for LZ match
  BerlinWall_bitstream_byte              db       ; unconsumed bits from last byte read from bitstream
  BerlinWall_bitstream_byte_bitcount     db       ; counter for bits left in bitstream_byte, must follow it
.ende

berlinwall_decompress:

; HL = source pointer
; DE = destination VRAM address (write bit set)
; BC = byte count / 16

; format (bitstream):
; 1 dddddddd: emit literal d
; 0 oooooooo llll: emit l+2 bytes from buffer offset o
; at each call, we retrieve the next byte, put it into the history buffer and return it
; the history buffer starts with all values = $20 and next write position = $ef

  ld a, e           ; Set VRAM address de
  out ($bf), a
  ld a, d
  out ($bf), a
  ; Get length, which is stored big-endian, into b and c
  ; such that we can loop over b and then c. This means that if b=0
  ; we need to increment c.
  ld c,(hl) ; MSB of count
  inc hl
  ld a,(hl) ; LSB of count
  inc hl
  and a
  jr z, +
  inc c
+:ld b, a

_init:              ; initialise work RAM
  ld a, (hl)        ; read first bitstream byte
  ld (BerlinWall_bitstream_byte), a
  inc hl            ; increment and save pointer
  ld (BerlinWall_bitstream_next_byte_pointer), hl
  exx
    ld hl, BerlinWall_history_buffer ; set buffer initial data
    ld de, BerlinWall_history_buffer+1
    ld bc, 256-1
    ld (hl), $20
    ldir

    xor a
    ld (BerlinWall_LZ_counter), a ; LZ_counter = 0
    ld hl, BerlinWall_history_buffer + $ef ; buffer_output_pointer, LZ_source point at $d9ef (buffer start point). This is mostly to get the right MSB.
    ld (BerlinWall_buffer_output_pointer), hl
    ld (BerlinWall_LZ_source), hl
    ld a, 8
    ld (BerlinWall_bitstream_byte_bitcount), a ; bitstream_byte_bitcount = 8
  exx

-:exx
    .repeat 16      ; 16 bytes emitted for each iteration
      call _getByte   ; get next byte
      out ($be), a    ; emit
    .endr
  exx
  djnz - ; loop
  dec c
  jp nz, -
  ret

_getByte: ; get a byte to emit

  ld a, (BerlinWall_LZ_counter) ; check if we are in a match
  and a
  jr nz, _getNextLZByte

  ld b, 1           ; get next flag bit
  call _getBits
  and a
  jr z, _lz

_literal:
  ld b, 8           ; get value
  call _getBits

_saveInBufferAndReturn:
  ld hl, (BerlinWall_buffer_output_pointer) ; save in buffer
  ld (hl), a
  inc l
  ld (BerlinWall_buffer_output_pointer), hl
  ret               ; return value in a

_lz:
  ld b, 8           ; read source offset in buffer
  call _getBits

  ld (BerlinWall_LZ_source), a ; save to LZ_source (pointer low byte)

  ld b, 4           ; get run length
  call _getBits
  add a, 2          ; add 2
  ld (BerlinWall_LZ_counter), a ; save to LZ_counter

  ; fall through

_getNextLZByte:
  ld hl, (BerlinWall_LZ_source) ; get next byte (wrapping within buffer)
  ld a, (hl)
  inc l
  ld (BerlinWall_LZ_source), hl

  ld hl, BerlinWall_LZ_counter ; decrement counter
  dec (hl)
  jp _saveInBufferAndReturn

_getBits:           ; gets b bits fro teh bitstream into a
  ld hl, BerlinWall_bitstream_byte ; get current byte
  ld a, (hl)

  inc l             ; point at bitstream_byte_bitcount
  ld c, 0
  ld de, (BerlinWall_bitstream_next_byte_pointer)
-:add a, a          ; shift byte into c
  rl c
  dec (hl)          ; decrement counter and check for zero
  jr z, _nextDataByte
  djnz -            ; else loop b times
--:
  ld (BerlinWall_bitstream_next_byte_pointer), de ; save bitstream pointer
  dec l
  ld (hl), a        ; save remaining bits in bitstream_byte
  ld a, c           ; return consumed bits in a
  ret

_nextDataByte:      ; Ran out of bits in bitstream_byte
  ld a, (de)        ; get next byte from *de++
  inc de
  ld (hl), 8        ; set counter for 8 more bits
  djnz -            ; continue looping
  jr --
