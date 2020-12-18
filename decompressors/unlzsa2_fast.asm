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
;  Speed-optimized LZSA2 decompressor by spke & uniabis (216 bytes)
;
;  ver.00 by spke for LZSA 1.0.0 (02-07/06/2019, 218 bytes);
;  ver.01 by spke for LZSA 1.0.5 (24/07/2019, added support for backward decompression);
;  ver.02 by spke for LZSA 1.0.6 (27/07/2019, fixed a bug in the backward decompressor);
;  ver.03 by uniabis (30/07/2019, 213(-5) bytes, +3.8% speed and support for Hitachi HD64180);
;  ver.04 by spke for LZSA 1.0.7 (01/08/2019, 214(+1) bytes, +0.2% speed and small re-organization of macros);
;  ver.05 by spke (27/08/2019, 216(+2) bytes, +1.1% speed);
;  ver.06 by spke for LZSA 1.1.0 (26/09/2019, added full revision history);
;  ver.07 by spke for LZSA 1.1.1 (10/10/2019, +0.2% speed and an option for unrolled copying of long matches)
;
;  The data must be compressed using the command line compressor by Emmanuel Marty
;  The compression is done as follows:
;
;  lzsa.exe -f2 -r <sourcefile> <outfile>
;
;  where option -r asks for the generation of raw (frame-less) data.
;
;  The decompression is done in the standard way:
;
;  ld hl,FirstByteOfCompressedData
;  ld de,FirstByteOfMemoryForDecompressedData
;  call DecompressLZSA2
;
;  Backward compression is also supported; you can compress files backward using:
;
;  lzsa.exe -f2 -r -b <sourcefile> <outfile>
;
;  and decompress the resulting files using:
;
;  ld hl,LastByteOfCompressedData
;  ld de,LastByteOfMemoryForDecompressedData
;  call DecompressLZSA2
;
;  (do not forget to uncomment the BACKWARD_DECOMPRESS option in the decompressor).
;
;  Of course, LZSA2 compression algorithms are (c) 2019 Emmanuel Marty,
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

; DEFINE  UNROLL_LONG_MATCHES           ; uncomment for faster decompression of very compressible data (+38 bytes)
; DEFINE  BACKWARD_DECOMPRESS           ; uncomment for data compressed with option -b
; DEFINE  HD64180               ; uncomment for systems using Hitachi HD64180

DecompressLZSA2:
.ifdef LZSAToVRAM
    ; Set VRAM address
    ld c,$bf
    out (c),e
    out (c),d
.endif

    ; A' stores next nibble as %1111.... or assumed to contain trash
    ; B is assumed to be 0
    ld b,0
    scf
    ex af,af'
    jp _ReadToken ; always taken

_ManyLiterals:
    ld a,18
    add (hl)
    inc hl
    jp nc,_CopyLiterals ; likely

    ld c,(hl)
    inc hl
    ld a,b
    ld b,(hl)
    jp _ReadToken@NEXTHLuseBC

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
    jp nz,_copy_c_bytes_vram_to_vram ; likely
    jp _CopyMatch_Done
.endif

_MoreLiterals:
    ld b,(hl)
    inc hl
    scf
    ex af,af'
    jr nc,+

    ld a,(hl)
    or $F0
    ex af,af'
    ld a,(hl)
    inc hl
    or $0F
    rrca
    rrca
    rrca
    rrca

+:  cp 15+3
    jr z,_ManyLiterals
    inc a
    jr z,_ManyLiterals
    sub $F0-3+1
    ; fall through

_CopyLiterals:
    ld c,a
    ld a,b ; save b
.ifdef LZSAToVRAM
    call _ldir_rom_to_vram_c_only
.else
    ld b,0
    ldir
.endif
    push de
      or a
      jp p,_CASE0xx ; can't jr on sign flag

      cp %11000000
      jr c,_CASE10x

_CASE11x:
      cp %11100000
      jr c,_CASE110

    ; "111": repeated offset
_CASE111:
      ld e,ixl
      ld d,ixh
      jp _MatchLen

_Literals0011:
    jr nz,_MoreLiterals ; unlikely?

    ; if "LL" of the byte token is equal to 0,
    ; there are no literals to copy
_NoLiterals:
    or (hl)
    inc hl
    push de
      jp m,_CASE1xx

    ; short (5 or 9 bit long) offsets
_CASE0xx:
      ld d,$FF
      cp %01000000
      jr c,_CASE00x

    ; "01x": the case of the 9-bit offset
_CASE01x:
      cp %01100000
      rl d

_ReadOffsetE:
      ld e,(hl)
      inc hl

_SaveOffset:
      ld ixl,e
      ld ixh,d

_MatchLen:
      inc a
      and %00000111
      jr z,_LongerMatch
      inc a

_CopyMatch:
      ld c,a
@useBC:
      ex (sp),hl            ; BC = len, DE = offset, HL = dest, SP ->[dest,src]
      ex de,hl
      add hl,de            ; BC = len, DE = dest, HL = dest-offset, SP->[src]
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
      ldi
      ldir
.endif
@popSrc:
    pop hl
    ; fall through

    ; compressed data stream contains records
    ; each record begins with the byte token "XYZ|LL|MMM"
_ReadToken:
    ld a,(hl)
    and %00011000
    jp pe,_Literals0011    ; process the cases 00 and 11 separately

    rrca
    rrca
    rrca

    ld c,a
    ld a,(hl)         ; token is re-read for further processing
@NEXTHLuseBC:
    inc hl
.ifdef LZSAToVRAM
    push af
      call _ldir_rom_to_vram
    pop af
.else
    ldir ; source to dest
.endif

    ; the token and literals are followed by the offset
    push de
      or a
      jp p,_CASE0xx

_CASE1xx:
      cp %11000000
      jr nc,_CASE11x

    ; "10x": the case of the 13-bit offset
_CASE10x:
      ld c,a
      ex af,af'
      jr nc,+

      ld a,(hl)
      or $F0
      ex af,af'
      ld a,(hl)
      inc hl
      or $0F
      rrca
      rrca
      rrca
      rrca

+:    ld d,a
      ld a,c
      cp %10100000
      dec d
      rl d
      jp _ReadOffsetE

    ; "110": 16-bit offset
_CASE110:
      ld d,(hl)
      inc hl
      jp _ReadOffsetE

    ; "00x": the case of the 5-bit offset
_CASE00x:
      ld c,a
      ex af,af'
      jr nc,+

      ld a,(hl)
      or $F0
      ex af,af'
      ld a,(hl)
      inc hl
      or $0F
      rrca
      rrca
      rrca
      rrca

+:    ld e,a
      ld a,c
      cp %00100000
      rl e
      jp _SaveOffset

_LongerMatch:
      scf
      ex af,af'
      jr nc,+

      ld a,(hl)
      or $F0
      ex af,af'
      ld a,(hl)
      inc hl
      or $0F
      rrca
      rrca
      rrca
      rrca

+:    sub $F0-9
      cp 15+9
      jp c,_CopyMatch ; too far for jr, likely

_LongMatch:
      add (hl)
      inc hl
      jp nc,_CopyMatch ; too far for jr, likely
      ld c,(hl)
      inc hl
      ld b,(hl)
      inc hl
      jp nz,_CopyMatch@useBC ; too far for jr, unlikely?
    pop de
    ret

.ifdef LZSAToVRAM
_ldir_rom_to_vram_c_only:
  ; copy c bytes from hl (ROM) to de (VRAM)
  ; leaves hl, de increased by c at the end
  ; does not use a
  ld b,0
  ex de,hl
    add hl,bc
  ex de,hl
  ld b,c
  ld c,$be
  otir
  ret

_ldir_rom_to_vram:
  ; copy bc bytes from hl (ROM) to de (VRAM)
  ; leaves hl, de increased by bc at the end
  ; trashes a (unlike ldir)
  
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
    ;inc d ; move de on by 256
    dec a
    jp nz,-
  pop bc
  ; If we have remainder, process it
  ld a,c
  or a
  ret z
  jp --

.endif
