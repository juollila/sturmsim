	.org $0100-2
	.word $100

chrout = $ffd2

	ldx	#0
loop:
	lda	text,x
	beq	end
	jsr	chrout
	inx
	bne	loop
end:
	brk
text:	.byte	"HELLO, WORLD!", $d, 0
	
	rts
