/*
extended_lemon_extra.h - Additional XL data and functions.
See LICENSE information in the end of the file.
*/

/************************************************************/
/* HEADER CODE                                              */
/************************************************************/

#ifndef XL_EXTRA_H
#define XL_EXTRA_H

#ifdef __cplusplus
extern "C" {
#endif

/************************************************************/
/* DEFINES                                                  */
/************************************************************/

/*
The number of instruction combinations.
*/
#define XL_NUM_COMBOS 256

/*
XLAS keywords X-macro.
*/
#define XL_KEYWORDS_XM \
  X(inv) \
  X(nop) \
  X(brk) \
  X(rti) \
  X(ret) \
  X(for) \
  X(fnd) \
  X(clc) \
  X(app) \
  X(amm) \
  X(spp) \
  X(smm) \
  X(xpp) \
  X(xmm) \
  X(ypp) \
  X(ymm) \
  X(inc) \
  X(dec) \
  X(jfb) \
  X(jfc) \
  X(jfd) \
  X(jfn) \
  X(jfr) \
  X(jfu) \
  X(jfv) \
  X(jfz) \
  X(jtb) \
  X(jtc) \
  X(jtd) \
  X(jtn) \
  X(jtr) \
  X(jtu) \
  X(jtv) \
  X(jtz) \
  X(jmp) \
  X(cal) \
  X(lda) \
  X(ldx) \
  X(ldy) \
  X(sta) \
  X(stx) \
  X(sty) \
  X(pla) \
  X(plf) \
  X(plx) \
  X(ply) \
  X(pha) \
  X(phf) \
  X(phx) \
  X(phy) \
  X(taf) \
  X(tas) \
  X(tax) \
  X(tay) \
  X(tfa) \
  X(tsa) \
  X(txa) \
  X(tya) \
  X(cmp) \
  X(cpx) \
  X(cpy) \
  X(sbc) \
  X(sub) \
  X(adc) \
  X(add) \
  X(bor) \
  X(xor) \
  X(and) \
  X(bit) \
  X(not) \
  X(nta) \
  X(shl) \
  X(shr) \
  X(sla) \
  X(sra) \
  X(zra) \
  X(zrx) \
  X(zry) \
  X(let) \
  X(rb) \
  X(db) \
  X(dw) \
  X(include) \
  X(incbin) \
  X(x) \
  X(y)

/*
XL addressing modes X-macro.
*/
#define XL_ADDRMODES_XM \
  X(nam) \
  X(imm) \
  X(abs) \
  X(abx) \
  X(aby) \
  X(rel) \
  X(zpg) \
  X(zpx) \
  X(zpy) \
  X(vec) \
  X(zvx) \
  X(zyv)

/*
XL interrupts X-macro.
*/
#define XL_INTERRUPTS_XM \
  X(reserved) \
  X(break) \
  X(react) \
  X(reset)

/************************************************************/
/* TYPES                                                    */
/************************************************************/

/*
XLAS keyword type.
*/
enum {
#define X(name) T##name,
XL_KEYWORDS_XM
#undef X
  Tcount
};

/*
Addressing mode type.
*/
enum {
#define X(name) M##name,
XL_ADDRMODES_XM
#undef X
  Mcount
};

/*
Interrupt type.
*/
enum {
#define X(name) I##name,
XL_INTERRUPTS_XM
#undef X
  Icount
};

/*
Combination of an instruction keyword and an addressing mode.
*/
typedef struct XL_Combo {
  unsigned char inst;
  unsigned char amode;
} XL_Combo;

/************************************************************/
/* VARIABLES                                                */
/************************************************************/

/*
All 256 combinations of instruction keywords and addressing
modes, where the index of a combo is the instruction byte.
*/
extern XL_Combo XL_combos[];

/*
Human-readable names of keywords.
*/
extern const char *XL_keywords[];

/*
Human-readable names of addresing modes.
*/
extern const char *XL_addrmodes[];

/*
Human-readable names of interrupts.
*/
extern const char *XL_interrupts[];

/*
XLAS look of addressing modes.
*/
extern const char *XL_msignatures[];

/*
Instruction sizes mapped to addressing modes.
*/
extern int XL_modesizes[];

#ifdef __cplusplus
};
#endif

#endif /* XL_EXTRA_H */

/************************************************************/
/* SOURCE CODE                                              */
/************************************************************/

#ifdef XL_EXTRA_C
#ifndef XL_EXTRA_IMPLEMENTED
#define XL_EXTRA_IMPLEMENTED

/************************************************************/
XL_Combo XL_combos[XL_NUM_COMBOS] = {
  {Tinv, Mnam}, {Tbrk, Mnam}, {Trti, Mnam}, {Tret, Mnam},
  {Tfor, Mimm}, {Tfnd, Mimm}, {Tclc, Mnam}, {Tnop, Mnam},
  {Tapp, Mnam}, {Tamm, Mnam}, {Tspp, Mnam}, {Tsmm, Mnam},
  {Txpp, Mnam}, {Txmm, Mnam}, {Typp, Mnam}, {Tymm, Mnam},
  {Tjfb, Mrel}, {Tjfc, Mrel}, {Tjfd, Mrel}, {Tjfn, Mrel},
  {Tjfr, Mrel}, {Tjfu, Mrel}, {Tjfv, Mrel}, {Tjfz, Mrel},
  {Tjtb, Mrel}, {Tjtc, Mrel}, {Tjtd, Mrel}, {Tjtn, Mrel},
  {Tjtr, Mrel}, {Tjtu, Mrel}, {Tjtv, Mrel}, {Tjtz, Mrel},
  {Tpha, Mnam}, {Tphf, Mnam}, {Tphx, Mnam}, {Tphy, Mnam},
  {Tpla, Mnam}, {Tplf, Mnam}, {Tplx, Mnam}, {Tply, Mnam},
  {Ttaf, Mnam}, {Ttas, Mnam}, {Ttax, Mnam}, {Ttay, Mnam},
  {Ttfa, Mnam}, {Ttsa, Mnam}, {Ttxa, Mnam}, {Ttya, Mnam},
  {Tlda, Mimm}, {Tlda, Mabs}, {Tlda, Mzpg}, {Tlda, Mvec},
  {Tlda, Mabx}, {Tlda, Maby}, {Tlda, Mzpx}, {Tlda, Mzpy},
  {Tzra, Mnam}, {Tsta, Mabs}, {Tsta, Mzpg}, {Tsta, Mvec},
  {Tsta, Mabx}, {Tsta, Maby}, {Tsta, Mzpx}, {Tsta, Mzpy},
  {Tzrx, Mnam}, {Tldx, Mimm}, {Tldx, Mabs}, {Tldx, Maby},
  {Tldx, Mzpg}, {Tldx, Mzpy}, {Tldx, Mvec}, {Tldx, Mzyv},
  {Tzry, Mnam}, {Tldy, Mimm}, {Tldy, Mabs}, {Tldy, Mabx},
  {Tldy, Mzpg}, {Tldy, Mzpx}, {Tldy, Mvec}, {Tldy, Mzvx},
  {Tcmp, Mimm}, {Tcmp, Mabs}, {Tcmp, Mzpg}, {Tcmp, Mvec},
  {Tcmp, Mabx}, {Tcmp, Maby}, {Tcmp, Mzpx}, {Tcmp, Mzpy},
  {Tjmp, Mrel}, {Tjmp, Mabs}, {Tjmp, Mzpg}, {Tjmp, Mvec},
  {Tjmp, Mabx}, {Tjmp, Maby}, {Tjmp, Mzpx}, {Tjmp, Mzpy},
  {Tstx, Mabs}, {Tstx, Maby}, {Tstx, Mzpg}, {Tstx, Mzpy},
  {Tstx, Mvec}, {Tstx, Mzyv}, {Tlda, Mzvx}, {Tlda, Mzyv},
  {Tsty, Mabs}, {Tsty, Mabx}, {Tsty, Mzpg}, {Tsty, Mzpx},
  {Tsty, Mvec}, {Tsty, Mzvx}, {Tsta, Mzvx}, {Tsta, Mzyv},
  {Tnta, Mnam}, {Tcal, Mabs}, {Tcal, Mzpg}, {Tcal, Mvec},
  {Tcal, Mabx}, {Tcal, Maby}, {Tcal, Mzpx}, {Tcal, Mzpy},
  {Tcal, Mzvx}, {Tcal, Mzyv}, {Tjmp, Mzvx}, {Tjmp, Mzyv},
  {Tcmp, Mzvx}, {Tcmp, Mzyv}, {Tsla, Mnam}, {Tsra, Mnam},
  {Tinc, Mabs}, {Tinc, Mabx}, {Tinc, Maby}, {Tinc, Mzpg},
  {Tinc, Mzpx}, {Tinc, Mzpy}, {Tinc, Mvec}, {Tinc, Mzvx},
  {Tinc, Mzyv}, {Tcpx, Mimm}, {Tcpx, Mabs}, {Tcpx, Maby},
  {Tcpx, Mzpg}, {Tcpx, Mzpy}, {Tcpx, Mvec}, {Tcpx, Mzyv},
  {Tdec, Mabs}, {Tdec, Mabx}, {Tdec, Maby}, {Tdec, Mzpg},
  {Tdec, Mzpx}, {Tdec, Mzpy}, {Tdec, Mvec}, {Tdec, Mzvx},
  {Tdec, Mzyv}, {Tcpy, Mimm}, {Tcpy, Mabs}, {Tcpy, Mabx},
  {Tcpy, Mzpg}, {Tcpy, Mzpx}, {Tcpy, Mvec}, {Tcpy, Mzvx},
  {Tbit, Mimm}, {Tbit, Mabs}, {Tbit, Mzpg}, {Tbit, Mvec},
  {Tbit, Mabx}, {Tbit, Maby}, {Tbit, Mzpx}, {Tbit, Mzpy},
  {Tand, Mimm}, {Tand, Mabs}, {Tand, Mzpg}, {Tand, Mvec},
  {Tand, Mabx}, {Tand, Maby}, {Tand, Mzpx}, {Tand, Mzpy},
  {Tbor, Mimm}, {Tbor, Mabs}, {Tbor, Mzpg}, {Tbor, Mvec},
  {Tbor, Mabx}, {Tbor, Maby}, {Tbor, Mzpx}, {Tbor, Mzpy},
  {Txor, Mimm}, {Txor, Mabs}, {Txor, Mzpg}, {Txor, Mvec},
  {Txor, Mabx}, {Txor, Maby}, {Txor, Mzpx}, {Txor, Mzpy},
  {Tadc, Mimm}, {Tadc, Mabs}, {Tadc, Mzpg}, {Tadc, Mvec},
  {Tadc, Mabx}, {Tadc, Maby}, {Tadc, Mzpx}, {Tadc, Mzpy},
  {Tsbc, Mimm}, {Tsbc, Mabs}, {Tsbc, Mzpg}, {Tsbc, Mvec},
  {Tsbc, Mabx}, {Tsbc, Maby}, {Tsbc, Mzpx}, {Tsbc, Mzpy},
  {Tadd, Mimm}, {Tadd, Mabs}, {Tadd, Mzpg}, {Tadd, Mvec},
  {Tadd, Mabx}, {Tadd, Maby}, {Tadd, Mzpx}, {Tadd, Mzpy},
  {Tsub, Mimm}, {Tsub, Mabs}, {Tsub, Mzpg}, {Tsub, Mvec},
  {Tsub, Mabx}, {Tsub, Maby}, {Tsub, Mzpx}, {Tsub, Mzpy},
  {Tbit, Mzvx}, {Tbit, Mzyv}, {Tand, Mzvx}, {Tand, Mzyv},
  {Tbor, Mzvx}, {Tbor, Mzyv}, {Txor, Mzvx}, {Txor, Mzyv},
  {Tadc, Mzvx}, {Tadc, Mzyv}, {Tsbc, Mzvx}, {Tsbc, Mzyv},
  {Tadd, Mzvx}, {Tadd, Mzyv}, {Tsub, Mzvx}, {Tsub, Mzyv},
  {Tnot, Mzpg}, {Tnot, Mzpx}, {Tnot, Mabs}, {Tnot, Mabx},
  {Tshl, Mzpg}, {Tshl, Mzpx}, {Tshl, Mabs}, {Tshl, Mabx},
  {Tshr, Mzpg}, {Tshr, Mzpx}, {Tshr, Mabs}, {Tshr, Mabx},
  {Tinv, Mnam}, {Tinv, Mnam}, {Tinv, Mnam}, {Tinv, Mnam}
};

/************************************************************/
const char *XL_keywords[Tcount + 1] = {
#define X(name) #name,
XL_KEYWORDS_XM
#undef X
  "unreachable"
};

/************************************************************/
const char *XL_addrmodes[Mcount + 1] = {
#define X(name) #name,
XL_ADDRMODES_XM
#undef X
  "unreachable"
};

/************************************************************/
const char *XL_interrupts[Icount + 1] = {
#define X(name) #name,
XL_INTERRUPTS_XM
#undef X
  "unreachable"
};

/************************************************************/
const char *XL_msignatures[Mcount] = {
  "", " #", " ", " x ", " y ", " ~", " ", " x ",
  " y ", " *", " x *", " y *"
};

/************************************************************/
int XL_modesizes[Mcount] = {
  1, 2, 3, 3, 3, 2, 2, 2, 2, 3, 2, 2
};

#endif /* XL_EXTRA_IMPLEMENTED */
#endif /* !XL_EXTRA_C */

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

