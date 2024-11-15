WBMWDecompressTiles:	
		ld a, $04
		di
---:	
		push af
		push de
--:	
		ld a, (hl)
		inc hl
		or a
		jp m, +
		jr nz, ++
		pop de
		inc de
		pop af
		dec a
		jr nz, ---
		ei
		ret
	
+:	
		neg
		ld b, a
		ld c, $be
-:	
		ld a, e
		out ($bf), a
		add a, $04
		ld e, a
		ld a, d
		out ($bf), a
		adc a, $00
		ld d, a
		outi
		jr nz, -
		jr --
	
++:	
		ld b, a
		ld c, (hl)
		inc hl
-:	
		ld a, e
		out ($bf), a
		add a, $04
		ld e, a
		ld a, d
		out ($bf), a
		adc a, $00
		ld d, a
		ld a, c
		out ($be), a
		djnz -
		jr --