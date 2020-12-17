; This file is an assembler port for WLA-DX by Maxim.
; It is forwards-only with no unrolling (as that used self-modifying code).
; Define LZSAToVRAM to enable VRAM mode.
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
    jr _ReadToken

_NoLiterals: 
    xor (hl)
    inc hl
    jp m,_LongOffset

_ShortOffset:  
    push de
      ld e,(hl)
      ld d,$FF

      ; short matches have length 0+3..14+3
      add 3
      cp 15+3
      jr nc,_LongerMatch

    ; placed here this saves a JP per iteration
_CopyMatch:
      ld c,a
@UseC:
      inc hl
      ex (sp),hl            ; BC = len, DE = offset, HL = dest, SP ->[dest,src]
      ex de,hl
      add hl,de               ; BC = len, DE = dest, HL = dest-offset, SP->[src]
.ifdef LZSAToVRAM
      call _ldir_vram_to_vram
.else
      ldi ; Not sure why it does this? If bc = 0..2 it will give us +64K but we don't need it?
      ldi
      ldir            ; BC = 0, DE = dest
.endif
    pop hl                ; HL = src
  
_ReadToken:  ; first a byte token "O|LLL|MMMM" is read from the stream,
    ; where LLL is the number of literals and MMMM is
    ; a length of the match that follows after the literals
    ld a,(hl)
    and $70
    jr z,_NoLiterals

    cp $70
    jr z,_MoreLiterals          ; LLL=7 means 7+ literals...
    rrca
    rrca
    rrca
    rrca
    ld c,a        ; LLL<7 means 0..6 literals...

    ld a,(hl) ; re-get token
    inc hl
.ifdef LZSAToVRAM
    push af
      call _ldir_rom_to_vram
    pop af
.else
    ldir
.endif

    ; the top bit of token is set if the offset contains two bytes
    and $8F
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
      inc hl
      add (hl)
      jr nc,_CopyMatch

      ; the codes are designed to overflow;
      ; the overflow value 1 means read 1 extra byte
      ; and overflow value 0 means read 2 extra bytes
@code1:
      ld b,a
      inc hl
      ld c,(hl)
      jr nz,_CopyMatch@UseC
@code0:
      inc hl
      ld b,(hl)

      ; the two-byte match length equal to zero
      ; designates the end-of-data marker
      ld a,b
      or c
      jr nz,_CopyMatch@UseC
    pop de
    ret

_MoreLiterals: ; there are three possible situations here
    xor (hl)
    inc hl
    ex af,af'
    ld a,7
    add (hl)
    jr c,_ManyLiterals

_CopyLiterals: 
    ld c,a
@UseC:
    inc hl
.ifdef LZSAToVRAM
    call _ldir_rom_to_vram
.else
    ldir ; source to dest
.endif
    ex af,af'
    jp p,_ShortOffset
    jr _LongOffset

_ManyLiterals:
@code1:
    ld b,a
    inc hl
    ld c,(hl)
    jr nz,_CopyLiterals@UseC
@code0:
    inc hl
    ld b,(hl)
    jr _CopyLiterals@UseC

.ifdef LZSAToVRAM
_ldir_vram_to_vram:
  ; Copy bc bytes from VRAM address hl to VRAM address de
  ; Both hl and de are "write" addresses ($4xxx)
  ; Make hl a read address
  res 6, h
  ; Check if the count is below 256
  ld a,b
  or a
  jp z,_below256 ; likely
  ; Else emit 256*b bytes
-:push bc
    ld b,0
    call +
  pop bc
  djnz -
  ; Then fall through for the rest  
_below256:
  ; By emitting <=256 at a time, we can use the out (c),r opcode
  ; for address setting, which then relieves pressure on a
  ; and saves some push/pops; and we can use djnz for the loop.
  ld b,c
+:ld c,$bf
-:out (c),l
  out (c),h
  in a,($be)
  out (c),e
  out (c),d
  out ($be),a
  inc hl
  inc de
  djnz -
  ret
  
_ldir_rom_to_vram:
  ; If b = 0, we can short-circuit. We optimise for this as we expect mostly short runs.
  ld a,b
  or a
  jr nz,_above256

_below256_2:
  ; add c to de
  ex de,hl
  ld b,0
  add hl,bc
  ex de,hl
  ld b,c
  ld c,$be
  otir
  ret
  
_above256:
  push bc
    ; We copy b*256 bytes
    ld c,$be
    ld a,b
    ld b,0
-:  otir
    inc d ; move de on by 256
    dec a
    jr nz,-
  pop bc
  ; If we have remainder, process it
  ld a,c
  or a
  ret z
  jp _below256_2
.endif
