# wssimms-minimach
A minimal 8-bit virtual machine

This is an exercise in developing a minimal (virtual) 8-bit CPU with as few instructions as possible without making things too unwieldy or inconvenient. The current version has 11 instructions used for computation, plus a stop instruction which stops the simulated CPU.

## Machine Description

There are two 8-bit registers: A and C. All ALU operations use these two registers as operands and their results are placed in these two registers. The load and store instructions only load into register A and store from register A. The only way to get a value in register C is to first load the value into A and then use the SWAP instruction to move it into C.

There is also a program counter, which contains the address of the next instruction to be executed. It can be altered by means of the JUMP and TEST instructions.

The available instructions are as follows:
```
Opcode	      Mnemonic

0	      END		Stops execution of the CPU

1 lo hi	      L    address	Loads a byte from memory into A

2 lo hi	      S	   address	Stores the contents of A into memory

3    	      SWAP 		Exchanges the contents of A and C

4 lo hi	      AND  address	The contents of A are replaced with the
	      			bitwise AND of A and the byte loaded from
				the specified memory address. The contents
				of C are replaced with the complement of
				the bitwise AND of A and C
				
5 lo hi	      OR   address	The contents of A are replaced with the
	      			bitwise OR of A and the byte loaded from
				the specified memory address. The contents
				of C are replaced with the complement of
				the bitwise OR of A and C
				
6	      SHL		The contents of C and A are shifted left one
	      			bit position. The high bit of A is shifted
				into the low bit of C. The low bit of A is
				replaced with a zero.
				
7	      SHR		The contents of C and A are shifted right one
	      			bit position. The low bit of C is shifted
				into the high bit of A. The high bit of C is
				replaced with a zero.
				
8 lo hi	      ADDC  address	The contents of C is treated as a signed quantity
     	      	    		and extended to 16 bits. A and the memory byte
				loaded from the specified address are treated as
	      			unsigned quantities and extended to 16 bits.
				Then all three of these operands are added. The
				high byte of the 16-bit result is placed in C.
				The low byte of the 16-bit result is placed in A.
				
9 lo hi	      SUBC  address	The contents of C is treated as a signed quantity
     	      	    		and extended to 16 bits. A and and the memory byte
				loaded from the specified address are treated as
	      			unsigned quantities and extended to 16 bits.
				Then C is added to A and the extended memory byte
				is subtracted from their sum. The high byte of the
				16-bite result is placed in C. The low byte of the
				16-bit result is placed in A.

10 lo hi      JUMP address	The high byte of the PC is placed into C. The
      	      	   		low byte of the PC is placed in A. Then the
				contents of the PC are replaced by the operand
				address.

11 o1 o2 o3   TEST t1,t2,t3	If A, treated as a signed quantity, is less
      	      	   		than zero, then the first byte after the
				the opcode (o1), treated as a signed quantity,
				is extended to 16 bits and added to the address
				of the byte 'o1'. Thus 'o1' is a PC-relative
				offset. If A is equal to zero, then byte 'o2'
				is similarly used as an offset to adjust PC.
				If A is greater than zero, then byte 'o3' is
				used as an offset to adject PC.
```
The instructions and their operation is briefly recapitulated below using
C language notation:
```
  ext(unsigned_byte) is defined: ((uint16_t)unsigned_byte)
  sext(unsigned_byte) is defined: ((uint16_t)(int16_t)(int8_t)unsigned_byte)
 
  0    END                stop processor
  1    L     address      A = read(address)
  2    S     address      write(address, A)
  3    SWAP               swaps A and C
  4    AND   address      b = read(address); A = A & b; C = ~(A & b)
  5    OR    address      b = read(address); A = A | b; C = ~(A | b)
  6    SHL                C:A = C:A << 1
  7    SHR                C:A = C:A >> 1
  8    ADDC  address      b = read(address); C:A = sext(c) + ext(A) + ext(b)
  9    SUBC  address      b = read(address); C:A = sext(c) + ext(A) - ext(b)
 11    JUMP  address      C:A = PC + 3; PC = address
 12    TEST  X,Y,Z        PC = PC + 1 + (A<0 ? sext(X)
                                             : (A=0 ? sext(Y)
                                                    : sext(Z)))
```

## Memory Map

Addresses 0x0000 - 0xEFFF are read/write memory
Addresses 0xF000 - 0xFEFF are read only memory
Addresses 0xFF00 - 0xFFFF are memory mapped I/O

Both read/write memory and read only memory can be loaded at the beginning of execution by using appropriate arguments on the command line when invoking the CPU simulation.

The following memory mapped I/O addresses are available:

```
0xFF00	      R: reads a character from standard input into register A
	      W: writes the contents of A to standard output.
```

## Compiling the virtual machine

The virtual machine source file (minimach.c) is standard C (but perhaps dependent on little-endian byte order in the host machine) and has no dependencies apart from the standard library. It can be compiled as follows:

cc -o mm minimach.c

## Running the virtual machine

The result of compiling minimach.c can be run directly without command line arguments. In this case, a short, built-in "hello world" program is executed. A binary file for execution can be supplied on the command line like so:

./mm myprog.o

In this case, the binary file "myprog.o" will be loaded into the virtual machine's read/write memory beginning at location 0, and then executed, again starting at location 0.

A binary file can be loaded into the virtual machine's read only memory by using the -r command like switch like so:

./mm -r myrom.o

This causes the binary file "myrom.o" to be loaded into the virtual machine's read only memory beginning at location 0xF000.

Of course, one may supply both two binary files, one for read/write memory and one for read only memory like so:

./mm -r myrom.o myprog.o

## How to obtain binary files to execute

The repository also includes a simple assembler, the source code for which is found in the file "mmas.c". This program is also written in standard C, also however dependent on a little endian host machine, and can be compiled like so:

cc -o mmas mmas.c

Once compiled, this assembler will assemble assembly language source files for the minimal CPU virtual machine using the instruction mnemonics given above.

## More about "minimach" assembly language

### Labels

Labels may be up to 8 characters long, and must be followed by a colon (:).

### Data

Data may be included in the program simply by typing it into the assembly language source file like this:
```
1,2,3,4
```
Each of the comma-separated items produces one byte in the output file. Each is actually an arithmetic expression, so the following is equivalent:
```
4-3,16/8,1|2,(6/2)^(3+4)
```
Expressions of this type may also be used as operands to the load, store, jump, and test instructions.

The high or low bytes of a 16-bit value may be included in the program using the < and > unary operators, like so:
```
<0x1C9A        ;yields 9A
>0x1C9A        ;yields 1C
```
A 16-bit value may be included in the program using the @ unary operator. The constituent bytes will be placed in the output file, in little endian order, like so:
```
@0xDA30        ;yields the byte sequence 30 DA
```
Character data may be included in the program inside double quotes like so:
```
"Hello",0
```
Unlike in C, a null byte is not automatically included, so the example above explicitly provides a terminating null byte.

Comments are preceded by a semicolon (;) character as is usual in assembly language programs.

### Symbols

Symbolic constants may be defined using the '=' operator like so:
```
rom = 0xF000
```
These are generally useful only as addresses as the virtual CPU has no immediate addressing modes.

The 'curent location' during assembly is represented by the character '.' and this character can be used to reserve zero-filled space in a program by using an expresion like:
```
.=.+16
```
to reserve 16 bytes. The current location may only be advanced, so an expression like this:
```
.=.-1
```
is not allowed. The character '.' can also be used to build targets for the jump and test instructions like so:
```
          TEST   .+4,itszero,.+4
	  ...
itszero:	 
```
This will cause a jump to the label itszero if the contents of A are zero. Otherwise execution will continue with the instruction after the test instruction, as that is the value at .+4, where . is the address of the TEST instruction.

Examination of the sample programs 'hw.s' and 'primes.s' will help clarify how assembly language programs may be written.
