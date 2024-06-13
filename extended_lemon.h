/*
extended_lemon.h - Extended Lemon Microprocessor.
See LICENSE information in the end of the file.

--------------------------------------------------------------
                        HOW TO INCLUDE
--------------------------------------------------------------

This file is a single header C libary!
Do this to include the header code:
```c
#include <extended_lemon.h>
```
And this to include the source code:
```c
#define EXTENDED_LEMON_C
#include <extended_lemon.h>
```
No linking is required.

--------------------------------------------------------------
                          OPTIONS
--------------------------------------------------------------

You can define declaration specifiers and a calling convention
for the libary functions:
```c
#define XL_DECL static
#define XL_CALL __cdecl
#include <extended_lemon.h>
```
and...
```c
#define XLI_DECL static
#define XLI_CALL __cdecl
#define EXTENDED_LEMON_C
#include <extended_lemon.h>
```

--------------------------------------------------------------
                        HOW TO USE
--------------------------------------------------------------

Before using your XL state in other functions you should
init it with `XL_init` function. You can and probably want
to specify methods for XL to load/store data. And save
a pointer to your data to use that data in the methods.

Once you have inited XL state you should start the CPU with
`XL_restart` function. After that the CPU is ready to cycle.
Call `XL_cycle` function `XL_FREQ` times per second to get
the designed frequency of the CPU.

```c
  XL *xl = malloc(sizeof(*xl));
  XL_init(xl);
  xl->error = my_error;
  xl->load = my_load;
  xl->store = my_store;
  xl->userdata = (void *)&my_data;
  XL_restart(xl);
  for (;;) {
    for (int i = 0; i < XL_FREQ; i++) {
      XL_cycle(xl);
    }
    // wait for the next second...
  }
```
*/

/************************************************************/
/* OPTIONS                                                  */
/************************************************************/

/*
Calling convention for the public XL functions.
*/
#ifndef XL_CALL
#define XL_CALL
#endif

/*
Calling convention for the private XL functions.
*/
#ifndef XLI_CALL
#define XLI_CALL
#endif

/*
Declaration specifiers the for public XL functions.
*/
#ifndef XL_DECL
#define XL_DECL
#endif

/*
Declaration specifiers the for private XL functions.
*/
#ifndef XLI_DECL
#define XLI_DECL
#endif

/************************************************************/
/* HEADER CODE                                              */
/************************************************************/

#ifndef EXTENDED_LEMON_H
#define EXTENDED_LEMON_H

#ifdef __cplusplus
extern "C" {
#endif

/************************************************************/
/* INCLUDES                                                 */
/************************************************************/

#include <stddef.h>

/************************************************************/
/* DEFINES                                                  */
/************************************************************/

/*
XL cycles per second.
*/
#define XL_FREQ 1000020

/************************************************************/
/* TYPES                                                    */
/************************************************************/

/*
XL error type.
*/
enum {
  XL_ERR_NO, /* No errors */
  XL_ERR_INVALID, /* Invalid instructions executed */
  XL_NUM_ERRS /* The number of error values */
};

/*
XL status flags mask.
*/
enum {
  XL_FLAG_B = (1 << 0),
  XL_FLAG_C = (1 << 1),
  XL_FLAG_D = (1 << 2),
  XL_FLAG_N = (1 << 3),
  XL_FLAG_R = (1 << 4),
  XL_FLAG_U = (1 << 5),
  XL_FLAG_V = (1 << 6),
  XL_FLAG_Z = (1 << 7)
};

/*
XL state.
See the definition below.
*/
typedef struct XL XL;

/*
1-bit unsigned integer.
*/
typedef unsigned char XL_Bool;

/*
8-bit unsigned integer.
*/
typedef unsigned char XL_Byte;

/*
16-bit unsigned integer.
*/
typedef unsigned short XL_Word;

/*
Unsigned integer (at least 16-bit).
*/
typedef unsigned int XL_Uint;

/*
Error method to handle XL exceptions.
\param ecode one of XL_ERR_.
*/
typedef void(*XL_Error_Func)(XL *xl, XL_Uint ecode);

/*
Load method to read one byte at address.
\param addr 16-bit address where to read.
*/
typedef XL_Byte(*XL_Load_Func)(XL *xl, XL_Word addr);

/*
Store method to write one byte to address.
\param addr 16-bit address where to write.
\param data The byte to write.
*/
typedef void(*XL_Store_Func)(XL *xl, XL_Word addr, XL_Byte data);

/*
XL state.
*/
struct XL {
  void *userdata; /* User-defined data */
  XL_Error_Func error; /* Error method */
  XL_Load_Func load; /* Load method */
  XL_Store_Func store; /* Store method */
  XL_Word icycles; /* Current instruction cycles */
  XL_Word addr; /* Address from the address mode */
  XL_Word p; /* The program counter */
  XL_Byte a; /* The accumulator */
  XL_Byte f; /* The status flags */
  XL_Byte s; /* The stack index */
  XL_Byte x; /* The X register */
  XL_Byte y; /* The Y register */
  XL_Bool next_b_flag; /* The B flag on the next interrupt */
  XL_Bool is_invalid; /* invalid insructions executed */
  XL_Bool is_break; /* Break interrupt */
  XL_Bool is_react; /* React interrupt */
  XL_Bool is_reset; /* Reset interrupt */
};

/************************************************************/
/* FUNCTIONS                                                */
/************************************************************/

/*
    GLOSSARY
XL_cycle           | Run 1 CPU cycle
XL_get_flag        | Get a flag from the status flags
XL_init            | Init a new CPU state
XL_int_break       | Request a BREAK interrupt
XL_int_react       | Request a REACT interrupt
XL_int_reset       | Request a RESET interrupt
XL_no_error        | Dummy error callback
XL_no_load         | Dummy load callback
XL_no_store        | Dummy store callback
XL_restart         | Start/restart CPU
XL_set_flag        | Set a flag in the status flags
*/

/*
Run exactly one CPU cycle. Return 1 if this CPU cycle has
started a new instruction.

Call this function `XL_FREQ` times per second to get
the true frequency of the CPU (the designed frequency).
You can cycle the CPU less frequently for debug purposes
or any other reason.
*/
XL_DECL XL_Bool XL_CALL
XL_cycle(XL *xl);

/*
Return the value of a flag.
*/
XL_DECL XL_Bool XL_CALL
XL_get_flag(XL *xl, XL_Byte fmask);

/*
Init XL state with default values. Call this function to get
a valid XL state. Then you can use that valid XL state in
other functions.
*/
XL_DECL void XL_CALL
XL_init(XL *xl);

/*
Request a BREAK interrupt. The requested interrupt will be
processed on the next cycle. The request can be ignored due
to priority and the D flag.
*/
XL_DECL void XL_CALL
XL_int_break(XL *xl);

/*
Request a REACT interrupt. The requested interrupt will be
processed on the next cycle. The request can be ignored due
to priority.
*/
XL_DECL void XL_CALL
XL_int_react(XL *xl);

/*
Request a RESET interrupt. The requested interrupt will be
processed on the next cycle.
*/
XL_DECL void XL_CALL
XL_int_reset(XL *xl);

/*
Deafult XL error method that just ignores all errors.
*/
XL_DECL void XL_CALL
XL_no_error(XL *xl, XL_Uint ecode);

/*
Deafult XL load method that always loads 0.
*/
XL_DECL XL_Byte XL_CALL
XL_no_load(XL *xl, XL_Word addr);

/*
Deafult XL store method that does nothing.
*/
XL_DECL void XL_CALL
XL_no_store(XL *xl, XL_Word addr, XL_Byte data);

/*
Start/restart XL microprocessor.
This function puts the CPU in a state as if it
was just powered. So after a call to this fuction
the CPU is ready to cycle.
*/
XL_DECL void XL_CALL
XL_restart(XL *xl);

/*
Change the value of a flag.
*/
XL_DECL void XL_CALL
XL_set_flag(XL *xl, XL_Byte fmask, XL_Bool value);

#ifdef __cplusplus
};
#endif

#endif /* EXTENDED_LEMON_H */

/************************************************************/
/* SOURCE CODE                                              */
/************************************************************/

#ifdef EXTENDED_LEMON_C
#ifndef EXTENDED_LEMON_IMPLEMENTED
#define EXTENDED_LEMON_IMPLEMENTED

/************************************************************/
/* PRIVATE TYPES                                            */
/************************************************************/

typedef void(*XLI_Addr_Mode_Func)(XL *xl);

typedef void(*XLI_Instruction_Func)(XL *xl);

typedef struct XLI_Opcode {
  XLI_Addr_Mode_Func am;
  XLI_Instruction_Func in;
} XLI_Opcode;

/************************************************************/
/* ADDRESS MODES                                            */
/************************************************************/

#define XLI_ADEC(name) XLI_DECL void XLI_am_ ## name(XL *xl)
#define XLI_ADEF(name) void XLI_am_ ## name(XL *xl)

XLI_ADEC(nam); XLI_ADEC(imm); XLI_ADEC(abs); XLI_ADEC(abx);
XLI_ADEC(aby); XLI_ADEC(rel); XLI_ADEC(zpg); XLI_ADEC(zpx);
XLI_ADEC(zpy); XLI_ADEC(vec); XLI_ADEC(zvx); XLI_ADEC(zyv);

/************************************************************/
/* INSTRUTIONS                                              */
/************************************************************/

#define XLI_IDEC(name) XLI_DECL void XLI_in_ ## name(XL *xl)
#define XLI_IDEF(name) void XLI_in_ ## name(XL *xl)

XLI_IDEC(inv); XLI_IDEC(nop);
XLI_IDEC(brk); XLI_IDEC(rti); XLI_IDEC(ret);
XLI_IDEC(for); XLI_IDEC(fnd); XLI_IDEC(clc);
XLI_IDEC(app); XLI_IDEC(amm); XLI_IDEC(spp); XLI_IDEC(smm);
XLI_IDEC(xpp); XLI_IDEC(xmm); XLI_IDEC(ypp); XLI_IDEC(ymm);
XLI_IDEC(inc); XLI_IDEC(dec);
XLI_IDEC(jfb); XLI_IDEC(jfc); XLI_IDEC(jfd); XLI_IDEC(jfn);
XLI_IDEC(jfr); XLI_IDEC(jfu); XLI_IDEC(jfv); XLI_IDEC(jfz);
XLI_IDEC(jtb); XLI_IDEC(jtc); XLI_IDEC(jtd); XLI_IDEC(jtn);
XLI_IDEC(jtr); XLI_IDEC(jtu); XLI_IDEC(jtv); XLI_IDEC(jtz);
XLI_IDEC(jmp); XLI_IDEC(cal);
XLI_IDEC(lda); XLI_IDEC(ldx); XLI_IDEC(ldy);
XLI_IDEC(sta); XLI_IDEC(stx); XLI_IDEC(sty);
XLI_IDEC(pla); XLI_IDEC(plf); XLI_IDEC(plx); XLI_IDEC(ply);
XLI_IDEC(pha); XLI_IDEC(phf); XLI_IDEC(phx); XLI_IDEC(phy);
XLI_IDEC(taf); XLI_IDEC(tas); XLI_IDEC(tax); XLI_IDEC(tay);
XLI_IDEC(tfa); XLI_IDEC(tsa); XLI_IDEC(txa); XLI_IDEC(tya);
XLI_IDEC(cmp); XLI_IDEC(cpx); XLI_IDEC(cpy);
XLI_IDEC(sbc); XLI_IDEC(sub); XLI_IDEC(adc); XLI_IDEC(add);
XLI_IDEC(bor); XLI_IDEC(xor); XLI_IDEC(and); XLI_IDEC(bit);
XLI_IDEC(not); XLI_IDEC(nta);
XLI_IDEC(shl); XLI_IDEC(shr); XLI_IDEC(sla); XLI_IDEC(sra);
XLI_IDEC(zra); XLI_IDEC(zrx); XLI_IDEC(zry);

/************************************************************/
/* FUNCTIONS                                                */
/************************************************************/

XLI_DECL XLI_Opcode *XLI_CALL
XLI_get_opcode(XL_Byte value);

XLI_DECL XL_Word XLI_CALL
XLI_load_word(XL *xl, XL_Word addr);

XLI_DECL XL_Word XLI_CALL
XLI_load_word_zpg(XL *xl, XL_Word addr);

XLI_DECL void XLI_CALL
XLI_set_flag(XL *xl, XL_Byte fmask, XL_Bool value);

XLI_DECL XL_Bool XLI_CALL
XLI_get_flag(XL *xl, XL_Byte fmask);

XLI_DECL void XLI_CALL
XLI_push_word(XL *xl, XL_Word data);

XLI_DECL void XLI_CALL
XLI_push(XL *xl, XL_Byte data);

XLI_DECL XL_Byte XLI_CALL
XLI_pull(XL *xl);

XLI_DECL XL_Word XLI_CALL
XLI_pull_word(XL *xl);

XLI_DECL XL_Byte XLI_CALL
XLI_alu_add(XL *xl, XL_Byte a, XL_Byte b, XL_Bool c);

XLI_DECL XL_Byte XLI_CALL
XLI_alu_sub(XL *xl, XL_Byte a, XL_Byte b, XL_Bool c);

XLI_DECL XL_Byte XLI_CALL
XLI_alu_shr(XL *xl, XL_Byte a, XL_Bool c);

XLI_DECL XL_Byte XLI_CALL
XLI_alu_shl(XL *xl, XL_Byte a, XL_Bool c);

XLI_DECL XL_Byte XLI_CALL
XLI_alu_inc(XL *xl, XL_Byte a);

XLI_DECL XL_Byte XLI_CALL
XLI_alu_dec(XL *xl, XL_Byte a);

XLI_DECL XL_Byte XLI_CALL
XLI_alu_bor(XL *xl, XL_Byte a, XL_Byte b);

XLI_DECL XL_Byte XLI_CALL
XLI_alu_xor(XL *xl, XL_Byte a, XL_Byte b);

XLI_DECL XL_Byte XLI_CALL
XLI_alu_and(XL *xl, XL_Byte a, XL_Byte b);

XLI_DECL XL_Byte XLI_CALL
XLI_alu_not(XL *xl, XL_Byte a);

/************************************************************/
/* IMPLEMENTATION                                           */
/************************************************************/

/************************************************************/
void
XL_init(XL *xl)
{
  if (xl == NULL) {
    return;
  }
  xl->userdata = NULL;
  xl->error = XL_no_error;
  xl->load = XL_no_load;
  xl->store = XL_no_store;
  xl->icycles = 0;
  xl->addr = 0;
  xl->p = 0;
  xl->a = 0;
  xl->f = 0;
  xl->s = 0;
  xl->x = 0;
  xl->y = 0;
  xl->next_b_flag = 0;
  xl->is_invalid = 0;
  xl->is_break = 0;
  xl->is_react = 0;
  xl->is_reset = 0;
}

/************************************************************/
void
XL_int_break(XL *xl)
{
  if (xl == NULL) {
    return;
  }
  xl->is_break = 1;
}

/************************************************************/
void
XL_int_react(XL *xl)
{
  if (xl == NULL) {
    return;
  }
  xl->is_react = 1;
}

/************************************************************/
void
XL_int_reset(XL *xl)
{
  if (xl == NULL) {
    return;
  }
  xl->is_reset = 1;
}

/************************************************************/
XL_Bool
XL_cycle(XL *xl)
{
  XLI_Opcode *op = NULL;
  XL_Word int_addr = 0;
  XL_Bool go_int = 0;
  /**/
  if (xl == NULL) {
    return 0;
  }
  if (xl->icycles != 0) {
    xl->icycles -= 1;
    return 0;
  }
  if (xl->is_reset) {
    xl->icycles = 1;
    xl->is_reset = 0;
    xl->is_break = 0;
    xl->is_react = 0;
    xl->next_b_flag = 0;
    xl->p = XLI_load_word(xl, 0xFFFE);
    xl->a = 0;
    xl->f = 0;
    xl->s = 0;
    xl->x = 0;
    xl->y = 0;
    return 0;
  }
  if (xl->is_break) {
    xl->is_break = 0;
    int_addr = 0xFFFA;
    go_int = !XLI_get_flag(xl, XL_FLAG_D);
  }
  if (xl->is_react) {
    xl->is_react = 0;
    int_addr = 0xFFFC;
    go_int = 1;
  }
  if (go_int) {
    xl->icycles = 4;
    XLI_push_word(xl, xl->p);
    XLI_push(xl, xl->f);
    xl->p = XLI_load_word(xl, int_addr);
    XLI_set_flag(xl, XL_FLAG_D, 1);
    XLI_set_flag(xl, XL_FLAG_B, xl->next_b_flag);
    xl->next_b_flag = 0;
    return 0;
  }
  op = XLI_get_opcode(xl->load(xl, xl->p));
  xl->p += 1;
  op->am(xl);
  op->in(xl);
  return 1;
}

/************************************************************/
void
XL_no_error(XL *xl, XL_Uint ecode)
{
  (void) xl;
  (void) ecode;
}

/************************************************************/
XL_Byte
XL_no_load(XL *xl, XL_Word addr)
{
  (void) xl;
  (void) addr;
  return 0;
}

/************************************************************/
void
XL_no_store(XL *xl, XL_Word addr, XL_Byte data)
{
  (void) xl;
  (void) addr;
  (void) data;
}

/************************************************************/
void
XL_restart(XL *xl)
{
  if (xl == NULL) {
    return;
  }
  xl->is_reset = 1;
}

/************************************************************/
void
XL_set_flag(XL *xl, XL_Byte fmask, XL_Bool value)
{
  if (xl == NULL) {
    return;
  }
  XLI_set_flag(xl, fmask, value);
}

/************************************************************/
XL_Bool
XL_get_flag(XL *xl, XL_Byte fmask)
{
  if (xl == NULL) {
    return 0;
  }
  return XLI_get_flag(xl, fmask);
}

/************************************************************/
void
XLI_set_flag(XL *xl, XL_Byte fmask, XL_Bool value)
{
  if (value) {
    xl->f |= fmask;
  }
  else {
    xl->f &= ~fmask;
  }
}

/************************************************************/
XL_Bool
XLI_get_flag(XL *xl, XL_Byte fmask)
{
  return (xl->f & fmask) != 0;
}

/************************************************************/
XL_Word
XLI_load_word(XL *xl, XL_Word addr)
{
  XL_Byte lsb = xl->load(xl, addr);
  XL_Byte msb = xl->load(xl, addr + 1);
  return (msb << 8) | lsb;
}

/************************************************************/
XL_Word
XLI_load_word_zpg(XL *xl, XL_Word addr)
{
  XL_Byte lsb = xl->load(xl, (addr) & 0xFF);
  XL_Byte msb = xl->load(xl, (addr + 1) & 0xFF);
  return (msb << 8) | lsb;
}

/************************************************************/
XL_Byte
XLI_pull(XL *xl)
{
  xl->s -= 1;
  return xl->load(xl, 0x0100 | xl->s);
}

/************************************************************/
XL_Word
XLI_pull_word(XL *xl)
{
  XL_Byte lsb = XLI_pull(xl);
  XL_Byte msb = XLI_pull(xl);
  return (msb << 8) | lsb;
}

/************************************************************/
void
XLI_push(XL *xl, XL_Byte data)
{
  xl->store(xl, 0x0100 | xl->s, data);
  xl->s += 1;
}

/************************************************************/
void
XLI_push_word(XL *xl, XL_Word data)
{
  XLI_push(xl, data >> 8);
  XLI_push(xl, data & 0xFF);
}

/************************************************************/
XL_Byte
XLI_alu_add(XL *xl, XL_Byte a, XL_Byte b, XL_Bool c)
{
  XL_Word tt = a + b + c;
  XL_Byte t = tt;
  XL_Byte v = ~(a ^ b) & (a ^ t) & 0x80;
  /**/
  XLI_set_flag(xl, XL_FLAG_V, (v) != 0);
  XLI_set_flag(xl, XL_FLAG_C, (tt & 0xFF00) != 0);
  XLI_set_flag(xl, XL_FLAG_Z, (t) == 0);
  XLI_set_flag(xl, XL_FLAG_N, (t & 0x80) != 0);
  return t;
}

/************************************************************/
XL_Byte
XLI_alu_sub(XL *xl, XL_Byte a, XL_Byte notb, XL_Bool c)
{
  XL_Byte b = ~notb;
  XL_Word tt = a + b + c;
  XL_Byte t = tt;
  XL_Byte v = (t ^ a) & (t ^ b) & 0x80;
  /**/
  XLI_set_flag(xl, XL_FLAG_V, (v) != 0);
  XLI_set_flag(xl, XL_FLAG_C, (tt & 0xFF00) != 0);
  XLI_set_flag(xl, XL_FLAG_Z, (t) == 0);
  XLI_set_flag(xl, XL_FLAG_N, (t & 0x80) != 0);
  return t;
}

/************************************************************/
XL_Byte
XLI_alu_shr(XL *xl, XL_Byte a, XL_Bool c)
{
  XL_Byte t = a >> 1;
  if (c) {
    t |= 0x80;
  }
  XLI_set_flag(xl, XL_FLAG_C, (a & 1) != 0);
  XLI_set_flag(xl, XL_FLAG_Z, (t) == 0);
  XLI_set_flag(xl, XL_FLAG_N, (t & 0x80) != 0);
  return t;
}

/************************************************************/
XL_Byte
XLI_alu_shl(XL *xl, XL_Byte a, XL_Bool c)
{
  XL_Byte t = (a << 1) | c;
  XLI_set_flag(xl, XL_FLAG_C, (a & 0x80) != 0);
  XLI_set_flag(xl, XL_FLAG_Z, (t == 0));
  XLI_set_flag(xl, XL_FLAG_N, (t & 0x80) != 0);
  return t;
}

/************************************************************/
XL_Byte
XLI_alu_inc(XL *xl, XL_Byte a)
{
  XL_Byte t = a + 1;
  XLI_set_flag(xl, XL_FLAG_Z, (t == 0));
  XLI_set_flag(xl, XL_FLAG_N, (t & 0x80) != 0);
  return t;
}

/************************************************************/
XL_Byte
XLI_alu_dec(XL *xl, XL_Byte a)
{
  XL_Byte t = a - 1;
  XLI_set_flag(xl, XL_FLAG_Z, (t == 0));
  XLI_set_flag(xl, XL_FLAG_N, (t & 0x80) != 0);
  return t;
}

/************************************************************/
XL_Byte
XLI_alu_bor(XL *xl, XL_Byte a, XL_Byte b)
{
  XL_Byte t = a | b;
  XLI_set_flag(xl, XL_FLAG_Z, (t == 0));
  XLI_set_flag(xl, XL_FLAG_N, (t & 0x80) != 0);
  return t;
}

/************************************************************/
XL_Byte
XLI_alu_xor(XL *xl, XL_Byte a, XL_Byte b)
{
  XL_Byte t = a ^ b;
  XLI_set_flag(xl, XL_FLAG_Z, (t == 0));
  XLI_set_flag(xl, XL_FLAG_N, (t & 0x80) != 0);
  return t;
}

/************************************************************/
XL_Byte
XLI_alu_and(XL *xl, XL_Byte a, XL_Byte b)
{
  XL_Byte t = a & b;
  XLI_set_flag(xl, XL_FLAG_Z, (t == 0));
  XLI_set_flag(xl, XL_FLAG_N, (t & 0x80) != 0);
  return t;
}

/************************************************************/
XL_Byte
XLI_alu_not(XL *xl, XL_Byte a)
{
  XL_Byte t = ~a;
  XLI_set_flag(xl, XL_FLAG_Z, (t == 0));
  XLI_set_flag(xl, XL_FLAG_N, (t & 0x80) != 0);
  return t;
}

/************************************************************/
XLI_ADEF(nam)
{
  (void) xl;
}

/************************************************************/
XLI_ADEF(imm)
{
  xl->addr = xl->p;
  xl->p += 1;
}

/************************************************************/
XLI_ADEF(abs)
{
  xl->addr = XLI_load_word(xl, xl->p);
  xl->p += 2;
  xl->icycles += 2;
}

/************************************************************/
XLI_ADEF(abx)
{
  xl->addr = XLI_load_word(xl, xl->p) + (XL_Word)xl->x;
  xl->p += 2;
  xl->icycles += 2;
}

/************************************************************/
XLI_ADEF(aby)
{
  xl->addr = XLI_load_word(xl, xl->p) + (XL_Word)xl->y;
  xl->p += 2;
  xl->icycles += 2;
}

XLI_ADEF(rel)
{
  XL_Word offset = 0;
  XL_Byte data = xl->load(xl, xl->p);
  xl->p += 1;
  offset = data;
  if (offset > 127)
    offset |= 0xFF00;
  xl->addr = xl->p + offset - 2;
  xl->icycles += 1;
}

/************************************************************/
XLI_ADEF(zpg)
{
  xl->addr = xl->load(xl, xl->p);
  xl->p += 1;
  xl->icycles += 1;
}

/************************************************************/
XLI_ADEF(zpx)
{
  xl->addr = (xl->load(xl, xl->p) + xl->x) & 0xFF;
  xl->p += 1;
  xl->icycles += 1;
}

/************************************************************/
XLI_ADEF(zpy)
{
  xl->addr = (xl->load(xl, xl->p) + xl->y) & 0xFF;
  xl->p += 1;
  xl->icycles += 1;
}

/************************************************************/
XLI_ADEF(vec)
{
  XL_Word vec = XLI_load_word(xl, xl->p);
  xl->p += 2;
  xl->addr = XLI_load_word(xl, vec);
  xl->icycles += 4;
}

/************************************************************/
XLI_ADEF(zvx)
{
  XL_Word vec = xl->load(xl, xl->p);
  xl->p += 1;
  xl->addr = XLI_load_word_zpg(xl, vec) + xl->x;
  xl->icycles += 3;
}

/************************************************************/
XLI_ADEF(zyv)
{
  XL_Word vec = (xl->load(xl, xl->p) + xl->y) & 0xFF;
  xl->p += 1;
  xl->addr = XLI_load_word_zpg(xl, vec);
  xl->icycles += 3;
}

/************************************************************/
XLI_IDEF(inv)
{
  if (!xl->is_invalid) {
    xl->is_invalid = 1;
    xl->error(xl, XL_ERR_INVALID);
  }
}

/************************************************************/
XLI_IDEF(nop)
{
  (void) xl;
}

/************************************************************/
XLI_IDEF(brk)
{
  xl->is_break = 1;
  xl->next_b_flag = 1;
}

/************************************************************/
XLI_IDEF(rti)
{
  xl->f = XLI_pull(xl);
  xl->p = XLI_pull_word(xl);
  xl->icycles += 3;
}

/************************************************************/
XLI_IDEF(ret)
{
  xl->p = XLI_pull_word(xl);
  xl->icycles += 2;
}

/************************************************************/
XLI_IDEF(for)
{
  xl->f |= xl->load(xl, xl->addr);
  xl->icycles += 1;
}

/************************************************************/
XLI_IDEF(fnd)
{
  xl->f &= xl->load(xl, xl->addr);
  xl->icycles += 1;
}

/************************************************************/
XLI_IDEF(clc)
{
  XLI_set_flag(xl, XL_FLAG_C, 0);
}

/************************************************************/
XLI_IDEF(app)
{
  xl->a = XLI_alu_inc(xl, xl->a);
}

/************************************************************/
XLI_IDEF(amm)
{
  xl->a = XLI_alu_dec(xl, xl->a);
}

/************************************************************/
XLI_IDEF(spp)
{
  xl->s = XLI_alu_inc(xl, xl->s);
}

/************************************************************/
XLI_IDEF(smm)
{
  xl->s = XLI_alu_dec(xl, xl->s);
}

/************************************************************/
XLI_IDEF(xpp)
{
  xl->x = XLI_alu_inc(xl, xl->x);
}

/************************************************************/
XLI_IDEF(xmm)
{
  xl->x = XLI_alu_dec(xl, xl->x);
}

/************************************************************/
XLI_IDEF(ypp)
{
  xl->y = XLI_alu_inc(xl, xl->y);
}

/************************************************************/
XLI_IDEF(ymm)
{
  xl->y = XLI_alu_dec(xl, xl->y);
}

/************************************************************/
XLI_IDEF(inc)
{
  XL_Byte data = xl->load(xl, xl->addr);
  data = XLI_alu_inc(xl, data);
  xl->store(xl, xl->addr, data);
  xl->icycles += 2;
}

/************************************************************/
XLI_IDEF(dec)
{
  XL_Byte data = xl->load(xl, xl->addr);
  data = XLI_alu_dec(xl, data);
  xl->store(xl, xl->addr, data);
  xl->icycles += 2;
}

#define XLI_JMP_IF(fmask, value) \
  if (XLI_get_flag(xl, (fmask)) == (value)) { \
    xl->p = xl->addr; \
  }

/************************************************************/
XLI_IDEF(jfb)
{
  XLI_JMP_IF(XL_FLAG_B, 0);
}

/************************************************************/
XLI_IDEF(jfc)
{
  XLI_JMP_IF(XL_FLAG_C, 0);
}

/************************************************************/
XLI_IDEF(jfd)
{
  XLI_JMP_IF(XL_FLAG_D, 0);
}

/************************************************************/
XLI_IDEF(jfn)
{
  XLI_JMP_IF(XL_FLAG_N, 0);
}

/************************************************************/
XLI_IDEF(jfr)
{
  XLI_JMP_IF(XL_FLAG_R, 0);
}

/************************************************************/
XLI_IDEF(jfu)
{
  XLI_JMP_IF(XL_FLAG_U, 0);
}

/************************************************************/
XLI_IDEF(jfv)
{
  XLI_JMP_IF(XL_FLAG_V, 0);
}

/************************************************************/
XLI_IDEF(jfz)
{
  XLI_JMP_IF(XL_FLAG_Z, 0);
}

/************************************************************/
XLI_IDEF(jtb)
{
  XLI_JMP_IF(XL_FLAG_B, 1);
}

/************************************************************/
XLI_IDEF(jtc)
{
  XLI_JMP_IF(XL_FLAG_C, 1);
}

/************************************************************/
XLI_IDEF(jtd)
{
  XLI_JMP_IF(XL_FLAG_D, 1);
}

/************************************************************/
XLI_IDEF(jtn)
{
  XLI_JMP_IF(XL_FLAG_N, 1);
}

/************************************************************/
XLI_IDEF(jtr)
{
  XLI_JMP_IF(XL_FLAG_R, 1);
}

/************************************************************/
XLI_IDEF(jtu)
{
  XLI_JMP_IF(XL_FLAG_U, 1);
}

/************************************************************/
XLI_IDEF(jtv)
{
  XLI_JMP_IF(XL_FLAG_V, 1);
}

/************************************************************/
XLI_IDEF(jtz)
{
  XLI_JMP_IF(XL_FLAG_Z, 1);
}

/************************************************************/
XLI_IDEF(jmp)
{
  xl->p = xl->addr;
}

/************************************************************/
XLI_IDEF(cal)
{
  XLI_push_word(xl, xl->p);
  xl->p = xl->addr;
  xl->icycles += 2;
}

/************************************************************/
XLI_IDEF(lda)
{
  xl->a = xl->load(xl, xl->addr);
  XLI_set_flag(xl, XL_FLAG_Z, (xl->a) == 0);
  XLI_set_flag(xl, XL_FLAG_N, (xl->a & 0x80) != 0);
  xl->icycles += 1;
}

/************************************************************/
XLI_IDEF(ldx)
{
  xl->x = xl->load(xl, xl->addr);
  XLI_set_flag(xl, XL_FLAG_Z, (xl->x) == 0);
  XLI_set_flag(xl, XL_FLAG_N, (xl->x & 0x80) != 0);
  xl->icycles += 1;
}

/************************************************************/
XLI_IDEF(ldy)
{
  xl->y = xl->load(xl, xl->addr);
  XLI_set_flag(xl, XL_FLAG_Z, (xl->y) == 0);
  XLI_set_flag(xl, XL_FLAG_N, (xl->y & 0x80) != 0);
  xl->icycles += 1;
}

/************************************************************/
XLI_IDEF(sta)
{
  xl->store(xl, xl->addr, xl->a);
  xl->icycles += 1;
}

/************************************************************/
XLI_IDEF(stx)
{
  xl->store(xl, xl->addr, xl->x);
  xl->icycles += 1;
}

/************************************************************/
XLI_IDEF(sty)
{
  xl->store(xl, xl->addr, xl->y);
  xl->icycles += 1;
}

/************************************************************/
XLI_IDEF(pla)
{
  xl->a = XLI_pull(xl);
  XLI_set_flag(xl, XL_FLAG_Z, (xl->a) == 0);
  XLI_set_flag(xl, XL_FLAG_N, (xl->a & 0x80) != 0);
  xl->icycles += 1;
}

/************************************************************/
XLI_IDEF(plf)
{
  xl->f = XLI_pull(xl);
  xl->icycles += 1;
}

/************************************************************/
XLI_IDEF(plx)
{
  xl->x = XLI_pull(xl);
  XLI_set_flag(xl, XL_FLAG_Z, (xl->x) == 0);
  XLI_set_flag(xl, XL_FLAG_N, (xl->x & 0x80) != 0);
  xl->icycles += 1;
}

/************************************************************/
XLI_IDEF(ply)
{
  xl->y = XLI_pull(xl);
  XLI_set_flag(xl, XL_FLAG_Z, (xl->y) == 0);
  XLI_set_flag(xl, XL_FLAG_N, (xl->y & 0x80) != 0);
  xl->icycles += 1;
}

/************************************************************/
XLI_IDEF(pha)
{
  XLI_push(xl, xl->a);
  xl->icycles += 1;
}

/************************************************************/
XLI_IDEF(phf)
{
  XLI_push(xl, xl->f);
  xl->icycles += 1;
}

/************************************************************/
XLI_IDEF(phx)
{
  XLI_push(xl, xl->x);
  xl->icycles += 1;
}

/************************************************************/
XLI_IDEF(phy)
{
  XLI_push(xl, xl->y);
  xl->icycles += 1;
}

/************************************************************/
XLI_IDEF(taf)
{
  xl->f = xl->a;
}

/************************************************************/
XLI_IDEF(tas)
{
  xl->s = xl->a;
}

/************************************************************/
XLI_IDEF(tax)
{
  xl->x = xl->a;
}

/************************************************************/
XLI_IDEF(tay)
{
  xl->y = xl->a;
}

/************************************************************/
XLI_IDEF(tfa)
{
  xl->a = xl->f;
}

/************************************************************/
XLI_IDEF(tsa)
{
  xl->a = xl->s;
}

/************************************************************/
XLI_IDEF(txa)
{
  xl->a = xl->x;
}

/************************************************************/
XLI_IDEF(tya)
{
  xl->a = xl->y;
}

/************************************************************/
XLI_IDEF(cmp)
{
  XL_Byte data = xl->load(xl, xl->addr);
  XLI_alu_sub(xl, xl->a, data, 0);
  xl->icycles += 1;
}

/************************************************************/
XLI_IDEF(cpx)
{
  XL_Byte data = xl->load(xl, xl->addr);
  XLI_alu_sub(xl, xl->x, data, 0);
  xl->icycles += 1;
}

/************************************************************/
XLI_IDEF(cpy)
{
  XL_Byte data = xl->load(xl, xl->addr);
  XLI_alu_sub(xl, xl->y, data, 0);
  xl->icycles += 1;
}

/************************************************************/
XLI_IDEF(sbc)
{
  XL_Bool c = XLI_get_flag(xl, XL_FLAG_C);
  XL_Byte data = xl->load(xl, xl->addr);
  xl->a = XLI_alu_sub(xl, xl->a, data, c);
  xl->icycles += 1;
}

/************************************************************/
XLI_IDEF(sub)
{
  XL_Byte data = xl->load(xl, xl->addr);
  xl->a = XLI_alu_sub(xl, xl->a, data, 0);
  xl->icycles += 1;
}

/************************************************************/
XLI_IDEF(adc)
{
  XL_Bool c = XLI_get_flag(xl, XL_FLAG_C);
  XL_Byte data = xl->load(xl, xl->addr);
  xl->a = XLI_alu_add(xl, xl->a, data, c);
  xl->icycles += 1;
}

/************************************************************/
XLI_IDEF(add)
{
  XL_Byte data = xl->load(xl, xl->addr);
  xl->a = XLI_alu_add(xl, xl->a, data, 0);
  xl->icycles += 1;
}

/************************************************************/
XLI_IDEF(bor)
{
  XL_Byte data = xl->load(xl, xl->addr);
  xl->a = XLI_alu_bor(xl, xl->a, data);
  xl->icycles += 1;
}

/************************************************************/
XLI_IDEF(xor)
{
  XL_Byte data = xl->load(xl, xl->addr);
  xl->a = XLI_alu_xor(xl, xl->a, data);
  xl->icycles += 1;
}

/************************************************************/
XLI_IDEF(and)
{
  XL_Byte data = xl->load(xl, xl->addr);
  xl->a = XLI_alu_and(xl, xl->a, data);
  xl->icycles += 1;
}

/************************************************************/
XLI_IDEF(bit)
{
  XLI_alu_and(xl, xl->a, xl->load(xl, xl->addr));
  xl->icycles += 1;
}

/************************************************************/
XLI_IDEF(not)
{
  XL_Byte data = xl->load(xl, xl->addr);
  xl->store(xl, xl->addr, XLI_alu_not(xl, data));
  xl->icycles += 2;
}

/************************************************************/
XLI_IDEF(nta)
{
  xl->a = XLI_alu_not(xl, xl->a);
}

/************************************************************/
XLI_IDEF(shl)
{
  XL_Bool c = XLI_get_flag(xl, XL_FLAG_C);
  XL_Byte data = xl->load(xl, xl->addr);
  xl->store(xl, xl->addr, XLI_alu_shl(xl, data, c));
  xl->icycles += 2;
}

/************************************************************/
XLI_IDEF(shr)
{
  XL_Bool c = XLI_get_flag(xl, XL_FLAG_C);
  XL_Byte data = xl->load(xl, xl->addr);
  xl->store(xl, xl->addr, XLI_alu_shr(xl, data, c));
  xl->icycles += 2;
}

/************************************************************/
XLI_IDEF(sla)
{
  xl->a = XLI_alu_shl(xl, xl->a, XLI_get_flag(xl, XL_FLAG_C));
}

/************************************************************/
XLI_IDEF(sra)
{
  xl->a = XLI_alu_shr(xl, xl->a, XLI_get_flag(xl, XL_FLAG_C));
}

/************************************************************/
XLI_IDEF(zra)
{
  xl->a = 0;
}

/************************************************************/
XLI_IDEF(zrx)
{
  xl->x = 0;
}

/************************************************************/
XLI_IDEF(zry)
{
  xl->y = 0;
}

/************************************************************/
/* OPCODES                                                  */
/************************************************************/

XLI_Opcode *XLI_CALL
XLI_get_opcode(XL_Byte value)
{
#define XLI_OP(in, am) { XLI_am_ ## am, XLI_in_ ## in }
  static XLI_Opcode table[256] = {
    /* $00 Specials and register incdec */
    XLI_OP(inv, nam), XLI_OP(brk, nam),
    XLI_OP(rti, nam), XLI_OP(ret, nam),
    XLI_OP(for, imm), XLI_OP(fnd, imm),
    XLI_OP(clc, nam), XLI_OP(nop, nam),
    XLI_OP(app, nam), XLI_OP(amm, nam),
    XLI_OP(spp, nam), XLI_OP(smm, nam),
    XLI_OP(xpp, nam), XLI_OP(xmm, nam),
    XLI_OP(ypp, nam), XLI_OP(ymm, nam),
    /* $10 Conditional jumps */
    XLI_OP(jfb, rel), XLI_OP(jfc, rel),
    XLI_OP(jfd, rel), XLI_OP(jfn, rel),
    XLI_OP(jfr, rel), XLI_OP(jfu, rel),
    XLI_OP(jfv, rel), XLI_OP(jfz, rel),
    XLI_OP(jtb, rel), XLI_OP(jtc, rel),
    XLI_OP(jtd, rel), XLI_OP(jtn, rel),
    XLI_OP(jtr, rel), XLI_OP(jtu, rel),
    XLI_OP(jtv, rel), XLI_OP(jtz, rel),
    /* $20 Stack ops */
    XLI_OP(pha, nam), XLI_OP(phf, nam),
    XLI_OP(phx, nam), XLI_OP(phy, nam),
    XLI_OP(pla, nam), XLI_OP(plf, nam),
    XLI_OP(plx, nam), XLI_OP(ply, nam),
    XLI_OP(taf, nam), XLI_OP(tas, nam),
    XLI_OP(tax, nam), XLI_OP(tay, nam),
    XLI_OP(tfa, nam), XLI_OP(tsa, nam),
    XLI_OP(txa, nam), XLI_OP(tya, nam),
    /* $30 */
    XLI_OP(lda, imm), XLI_OP(lda, abs),
    XLI_OP(lda, zpg), XLI_OP(lda, vec),
    XLI_OP(lda, abx), XLI_OP(lda, aby),
    XLI_OP(lda, zpx), XLI_OP(lda, zpy),
    XLI_OP(zra, nam), XLI_OP(sta, abs),
    XLI_OP(sta, zpg), XLI_OP(sta, vec),
    XLI_OP(sta, abx), XLI_OP(sta, aby),
    XLI_OP(sta, zpx), XLI_OP(sta, zpy),
    /* $40 */
    XLI_OP(zrx, nam), XLI_OP(ldx, imm),
    XLI_OP(ldx, abs), XLI_OP(ldx, aby),
    XLI_OP(ldx, zpg), XLI_OP(ldx, zpy),
    XLI_OP(ldx, vec), XLI_OP(ldx, zyv),
    XLI_OP(zry, nam), XLI_OP(ldy, imm),
    XLI_OP(ldy, abs), XLI_OP(ldy, abx),
    XLI_OP(ldy, zpg), XLI_OP(ldy, zpx),
    XLI_OP(ldy, vec), XLI_OP(ldy, zvx),
    /* $50 */
    XLI_OP(cmp, imm), XLI_OP(cmp, abs),
    XLI_OP(cmp, zpg), XLI_OP(cmp, vec),
    XLI_OP(cmp, abx), XLI_OP(cmp, aby),
    XLI_OP(cmp, zpx), XLI_OP(cmp, zpy),
    XLI_OP(jmp, rel), XLI_OP(jmp, abs),
    XLI_OP(jmp, zpg), XLI_OP(jmp, vec),
    XLI_OP(jmp, abx), XLI_OP(jmp, aby),
    XLI_OP(jmp, zpx), XLI_OP(jmp, zpy),
    /* $60 */
    XLI_OP(stx, abs), XLI_OP(stx, aby),
    XLI_OP(stx, zpg), XLI_OP(stx, zpy),
    XLI_OP(stx, vec), XLI_OP(stx, zyv),
    XLI_OP(lda, zvx), XLI_OP(lda, zyv),
    XLI_OP(sty, abs), XLI_OP(sty, abx),
    XLI_OP(sty, zpg), XLI_OP(sty, zpx),
    XLI_OP(sty, vec), XLI_OP(sty, zvx),
    XLI_OP(sta, zvx), XLI_OP(sta, zyv),
    /* $70 */
    XLI_OP(nta, nam), XLI_OP(cal, abs),
    XLI_OP(cal, zpg), XLI_OP(cal, vec),
    XLI_OP(cal, abx), XLI_OP(cal, aby),
    XLI_OP(cal, zpx), XLI_OP(cal, zpy),
    XLI_OP(cal, zvx), XLI_OP(cal, zyv),
    XLI_OP(jmp, zvx), XLI_OP(jmp, zyv),
    XLI_OP(cmp, zvx), XLI_OP(cmp, zyv),
    XLI_OP(sla, nam), XLI_OP(sra, nam),
    /* $80 */
    XLI_OP(inc, abs), XLI_OP(inc, abx),
    XLI_OP(inc, aby), XLI_OP(inc, zpg),
    XLI_OP(inc, zpx), XLI_OP(inc, zpy),
    XLI_OP(inc, vec), XLI_OP(inc, zvx),
    XLI_OP(inc, zyv), XLI_OP(cpx, imm),
    XLI_OP(cpx, abs), XLI_OP(cpx, aby),
    XLI_OP(cpx, zpg), XLI_OP(cpx, zpy),
    XLI_OP(cpx, vec), XLI_OP(cpx, zyv),
    /* $90 */
    XLI_OP(dec, abs), XLI_OP(dec, abx),
    XLI_OP(dec, aby), XLI_OP(dec, zpg),
    XLI_OP(dec, zpx), XLI_OP(dec, zpy),
    XLI_OP(dec, vec), XLI_OP(dec, zvx),
    XLI_OP(dec, zyv), XLI_OP(cpy, imm),
    XLI_OP(cpy, abs), XLI_OP(cpy, abx),
    XLI_OP(cpy, zpg), XLI_OP(cpy, zpx),
    XLI_OP(cpy, vec), XLI_OP(cpy, zvx),
    /* $A0 */
    XLI_OP(bit, imm), XLI_OP(bit, abs),
    XLI_OP(bit, zpg), XLI_OP(bit, vec),
    XLI_OP(bit, abx), XLI_OP(bit, aby),
    XLI_OP(bit, zpx), XLI_OP(bit, zpy),
    XLI_OP(and, imm), XLI_OP(and, abs),
    XLI_OP(and, zpg), XLI_OP(and, vec),
    XLI_OP(and, abx), XLI_OP(and, aby),
    XLI_OP(and, zpx), XLI_OP(and, zpy),
    /* $B0 */
    XLI_OP(bor, imm), XLI_OP(bor, abs),
    XLI_OP(bor, zpg), XLI_OP(bor, vec),
    XLI_OP(bor, abx), XLI_OP(bor, aby),
    XLI_OP(bor, zpx), XLI_OP(bor, zpy),
    XLI_OP(xor, imm), XLI_OP(xor, abs),
    XLI_OP(xor, zpg), XLI_OP(xor, vec),
    XLI_OP(xor, abx), XLI_OP(xor, aby),
    XLI_OP(xor, zpx), XLI_OP(xor, zpy),
    /* $C0 */
    XLI_OP(adc, imm), XLI_OP(adc, abs),
    XLI_OP(adc, zpg), XLI_OP(adc, vec),
    XLI_OP(adc, abx), XLI_OP(adc, aby),
    XLI_OP(adc, zpx), XLI_OP(adc, zpy),
    XLI_OP(sbc, imm), XLI_OP(sbc, abs),
    XLI_OP(sbc, zpg), XLI_OP(sbc, vec),
    XLI_OP(sbc, abx), XLI_OP(sbc, aby),
    XLI_OP(sbc, zpx), XLI_OP(sbc, zpy),
    /* $D0 */
    XLI_OP(add, imm), XLI_OP(add, abs),
    XLI_OP(add, zpg), XLI_OP(add, vec),
    XLI_OP(add, abx), XLI_OP(add, aby),
    XLI_OP(add, zpx), XLI_OP(add, zpy),
    XLI_OP(sub, imm), XLI_OP(sub, abs),
    XLI_OP(sub, zpg), XLI_OP(sub, vec),
    XLI_OP(sub, abx), XLI_OP(sub, aby),
    XLI_OP(sub, zpx), XLI_OP(sub, zpy),
    /* $E0 */
    XLI_OP(bit, zvx), XLI_OP(bit, zyv),
    XLI_OP(and, zvx), XLI_OP(and, zyv),
    XLI_OP(bor, zvx), XLI_OP(bor, zyv),
    XLI_OP(xor, zvx), XLI_OP(xor, zyv),
    XLI_OP(adc, zvx), XLI_OP(adc, zyv),
    XLI_OP(sbc, zvx), XLI_OP(sbc, zyv),
    XLI_OP(add, zvx), XLI_OP(add, zyv),
    XLI_OP(sub, zvx), XLI_OP(sub, zyv),
    /* $F0 */
    XLI_OP(not, zpg), XLI_OP(not, zpx),
    XLI_OP(not, abs), XLI_OP(not, abx),
    XLI_OP(shl, zpg), XLI_OP(shl, zpx),
    XLI_OP(shl, abs), XLI_OP(shl, abx),
    XLI_OP(shr, zpg), XLI_OP(shr, zpx),
    XLI_OP(shr, abs), XLI_OP(shr, abx),
    XLI_OP(inv, nam), XLI_OP(inv, nam),
    XLI_OP(inv, nam), XLI_OP(inv, nam),
  };
  return &table[value];
}

#endif /* EXTENDED_LEMON_IMPLEMENTED */
#endif /* !EXTENDED_LEMON_C */

/************************************************************/
/* LICENSE                                                  */
/************************************************************/

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

