	.org $8000
	jmp start
	.include "sample/out.asm"

print:
	ldx #'*'		;Print a *
	sta $01			;Store a at $01
loop:
	cmp #$00		;If not zero, keep going.
	bne $08
	ldx #'\n'
	jsr @putch
	lda $01
	rts
	
	tay			;tmp a at y
	and #$01		;Check low bit
	cmp #$01
	bne $03			;If low, don't print
	jsr @putch
	tya			;Shift right and keep going.
	lsr a
	jmp loop
start:	
	lda #$40
	sta $02
	lda #$0

johnson:	
	jsr print
	tax
	eor #$01
	lsr a
	txa	
	ror a
	
	pha
	dec $02
	lda $02	
	cmp #$0
	beq $06
	sta $02
	pla
	jmp johnson
	sta $4000
	
	
