
	outloc = 0xFF00
	ramtop = 0xF000

main:
	;; push ptr to message onto stack
	L	pmsg
	S	stkw
	L	pmsg+1
	S	stkw+1
	JUMP	phw
 	JUMP	puts
	END

	
phw:	
	;; push 16-bit value onto stack
	S	ephw+1		;save return address
	SWAP
	S	ephw+2
	JUMP	dsp		;decrement SP
	L	sp
	S	phhb+1
	L	sp+1
	S	phhb+2
	L	stkw+1
phhb:	S	0
	JUMP	dsp		;decrement SP
	L	sp
	S	phlb+1
	L	sp+1
	S	phlb+2
	L	stkw
phlb:	S	0
ephw:	JUMP	0		;return to caller

	
pow:		
	;; pop 16-bit value from stack
	S	epow+1		;save return address
	SWAP
	S	epow+2
	;;
	L	sp
	S	polb+1
	L	sp+1
	S	polb+2
polb:	L	0
	S	stkw
	JUMP	isp		;increment SP
	L	sp
	S	pohb+1
	L	sp+1
	S	pohb+2
pohb:	L	0
	S	stkw+1
epow:	JUMP	0		;return to caller
	
stkw:	.=.+2
	
	;; decrement the stack pointer
dsp:
	S	dsp0+1
	SWAP
	S	dsp0+2		;save the return address
	;;
	L	c0
	SWAP			;clear carry
	L	sp
	SUBC	c1
	S	sp
	L	sp+1
	SUBC	c0
	S	sp+1
	;; 
dsp0:
	JUMP	0		;return to caller


	;; increment the stack pointer
isp:
	S	isp0+1
	SWAP
	S	isp0+2		;save the return address
	;;
	L	0
	SWAP			;clear carry
	L	c1
	ADDC	sp
	S	sp
	L	c0
	ADDC	sp+1
	S	sp+1
	;; 
isp0:	JUMP	0		;return to caller
	

puts:
	;; output a string, a ptr to which is on the stack
	S	puts1+1
	SWAP
	S	puts1+2		;save the return address
	;; 
	JUMP	pow		;pop the ptr from the stack
	L	stkw
	S	puts0+1
	L	stkw+1
	S	puts0+2
puts0:	L	0		;get character
	TEST	.+4,puts1,.+4
	S	outloc		;print the non-null character
	;; increment the pointer
	L	c0
	SWAP			;clear carry
	L	c1
	ADDC	puts0+1
	S	puts0+1
	L	c0
	ADDC	puts0+2
	S	puts0+2
	JUMP	puts0
puts1:	JUMP	0		;return to caller

c0:	0
c1:	1

msg:	"Hello world.\n",0
pmsg:	@msg
sp:	@ramtop
