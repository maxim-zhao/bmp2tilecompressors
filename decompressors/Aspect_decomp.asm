;
; Legend of Illusion (Aspect) compression decompressor
;
; IN: HL = compressed data
;     DE = VRAM address for decompression, with WRITE flag
;
; CLOBBERS: A,Flags,BC,HL,DE,IY
;
; code based on original code found in Legend of Illusion SMS ROM
;                                                          by sverx\2024
;

.ramsection "variables" slot 3 free
  buffer     dsb 32           ; the 32 bytes buffer for the decompression/deXORing
  tile_count dw               ; how many tiles we have to transfer
  comp_tab   dw               ; pointer to the compression modes table
  part_cnt   db               ; count which bits of the byte we've got to consider
.ends

.section "aspect_decomp" free
aspect_decomp:
    ld a,e                    ; set VDPAddress from DE
    di                        ; make sure no interrupt happen between the two OUT
		out ($bf),a
		ld a,d
		out ($bf),a
    ei

    push hl
		    inc hl                ; skip two bytes (was that the version?)
		    inc hl
		    ld a,(hl)
		    ld (tile_count),a     ; tile counter LOW
		    inc hl
		    ld a,(hl)
		    ld (tile_count+1),a   ; tile counter HIGH
		    inc hl
		    ld e,(hl)             ; compression modes stream offset LOW
		    inc hl
		    ld d,(hl)             ; compression modes stream offset HIGH
		    inc hl
		    push hl               ; preserve the pointer to source data
        pop iy
		pop hl
		add hl,de
    ld (comp_tab),hl          ; save the pointer to compression modes stream (two bits per tile, from right to left)

    xor a
    ld (part_cnt),a           ; zeroes the part counter

    push iy
    pop hl                    ; retrieve the pointer to source data

_tile_loop:
    push hl                   ; preserve the pointer to source data
        ld a,(part_cnt)       ; load part counter (0-3)
        cp 4
		    jr nz,+
		    ld hl,(comp_tab)      ; if 4, advance comp tab stream
		    inc hl
		    ld (comp_tab),hl
		    xor a
		    ld (part_cnt),a       ; reset part counter
+:
		    ld b,a
		    ld hl,(comp_tab)      ; get the compression mode byte from the table
		    ld a,(hl)
-:
		    dec b
		    jp m,+                ; rotate right part_cnt*2 times
		    rrca
		    rrca
		    jp -

+:
		    and $03               ; isolate the rightmost 2 bits -> this is the compression mode for the tile (0 to 3)
        ld hl,part_cnt
        inc (hl)              ; increment the part counter for next time

    pop hl                    ; retrieve the pointer to source data
    dec a
    jr z,_mode_1
    dec a
    jr z,_mode_2
    dec a
    jr z,_mode_3

    ld b,32                   ; mode 0: write 32 zeroes to VRAM
    xor a
-:
    out ($be),a               ; VRAM timing ***NOT SAFE*** for SMS/GG here
    djnz -
    jp _loop_end

_mode_1:                      ; mode 1: uncompressed data in the stream, copy that to VRAM
    ld b,32
-:
    ld a,(hl)
		out ($be),a               ; VRAM timing SAFE
		inc hl
		djnz -
    jp _loop_end

_mode_2:
    call _depack_to_buffer
    jp _copy_buffer_to_VRAM

_mode_3:
    call _depack_to_buffer

    ld iy,buffer              ; dexor buffer
		ld b,7
-:
		ld a,(iy+0)
		xor (iy+2)
		ld (iy+2),a
		ld a,(iy+1)
		xor (iy+3)
		ld (iy+3),a
		ld a,(iy+16)
		xor (iy+18)
		ld (iy+18),a
		ld a,(iy+17)
		xor (iy+19)
		ld (iy+19),a
		inc iy
		inc iy
		djnz -

_copy_buffer_to_VRAM:
    ld de,buffer
    ld b,32
-:
    ld a,(de)
		out ($be),a               ; VRAM timing SAFE
		inc de
		djnz -

_loop_end:
    push hl
        ld hl,(tile_count)
        dec hl
        ld (tile_count),hl
        ld a,h
        or a,l
    pop hl
    ret z
    jp _tile_loop

_depack_to_buffer:
    ld iy,buffer
		ld e,(hl)                 ; read 4 bytes and create the bit pattern in BCDE
		inc hl
		ld d,(hl)
		inc hl
		ld c,(hl)
		inc hl
		ld b,(hl)
		inc hl
		ld a,32                   ; loop 32 times
-:
		push af
	      rr b                  ; rotate right BCDE
		    rr c
		    rr d
		    rr e
		    jr c,+                ; if carry bit = 1, then copy byte from stream, else write zero
		    ld (iy+0),0
		    jr ++

+:
        ld a,(hl)
		    ld (iy+0),a
		    inc hl

++:
		    inc iy
		pop af
		dec a
		jr nz,-
		ret
.ends
