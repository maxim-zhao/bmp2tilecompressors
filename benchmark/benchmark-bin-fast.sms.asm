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
	ld hl,$4000
	ld de,$4000
	ld bc,8416
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
+:	sla e
	sub e ; a = -e*2, carry is set if e > 255
	ret z ; e was 0
	ld e,a
	ld a,>outis+2
	sbc 1 ; this captures the carry from earlier
	ld d,a
	push de
	ret

.section "outis" align 256
outis:
	.repeat 256
	outi
	.endr
	ret
.ends
