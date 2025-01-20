# wssimms-minimach
A minimal 8-bit virtual machine

This is an exercise in developing a minimal (virtual) 8-bit CPU with as few instructions as possible without making things too unwieldy or inconvenient. The current version has 12 instructions used for computation, plus a stop instruction which stops the simulated CPU.

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

4	      AND		The contents of A are replaced with the
	      			bitwise AND of A and C. The contents of C
				are replaced with the complement of the
				bitwise AND of A and C
				
5	      OR		The contents of A are replaced with the
	      			bitwise OR of A and C. The contents of C
				are replaced with the complement of the
				bitwise OR of A and C
				
6	      EOR		The contents of A are replaced with the
	      			bitwise EXCLUSIVE OR of A and C. The contents
				of C are replaced with the complement of the
				bitwise EXCLUSIVE OF of A and C
				
7	      SHL		The contents of C and A are shifted left one
	      			bit position. The high bit of A is shifted
				into the low bit of C. The low bit of A is
				replaced with a zero.
				
8	      SHR		The contents of C and A are shifted right one
	      			bit position. The low bit of C is shifted
				into the high bit of A. The high bit of C is
				replaced with a zero.
				
9	      ADD		The contents of A and C are treated as
	      			unsigned quantities, extended to 16 bits,
				and added. The high byte of the 16-bit result
				is placed in C. The low byte of the 16-bit
				result is placed in A.
				
10	      SUB		The contents of A and C are treated as
	      			unsigned quantities, extended to 16 bits,
				and C is subtracted from A. The high byte of
				the 16-bit result is placed in C. The low byte
				of the 16-bit result is placed in A.

11 lo hi      JUMP address	The high byte of the PC is placed into C. The
      	      	   		low byte of the PC is placed in A. Then the
				contents of the PC are replaced by the operand
				address.

12 o1 o2 o3   TEST t1,t2,t3	If A, treated as a signed quantity, is less
      	      	   		than zero, then the first byte after the
				the opcode (o1), treated as a signed quantity,
				is extended to 16 bits and added to the address
				of the byte 'o1'. Thus 'o1' is a PC-relative
				offset. If A is equal to zero, then byte 'o2'
				is similarly used as an offset to adjust PC.
				If A is greater than zero, then byte 'o3' is
				used as an offset to adject PC.

The instructions and their operation is briefly recapitulated below using
C language notation:

  0    END                stop processor
  1    L     address      A = read(address)
  2    S     address      write(address, A)
  3    SWAP               swaps A and C
  4    AND                A = A & C; C = ~(A & C)
  5    OR                 A = A | C; C = ~(A | C)
  6    EOR                A = A ^ C; C = ~(A ^ C)
  7    SHL                C:A = C:A << 1
  8    SHR                C:A = C:A >> 1
  9    ADD                C:A = ext(A) + ext(C)
 10    SUB                C:A = ext(A) - ext(C)
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

