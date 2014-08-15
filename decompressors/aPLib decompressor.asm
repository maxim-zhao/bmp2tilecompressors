; Usage:
;
; 1. Define a symbol "aPLibMemory" which points to the start of 5 bytes of RAM.
; 2. If you want to decompress to VRAM:
;    .define aPLibToVRAM
; 3. .include this file in your code
; 4. ld hl,<source address>
;    ld de,<destination address> ; e.g. $4000 for VRAM address 0
;    call aPLib_decompress
;
; The stack is used a bit, I never saw more than 12 bytes used.
; ROM usage is 297 bytes in VRAM mode, 242 in RAM mode. The extra bytes are the cost 
; of VRAM to VRAM copies, which also makes it pretty slow.
; This file is using WLA-DX syntax quite heavily, you'd better use it too...

.define calcblocks
; comment out this line to suppress the block size notifications
; (useful for optimising to see size changes)

.struct aPLibMemoryStruct
bits     db
byte     db ; not directly referenced, assumed to come after bits
LWM      db
R0       dw
.endst

.enum aPLibMemory
mem instanceof aPLibMemoryStruct
.ende

; Reader's note:
; The structure of the code has been arranged such that the entry point is in the middle -
; this is so it can use jr to branch out to the various subsections to save a few bytes,
; but it makes it somewhat harder to read. "depack" is the entry point and "aploop" is
; the main loop.

.section "aPLib" free
.ifdef calcblocks
.block "aPLib"
.endif
_ap_getbit:
	push bc
		ld bc,(mem.bits)
		rrc c
		jr nc,+
		ld b,(hl)
		inc hl
+:	ld a,c
		and b
		ld (mem.bits),bc
	pop bc
	ret

_ap_getbitbc: ;doubles BC and adds the read bit
	sla c
	rl b
	call _ap_getbit
	ret z
	inc bc
	ret

_ap_getgamma:
	ld bc,1
-:call _ap_getbitbc
	call _ap_getbit
	jr nz,-
	ret

_apbranch1:
.ifdef aPLibToVRAM
  ld a,(hl)
  out ($be),a
  inc hl
  inc de
.else
	ldi
.endif
	xor a
	ld (mem.LWM),a
	jr _aploop

_apbranch2:
	;use a gamma code * 256 for offset, another gamma code for length
	call _ap_getgamma
	dec bc
	dec bc
	ld a,(mem.LWM)
	or a
	jr nz,++
	;bc = 2? ; Maxim: I think he means 0
	ld a,b
	or c
	jr nz,+
	;if gamma code is 2, use old R0 offset, and a new gamma code for length
	call _ap_getgamma
	push hl
		ld h,d
		ld l,e
		push bc
			ld bc,(mem.R0)
			sbc hl,bc
		pop bc
.ifdef aPLibToVRAM
    call _ldir_vram_to_vram
.else
    ldir
.endif
	pop hl
	jr +++
  
+:dec bc
++:
	;do I even need this code? ; Maxim: seems so, it's broken without it
	;bc=bc*256+(hl), lazy 16bit way
	ld b,c
	ld c,(hl)
	inc hl
	ld (mem.R0),bc
	push bc
		call _ap_getgamma
		ex (sp),hl
		;bc = len, hl=offs
		push de
			ex de,hl
			;some comparison junk for some reason
			; Maxim: optimised to use add instead of sbc
      ld hl,-32000
      add hl,de
			jr nc,+
			inc bc
+:    ld hl,-1280
      add hl,de
			jr nc,+
			inc bc
+:    ld hl,-128
      add hl,de
			jr c,+
			inc bc
			inc bc
+:		;bc = len, de = offs, hl=junk
		pop hl
		push hl
			or a
			sbc hl,de
		pop de
		;hl=dest-offs, bc=len, de = dest
.ifdef aPLibToVRAM
    call _ldir_vram_to_vram
.else
		ldir
.endif
	pop hl
+++:
	ld a,1
	ld (mem.LWM),a
	jr _aploop

aPLib_decompress:
	;hl = source
	;de = dest (VRAM address with write bit set)
  ld c,$bf
  out (c),e
  out (c),d

	; ldi
  ld a,(hl)
  out ($be),a
  inc hl
  inc de
  
	xor a
	ld (mem.LWM),a
	inc a
	ld (mem.bits),a

_aploop:
	call _ap_getbit
	jr z, _apbranch1
	call _ap_getbit
	jr z, _apbranch2
	call _ap_getbit
	jr z, _apbranch3
	;LWM = 0
	xor a
	ld (mem.LWM),a
	;get an offset
	ld bc,0
	call _ap_getbitbc
	call _ap_getbitbc
	call _ap_getbitbc
	call _ap_getbitbc
	ld a,b
	or c
	jr nz, _apbranch4
;	xor a  ;write a 0 ; Maxim: a is zero already (just failed nz test), optimise this line away

.ifdef aPLibToVRAM
  out ($be),a
.else
  ld (de),a
.endif
  inc de

	jr _aploop

_apbranch4:
	ex de,hl ;write a previous bit (1-15 away from dest)
	push hl
		sbc hl,bc
.ifdef aPLibToVRAM
    ld c,$bf
    out (c),l
    ld a,h
    xor $40
    out (c),a
    in a,($be)
.else
		ld a,(hl)
.endif
	pop hl
.ifdef aPLibToVRAM
  out (c),l
  out (c),h
  out ($be),a
.else
	ld (hl),a
.endif
	inc hl
	ex de,hl
	jr _aploop
_apbranch3:
	;use 7 bit offset, length = 2 or 3
	;if a zero is encountered here, it's EOF
	ld c,(hl)
	inc hl
	rr c
	ret z
	ld b,2
	jr nc,+
	inc b
+:;LWM = 1
	ld a,1
	ld (mem.LWM),a

	push hl
		ld a,b
		ld b,0
		;R0 = c
		ld (mem.R0),bc
		ld h,d
		ld l,e
		or a
		sbc hl,bc
		ld c,a
.ifdef aPLibToVRAM
    call _ldir_vram_to_vram
.else
		ldir
.endif
	pop hl
	jr _aploop
  
.ifdef aPLibToVRAM
_ldir_vram_to_vram:
  ; Copy bc bytes from VRAM address hl to VRAM address de
  ; Both hl and de are "write" addresses ($4xxx)

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
  ret
  
/*
  ; Make hl a read address
  ld a,h
  xor $40
  ld h,a
-:; Read a byte
  ld a,l
  out ($bf),a
  ld a,h
  out ($bf),a
  in a,($be)
  push af
    ; Write it
    ld a,e
    out ($bf),a
    ld a,d
    out ($bf),a
  pop af
  out ($be),a
  ; Increment
  inc hl
  inc de
  dec bc
  ; Loop
  ld a,b
  or c
  jr nz,-
*/
  ret
.endif
  
.ifdef calcblocks
.endb
.endif
.ends
