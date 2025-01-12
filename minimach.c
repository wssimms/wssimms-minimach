#include <string.h>
#include <stdio.h>
#include <stdint.h>

/*
 * Machine Description (v1.0)
 *
 * PC: 16 bit program counter
 * A: 8 bit accumulator
 * C: 8 bit register that is used as an operand in ALU operations
 *    and receives high byte results of <<, >>, +, - operations
 *
 * ext(unsigned_byte) is defined: ((uint16_t)(int16_t)(int8_t)unsigned_byte)
 *
 *  0 E                  stop processor
 *  1 L   address        A = read(address)
 *  2 S   address        write(address, A)
 *  3 SWP                swaps A and C
 *  4 AND                A = A & C
 *  5 OR                 A = A | C
 *  6 EOR                A = A ^ C
 *  7 SHL                C:A = C:A << 1
 *  8 SHR                C:A = C:A >> 1
 *  9 ADD                C:A = ext(A) + ext(C)
 * 10 SUB                C:A = ext(A) - ext(C)
 * 11 J   address        AC = PC+3; PC = address
 * 12 TST X,Y,Z          PC = PC + 1 + (A<0 ? ext(X)
 *                                          : (A=0 ? ext(Y)
 *                                                 : ext(Z)))
 *
 * Memory Map
 *   0x0000 - 0xEFFF : read/write memory
 *   0xF000 - 0xFEFF : read only memory
 *   0xFF00 - 0xFFFF : memory mapped i/o
 *
 * I/O area
 *   0xFF00 : R - getchar(), W - putchar(A)
 */

int options (int argc, char **argv);

#define EXTEND(ub) ((uint16_t)(int16_t)(int8_t)(ub))

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
    if (a < 0xF000) return mem[a];
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

//#define DBG 100

void execute (void)
{
    uint8_t opcode;
#ifdef DBG
    unsigned count = 0;
#endif    
    while (1) {
	opcode = mem[pc.w++];
#ifdef DBG	
	printf("pc:%3d  ac:%4.4x  a:%3d  [%3d]\n", pc.w-1, ac.w, ac.b[0], opcode);
	if (++count > DBG) break;
#endif	
        switch (opcode) {
        case 0:                                      /* E */
            return;
        case 1:                                      /* L */
            tm.b[0] = mem[pc.w++];
            tm.b[1] = mem[pc.w++];
	    ac.b[0] = rd(tm.w);
	    break;
        case 2:                                      /* S */
            tm.b[0] = mem[pc.w++];
            tm.b[1] = mem[pc.w++];
	    wr(tm.w, ac.b[0]);
            break;
        case 3:                                      /* SW */
            tm.b[0] = ac.b[0];
            ac.b[0] = ac.b[1];
            ac.b[1] = tm.b[0];
            break;
        case 4:                                      /* AND */
            ac.b[0] = ac.b[0] & ac.b[1];
            break;
        case 5:                                      /* OR */
            ac.b[0] = ac.b[0] | ac.b[1];
            break;
        case 6:                                      /* EOR */
            ac.b[0] = ac.b[0] ^ ac.b[1];
            break;
        case 7:                                      /* SHL */
            ac.w <<= 1;
            break;
        case 8:                                      /* SHR */
	    ac.w >>= 1;
            break;
        case 9:                                      /* ADD */
	    ac.w = EXTEND(ac.b[0]) + EXTEND(ac.b[1]);
            break;
        case 10:                                     /* SUB */
            ac.w = EXTEND(ac.b[0]) - EXTEND(ac.b[1]);
            break;
        case 11:                                     /* J */
            tm.b[0] = mem[pc.w++];
            tm.b[1] = mem[pc.w++];
	    ac.w = pc.w;
            pc.w = tm.w;
            break;
        case 12:                                     /* TST */
            if (ac.b[0] < 0)
                tm.w = EXTEND(mem[pc.w+0]);
            else if (ac.b[0] > 0)
                tm.w = EXTEND(mem[pc.w+2]);
            else
                tm.w = EXTEND(mem[pc.w+1]);
            pc.w += tm.w;
            break;
        default:
	    reset();
            break;
        }
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
	    }
	}
	else {
	    fprintf(stderr, "Can't open program file '%s' for reading.\n", argv[prog]);
	    prog = 0;
	}
    }

    return prog;
}

