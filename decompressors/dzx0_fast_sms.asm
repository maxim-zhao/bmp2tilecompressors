;
;  Speed-optimized ZX0 decompressor by spke (191 bytes)
;
;  ver.00 by spke (27/01-23/03/2021, 191 bytes)
;  ver.01 by spke (24/03/2021, 193(+2) bytes - fixed a bug in the initialization)
;  ver.01patch2 by uniabis (25/03/2021, 191(-2) bytes - fixed a bug with elias over 8bits)
;  ver.01patch5 by uniabis (29/03/2021, 191 bytes - a bit faster)
;
;  Original ZX0 decompressors were written by Einar Saukas
;
;  This decompressor was written on the basis of "Standard" decompressor by
;  Einar Saukas and optimized for speed by spke. This decompressor is
;  about 5% faster than the "Turbo" decompressor, which is 128 bytes long.
;  It has about the same speed as the 412 bytes version of the "Mega" decompressor.
;  
;  The decompressor uses AF, AF', BC, DE, HL and IX and relies upon self-modified code.
;
;  The decompression is done in the standard way:
;
;  ld hl,FirstByteOfCompressedData
;  ld de,FirstByteOfMemoryForDecompressedData
;  call DecompressZX0
;
;  Of course, ZX0 compression algorithms are (c) 2021 Einar Saukas,
;  see https://github.com/einar-saukas/ZX0 for more information
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

.define PrevOffset $c010
DecompressZX0:
.ifdef ZX0ToVRAM
  ; Set VRAM address
  ld c,$bf
  out (c),e
  out (c),d
.endif
        scf 
        ex      af, af'
        ld      ix, CopyMatch1
        ld      bc, $ffff
        ld      (PrevOffset), bc      ; default offset is -1
        inc     bc
        ld      a, $80
        jr      RunOfLiterals           ; BC is assumed to contains 0 most of the time

        ; 7-bit offsets allow additional optimizations, based on the facts that C==0 and AF' has C ON!
ShorterOffsets:
        ex      af, af'
        sbc     a, a
        ld      (PrevOffset+1), a       ; the top byte of the offset is always $FF
        ld      a, (hl)
        inc     hl
        rra
        ld      (PrevOffset), a       ; note that AF' always has flag C ON
        jr      nc, LongerMatch

CopyMatch2:                             ; the case of matches with len=2
        ex      af, af'
        ld      c, 2

        ; the faster match copying code
CopyMatch1:
        push    hl                      ; preserve source

        ld      hl, (PrevOffset)        ; restore offset (default offset is -1)
        add     hl, de                  ; HL = dest - offset
.ifdef ZX0ToVRAM
        call _ldir_vram_to_vram
.else
        ldir
.endif
        pop     hl                      ; restore source

        ; after a match you can have either
        ; 0 + <elias length> = run of literals, or
        ; 1 + <elias offset msb> + [7-bits of offset lsb + 1-bit of length] + <elias length> = another match
AfterMatch1:
        add     a, a
        jr      nc, RunOfLiterals

UsualMatch:                             ; this is the case of usual match+offset
        add     a, a
        jr      nc, LongerOffets
        jr      nz, ShorterOffsets      ; NZ after NC == "confirmed C"
        
        ld      a, (hl)                 ; reload bits
        inc     hl
        rla

        jr      c, ShorterOffsets

LongerOffets:
        inc     c

-:      add     a, a                    ; inline read gamma
        rl      c
        add     a, a
        jr      nc, -

        call    z, ReloadReadGamma

ProcessOffset:
        ex      af, af'
        xor     a
        sub     c
        ret     z                       ; end-of-data marker (only checked for longer offsets)
        rra
        ld      (PrevOffset+1),a
        ld      a, (hl)
        inc     hl
        rra
        ld      (PrevOffset), a

        ; lowest bit is the first bit of the gamma code for length
        jr      c, CopyMatch2

        ; this wastes 1 t-state for longer matches far away,
        ; but saves 4 t-states for longer nearby (seems to pay off in testing)
        ld      c, b
LongerMatch:
        inc     c
        ; doing SCF here ensures that AF' has flag C ON and costs
        ; cheaper than doing SCF in the ShortestOffsets branch
        scf
        ex      af, af'

-:      add     a, a                    ; inline read gamma
        rl      c
        add     a, a
        jr      nc, -

        call    z,ReloadReadGamma

CopyMatch3:
        push    hl                      ; preserve source
        ld      hl, (PrevOffset)        ; restore offset
        add     hl, de                  ; HL = dest - offset

.ifdef ZX0ToVRAM
        inc bc
        call _ldir_vram_to_vram
.else
        ; because BC>=3-1, we can do 2 x LDI safely
        ldi
        ldir
        inc     c
        ldi
.endif

        pop     hl                      ; restore source

        ; after a match you can have either
        ; 0 + <elias length> = run of literals, or
        ; 1 + <elias offset msb> + [7-bits of offset lsb + 1-bit of length] + <elias length> = another match
AfterMatch3:
        add     a, a
        jr      c, UsualMatch

RunOfLiterals:
        inc     c
        add     a, a
        jr      nc, LongerRun
        jr      nz, CopyLiteral         ; NZ after NC == "confirmed C"
        
        ld      a, (hl)                 ; reload bits
        inc     hl
        rla

        jr      c, CopyLiteral

LongerRun:
-:      add     a, a                    ; inline read gamma
        rl      c
        add     a, a
        jr      nc, -

        jr      nz, CopyLiterals
        
        ld      a, (hl)                 ; reload bits
        inc     hl
        rla

        call    nc, ReadGammaAligned


.ifdef ZX0ToVRAM
CopyLiterals:
CopyLiteral:
        call _ldir_rom_to_vram
.else
CopyLiterals:
        ldi
CopyLiteral:
        ldir
.endif

        ; after a literal run you can have either
        ; 0 + <elias length> = match using a repeated offset, or
        ; 1 + <elias offset msb> + [7-bits of offset lsb + 1-bit of length] + <elias length> = another match
        add     a, a
        jr      c, UsualMatch

RepMatch:
        inc     c
        add     a, a
        jr      nc, LongerRepMatch
        jr      nz, CopyMatch1          ; NZ after NC == "confirmed C"
        
        ld      a, (hl)                 ; reload bits
        inc     hl
        rla

        jr      c, CopyMatch1

LongerRepMatch:
-:      add     a, a                    ; inline read gamma
        rl      c
        add     a, a
        jr      nc, -

        jp      nz, CopyMatch1

        ; this is a crafty equivalent of CALL ReloadReadGamma : JP CopyMatch1
        push    ix

        ;  the subroutine for reading the remainder of the partly read Elias gamma code.
        ;  it has two entry points: ReloadReadGamma first refills the bit reservoir in A,
        ;  while ReadGammaAligned assumes that the bit reservoir has just been refilled.
ReloadReadGamma:
        ld      a, (hl)                 ; reload bits
        inc     hl
        rla

        ret     c
ReadGammaAligned:
        add     a, a
        rl      c
        add     a, a
        ret     c
        add     a, a
        rl      c
        add     a, a

ReadingLongGamma:                       ; this loop does not need unrolling, as it does not get much use anyway
        ret     c
        add     a, a
        rl      c
        rl      b
        add     a, a
        jr      nz, ReadingLongGamma

        ld      a, (hl)                 ; reload bits
        inc     hl
        rla

        jr      ReadingLongGamma

.ifdef ZX0ToVRAM
_ldir_vram_to_vram:
  ; Copy bc bytes from VRAM address hl to VRAM address de
  ; Both hl and de are "write" addresses ($4xxx)
  push af
  ; Make hl a read address
  ld a,h
  xor $40
  ld h,a
  ; Check if the count is below 256
  ld a,b
  or a
  jr z,_below256
  ; Else emit 256*b bytes
-:push bc
    ld c,$bf
    ld b,0
    call +
  pop bc
  djnz -
  ; Then fall through for the rest  
_below256:
  ; By emitting 256 at a time, we can use the out (c),r opcode
  ; for address setting, which then relieves pressure on a
  ; and saves some push/pops; and we can use djnz for the loop.
  ld b,c
  ld c,$bf
+:
-:out (c),l
  out (c),h
  in a,($be)
  out (c),e
  out (c),d
  out ($be),a
  inc hl
  inc de
  djnz -
  ld c,0
  pop af
  ret

_ldir_rom_to_vram:
  ; copy bc bytes from hl (ROM) to de (VRAM)
  ; leaves hl, de increased by bc at the end
  ; leaves bc=0 at the end
  push af
  
    ; add bc to de to mimic ldir
    ex de,hl
      add hl,bc
    ex de,hl

    ; If b = 0, we can short-circuit. We optimise for this as we expect mostly short runs.
    ld a,b
    or a
    jr nz,+ ; unlikely

--: ld b,c
    ld c,$be
    otir
---:
  pop af
  ld c,0
  ret

+:  push bc
      ; We copy b*256 bytes
      ld c,$be
      ld b,0
-:    otir
      inc d ; move de on by 256
      dec a
      jp nz,-
    pop bc
    ; If we have remainder, process it
    ld a,c
    or a
    jr z,--- ; unlikely
    jp --

.endif
