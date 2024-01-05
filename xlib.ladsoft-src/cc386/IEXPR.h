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
 * iexpr.h
 *
 * ICODE structures
 */

#define F_ADDR 1
#define F_NOVALUE 2
#define F_VOL 4

/* icode innstruction opcodes */
enum i_ops
{
        /* note, the branches MUST be higher in the ordering than the sets */
    	i_nop, i_line, i_passthrough, i_label, i_goto, i_gosub, i_fargosub, i_trap, i_int, i_ret,
        i_rett, i_add, i_sub, i_udiv, i_umod, i_sdiv, i_smod, i_umul, i_smul,
        i_lsl, i_lsr, i_asl, i_asr, i_neg, i_not, i_and, i_or, i_eor, 
        i_asadd, i_assub, i_asudiv, i_asumod, i_assdiv, i_assmod, i_asumul, i_assmul,
        i_aslsl, i_aslsr, i_asasl, i_asasr, i_asand, i_asor, i_aseor, i_asuminus, i_ascompl,
        i_setne, i_sete, i_setc, i_seta, i_setnc, i_setbe, i_setl, i_setg, i_setle,
        i_setge, i_assn, i_genword, i_coswitch, i_swbranch, i_dc, i_assnblock, i_clrblock, i_jc, i_ja,
        i_je, i_jnc, i_jbe, i_jne, i_jl, i_jg, i_jle, i_jge, i_parm, i_parmadj, i_parmblock,
        i_array, i_arrayindex, i_cppini, i_block, i_livein, i_icon, i_fcon, i_imcon, i_cxcon, 
		i_prologue, i_epilogue, i_blockstart, i_blockend, i_pushcontext, i_popcontext, i_loadcontext,
        i_substack,
    /* Dag- specific stuff */
    	i_var, i_const, i_ptr, i_labcon, 
        /* end marker */
        i_endoflist
};

/* icode address modes annd special regs */
enum i_adr
{
    i_none, i_immed, i_direct, i_ind, i_bf, i_specialreg, 
    /* special named nodes come AFTER rsp for no bugs */
    i_rsp, i_rret, i_this, i_rlink, i_rstruct, i_rstructstar
};
/*
 * structure for startup/rundown records (used in #pragma ) 
 */
typedef struct _startup
{
    struct _startup *link;
    char *name; /* name */
    int val; /* prio */
} STARTUP;

/*
 * address mode
 */
typedef struct _imode_
{
    enum i_adr mode; /* mode */
    int useindx;
    char size; /* size */
    char startbit, bits; /* bit width  for i_Bf*/
    char vol; /* TRUE if is a node for a volatile var */
	char restricted; /* TRUE if pointer type is set to restricted */
	char seg;	/* seg reg  for segmented architectures */
    struct enode *offset; /* offset */
}

//-------------------------------------------------------------------------

IMODE;

#define IM_LIVELEFT 1
#define IM_LIVERIGHT 2
/*
 * icode node
 * this is also used for dag nodes
 */

typedef struct quad
{
    struct _basic_dag
    {
        enum i_ops opcode; /* opcode */
        IMODE *left; /* ans = left opcode right */
        /* for a i_label node, left is the label nnumber cast
         * to a imode* */
        IMODE *right;
        union ival
        {
             /* values for constant nodes */
            LLONG_TYPE i;
            long double f;
			struct {
				long double r;
				long double i;
			} c;
        } v;
        long label; /* aux values used by i_coswitch */
    }
    dc;
    char livein;
	struct quad *fwd, *back;
    IMODE *ans;
    int definition;
    int available;
    int sourceindx;
    int copy;
}

//-------------------------------------------------------------------------

QUAD;

#define DAGCOMPARE sizeof(struct _basic_dag)
#define DAGSIZE 1023

/* constant node combinattions:
 * ic = prefix
 * l = int
 * r = real
 * i = imaginary
 * c = complex
 */
enum e_icmode {
	icnone,icnl,icnr,icni,icnc, 
	icln,icll,iclr,icli,iclc, 
	icrn,icrl,icrr,icri,icrc,
	icin,icil,icir,icii,icic,
	iccn,iccl,iccr,icci,iccc
};
struct cases {
    LLONG_TYPE bottom;
    LLONG_TYPE top;
    int count;
    int deflab;
    int tablepos;
    int diddef : 1;
    struct caseptrs {
        int label;
        int binlabel;
        LLONG_TYPE id;
    } *ptrs;
} ;

#include "iexpr.p"
