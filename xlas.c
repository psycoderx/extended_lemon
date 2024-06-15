/*
xlas.c - is the XL assembler.
See LICENSE information in the end of the file.
  BUILD
gcc -std=c89 -pedantic -Wall -Wextra -o xlas xlas.c
  RUN
xlas <input-file> <output-file>
*/

#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define new(type) \
  memset(domalloc(sizeof(type)), 0, sizeof(type))

#define numof(arr) \
  (sizeof(arr)/sizeof((arr)[0]))

#define TOKCAP 512
#define SECTCAP 0x10000

enum {
  Teof     = -17,
  Tnewline = -16,
  Tconst   = -15,
  Tstrlit  = -14,
  Tsharp   = -13,
  Tdiv     = -12,
  Tmult    = -11,
  Tmore    = -10,
  Tless    = -9,
  Tqand    = -8,
  Tqor     = -7,
  Tqnor    = -6,
  Tqxor    = -5,
  Tminus   = -4,
  Tplus    = -3,
  Tcomma   = -2,
  Tcolon   = -1,
#define X(name) T##name,
#include "xli.x.h"
#undef X
  Tcount
};

enum {
  Mnam, Mimm, Mabs, Mabx, Maby, Mrel, Mzpg, Mzpx,
  Mzpy, Mvec, Mzvx, Mzyv
};

typedef struct Lexer {
  FILE *file;
  struct Lexer *prev;
  const char *filename;
  int ahead;
  int chr;
  int row;
  int col;
  int ttype;
  int trow;
  int tcol;
  int tconst;
  int tstrlit;
} Lexer;

typedef struct Tok {
  const char *filename;
  int type;
  int row;
  int col;
  int iconst;
  int strlit;
} Tok;

typedef struct Sect {
  char buf[SECTCAP];
  int size;
  int maxsize;
} Sect;

typedef struct Var {
  int name;
  int val;
  int islabel;
} Var;

typedef struct Backpatch {
  Tok tok;
  int isrel;
  int offset;
  int label;
} Backpatch;

#include "xlitab.c"

/*
Print error and exit.
*/
static void
errf(const char *fmt, ...);

/*
Read the next character.
*/
static int
lgetc(Lexer *lex);

/*
Read the next proto-token.
*/
static int
readprototok(void);

/*
Read the next token.
*/
static int
readtok(void);

/*
Convert the string to the string index.
*/
static int
getsi(const char *str);

/*
Convert the string index to the string.
*/
static const char *
getstr(int si);

/*
Garantee to malloc
*/
static void *
domalloc(size_t size);

/*
Garantee to realloc
*/
static void *
dorealloc(void *oldptr, size_t newsize);

/*
Parse integer.
*/
static int
strint(const char *s, int len, int *err_out);

/*
Check if this is an indentifier character.
*/
static int
isident(int ch);

/*
Get digit value.
*/
static int
digitv(int ch);

/*
Create new lexer.
*/
static Lexer *
newlex(const char *filename, Lexer *prev);

/*
Write bytes into the section.
*/
static void
emit(Sect *sect, const char *buf, int len);

/*
Write one byte several times into the section.
*/
static void
emitbyte(Sect *sect, unsigned char data, int times);

/*
Write one word into the section (little endian).
*/
static void
emitle16(Sect *sect, unsigned int word);

static void
emit8method(Sect *sect, int data);

static void
emit16method(Sect *sect, int data);

/*
Interpret the next line.
*/
static int
readline(void);

static void
readinst(int inst);

static void
dolet(void);

static void
dorb(void);

static void
doinclude(void);

static void
doincbin(void);

static int
evalexpr(void);

static int
evalterm(void);

static int
findvar(int name);

static int
setvar(int name, int val, int islabel);

static void
defvals(int iswords);

static void
planpatch(int offset, int label, int isrel, Tok tok);

static const char **strtab;
static int strnum;
static int strcap;

static Var *vartab;
static int varnum;
static int varcap;

static Backpatch *bptab;
static int bpnum;
static int bpcap;

static Lexer *L;
static Tok curtok;
static Sect *outbuf;

static const char *inames[] = {
#define X(name) #name,
#include "xli.x.h"
#undef X
  "unreachable"
};

/************************************************************/
int
main(int argc, char **argv)
{
  Backpatch *bp = NULL;
  FILE *out = NULL;
  const char *iname, *oname;
  size_t writn = 0;
  int i = 0, osize = 0, vi = 0, addr = 0, rel = 0, dlr = 0;
  if (argc < 3)
    errf("xlas: Missing input and output file names\n");
  iname = argv[1];
  oname = argv[2];
  out = fopen(oname, "wb");
  if (out == NULL)
    errf("%s: %s\n", oname, strerror(errno));
  L = newlex(iname, NULL);
  outbuf = new(Sect);
  outbuf->maxsize = 0x8000;
  strcap = Tcount * 2;
  strtab = domalloc(strcap * sizeof(strtab[0]));
  for (i = 0; i < Tcount; ++i)
    getsi(inames[i]);
  varcap = 8;
  vartab = domalloc(varcap * sizeof(vartab[0]));
  setvar(getsi("$"), 0, 0);
  setvar(getsi("$$"), 0x8000, 0);
  bpcap = 8;
  bptab = domalloc(bpcap * sizeof(bptab[0]));
  readtok();
  while (readline() == 0)
    /* nothing */;
  #if 1
  osize = outbuf->size;
  for (i = 0; i < bpnum; ++i) {
    bp = &bptab[i];
    vi = findvar(bp->label);
    if (vi < 0)
      errf("%s:%i:%i: The label is never defined\n"
          , bp->tok.filename, bp->tok.row, bp->tok.col);
    addr = vartab[vi].val;
    outbuf->size = bp->offset; /* hack the buffer */
    if (bp->isrel) {
      dlr = 0x8000 + bp->offset - 1;
      rel = addr - dlr;
      if (rel > 127 || rel < -128)
        errf("%s:%i:%i: The label is too far\n"
            , bp->tok.filename, bp->tok.row, bp->tok.col);
      emitbyte(outbuf, rel, 1);
    }
    else {
      emitle16(outbuf, addr);
    }
  }
  outbuf->size = osize;
  #endif
  writn = fwrite(outbuf->buf, 1, outbuf->size, out);
  if ((int)writn != outbuf->size)
    errf("%s: Cannot write the file\n", oname);
  fclose(out);
  return 0;
}

/************************************************************/
void
planpatch(int offset, int label, int isrel, Tok tok)
{
  Backpatch *bp = NULL;
  if (bpnum + 1 > bpcap) {
    bpcap *= 2;
    bptab = dorealloc(bptab, bpcap * sizeof(bptab[0]));
  }
  bp = &bptab[bpnum];
  ++bpnum;
  bp->offset = offset;
  bp->label = label;
  bp->isrel = isrel;
  bp->tok = tok;
}

/************************************************************/
int
findvar(int name)
{
  int i = 0;
  for (i = 0; i < varnum; ++i)
    if (vartab[i].name == name)
      return i;
  return -1;
}

/************************************************************/
int
setvar(int name, int val, int islabel)
{
  Var *var = NULL;
  int vi = findvar(name);
  if (vi >= 0) {
    var = &vartab[vi];
    if (var->islabel || islabel)
      return 1;
    var->val = val;
    return 0;
  }
  if (varnum + 1 > varcap) {
    varcap *= 2;
    vartab = dorealloc(vartab, varcap * sizeof(vartab[0]));
  }
  var = &vartab[varnum];
  ++varnum;
  var->name = name;
  var->val = val;
  var->islabel = islabel;
  return 0;
}

/************************************************************/
void
readinst(int inst)
{
  Tok tok = curtok, labtok;
  Pattern *p = NULL;
  int mtype = Mnam, t = 0, i = 0, val = 0;
  int label = 0, sz = 0, rel = 0, addr = 0;
  t = readtok();
  /**/ if (t == Tnewline || t == Teof) {
    mtype = Mnam;
    goto match;
  }
  else if (t == Tsharp) {
    mtype = Mimm;
    readtok();
  }
  else if (t == Tx || t == Ty) {
    mtype = (t == Tx) ? Mabx : Maby;
    readtok();
    if (curtok.type == Tmult) {
      mtype = (t == Tx) ? Mzvx : Mzyv;
      readtok();
    }
  }
  else if (t == Tmult) {
    mtype = Mvec;
    readtok();
  }
  else if (t == Tqnor) {
    mtype = Mrel;
    readtok();
  }
  else {
    mtype = Mabs;
  }
  if (curtok.type == Tnewline || curtok.type == Teof)
    errf("%s:%i:%i: No arument in the instruction\n"
        , tok.filename, tok.row, tok.col);
  if (curtok.type > Ty && findvar(curtok.type) < 0) {
    label = curtok.type;
    labtok = curtok;
    sz = (mtype != Mrel) ? 2 : 1;
    readtok();
  }
  else {
    val = evalexpr();
    sz = (val > 255) ? 2 : 1;
  }
  if (mtype == Mabx && sz == 1)
    mtype = Mzpx;
  if (mtype == Maby && sz == 1)
    mtype = Mzpy;
  if (mtype == Mabs && sz == 1)
    mtype = Mzpg;
match:
  if (curtok.type != Tnewline && curtok.type != Teof)
    errf("%s:%i:%i: Unexpected token\n"
        , curtok.filename, curtok.row, curtok.col);
  for (i = 0; i < (int)numof(itable); ++i) {
    p = &itable[i];
    if (p->inst == inst && p->amode == mtype)
      break;
  }
  if (i == numof(itable))
    errf("%s:%i:%i: Unknown instruction pattern\n"
        , tok.filename, tok.row, tok.col);
  emitbyte(outbuf, i, 1);
  if (mtype == Mimm) {
    emitbyte(outbuf, val, 1);
    return;
  }
  if (label != 0) {
    planpatch(outbuf->size, label, mtype == Mrel, labtok);
    if (sz == 1) emitbyte(outbuf, 0, 1);
    if (sz == 2) emitle16(outbuf, 0);
  }
  else if (mtype == Mrel) {
    addr = 0x8000 + outbuf->size - 1;
    rel = val - addr;
    if (rel > 127 || rel < -128)
      errf("%s:%i:%i: The location is too far\n"
          , tok.filename, tok.row, tok.col);
    emitbyte(outbuf, rel, 1);
  }
  else {
    if (sz == 1) emitbyte(outbuf, val, 1);
    if (sz == 2) emitle16(outbuf, val);
  }
}

/************************************************************/
void
dolet(void)
{
  Tok tok = curtok;
  int val = 0, name = 0, t = 0;
  name = readtok();
  if (name <= Ty)
    errf("%s:%i:%i: The let requires a name\n"
        , tok.filename, tok.row, tok.col);
  t = readtok();
  if (t == Tnewline || t == Teof)
    errf("%s:%i:%i: The let requires an expression\n"
        , tok.filename, tok.row, tok.col);
  val = evalexpr();
  if (curtok.type != Tnewline && curtok.type != Teof)
    errf("%s:%i:%i: Unexpected token\n"
        , curtok.filename, curtok.row, curtok.col);
  readtok();
  if (setvar(name, val, 0) != 0)
    errf("%s:%i:%i: Label redefinition\n"
        , tok.filename, tok.row, tok.col);
}

/************************************************************/
void
dorb(void)
{
  Tok tok = curtok;
  int n = 0, t = 0;
  t = readtok();
  if (t == Tnewline || t == Teof)
    errf("%s:%i:%i: The rb requires an expression\n"
        , tok.filename, tok.row, tok.col);
  n = evalexpr();
  if (curtok.type != Tnewline && curtok.type != Teof)
    errf("%s:%i:%i: Unexpected token\n"
        , curtok.filename, curtok.row, curtok.col);
  readtok();
  emitbyte(outbuf, 0, n);
}

/************************************************************/
void
defvals(int iswords)
{
  Tok tok = curtok;
  void(*emitmethod)(Sect *, int);
  const char *ref = (iswords) ? "dw" : "db";
  const char *str = NULL;
  int t = 0;
  emitmethod = (iswords) ? emit16method : emit8method;
  for (;;) {
    t = readtok();
    if (t == Tnewline || t == Teof)
      errf("%s:%i:%i: The %s requires an expression\n"
          , tok.filename, tok.row, tok.col, ref);
    /**/ if (t == Tstrlit) {
      str = getstr(curtok.strlit);
      emit(outbuf, str, strlen(str));
      readtok();
    }
    else if (t > Ty && findvar(t) < 0) {
      planpatch(outbuf->size, curtok.type, 0, curtok);
      emitle16(outbuf, 0);
      readtok();
    }
    else {
      emitmethod(outbuf, evalexpr());
    }
    if (curtok.type == Tnewline || curtok.type == Teof)
      break;
    if (curtok.type != Tcomma)
      errf("%s:%i:%i: Unexpected token\n"
        , curtok.filename, curtok.row, curtok.col);
    tok = curtok;
  }
  readtok();
}

/************************************************************/
void
doinclude(void)
{
  Tok tok;
  const char *filename = NULL;
  tok = curtok;
  if (readtok() != Tstrlit)
    errf("%s:%i:%i: The include requires a filename string\n"
        , tok.filename, tok.row, tok.col);
  filename = getstr(curtok.strlit);
  readtok();
  if (curtok.type != Tnewline && curtok.type != Teof)
    errf("%s:%i:%i: Unexpected token\n"
        , curtok.filename, curtok.row, curtok.col);
  readtok();
  L = newlex(filename, L);
}

/************************************************************/
void
doincbin(void)
{
  char buf[1024];
  Tok tok;
  FILE *bin = NULL;
  const char *filename = NULL;
  size_t readn = 0;
  tok = curtok;
  if (readtok() != Tstrlit)
    errf("%s:%i:%i: The incbin requires a filename string\n"
        , tok.filename, tok.row, tok.col);
  filename = getstr(curtok.strlit);
  readtok();
  if (curtok.type != Tnewline && curtok.type != Teof)
    errf("%s:%i:%i: Unexpected token\n"
        , curtok.filename, curtok.row, curtok.col);
  readtok();
  bin = fopen(filename, "rb");
  if (bin == NULL)
    errf("%s:%i:%i: incbin: %s: %s\n", tok.filename, tok.row
        , tok.col, filename, strerror(errno));
  while ((readn = fread(buf, 1, sizeof(buf), bin)) != 0)
    emit(outbuf, buf, (int)readn);
  if (ferror(bin))
    errf("%s:%i:%i: incbin: %s: %s\n", tok.filename, tok.row
        , tok.col, filename, "Cannot read the file");
  fclose(bin);
}

/************************************************************/
int
evalexpr(void)
{
  Tok tok;
  int t = 0, term = 0, val = evalterm();
  for (;;) {
    t = readtok();
    tok = curtok;
    if (t == Tnewline || t == Teof || t == Tcomma)
      break;
    readtok();
    term = evalterm();
    /**/ if (t == Tminus) val -= term;
    else if (t == Tplus)  val += term;
    else if (t == Tmult)  val *= term;
    else if (t == Tdiv) {
      if (term == 0)
        errf("%s:%i:%i: Division by zero\n"
            , curtok.filename, curtok.row, curtok.col);
      val /= term;
    }
    else if (t == Tqxor)  val ^= term;
    else if (t == Tqnor)  val = ~(val | term);
    else if (t == Tqor)   val |= term;
    else if (t == Tqand)  val &= term;
    else if (t == Tmore || t == Tless) {
      if (term < 0 || term > 16)
        errf("%s:%i:%i: Binary shift by more then 16\n"
            , curtok.filename, curtok.row, curtok.col);
      if (t == Tmore) val = val >> term;
      if (t == Tless) val = val << term;
    }
    else errf("%s:%i:%i: Invalid operator\n"
             , tok.filename, tok.row, tok.col);
  }
  return val & 0xFFFF;
}

/************************************************************/
int
evalterm(void)
{
  int vi = 0;
  if (curtok.type > Ty) {
    vi = findvar(curtok.type);
    if (vi < 0)
      errf("%s:%i:%i: Undefined identifier\n"
          , curtok.filename, curtok.row, curtok.col);
    return vartab[vi].val;
  }
  if (curtok.type == Tconst)
    return curtok.iconst;
  errf("%s:%i:%i: Is not an integer or a defined name\n"
      , curtok.filename, curtok.row, curtok.col);
  return 0;
}

/************************************************************/
int
readline(void)
{
  Tok tok;
  int t = 0;
  vartab[0].val = 0x8000 + outbuf->size;
  t = curtok.type;
  tok = curtok;
  if (t > Ty) {
    if (readtok() != Tcolon)
      errf("%s:%i:%i: No colon after the label\n"
          , tok.filename, tok.row, tok.col);
    if (setvar(t, 0x8000 + outbuf->size, 1) != 0)
      errf("%s:%i:%i: Variable or label redefinition\n"
          , tok.filename, tok.row, tok.col);
    readtok();
    return 0;
  }
  if (t == Teof) return 1;
  if (t == Tnewline) {
    readtok();
    return 0;
  }
  if (0 <= t && t < Tlet) readinst(t);
  if (t == Tlet) dolet();
  if (t == Trb) dorb();
  if (t == Tdb) defvals(0);
  if (t == Tdw) defvals(1);
  if (t == Tinclude) doinclude();
  if (t == Tincbin) doincbin();
  if (t == Tx || t == Ty || t < 0)
    errf("%s:%i:%i: Unexpected token\n"
        , tok.filename, tok.row, tok.col);
  return 0;
}

/************************************************************/
void
emit8method(Sect *sect, int data)
{
  emitbyte(sect, data, 1);
}

/************************************************************/
void
emit16method(Sect *sect, int data)
{
  emitle16(sect, data);
}

/************************************************************/
void
emitle16(Sect *sect, unsigned int word)
{
  emitbyte(sect, word & 0xFF, 1);
  emitbyte(sect, (word >> 8) & 0xFF, 1);
}

/************************************************************/
void
emitbyte(Sect *sect, unsigned char data, int times)
{
  int i = 0;
  char ch = data;
  for (i = 0; i < times; ++i)
    emit(sect, &ch, 1);
}

/************************************************************/
void
emit(Sect *sect, const char *buf, int len)
{
  if (sect->size + len > sect->maxsize)
    errf("xlx: Too much bytes in the program\n");
  memcpy(sect->buf + sect->size, buf, len);
  sect->size += len;
}

/************************************************************/
Lexer *
newlex(const char *filename, Lexer *prev)
{
  Lexer *lex = new(Lexer);
  lex->prev = prev;
  lex->filename = filename;
  lex->file = fopen(lex->filename, "rb");
  if (lex->file == NULL)
    errf("%s: %s\n", lex->filename, strerror(errno));
  lex->ahead = '\n';
  lgetc(lex);
  lgetc(lex);
  return lex;
}

/************************************************************/
int
readtok(void)
{
start:
  readprototok();
  if (L->ttype == Teof && L->prev != NULL) {
    fclose(L->file);
    L = L->prev;
    goto start;
  }
  curtok.filename = L->filename;
  curtok.row = L->trow;
  curtok.col = L->tcol;
  curtok.type = L->ttype;
  curtok.strlit = L->tstrlit;
  curtok.iconst = L->tconst;
  return curtok.type;
}

/************************************************************/
int
readprototok(void)
{
  static struct { int type; char ch; } ops[] = {
    {Tmore,   '>'}, {Tless,   '<'}, {Tqand,   '&'},
    {Tqor,    '|'}, {Tqnor,   '~'}, {Tqxor,   '^'},
    {Tminus,  '-'}, {Tplus,   '+'}, {Tdiv,    '/'},
    {Tcomma,  ','}, {Tcolon,  ':'}, {Tmult,   '*'},
    {Tsharp,  '#'}, {Tnewline, '\n'}
  };
  char buf[TOKCAP];
  int i = 0, err = 0;
  memset(buf, 0, TOKCAP);
  /* spaces and comment */
  for (;;) {
    while (L->chr != EOF && L->chr != '\n' && isspace(L->chr))
      lgetc(L);
    if (L->chr != ';')
      break;
    while (L->chr != EOF && L->chr != '\n')
      lgetc(L);
  }
  L->trow = L->row;
  L->tcol = L->col;
  if (L->chr == EOF)
    return L->ttype = Teof;
  /* operators and separators */
  for (i = 0; i < (int)numof(ops); ++i)
    if (L->chr == ops[i].ch) {
      lgetc(L);
      return L->ttype = ops[i].type;
    }
  /* string-literal */
  if (L->chr == '\'') {
    lgetc(L);
    for (i = 0; i < TOKCAP - 1; ++i) {
      if (L->chr == EOF || L->chr == '\'' || L->chr == '\n')
        break;
      buf[i] = L->chr;
      lgetc(L);
    }
    if (i + 1 == TOKCAP)
      errf("%s:%i:%i: The string is too long\n"
          , L->filename, L->trow, L->tcol);
    if (L->chr != '\'')
      errf("%s:%i:%i: Missing closing quote\n"
          , L->filename, L->trow, L->tcol);
    lgetc(L);
    L->tstrlit = getsi(buf);
    return L->ttype = Tstrlit;
  }
  /* identifier or integer constant */
  if (isident(L->chr) || L->chr == '-' || L->chr == '+') {
    i = 0;
    if (L->chr == '-' || L->chr == '+') {
      buf[i] = L->chr;
      ++i;
    }
    for (; isident(L->chr) && i < TOKCAP - 1; ++i) {
      buf[i] = L->chr;
      lgetc(L);
    }
    if (i + 1 == TOKCAP)
      errf("%s:%i:%i: The token is too long\n"
          , L->filename, L->trow, L->tcol);
    if (isdigit(buf[0])) {
      L->tconst = strint(buf, i, &err);
      if (err)
        errf("%s:%i:%i: Invalid integer constant\n"
            , L->filename, L->trow, L->tcol);
      return L->ttype = Tconst;
    }
    return L->ttype = getsi(buf);
  }
  errf("%s:%i:%i: Invalid token\n"
      , L->filename, L->trow, L->tcol);
  return 0;
}

/************************************************************/
int
strint(const char *s, int len, int *err_out)
{
  int sign = 1, base = 10, i = 0, num = 0, digit = 0, a = 0;
  if (len == 0)
    return *err_out = 1, 0;
  *err_out = 0;
  if (s[0] == '+' || s[0] == '-') {
    sign = (s[0] == '+') ? 1 : -1;
    if (len == 1)
      return *err_out = 1, 0;
    len -= 1;
    s += 1;
  }
  if (s[0] == '0' && len > 1) {
    base = 8;
    i = 1;
    a = tolower(s[1]);
    if (a == 'x') {
      base = 16;
      i = 2;
    }
    else if (a == 'o') {
      base = 8;
      i = 2;
    }
    else if (a == 'b') {
      base = 2;
      i = 2;
    }
  }
  for (; i < len; i++) {
    digit = digitv(s[i]);
    if (digit < 0 || digit >= base)
      return *err_out = 1, 0;
    num *= base;
    num += digit;
  }
  return sign * num;
}

/************************************************************/
int
digitv(int ch) {
  if ('0' <= ch && ch <= '9')
    return ch - '0';
  if ('A' <= ch && ch <= 'F')
    return ch - 'A' + 10;
  if ('a' <= ch && ch <= 'f')
    return ch - 'a' + 10;
  return -1;
}

/************************************************************/
int
isident(int ch)
{
  return isalpha(ch) || isdigit(ch) || ch == '_' || ch == '$';
}

/************************************************************/
int
getsi(const char *str)
{
  char *cpy = NULL;
  size_t len = 0;
  int i = 0;
  for (i = 0; i < strnum; ++i)
    if (strcmp(str, strtab[i]) == 0)
      return i;
  if (strnum + 1 > strcap) {
    strcap *= 2;
    strtab = dorealloc(strtab, strcap * sizeof(strtab[0]));
  }
  len = strlen(str);
  cpy = malloc(len + 1);
  memcpy(cpy, str, len + 1);
  strtab[strnum] = cpy;
  ++strnum;
  return strnum - 1;
}

/************************************************************/
const char *
getstr(int si)
{
  if (si < 0 || si >= strnum)
    errf("xlas: invalid string index\n");
  return strtab[si];
}

/************************************************************/
int
lgetc(Lexer *lex)
{
start:
  if (lex->chr == '\n') {
    lex->row++;
    lex->col = 0;
  }
  lex->col++;
  lex->chr = lex->ahead;
  lex->ahead = fgetc(lex->file);
  if (lex->chr == '\\' && lex->ahead == '\n') {
    lex->row++;
    lex->col = 0;
    lex->ahead = fgetc(lex->file);
    goto start;
  }
  return lex->chr;
}

/************************************************************/
void
errf(const char *fmt, ...)
{
  va_list args;
  assert(fmt != NULL);
  va_start(args, fmt);
  vfprintf(stderr, fmt, args);
  va_end(args);
  exit(EXIT_FAILURE);
}

/************************************************************/
void *
domalloc(size_t size)
{
  void *ptr = malloc(size);
  if (ptr == NULL)
    errf("xlas: malloc failed\n");
  return ptr;
}

/************************************************************/
void *
dorealloc(void *oldptr, size_t newsize)
{
  void *ptr = realloc(oldptr, newsize);
  if (ptr == NULL)
    errf("xlas: realloc failed\n");
  return ptr;
}

/*
MIT License

Copyright (c) 2024 Artem Pirunov

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

