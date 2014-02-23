;;; Put the character in x and this will print it to the screen.
@putch:
pha
lda #$42			;Print characters at a time.
sta $2000

stx $2001
pla
rts
