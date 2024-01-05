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
/*
 *      code generation structures and constants
 */

/* address mode specifications */
#define F_DREG 1 /* data register     dx   */
#define F_AREG 2 /* address register    ax   */
#define F_REGI 4 /* register indirect     (y,ax) */
#define F_REGIX 8 /* register indirect with index   (y,dx,ax) */
#define F_PCI 16 /* pc indirect      (y,pc) */
#define F_PCIX 32 /* pc indirect with index    (y,dx,pc) */
#define F_ABS 64 /* absolute addressing     $xxxxxxxx */
#define F_IMMED 128 /* immediate mode     #????  */
#define F_FREG 256 /* floating point register    fpx  */
#define F_VOL 512 /* need volatile operand */
#define F_NOVALUE 1024 /* don't need result value */
#define F_NOBIT 2048 /* don't get the bit val, get the address */
#define F_DOUBLEREG 4096 /* long long in regs 0 & 1 */
#define F_NOREUSE 8192 /* don't reuse the reg */

#define F_MEM (F_IMMED|F_ABS|F_PCIX|F_PCI|F_REGIX|F_REGI)  /* memory alterable
    modes */
#define F_DALT (F_IMMED|F_ABS|F_PCIX|F_PCI|F_REGIX|F_REGI|F_DREG) /* data
    alterable modes */
#define F_ALL (F_FREG|F_IMMED|F_ABS|F_PCIX|F_PCI|F_REGIX|F_REGI|F_AREG|F_DREG)
    /* all modes */
#define F_ALT (F_REGIX|F_REGI|F_AREG|F_DREG)    /* alterable modes */
#define F_DIVL (F_REGI|F_AREG|F_DREG)     /* divl modes */

enum e_op
{
    op_reserved, op_line, op_seqx, op_slit, op_label, op_funclabel, op_genword,
        op_void, op_dcl, op_dcr, op_blockstart, op_blockend, op_abcd, op_add,
        op_adda, op_addi, op_addq, op_addx, op_and, op_andi, op_asl, op_asr,
        op_bra, op_beq, op_bne, op_blt, op_ble, op_bgt, op_bge, op_bhi, op_bhs,
        op_blo, op_bls, op_bsr, op_bcc, op_bcs, op_bmi, op_bpl, op_bvc, op_bvs,
        op_bchg, op_bclr, op_bfchg, op_bfclr, op_bfexts, op_bfextu, op_bfffo,
        op_bfins, op_bfset, op_bftst, op_bkpt, op_bset, op_btst, op_chk,
        op_chk2, op_clr, op_cmp, op_cmpa, op_cmpi, op_cmpm, op_cmp2, op_dbeq,
        op_dbne, op_dblt, op_dble, op_dbgt, op_dbge, op_dbhi, op_dbhs, op_dblo,
        op_dbls, op_dbsr, op_dbcc, op_dbcs, op_dbmi, op_dbpl, op_dbvc, op_dbvs,
        op_dbt, op_dbf, op_dbra, op_divs, op_divu, op_divsl, op_divul, op_eor,
        op_eori, op_exg, op_ext, op_extb, op_illegal, op_jmp, op_jsr, op_lea,
        op_link, op_lsl, op_lsr, op_move, op_movea, op_movec, op_movem,
        op_movep, op_moveq, op_moves, op_muls, op_mulu, op_nbcd, op_neg,
        op_negx, op_nop, op_not, op_or, op_ori, op_pack, op_pea, op_rems,
        op_remu, op_reset, op_rol, op_ror, op_roxl, op_roxr, op_rtd, op_rte,
        op_rtr, op_rts, op_sbcd, op_seq, op_sne, op_slt, op_sle, op_sgt, op_sge,
        op_shi, op_shs, op_slo, op_sls, op_ssr, op_scc, op_scs, op_smi, op_spl,
        op_svc, op_svs, op_st, op_sf, op_sub, op_stop, op_suba, op_subi,
        op_subq, op_subx, op_swap, op_tas, op_trap, op_trapeq, op_trapne,
        op_traplt, op_traple, op_trapgt, op_trapge, op_traphi, op_traphs,
        op_traplo, op_trapls, op_trapsr, op_trapcc, op_trapcs, op_trapmi,
        op_trappl, op_trapvc, op_trapvs, op_trapt, op_trapf, op_trapv, op_tst,
        op_unlk, op_unpk, op_fabs, op_facos, op_fadd, op_fasin, op_fatan,
        op_fatanh, op_fbeq, op_fbne, op_fbgt, op_fbngt, op_fbge, op_fbnge,
        op_fblt, op_fbnlt, op_fble, op_fbnle, op_fbgl, op_fbngl, op_fbgle,
        op_fbngle, op_fbogt, op_fbule, op_fboge, op_fbult, op_fbolt, op_fbuge,
        op_fbole, op_fbugt, op_fbogl, op_fbueq, op_fbor, op_fbun, op_fbt,
        op_fbf, op_fbst, op_fbsf, op_fbseq, op_fbsne, op_fcmp, op_fcos,
        op_fcosh, op_fdbeq, op_fdbne, op_fdbgt, op_fdbngt, op_fdbge, op_fdbnge,
        op_fdblt, op_fdbnlt, op_fdble, op_fdbnle, op_fdbgl, op_fdbngl,
        op_fdbgle, op_fdbngle, op_fdbogt, op_fdbule, op_fdboge, op_fdbult,
        op_fdbolt, op_fdbuge, op_fdbole, op_fdbugt, op_fdbogl, op_fdbueq,
        op_fdbor, op_fdbun, op_fdbt, op_fdbf, op_fdbst, op_fdbsf, op_fdbseq,
        op_fdbsne, op_fdiv, op_fetox, op_fetoxm1, op_fgetexp, op_fgetman,
        op_fint, op_fintrz, op_flog10, op_flog2, op_flogn, op_flognp1, op_fmod,
        op_fmove, op_fmovecr, op_fmovem, op_fmul, op_fneg, op_fnop, op_frem,
        op_frestore, op_fsave, op_fscale, op_fseq, op_fsne, op_fsgt, op_fsngt,
        op_fsge, op_fsnge, op_fslt, op_fsnlt, op_fsle, op_fsnle, op_fsgl,
        op_fsngl, op_fsgle, op_fsngle, op_fsogt, op_fsule, op_fsoge, op_fsult,
        op_fsolt, op_fsuge, op_fsole, op_fsugt, op_fsogl, op_fsueq, op_fsor,
        op_fsun, op_fst, op_fsf, op_fsst, op_fssf, op_fsseq, op_fssne,
        op_fsgldiv, op_fsglmul, op_fsin, op_fsincos, op_fsinh, op_fsqrt,
        op_fsub, op_ftan, op_ftanh, op_ftentox, op_ftrapeq, op_ftrapne,
        op_ftrapgt, op_ftrapngt, op_ftrapge, op_ftrapnge, op_ftraplt,
        op_ftrapnlt, op_ftraple, op_ftrapnle, op_ftrapgl, op_ftrapngl,
        op_ftrapgle, op_ftrapngle, op_ftrapogt, op_ftrapule, op_ftrapoge,
        op_ftrapult, op_ftrapolt, op_ftrapuge, op_ftrapole, op_ftrapugt,
        op_ftrapogl, op_ftrapueq, op_ftrapor, op_ftrapun, op_ftrapt, op_ftrapf,
        op_ftrapst, op_ftrapsf, op_ftrapseq, op_ftrapsne, op_ftst, op_ftwotox
};

#define OPE_NEGBCD 1
#define OPE_MATH 2
#define OPE_AMATH 3
#define OPE_IMATH 4
#define OPE_QMATH 5
#define OPE_LOG 6
#define OPE_ILOG 7
#define OPE_SHIFT 8
#define OPE_BRA 9
#define OPE_BIT 10
#define OPE_BF1 11
#define OPE_BF2 12
#define OPE_BF3 13
#define OPE_EAD 14
#define OPE_EAR 15
#define OPE_EA 16
#define OPE_CMP 17
#define OPE_EAA 18
#define OPE_IEA 19
#define OPE_POSBCD 20
#define OPE_DBR 21
#define OPE_DIV 22
#define OPE_DIVL 23
#define OPE_EOR 24
#define OPE_EAI 25
#define OPE_RRL 26
#define OPE_EXT 27
#define OPE_EXTB 28
#define OPE_AI 29
#define OPE_MOVE 30
#define OPE_MOVEA 31
#define OPE_MOVEC 32
#define OPE_MOVEM 33
#define OPE_MOVEP 34
#define OPE_MOVEQ 35
#define OPE_MOVES 36
#define OPE_MUL 37
#define OPE_EAB 38
#define OPE_PACK 39
#define OPE_I 40
#define OPE_SET 41
#define OPE_D 42
#define OPE_TRAPcc 43
#define OPE_A 44
#define OPE_FMATH 45
#define OPE_FMATHX 46
#define OPE_FMOVE 47
#define OPE_FMOVECR 48
#define OPE_FMOVEM 49
#define OPE_FSINCOS 50
#define OPE_FONE 51
#define OPE_JEA 52
#define OPE_LEA 53
#define OPE_TEA 54
#define OPE_ADDX 55
#define OPE_FSAVE 56 
#define OPE_FRESTORE 57

enum e_am
{
    am_none, am_dreg, am_areg, am_freg, am_cfreg, am_ind, am_ainc, am_adec,
        am_indx, am_baseindxaddr, am_direct, am_adirect, am_immed, am_mask,
        am_fmask, am_baseindxdata, am_pcindx, am_pcindxdata, am_pcindxaddr,
        am_muldiv, am_divsl, am_bf, am_bfreg, am_sr, am_ccr, am_pc,  /* latter
        is an assembler dummy */
    am_fpcr, am_fpsr, am_usp, am_sfc, am_dfc, am_vbr, am_cacr, am_caar, am_msp,
        am_isp, am_iiprepca, am_iipostpca, am_iimem, am_iiprepcd, am_iipostpcd,
        am_iiprea, am_iipred, am_iiposta, am_iipostd, am_doublereg, am_bits =
        0x4000
};
/*      addressing mode structure       */

struct amode
{
    enum e_am mode;
    char preg;
    char sreg;
    char scale;
    char tempflag;
    char sz; /* 4-scaled modes, inline assembler */
    char bf1, bf2;
    char length;
    struct enode *offset;
    struct enode *offset2;
};

/*      output code structure   */

struct ocode
{
    struct ocode *fwd,  *back;
    enum e_op opcode;
    short length;
    struct amode *oper1,  *oper2,  *oper3,  *reloffset;
    long address;
    unsigned char outbuf[32];
    unsigned char addroffset[4];
    unsigned char ofs[4];
    unsigned char resobyte[4];
    unsigned char rescount;
    unsigned char outlen;
    unsigned char branched;
    unsigned char pcrelofs;
    unsigned char pcrelres;
    char noopt;
    char diag;
    int blocknum;
};

/* Used for fixup gen */
typedef struct dl
{
    struct dl *next;
    SYM *sp;
    int offset;
    short type;
} DATALINK;

#define AMODE struct amode
#define OCODE struct ocode


enum e_gt
{
    nogen, bytegen, wordgen, longgen, longlonggen, floatgen, doublegen,
        longdoublegen, srrefgen
};
enum e_sg
{
    noseg, codeseg, dataseg, bssxseg, constseg, stringseg, startupxseg,
        rundownxseg, cppxseg, cpprseg, fixcseg, fixdseg, lineseg, typeseg,
        symseg, browseseg, virtseg = 100
};
#define MAX_SEGS (browseseg + 1)

#define BR_FLOAT 32
#define BR_DBRA 16
#define BR_PCREL 8
#define BR_LONG 4
#define BR_SHORT 2
#define BR_BYTE 1

typedef struct
{
    int address;
    int seg;
} LABEL;

typedef struct fixup
{
    struct fixup *next;
    int address;
    int size;
    int ofs;
    enum mode
    {
        fm_label, fm_symbol, fm_rellabel, fm_relsymbol
    } fmode;
    SYM *sym;
    int label;
}

//-------------------------------------------------------------------------

FIXUP;

typedef struct emitlist
{
    struct emitlist *next;
    int filled, lastfilled;
    int address;
    unsigned char data[1024];
    FIXUP *fixups,  *lastfixup;
} EMIT_LIST;

typedef struct
{
    int curbase;
    int curlast;
    EMIT_LIST *first;
    EMIT_LIST *last;
} EMIT_TAB;

typedef struct _virtual_list
{
    struct _virtual_list *next;
    SYM *sp;
    EMIT_TAB *seg;
    int data;
} VIRTUAL_LIST;
typedef struct _linebuf
{
    struct _linebuf *next;
    int lineno;
    int address;
    int file;
} LINEBUF;

typedef struct _dbgblock
{
    struct _dbgblock *next;
    struct _dbgblock *parent;
    struct _dbgblock *child;
    SYM *syms;
    SYM *oldsyms;
    int startofs;
    int endofs;
} DBGBLOCK;
#include "cc68.p"
