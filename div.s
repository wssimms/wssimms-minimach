
	;; Edit the following to specify the maximum number to be
	;; checked for primeness. MAXVAL must be >= 3 and <= 32767
	MAXVAL = 100

	;; writing to this address prints a character to the terminal
	outloc = 0xFF00

;;; prime sieve
;;; 
;;; This could be better. Specifically, it does not stop dividing the
;;; number currently being checked with successive numbers from the
;;; list of primes when the current prime exceeds the square root
;;; of the number being checked, at it should. So it is therefore
;;; unnecessarily slow for 'large' values of MAXVAL.
	
	;; print message
	L	pintro
	S	pmsg
	L	pintro+1
	S	pmsg+1
	JUMP	puts
	L	enum
	S	decv
	L	enum+1
	S	decv+1
	JUMP	pdec
	L	colon
	S	outloc
	L	c10		;'\n'
	S	outloc
	;; print initial 2 in pvec
	L	pvec
	S	decv
	L	pvec+2
	S	decv+1
	JUMP	pdec
cknum:
	L	pptr
	S	cptr
	L	pptr+1
	S	cptr+1		;initialize cptr to start of pvec
cknum0:	
	L	tnum
	S	dend
	L	tnum+1
	S	dend+1
	L	cptr
	S	cknum1+1
	L	cptr+1
	S	cknum1+2
cknum1:
	L	0		;lo byte of next prime
	S	dsor
	JUMP	icptr
	L	cptr
	S	cknum2+1
	L	cptr+1
	S	cknum2+2
cknum2:
	L	0		;hi byte of next prime
	S	dsor+1
	JUMP	icptr
	JUMP	udiv		;divide tnum by prime
	L	urem
	SWAP
	L	urem+1
	OR			;check for 0 remainder
	TEST	cknum4,cknumB,cknum4
cknumB:
	;; increment tnum
	L	c2
	SWAP
	L	tnum
	ADD
	S	tnum
	L	tnum+1
	ADD			;add carry, if any
	S	tnum+1		;increment tnum
	;; check to see if finished
	L	tnum+1
	SWAP
	L	enum+1
	SUB
	SWAP			;check borrow
	TEST	cknumC,.+4,.+4  ;enum < tnum => finished
	SWAP			;check result
	TEST	cknum3,.+4,cknum3 ; enum != tnum => next tnum
	L	tnum
	SWAP
	L	enum
	SUB
	SWAP			;check borrow
	TEST	cknumC,cknum3,cknum3	;enum < tnum => finished
cknumC:	
	;; here tnum >= enum
	JUMP	done
cknum3:	
	;; try again
	JUMP	cknum		;loop check next tnum
cknum4:
	;; here remainder was not 0
	L	cptr+1
	SWAP
	L	nptr+1
	SUB			;check cptr (hi) == nptr (hi)
	TEST	cknumA,.+4,cknumA
	L	cptr
	SWAP
	L	nptr
	SUB			;check cptr (lo) == nptr (lo)
	TEST	.+4,cknum5,.+4
cknumA:	
	;; here cptr != nptr, so try the next prime
	JUMP	cknum0
cknum5:
	;; here cptr == nptr, all primes have been checked
	;; this means tnum is prime
	;; print the prime
	L	comma
	S	outloc		;print ','
	L	tnum
	S	decv
	L	tnum+1
	S	decv+1
	JUMP	pdec		;print the prime
	;; count the prime
	L	c1
	SWAP
	L	pcnt
	ADD
	S	pcnt
	L	pcnt+1
	ADD
	S	pcnt+1
	;; now put the new prime into pvec
	L	cptr
	S	cknum6+1
	L	cptr+1
	S	cknum6+2
	L	tnum
cknum6:
	S	0		;store lo byte of tnum in pvec
	JUMP	icptr
	L	cptr
	S	cknum7+1
	L	cptr+1
	S	cknum7+2
	L	tnum+1
cknum7:
	S	0		;store hi byte of tnum in pvec
	JUMP	icptr
	L	cptr
	S	nptr
	L	cptr+1
	S	nptr+1		;update nptr
	JUMP	cknumB
done:
	L	c10		;'\n'
	S	outloc
	L	ptotal
	S	pmsg
	L	ptotal+1
	S	pmsg+1
	JUMP	puts
	L	pcnt
	S	decv
	L	pcnt+1
	S	decv+1
	JUMP	pdec
	L	c10
	S	outloc
	END

	;; increment cptr
icptr:
	S	icptr1+1
	SWAP
	S	icptr1+2	;save return address
	L	c1		;1
	SWAP
	L	cptr
	ADD
	S	cptr
	L	cptr+1
	ADD			;add carry, if any
	S	cptr+1
icptr1:	JUMP	0
	

puts:
	;; output a string, a ptr to which is in variable 'pmsg'
	S	puts1+1
	SWAP
	S	puts1+2		;save the return address
	L	pmsg
	S	puts0+1
	L	pmsg+1
	S	puts0+2
puts0:
	L	0		;get character
	TEST	.+4,puts1,.+4
	S	outloc		;print the non-null character
	;; increment the pointer
	L	c1		;1
	SWAP
	L	puts0+1
	ADD
	S	puts0+1
	L	puts0+2
	ADD
	S	puts0+2
	JUMP	puts0
puts1:	JUMP	0		;return to caller

pmsg:	.=.+2


;; 	;; print a 16-bit number as 4 hexadecimal digits
;; phex:
;; 	S	phex1+1
;; 	SWAP
;; 	S	phex1+2		;store return address
;; 	L	c0
;; 	SWAP
;; 	L	decv+1
;; 	SHR
;; 	SHR
;; 	SHR
;; 	SHR
;; 	S	hdig
;; 	JUMP	phdig
;; 	L	c15
;; 	SWAP
;; 	L	decv+1
;; 	AND
;; 	S	hdig
;; 	JUMP	phdig
;; 	L	c0
;; 	SWAP
;; 	L	decv
;; 	SHR
;; 	SHR
;; 	SHR
;; 	SHR
;; 	S	hdig
;; 	JUMP	phdig
;; 	L	c15
;; 	SWAP
;; 	L	decv
;; 	AND
;; 	S	hdig
;; 	JUMP	phdig
;; phex1:
;; 	JUMP	0

;; 	;; print a value in the range 0-15 as a hexadecimal digit
;; phdig:
;; 	S	phdig2+1
;; 	SWAP
;; 	S	phdig2+2	;store return address
;; 	L	hdig
;; 	SWAP
;; 	L	c10
;; 	SUB
;; 	TEST	.+4,.+4,phdig1
;; 	L	hdig
;; 	SWAP
;; 	L	zero
;; 	ADD
;; 	SWAP
;; 	L	c7
;; 	ADD
;; 	S	outloc
;; 	JUMP	phdig2
;; phdig1:
;; 	L	hdig
;; 	SWAP
;; 	L	zero
;; 	ADD
;; 	S	outloc
;; phdig2:
;; 	JUMP	0
	
;; hdig:	.=.+2
	
	
	;; print a decimal number <= 32767 which
	;; has been stored in variable decv
pdec:
	S	pdec5+1
	SWAP
	S	pdec5+2		;save return address
	L	c0		;0
	JUMP	adig		;place sentinel value of 0
pdec1:	L	decv
	S	dend
	L	decv+1
	S	dend+1		;store the dividend
	L	c10
	S	dsor
	L	c0
	S	dsor+1		;store the divisor (10)
	JUMP	udiv		;divide
	L	urem
	SWAP
	L	zero		;'0'
	ADD
	S	ddig
	JUMP	adig		;push the digit
	L	uquo
	SWAP
	L	uquo+1
	OR			;quotient == zero ?
	TEST	pdec2,pdec3,pdec2
	;; Here the quotient is not zero, make more digits
pdec2:	L	uquo
	S	decv
	L	uquo+1
	S	decv+1
	JUMP	pdec1
	;; Here the quotient is zero, print the digits
pdec3:	JUMP	rdig		;pop the digit
	L	ddig
	TEST	pdec4,pdec5,pdec4
pdec4:	S	outloc		;print the digit
	JUMP	pdec3		;next digit
	;; here we popped the sentinel (0)
pdec5:	JUMP	0		;return to caller

decv:	.=.+2			;unsigned 16-bit value to print

	;; pop a digit off the digit stack
rdig:
	S	rdig2+1
	SWAP
	S	rdig2+2		;save return address
	L	c1		;1
	SWAP
	L	dptr
	SUB			;decrement dptr
	S	dptr
	L	dptr+1
	ADD
	S	dptr+1
	L	dptr
	S	rdig1+1
	L	dptr+1
	S	rdig1+2
rdig1:	L	0		;get byte at (dptr)
	S	ddig
rdig2:	JUMP	0		;return to caller

	;; push a digit onto the digit stack
adig:
	S	adig2+1
	SWAP
	S	adig2+2		;save return address
	L	dptr
	S	adig1+1
	L	dptr+1
	S	adig1+2
	L	ddig
adig1:	S	0		;store byte to (dptr)
	L	c1		;1
	SWAP
	L	dptr
	ADD			;increment dptr
	S	dptr
	L	dptr+1
	ADD
	S	dptr+1
adig2:	JUMP	0		;return to caller
	
dptr:	<dbuf,>dbuf		;ptr to top of digit stack
dbuf:	.=.+6			;digit stack
ddig:	.=.+1			;digit to push, or popped digit

	
udiv:
	S	udiv10+1
	SWAP
	S	udiv10+2	;save return address
	L	dsor+1
	TEST	udiv4,udiv0,udiv4
udiv0:	L	dsor
	TEST	udiv2,udiv1,udiv2
udiv1:	L	c1
	S	divz		;indicate divide by zero
	JUMP	udiv10		;return to caller
udiv2:	SWAP			;lo byte of divisor in C
	L	c1
	SUB			;C:A = 1 - divisor lo
	OR
	TEST	udiv4,udiv3,udiv4
udiv3:	;; here the divisor was 1
	L	dend
	S	uquo
	L	dend+1
	S	uquo+1		;quotient = dividend
	L	c0
	S	urem
	S	urem+1		;remainder = 0
	S	divz		;divisor != 0
	JUMP	udiv10
udiv4:	;; here we actually do the division
	L	c0
	S	divz		;divisor != 0
	SWAP
	L	c1
	S	pquo
	SWAP
	S	pquo+1		;initialize pquo to 0x0001
	S	uquo
	S	uquo+1		;initialize uquo to 0x0000
udiv5:	
	;; double the divisor until it exceeds the dividend. this is ok
	;; because the divisor and dividend are no more than 15 bits long
	L	dend+1
	SWAP
	L	dsor+1
	SUB			;divisor hi - dividend hi
	SWAP			;check borrow
	TEST	udiv6,udivA,udivA ;if borrow, divisor is less
udivA:	SWAP			  ;otherwise check result
	TEST	udiv7,udivB,udiv7 ;if result !=0, divisor is >= dividend
	;; check the lo byte difference
udivB:	L	dend
	SWAP
	L	dsor
	SUB			;divisor lo - dividend lo
	SWAP			;check borrow
	TEST	udiv6,udiv7,udiv7 ;if borrow, divisor <, else divisor >=
	;; here divisor < dividend
udiv6:	L	dsor+1
	SWAP
	L	dsor
	SHL
	S	dsor
	SWAP
	S	dsor+1		;divisor <<= 1
	L	pquo+1
	SWAP
	L	pquo
	SHL
	S	pquo
	SWAP
	S	pquo+1		;partial quotient <<= 1
	JUMP	udiv5
udiv7:
	;; halve the divisor until it is less than the dividend
	;; this yields the largest multiple of the divisor by a power of 2
	;; that can be subtracted from the dividend with a positive difference
	L	dsor+1
	SWAP
	L	dend+1
	SUB			;dividend hi - divisor hi
	SWAP			;check borrow
	TEST	udiv8,udivC,udivC ;if borrow, divisor is greater
udivC:	SWAP			  ;otherwise check result
	TEST	udiv9,udivD,udiv9 ;if result !=0, dividend is >= divisor
	;; check the lo byte difference
udivD:	L	dsor
	SWAP
	L	dend
	SUB			;divisor lo - dividend lo
	SWAP			;check borrow
	TEST	udiv8,udiv9,udiv9 ;if borrow, dividend <, else dividend >=
	;; here dividend < divisor
udiv8:	L	dsor+1
	SWAP
	L	dsor
	SHR
	S	dsor
	SWAP
	S	dsor+1		;divisor >>= 1
	L	pquo+1
	SWAP
	L	pquo
	SHR
	S	pquo
	SWAP
	S	pquo+1		;partial quotient >>= 1
	OR			;test for partial quotient == 0
	TEST	udiv7,udiv10,udiv7
udiv9:
	;; subtract the multiple of the divisor from the dividend and add
	;; the corresponding power of 2 (partial quotient) to the quotient
	L	dsor
	SWAP
	L	dend
	SUB			;dividend lo - divisor lo
	S	dend
	L	dend+1
	ADD			;borrow from dividend hi, if necessary
	SWAP
	L	dsor+1
	SWAP
	SUB			;dividend hi - dividend lo
	S	dend+1
	L	uquo
	SWAP
	L	pquo
	ADD			;add lo bytes of pquo and uquo
	S	uquo		;save lo byte of result
	L	uquo+1
	ADD			;add carry, if any
	SWAP
	L	pquo+1
	ADD			;add hi bytes of pquo and uquo
	S	uquo+1
	JUMP	udiv7
udiv10:
	JUMP	0		;return address will be filled in

pquo:	.=.+2
uquo:	.=.+2
urem:
dend:	.=.+2
dsor:	.=.+2
diff:	.=.+2
divz:	0
c0:	0
c1:	1
c2:	2
c7:	7
c10:	10
c15:	15
zero:	'0'
comma:	','
colon:	':'
space:	' '

intro:	"Here are the primes <= ",0
total:	"Number of primes is ",0
pintro:	<intro,>intro
ptotal:	<total,>total

pcnt:	1,0
tnum:	<3,>3			;start checking with 3
enum:	<MAXVAL,>MAXVAL		;end checking with MAXVAL
nptr:	<(pvec+2),>(pvec+2)	;points to where the next prime will go.
cptr:	<pvec,>pvec		;points to the current prime being checked
pptr:	<pvec,>pvec		;points to the start of pvec
pvec:	2,0			;primes will go here
