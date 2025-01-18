#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define TNUM 200
#define TSYM 201
#define TLAB 202
#define TSTR 203

#define TE   300
#define TL   301
#define TS   302
#define TSW  303
#define TAND 304
#define TOR  305
#define TEOR 306
#define TSHL 307
#define TSHR 308
#define TADD 309
#define TSUB 310
#define TJ   311
#define TTST 312

#define TERR 400

FILE *inf, *outf;

int dot;
unsigned pass;
unsigned lines;
unsigned errors = 0;

int oldc;
int oldtoken;
int oldtokval;

int tokval;


#define STRMAX 1024
char strbuf[STRMAX];

#define IDMAX sizeof(uint64_t)
char idbuf[IDMAX];

#define SYMTABMAX 1024
struct symbol {
    uint64_t name;
    int token;
    int value;
};

unsigned nsyms = 0;
struct symbol symtab[SYMTABMAX];
char symdef[SYMTABMAX];

//#define TEXTOUT

void emit (int x)
{
#ifdef TEXTOUT
    fprintf(outf, "[%d/%2.2x]", (unsigned char)x, (unsigned char)x);
#else
    fputc(x, outf);
#endif
}

void pushch (int c)
{
    oldc = c;
}

int nextch (void)
{
    if (oldc) {
	int c = oldc;
	oldc = 0;
	return c;
    }

    return fgetc(inf);
}

void consume (void) {
    int c;
    while (1) {
	c = nextch();
	if (c == '\n' || c == EOF) break;
    }
    pushch(c);
}

void error (char *msg) {
    fprintf(stderr, "%u: Error: %s\n", lines, msg);
    ++errors;
    consume();
}

void initsym (char *id, int t, int v)
{
    uint64_t n = 0;
    char *p = id;
    
    if (nsyms >= (SYMTABMAX - 1)) {
	fprintf(stderr, "Internal error: too many init syms.\n");
	fflush(outf);
	fclose(outf);
	fclose(inf);
	exit(1);
    }

    while (*p) ++p;
    if (p > id + 7) p = id + 7;

    while (p >= id)
	n = (n << 8) + *p--;

    symtab[nsyms].name = n;
    symtab[nsyms].token = t;
    symtab[nsyms].value = v;
    symdef[nsyms] = 1;
    
    ++nsyms;
}

void initsymtab (void)
{
    initsym("END",  TE,   0);
    initsym("L",    TL,   1);
    initsym("S",    TS,   2);
    initsym("SWAP", TSW,  3);
    initsym("AND",  TAND, 4);
    initsym("OR",   TOR,  5);
    initsym("EOR",  TEOR, 6);
    initsym("SHL",  TSHL, 7);
    initsym("SHR",  TSHR, 8);
    initsym("ADD",  TADD, 9);
    initsym("SUB",  TSUB, 10);
    initsym("JUMP", TJ,   11);
    initsym("TEST", TTST, 12);
}

void stringize (uint64_t n, char *s)
{
    char *p = s;
    char *e = s + 8;
    
    while (n && p < e) {
	*p++ = (char)(n & 255);
	n = n / 256;
    }

    *p = '\0';
}

int lookup (uint64_t *n)
{
    for (int i = 0; i < nsyms; ++i) {
	if (*n == symtab[i].name) {
	    tokval = i;
	    return symtab[i].token;
	}
    }

    if (nsyms >= SYMTABMAX) {
	fprintf(stderr, "Symbol table full.\n");
	fflush(outf);
	fclose(outf);
	fclose(inf);
	exit(1);
    }

    symtab[nsyms].name = *n;
    symtab[nsyms].token = TSYM;
    tokval = nsyms++;
    /*
    char sname[12];
    stringize(*n, sname);
    fprintf(outf, "{added symbol '%s' at index %d}", sname, nsyms);
    */
    return TSYM;
}

void define (int i, int v)
{
    char msg[44];
    
    if (i < 0 || i >= nsyms) {
	fprintf(stderr, "Internal error: bad index (%d/%d) into symbol table.\n", i, nsyms);
	fflush(outf);
	fclose(outf);
	fclose(inf);
	exit(1);
    }

    if (symtab[i].token != TSYM) {
	error("Can't define a non-symbol.");
	return;
    }

    if (pass == 0) {
	if (symdef[i]) {
	    char sname[12];
	    stringize(symtab[i].name, sname);
	    sprintf(msg, "Redefined symbol '%s'.", sname);
	    error(msg);
	}

	symtab[i].value = v;
	symdef[i] = 1;
    }
}

int escseq (void)
{
    int c = nextch();
    if (c == 'r') return 13;
    if (c == 'n') return 10;
    if (c == 't') return 8;
    if (c == '0') return 0;
    return c;
}

int scanhex (void)
{
    int c = nextch();

    tokval = 0;
    while ((c >= '0' && c <= '9') || (c >= 'A' && c <= 'F') || (c >= 'a' && c <= 'f')) {
	c &= 0xDF;
	tokval *= 16;
	if (c < 'A')
	    tokval += (c - '0' + 32);
	else
	    tokval += (c - 'A' + 10);
	c = nextch();
    }

    pushch(c);
    return TNUM;
}

int scannum (int first)
{
    int c = nextch();
    if (c == 'x' || c == 'X') return scanhex();

    tokval = first;
    while (c >= '0' && c <= '9') {
	tokval = 10 * tokval + (c - '0');
	c = nextch();
    }

    pushch(c);
    return TNUM;
}

int scanstr()
{
    int c = nextch();
    if (c == '"') {
	error("Empty string constant");
	return TERR;
    }
    
    tokval = 0;
    while (c != '"' && c != '\n' && c != EOF) {
	if (c == '\\') c = escseq();
	if (tokval < (STRMAX - 1)) strbuf[tokval++] = c;
	c = nextch();
    }

    strbuf[tokval] = '\0';

    if (c != '"') {
	pushch(c);
	error("Unterminated string constant");
	return TERR;
    }
    
    return TSTR;
}

int scanbyte()
{
    tokval = 0;

    int c = nextch();
    if (c == '\'') {
	error("Empty character constant");
	return TERR;
    }
    
    while (c != '\'' && c != '\n' && c != EOF) {
	if (c == '\\') c = escseq();
	tokval = (tokval << 8) | c;
	c = nextch();
    }

    if (c != '\'') {
	pushch(c);
	error("Unterminated character constant");
	return TERR;
    }
    
    return TNUM;
}

int scanword(int c)
{
    int tok, i = 0;

    while (1) {
	if (c < '0')  break;
	if (c <= '9') goto yes;
	if (c < 'A')  break;
	if (c <= 'Z') goto yes;
	if (c == '_') goto yes;
	if (c < 'a')  break;
	if (c > 'z')  break;

	c &= 0xDF;  /* force uppercase */
	
    yes:
	if (i < IDMAX) idbuf[i++] = c;
	c = nextch();
    }

    while (i < IDMAX) idbuf[i++] = 0;

    tok = lookup((uint64_t *)idbuf);

    if (c == ':') {
	if (tok == TSYM) {
	    define(tokval, dot);
	    tok = symtab[tokval].token = TLAB;
	    /*
	    char sname[12];
	    stringize(symtab[tokval].name, sname);
	    fprintf(outf, "{'%s' is now a label.}", sname);
	    */
	}
	else if (tok == TLAB) {
	    if (pass == 0)
		error("Label redefined.");
	}
	else {
	    error("Bad label.");
	}
	return tok;
    }

    pushch(c);
    return tok;
}

int scan (void)
{
    int c = ' ';

    if (oldtoken) {
      tokval = oldtokval;
      c = oldtoken;
      oldtoken = 0;
      return c;
    }
    
    while (1) {
	c = nextch();
	
	while (c == ' ' || c == '\t')
	    c = nextch();

	if (c == EOF) return c;
	if (c == '\n') return c;
    
	if (c >= '0' && c <= '9')
	    return scannum(c - '0');

	if ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z'))
	    return scanword(c);

	if (c == '"')
	    return scanstr();

	if (c == '\'')
	    return scanbyte();

	if (c == ';') {
	    while (c != '\n' && c != EOF)
		c = nextch();
	    pushch(c);
	    continue;
	}
	
	if (c == '.') return c;
	if (c == '=') return c;
	if (c == ',') return c;
	if (c == '!') return c;
	if (c == '+') return c;
	if (c == '-') return c;
	if (c == '*') return c;
	if (c == '/') return c;
	if (c == '%') return c;
	if (c == '&') return c;
	if (c == '|') return c;
	if (c == '^') return c;
	if (c == '(') return c;
	if (c == ')') return c;
	if (c == '<') return c;
	if (c == '>') return c;

	error("Bad character in input.");
    }
}

void pushtok (int token)
{
    oldtoken = token;
    oldtokval = tokval;
}

int expression (int *value);

int term (int *value)
{
    int token;
    int result;
    
    token = scan();
    if (token == '(') {
	result = expression(value);
	if (result < 0) return result;
	
	token = scan();
	if (token != ')') return -1;
	return 0;
    }

    if (token == '.') {
	*value = dot;
	return 0;
    }

    if (token == TNUM) {
	*value = tokval;
	return 0;
    }

    if (token == TSYM || token == TLAB) {
	char sname[12];
	stringize(symtab[tokval].name, sname);
	//printf("%s: %d\n", sname, symtab[tokval].value);
	*value = symtab[tokval].value;
	return 0;
    }

    error("Syntax error in expression.");
    return -1;
}

int unary (int *value)
{
    int result;
    int token;

    token = scan();
    switch (token) {
    case '!':
	result = term(value);
	if (result == 0)
	    *value = -1 ^ *value;
	return result;
	
    case '-':
	result = term(value);
	if (result == 0)
	    *value = 0 - *value;
	return result;
	
    case '+':
	result = term(value);
	return result;

    case '<':
	result = term(value);
	if (result == 0)
	    *value = *value % 256;
	return result;
	
    case '>':
	result = term(value);
	if (result == 0)
	    *value = (*value / 256) % 256;
	return result;
	
    default:
	pushtok(token);
	return term(value);
    }
}

int product (int *value)
{
    int result;
    int token;
    int v1, v2;

    result = unary(&v1);
    if (result < 0) return result;

    while (1) {
	token = scan();
	if (token != '*' && token != '/' && token != '%') {
	    pushtok(token);
	    break;
	}

	result = unary(&v2);
	if (result < 0) return result;

	if (token == '*')
	    v1 *= v2;
	else if (token == '/')
	    v1 /= v2;
	else
	    v1 %= v2;
    }

    *value = v1;
    return result;
}

int sum (int *value)
{
    int result;
    int token;
    int v1, v2;

    result = product(&v1);
    if (result < 0) return result;

    while (1) {
	token = scan();
	if (token != '+' && token != '-') {
	    pushtok(token);
	    break;
	}

	result = product(&v2);
	if (result < 0) return result;

	if (token == '+') {
	    //printf("%d + %d = %d\n", v1, v2, v1+v2);
	    v1 += v2;
	}
	else
	    v1 -= v2;
    }

    *value = v1;
    return result;
}

int conjunction (int *value)
{
    int result;
    int token;
    int v1, v2;

    result = sum(&v1);
    if (result < 0) return result;

    while (1) {
	token = scan();
	if (token != '&') {
	    pushtok(token);
	    break;
	}

	result = sum(&v2);
	if (result < 0) return result;

	v1 &= v2;
    }

    *value = v1;
    return result;
}

int expression (int *value)
{
    int result;
    int token;
    int v1, v2;

    result = conjunction(&v1);
    if (result < 0) return result;

    while (1) {
	token = scan();
	if (token != '|' && token != '^') {
	    //printf("Push %d\n", token);
	    pushtok(token);
	    break;
	}

	result = conjunction(&v2);
	if (result < 0) return result;

	if (token == '|')
	    v1 |= v2;
	else
	    v1 ^= v2;
    }

    *value = v1;
    //printf("Expression value: %d\n", v1);
    return result;
}

void outtoken (int token);

void line (void)
{
    int opcode;
    int result;
    int token;
    int value = 0;

    //fprintf(stderr, "%4.4d: ", lines);
    token = scan();
    
    while (token == TLAB) {
	//fprintf(stderr, "[label]");
	token = scan();
    }

    if (token == TSYM || token == '.') {
	//fprintf(stderr, "{Here I am}");
	int symval = tokval;
	int nexttok = scan();
	if (nexttok == '=')
	{
	    result = expression(&value);
	    if (result == 0) {
		if (token == TSYM) {
		    define(symval, value);
		}
		else {
		    if (value < dot) {
			error("Dot may only be advanced.");
			return;
		    }
		    while (dot < value) {
			if (pass > 0) emit(0);
			++dot;
		    }
		}
	    }
	    return;
	}
	else {
	    tokval = symval;
	}
    }

    while (1) {
	switch (token) {
	case '.':
	case '!':
	case '<':
	case '>':
	case '+':
	case '-':
	case TNUM:
	case TSYM:
	    pushtok(token);
	    result = expression(&value);
	    if (result < 0) return;
	    ++dot;
	    if (pass > 0) emit(value);
	    break;
	
	case TSTR:
	    for (value = 0; strbuf[value]; ++value) {
		++dot;
		if (pass > 0) emit(strbuf[value]);
	    }
	    break;
	
	case TE:
	case TSW:
	case TAND:
	case TOR:
	case TEOR:
	case TSHL:
	case TSHR:
	case TADD:
	case TSUB:
	    ++dot;
	    if (pass > 0) emit(symtab[tokval].value);
	    //outtoken(token);
	    return;
	
	case TL:
	case TS:
	case TJ:
	    opcode = symtab[tokval].value;
	    result = expression(&value);
	    if (result < 0) return;
	    //printf("Got %d\n", value);
	    dot += 3;
	    if (pass > 0) {
		emit(opcode);
		emit(value % 256);
		emit(value / 256);
	    }
	    //outtoken(token);
	    return;
	
	case TTST: {
	    //fprintf(stderr, "{TEST}");
	    int t1, t2, t3;

	    opcode = symtab[tokval].value;
	    
	    result = expression(&t1);
	    if (result < 0) return;
	
	    token = scan();
	    if (token != ',') {
		error("Syntax error in TEST (1).");
		return;
	    }
	
	    result = expression(&t2);
	    if (result < 0) return;
	
	    token = scan();
	    if (token != ',') {
		error("Syntax error in TEST (2).");
		return;
	    }
	
	    result = expression(&t3);
	    if (result < 0) return;

	    ++dot;

	    if (pass > 0) {
		value = t1 - dot;
		if (value < -128 || value > 127) {
		    //fprintf(stderr, "(t:%d, dot:%d, t-dot:%d) ", t1, dot, t1-dot);
		    error("TEST operand 1 out of range.");
		}
	
		value = t2 - dot;
		if (value < -128 || value > 127) {
		    //fprintf(stderr, "(t:%d, dot:%d, t-dot:%d) ", t2, dot, t2-dot);
		    error("TEST operand 2 out of range.");
		}
	
		value = t3 - dot;
		if (value < -128 || value > 127) {
		    //fprintf(stderr, "(t:%d, dot:%d, t-dot:%d) ", t3, dot, t3-dot);
		    error("TEST operand 3 out of range.");
		}
	
		emit(opcode);
		emit(t1 - dot);
		emit(t2 - dot);
		emit(t3 - dot);
	    }

	    dot += 3;
	    //outtoken(TTST);
	    return;
	}

	case '\n':
	case EOF:
	    pushtok(token);
	    return;
    
	default: {
	    char msg[40];
	    sprintf(msg, "Unexpected token (%d).", token);
	    error(msg);
	    return;
	}
	}

	token = scan();
	//printf("Read %d/%d\n", token, ',');
	
	if (token != ',') {
	    pushtok(token);
	    return;
	}

	token = scan();
    }
}

void dopass (void)
{
    int token;
    
    rewind(inf);

    while (1) {
	line();
	token = scan();
	if (token == EOF) break;
	if (token != '\n') {
	    error("Spurious characters at end of line.");
	    pushtok(token);
	    consume();
	}
#ifdef TEXTOUT
	if (pass > 0)
	    fprintf(outf, "\n%4d ", dot);
#endif
	++lines;
    }
}

void dumpsyms (void)
{
    char sname[12];

    for (int i = 0; i < nsyms; ++i) {
	if (symtab[i].token == TSYM || symtab[i].token == TLAB) {
	    stringize(symtab[i].name, sname);
	    if (symdef[i]) {
		if (symtab[i].token == TLAB)
		    printf("%3d:  Label %s = %d\n", i, sname, symtab[i].value);
		else
		    printf("%3d: Symbol %s = %d\n", i, sname, symtab[i].value);
	    }
	    else {
		printf("%3d: Symbol %s = undefined\n", i, sname);
	    }
	}
    }
}

int openfiles (char * fname)
{
    int i, j;
    
    inf = fopen(fname, "rb");
    if (inf == 0) {
	fprintf(stderr, "Can't open input file '%s'.\n", fname);
	return -1;
    }
    
    for (i = 0; i < (STRMAX - 3) && fname[i]; ++i)
	strbuf[i] = fname[i];

    for (j = i; j > 0; --j)
	if (strbuf[j] == '.')
	    break;

    if (j > 0)
	strcpy(strbuf+j, ".o");
    else
	strcpy(strbuf+i, ".o");

    outf = fopen(strbuf, "wb");
    if (inf == 0) {
	fprintf(stderr, "Can't open output file '%s'.\n", strbuf);
	fclose(inf);
	return -1;
    }

    return 0;
}

void assemble (char *fname)
{
    if (openfiles(fname) < 0) return;
    
    initsymtab();

    printf("%s: 0\n", fname);

    dot = 0;
    pass = 0;
    lines = 1;
    dopass();

    for (int i = 0; i < nsyms; ++i) {
	if (symdef[i] == 0) {
	    char sname[12];
	    stringize(symtab[i].name, sname);
	    fprintf(stderr, "Undefined symbol '%s'.\n", sname);
	    ++errors;
	}
    }
    
    if (errors) return;
    
    printf("%s: 1\n", fname);
    
    dot = 0;
    pass = 1;
    lines = 1;
    dopass();

    fclose(outf);
    fclose(inf);
    
    dumpsyms();    
}

//#define TESTSCAN

void outtoken (int token)
{
    char sname[12];
    FILE *outf = stderr;
    
    switch (token) {
    case TNUM: fprintf(outf, "[%d]", tokval); break;
    case TSTR: fprintf(outf, "\"%s\"", strbuf); break;
	
    case TLAB:
	stringize(symtab[tokval].name, sname);
	fprintf(outf, "[%s:]", sname);
	break;
	
    case TSYM:
	stringize(symtab[tokval].name, sname);
	fprintf(outf, "[%s]", sname);
	break;
	
    case TE:   fprintf(outf, "<END>");  break;
    case TL:   fprintf(outf, "<L>");    break;
    case TS:   fprintf(outf, "<S>");    break;
    case TSW:  fprintf(outf, "<SWAP>"); break;
    case TAND: fprintf(outf, "<AND>");  break;
    case TOR:  fprintf(outf, "<OR>");   break;
    case TEOR: fprintf(outf, "<EOR>");  break;
    case TSHL: fprintf(outf, "<SHL>");  break;
    case TSHR: fprintf(outf, "<SHR>");  break;
    case TADD: fprintf(outf, "<ADD>");  break;
    case TSUB: fprintf(outf, "<SUB>");  break;
    case TJ:   fprintf(outf, "<JUMP>"); break;
    case TTST: fprintf(outf, "<TEST>"); break;

    case '\n':
 	fprintf(outf, "$\n");
	fprintf(outf, "L%4d: ", lines);
	break;
	
    default:
 	fputc(token, outf);
	break;
    }
}

#ifdef TESTSCAN
void testscan (char *fname)
{
    int token;
    
    if (openfiles(fname) < 0) return;

    initsymtab();
    fprintf(outf, "L%4d: ", lines);
    
    while (1) {
	token = scan();
	if (token == EOF) break;
	outtoken(token);
    }
    
    fclose(outf);
    fclose(inf);
}

int main (int argc, char **argv)
{    
    if (argc > 1)
	testscan(argv[1]);
}
#endif

#ifndef TESTSCAN
int main (int argc, char **argv)
{
    for (int i = 1; i < argc; ++i) {
	assemble(argv[i]);
    }

    return 0;
}
#endif

