.define RNC_HEADER_SIZE 17

.enum RNC_MEMORY
  rawtab  dsb $80
  postab  dsb $80
  slntab  dsb $80
  tmptab  dsb $10
  regx    db
  regy    db
  bitbufl dw
  bitbufh dw
  rncdat  dsb 4
  InPtr   dw ; input data pointer
  OutPtr  dw ; output data pointer
  hufftab dw

  cartdat dw
  counts  dw
  blocks  db
  bufbits db
  bitlen  db
  hufcde  dw
  hufbse  dw
  temp1   db
  temp2   db
  temp3   db
.ende

; ****** Unpack RNC format 1 data ******
; Entry: HL = Source packed data
;  DE = Destination for unpacked data

Unpack:
  ld a, e
  ld (OutPtr), a
  ld a, d
  ld (OutPtr+1), a

  ld bc, RNC_HEADER_SIZE
  add hl, bc

  ld a, (hl)
  inc hl
  ld (blocks), a

  xor a
  ld (bufbits), a

  ld a, (hl)
  inc hl
  ld (bitbufl), a

  ld a, (hl)
  inc hl
  ld (bitbufl+1), a

  ld a, l
  ld (InPtr), a
  ld a, h
  ld (InPtr+1), a

  ld a, 2
  call _GetBits

_rnc01:
  ld a, rawtab#256
  ld (hufftab), a
  ld a, rawtab/256
  ld (hufftab+1), a

  call _MakeHuff

  ld a, postab#256
  ld (hufftab), a
  ld a, postab/256
  ld (hufftab+1), a

  call _MakeHuff

  ld a, slntab#256
  ld (hufftab), a
  ld a, slntab/256
  ld (hufftab+1), a

  call _MakeHuff

  ld a, 16
  call _GetBits

  ld a, (rncdat+1)
  ld b, a

  ld a, (rncdat)
  ld (counts), a

  or a
  jr z, _rnc02

  inc b
_rnc02:

  ld a, b
  ld (counts+1), a

  jp _rnc10

_rnc03:
  ld a, postab#256
  ld (hufftab), a
  ld a, postab/256
  ld (hufftab+1), a

  call _GetVal

  ld a, (rncdat)
  ld c, a
  ld a, (rncdat+1)
  ld b, a

  ld a, (OutPtr)
  scf
  sbc c
  ld (temp1), a
  ld a, (OutPtr+1)
  sbc b
  ld (temp2), a

  ld a, slntab#256
  ld (hufftab), a
  ld a, slntab/256
  ld (hufftab+1), a

  call _GetVal

  ld a, (rncdat+1)
  ld b, a

  ld a, (rncdat)
  add 2
  ld c, a

  jr z, _rnc04

  inc b

_rnc04:
  jr nc, _rnc05

  inc b
_rnc05:
  ld a, (temp1)
  ld l, a
  ld a, (temp1+1)
  ld h, a

  ld a, (OutPtr)
  ld e, a
  ld a, (OutPtr+1)
  ld d, a

_rnc06:
  ld a, (hl)
  inc hl
  ld (de), a
  inc de

  dec c
  jr z, _rnc06a

  ld a, (hl)
  inc hl
  ld (de), a
  inc de

  dec c
  jr nz, _rnc06

_rnc06a:
  dec b
  jr nz, _rnc06

  ld a, e
  ld (OutPtr), a
  ld a, d
  ld (OutPtr+1), a

_rnc10:
  ld a, rawtab#256
  ld (hufftab), a
  ld a, rawtab/256
  ld (hufftab+1), a

  call _GetVal

  ld a, (rncdat)
  ld b, a

  ld a, (rncdat+1)
  or b
  jp z, _rnc22


  ld a, (rncdat+1)
  ld b, a

  ld a, (rncdat)
  ld c, a
  or a
  jr z, _rnc12

  inc b
_rnc12:
  ld a, b
  ld (rncdat+1), a

  ld b, 3
  ld hl, cartdat

  ld a, (OutPtr)
  ld e, a
  ld a, (OutPtr+1)
  ld d, a

_rnc13:
  dec b
  jr z, _rnc14

  ld a, (hl)
  inc hl
  ld (de), a
  inc de

  dec c
  jr nz, _rnc13

  ld a, (rncdat+1)
  dec a
  ld (rncdat+1), a

  jr nz, _rnc13

  dec b
  jr z, _rnc17

  ld a, (cartdat+1)
  jr _rnc18

_rnc14:
  ld a, (InPtr)
  ld l, a
  ld a, (InPtr+1)
  ld h, a

  ld a, (rncdat+1)
  ld b, a

_rnc14a:
  ld a, (hl)
  inc hl
  ld (de), a
  inc de

  dec c
  jr z, _rnc14b

  ld a, (hl)
  inc hl
  ld (de), a
  inc de

  dec c
  jr nz, _rnc14a

_rnc14b:
  dec b
  jr nz, _rnc14a

  ld a, b
  ld (rncdat+1), a

  ld a, l
  ld (InPtr), a
  ld a, h
  ld (InPtr+1), a

_rnc17:
  ld a, (InPtr)
  ld l, a
  ld a, (InPtr+1)
  ld h, a

  ld a, (hl)
  inc hl
  ld (temp1), a
  ld (cartdat), a

  jr _rnc18a

_rnc18:
  ld (temp1), a
  ld (cartdat), a

  ld a, (InPtr)
  ld l, a
  ld a, (InPtr+1)
  ld h, a

_rnc18a:
  ld a, (hl)
  inc hl
  ld (temp2), a
  ld (cartdat+1), a

  ld a, l
  ld (InPtr), a
  ld a, h
  ld (InPtr+1), a

  ld a, e
  ld (OutPtr), a
  ld a, d
  ld (OutPtr+1), a

_rnc19:
  ld a, (temp1)
  ld e, a
  ld a, (temp2)
  ld d, a

  ld hl, 0

  ld a, (bufbits)
  or a
  jr z, _rnc21

  ld b, a
_rnc20:
  sla e
  rl d
  rl l
  rl h

  dec b
  jr nz, _rnc20

_rnc21:
  ld a, l
  ld (bitbufh), a
  ld a, h
  ld (bitbufh+1), a

  ld a, (bitbufl)
  ld c, a
  ld a, (bitbufl+1)
  ld b, a

  ld a, (bufbits)
  add a

  ld l, _msktab#256
  add l
  ld l, a
  ld a, _msktab/256
  adc 0
  ld h, a

  ld a, (hl)
  and c
  or e
  ld (bitbufl), a

  inc hl

  ld a, (hl)
  and b
  or d
  ld (bitbufl+1), a
_rnc22:
  ld hl, counts
  dec (hl)

  jp nz, _rnc03

  ld hl, counts+1
  dec (hl)

  jp nz, _rnc03


  ld hl, blocks
  dec (hl)

  jp nz, _rnc01

  ret ; Done!!!!


;**********************************

_GetVal:
  ld a, (hufftab)
  ld l, a
  ld a, (hufftab+1)
  ld h, a

  dec hl
_GetVal2:

  inc hl

_GetVal3:
  ld a, (bitbufl)
  and (hl)
  ld b, a
  ld (rncdat), a

  inc hl

  ld a, (bitbufl+1)
  and (hl)
  ld c, a
  ld (rncdat+1), a

  inc hl

  ld a, b
  cp (hl)
  inc hl

  jr nz, _GetVal2

  ld a, c
  cp (hl)
  inc hl

  jr nz, _GetVal3

  ld a, l
  add 15*4
  ld l, a
  jr nc, _nocarry
  inc h
_nocarry:

  ld a, (hl)
  inc hl
  push af

  ld a, (hl)

  call _GetBits

  pop af
  cp 2
  jr nc, _GetVal4

  ld (rncdat), a

  ld hl, rncdat+1
  ld (hl), 0

  ret

_GetVal4:
  dec a
  push af
  call _GetBits
  pop af

  cp 8
  jr c, _GetVal5

  ld l, (bittabl-8)#256
  add l
  ld l, a
  ld a, (bittabl-8)/256
  adc 0
  ld h, a

  ld a, (rncdat+1)
  or (hl)
  ld (rncdat+1), a

  ret

_GetVal5:
  ld l, (bittabl)#256
  add l
  ld l, a
  ld a, (bittabl)/256
  adc 0
  ld h, a

  ld a, (rncdat)
  or (hl)
  ld (rncdat), a
  ret

bittabl:
  .db 1, 2, 4, 8, 16, 32, 64, 128

;*********************************

_GetBits:
  ld (regx), a
  add a

  ld l, _msktab#256
  add l
  ld l, a
  ld a, _msktab/256
  adc 0
  ld h, a

  ld a, (bitbufh)
  ld e, a
  ld a, (bitbufh+1)
  ld d, a

  ld a, (bitbufl)
  ld c, a
  and (hl)
  ld (rncdat), a

  inc hl

  ld a, (bitbufl+1)
  ld b, a
  and (hl)
  ld (rncdat+1), a

  ld a, (bufbits)
  or a
  jr nz, _GetBits4

_GetBits3:

  ld a, (InPtr)
  ld l, a
  ld a, (InPtr+1)
  ld h, a

  ld a, (hl)
  inc hl
  ld (cartdat), a
  ld e, a

  ld a, (hl)
  inc hl
  ld (cartdat+1), a
  ld d, a

  ld a, l
  ld (InPtr), a
  ld a, h
  ld (InPtr+1), a

  ld a, 16
  ld (bufbits), a

_GetBits4:

  ld a, (regx)
  ld hl, bufbits

_GetBits4a:
  srl d
  rr e
  rr b
  rr c

  dec a
  jr z, _GetBits5

  dec (hl)
  jr nz, _GetBits4a

  ld (regx), a

  jr _GetBits3

_GetBits5:
  ld a, c
  ld (bitbufl), a
  ld a, b
  ld (bitbufl+1), a

  ld a, e
  ld (bitbufh), a
  ld a, d
  ld (bitbufh+1), a

  ld hl, bufbits
  dec (hl)
  ret

_msktab:
  .dw 0, 1, 3, 7, $f, $1f, $3f, $7f, $ff, $1ff, $3ff
  .dw $7ff, $fff, $1fff, $3fff, $7fff, $ffff

;***********************

_MakeHuff:
  ld a, 5
  call _GetBits

  ld a, (rncdat)
  or a
  ret z

  ld (temp1), a
  ld (temp2), a

  ld hl, tmptab

_MakeHuff2:
  push hl
  ld a, 4
  call _GetBits

  ld hl, temp2
  dec (hl)

  pop hl

  ld a, (rncdat)
  ld (hl), a
  inc hl

  jr nz, _MakeHuff2

  xor a
  ld (regy), a
  ld (hufcde), a
  ld (hufcde+1), a
  ld (hufbse), a

  inc a

  ld (bitlen), a

  ld a, $80
  ld (hufbse+1), a
_MakeHuff3:
  ld a, (temp1)
  ld (temp2), a

  xor a
  ld (temp3), a

_MakeHuff4:
  ld a, (temp3)
  ld (regx), a

  ld l, tmptab#256
  add l
  ld l, a
  ld a, tmptab/256
  adc 0
  ld h, a

  ld a, (bitlen)
  cp (hl)
  jp nz, _MakeHuff8

  ld (regx), a
  add a

  ld l, _msktab#256
  add l
  ld l, a
  ld a, _msktab/256
  adc 0
  ld h, a

  ld b, (hl)

  ld a, (regy)
  ld c, a
  add 2
  ld (regy), a

  ld a, (hufftab)
  add c
  ld e, a
  ld a, (hufftab+1)
  adc 0
  ld d, a

  ld a, b
  ld (de), a

  inc hl
  inc de

  ld a, (hl)
  ld (de), a

  ld a, (rncdat)
  ld c, a
  ld a, (rncdat+1)
  ld b, a

  ld a, (hufcde)
  ld e, a

  ld a, (hufcde+1)
  ld d, a

  ld a, (regx)

_MakeHuff5:
  sla e
  rl d
  rr b
  rr c

  dec a
  jr nz, _MakeHuff5

  ld hl, rncdat+3
  ld a, d
  ld (hl), a
  dec hl
  ld a, e
  ld (hl), a
  dec hl

  ld a, (bitlen)
  ld e, a

  ld a, 16
  sub e
  jr z, _MakeHuff7

  ld d, a

_MakeHuff6:
  srl b
  rr c

  dec d
  jr nz, _MakeHuff6

_MakeHuff7:
  ld a, b
  ld (hl), a
  dec hl
  ld a, c
  ld (hl), a

  ld a, (regy)
  ld b, a
  add 2
  ld (regy), a

  ld a, (hufftab)
  add b
  ld l, a
  ld a, (hufftab+1)
  adc 0
  ld h, a

  ld a, (rncdat)
  ld (hl), a
  inc hl

  ld a, (rncdat+1)
  ld (hl), a
  inc hl

  ld a, l
  add 15*4
  ld l, a

  jr nc, _nocarry2
  inc h
_nocarry2:

  ld a, (temp3)
  ld (hl), a

  inc hl

  ld a, (bitlen)
  ld (hl), a

  ld a, (hufbse)
  ld c, a
  ld a, (hufbse+1)
  ld b, a

  ld a, (hufcde)
  add c
  ld (hufcde), a
  ld a, (hufcde+1)
  adc b
  ld (hufcde+1), a

_MakeHuff8:
  ld hl, temp3
  inc (hl)

  ld hl, temp2
  dec (hl)
  jp nz, _MakeHuff4

  ld hl, hufbse+1
  srl (hl)
  dec hl
  rr (hl)

  ld a, (bitlen)
  inc a
  ld (bitlen), a

  cp 17
  jp nz, _MakeHuff3

  ret
