
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
	S	edsp+1
	SWAP
	S	edsp+2		;save the return address
	L	sp
	SUB	one
	S	sp
	SWAP
	ADD	sp+1
	S	sp+1
edsp:	JUMP	0		;return to caller


	;; increment the stack pointer
isp:
	S	eisp+1
	SWAP
	S	eisp+2		;save the return address
	L	sp
	ADD	sp
	S	sp
	SWAP
	ADD	sp+1
	S	sp+1
eisp:	JUMP	0		;return to caller
	

puts:
	;; output a string, a ptr to which is on the stack
	S	eputs+1
	SWAP
	S	eputs+2		;save the return address
	JUMP	pow		;pop the ptr from the stack
	L	stkw
	S	ckb+1
	L	stkw+1
	S	ckb+2
ckb:	L	0		;get character
	TEST	pch,eputs,pch
pch:	S	outloc		;print the non-null character
	;; increment the pointer
	L	ckb+1
	ADD	one
	S	ckb+1
	SWAP
	ADD	ckb+2
	S	ckb+2
	JUMP	ckb
eputs:	JUMP	0		;return to caller
	
zero:	0
one:	1
two:	2

msg:	"Hello world.\n",0
pmsg:	@msg
sp:	@ramtop
