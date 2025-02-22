
	;; Edit the following to specify the maximum number to be
	;; checked for primeness. MAXVAL must be >= 3 and <= 32767
	MAXVAL = 500

	;; writing to this address prints a character to the terminal
	outloc = 0xFF00

;;; prime sieve

	;; print message
	JUMP	puts
	@intro
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
	L	pvec+1
	S	decv+1
	JUMP	pdec
cknum:
	;; this is the head of the outermost loop, in which we check
	;; to see if the current 'tnum' is prime. tnum is initialized
	;; to 3, and is incremented by 2 every time through this loop.
	L	pptr
	S	cptr
	L	pptr+1
	S	cptr+1		;initialize cptr to start of pvec
	L	tnum
	S	sqrtv
	L	tnum+1
	S	sqrtv+1
	JUMP	sqrt		;find sqrt of tnum
cknum0:
	;; this is the head of the next loop that checks to see if each
	;; of the currently stored primes in turn divides tnum with a
	;; zero remainder.
	L	tnum
	S	dend
	L	tnum+1
	S	dend+1
	;; get the 16-bit word pointed to by variable 'cptr'
	;; into the the variable 'dsor' (the divisor)
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
	;; before we divide, check to see if the prime number is
	;; greater than the integer square root of 'tnum'. If it is,
	;; then it can't possibly divide 'tnum' and thus 'tnum' is prime
	;; the sqrt of 'tnum' is necessarily only one byte, so if the
	;; hi byte of the prime != 0, then no need to check further
	L	dsor+1
	TEST	cknum5,.+4,cknum5 ;cknum5 is the label for "it's prime"
	L	c0
	SWAP			;clear carry
	L	dsor
	SUBC	sqrtm
	TEST	.+4,.+4,cknum5
	JUMP	udiv		;divide tnum by prime
	L	urem
	OR	urem+1		;check for 0 remainder
	TEST	cknum0,cknumB,cknum0
cknumB:
	;; this is the bottom of the outermost loop. Here we increment tnum,
	;; and if the incremented tnum > enum, then exit the loop and the
	;; program. Otherwise go back to the top of the outermost loop to
	;; check the newly incremented value of tnum for primeness.
	L	c0
	SWAP			;clear carry
	L	c2
	ADDC	tnum
	S	tnum
	L	c0
	ADDC	tnum+1		;add carry, if any
	S	tnum+1		;increment tnum
	;; check to see if finished
	L	c0
	SWAP			;clear carry
	L	enum+1
	SUBC	tnum+1
	SWAP			;check borrow
	TEST	cknum3,.+4,.+4  ;enum < tnum => finished
	SWAP			;check result
	TEST	cknum4,.+4,cknum4 ; enum != tnum => next tnum
	L	c0
	SWAP			;clear carry
	L	enum
	SUBC	tnum
	SWAP			;check borrow
	TEST	cknum3,cknum4,cknum4	;enum < tnum => finished
cknum3:	
	;; here tnum >= enum
	JUMP	done
cknum4:	
	;; try again
	JUMP	cknum		;loop check next tnum
cknum5:
	;; here all primes <= sqrt(tnum) have been checked and
	;; don't divide tnum, so tnum is prime
	L	nptr
	S	cptr
	L	nptr+1
	S	cptr+1		;set cptr at end of prime list
	;; print the prime
	L	comma
	S	outloc		;print ','
	L	tnum
	S	decv
	L	tnum+1
	S	decv+1
	JUMP	pdec		;print the prime
	;; count the prime
	L	c0
	SWAP			;clear carry
	L	c1
	ADDC	pcnt
	S	pcnt
	L	c0
	ADDC	pcnt+1
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
	JUMP	puts
	@total
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
	;;
	L	c0
	SWAP			;clear carry
	L	c1
	ADDC	cptr
	S	cptr
	L	c0
	ADDC	cptr+1		;add carry, if any
	S	cptr+1
icptr1:
	JUMP	0
	
;;; output a null-terminated string
;;; a ptr to the message is inserted after the call to puts
;;; and puts picks it up and prints the message
	
puts:
	;; output a string
	S	stkptr
	SWAP
	S	stkptr+1	;save link in stack pointer
	;; get parameter by popping the stack
	JUMP	popb
	L	stkbyte
	S	puts0+1
	JUMP	popb
	L	stkbyte
	S	puts0+2
	;; save return address
	L	stkptr
	S	puts1+1
	L	stkptr+1
	S	puts1+2
puts0:
	L	0		;get character
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
puts1:
	JUMP	0		;return to caller

	;; print a 16-bit number as 4 hexadecimal digits
phex:
	S	phex1+1
	SWAP
	S	phex1+2		;store return address
	;; 
	L	c0
	SWAP
	L	decv+1
	SHR
	SHR
	SHR
	SHR
	S	hdig
	JUMP	phdig
	L	c15
	AND	decv+1
	S	hdig
	JUMP	phdig
	L	c0
	SWAP
	L	decv
	SHR
	SHR
	SHR
	SHR
	S	hdig
	JUMP	phdig
	L	c15
	AND	decv
	S	hdig
	JUMP	phdig
phex1:
	JUMP	0

	;; print a value in the range 0-15 as a hexadecimal digit
phdig:
	S	phdig2+1
	SWAP
	S	phdig2+2	;store return address
	;;
	L	c0
	SWAP			;clear carry
	L	c10
	SUBC	hdig
	TEST	.+4,.+4,phdig1
	L	c0
	SWAP			;clear carry
	L	hdig
	ADDC	czero		;+'0'
	ADDC	c7
	S	outloc
	JUMP	phdig2
phdig1:
	L	c0
	SWAP			;clear carry
	L	hdig
	ADDC	czero
	S	outloc
phdig2:
	JUMP	0
	
hdig:	.=.+2
	
	
	;; print a decimal number <= 32767 which
	;; has been stored in variable decv
pdec:
	S	pdec5+1
	SWAP
	S	pdec5+2		;save return address
	;;
	L	dptr
	S	stkptr
	L	dptr+1
	S	stkptr+1
	L	c0		;0
	S	stkbyte
	JUMP	pushb		;place sentinel value of 0
pdec1:	L	decv
	S	dend
	L	decv+1
	S	dend+1		;store the dividend
	L	c10
	S	dsor
	L	c0
	S	dsor+1		;store the divisor (10)
	JUMP	udiv		;divide
	L	c0
	SWAP			;clear carry
	L	urem
	ADDC	czero		;+ '0'
	S	stkbyte
	JUMP	pushb		;push the digit
	L	uquo
	OR	uquo+1		;quotient == zero ?
	TEST	pdec2,pdec3,pdec2
	;; Here the quotient is not zero, make more digits
pdec2:	L	uquo
	S	decv
	L	uquo+1
	S	decv+1
	JUMP	pdec1
	;; Here the quotient is zero, print the digits
pdec3:	JUMP	popb		;pop the digit
	L	stkbyte
	TEST	pdec4,pdec5,pdec4
pdec4:	S	outloc		;print the digit
	JUMP	pdec3		;next digit
	;; here we popped the sentinel (0)
pdec5:	JUMP	0		;return to caller

decv:	.=.+2			;unsigned 16-bit value to print	
dbuf:	.=.+6			;digit stack
dptr:	@dptr			;ptr to top of digit stack

;;; general purpose stack implementation
;;; The 16-bit variable 'stkptr' can be loaded with an address.
;;; The 8-bit variable 'stkbyte' is used to supply bytes to push
;;; when calling 'pushb' and to receive popped bytes when calling
;;; 'popb'
	
	;; pop a byte from a stack
popb:
	S	popb0+1
	SWAP
	S	popb0+2		;save return address
	;; retrieve the byte
	L	stkptr
	S	popptr+1
	L	stkptr+1
	S	popptr+2
popptr:
	L	0
	S	stkbyte
	;; increment the stack pointer
	L	c0
	SWAP			;clear carry
	L	c1
	ADDC	stkptr		;increment lo byte
	S	stkptr
	L	c0
	ADDC	stkptr+1	;add carry, if necessary
	S	stkptr+1
popb0:
	JUMP	0		;return to caller
	
	;; push a byte onto a stack
pushb:
	S	pushb0+1
	SWAP
	S	pushb0+2	;save return address
	;; decrement stack pointer
	L	c0
	SWAP			;clear carry
	L	stkptr
	SUBC	c1		;decrement lo byte
	S	stkptr
	S	pushptr+1
	L	stkptr+1
	SUBC	c0		;borrow, if necessary
	S	stkptr+1
	S	pushptr+2
	;; store the byte
	L	stkbyte
pushptr:
	S	0
pushb0:
	JUMP	0		;return to caller

	;; variables for pushb and popb
stkptr:	.=.+2
stkbyte:.=.+1

;;; unsigned 16-bit divide
;;; producing an unsigned 16-bit quotient and an unsigned
;;; 16-bit remainder
	
udiv:
	S	udiv10+1
	SWAP
	S	udiv10+2	;save return address
	;; 
	L	dsor+1
	TEST	udiv4,.+4,udiv4
	L	dsor
	TEST	udiv2,.+4,udiv2
	L	c1
	S	divz		;indicate divide by zero
	JUMP	udiv10		;return to caller
udiv2:
	SWAP
	L	c0
	SWAP			;clear carry
	SUBC	c1		;compare divisor to 1
	TEST	udiv4,.+4,udiv4
	;; here the divisor was 1
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
	L	c1
	S	pquo
	L	c0
	S	divz		;divisor != 0
	S	pquo+1		;initialize pquo to 0x0001
	S	uquo
	S	uquo+1		;initialize uquo to 0x0000
udiv5:	
	;; double the divisor until it is >= the dividend. this is ok
	;; because the divisor and dividend are no more than 15 bits long
	L	c0
	SWAP			;clear carry
	L	dsor+1
	SUBC	dend+1		;divisor hi - dividend hi
	SWAP			;check borrow
	TEST	udiv6,.+4,.+4	;if borrow, divisor is less
	SWAP			;otherwise check result
	TEST	udiv7,.+4,udiv7	;if result !=0, divisor is >= dividend
	;; check the lo byte difference
	L	c0
	SWAP			;clear carry
	L	dsor
	SUBC	dend		;divisor lo - dividend lo
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
	L	c0
	SWAP			;clear carry
	L	dend+1
	SUBC	dsor+1		;dividend hi - divisor hi
	SWAP			;check borrow
	TEST	udiv8,.+4,.+4	;if borrow, divisor is greater
	SWAP			;otherwise check result
	TEST	udiv9,.+4,udiv9 ;if result !=0, dividend is >= divisor
	;; check the lo byte difference
	L	c0
	SWAP			;clear carry
	L	dend
	SUBC	dsor		;dividend lo - divisor lo
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
	OR	pquo		;test for partial quotient == 0
	TEST	udiv7,udiv10,udiv7
udiv9:
	;; subtract the multiple of the divisor from the dividend and add
	;; the corresponding power of 2 (partial quotient) to the quotient
	L	c0
	SWAP			;clear carry
	L	dend
	SUBC	dsor		;dividend lo - divisor lo
	S	dend
	L	dend+1
	SUBC	dsor+1		;dividend hi - divisor lo
	S	dend+1
	L	c0
	SWAP			;clear carry
	L	pquo
	ADDC	uquo		;add lo bytes of pquo and uquo
	S	uquo		;save lo byte of result
	L	uquo+1		;add carry, if any
	ADDC	pquo+1		;add hi bytes of pquo and uquo
	S	uquo+1
	JUMP	udiv7
udiv10:
	JUMP	0		;return address will be filled in

	;; finds the integer square root of the 16-bit positive
	;; value in variable 'sqrtv'. The result is stored in the
	;; variable 'sqrtm'.
sqrt:
	S	sqrt3+1
	SWAP
	S	sqrt3+2		;store return address
	;; 
	L	c1
	S	sqrtd
	L	c0
	S	sqrtd+1		;start with divisor 0x0001
sqrt0:	
	L	sqrtv
	S	dend
	L	sqrtv+1
	S	dend+1
	L	sqrtd
	S	dsor
	L	sqrtd+1
	S	dsor+1
	JUMP	udiv		;divide num by divisor
	;; now find the mean of the divisor and the quotient
	L	c0
	SWAP			;clear carry
	L	uquo
	ADDC	sqrtd		;add lo bytes
	S	sqrtm		;remember lo byte sum for a moment
	L	uquo+1
	ADDC	sqrtd+1		;add carry and hi byte of divisor
	SWAP			;hi byte sum in C
	L	sqrtm		;lo byte sum in A
	SHR			;divide by 2
	S	sqrtm
	SWAP
	S	sqrtm+1		;store mean
	;; if mean == divisor then done
	L	c0
	SWAP			;clear carry
	L	sqrtm+1
	SUBC	sqrtd+1		;compare hi bytes of mean and divisor
	TEST	sqrt1,.+4,sqrt1
	L	c0
	SWAP			;clear carry
	L	sqrtm
	SUBC	sqrtd		;compare lo bytes of mean and divisor
	TEST	sqrt1,sqrt3,sqrt1
sqrt1:
	;; if mean == quotient then done
	L	c0
	SWAP			;clear carry
	L	uquo
	SUBC	sqrtm		;compare lo bytes of mean and quotient
	TEST	sqrt2,.+4,sqrt2
	L	c0
	SWAP			;clear carry
	L	uquo+1
	SUBC	sqrtm+1		;compare hi bytes of mean and quotient
	TEST	sqrt2,sqrt3,sqrt2
sqrt2:
	;; here we are not done, the mean is the new divisor
	L	sqrtm
	S	sqrtd
	L	sqrtm+1
	S	sqrtd+1
	JUMP	sqrt0
sqrt3:
	JUMP	0		;return to caller
	

sqrtv:	.=.+2
sqrtd:	.=.+2
sqrtm:	.=.+2
	
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
czero:	'0'
comma:	','
colon:	':'
space:	' '
lpar:	'('
rpar:	')'
lbrack:	'['
rbrack:	']'

	
intro:	"Here are the primes <= ",0
total:	"Number of primes is ",0

pcnt:	@1
tnum:	@3			;start checking with 3
enum:	@MAXVAL			;end checking with MAXVAL
nptr:	@(pvec+2)		;points to where the next prime will go.
cptr:	@pvec			;points to the current prime being checked
pptr:	@pvec			;points to the start of pvec
pvec:	@2			;primes will go here
