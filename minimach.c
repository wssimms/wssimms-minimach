#include <string.h>
#include <stdio.h>
#include <stdint.h>

/*
 * Machine Description (v1.1)
 *
 * PC: 16 bit program counter
 * A: 8 bit accumulator
 * C: 8 bit register that is used as an operand in ALU operations
 *    and receives high byte results of <<, >>, +, - operations
 *
 * ext(unsigned_byte) is defined: ((uint16_t)unsigned_byte)
 * sext(unsigned_byte) is defined: ((uint16_t)(int16_t)(int8_t)unsigned_byte)
 *
 *  0 END                stop processor
 *  1 L     address      A = read(address)
 *  2 S     address      write(address, A)
 *  3 SWAP               swaps A and C
 *  4 AND                A = A & C
 *  5 OR                 A = A | C
 *  6 EOR                A = A ^ C
 *  7 SHL                C:A = C:A << 1
 *  8 SHR                C:A = C:A >> 1
 *  9 ADD                C:A = ext(A) + ext(C)
 * 10 SUB                C:A = ext(A) - ext(C)
 * 11 JUMP  address      C:A = PC+3; PC = address
 * 12 TEST  X,Y,Z        PC = PC + 1 + (A<0 ? sext(X)
 *                                          : (A=0 ? sext(Y)
 *                                                 : sext(Z)))
 *
 * Memory Map
 *   0x0000 - 0xEFFF : read/write memory
 *   0xF000 - 0xFEFF : read only memory
 *   0xFF00 - 0xFFFF : memory mapped i/o
 *
 * I/O area
 *   0xFF00 : R - A = getchar(), W - putchar(A)
 */

int options (int argc, char **argv);

#define EXT(ub) ((uint16_t)(ub))
#define SEXT(ub) ((uint16_t)(int16_t)(int8_t)(ub))

union {
    uint8_t b[2];
    uint16_t w;
} ac, pc, tm;

uint8_t mem[0xFF00];
uint8_t rom[0xF00];

uint8_t program[] = {
    1, 38, 0,
    2, 7, 0,
    1, 0, 0,
    12, 4, 3, 4,
    0,
    2, 0, 255,
    1, 0, 0,
    3,
    1, 7, 0,
    9,
    2, 7, 0,
    1, 8, 0,
    9,
    2, 8, 0,
    11, 6, 0,
    39, 72, 101, 108, 108, 111, 32, 87, 111, 114, 108, 100, 46, 10, 0
};

uint8_t rd (uint16_t a)
{
    if (a < 0xF000) {
	//printf("L %4.4x : %2.2x\n", a, mem[a]);
	return mem[a];
    }
    if (a < 0xFF00) return rom[a - 0xFF00];
    
    switch (a & 255) {
    case 0: return getchar();
    default: return 0;
    }
}

void wr (uint16_t a, uint8_t b)
{
    if (a < 0xF000) {
	mem[a] = b;
	//printf("S %4.4x <- %2.2x\n", a, b);
	return;
    }
    
    if (a < 0xFF00) return;

    switch (a & 255) {
    case 0: putchar(b); return;
    default: return;
    }
}

void reset (void)
{
    ac.w = 0;
    pc.b[0] = mem[0xFEFE];
    pc.b[1] = mem[0xFEFF];
}

//#define DBG 500

void execute (void)
{
    uint8_t opcode;
#ifdef DBG
    unsigned count = 0;
#endif    
    while (1) {

	opcode = mem[pc.w++];
#ifdef DBG	
	printf("%4d ", pc.w-1);
#endif	
        switch (opcode) {
        case 0:                                      /* E */
	    printf("End at pc = %d\n", pc.w-1);
            return;
        case 1:                                      /* L */
            tm.b[0] = mem[pc.w++];
            tm.b[1] = mem[pc.w++];
	    ac.b[0] = rd(tm.w);
#ifdef DBG
	    printf("L    %5d (%2.2x/%3d) ", tm.w, ac.b[0], ac.b[0]);
#endif
	    break;
        case 2:                                      /* S */
            tm.b[0] = mem[pc.w++];
            tm.b[1] = mem[pc.w++];
	    wr(tm.w, ac.b[0]);
#ifdef DBG
	    printf("S    %5d (%2.2x/%3d) ", tm.w, ac.b[0], ac.b[0]);
#endif
            break;
        case 3:                                      /* SW */
            tm.b[0] = ac.b[0];
            ac.b[0] = ac.b[1];
            ac.b[1] = tm.b[0];
#ifdef DBG
	    printf("SWAP                ");
#endif
            break;
        case 4:                                      /* AND */
            ac.b[0] = ac.b[0] & ac.b[1];
#ifdef DBG
	    printf("AND                 ");
#endif
            break;
        case 5:                                      /* OR */
            ac.b[0] = ac.b[0] | ac.b[1];
#ifdef DBG
	    printf("OR                  ");
#endif
            break;
        case 6:                                      /* EOR */
            ac.b[0] = ac.b[0] ^ ac.b[1];
#ifdef DBG
	    printf("EOR                 ");
#endif
            break;
        case 7:                                      /* SHL */
            ac.w <<= 1;
#ifdef DBG
	    printf("SHL                 ");
#endif
            break;
        case 8:                                      /* SHR */
	    ac.w >>= 1;
#ifdef DBG
	    printf("SHR                 ");
#endif
            break;
        case 9:                                      /* ADD */
	    ac.w = EXT(ac.b[0]) + EXT(ac.b[1]);
#ifdef DBG
	    printf("ADD                 ");
#endif
            break;
        case 10:                                     /* SUB */
            ac.w = EXT(ac.b[0]) - EXT(ac.b[1]);
#ifdef DBG
	    printf("SUB                 ");
#endif
            break;
        case 11:                                     /* J */
            tm.b[0] = mem[pc.w++];
            tm.b[1] = mem[pc.w++];
	    ac.w = pc.w;
            pc.w = tm.w;
#ifdef DBG
	    printf("JUMP %5d          ", tm.w);
#endif
            break;
        case 12:                                     /* TST */
	    if (((int16_t)SEXT(ac.b[0])) < 0)
                tm.w = SEXT(mem[pc.w+0]);
            else if (((int16_t)SEXT(ac.b[0])) > 0)
                tm.w = SEXT(mem[pc.w+2]);
            else
                tm.w = SEXT(mem[pc.w+1]);
            pc.w += tm.w;
#ifdef DBG
	    printf("TEST  %4.4x          ", SEXT(ac.b[0]));
#endif
            break;
        default:
	    reset();
            break;
        }
#ifdef DBG
	printf(" CA:%4.4x/%5d  C:%2.2x/%3d  A:%2.2x/%3d\n",
	       ac.w, ac.w, ac.b[1], ac.b[1], ac.b[0], ac.b[0]);
	if (++count > DBG) break;
#endif
    }
}

int main (int argc, char **argv)
{
    int n = sizeof program / sizeof program[0];
    for (int i = 0; i < n; ++i)
        mem[i] = program[i];

    if (options(argc, argv) == 0)
	printf("No valid program file specified. Running default program.\n\n");
    
    reset();
    execute();
}

long flen (FILE *f, long max, char *type, char *name)
{
    if (0 == fseek(f, 0, SEEK_END)) {
	long len = ftell(f);
	if (len > 0 && len < max) {
	    if (0 == fseek(f, 0, SEEK_SET))
		return len;
	    else
		fprintf(stderr, "Can't seek to beginning of %s file '%s'.\n", type, name);
	}
	else if (len == 0)
	    fprintf(stderr, "%s file '%s' is empty.\n", type, name);
	else if (len < 0)
	    fprintf(stderr, "Can't determine length of %s file '%s'.\n", type, name);
	else
	    fprintf(stderr, "%s file '%s' is too long"
		    " (length %ld, max %ld)\n", type, name, len, max);
    }
    else
	fprintf(stderr, "Can't seek to end of %s file '%s'.\n", type, name);

    return 0;
}

int options (int argc, char **argv)
{
    int prog = 0;
    
    if (argc < 2) return 0;

    for (int i = 1; i < argc; ++i) {
	if (0 == strcmp(argv[i], "-r")) {
	    if (++i < argc) {
		FILE * romf = fopen(argv[i], "rb");
		if (romf) {
		    long romlen = flen(romf, 0xE00, "rom", argv[i]);
		    if (romlen != 0) {
			size_t sz = fread(rom, 1, romlen, romf);
			fclose(romf);
			if (sz < romlen) {
			    fprintf(stderr,
				    "Read only %ld bytes from rom file '%s'"
				    " (length %ld).\n",
				    sz, argv[i], romlen);
			}
		    }
		}
		else
		    fprintf(stderr, "Can't open rom file '%s' for reading.\n", argv[i]);
	    }
	    else
		fprintf(stderr, "No filename specified for '-r' option.\n");
	}
	else if (prog)
	    fprintf(stderr, "Multiple program files specified. Using '%s'\n", argv[prog]);
	else
	    prog = i;
    }

    if (prog) {
	FILE * progf = fopen(argv[prog], "rb");
	if (progf) {
	    long proglen = flen(progf, 0xF000, "program", argv[prog]);
	    if (proglen == 0)
		prog = 0;
	    else {
		size_t sz = fread(mem, 1, proglen, progf);
		fclose(progf);
		if (sz != proglen) {
		    fprintf(stderr,
			    "Read only %ld bytes from program file '%s'"
			    " (length %ld).\n",
			    sz, argv[prog], proglen);
		    prog = 0;
		}
		/*
		for (size_t i = 0; i < sz; ++i) {
		    printf("%lu: %d\n", i, mem[i]);
		}
		*/
	    }
	}
	else {
	    fprintf(stderr, "Can't open program file '%s' for reading.\n", argv[prog]);
	    prog = 0;
	}
    }

    return prog;
}

