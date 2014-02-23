;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;
; Purpose: This program searches a file for a string
; and prints a line if it contains the string.  The
; program will prompt for both the string and the
; file name.
;
; Contents:		Line
;
; Data: 27
; Subroutines:
;	@scan:		80
;	@print: 	152
;	@print_raw:	173
;	@open:		206
;	@strcpy:	233
;	@nl_strip:	254
;	@strcmp:	279
;	@substr:	308
; Main: 369
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Data 
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

.org $9000


message1:
.text "\nEnter a file name!\n\n"


msg1ptr:
.word message1	;This holds a pointer to the above string.


message2:
.text "Enter a string to search for!\n\n"

msg2ptr:
.word message2

message3:
.text "Here are the lines containing that string:\n\n"

msg3ptr:
.word message3

finish_nl:
.text "\n\nEnd Program\n"

finish_ptr:
.word finish_nl


;Note that you input in big endian, but the assembler converts
;to little endian.

file_name:
.word $1000  ; Store ram address $1000 here to put the name in.

search_string:
.word $1100  ; Store a search string here.

file_input:
.word $1200  ; Store the file buffer here.


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Subroutines 
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; @scan is a scan function.  Pass it a RAM pointer at $10 and it
;; will stick a scanned string of up to 255 characters there,
;; ended by the newline or the null terminator if you go too far.
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

@scan:

ldy #$00   ; Zero register y. It will be used to keep
    	   ; track of the scan destination

lda $14; Load a with the port to set the control reg to

asl a	   ; Shift the value to conform to the control reg.
asl a
asl a
asl a
asl a 

sta $2000  ; Tell the io control reg to output to the given port.


scan_begin:

lda $2001  ; Grab a byte from the port.

sta ($10),y; Stick that in the byte pointed to by $10, 
    	   ; offset by y
  
iny        ; Increment y.


cmp #$A    ; The buffer is finished at newline, or A in ascii.
    	   ; If there is an EOF($FF), check that too.

bne $7 	   ; If newline is found, store a NT and return.
lda #$0
sta ($10),y
sta $15
rts

cmp #$FF   ; Check for EOF. If so, return it and replace it with NL.
beq $3	   ; Branch to cpy.
jmp @scan_cpy

cpy #$1	   ; If the first byte is EOF, leave it
bne $3	   ; and add a NT.
jmp @scan_done


sta $15
dey	   ; Otherwise, replace EOF with NL by going back 1.
cmp #$ff
lda #$A
sta ($10),y
jmp @scan_done
@scan_cpy:

cpy #$FF   ; Compare again with y to prevent infinite loops.
bne $5	   

@scan_done:
lda #$0
sta ($10),y; Null terminate the string.
rts 	   ; Return from the subroutine.

jmp scan_begin

;;; Put the character in x and this will print it to the screen.
@putch:

lda #$01			;Print characters at a time.
sta $2000

stx $2001
rts
	
@printf:

lda #$0   ; Print to stdio
sta $2000

jsr @print_raw
rts

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; @print is a print function.  Stick a pointer to a text
;; string in $10 and make sure that you don't need registers
;; A and Y and you should be good.  It will print up to
;; 255 characters.  Also, you can control the output file by
;; sticking the port number in $14.
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

@print:

lda $14; Load a with the port to set the control reg to

asl a	   ; Shift the value to conform to the control reg's inputs.
asl a
asl a
asl a
asl a 

sta $2000  ; Tell the io control reg to output to the given port.

jsr @print_raw

rts

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; @print_raw outputs directly to io, without worrying
;; about the control registers. Just stick the pointer in 10.
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

@print_raw:

ldy #$00   ; Zero register y. It will be used to keep
    	   ; track of the printer pointer.

@begin_print:

lda ($10),y; Grab the current byte offset by y.
  
sta $2001  ; Stick that byte into the output buffer

iny        ;Increment y.

cmp #$00   ; The buffer outputs to STDOUT at \0

beq $9 	   ; This and the following cmp are equivalent
    	   ; to an AND logical, short circuiting if this
	   ; ends up false.

cpy #$FF   ; Compare again to prevent buffer overflows.
bne $6	   
lda #$41
sta $2000  ; Force the buffer to output if overflowing.


rts 	   ; Return from the subroutine.
jmp @begin_print

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; @open opens a file for you.  Stick a pointer to the name
;; in $10, the port number in $12, and the direction in $13.
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

@open:

lda $12    ; This grabs the port number

asl a 	   ; This shifts one bit to make room for the port dir.

ora $13    ; This sticks the direction in this byte.

asl a	   ; This makes room for the final control reg byte
asl a
asl a
asl a

ora #$08   ; This tells the control reg to open a port.

sta $14

sta $2000  ; Set the control reg to the assembled byte.

jsr @print_raw ; Print to the io register

rts 	   ; End the subroutine.

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; @strcpy Copy the pointer at $10 into  the one at $12.
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

@strcpy:

ldy #$0

@scpy_loop:

lda ($10),y
sta ($12),y

cmp #$0
bne $1
rts

iny
jmp @scpy_loop


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; @nl_strip Turn the first newline in a string to a NT.
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

@nl_strip:

ldy #$0

@nl_loop:

lda ($10),y

; If we have a newline, turn it to a NT and return.
cmp #$A
bne $5
lda #$0
sta ($10),y
rts

iny
jmp @nl_loop


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; @strcmp Compare two strings, pointed to by $10 and $12.
;; If they are equal, return #$0, if $10 > 12, return
;; positive, else return negative
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

@strcmp:

ldy #$0    ; Zero out y for iteration.

@scmp_loop:

lda ($12),y

sec	    ;Set the carry for two's complement addition.

sbc ($10),y; Compare the yth byte through subtraction.

;if we didn't get zero, return.
beq $1     

rts


;else check the next character.

iny
jmp @scmp_loop

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; @substr Looks if the string pointed to by $12 is a
;; substring of $10.  If they are, $15 will have a 0. Else,
;; $15 will have non-zero.
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

@substr:

;Increment $10 and $11 until we get to a NT.

;Store $10 because we are modifying that pointer.  We don't want to destroy it.

lda $10
sta $1

lda $11
sta $2

@substr_loop:

; Exit with nonzero if the pointer is at a NT(String not found).
ldy #$0

lda ($10),y
cmp #$0
bne $7
lda #$1
sta $15
jmp @endsubstr

jsr @strcmp

lda ($12),y ;Check if we've reached NT in the substring.
    	    ;If so, then it is a substring and we return 0.
cmp #$0	
bne $3
jmp @endsubstr


inc $10

;If you carried, carry into the most significant byte.
bcs $2
inc $11

jmp @substr_loop

@endsubstr:

;Store the result in $15
sta $15

; Set back the modified pointer.

lda $1
sta $10

lda $2
sta $11

rts

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Main Program
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;


.org $8000

;;;;;;;;;;;;;;;;;;;;;
;; Print 1st Prompt
;;;;;;;;;;;;;;;;;;;;;

ldx #$00
lda msg1ptr,x ; Store the first message's address in the @print's 
sta $10       ; designated pointer register.

inx
lda msg1ptr,x
sta $11

lda #$2	      ; Store the output port in $14 for @print.
sta $14

jsr @print ; Print a message asking for a file name

;;;;;;;;;;;;;;;;;;
;; Get file name
;;;;;;;;;;;;;;;;;;

ldx #$00
lda file_name,x ; Stick the scanned message here..

sta $10 	;Store the pointer in the designated register.

inx
lda file_name,x
sta $11

lda #$1		; Store the input port in $14 for @scan.
sta $14

jsr @scan

jsr @nl_strip

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; Open file of said name
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

lda #$3	      ; Store the port number in the designated
sta $12	      ; io byte.

lda #$0	      ; Store the direction(out) in the next
sta $13	      ; io byte.

jsr @open     ; Open a file of the name of the scanned string.

;;;;;;;;;;;;;;;;;;;;;
;; Print 2nd Prompt
;;;;;;;;;;;;;;;;;;;;;

lda #$2	      ; Print to stdout.
sta $14

ldx #$00      ; Stick a new string in the io pointer.
lda msg2ptr,x 
sta $10

inx
lda msg2ptr,x
sta $11

jsr @print    ; Print the second message.

;;;;;;;;;;;;;;;;;;;;;;;;
;; Get a search string 
;;;;;;;;;;;;;;;;;;;;;;;;

ldx #$00
lda search_string,x ; Stick the search string here.

sta $10

inx
lda search_string,x
sta $11

lda #$1	      ; Store the input port in $14 for @scan.
sta $14



jsr @scan

jsr @nl_strip

;;;;;;;;;;;;;;;;;;;;;
;; Announce printing:
;;;;;;;;;;;;;;;;;;;;;

lda #$2	      ; Print to stdout.
sta $14

ldx #$00      ; Stick a new string in the io pointer.
lda msg3ptr,x 
sta $10


inx
lda msg3ptr,x
sta $11


jsr @print    ; Print the message.


;;;;;;;;;;;;;;;;;;;;;;;;;;
;; Scan the file for the
;; String and print the
;; line if it is there. 
;;;;;;;;;;;;;;;;;;;;;;;;;;

;Copy the search string pointer into $12.
ldx #$00
lda search_string,x ; Stick the search string here.

sta $12

inx
lda search_string,x
sta $13
	
ldx #$00
lda file_input,x ; Stick the scanned message here..

sta $10 	;Store the pointer in the designated register.

inx
lda file_input,x
sta $11

;Begin scanning the input stuff.
scan_loop:

lda #$3		; Store the input port in $14 for @scan.
sta $14

jsr @scan

;Check for EOF at the beginning. Exit if found.

ldy #$0

lda ($10),y

cmp #$FF
bne $3
jmp end

jsr @substr

; Check if you found the string in the file.

lda $15

cmp #$0

bne $7

; Print the line.
lda #$2	;Store the output port.
sta $14
jsr @print

jmp scan_loop

end:

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; Print a final message.
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

lda #$2	      ; Print to stdout.
sta $14

ldx #$00      ; Stick a newline string in the io pointer.
lda finish_ptr,x 
sta $10

inx
lda finish_ptr,x
sta $11

jsr @print    ; Print the message.


sta $4000     ; Exit the program

