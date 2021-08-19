; { "technology": "Uncompressed (fast)", "extension": "bin" }

.memorymap
defaultslot 0
slotsize $4000
slot 0 $0000
.endme

.rombankmap
bankstotal 1
banksize $4000
banks 1
.endro

.bank 0 slot 0

.org 0
	ld hl,data
	ld de,$4000
	ld bc,dataSize
	call copy
	ret ; ends the test

; This is a somewhat speed-optimised routine for emitting bc bytes from hl to VRAM address de.
; It uses a 256-byte-aligned block of outi opcodes to maximise speed.
copy:
	push bc
		ld c,$bf
		out (c),e
		out (c),d
	pop de
	dec c ; $be
	; compute the number of outis
	ld a,d
	or a
	jr z,+
-:	call outis
	dec a
	jp nz, -
	; now compute the jump for the last part
	; we want de = outis + 512 - e*2
	xor a
+:	ld d,>outis+2
	sub e
	jr nc,+
	dec d
+:	sub e
	ld e,a
	jr nc,+
	dec d
+:	push de
	ret

.section "outis" align 256
outis:
	.repeat 256
	outi
	.endr
	ret
.ends

data: .incbin "data.bin" fsize dataSize
