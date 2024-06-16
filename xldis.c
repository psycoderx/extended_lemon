/*
xldis.c - is the XL disassembler.
See LICENSE information in the end of the file.
  BUILD
gcc -std=c89 -pedantic -Wall -Wextra -o xldis xldis.c
  RUN
xldis <input-files...>
*/

#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define numof(arr) \
  (sizeof(arr)/sizeof((arr)[0]))

#include "extended_lemon.h"

enum {
#define X(name) T##name,
#include "xli.x.h"
#undef X
  Tcount
};

enum {
  Mnam, Mimm, Mabs, Mabx, Maby, Mrel, Mzpg, Mzpx,
  Mzpy, Mvec, Mzvx, Mzyv
};

#include "xlitab.c"

static const char *inames[] = {
#define X(name) #name,
#include "xli.x.h"
#undef X
  "unreachable"
};

static int sizes[] = {
  1, 2, 3, 3, 3, 2, 2, 2, 2, 3, 2, 2
};

static const char *intnames[] = {
  "reserved", "break", "react", "reset"
};

/*
Print error and exit.
*/
static void
errf(const char *fmt, ...);

/************************************************************/
int
main(int argc, char **argv)
{
  unsigned char prg[0x8000];
  FILE *f = NULL;
  const char *name = NULL;
  Pattern *p = NULL;
  size_t readn = 0;
  int a = 0, i = 0, n = 0, k = 0, limit = 0, zeros = 0;
  int m = 0, nomem = 0, val = 0, prgsize = 0;
  XL_Word addr = 0;
  if (argc < 2)
    errf("xldis: No input files\n");
  prgsize = 0x8000 - 8;
  for (a = 1; a < argc; ++a) {
    name = argv[a];
    f = fopen(name, "rb");
    if (f == NULL)
      errf("%s: %s\n", name, strerror(errno));
    readn = fread(prg, 1, 0x8000, f);
    if (ferror(f))
      errf("%s: Cannot read the file\n", name);
    if (readn < 0x8000)
      errf("%s: Too few bytes in the file\n", name);
    fclose(f);
    printf("   \'%s\'\n", name);
    printf("_addr__txt__b1_b2_b3__xlas_________________\n");
    zeros = 0;
    for (i = 0; i < prgsize; i += n) {
      n = 0;
      printf(" %04X  ", 0x8000 + i);
      for (zeros = 0; prg[i] == 0 && i < prgsize; ++i)
        ++zeros;
      if (zeros > 0) {
        printf("               rb %i\n", zeros);
        zeros = 0;
        continue;
      }
      p = &itable[prg[i]];
      n = sizes[p->amode];
      nomem = i + n > prgsize;
      if (nomem) limit = prgsize;
      else       limit = i + n;
      for (k = i; k < limit; ++k)
        putchar(isprint(prg[k]) ? prg[k] : '.');
      if (n == 2) printf(" ");
      if (n == 1) printf("  ");
      putchar(' ');
      for (k = i; k < limit; ++k)
        printf(" %02X", prg[k]);
      if (n == 2) printf("   ");
      if (n == 1) printf("      ");
      if (nomem) {
        putchar('\n');
        continue;
      }
      printf("  %s", inames[p->inst]);
      m = p->amode;
      if (m == Mabs || m == Mzpg) printf(" ");
      if (m == Mabx || m == Mzpx) printf(" x ");
      if (m == Maby || m == Mzpy) printf(" y ");
      if (m == Mzvx) printf(" x *");
      if (m == Mzyv) printf(" y *");
      if (m == Mvec) printf(" *");
      if (m == Mimm) printf(" #");
      if (m == Mrel) printf(" ~");
      if (n == 2) {
        val = prg[i + 1];
        /**/ if (m == Mimm) {
          printf("%i", val);
        }
        else if (m == Mrel) {
          if (val > 127)
            val |= ~0xFF;
          addr = 0x8000 + i + val;
          printf("%i -> 0x%04X", val, addr);
        }
        else {
          printf("0x%02X", val);
        }
      }
      if (n == 3) {
        val = (prg[i + 2] << 8) | prg[i + 1];
        printf("0x%04X", val);
      }
      putchar('\n');
    }
    k = 0;
    for (i = 0x8000 - 8; i < 0x8000; i += 2) {
      val = (prg[i + 1] << 8) | prg[i];
      printf(" %04X                 dw ", 0x8000 + i);
      printf("0x%04X; %s\n", val, intnames[k]);
      ++k;
    }
  }
  return 0;
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

