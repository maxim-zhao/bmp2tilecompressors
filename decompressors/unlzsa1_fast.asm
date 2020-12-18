; This file is an assembler port for WLA-DX by Maxim.
; It is forwards-only with no unrolling (as that used self-modifying code).
; Define LZSAToVRAM to enable VRAM mode for SMS.
; Usage:
;
;  ld hl,data
;  ld de,dest
;  call DecompressLZSA1
;
; Original comments follow.
;
;  Speed-optimized LZSA1 decompressor by spke & uniabis (109 bytes)
;
;  ver.00 by spke for LZSA 0.5.4 (03-24/04/2019, 134 bytes);
;  ver.01 by spke for LZSA 0.5.6 (25/04/2019, 110(-24) bytes, +0.2% speed);
;  ver.02 by spke for LZSA 1.0.5 (24/07/2019, added support for backward decompression);
;  ver.03 by uniabis (30/07/2019, 109(-1) bytes, +3.5% speed);
;  ver.04 by spke (31/07/2019, small re-organization of macros);
;  ver.05 by uniabis (22/08/2019, 107(-2) bytes, same speed);
;  ver.06 by spke for LZSA 1.0.7 (27/08/2019, 111(+4) bytes, +2.1% speed);
;  ver.07 by spke for LZSA 1.1.0 (25/09/2019, added full revision history);
;  ver.08 by spke for LZSA 1.1.2 (22/10/2019, re-organized macros and added an option for unrolled copying of long matches);
;  ver.09 by spke for LZSA 1.2.1 (02/01/2020, 109(-2) bytes, same speed)
;
;  The data must be compressed using the command line compressor by Emmanuel Marty
;  The compression is done as follows:
;
;  lzsa.exe -f1 -r <sourcefile> <outfile>
;
;  where option -r asks for the generation of raw (frame-less) data.
;
;  The decompression is done in the standard way:
;
;  ld hl,FirstByteOfCompressedData
;  ld de,FirstByteOfMemoryForDecompressedData
;  call DecompressLZSA1
;
;  Backward compression is also supported; you can compress files backward using:
;
;  lzsa.exe -f1 -r -b <sourcefile> <outfile>
;
;  and decompress the resulting files using:
;
;  ld hl,LastByteOfCompressedData
;  ld de,LastByteOfMemoryForDecompressedData
;  call DecompressLZSA1
;
;  (do not forget to uncomment the BACKWARD_DECOMPRESS option in the decompressor).
;
;  Of course, LZSA compression algorithms are (c) 2019 Emmanuel Marty,
;  see https://github.com/emmanuel-marty/lzsa for more information
;
;  Drop me an email if you have any comments/ideas/suggestions: zxintrospec@gmail.com
;
;  This software is provided 'as-is', without any express or implied
;  warranty.  In no event will the authors be held liable for any damages
;  arising from the use of this software.
;
;  Permission is granted to anyone to use this software for any purpose,
;  including commercial applications, and to alter it and redistribute it
;  freely, subject to the following restrictions:
;
;  1. The origin of this software must not be misrepresented; you must not
;     claim that you wrote the original software. If you use this software
;     in a product, an acknowledgment in the product documentation would be
;     appreciated but is not required.
;  2. Altered source versions must be plainly marked as such, and must not be
;     misrepresented as being the original software.
;  3. This notice may not be removed or altered from any source distribution.

DecompressLZSA1:
.ifdef LZSAToVRAM
    ; Set VRAM address
    ld c,$bf
    out (c),e
    out (c),d
.endif
    ld b,0
    jp _ReadToken

.ifdef LZSAToVRAM
_copy_256b_bytes_vram_to_vram:
    ; Emit 256*b bytes
--: push bc
      ld b,0
      ld c,$bf
-:    out (c),l
      out (c),h
      in a,($be)
      out (c),e
      out (c),d
      out ($be),a
      inc hl
      inc de
      djnz -
    pop bc
    djnz --
    ; Then the rest, if any
    ld a,c
    or a
    jp z, _CopyMatch_Done
    jp _copy_c_bytes_vram_to_vram
.endif

_NoLiterals:
    ; Get the O, M bits from the token
    xor (hl)
    inc hl
    ; O = 1 -> "long" offset
    jp m,_LongOffset

    ; O = 0 -> "short" offset
_ShortOffset:
    push de
      ; Next byte is a negative number for the offset
      ld e,(hl)
      ld d,$FF

      ; MMMM = 15 means a "longer" match length; MMMM < 15 means match length is MMMM + 3
      add 3
      cp 15+3
      jr nc,_LongerMatch ; unlikely
      ; fall through with a = match length and de = offset

_CopyMatch:
      ; Copy an LZ run of length ba from de bytes before the dest pointer
      ld c,a
@UseBC:
      inc hl
      ex (sp),hl            ; BC = len, DE = offset, HL = dest, SP ->[dest,src]
      ex de,hl
      add hl,de               ; BC = len, DE = dest, HL = dest-offset, SP->[src]
.ifdef LZSAToVRAM
      ; Copy bc bytes from VRAM address hl to VRAM address de
      ; Both hl and de are "write" addresses ($4xxx)
      ; Make hl a read address
      res 6, h
      ; Check if the count is below 256
      ld a,b
      or a
      jr nz,_copy_256b_bytes_vram_to_vram ; unlikely
      ; By emitting <=256 at a time, we can use the out (c),r opcode
      ; for address setting, which then relieves pressure on a
      ; and saves some push/pops; and we can use djnz for the loop.
_copy_c_bytes_vram_to_vram:
      ld b,c
      ld c,$bf
-:    out (c),l
      out (c),h
      in a,($be)
      out (c),e
      out (c),d
      out ($be),a
      inc hl
      inc de
      djnz -
_CopyMatch_Done:
.else
      ldi ; Not sure why it does this? If bc = 0..2 it will give us +64K but we don't need it?
      ldi
      ldir            ; BC = 0, DE = dest
.endif
    pop hl                ; HL = src
    ; fall through

_ReadToken:
    ; Token is one of
    ; OLLLMMMM where O = offset size flag, L = literal count, m = match length
    ;  LLL = 0 -> no literals
    ;  LLL = 7 -> 
    ld a,(hl)
    and %01110000
    jr z,_NoLiterals ; unlikely

    cp %01110000
    jr z,_MoreLiterals ; unlikely? ; 7 indicates 7+ literals

    ; Get the literal count into c for 1..6 literals
    rrca
    rrca
    rrca
    rrca
    ld c,a

    ;
    ld a,(hl) ; re-get token
    inc hl
.ifdef LZSAToVRAM
    ex af,af'
      call _ldir_rom_to_vram
    ex af,af'
.else
    ldir
.endif

    ; the top bit of token is set if the offset contains two bytes
    and %10001111
    jp p,_ShortOffset

_LongOffset: ; read second byte of the offset
    push de
      ld e,(hl)
      inc hl
      ld d,(hl)
      add -128+3
      cp 15+3
      jp c,_CopyMatch

      ; MMMM=15 indicates a multi-byte number of literals
_LongerMatch:
      ; Next byte is the extra match length
      inc hl
      add (hl)
      ; If it does not overflow 8 bits then use it
      jp nc,_CopyMatch ; likely

      ; If it overflows past 0 then the result is the high byte and we read the low byte.
      ld b,a
      inc hl
      ld c,(hl)
      jp nz,_CopyMatch@UseBC ; likely

      ; If it overflows to exactly 0 then we have a two-byte match length to read.
      inc hl
      ld b,(hl)

      ; If these two bytes are 0 then we have got to the end of the data
      ld a,b
      or c
      jr nz,_CopyMatch@UseBC ; unlikely to have a two-byte length other than 0
    pop de
    ret ; This is the exit point fo the decompressor

_MoreLiterals: ; there are three possible situations here
    ; Get the O, MMMM bits
    xor (hl)
    inc hl
    ; Save them temporarily
    ex af,af'
      ; Next byte is a literal count minus 7. High values indicate longer counts.
      ld a,7
      add (hl)
      jr c,_ManyLiterals ; unlikely

      ld c,a ; b is 0?

_CopyLiterals:
      inc hl
.ifdef LZSAToVRAM
      call _ldir_rom_to_vram
.else
      ldir ; source to dest
.endif
    ex af,af'
    ; Then continue on to the LZ part based on the O bit
    jp p,_ShortOffset ; likely
    jp _LongOffset

_ManyLiterals:
    ; value is high byte of count if non-zero
    ld b,a
    inc hl
    ; get low byte
    ld c,(hl)
    jp nz,_CopyLiterals ; likely
    ; if zero, get high byte of count
    inc hl
    ld b,(hl)
    jp _CopyLiterals

.ifdef LZSAToVRAM
_ldir_rom_to_vram:
  ; add bc to de to mimic ldir
  ex de,hl
    add hl,bc
  ex de,hl

  ; If b = 0, we can short-circuit. We optimise for this as we expect mostly short runs.
  ld a,b
  or a
  jr nz,+ ; unlikely

--:
  ld b,c
  ld c,$be
  otir
  ret

+:push bc
    ; We copy b*256 bytes
    ld c,$be
    ld b,0
-:  otir
    dec a
    jp nz,-
  pop bc
  ; If we have remainder, process it
  ld a,c
  or a
  ret z
  jp --
.endif
