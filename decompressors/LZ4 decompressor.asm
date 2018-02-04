; LZ4 decompression algorithm - Copyright (c) 2011-2015, Yann Collet
; All rights reserved. 
; LZ4 implementation for z80 and compatible processors - Copyright (c) 2013-2015 Piotr Drapich
; All rights reserved.
; Modification for VRAM by Maxim 2018
;
; Redistribution and use in source and binary forms, with or without modification, 
; are permitted provided that the following conditions are met: 
; 
; * Redistributions of source code must retain the above copyright notice, this 
; list of conditions and the following disclaimer. 
; 
; * Redistributions in binary form must reproduce the above copyright notice, this 
; list of conditions and the following disclaimer in the documentation and/or 
; other materials provided with the distribution. 
;
; THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND 
; ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED 
; WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE 
; DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR 
; ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES 
; (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; 
; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON 
; ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
; (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS 
; SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. 
;
;; The latest version always available from http://www.union.org.pl/download/lz4/z80
:; Questions, ideas, optimization suggestions, consulting requests? Send email to union@union.pl
; 
; History:
; version 1.0 (18.09.2013)
; - initial implementation for legacy compression formats, generated with lz4 1.3.3 or earlier
; version 1.1 (28.02.2015)
; - added support for files, compressed with the lz4 version 1.4
; - Only 219 bytes of code with fully relocatable raw data decompression routine taking only 109 bytes.
; version 1.2 (10.09.2016)
; - updated to support lz4 frame format specification version 1.5.1
; - added support for files, compressed with the lz4 version up to 1.7.2
; - code reorganization and optimization. The effect - 21% boost in decompression speed. 
; Now the decompression routine is just a few percent slower than a simple loop, copying uncompressed data.

; Usage:
; _decompress
; HL - pointer to the buffer with compressed source data
; DE - pointer to the destination buffer for decompressed data
; IX - pointer to the end of the compressed source data
lz4_decompress:
  ; Set write address
  ld c,$bf
  out (c),e
  out (c),d
  
  ld b,0 ; clear b, c is set later
  ; get decompression data token
_GetToken:
  ld a,(hl) ; read token
  inc hl
  ld c,a ; store token in c for later use
  cp $10 ; check the literal length (upper 4 bits) is not 0
  jr c,_CopyMatches ; there is no literals, skip calculation of literal size
  ; unpack 4 high bits to get the length of literal
  ex af,af'
    ld a,c ; get token back 
    and $f0 ; mask other uneccessary bits
    rlca ; rotate upper 4 bits into lower part
    rlca
    rlca
    rlca
    cp $f ; if literal size <15
    jr nz, _CopyLiterals ; copy literal, else
; calculate total literal size by adding contents of following bytes
_Calcloop:
    ld c,(hl) ; get additional literal size to add 
    inc hl
    add a,c ; add it to the total literal size
    jr nc, _CalcloopNoCarry ; if size > 255
    ccf ; clear carry
    inc b ; increment b to accomodate for new size
_CalcloopNoCarry:
    inc c ; test if literal size equals 255
    jr z, _Calcloop ; yes - continue calculation

_CopyLiterals:
    ld c,a
    ; ldir ; copy literal to destination
    ; emit bc bytes from ROM to VRAM, increment hl and de
-:  ld a,(hl)
    out ($be),a
    inc hl
    inc de
    dec bc
    ld a,b
    or c
    jp nz,-
  ex af,af'
_CopyMatches: ; Copy Matches to the destination
  ex af,af' ; check for end of compressed data
    ld a,ixl
    cp l
    jr nz, _GetOffset
    ld a, ixh
    cp h
    ret z ; decompression finished
_GetOffset:
  ex af,af'
  and $f ; token can be max 15 - mask out unimportant bits. resets also c flag for sbc later
  add a,4 ; as the offset of 0 defines the size of 4 bytes, add it here 
  ld c,(hl) ; get the offset into bc
  inc hl
  ld b,(hl)
  inc hl
  push hl ; store current compressed data pointer
    ld h,d
    ld l,e
    sbc hl,bc ; calculate the new decompressed data source to copy from
    ld b,0 ; load bc with the token
    ld c,a
    cp $f+4 ; check if matchlength <15 (19 because of added 4 earlier)
    jr nz, _copymatch ; copy matches. else 
    ex (sp), hl ; get current compressed pointer into hl
; calculate total matches size by adding contents of following bytes
_Calcloop2:
    ld c,(hl) ; get additional match sizes to add
    inc hl
    add a,c ; add it to the total match size
    jr nc, _CalcloopNoCarry2 ; if size > 255
    ccf ; clear carry
    inc b ; increment b to accomodate for new size
_CalcloopNoCarry2:
    inc c ; test if match size equals 255
    jr z, _Calcloop2 ; yes - continue calculation
    ld c,a
    ex (sp), hl ; store current compressed data pointer on stack

_copymatch:
    call _ldir_vram_to_vram ; ldir ; copy match
  pop hl ; restore current compressed data pointer from stack
  jr _GetToken ; continue decompression

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
