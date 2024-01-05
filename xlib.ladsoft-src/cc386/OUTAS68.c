/* 
CC386 C Compiler
Copyright 1994-2011 David Lindauer.

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

This program is derived from the cc68k complier by 
Matthew Brandt (mailto::mattb@walkingdog.net) 

You may contact the author of this derivative at:

mailto::camille@bluegrass.net
 */
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include "lists.h"
#include "expr.h"
#include "c.h"
#include "gen68.h"
#include "diag.h"

/*      variable initialization         */
extern int prm_cplusplus;
extern HASHREC **templateFuncs;
extern struct _templatelist *templateList;
extern int global_flag;
extern SYM *currentfunc;
extern HASHREC **gsyms;
extern OCODE *peep_insert,  *peep_head,  *peep_tail;
extern int prm_datarel, prm_pcrel;
extern long nextlabel;
extern FILE *outputFile;
extern int prm_asmfile;
extern int prm_lines;
extern int prm_cmangle;
extern int prm_optmult;
extern LIST *localfuncs,  *localdata;
extern LIST *libincludes;

struct slit *strtab;
enum e_gt gentype = nogen; /* Current DC type */
enum e_sg curseg = noseg; /* Current seg */
int outcol = 0; /* Curront col (roughly) */
int dataofs; /* Offset from last label */
SYM *datasp; /* Symbol of last named label */
static DATALINK *datahead,  *datatail; /* links for fixup gen */
static LIST *mpthunklist;

//ASMREG reglst[] = {
//   { 0 , 0 , 0 },
//};
/* List of opcodes
 * This list MUST be in the same order as the op_ enums 
 */
ASMNAME oplst[] = 
{
    {
        "?reserved", op_reserved, 0
    } , 
    {
        "?line#", op_reserved, 0
    }
    , 
    {
        "?seq@", op_reserved, 0
    }
    , 
    {
        "?slit", op_reserved, 0
    }
    , 
    {
        "?label", op_reserved, 0
    }
    , 
    {
        "?flabel", op_reserved, 0
    }
    , 
    {
        "dc", op_reserved, 0
    }
    , 
    {
        "?void", op_reserved, 0
    }
    , 
    {
        "dc", op_reserved, 0
    }
    , 
    {
        "dc", op_reserved, 0
    }
    , 
    {
        "?bstart", op_reserved, 0
    }
    , 
    {
        "?bend", op_reserved, 0
    }
    , 
    {
        "abcd", op_abcd, OPE_NEGBCD
    }
    , 
    {
        "add", op_add, OPE_MATH
    }
    , 
    {
        "adda", op_adda, OPE_AMATH
    }
    , 
    {
        "addi", op_addi, OPE_IMATH
    }
    , 
    {
        "addq", op_addq, OPE_QMATH
    }
    , 
    {
        "addx", op_addx, OPE_ADDX
    }
    , 
    {
        "and", op_and, OPE_LOG
    }
    , 
    {
        "andi", op_andi, OPE_ILOG
    }
    , 
    {
        "asl", op_asl, OPE_SHIFT
    }
    , 
    {
        "asr", op_asr, OPE_SHIFT
    }
    , 
    {
        "bra", op_bra, OPE_BRA
    }
    , 
    {
        "beq", op_beq, OPE_BRA
    }
    , 
    {
        "bne", op_bne, OPE_BRA
    }
    , 
    {
        "blt", op_blt, OPE_BRA
    }
    , 
    {
        "ble", op_ble, OPE_BRA
    }
    , 
    {
        "bgt", op_bgt, OPE_BRA
    }
    , 
    {
        "bge", op_bge, OPE_BRA
    }
    , 
    {
        "bhi", op_bhi, OPE_BRA
    }
    , 
    {
        "bhs", op_bhs, OPE_BRA
    }
    , 
    {
        "blo", op_blo, OPE_BRA
    }
    , 
    {
        "bls", op_bls, OPE_BRA
    }
    , 
    {
        "bsr", op_bsr, OPE_BRA
    }
    , 
    {
        "bcc", op_bcc, OPE_BRA
    }
    , 
    {
        "bcs", op_bcs, OPE_BRA
    }
    , 
    {
        "bmi", op_bmi, OPE_BRA
    }
    , 
    {
        "bpl", op_bpl, OPE_BRA
    }
    , 
    {
        "bvc", op_bvc, OPE_BRA
    }
    , 
    {
        "bvs", op_bvs, OPE_BRA
    }
    , 
    {
        "bchg", op_bchg, OPE_BIT
    }
    , 
    {
        "bclr", op_bclr, OPE_BIT
    }
    , 
    {
        "bfchg", op_bfchg, OPE_BF1
    }
    , 
    {
        "bfclr", op_bfclr, OPE_BF1
    }
    , 
    {
        "bfexts", op_bfexts, OPE_BF2
    }
    , 
    {
        "bfextu", op_bfextu, OPE_BF2
    }
    , 
    {
        "bfffo", op_bfffo, OPE_BF2
    }
    , 
    {
        "bfins", op_bfins, OPE_BF3
    }
    , 
    {
        "bfset", op_bfset, OPE_BF1
    }
    , 
    {
        "bftst", op_bftst, OPE_BF1
    }
    , 
    {
        "bkpt", op_bkpt, OPE_I
    }
    , 
    {
        "bset", op_bset, OPE_BIT
    }
    , 
    {
        "btst", op_btst, OPE_BIT
    }
    , 
    {
        "chk", op_chk, OPE_EAD
    }
    , 
    {
        "chk2", op_chk2, OPE_EAR
    }
    , 
    {
        "clr", op_clr, OPE_EA
    }
    , 
    {
        "cmp", op_cmp, OPE_CMP
    }
    , 
    {
        "cmpa", op_cmpa, OPE_EAA
    }
    , 
    {
        "cmpi", op_cmpi, OPE_IEA
    }
    , 
    {
        "cmpm", op_cmpm, OPE_POSBCD
    }
    , 
    {
        "cmp2", op_cmp2, OPE_EAR
    }
    , 
    {
        "dbeq", op_dbeq, OPE_DBR
    }
    , 
    {
        "dbne", op_dbne, OPE_DBR
    }
    , 
    {
        "dblt", op_dblt, OPE_DBR
    }
    , 
    {
        "dble", op_dble, OPE_DBR
    }
    , 
    {
        "dbgt", op_dbgt, OPE_DBR
    }
    , 
    {
        "dbge", op_dbge, OPE_DBR
    }
    , 
    {
        "dbhi", op_dbhi, OPE_DBR
    }
    , 
    {
        "dbhs", op_dbhs, OPE_DBR
    }
    , 
    {
        "dblo", op_dblo, OPE_DBR
    }
    , 
    {
        "dbls", op_dbls, OPE_DBR
    }
    , 
    {
        "dbsr", op_dbsr, OPE_DBR
    }
    , 
    {
        "dbcc", op_dbcc, OPE_DBR
    }
    , 
    {
        "dbcs", op_dbcs, OPE_DBR
    }
    , 
    {
        "dbmi", op_dbmi, OPE_DBR
    }
    , 
    {
        "dbpl", op_dbpl, OPE_DBR
    }
    , 
    {
        "dbvc", op_dbvc, OPE_DBR
    }
    , 
    {
        "dbvs", op_dbvs, OPE_DBR
    }
    , 
    {
        "dbt", op_dbt, OPE_DBR
    }
    , 
    {
        "dbf", op_dbf, OPE_DBR
    }
    , 
    {
        "dbra", op_dbra, OPE_DBR
    }
    , 
    {
        "divs", op_divs, OPE_DIV
    }
    , 
    {
        "divu", op_divu, OPE_DIV
    }
    , 
    {
        "divsl", op_divsl, OPE_DIVL
    }
    , 
    {
        "divul", op_divul, OPE_DIVL
    }
    , 
    {
        "eor", op_eor, OPE_EOR
    }
    , 
    {
        "eori", op_eori, OPE_ILOG
    }
    , 
    {
        "exg", op_exg, OPE_RRL
    }
    , 
    {
        "ext", op_ext, OPE_EXT
    }
    , 
    {
        "extb", op_extb, OPE_EXTB
    }
    , 
    {
        "illegal", op_illegal, 0
    }
    , 
    {
        "jmp", op_jmp, OPE_JEA
    }
    , 
    {
        "jsr", op_jsr, OPE_JEA
    }
    , 
    {
        "lea", op_lea, OPE_LEA
    }
    , 
    {
        "link", op_link, OPE_AI
    }
    , 
    {
        "lsl", op_lsl, OPE_SHIFT
    }
    , 
    {
        "lsr", op_lsr, OPE_SHIFT
    }
    , 
    {
        "move", op_move, OPE_MOVE
    }
    , 
    {
        "movea", op_movea, OPE_MOVEA
    }
    , 
    {
        "movec", op_movec, OPE_MOVEC
    }
    , 
    {
        "movem", op_movem, OPE_MOVEM
    }
    , 
    {
        "movep", op_movep, OPE_MOVEP
    }
    , 
    {
        "moveq", op_moveq, OPE_MOVEQ
    }
    , 
    {
        "moves", op_moves, OPE_MOVES
    }
    , 
    {
        "muls", op_muls, OPE_MUL
    }
    , 
    {
        "mulu", op_mulu, OPE_MUL
    }
    , 
    {
        "nbcd", op_nbcd, OPE_EAB
    }
    , 
    {
        "neg", op_neg, OPE_EA
    }
    , 
    {
        "negx", op_negx, OPE_EA
    }
    , 
    {
        "nop", op_nop, 0
    }
    , 
    {
        "not", op_not, OPE_EA
    }
    , 
    {
        "or", op_or, OPE_LOG
    }
    , 
    {
        "ori", op_ori, OPE_ILOG
    }
    , 
    {
        "pack", op_pack, OPE_PACK
    }
    , 
    {
        "pea", op_pea, OPE_JEA
    }
    , 
    {
        "rems", op_rems, OPE_DIV
    }
    , 
    {
        "remu", op_remu, OPE_DIV
    }
    , 
    {
        "reset", op_reset, 0
    }
    , 
    {
        "rol", op_rol, OPE_SHIFT
    }
    , 
    {
        "ror", op_ror, OPE_SHIFT
    }
    , 
    {
        "roxl", op_roxl, OPE_SHIFT
    }
    , 
    {
        "roxr", op_roxr, OPE_SHIFT
    }
    , 
    {
        "rtd", op_rtd, OPE_I
    }
    , 
    {
        "rte", op_rte, 0
    }
    , 
    {
        "rtr", op_rtr, 0
    }
    , 
    {
        "rts", op_rts, 0
    }
    , 
    {
        "sbcd", op_sbcd, OPE_NEGBCD
    }
    , 
    {
        "seq", op_seq, OPE_SET
    }
    , 
    {
        "sne", op_sne, OPE_SET
    }
    , 
    {
        "slt", op_slt, OPE_SET
    }
    , 
    {
        "sle", op_sle, OPE_SET
    }
    , 
    {
        "sgt", op_sgt, OPE_SET
    }
    , 
    {
        "sge", op_sge, OPE_SET
    }
    , 
    {
        "shi", op_shi, OPE_SET
    }
    , 
    {
        "shs", op_shs, OPE_SET
    }
    , 
    {
        "slo", op_slo, OPE_SET
    }
    , 
    {
        "sls", op_sls, OPE_SET
    }
    , 
    {
        "ssr", op_ssr, OPE_SET
    }
    , 
    {
        "scc", op_scc, OPE_SET
    }
    , 
    {
        "scs", op_scs, OPE_SET
    }
    , 
    {
        "smi", op_smi, OPE_SET
    }
    , 
    {
        "spl", op_spl, OPE_SET
    }
    , 
    {
        "svc", op_svc, OPE_SET
    }
    , 
    {
        "svs", op_svs, OPE_SET
    }
    , 
    {
        "st", op_st, OPE_SET
    }
    , 
    {
        "sf", op_sf, OPE_SET
    }
    , 
    {
        "sub", op_sub, OPE_MATH
    }
    , 
    {
        "stop", op_stop, 0
    }
    , 
    {
        "suba", op_suba, OPE_AMATH
    }
    , 
    {
        "subi", op_subi, OPE_IMATH
    }
    , 
    {
        "subq", op_subq, OPE_QMATH
    }
    , 
    {
        "subx", op_subx, OPE_ADDX
    }
    , 
    {
        "swap", op_swap, OPE_D
    }
    , 
    {
        "tas", op_tas, OPE_EAB
    }
    , 
    {
        "trap", op_trap, OPE_I
    }
    , 
    {
        "trapeq", op_trapeq, OPE_TRAPcc
    }
    , 
    {
        "trapne", op_trapne, OPE_TRAPcc
    }
    , 
    {
        "traplt", op_traplt, OPE_TRAPcc
    }
    , 
    {
        "traple", op_traple, OPE_TRAPcc
    }
    , 
    {
        "trapgt", op_trapgt, OPE_TRAPcc
    }
    , 
    {
        "trapge", op_trapge, OPE_TRAPcc
    }
    , 
    {
        "traphi", op_traphi, OPE_TRAPcc
    }
    , 
    {
        "traphs", op_traphs, OPE_TRAPcc
    }
    , 
    {
        "traplo", op_traplo, OPE_TRAPcc
    }
    , 
    {
        "trapls", op_trapls, OPE_TRAPcc
    }
    , 
    {
        "trapsr", op_trapsr, OPE_TRAPcc
    }
    , 
    {
        "trapcc", op_trapcc, OPE_TRAPcc
    }
    , 
    {
        "trapcs", op_trapcs, OPE_TRAPcc
    }
    , 
    {
        "trapmi", op_trapmi, OPE_TRAPcc
    }
    , 
    {
        "trappl", op_trappl, OPE_TRAPcc
    }
    , 
    {
        "trapvc", op_trapvc, OPE_TRAPcc
    }
    , 
    {
        "trapvs", op_trapvs, OPE_TRAPcc
    }
    , 
    {
        "trapt", op_trapt, OPE_TRAPcc
    }
    , 
    {
        "trapf", op_trapf, OPE_TRAPcc
    }
    , 
    {
        "trapv", op_trapv, 0
    }
    , 
    {
        "tst", op_tst, OPE_TEA
    }
    , 
    {
        "unlk", op_unlk, OPE_A
    }
    , 
    {
        "unpk", op_unpk, OPE_PACK
    }
    , 
    {
        "fabs", op_fabs, OPE_FMATH
    }
    , 
    {
        "facos", op_facos, OPE_FMATH
    }
    , 
    {
        "fadd", op_fadd, OPE_FMATHX
    }
    , 
    {
        "fasin", op_fasin, OPE_FMATH
    }
    , 
    {
        "fatan", op_fatan, OPE_FMATH
    }
    , 
    {
        "fatanh", op_fatanh, OPE_FMATH
    }
    , 
    {
        "fbeq", op_fbeq, OPE_BRA
    }
    , 
    {
        "fbne", op_fbne, OPE_BRA
    }
    , 
    {
        "fbgt", op_fbgt, OPE_BRA
    }
    , 
    {
        "fbngt", op_fbngt, OPE_BRA
    }
    , 
    {
        "fbge", op_fbge, OPE_BRA
    }
    , 
    {
        "fbnge", op_fbnge, OPE_BRA
    }
    , 
    {
        "fblt", op_fblt, OPE_BRA
    }
    , 
    {
        "fbnlt", op_fbnlt, OPE_BRA
    }
    , 
    {
        "fble", op_fble, OPE_BRA
    }
    , 
    {
        "fbnle", op_fbnle, OPE_BRA
    }
    , 
    {
        "fbgl", op_fbgl, OPE_BRA
    }
    , 
    {
        "fbngl", op_fbngl, OPE_BRA
    }
    , 
    {
        "fbgle", op_fbgle, OPE_BRA
    }
    , 
    {
        "fbngle", op_fbngle, OPE_BRA
    }
    , 
    {
        "fbogt", op_fbogt, OPE_BRA
    }
    , 
    {
        "fbule", op_fbule, OPE_BRA
    }
    , 
    {
        "fboge", op_fboge, OPE_BRA
    }
    , 
    {
        "fbult", op_fbult, OPE_BRA
    }
    , 
    {
        "fbolt", op_fbolt, OPE_BRA
    }
    , 
    {
        "fbuge", op_fbuge, OPE_BRA
    }
    , 
    {
        "fbole", op_fbole, OPE_BRA
    }
    , 
    {
        "fbugt", op_fbugt, OPE_BRA
    }
    , 
    {
        "fbogl", op_fbogl, OPE_BRA
    }
    , 
    {
        "fbueq", op_fbueq, OPE_BRA
    }
    , 
    {
        "fbor", op_fbor, OPE_BRA
    }
    , 
    {
        "fbun", op_fbun, OPE_BRA
    }
    , 
    {
        "fbt", op_fbt, OPE_BRA
    }
    , 
    {
        "fbf", op_fbf, OPE_BRA
    }
    , 
    {
        "fbst", op_fbst, OPE_BRA
    }
    , 
    {
        "fbsf", op_fbsf, OPE_BRA
    }
    , 
    {
        "fbseq", op_fbseq, OPE_BRA
    }
    , 
    {
        "fbsne", op_fbsne, OPE_BRA
    }
    , 
    {
        "fcmp", op_fcmp, OPE_FMATH
    }
    , 
    {
        "fcos", op_fcos, OPE_FMATH
    }
    , 
    {
        "fcosh", op_fcosh, OPE_FMATH
    }
    , 
    {
        "fdbeq", op_fdbeq, OPE_DBR
    }
    , 
    {
        "fdbne", op_fdbne, OPE_DBR
    }
    , 
    {
        "fdbgt", op_fdbgt, OPE_DBR
    }
    , 
    {
        "fdbngt", op_fdbngt, OPE_DBR
    }
    , 
    {
        "fdbge", op_fdbge, OPE_DBR
    }
    , 
    {
        "fdbnge", op_fdbnge, OPE_DBR
    }
    , 
    {
        "fdblt", op_fdblt, OPE_DBR
    }
    , 
    {
        "fdbnlt", op_fdbnlt, OPE_DBR
    }
    , 
    {
        "fdble", op_fdble, OPE_DBR
    }
    , 
    {
        "fdbnle", op_fdbnle, OPE_DBR
    }
    , 
    {
        "fdbgl", op_fdbgl, OPE_DBR
    }
    , 
    {
        "fdbngl", op_fdbngl, OPE_DBR
    }
    , 
    {
        "fdbgle", op_fdbgle, OPE_DBR
    }
    , 
    {
        "fdbngle", op_fdbngle, OPE_DBR
    }
    , 
    {
        "fdbogt", op_fdbogt, OPE_DBR
    }
    , 
    {
        "fdbule", op_fdbule, OPE_DBR
    }
    , 
    {
        "fdboge", op_fdboge, OPE_DBR
    }
    , 
    {
        "fdbult", op_fdbult, OPE_DBR
    }
    , 
    {
        "fdbolt", op_fdbolt, OPE_DBR
    }
    , 
    {
        "fdbuge", op_fdbuge, OPE_DBR
    }
    , 
    {
        "fdbole", op_fdbole, OPE_DBR
    }
    , 
    {
        "fdbugt", op_fdbugt, OPE_DBR
    }
    , 
    {
        "fdbogl", op_fdbogl, OPE_DBR
    }
    , 
    {
        "fdbueq", op_fdbueq, OPE_DBR
    }
    , 
    {
        "fdbor", op_fdbor, OPE_DBR
    }
    , 
    {
        "fdbun", op_fdbun, OPE_DBR
    }
    , 
    {
        "fdbt", op_fdbt, OPE_DBR
    }
    , 
    {
        "fdbf", op_fdbf, OPE_DBR
    }
    , 
    {
        "fdbst", op_fdbst, OPE_DBR
    }
    , 
    {
        "fdbsf", op_fdbsf, OPE_DBR
    }
    , 
    {
        "fdbseq", op_fdbseq, OPE_DBR
    }
    , 
    {
        "fdbsne", op_fdbsne, OPE_DBR
    }
    , 
    {
        "fdiv", op_fdiv, OPE_FMATHX
    }
    , 
    {
        "fetox", op_fetox, OPE_FMATH
    }
    , 
    {
        "fetoxm1", op_fetoxm1, OPE_FMATH
    }
    , 
    {
        "fgetexp", op_fgetexp, OPE_FMATH
    }
    , 
    {
        "fgetman", op_fgetman, OPE_FMATH
    }
    , 
    {
        "fint", op_fint, OPE_FMATH
    }
    , 
    {
        "fintrz", op_fintrz, OPE_FMATH
    }
    , 
    {
        "flog10", op_flog10, OPE_FMATH
    }
    , 
    {
        "flog2", op_flog2, OPE_FMATH
    }
    , 
    {
        "flogn", op_flogn, OPE_FMATH
    }
    , 
    {
        "flognp1", op_flognp1, OPE_FMATH
    }
    , 
    {
        "fmod", op_fmod, OPE_FMATH
    }
    , 
    {
        "fmove", op_fmove, OPE_FMOVE
    }
    , 
    {
        "fmovecr", op_fmovecr, OPE_FMOVECR
    }
    , 
    {
        "fmovem", op_fmovem, OPE_FMOVEM
    }
    , 
    {
        "fmul", op_fmul, OPE_FMATHX
    }
    , 
    {
        "fneg", op_fneg, OPE_FMATH
    }
    , 
    {
        "fnop", op_fnop, 0
    }
    , 
    {
        "frem", op_frem, OPE_FMATHX
    }
    , 
    {
        "frestore", op_frestore, OPE_FRESTORE
    }
    , 
    {
        "fsave", op_fsave, OPE_FSAVE
    }
    , 
    {
        "fscale", op_fscale, OPE_FMATHX
    }
    , 
    {
        "fseq", op_fseq, OPE_SET
    }
    , 
    {
        "fsne", op_fsne, OPE_SET
    }
    , 
    {
        "fsgt", op_fsgt, OPE_SET
    }
    , 
    {
        "fsngt", op_fsngt, OPE_SET
    }
    , 
    {
        "fsge", op_fsge, OPE_SET
    }
    , 
    {
        "fsnge", op_fsnge, OPE_SET
    }
    , 
    {
        "fslt", op_fslt, OPE_SET
    }
    , 
    {
        "fsnlt", op_fsnlt, OPE_SET
    }
    , 
    {
        "fsle", op_fsle, OPE_SET
    }
    , 
    {
        "fsnle", op_fsnle, OPE_SET
    }
    , 
    {
        "fsgl", op_fsgl, OPE_SET
    }
    , 
    {
        "fsngl", op_fsngl, OPE_SET
    }
    , 
    {
        "fsgle", op_fsgle, OPE_SET
    }
    , 
    {
        "fsngle", op_fsngle, OPE_SET
    }
    , 
    {
        "fsogt", op_fsogt, OPE_SET
    }
    , 
    {
        "fsule", op_fsule, OPE_SET
    }
    , 
    {
        "fsoge", op_fsoge, OPE_SET
    }
    , 
    {
        "fsult", op_fsult, OPE_SET
    }
    , 
    {
        "fsolt", op_fsolt, OPE_SET
    }
    , 
    {
        "fsuge", op_fsuge, OPE_SET
    }
    , 
    {
        "fsole", op_fsole, OPE_SET
    }
    , 
    {
        "fsugt", op_fsugt, OPE_SET
    }
    , 
    {
        "fsogl", op_fsogl, OPE_SET
    }
    , 
    {
        "fsueq", op_fsueq, OPE_SET
    }
    , 
    {
        "fsor", op_fsor, OPE_SET
    }
    , 
    {
        "fsun", op_fsun, OPE_SET
    }
    , 
    {
        "fst", op_fst, OPE_SET
    }
    , 
    {
        "fsf", op_fsf, OPE_SET
    }
    , 
    {
        "fsst", op_fsst, OPE_SET
    }
    , 
    {
        "fssf", op_fssf, OPE_SET
    }
    , 
    {
        "fsseq", op_fsseq, OPE_SET
    }
    , 
    {
        "fssne", op_fssne, OPE_SET
    }
    , 
    {
        "fsgldiv", op_fsgldiv, OPE_FMATHX
    }
    , 
    {
        "fsglmul", op_fsglmul, OPE_FMATHX
    }
    , 
    {
        "fsin", op_fsin, OPE_FMATH
    }
    , 
    {
        "fsincos", op_fsincos, OPE_FSINCOS
    }
    , 
    {
        "fsinh", op_fsinh, OPE_FMATH
    }
    , 
    {
        "fsqrt", op_fsqrt, OPE_FMATH
    }
    , 
    {
        "fsub", op_fsub, OPE_FMATHX
    }
    , 
    {
        "ftan", op_ftan, OPE_FMATH
    }
    , 
    {
        "ftanh", op_ftanh, OPE_FMATH
    }
    , 
    {
        "ftentox", op_ftentox, OPE_FMATH
    }
    , 
    {
        "ftrapeq", op_ftrapeq, OPE_TRAPcc
    }
    , 
    {
        "ftrapne", op_ftrapne, OPE_TRAPcc
    }
    , 
    {
        "ftrapgt", op_ftrapgt, OPE_TRAPcc
    }
    , 
    {
        "ftrapngt", op_ftrapngt, OPE_TRAPcc
    }
    , 
    {
        "ftrapge", op_ftrapge, OPE_TRAPcc
    }
    , 
    {
        "ftrapnge", op_ftrapnge, OPE_TRAPcc
    }
    , 
    {
        "ftraplt", op_ftraplt, OPE_TRAPcc
    }
    , 
    {
        "ftrapnlt", op_ftrapnlt, OPE_TRAPcc
    }
    , 
    {
        "ftraple", op_ftraple, OPE_TRAPcc
    }
    , 
    {
        "ftrapnle", op_ftrapnle, OPE_TRAPcc
    }
    , 
    {
        "ftrapgl", op_ftrapgl, OPE_TRAPcc
    }
    , 
    {
        "ftrapngl", op_ftrapngl, OPE_TRAPcc
    }
    , 
    {
        "ftrapgle", op_ftrapgle, OPE_TRAPcc
    }
    , 
    {
        "ftrapngle", op_ftrapngle, OPE_TRAPcc
    }
    , 
    {
        "ftrapogt", op_ftrapogt, OPE_TRAPcc
    }
    , 
    {
        "ftrapule", op_ftrapule, OPE_TRAPcc
    }
    , 
    {
        "ftrapoge", op_ftrapoge, OPE_TRAPcc
    }
    , 
    {
        "ftrapult", op_ftrapult, OPE_TRAPcc
    }
    , 
    {
        "ftrapolt", op_ftrapolt, OPE_TRAPcc
    }
    , 
    {
        "ftrapuge", op_ftrapuge, OPE_TRAPcc
    }
    , 
    {
        "ftrapole", op_ftrapole, OPE_TRAPcc
    }
    , 
    {
        "ftrapugt", op_ftrapugt, OPE_TRAPcc
    }
    , 
    {
        "ftrapogl", op_ftrapogl, OPE_TRAPcc
    }
    , 
    {
        "ftrapueq", op_ftrapueq, OPE_TRAPcc
    }
    , 
    {
        "ftrapor", op_ftrapor, OPE_TRAPcc
    }
    , 
    {
        "ftrapun", op_ftrapun, OPE_TRAPcc
    }
    , 
    {
        "ftrapt", op_ftrapt, OPE_TRAPcc
    }
    , 
    {
        "ftrapf", op_ftrapf, OPE_TRAPcc
    }
    , 
    {
        "ftrapst", op_ftrapst, OPE_TRAPcc
    }
    , 
    {
        "ftrapsf", op_ftrapsf, OPE_TRAPcc
    }
    , 
    {
        "ftrapseq", op_ftrapseq, OPE_TRAPcc
    }
    , 
    {
        "ftrapsne", op_ftrapsne, OPE_TRAPcc
    }
    , 
    {
        "ftst", op_ftst, OPE_FONE
    }
    , 
    {
        "ftwotox", op_ftwotox, OPE_FMATH
    }
    , 
    {
        0, 0, 0
    }
    , 
};
void dump_muldivval(void)
{}
void dump_browsedata(unsigned char *data, int len)
{
    if (!prm_asmfile)
        outcode_dump_browsedata(data, len);
}

/* Init module */
void outcodeini(void)
{
    strtab = 0;
    gentype = nogen;
    curseg = noseg;
    outcol = 0;
    datahead = datatail = 0;
    mpthunklist = 0;
}

/*
 * Register a fixup 
 */
void datalink(int flag)
{
    DATALINK *p;
    if (!prm_datarel)
        return ;
    global_flag++; /* Global tab */
    p = xalloc(sizeof(DATALINK));
    p->sp = datasp;
    p->type = flag;
    p->offset = dataofs;
    p->next = 0;
    if (datahead)
    {
        datatail->next = p;
        datatail = datatail->next;
    }
    else
        datahead = datatail = p;
    global_flag--;
}

//-------------------------------------------------------------------------

void nl(void)
/*
 * New line
 */
{
    if (prm_asmfile)
    {
        if (outcol > 0)
        {
            oputc('\n', outputFile);
            outcol = 0;
            gentype = nogen;
        }
    }
}

/* Put an opcode
 */
void outop(char *name)
{
	if (name[0] == '?')
		oputc(';', outputFile);
    oputc('\t', outputFile);
    while (*name)
        oputc(toupper(*name++), outputFile);
}

//-------------------------------------------------------------------------

void putop(int op)
{
    if (op > op_ftwotox)
        DIAG("illegal opcode.");
    else
        outop(oplst[op].word);
}

//-------------------------------------------------------------------------

char *dumpns(NAMESPACE *ns, char *buf, int *done)
{
    if (!ns)
        return buf;
    buf = dumpns(ns->next, buf, done);
    *done = 1;
    *buf++ = '@';
    strcpy(buf, ns->sp->name + prm_cmangle);
    buf += strlen(buf);
    return buf;
}

//-------------------------------------------------------------------------

void putsym(char *buf, SYM *sp, char *p)
{
    int done = 0;
    if (!p)
    {
        buf[0] = 0;
        return ;
    }
    if (sp->staticlabel)
        sprintf(buf, "L_%d", sp->value.i);
    else if (sp->alias)
    {
        strcpy(buf, sp->alias);
        goto reppercent;
    }
    else
    {
        char *q = buf;
        if (sp->pascaldefn)
        {
            if (prm_cmangle)
                p++;
            while (*p)
            #ifdef COLDFIRE
                    *q++ =  *p++;
            #else 
                *q++ = toupper(*p++);
            #endif 
            *q = 0;
        }
        else if (sp->isstdcall)
        {
            if (prm_cmangle)
                p++;
            strcpy(buf, p);
        }
        else
        {
            if (sp->mangled && !(sp->tp->type == bt_func || sp->tp->type ==
                bt_ifunc))
            {
                q = dumpns(sp->value.classdata.parentns, buf, &done);
                if (done)
                {
                    *q++ = '@';
                    strcpy(q, p + prm_cmangle);
                }
                else
                    strcpy(q, p);
            }
            else
                strcpy(buf, p);
            reppercent: if (prm_cplusplus)
            {
                // problematic for compiling templates via ASM and TASM
                q = buf;
                while (q = strchr(q, '#'))
                    *q = '%';
            }
        }
    }
}

//-------------------------------------------------------------------------

void putconst(ENODE *offset)
/*
 *      put a constant to the outputFile file.
 */
{
    char buf[512];
    SYM *sp;
    switch (offset->nodetype)
    {
        case en_autoreg:
        case en_autocon:
        case en_icon:
        case en_lcon:
        case en_iucon:
        case en_lucon:
        case en_ccon:
        case en_cucon:
        case en_llcon:
        case en_llucon:
        case en_absacon:
            oprintf(outputFile, "$%lX", offset->v.i);
            break;
        case en_fcon:
        case en_fimaginarycon:
        case en_rcon:
        case en_rimaginarycon:
        case en_lrcon:
        case en_lrimaginarycon:
		{
			char buf[256];
			FPFToString(buf, &offset->v.f);
            oprintf(outputFile, "%s", buf);
		}
            break;
        case en_fcomplexcon:
        case en_rcomplexcon:
        case en_lrcomplexcon:
            oprintf(outputFile, "%.16e,%.16e", offset->v.c.r, offset->v.c.i);
            break;
        case en_nalabcon:
            oprintf(outputFile, "L_%ld", offset->v.sp->value.i);
            break;
        case en_labcon:
            oprintf(outputFile, "L_%ld", offset->v.i);
            break;
        case en_napccon:
        case en_nacon:
            sp = offset->v.sp;
            putsym(buf, sp, sp->name);
            oprintf(outputFile, "%s", buf);
            break;
        case en_add:
        case en_addstruc:
            putconst(offset->v.p[0]);
            oprintf(outputFile, "+");
            putconst(offset->v.p[1]);
            break;
        case en_sub:
            putconst(offset->v.p[0]);
            oprintf(outputFile, "-");
            putconst(offset->v.p[1]);
            break;
        case en_uminus:
            oprintf(outputFile, "-");
            putconst(offset->v.p[0]);
            break;
        default:
            DIAG("illegal constant node.");
            break;
    }
}

//-------------------------------------------------------------------------

void putlen(int l)
/*
 *      append the length field to an instruction.
 */
{
    switch (l)
    {
        case 0:
            break; /* no length field */
        case 1:
            oprintf(outputFile, ".B");
            break;
        case 2:
            oprintf(outputFile, ".W");
            break;
        case 4:
            oprintf(outputFile, ".L");
            break;
        case 7:
            oprintf(outputFile, ".S");
            break;
        case 8:
            oprintf(outputFile, ".D");
            break;
        case 10:
            oprintf(outputFile, ".X");
            break;
        default:
            DIAG("illegal length field.");
            break;
    }
}

//-------------------------------------------------------------------------

void putsize(AMODE *ap)
{
    switch (ap->sz)
    {
        case 0:
        case 4:
            oprintf(outputFile, ".L");
            break;
        case 2:
            oprintf(outputFile, ".W");
            break;
        case 1:
            oprintf(outputFile, ".B");
            break;
    }
}

//-------------------------------------------------------------------------

void putamode(AMODE *ap)
/*
 *      outputFile a general addressing mode.
 */
{
    int scale, t;
    switch (ap->mode &0x1ff)
    {
        case am_ccr:
            oprintf(outputFile, "CCR");
            break;
        case am_fpcr:
            oprintf(outputFile, "FPCR");
            break;
        case am_fpsr:
            oprintf(outputFile, "FPSR");
            break;
        case am_usp:
            oprintf(outputFile, "USP");
            break;
        case am_sfc:
            oprintf(outputFile, "SFC");
            break;
        case am_dfc:
            oprintf(outputFile, "DFC");
            break;
        case am_vbr:
            oprintf(outputFile, "VBR");
            break;
        case am_cacr:
            oprintf(outputFile, "CACR");
            break;
        case am_caar:
            oprintf(outputFile, "CAAR");
            break;
        case am_msp:
            oprintf(outputFile, "MSP");
            break;
        case am_isp:
            oprintf(outputFile, "ISP");
            break;
        case am_sr:
            oprintf(outputFile, "SR");
            break;
        case am_bf:
            oprintf(outputFile, " {%d:%d}", ap->preg, ap->sreg);
            break;
        case am_bfreg:
            oprintf(outputFile, " {D%d:D%d}", ap->preg, ap->sreg);
            break;
        case am_divsl:
            oprintf(outputFile, "D%d:D%d", ap->preg, ap->sreg);
            break;
        case am_immed:
            oprintf(outputFile, "#");
        case am_direct:
            putconst(ap->offset);
            break;
        case am_adirect:
            oputc('(', outputFile);
            putconst(ap->offset);
            oputc(')', outputFile);
            putlen(ap->preg);
            break;
        case am_areg:
            oprintf(outputFile, "A%d", ap->preg);
            break;
        case am_dreg:
            oprintf(outputFile, "D%d", ap->preg);
            break;
        case am_freg:
            oprintf(outputFile, "FP%d", ap->preg);
            break;
        case am_ind:
            oprintf(outputFile, "(A%d)", ap->preg);
            break;
        case am_ainc:
            oprintf(outputFile, "(A%d)+", ap->preg);
            break;
        case am_adec:
            oprintf(outputFile, "-(A%d)", ap->preg);
            break;
        case am_indx:
            oprintf(outputFile, "(");
            putconst(ap->offset);
            oprintf(outputFile, ",A%d)", ap->preg);
            break;
        case am_pcindx:
            oprintf(outputFile, "(");
            putconst(ap->offset);
            oprintf(outputFile, ",PC)");
            break;
        case am_pcindxaddr:
            scale = 1;
            t = ap->scale;
            while (t--)
                scale <<= 1;
            oprintf(outputFile, "(");
            putconst(ap->offset);
            oprintf(outputFile, ",PC");
            oprintf(outputFile, ",A%d", ap->sreg);
            putsize(ap);
            if (scale != 1)
                oprintf(outputFile, "*%d", scale);
            oputc(')', outputFile);
            break;
        case am_pcindxdata:
            scale = 1;
            t = ap->scale;
            while (t--)
                scale <<= 1;
            oprintf(outputFile, "(");
            putconst(ap->offset);
            oprintf(outputFile, ",PC");
            oprintf(outputFile, ",D%d", ap->sreg);
            putsize(ap);
            if (scale != 1)
                oprintf(outputFile, "*%d", scale);
            oputc(')', outputFile);
            break;
        case am_baseindxdata:
            scale = 1;
            t = ap->scale;
            while (t--)
                scale <<= 1;
            oprintf(outputFile, "(");
            putconst(ap->offset);
            if (ap->preg !=  - 1)
                oprintf(outputFile, ",A%d", ap->preg);
            oprintf(outputFile, ",D%d", ap->sreg);
            putsize(ap);
            if (scale != 1)
                oprintf(outputFile, "*%d", scale);
            oputc(')', outputFile);
            break;
        case am_baseindxaddr:
            scale = 1;
            t = ap->scale;
            while (t--)
                scale <<= 1;
            oprintf(outputFile, "(");
            putconst(ap->offset);
            if (ap->preg !=  - 1)
                oprintf(outputFile, ",A%d", ap->preg);
            oprintf(outputFile, ",A%d", ap->sreg);
            putsize(ap);
            if (scale != 1)
                oprintf(outputFile, "*%d", scale);
            oputc(')', outputFile);
            break;
        case am_iimem:
            oprintf(outputFile, "([");
            putconst(ap->offset);
            oprintf(outputFile, "],");
            putconst(ap->offset2);
            oprintf(outputFile, ")");
            break;
        case am_iiprea:
            scale = 1;
            t = ap->scale;
            while (t--)
                scale <<= 1;
            oprintf(outputFile, "([");
            putconst(ap->offset);
            if (ap->preg !=  - 1)
                oprintf(outputFile, ",A%d", ap->preg);
            if (ap->sreg !=  - 1)
            {
                oprintf(outputFile, ",A%d", ap->sreg);
                putsize(ap);
                if (scale != 1)
                    oprintf(outputFile, "*%d", scale);
            }
            oprintf(outputFile, "],");
            putconst(ap->offset2);
            oprintf(outputFile, ")");
            break;
        case am_iipred:
            scale = 1;
            t = ap->scale;
            while (t--)
                scale <<= 1;
            oprintf(outputFile, "([");
            putconst(ap->offset);
            if (ap->preg !=  - 1)
                oprintf(outputFile, ",A%d", ap->preg);
            if (ap->sreg !=  - 1)
            {
                oprintf(outputFile, ",D%d", ap->sreg);
                putsize(ap);
                if (scale != 1)
                    oprintf(outputFile, "*%d", scale);
            }
            oprintf(outputFile, "],");
            putconst(ap->offset2);
            oprintf(outputFile, ")");
            break;
        case am_iiposta:
            scale = 1;
            t = ap->scale;
            while (t--)
                scale <<= 1;
            oprintf(outputFile, "([");
            putconst(ap->offset);
            if (ap->preg !=  - 1)
                oprintf(outputFile, ",A%d", ap->preg);
            oprintf(outputFile, "]");
            if (ap->sreg !=  - 1)
            {
                oprintf(outputFile, ",A%d", ap->sreg);
                putsize(ap);
                if (scale != 1)
                    oprintf(outputFile, "*%d", scale);
            }
            oprintf(outputFile, ",");
            putconst(ap->offset2);
            oprintf(outputFile, ")");
            break;
        case am_iipostd:
            scale = 1;
            t = ap->scale;
            while (t--)
                scale <<= 1;
            oprintf(outputFile, "([");
            putconst(ap->offset);
            if (ap->preg !=  - 1)
                oprintf(outputFile, ",A%d", ap->preg);
            oprintf(outputFile, "]");
            if (ap->sreg !=  - 1)
            {
                oprintf(outputFile, ",D%d", ap->sreg);
                putsize(ap);
                if (scale != 1)
                    oprintf(outputFile, "*%d", scale);
            }
            oprintf(outputFile, ",");
            putconst(ap->offset2);
            oprintf(outputFile, ")");
            break;
        case am_iiprepca:
            scale = 1;
            t = ap->scale;
            while (t--)
                scale <<= 1;
            oprintf(outputFile, "([");
            putconst(ap->offset);
            oprintf(outputFile, ",PC", ap->preg);
            if (ap->sreg !=  - 1)
            {
                oprintf(outputFile, ",A%d", ap->sreg);
                putsize(ap);
                if (scale != 1)
                    oprintf(outputFile, "*%d", scale);
            }
            oprintf(outputFile, "],");
            putconst(ap->offset2);
            oprintf(outputFile, ")");
            break;
        case am_iiprepcd:
            scale = 1;
            t = ap->scale;
            while (t--)
                scale <<= 1;
            oprintf(outputFile, "([");
            putconst(ap->offset);
            oprintf(outputFile, ",PC", ap->preg);
            if (ap->sreg !=  - 1)
            {
                oprintf(outputFile, ",D%d", ap->sreg);
                putsize(ap);
                if (scale != 1)
                    oprintf(outputFile, "*%d", scale);
            }
            oprintf(outputFile, "],");
            putconst(ap->offset2);
            oprintf(outputFile, ")");
            break;
        case am_iipostpca:
            scale = 1;
            t = ap->scale;
            while (t--)
                scale <<= 1;
            oprintf(outputFile, "([");
            putconst(ap->offset);
            oprintf(outputFile, ",PC]", ap->preg);
            if (ap->sreg !=  - 1)
            {
                oprintf(outputFile, ",A%d", ap->sreg);
                putsize(ap);
                if (scale != 1)
                    oprintf(outputFile, "*%d", scale);
            }
            oprintf(outputFile, ",");
            putconst(ap->offset2);
            oprintf(outputFile, ")");
            break;
        case am_iipostpcd:
            scale = 1;
            t = ap->scale;
            while (t--)
                scale <<= 1;
            oprintf(outputFile, "([");
            putconst(ap->offset);
            oprintf(outputFile, ",PC]", ap->preg);
            if (ap->sreg !=  - 1)
            {
                oprintf(outputFile, ",D%d", ap->sreg);
                putsize(ap);
                if (scale != 1)
                    oprintf(outputFile, "*%d", scale);
            }
            oprintf(outputFile, ",");
            putconst(ap->offset2);
            oprintf(outputFile, ")");
            break;
        case am_mask:
            put_mask((int)ap->offset);
            break;
        case am_fmask:
            put_fmask((int)ap->offset);
            break;
        default:
            DIAG("illegal address mode.");
            break;
    }
    if (ap->mode &am_bits)
        oprintf(outputFile, " { %d : %d }", ap->bf1, ap->bf2);
}

//-------------------------------------------------------------------------

void put_code(OCODE *cd)
/*
 *      outputFile a generic instruction.
 */
{
    int op = cd->opcode, len = cd->length;
    AMODE *aps = cd->oper1,  *apd = cd->oper2,  *ape = cd->oper3;
    if (op == op_void)
        return ;
    nl();
    if (!prm_asmfile)
    {
        ; /* oc_putop(cd); */
        return ;
    }
    if (op == op_line)
    {
        if (!prm_lines)
            return ;
        oprintf(outputFile, ";\n; Line %d:\t%s\n;\n", len, (char*)aps);
        return ;
    }
    if (op == op_slit)
        DIAG("slit in put_code");

    if (op == op_dcl)
    {
        putop(op);
        putlen(len);
        oprintf(outputFile, "\t");
        putamode(aps);
        if (prm_pcrel)
            oprintf(outputFile, "-*");
        oprintf(outputFile, "\n");
        return ;
    }
    else
    {
        putop(op);
        putlen(len);
    }
    if (aps != 0)
    {
        oprintf(outputFile, "\t");
        putamode(aps);
        if (apd != 0)
        {
            if (apd->mode != am_bf && apd->mode != am_bfreg)
                oprintf(outputFile, ",");
            putamode(apd);
            if (ape)
            {
                if (ape->mode != am_bf && ape->mode != am_bfreg)
                    oprintf(outputFile, ",");
                putamode(ape);
            }
        }
    }
    oprintf(outputFile, "\n");
}

//-------------------------------------------------------------------------

void put_fmask(int mask)
/*
 *      generate a register mask for floating restore and save.
 */
{
    unsigned put = FALSE, i, bit;
    bit = 1;
    for (i = 0; i < 8; i++)
    {
        if (bit &(unsigned)mask)
        {
            if (put)
                oputc('/', outputFile);
            put = TRUE;
            putreg(i + 16);
        }
        bit <<= 1;
    }
}

//-------------------------------------------------------------------------

void put_mask(int mask)
/*
 *      generate a register mask for integer restore and save.
 */
{
    unsigned put = FALSE, i, bit;
    bit = 1;
    for (i = 0; i < 16; i++)
    {
        if (bit &(unsigned)mask)
        {
            if (put)
                oputc('/', outputFile);
            put = TRUE;
            putreg(i);
        }
        bit <<= 1;
    }
}

//-------------------------------------------------------------------------

void putreg(int r)
/*
 *      generate a register name from a tempref number.
 */
{
    if (r < 8)
        oprintf(outputFile, "D%d", r);
    else if (r < 16)
        oprintf(outputFile, "A%d", r - 8);
    else
        oprintf(outputFile, "FP%d", r - 16);
}

//-------------------------------------------------------------------------

void gen_strlab(SYM *sp)
/*
 *      generate a named label.
 */
{
    char buf[512];
    putsym(buf, sp, sp->name);
    datasp = sp;
    if (prm_asmfile)
    {
        nl();
        oprintf(outputFile, "%s:\n", buf);
    }
    dataofs = 0;
}

//-------------------------------------------------------------------------

void put_label(int label)
/*
 *      outputFile a compiler generated label.
 */
{
    if (prm_asmfile)
    {
        nl();
        oprintf(outputFile, "L_%ld:\n", label);
    }
    else
        outcode_put_label(label);
}

//-------------------------------------------------------------------------

void put_staticlabel(long label)
{
    if (prm_asmfile)
    {
        nl();
        oprintf(outputFile, "L_%ld:\n", label);
    }
    else
        outcode_put_label(label);
}

//-------------------------------------------------------------------------

void genfloat(FPF *val)
/*
 * Output a float value
 */
{
    if (prm_asmfile)
    {
		char buf[512];
        nl();
        FPFToString(buf,val);
		if (!strcmp(buf,"inf") || !strcmp(buf, "nan")
			|| !strcmp(buf,"-inf") || !strcmp(buf, "-nan"))
		{
			unsigned char dta[4];
			int i;
			FPFToFloat(dta, val);
			oprintf(outputFile,"\tDC.B\t");
			for (i=3; i >=0; i--)
			{
				oprintf(outputFile, "0%02XH", dta[i]);
				if (i != 0)
					oprintf(outputFile,", ");
			}
		}
		else
		{
            oprintf(outputFile, "\tDC.S\t%s", buf);
            gentype = floatgen;
            outcol = 19;
        }
    }
    else
        outcode_genfloat(val);
	dataofs += 4;
}

//-------------------------------------------------------------------------

void gendouble(FPF *val)
/*
 * Output a double value
 */
{
    if (prm_asmfile)
    {
		char buf[512];
        nl();
        FPFToString(buf,val);
		if (!strcmp(buf,"inf") || !strcmp(buf, "nan")
			|| !strcmp(buf,"-inf") || !strcmp(buf, "-nan"))
		{
			unsigned char dta[8];
			int i;
			FPFToDouble(dta, val);
			oprintf(outputFile,"\tDC.B\t");
			for (i=7; i >=0; i--)
			{
				oprintf(outputFile, "0%02XH", dta[i]);
				if (i != 0)
					oprintf(outputFile,", ");
			}
		}
		else
		{
            oprintf(outputFile, "\tDC.D\t%s", buf);
            gentype = doublegen;
            outcol = 19;
        }
    }
    else
        outcode_gendouble(val);
    dataofs += 8;
}

//-------------------------------------------------------------------------

void genlongdouble(FPF *val)
/*
 * Output a double value
 */
{
    if (prm_asmfile)
    {
		char buf[512];
        nl();
        FPFToString(buf,val);
		if (!strcmp(buf,"inf") || !strcmp(buf, "nan")
			|| !strcmp(buf,"-inf") || !strcmp(buf, "-nan"))
		{
			unsigned char dta[12];
			int i;
			FPFToLongDouble(dta, val);
			oprintf(outputFile,"\tDC.B\t");
			for (i=11; i >=10; i--)
			{
				oprintf(outputFile, "0%02XH", dta[i-2]);
			}
			oprintf(outputFile, "000H, 000H, ");
			for (i=7; i >=0; i--)
			{
				oprintf(outputFile, "0%02XH", dta[i]);
				if (i != 0)
					oprintf(outputFile,", ");
			}
		}
		else
		{
            oprintf(outputFile, "\tDC.X\t%s", buf);
            gentype = longdoublegen;
            outcol = 19;
        }
    }
    else
        outcode_genlongdouble(val);
    dataofs += 12;
}

//-------------------------------------------------------------------------

int genstring(char *str, int uselong, int len)
/*
 * Generate a string literal
 */
{
    xstringseg();
    if (prm_asmfile)
    if (uselong)
    {
        len /= 2;
        while (len--)
        {
            genword(*((short*)str));
            str += 2;
        }
        return pstrlen(str) *2;
    }
    else
    {
        while (len--)
            genbyte(*str++);
        return strlen(str);
    }
    else
        outcode_genstring(str, len);
}

//-------------------------------------------------------------------------

void genbyte(long val)
/*
 * Output a byte value
 */
{
    if (prm_asmfile)
    if (gentype == bytegen && outcol < 60)
    {
        oprintf(outputFile, ",$%X", val &0x00ff);
        outcol += 4;
    }
    else
    {
        nl();
        oprintf(outputFile, "\tDC.B\t$%X", val &0x00ff);
        gentype = bytegen;
        outcol = 19;
    }
    else
        outcode_genbyte(val);
    dataofs += 1;
}
void genbool(int val)
{
	genbyte(val);
}

//-------------------------------------------------------------------------

void genword(long val)
/*
 * Output a word value
 */
{
    if (prm_asmfile)
    if (gentype == wordgen && outcol < 58)
    {
        oprintf(outputFile, ",$%X", val &0x0ffff);
        outcol += 6;
    }
    else
    {
        nl();
        oprintf(outputFile, "\tDC.W\t$%X", val &0x0ffff);
        gentype = wordgen;
        outcol = 21;
    }
    else
        outcode_genword(val);
    dataofs += 2;
}

//-------------------------------------------------------------------------

void genlong(long val)
/*
 * Output a long value
 */
{
    if (prm_asmfile)
    if (gentype == longgen && outcol < 56)
    {
        oprintf(outputFile, ",$%lX", val);
        outcol += 10;
    }
    else
    {
        nl();
        oprintf(outputFile, "\tDC.L\t$%lX", val);
        gentype = longgen;
        outcol = 25;
    }
    else
        outcode_genlong(val);
    dataofs += 4;
}
void genint(long val)
{
	genlong(val);
}
void genenum(long val)
{
	genlong(val);
}

//-------------------------------------------------------------------------

void genlonglong(LLONG_TYPE val)
/*
 * Output a long value
 */
{
    if (prm_asmfile)
    if (gentype == longgen && outcol < 56)
    {
        #if sizeof(ULLONG_TYPE) == 4
            oprintf(outputFile, "\tDC.L\t0,$%lX, $%lX", val < 0 ?  - 1: 0, val);
        #else 
            oprintf(outputFile, "\tDC.L\t0,$%lX, $%lX", val >> 32, val);
        #endif 
        outcol += 10;
    }
    else
    {
        nl();
        #if sizeof(ULLONG_TYPE) == 4
            oprintf(outputFile, "\tDC.L\t0,$%lX, $%lX", val < 0 ?  - 1: 0, val);
        #else 
            oprintf(outputFile, "\tDC.L\t0,$%lX, $%lX", val >> 32, val);
        #endif 
        gentype = longgen;
        outcol = 25;
    }
    else
        outcode_genlonglong(val);
    dataofs += 8;
}

/*
 * Generate a startup or rundown reference
 */
void gensrref(SYM *sp, int val)
{
    char buf[512];
    putsym(buf, sp, sp->name);
    if (prm_asmfile)
    {
        if (gentype == srrefgen && outcol < 56)
        {
            oprintf(outputFile, ",%s,%d", buf, val);
            outcol += strlen(sp->name) + 1;
        }
        else
        {
            nl();
            oprintf(outputFile, "\tDC.L\t%s,%d", buf, val);
            gentype = srrefgen;
            outcol = 25;
        }
    }
    else
        outcode_gensrref(sp, val);
}

//-------------------------------------------------------------------------

void genref(SYM *sp, int offset)
/*
 * Output a reference to the data area (also gens fixups )
 */
{
    char sign;
    char buf[512], name[512];
    putsym(name, sp, sp->name);
    if (offset < 0)
    {
        sign = '-';
        offset =  - offset;
    }
    else
        sign = '+';
    sprintf(buf, "%s%c%d", name, sign, offset);
    datalink(FALSE);
    if (prm_asmfile)
    {
        if (gentype == longgen && outcol < 55-strlen(name))
        {
            oprintf(outputFile, ",%s", buf);
            outcol += (11+strlen(name));
        }
        else
        {
            nl();
            oprintf(outputFile, "\tDC.L\t%s", buf);
            outcol = 26+strlen(name);
            gentype = longgen;
        }
    }
    else
        outcode_genref(sp, offset);
    dataofs += 4;
}

//-------------------------------------------------------------------------

void genpcref(SYM *sp, int offset)
/*
 * Output a reference to the code area (also gens fixups )
 */
{
    char sign;
    char buf[512], name[512];
    putsym(name, sp, sp->name);
    if (offset < 0)
    {
        sign = '-';
        offset =  - offset;
    }
    else
        sign = '+';
    sprintf(buf, "%s%c%d", name, sign, offset);
    datalink(TRUE);
    if (prm_asmfile)
    {
        if (gentype == longgen && outcol < 55-strlen(name))
        {
            oprintf(outputFile, ",%s", buf);
            outcol += (11+strlen(name));
        }
        else
        {
            nl();
            oprintf(outputFile, "\tDC.L\t%s", buf);
            outcol = 26+strlen(name);
            gentype = longgen;
        }
    }
    else
        outcode_genref(sp, offset);
    dataofs += 4;
}

//-------------------------------------------------------------------------

void genstorage(int nbytes)
/*
 * Output bytes of storage
 */
{
    if (prm_asmfile)
    {
        nl();
        oprintf(outputFile, "\tDS.B\t$%X\n", nbytes);
    }
    else
        outcode_genstorage(nbytes);
    dataofs += nbytes;
}

//-------------------------------------------------------------------------

void gen_labref(int n)
/*
 * Generate a reference to a label
 */
{
    if (prm_asmfile)
    if (gentype == longgen && outcol < 58)
    {
        oprintf(outputFile, ",L_%d", n);
        outcol += 6;
    }
    else
    {
        nl();
        oprintf(outputFile, "\tDC.L\tL_%d", n);
        outcol = 22;
        gentype = longgen;
    }
    else
        outcode_gen_labref(n);
    datalink(TRUE);
    dataofs += 4;
}

//-------------------------------------------------------------------------

void gen_labdifref(int n1, int n2)
/*
 * Generate a reference to a label
 */
{
    if (prm_asmfile)
    if (gentype == longgen && outcol < 58)
    {
        oprintf(outputFile, ",L_%d-L_%d", n1, n2);
        outcol += 6;
    }
    else
    {
        nl();
        oprintf(outputFile, "\tDC.L\tL_%d-L_%d", n1, n2);
        outcol = 22;
        gentype = longgen;
    }
    else
        outcode_gen_labdifref(n1, n2);
    datalink(TRUE);
    dataofs += 4;
}

//-------------------------------------------------------------------------

int stringlit(char *s, int uselong, int len)
/*
 *      make s a string literal and return it's label number.
 */
{
    struct slit *lp = strtab;
    if (uselong)
        len *= 2;
    if (prm_optmult)
    {
        while (lp)
        {
            if (len == lp->len && !memcmp(lp->str, s, len))
                return lp->label;
            lp = lp->next;
        } 
    }
    ++global_flag; /* always allocate from global space. */
    lp = xalloc(sizeof(struct slit));
    lp->label = nextlabel++;
    lp->str = xalloc(len);
    memcpy(lp->str, s, len);
    lp->len = len;
    lp->next = strtab;
    lp->type = uselong;
    strtab = lp;
    --global_flag;
    return lp->label;
}

//-------------------------------------------------------------------------

void dumplits(void)
{
    if (prm_asmfile)
    {
        while (strtab != 0)
        {
            xstringseg();
            nl();
            put_label(strtab->label);
            genstring(strtab->str, strtab->type, strtab->len - 1);
            if (strtab->type)
                genword(0);
            else
                genbyte(0);
			switch(strtab->len & 3)
			{
				case 1:
					genbyte(0);
				case 2:
					genbyte(0);
				case 3:
					genbyte(0);
					break;
			}
            strtab = strtab->next;
        } 
		nl();
    }
    else
        outcode_dumplits();
}

/*
 * Switch to cseg 
 */
void cseg(void)
{
    if (prm_asmfile)
    if (curseg != codeseg)
    {
        nl();
        oprintf(outputFile, "\tSECTION\tcode\n");
    }
    curseg = codeseg;

}

//-------------------------------------------------------------------------

void xconstseg(void)
{
    if (prm_asmfile)
    if (curseg != constseg)
    {
        nl();
        oprintf(outputFile, "\tSECTION\tconst\n");
    }
    curseg = constseg;
}

//-------------------------------------------------------------------------

void xstringseg(void)
{
    if (prm_asmfile)
    if (curseg != stringseg)
    {
        nl();
        oprintf(outputFile, "\tSECTION\tstring\n");
    }
    curseg = stringseg;
}

/*
 * Switch to dseg
 */
void dseg(void)
{
    if (prm_asmfile)
    if (curseg != dataseg)
    {
        nl();
        oprintf(outputFile, "\tSECTION\tdata\n");
    }
    curseg = dataseg;
}

/*
 * Switch to bssseg
 */
void bssseg(void)
{
    if (prm_asmfile)
    if (curseg != bssxseg)
    {
        nl();
        oprintf(outputFile, "\tSECTION\tbss\n");
    }
    curseg = bssxseg;
}

/*
 * Switch to startupseg
 */
void startupseg(void)
{
    if (prm_asmfile)
    if (curseg != startupxseg)
    {
        nl();
        oprintf(outputFile, "\tSECTION\tcstartup\n");
    }
    curseg = startupxseg;
}

/*
 * Switch to rundownseg
 */
void rundownseg(void)
{
    if (prm_asmfile)
    if (curseg != rundownxseg)
    {
        nl();
        oprintf(outputFile, "\tSECTION\tcrundown\n");
    }
    curseg = rundownxseg;
}


//-------------------------------------------------------------------------

void gen_virtual(SYM *sp, int data)
/*
 * Generate a virtual segment
 */
{
    char buf[512];
    if (!data && curseg != codeseg)
        cseg();
    curseg = virtseg;
    if (prm_asmfile)
    {
        nl();
        putsym(buf, sp, sp->name);
        oprintf(outputFile, "\tVIRTUAL\t @%s\n", buf);
        oprintf(outputFile, "%s:\n", buf);
    }
    else
        outcode_start_virtual_seg(sp, data);
}

//-------------------------------------------------------------------------

void gen_endvirtual(SYM *sp)
/*
 * Generate the end of a virtual segment
 */
{
    if (prm_asmfile)
    {
        //      nl();
        //      oprintf(outputFile,"\tENDVIRTUAL @%s\n",sp->name);
    }
    else
        outcode_end_virtual_seg(sp);
    curseg = noseg;
}

//-------------------------------------------------------------------------

void genlongref(DATALINK *p)
/*
 * Generate a reference reference for fixup tables
 */
{
    if (prm_asmfile)
    if (gentype == longgen && outcol < 56)
    {
        oprintf(outputFile, ",%s+$%X", p->sp->name, p->offset);
        outcol += 10;
    }
    else
    {
        nl();
        oprintf(outputFile, "\tDC.L\t%s+$%X", p->sp->name, p->offset);
        gentype = longgen;
        outcol = 25;
    }
    else
        outcode_genref(p->sp, p->offset);
}

/*
 * Assembly file header
 */
void asm_header(void){}
void globaldef(SYM *sp)
/*
 * Stick in a global definition
 */
{
    char name[512];
    putsym(name, sp, sp->name);
    if (prm_asmfile)
    {
        nl();
        oprintf(outputFile, "\tXDEF\t%s\n", name);
    }
}

//-------------------------------------------------------------------------

void output_alias(char *name, char *alias)
{
    oprintf(outputFile, "%s EQU\t<%s>\n", name, alias);
}

//-------------------------------------------------------------------------

static void dumphashtable(HASHREC **syms)
{
    SYM *sp;
    int i;
    char buf[512];
    for (i = 0; i < HASHTABLESIZE; i++)
    {
        if ((sp = (SYM*)syms[i]) != 0)
        {
            while (sp)
            {
                if (sp->storage_class == sc_externalfunc && sp->mainsym
                    ->extflag && !(sp->tp->cflags &DF_INTRINS))
                {
                    putsym(buf, sp, sp->name);
                    oprintf(outputFile, "\tXREF\t%s\n", buf);
                }
                if (sp->storage_class == sc_external && sp->mainsym->extflag)
                {
                    putsym(buf, sp, sp->name);
                    oprintf(outputFile, "\tXREF\t%s\n", buf);
                }
                if (sp->storage_class == sc_defunc)
                {
                    SYM *sp1 = sp->tp->lst.head;
                    while (sp1)
                    {
                        if (sp1->storage_class == sc_externalfunc && sp1
                            ->mainsym->extflag && !(sp1->tp->cflags &DF_INTRINS)
                            )
                        {
                            putsym(buf, sp1, sp1->name);
                            oprintf(outputFile, "\tXREF\t%s\n", buf);
                        }
                        sp1 = sp1->next;
                    }
                }
                if (sp->storage_class == sc_namespace && !sp
                    ->value.classdata.parentns->anonymous)
                    dumphashtable(sp->value.classdata.parentns->table);
                sp = sp->next;
            }
        }
    }
}

//-------------------------------------------------------------------------

void putexterns(void)
/*
 * Output the fixup tables and the global/external list
 */
{
    SYM *sp;
    DATALINK *p;
    int i, notyetg;
    int started = FALSE;
    p = datahead;
    curseg = fixcseg;
    if (!prm_asmfile)
        return ;
    #ifndef COLDFIRE
        while (p)
        {
            if (p->type)
            {
                if (!started && prm_asmfile)
                {
                    nl();
                    oprintf(outputFile, "\tSECTION\tcodefix\n");
                    started = TRUE;
                }
                genlongref(p);
            }
            p = p->next;
        }
        started = FALSE;
        p = datahead;
        curseg = fixdseg;
        while (p)
        {
            if (!p->type)
            {
                if (!started && prm_asmfile)
                {
                    nl();
                    oprintf(outputFile, "\tSECTION\tdatafix\n");
                    started = TRUE;
                }
                genlongref(p);
            }
            p = p->next;
        }
    #endif 
    curseg = noseg;
    if (prm_asmfile)
    {
        nl();
        dumphashtable(gsyms);
    }
    while (localfuncs)
    {
        sp = localfuncs->data;
        if (sp->storage_class == sc_externalfunc && sp->mainsym->extflag && !
            (sp->tp->cflags &DF_INTRINS))
        {
            if (!sp->mainsym->gennedvirtfunc)
            {
                char buf[512];
                putsym(buf, sp, sp->name);
                oprintf(outputFile, "\tXREF\t%s\n", buf);
            }
        }
        localfuncs = localfuncs->link;
    }
    for (i = 0; i < HASHTABLESIZE; i++)
    {
        struct _templatelist *tl;
        if ((tl = (struct templatelist*)templateFuncs[i]) != 0)
        {
            while (tl)
            {
                if (!tl->sp->value.classdata.templatedata->hasbody)
                {
                    if (tl->finalsp && !tl->finalsp->gennedvirtfunc)
                    {
                        char buf[512];
                        putsym(buf, tl->finalsp, sp->name);
                        oprintf(outputFile, "\tXREF\t%s\n", buf);
                    } 
                }
                tl = tl->next;
            }
        }
    }
    while (localdata)
    {
        sp = localdata->data;
        if (sp->mainsym->extflag)
        {
            char buf[512];
            putsym(buf, sp, sp->name);
            oprintf(outputFile, "\tXREF\t%s\n", buf);
        }
        localdata = localdata->link;
    }
    if (libincludes)
    {
        while (libincludes)
        {
            oprintf(outputFile, "\tINCLUDELIB\t%s\n", libincludes->data);
            libincludes = libincludes->link;
        }
        oputc('\n', outputFile);
    }
    oprintf(outputFile, "\tEND\n");


}
