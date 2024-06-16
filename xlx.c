/*
xlx.c - is a simple XL powered virtual vachine.
See LICENSE information in the end of the file.
  BUILD
gcc -std=c89 -pedantic -Wall -Wextra -o xlx xlx.c
 or
gcc -std=c89 -pedantic -Wall -Wextra -DXLXDB -o xlxdb xlx.c
  RUN
xlx <input-files...>
 or
xlxdb <input-files...>
*/

#include <assert.h>
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "extended_lemon.h"
#include "extended_lemon_extra.h"

#ifdef XLXDB
#define XLXDEBUG 1
#else
#define XLXDEBUG 0
#endif

typedef struct XLX {
  XL_Byte mem[0x10000];
  const char *filename;
  int stop;
} XLX;

/*
Print error and exit.
*/
void
errf(const char *fmt, ...);

/*
Error method to handle XL exceptions.
*/
void
xlx_error(XL *xl, XL_Uint ecode);

/*
Load method to read one byte at address.
*/
XL_Byte
xlx_load(XL *xl, XL_Word addr);

/*
Store method to write one byte to address.
*/
void
xlx_store(XL *xl, XL_Word addr, XL_Byte data);

/*
Open the file with XLX.
*/
void
xlx_init(XLX *xlx, const char *filename);

/*
Printf the difference.
*/
void
xlxdb_diff(XL *prevxl, XL *xl);

/************************************************************/
int
main(int argc, char **argv)
{
  static XLX static_xlx;
  static XL static_xl;
  static XL static_pervxl;
  XL *xl = &static_xl;
  XL *prevxl = &static_pervxl;
  XLX *xlx = &static_xlx;
  XL_Combo *p = NULL;
  time_t t0, t;
  int i = 0, c = 0, m = 0, n = 0, val = 0;
  XL_Word addr = 0;
  if (argc < 2)
    errf("xlx: No input files\n");
  XL_init(xl);
  xl->error = xlx_error;
  xl->load = xlx_load;
  xl->store = xlx_store;
  xl->userdata = (void *)xlx;
  t0 = t = time(NULL);
  for (i = 1; i < argc; ++i) {
    xlx_init(xlx, argv[i]);
    XL_restart(xl);
    memcpy(prevxl, xl, sizeof(*xl));
    prevxl->p = xlx->mem[0xFFFE];
    prevxl->p |= xlx->mem[0xFFFF] << 8;
    while (!xlx->stop) {
      if (!XLXDEBUG) {
        for (c = 0; c < XL_FREQ && !xlx->stop; ++c)
          XL_cycle(xl);
        while (t0 == t && !xlx->stop)
          t = time(NULL);
        t0 = t;
      }
      else {
        while (!XL_cycle(xl))
          /* nothing */;
        p = &XL_combos[xlx->mem[prevxl->p]];
        n = XL_modesizes[p->amode];
        m = p->amode;
        fprintf(stderr, " %04X  ", prevxl->p);
        fprintf(stderr, "%s", XL_keywords[p->inst]);
        fprintf(stderr, "%s", XL_msignatures[m]);
        addr = prevxl->p + 1;
        if (n == 2) {
          val = xlx->mem[addr];
          /**/ if (m == Mimm) {
            fprintf(stderr, "%i", val);
          }
          else if (m == Mrel) {
            if (val > 127)
              val |= ~0xFF;
            addr = prevxl->p + val;
            fprintf(stderr, "%i -> 0x%04X", val, addr);
          }
          else {
            fprintf(stderr, "0x%02X", val);
          }
        }
        if (n == 3) {
          val = xlx->mem[addr];
          ++addr;
          val |= xlx->mem[addr] << 8;
          fprintf(stderr, "0x%04X", val);
        }
        xlxdb_diff(prevxl, xl);
        fprintf(stderr, "\n");
        memcpy(prevxl, xl, sizeof(*xl));
      }
    }
  }
  return 0;
}

/************************************************************/
void
xlxdb_diff(XL *prevxl, XL *xl)
{
  int diff = (prevxl->f != xl->f) || (prevxl->a != xl->a)
          || (prevxl->s != xl->s) || (prevxl->x != xl->x)
          || (prevxl->y != xl->y);
  if (diff)
    fprintf(stderr, " >>>");
  if (prevxl->f != xl->f) {
    fprintf(stderr, " f: %c%c%c%c%c%c%c%c;"
           , XL_get_flag(xl, XL_FLAG_Z) ? 'Z' : '-'
           , XL_get_flag(xl, XL_FLAG_V) ? 'V' : '-'
           , XL_get_flag(xl, XL_FLAG_U) ? 'U' : '-'
           , XL_get_flag(xl, XL_FLAG_R) ? 'R' : '-'
           , XL_get_flag(xl, XL_FLAG_N) ? 'N' : '-'
           , XL_get_flag(xl, XL_FLAG_D) ? 'D' : '-'
           , XL_get_flag(xl, XL_FLAG_C) ? 'C' : '-'
           , XL_get_flag(xl, XL_FLAG_B) ? 'B' : '-');
  }
  if (prevxl->a != xl->a)
    fprintf(stderr, " a = %i;", xl->a);
  if (prevxl->s != xl->s)
    fprintf(stderr, " s = %i;", xl->s);
  if (prevxl->x != xl->x)
    fprintf(stderr, " x = %i;", xl->x);
  if (prevxl->y != xl->y)
    fprintf(stderr, " y = %i;", xl->y);
}

/************************************************************/
void
xlx_init(XLX *xlx, const char *filename)
{
  FILE *file = NULL;
  size_t readn = 0;
  assert(xlx != NULL && filename != NULL);
  memset(xlx, 0, sizeof(*xlx));
  xlx->filename = filename;
  file = fopen(filename, "rb");
  if (file == NULL)
    errf("%s: %s\n", filename, strerror(errno));
  readn = fread(xlx->mem + 0x8000, 1, 0x8000, file);
  if (ferror(file))
    errf("%s: Cannot read the file\n", filename);
  if (readn < 0x8000)
    errf("%s: Too few bytes in the file\n", filename);
  fclose(file);
}

/************************************************************/
void
xlx_error(XL *xl, XL_Uint ecode)
{
  XLX *xlx = xl->userdata;
  assert(xlx != NULL && ecode == XL_ERR_INVALID);
  xl->p -= 1;
  errf("%s: Invalid instruction executed at 0x%04X\n"
      , xlx->filename, (int)xl->p);
}

/************************************************************/
XL_Byte
xlx_load(XL *xl, XL_Word addr)
{
  XLX *xlx = xl->userdata;
  int ch = 0;
  assert(xlx != NULL);
  if (addr == 0x00FF) {
    if (!XLXDEBUG) {
      return getchar();
    }
    else {
      ch = getchar();
      fprintf(stderr, "input VVV %i\n", ch);
      return ch;
    }
  }
  return xlx->mem[addr];
}

/************************************************************/
void
xlx_store(XL *xl, XL_Word addr, XL_Byte data)
{
  XLX *xlx = xl->userdata;
  assert(xlx != NULL);
  if (addr == 0x00FF) {
    if (!XLXDEBUG) {
      putchar(data);
    }
    else {
      putchar(data);
      fprintf(stderr, "output VVV %i\n", data);
    }
  }
  if (addr <= 0x7FFE)
    xlx->mem[addr] = data;
  if (addr == 0x7FFF)
    xlx->stop = 1;
  if (addr >= 0x8000)
    errf("%s: Attempt to write to 0x%04X"
        , xlx->filename, addr);
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

#define EXTENDED_LEMON_C
#include "extended_lemon.h"
#define XL_EXTRA_C
#include "extended_lemon_extra.h"

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

