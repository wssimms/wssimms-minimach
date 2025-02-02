#include <string.h>
#include <stdio.h>
#include <stdint.h>

/*
 * Machine Description (v1.3)
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
 *  4 AND                A = A & C; C = ~(A & C)
 *  5 OR                 A = A | C; C = ~(A | C)
 *  6 EOR                A = A ^ C; C = ~(A ^ C)
 *  7 SHL                C:A = C:A << 1
 *  8 SHR                C:A = C:A >> 1
 *  9 ADD                C:A = ext(A) + ext(C)
 * 10 SUB                C:A = ext(A) - ext(C)
 * 11 JUMP  address      C:A = PC + 3; PC = address
 * 12 TEST  X,Y,Z        PC = PC + 1 + (A<0 ? sext(X)
 *                                          : (A=0 ? sext(Y)
 *                                                 : sext(Z)))
 * 14 AND   address
 * 15 OR    address
 * 19 ADD   address
 * 20 SUB   address
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

FILE *diskf = 0;

uint64_t cycles = 0;

uint8_t  track = 0;
uint8_t  sector = 0;
uint16_t address = 0;

enum disk_state { DISK_IDLE=0, DISK_SUCCESS=1, DISK_ERROR=2, DISK_BUSY=4 } state;

#define DISK_ID_BYTE '2';

uint8_t rd (uint16_t a);
void wr (uint16_t a, uint8_t b);

FILE * disk_file (void)
{
    static char *default_name = "minimach.dsk";

    if (diskf == 0) {
	diskf = fopen(default_name, "r+b");
	if (diskf == 0) {
	    diskf = fopen(default_name, "w+b");
	    if (diskf) {
		if (fseek(diskf, 2*1024*1024 - 1, SEEK_SET)) {
		    fclose(diskf);
		    diskf = 0;
		}
		else if (fwrite("\0", 1, 1, diskf) != 1) {
		    fclose(diskf);
		    diskf = 0;
		}
	    }
	}
    }

    if (diskf) {
	long offs = track * 8192 + sector * 512;
	if (fseek(diskf, offs, SEEK_SET))
	    state = DISK_ERROR;
    }
    
    return diskf;
}

void get_buf (uint16_t a, uint8_t *buf)
{
    for (int i = 0; i < 512; ++i)
	buf[i] = rd((uint16_t)(a+i));
}

void put_buf (uint16_t a, uint8_t *buf)
{
    for (int i = 0; i < 512; ++i)
	wr((uint16_t)(a+i), buf[i]);
}

uint8_t disk_rd (uint16_t a)
{
    if (a == 0) {
	return DISK_ID_BYTE;
    }
    else if (a == 1) {
	return 0;        // lo byte of # of tracks (256)
    }
    else if (a == 2) {
	return 1;        // hi byte of # of tracks (256)
    }
    else if (a == 3) {
	return 16;       // # of 512 byte sectors per track
    }
    else if (a == 4)	{
	uint8_t b = (uint8_t)state;
	state = DISK_IDLE;
	//printf("status: %d\n", b);
	return b;
    }
    return 0;
}

void disk_wr (uint16_t a, uint8_t b)
{
    if (a == 0) {
	track = b;
	//printf("track: %d\n", track);
    }
    else if (a == 1) {
	sector = b & 15;
	//printf("sector: %d\n", sector);
    }
    else if (a == 2) {
	address = (address & 0xFF00) | b;
	//printf("(1) address: %d\n", address);
    }
    else if (a == 3) {
	address = (address & 0x00FF) | (b << 8);
	//printf("(2) address: %d\n", address);
    }
    else if (a == 4) {
	static uint8_t buf[512];

	if (state != DISK_IDLE) return;
	state = DISK_BUSY;
	
	FILE *f = disk_file();
	if (f == 0 || state == DISK_ERROR) return;
	
	    
	if (b & 1) {
	    //printf("write\n");
	    get_buf(address, buf);
	    if (fwrite(buf, 1, 512, f) != 512) {
		state = DISK_ERROR;
		return;
	    }
	}
	else {
	    //printf("read\n");
	    if (fread(buf, 1, 512, f) != 512) {
		state = DISK_ERROR;
		return;
	    }
	    put_buf(address, buf);
	}
	
	state = DISK_SUCCESS;
    }
}

uint8_t rd (uint16_t a)
{
    ++cycles;
    
    if (a < 0xF000) {
	return mem[a];
    }
    if (a < 0xFF00) return rom[a - 0xFF00];
    
    switch (a & 255) {
    case 0:
	return getchar();
    default:
	if ((a & 0xFFF8) == 0xFF10)
	    return disk_rd(a & 15);
	else
	    return 0;
    }
}

void wr (uint16_t a, uint8_t b)
{
    ++cycles;
    
    if (a < 0xF000) {
	mem[a] = b;
	return;
    }
    
    if (a < 0xFF00) return;

    switch (a & 255) {
    case 0:
	putchar(b); return;
    default:
	if ((a & 0xFFF8) == 0xFF10)
	    return disk_wr(a & 15, b);

    }
}

void reset (void)
{
    ac.w = 0;
    pc.b[0] = rd(0xFEFE);
    pc.b[1] = rd(0xFEFF);
}

//#define DBG 100000

void execute (void)
{
    uint8_t opcode;
#ifdef DBG
    unsigned count = 0;
#endif    
    while (1) {

	opcode = rd(pc.w++);
#ifdef DBG	
	printf("%4d ", pc.w-1);
#endif	
        switch (opcode) {
        case 0:                                      /* E */
	    printf("End at pc = %d   Total # cycles = %llu\n", pc.w-1, cycles);
            return;
	    
        case 1:                                      /* L */
            tm.b[0] = rd(pc.w++);
            tm.b[1] = rd(pc.w++);
	    ac.b[0] = rd(tm.w);
#ifdef DBG
	    printf("L    %5d (%2.2x/%3d) ", tm.w, ac.b[0], ac.b[0]);
#endif
	    break;
	    
        case 2:                                      /* S */
            tm.b[0] = rd(pc.w++);
            tm.b[1] = rd(pc.w++);
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

	    /*
        case 4:                                      // AND
            ac.b[0] = ac.b[0] & ac.b[1];
            ac.b[1] = ~(ac.b[0]);
#ifdef DBG
	    printf("AND                 ");
#endif
            break;
	    
        case 5:                                      // OR
            ac.b[0] = ac.b[0] | ac.b[1];
            ac.b[1] = ~(ac.b[0]);
#ifdef DBG
	    printf("OR                  ");
#endif
            break;
	    
        case 6:                                      // EOR
            ac.b[0] = ac.b[0] ^ ac.b[1];
            ac.b[1] = ~(ac.b[0]);
#ifdef DBG
	    printf("EOR                 ");
#endif
            break;
	    */
	    
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
	    /*
        case 9:                                      // ADD
	    ac.w = EXT(ac.b[0]) + EXT(ac.b[1]);
#ifdef DBG
	    printf("ADD                 ");
#endif
            break;
	    
        case 10:                                     // SUB
            ac.w = EXT(ac.b[0]) - EXT(ac.b[1]);
#ifdef DBG
	    printf("SUB                 ");
#endif
            break;
	    */
        case 11:                                     /* J */
            tm.b[0] = rd(pc.w++);
            tm.b[1] = rd(pc.w++);
	    ac.w = pc.w;
            pc.w = tm.w;
#ifdef DBG
	    printf("JUMP %5d          ", tm.w);
#endif
            break;
	    
        case 12:                                     /* TST */
	    if (((int16_t)SEXT(ac.b[0])) < 0)
                tm.w = SEXT(rd(pc.w+0));
            else if (((int16_t)SEXT(ac.b[0])) > 0)
                tm.w = SEXT(rd(pc.w+2));
            else
                tm.w = SEXT(rd(pc.w+1));
            pc.w += tm.w;
#ifdef DBG
	    printf("TEST  %4.4x          ", SEXT(ac.b[0]));
#endif
            break;
	    
        case 14:                                     /* AND abs */
            tm.b[0] = rd(pc.w++);
            tm.b[1] = rd(pc.w++);
            ac.b[0] = ac.b[0] & rd(tm.w);
            ac.b[1] = ~(ac.b[0]);
#ifdef DBG
	    printf("AND  %5d          ", tm.w);
#endif
	    break;
	    
        case 15:                                     /* OR abs  */
            tm.b[0] = rd(pc.w++);
            tm.b[1] = rd(pc.w++);
            ac.b[0] = ac.b[0] | rd(tm.w);
            ac.b[1] = ~(ac.b[0]);
#ifdef DBG
	    printf("OR   %5d          ", tm.w);
#endif
	    break;
	    
        case 19:                                     /* ADD abs */
            tm.b[0] = rd(pc.w++);
            tm.b[1] = rd(pc.w++);
	    ac.w = EXT(ac.b[0]) + EXT(rd(tm.w));
#ifdef DBG
	    printf("ADD  %5d          ", tm.w);
#endif
	    break;
	    
        case 20:                                     /* SUB abs */
            tm.b[0] = rd(pc.w++);
            tm.b[1] = rd(pc.w++);
	    ac.w = EXT(ac.b[0]) - EXT(rd(tm.w));
#ifdef DBG
	    printf("SUB  %5d          ", tm.w);
#endif
	    break;
	    
        default:
#ifdef DBG
	    printf("??? (%2d)              ", opcode);
#endif
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
	else if (0 == strcmp(argv[i], "-d")) {
	    if (++i < argc) {
		diskf = fopen(argv[i], "r+b");
		if (diskf == 0) {
		    diskf = fopen(argv[i], "w+b");
		    if (diskf) {
			if (fseek(diskf, 2*1024*1024 - 1, SEEK_SET)) {
			    fclose(diskf);
			    diskf = 0;
			}
			else if (fwrite("\0", 1, 1, diskf) != 1) {
			    fclose(diskf);
			    diskf = 0;
			}
		    }
		}
		printf("1b) disk file: %p\n", diskf);
	    }
	    else
		fprintf(stderr, "Can't open disk file '%s' for reading.\n", argv[i]);
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

