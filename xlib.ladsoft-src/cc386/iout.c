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
 * iout.c
 *
 * output routines for icode code gen.  Used only in optimizer tests.
 *
 */
#include <stdio.h>
#include "lists.h"
#include "expr.h"
#include "c.h"
#include "iexpr.h"
#include "diag.h"
#include "lists.h"

/*      variable initialization         */
extern HASHREC **gsyms;
extern QUAD *intermed_head;
extern int prm_optmult, prm_cplusplus;
extern int stdintsize, stdretblocksize;

enum e_gt
{
    nogen, bytegen, wordgen, intgen, enumgen, longgen, floatgen, doublegen, longdoublegen,
        srrefgen
};
enum e_sg
{
    noseg, codeseg, dataseg, bssxseg, stringseg, constseg, startupxseg,
        rundownxseg, cppxseg, xcpprdseg, virtseg = 100
};

extern LIST *localfuncs;
extern long nextlabel;
extern int global_flag;
extern int prm_lines;
extern FILE *icdFile;
extern char outfile[];
extern LIST *libincludes;
extern int prm_cmangle;
extern SYM *currentfunc;


int gentype = nogen; /* Current DC type */
int curseg = noseg; /* Current seg */
int outcol = 0; /* Curront col (roughly) */
int dataofs; /* Offset from last label */
char dataname[40]; /* Name of last label */

static int virtual_mode;
static struct slit *strtab;
static int newlabel;
/* Init module */
void outcodeini(void)
{
    gentype = nogen;
    curseg = noseg;
    outcol = 0;
	virtual_mode = 0;
	newlabel = FALSE;
} 
static void op_nop(QUAD *q)
{
    oprintf(icdFile,"\tNOP");
}
/*
 * ICODE op display handlers
 */
static void op_line(QUAD *q)
{
    if (!prm_lines)
        return ;
    oprintf(icdFile, ";\n; Line %d:\t%s\n;", (int)q->dc.label, (char*)q
        ->dc.left);
}

//-------------------------------------------------------------------------

static void op_passthrough(QUAD *q)
{
    oprintf(icdFile, "%s\n", (char*)q->dc.left);
}

//-------------------------------------------------------------------------

static void op_label(QUAD *q)
{
    oprintf(icdFile, "L_%d:", q->dc.label);
}

//-------------------------------------------------------------------------

static void putsingle(IMODE *ap, char *string)
{
    oprintf(icdFile, "\t%s", string);
    if (ap)
    {
        oputc('\t', icdFile);
        putamode(ap);
    }
}

//-------------------------------------------------------------------------

static void op_goto(QUAD *q)
{
    oprintf(icdFile, "\tGOTO\tL_%d:PC", q->dc.label);
}

//-------------------------------------------------------------------------

static void op_gosub(QUAD *q)
{
    putsingle(q->dc.left, "GOSUB");
}

//-------------------------------------------------------------------------

static void op_trap(QUAD *q)
{
    putsingle(q->dc.left, "TRAP");
}

//-------------------------------------------------------------------------

static void op_int(QUAD *q)
{
    putsingle(q->dc.left, "INT");
}

//-------------------------------------------------------------------------

static void op_ret(QUAD *q)
{
    oprintf(icdFile, "\tRET\n");
}

//-------------------------------------------------------------------------

static void op_rett(QUAD *q)
{
    oprintf(icdFile, "\tRETT\n");
}

//-------------------------------------------------------------------------

static void putbin(QUAD *q, char *str)
{
    oputc('\t', icdFile);
    putamode(q->ans);
    oprintf(icdFile, " = ");
    putamode(q->dc.left);
    oprintf(icdFile, " %s ", str);
    putamode(q->dc.right);
}

//-------------------------------------------------------------------------

static void op_add(QUAD *q)
{
    putbin(q, "+");
}

//-------------------------------------------------------------------------

static void op_sub(QUAD *q)
{
    putbin(q, "-");
}

//-------------------------------------------------------------------------

static void op_udiv(QUAD *q)
{
    putbin(q, "U/");
}

//-------------------------------------------------------------------------

static void op_umod(QUAD *q)
{
    putbin(q, "U%");
}

//-------------------------------------------------------------------------

static void op_sdiv(QUAD *q)
{
    putbin(q, "S/");
}

//-------------------------------------------------------------------------

static void op_smod(QUAD *q)
{
    putbin(q, "S%");
}

//-------------------------------------------------------------------------

static void op_umul(QUAD *q)
{
    putbin(q, "U*");
}

//-------------------------------------------------------------------------

static void op_smul(QUAD *q)
{
    putbin(q, "S*");
}

//-------------------------------------------------------------------------

static void op_lsl(QUAD *q)
{
    putbin(q, "U<<");
}

//-------------------------------------------------------------------------

static void op_lsr(QUAD *q)
{
    putbin(q, "U>>");
}

//-------------------------------------------------------------------------

static void op_asl(QUAD *q)
{
    putbin(q, "S<<");
}

//-------------------------------------------------------------------------

static void op_asr(QUAD *q)
{
    putbin(q, "S>>");
}

//-------------------------------------------------------------------------

static void op_and(QUAD *q)
{
    putbin(q, "&");
}

//-------------------------------------------------------------------------

static void op_or(QUAD *q)
{
    putbin(q, "|");
}

//-------------------------------------------------------------------------

static void op_eor(QUAD *q)
{
    putbin(q, "^");
}

//-------------------------------------------------------------------------

static void putunary(QUAD *q, char *str)
{
    oputc('\t', icdFile);
    putamode(q->ans);
    oprintf(icdFile, " = ");
    oprintf(icdFile, " %s ", str);
    putamode(q->dc.left);
}

//-------------------------------------------------------------------------

static void putasunary(QUAD *q, char *str)
{
    oputc('\t', icdFile);
    putamode(q->ans);
    oprintf(icdFile, " %s ", str);
    putamode(q->dc.left);
}

//-------------------------------------------------------------------------

static void op_neg(QUAD *q)
{
    putunary(q, "-");
}

//-------------------------------------------------------------------------

static void op_not(QUAD *q)
{
    putunary(q, "~");
}

//-------------------------------------------------------------------------

static void op_assn(QUAD *q)
{
    putunary(q, "");
}

//-------------------------------------------------------------------------

static void op_asadd(QUAD *q)
{
    putasunary(q, "+=");
}

//-------------------------------------------------------------------------

static void op_assub(QUAD *q)
{
    putasunary(q, "-=");
}

//-------------------------------------------------------------------------

static void op_assmul(QUAD *q)
{
    putasunary(q, "S*=");
}

//-------------------------------------------------------------------------

static void op_asumul(QUAD *q)
{
    putasunary(q, "U*=");
}

//-------------------------------------------------------------------------

static void op_assdiv(QUAD *q)
{
    putasunary(q, "S/=");
}

//-------------------------------------------------------------------------

static void op_asudiv(QUAD *q)
{
    putasunary(q, "U/=");
}

//-------------------------------------------------------------------------

static void op_assmod(QUAD *q)
{
    putasunary(q, "S%=");
}

//-------------------------------------------------------------------------

static void op_asumod(QUAD *q)
{
    putasunary(q, "U%=");
}

//-------------------------------------------------------------------------

static void op_aslsl(QUAD *q)
{
    putasunary(q, "U<<=");
}

//-------------------------------------------------------------------------

static void op_asasl(QUAD *q)
{
    putasunary(q, "S<<=");
}

//-------------------------------------------------------------------------

static void op_aslsr(QUAD *q)
{
    putasunary(q, "U>>=");
}

//-------------------------------------------------------------------------

static void op_asasr(QUAD *q)
{
    putasunary(q, "S>>=");
}

//-------------------------------------------------------------------------

static void op_asand(QUAD *q)
{
    putasunary(q, "&=");
}

//-------------------------------------------------------------------------

static void op_asor(QUAD *q)
{
    putasunary(q, "|=");
}

//-------------------------------------------------------------------------

static void op_aseor(QUAD *q)
{
    putasunary(q, "^=");
}

//-------------------------------------------------------------------------

static void op_asuminus(QUAD *q)
{
    putasunary(q, "=-");
}

//-------------------------------------------------------------------------

static void op_ascompl(QUAD *q)
{
    putasunary(q, "=~");
}

//-------------------------------------------------------------------------

static void op_genword(QUAD *q)
{
    putsingle(q->dc.left, "GENWORD");
}

//-------------------------------------------------------------------------

static void op_coswitch(QUAD *q)
{
    oprintf(icdFile, "\tCOSWITCH(");
    putamode(q->ans);
    oputc(',', icdFile);
    putamode(q->dc.left);
    oputc(',', icdFile);
    putamode(q->dc.right);
    oputc(',', icdFile);
    oprintf(icdFile, "L_%d:PC)", q->dc.label);
}
static void op_swbranch(QUAD *q)
{
    oprintf(icdFile, "\tSWBRANCH(");
    putamode(q->dc.left);
    oputc(',', icdFile);
    oprintf(icdFile, "L_%d:PC)", q->dc.label);
}

//-------------------------------------------------------------------------

static void op_dc(QUAD *q)
{
    oprintf(icdFile, "\tDC.L\tL_%d:PC", q->dc.label);
}

//-------------------------------------------------------------------------

static void op_array(QUAD *q)
{
	oputc('\t',icdFile);
    putamode(q->ans);
    oprintf(icdFile, " = ");
    oprintf(icdFile, " ARRAY ");
    putamode(q->dc.left);
	oputc(',',icdFile);
    putamode(q->dc.right);
}

//-------------------------------------------------------------------------

static void op_arrayindex(QUAD *q)
{
	oputc('\t',icdFile);
    putamode(q->ans);
    oprintf(icdFile, " = ");
    oprintf(icdFile, " ARRIND ");
    putamode(q->dc.left);
	oputc(',',icdFile);
    putamode(q->dc.right);
}
//-------------------------------------------------------------------------

static void op_assnblock(QUAD *q)
{
    oputc('\t', icdFile);
    putamode(q->ans);
    oprintf(icdFile, " BLOCK= ");
    putamode(q->dc.left);
    oprintf(icdFile, "(");
    putamode(q->dc.right);
    oprintf(icdFile, ")");
}

//-------------------------------------------------------------------------

static void op_clrblock(QUAD *q)
{
    oputc('\t', icdFile);
    putamode(q->dc.left);
    oprintf(icdFile, " BLKCLR ");
    oprintf(icdFile, "(");
    putamode(q->dc.right);
    oprintf(icdFile, ")");
}
//-------------------------------------------------------------------------

static void putjmp(QUAD *q, char *str)
{
    oprintf(icdFile, "\tCONDGO\tL_%d:PC ; ", q->dc.label);
    if (q->dc.left)
        putamode(q->dc.left);
    oprintf(icdFile, " %s ", str);
    if (q->dc.right)
        putamode(q->dc.right);
}


//-------------------------------------------------------------------------

static void putset(QUAD *q, char *str)
{
    oputc('\t',icdFile);
    putamode(q->ans);
    oprintf(icdFile, " = ");
    putamode(q->dc.left);
    oprintf(icdFile, " %s ", str);
    putamode(q->dc.right);
}

//-------------------------------------------------------------------------

static void op_jc(QUAD *q)
{
    putjmp(q, "U<");
}

//-------------------------------------------------------------------------

static void op_ja(QUAD *q)
{
    putjmp(q, "U>");
}

//-------------------------------------------------------------------------

static void op_je(QUAD *q)
{
    putjmp(q, "==");
}

//-------------------------------------------------------------------------

static void op_jnc(QUAD *q)
{
    putjmp(q, "U>=");
}

//-------------------------------------------------------------------------

static void op_jbe(QUAD *q)
{
    putjmp(q, "U<=");
}

//-------------------------------------------------------------------------

static void op_jne(QUAD *q)
{
    putjmp(q, "!=");
}

//-------------------------------------------------------------------------

static void op_jl(QUAD *q)
{
    putjmp(q, "S<");
}

//-------------------------------------------------------------------------

static void op_jg(QUAD *q)
{
    putjmp(q, "S>");
}

//-------------------------------------------------------------------------

static void op_jle(QUAD *q)
{
    putjmp(q, "S<=");
}

//-------------------------------------------------------------------------

static void op_jge(QUAD *q)
{
    putjmp(q, "S>=");
}

//-------------------------------------------------------------------------

static void op_setc(QUAD *q)
{
    putset(q, "U<");
}

//-------------------------------------------------------------------------

static void op_seta(QUAD *q)
{
    putset(q, "U>");
}

//-------------------------------------------------------------------------

static void op_setnz(QUAD *q)
{
    putset(q, "!=");
}

//-------------------------------------------------------------------------

static void op_sete(QUAD *q)
{
    putset(q, "==");
}

//-------------------------------------------------------------------------

static void op_setnc(QUAD *q)
{
    putset(q, "U>=");
}

//-------------------------------------------------------------------------

static void op_setbe(QUAD *q)
{
    putset(q, "U<=");
}

//-------------------------------------------------------------------------

static void op_setne(QUAD *q)
{
    putset(q, "!=");
}

//-------------------------------------------------------------------------

static void op_setl(QUAD *q)
{
    putset(q, "S<");
}

//-------------------------------------------------------------------------

static void op_setg(QUAD *q)
{
    putset(q, "S>");
}

//-------------------------------------------------------------------------

static void op_setle(QUAD *q)
{
    putset(q, "S<=");
}

//-------------------------------------------------------------------------

static void op_setge(QUAD *q)
{
    putset(q, "S>=");
}

//-------------------------------------------------------------------------

static void op_parm(QUAD *q)
{
    oprintf(icdFile, "\tPARM\t");
    putamode(q->dc.left);
}

//-------------------------------------------------------------------------

static void op_parmadj(QUAD *q)
{
    oprintf(icdFile, "\tPARMADJ\t");
    putamode(q->dc.left);
}

//-------------------------------------------------------------------------

static void op_parmblock(QUAD *q)
{
    fputs("\tPARMBLOCK", icdFile);
    oprintf(icdFile, "{%d}=\t ", (int)q->dc.right);
    putamode(q->dc.left);
}

//-------------------------------------------------------------------------

static void op_cppini(QUAD *q)
{
    fputs("\tCPPINI", icdFile);
}

//-------------------------------------------------------------------------

static void op_block(QUAD *q)
{
    oprintf(icdFile, "\tBLOCK\t%d", q->dc.label + 1);
}

//-------------------------------------------------------------------------

static void op_livein(QUAD *q)
{
    DIAG("op_livein: propogated live-in node");
}

//-------------------------------------------------------------------------

static void op_icon(QUAD *q)
{
    oputc('\t', icdFile);
    putamode(q->ans);
    oprintf(icdFile, " = #%llX", q->dc.v.i);
}

//-------------------------------------------------------------------------

static void op_fcon(QUAD *q)
{
    oputc('\t', icdFile);
    putamode(q->ans);
    oprintf(icdFile, " = #%f", q->dc.v.f);
}
static void op_imcon(QUAD *q)
{
    oputc('\t', icdFile);
    putamode(q->ans);
    oprintf(icdFile, " = #%f", q->dc.v.f);
}
static void op_cxcon(QUAD *q)
{
    oputc('\t', icdFile);
    putamode(q->ans);
    oprintf(icdFile, " = #%f + %f * I", q->dc.v.c.r, q->dc.v.c.i);
}
static void op_prologue(QUAD *q)
{
	oprintf(icdFile,"\tPROLOGUE");
}
static void op_epilogue(QUAD *q)
{
	oprintf(icdFile,"\tEPILOGUE");
}
static void op_blockstart(QUAD *q)
{
	oprintf(icdFile,"\tDBGBLK START #%d",q->dc.label);
}
static void op_blockend(QUAD *q)
{
	oprintf(icdFile,"\tDBGBLK END #%d",q->dc.label);
}
/* List of opcodes
 * This list MUST be in the same order as the op_ enums 
 */
static void(*oplst[])() = 
{
     /* NOPROTO */
	    op_nop, op_line, op_passthrough, op_label, op_goto, op_gosub, op_trap, op_int,
        op_ret, op_rett, op_add, op_sub, op_udiv, op_umod, op_sdiv, op_smod,
        op_umul, op_smul, op_lsl, op_lsr, op_asl, op_asr, op_neg, op_not,
        op_and, op_or, op_eor, 
        op_asadd, op_assub, op_asudiv, op_asumod, op_assdiv, op_assmod, op_asumul, op_assmul,
        op_aslsl, op_aslsr, op_asasl, op_asasr, op_asand, op_asor, op_aseor, op_asuminus, op_ascompl,
        op_setne, op_sete, op_setc, op_seta, op_setnc,
        op_setbe, op_setl, op_setg, op_setle, op_setge, op_assn, op_genword,
        op_coswitch, op_swbranch, op_dc, op_assnblock, op_clrblock, op_jc, op_ja, op_je, op_jnc, op_jbe,
        op_jne, op_jl, op_jg, op_jle, op_jge, op_parm, op_parmadj, op_parmblock, op_array, op_arrayindex, 
        op_cppini, op_block, op_livein, op_icon, op_fcon, op_imcon, op_cxcon, op_prologue, op_epilogue,
		op_blockstart, op_blockend

};
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
        strcpy(buf, sp->alias);
    else
    {
        if (sp->pascaldefn)
        {
            char *q = buf;
            if (prm_cmangle)
                p++;
            while (*p)
                *q++ = toupper(*p++);
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
            char *q;
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
            if (prm_cplusplus)
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
 *      put a constant to the icdFile file.
 */
{
    switch (offset->nodetype)
    {
        case en_icon:
        case en_lcon:
        case en_iucon:
        case en_lucon:
        case en_ccon:
        case en_boolcon:
        case en_cucon:
            oprintf(icdFile, "%lX", offset->v.i);
            break;
        case en_llcon:
        case en_llucon:
            oprintf(icdFile, "%llX", offset->v.i);
            break;
        case en_fcon:
        case en_rcon:
        case en_lrcon:
        case en_fimaginarycon:
        case en_rimaginarycon:
        case en_lrimaginarycon:
            oprintf(icdFile, "%e", (double)offset->v.f);
			break;
        case en_tempref:
            oprintf(icdFile, "TEMP%d", ((SYM*)offset->v.sp)->value.i);
            break;
        case en_autoreg:
            oprintf(icdFile, "%s:REG", ((SYM*)offset->v.sp)->name);
            break;
        case en_autocon:
            oprintf(icdFile, "%s:LINK", ((SYM*)offset->v.sp)->name);
            if (offset->v.sp->value.i >= stdretblocksize)
                oprintf(icdFile, "+%d", (offset->v.sp->value.i - stdretblocksize)/stdintsize);
            break;
        case en_nalabcon:
            oprintf(icdFile, "L_%ld:RAM", offset->v.sp->value.i);
            break;
        case en_labcon:
            oprintf(icdFile, "L_%ld:PC", offset->v.i);
            break;
        case en_napccon:
            oprintf(icdFile, "%s:PC", ((SYM*)offset->v.sp)->name);
            break;
        case en_nacon:
            oprintf(icdFile, "%s:RAM", ((SYM*)offset->v.sp)->name);
            break;
        case en_absacon:
            oprintf(icdFile, "$%lX:ABS", ((SYM*)offset->v.sp)->name);
            break;
        case en_add:
            putconst(offset->v.p[0]);
            oprintf(icdFile, "+");
            putconst(offset->v.p[1]);
            break;
        case en_sub:
            putconst(offset->v.p[0]);
            oprintf(icdFile, "-");
            putconst(offset->v.p[1]);
            break;
        case en_uminus:
            oputc('-', icdFile);
            putconst(offset->v.p[0]);
            break;
        default:
            DIAG("putconst: illegal constant node.");
            break;
    }
}

//-------------------------------------------------------------------------

void putlen(int l)
/*
 *      append the length field to a value
 */
{
    switch (l)
    {
        case ISZ_NONE:
            oprintf(icdFile, ".0");
            break;
        case ISZ_BOOL:
            oprintf(icdFile, ".BOOL");
            break;
		case ISZ_ADDR:
            oprintf(icdFile, ".A");
            break;
        case ISZ_UCHAR:
            oprintf(icdFile, ".UC");
            break;
            case  - ISZ_UCHAR: oprintf(icdFile, ".C");
            break;
        case ISZ_USHORT:
            oprintf(icdFile, ".US");
            break;
            case  - ISZ_USHORT: oprintf(icdFile, ".S");
            break;
		case ISZ_UINT:
			oprintf(icdFile, ".UI");
			break;
		case -ISZ_UINT:
			oprintf(icdFile, ".I");
			break;
        case ISZ_ULONG:
            oprintf(icdFile, ".UL");
            break;
            case  - ISZ_ULONG: oprintf(icdFile, ".L");
            break;
        case ISZ_ULONGLONG:
            oprintf(icdFile, ".ULL");
            break;
        case  - ISZ_ULONGLONG: oprintf(icdFile, ".LL");
            break;
        case ISZ_FLOAT:
            oprintf(icdFile, ".F");
            break;
        case ISZ_DOUBLE:
            oprintf(icdFile, ".D");
            break;
        case ISZ_LDOUBLE:
            oprintf(icdFile, ".LD");
            break;
        case ISZ_IFLOAT:
            oprintf(icdFile, ".IF");
            break;
        case ISZ_IDOUBLE:
            oprintf(icdFile, ".ID");
            break;
        case ISZ_ILDOUBLE:
            oprintf(icdFile, ".ILD");
            break;
        case ISZ_CFLOAT:
            oprintf(icdFile, ".CF");
            break;
        case ISZ_CDOUBLE:
            oprintf(icdFile, ".CD");
            break;
        case ISZ_CLDOUBLE:
            oprintf(icdFile, ".CLD");
            break;
        default:
            DIAG("putlen: illegal length field.");
            break;
    }
}

//-------------------------------------------------------------------------

void putamode(IMODE *ap)
/*
 *      output a general addressing mode.
 */
{
    /* We want to see if the compiler does anything wrong with
     * volatiles
     */
    if (ap->offset && ap->offset->cflags &DF_VOL)
        oputc('%', icdFile);
    switch (ap->mode)
    {
        case i_immed:
            oputc('#', icdFile);
            putconst(ap->offset);
            break;
        case i_ind:
            oputc('*', icdFile);
        case i_direct:
            putconst(ap->offset);
            break;
        case i_rsp:
            oprintf(icdFile, "SP");
            break;
        case i_rret:
            oprintf(icdFile, "RV");
            break;
        case i_rlink:
            oprintf(icdFile, "LINK");
            break;
        case i_rstruct:
            oprintf(icdFile, "STRUCTRET");
            return ;
        case i_rstructstar:
            oprintf(icdFile, "*STRUCTRET");
            return ;
        default:
            DIAG("putamode: illegal address mode.");
            break;
    }
    if (ap->bits)
        oprintf(icdFile, "{%d:%d}", ap->startbit, ap->bits);
    putlen(ap->size);
}

//-------------------------------------------------------------------------

void put_code(QUAD *q)
/*
 *      output a generic instruction.
 */
{
    (*oplst[q->dc.opcode])(q);
    oputc('\n', icdFile);
}

/*
 * Low level routinne to rewrite code for processor and dump it.
 * Here we only need dump
 */
void rewrite_icode(void)
{
    QUAD *q = intermed_head;
    while (q)
    {
        put_code(q);
        q = q->fwd;
    }
    intermed_head = 0;
}

//-------------------------------------------------------------------------

void gen_strlab(SYM *sp)
/*
 *      generate a named label.
 */
{
    char buf[100];
    putsym(buf, sp, sp->name);
	newlabel = TRUE;
    oprintf(icdFile, "%s:\n", buf);
    dataofs = 0;
}

//-------------------------------------------------------------------------

void put_label(int lab)
/*
 *      icdFile a compiler generated label.
 */
{
    sprintf(dataname, "L_%d", lab);
    oprintf(icdFile, "%s:\n", dataname);
    dataofs = 0;
}

//-------------------------------------------------------------------------

void put_staticlabel(long label)
{
    put_label(label);
}

//-------------------------------------------------------------------------

void genfloat(float val)
/*
 * Output a float value
 */
{
    if (gentype == floatgen && outcol < 60)
    {
        oprintf(icdFile, ",%f", val);
        outcol += 8;
    }
    else
    {
        nl();
        oprintf(icdFile, "\tDC.S\t%f", val);
        gentype = floatgen;
        outcol = 19;
    }
    dataofs += 4;
}

//-------------------------------------------------------------------------

void gendouble(double val)
/*
 * Output a double value
 */
{
    if (gentype == doublegen && outcol < 60)
    {
        oprintf(icdFile, ",%f", val);
        outcol += 8;
    }
    else
    {
        nl();
        oprintf(icdFile, "\tDC.D\t%f", val);
        gentype = doublegen;
        outcol = 19;
    }
    dataofs += 8;
}

//-------------------------------------------------------------------------

void genlongdouble(long double val)
/*
 * Output a double value
 */
{
    if (gentype == longdoublegen && outcol < 60)
    {
        oprintf(icdFile, ",%f", val);
        outcol += 8;
    }
    else
    {
        nl();
        oprintf(icdFile, "\tDC.X\t%f", val);
        gentype = longdoublegen;
        outcol = 19;
    }
    dataofs += 8;
}

//-------------------------------------------------------------------------
int genstring(char *str, int uselong, int len)
/*
 * Generate a string literal
 */
{
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
}

//-------------------------------------------------------------------------

void genbyte(long val)
/*
 * Output a byte value
 */
{
    if (gentype == bytegen && outcol < 60)
    {
        oprintf(icdFile, ",$%X", val &0x00ff);
        outcol += 4;
    }
    else
    {
        nl();
        oprintf(icdFile, "\tDC.B\t$%X", val &0x00ff);
        gentype = bytegen;
        outcol = 19;
    }
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
    if (gentype == wordgen && outcol < 58)
    {
        oprintf(icdFile, ",$%X", val &0x0ffff);
        outcol += 6;
    }
    else
    {
        nl();
        oprintf(icdFile, "\tDC.W\t$%X", val &0x0ffff);
        gentype = wordgen;
        outcol = 21;
    }
    dataofs += 2;
}

//-------------------------------------------------------------------------

void genlong(long val)
/*
 * Output a long value
 */
{
    if (gentype == longgen && outcol < 56)
    {
        oprintf(icdFile, ",$%lX", val);
        outcol += 10;
    }
    else
    {
        nl();
        oprintf(icdFile, "\tDC.L\t$%lX", val);
        gentype = longgen;
        outcol = 25;
    }
    dataofs += 4;
}

//-------------------------------------------------------------------------

void genlonglong(LLONG_TYPE val)
/*
 * Output a long value
 */
{
    if (gentype == longgen && outcol < 56)
    {
        #ifdef BCC32
            oprintf(icdFile, "\tDC.L\t0%lXH,0%lXH", val, val < 0 ?  - 1: 0);
        #else 
            oprintf(icdFile, "\tDC.L\t0%lXH,0%lXH", val, val >> 32);
        #endif 
        outcol += 10;
    }
    else
    {
        nl();
        #ifdef BCC32
            oprintf(icdFile, "\tDC.L\t0%lXH,0%lXH", val, val < 0 ?  - 1: 0);
        #else 
            oprintf(icdFile, "\tDC.L\t0%lXH,0%lXH", val, val >> 32);
        #endif 
        gentype = longgen;
        outcol = 25;
    }
}
void genint(int val)
{
    if (gentype == intgen && outcol < 56)
    {
        oprintf(icdFile, ",$%lX", val);
        outcol += 10;
    }
    else
    {
        nl();
        oprintf(icdFile, "\tDC.I\t$%lX", val);
        gentype = intgen;
        outcol = 25;
    }
    dataofs += 4;
}
void genenum(int val)
{
    if (gentype == enumgen && outcol < 56)
    {
        oprintf(icdFile, ",$%lX", val);
        outcol += 10;
    }
    else
    {
        nl();
        oprintf(icdFile, "\tDC.ENUM\t$%lX", val);
        gentype = enumgen;
        outcol = 25;
    }
    dataofs += 4;
}
//-------------------------------------------------------------------------

void genaddress(char *string)
/*
 * Output a long value
 */
{
    if (gentype == longgen && outcol < 56)
    {
        oprintf(icdFile, ",%s", string);
        outcol += strlen(string);
    }
    else
    {
        nl();
        oprintf(icdFile, "\tDC.A\t%s", string);
        gentype = longgen;
        outcol = 25;
    }
    dataofs += 4;
}

//-------------------------------------------------------------------------

void gensrref(char *name, int val)
/*
 * Output a startup/rundown reference
 */
{
    if (gentype == srrefgen && outcol < 56)
    {
        oprintf(icdFile, ",%s,%d", name, val);
        outcol += strlen(name) + 1;
    }
    else
    {
        nl();
        oprintf(icdFile, "\tDC.A\t%s,%d", name, val);
        gentype = srrefgen;
        outcol = 25;
    }
}

//-------------------------------------------------------------------------

void genref(SYM *sp, int offset)
/*
 * Output a reference to the data area (also gens fixups )
 */
{
    char sign;
    char buf[40];
    if (offset < 0)
    {
        sign = '-';
        offset =  - offset;
    }
    else
        sign = '+';
    sprintf(buf, "%s%c%d", sp->name, sign, offset);
    {
        if (gentype == longgen && outcol < 55-strlen(sp->name))
        {
            oprintf(icdFile, ",%s", buf);
            outcol += (11+strlen(sp->name));
        }
        else
        {
            nl();
            oprintf(icdFile, "\tDC.A\t%s", buf);
            outcol = 26+strlen(sp->name);
            gentype = longgen;
        }
    }
    dataofs += 4;
}

//-------------------------------------------------------------------------

void genpcref(SYM *sp, int offset)
/*
 * Output a reference to the code area (also gens fixups )
 */
{
    char sign;
    char buf[40];
    if (offset < 0)
    {
        sign = '-';
        offset =  - offset;
    }
    else
        sign = '+';
    sprintf(buf, "%s%c%d", sp->name, sign, offset);
    {
        if (gentype == longgen && outcol < 55-strlen(sp->name))
        {
            oprintf(icdFile, ",%s", buf);
            outcol += (11+strlen(sp->name));
        }
        else
        {
            nl();
            oprintf(icdFile, "\tDC.A\t%s", buf);
            outcol = 26+strlen(sp->name);
            gentype = longgen;
        }
    }
    dataofs += 4;
}

//-------------------------------------------------------------------------

void genstorage(int nbytes)
/*
 * Output bytes of storage
 */
{
    {
        nl();
        oprintf(icdFile, "\tDS.B\t$%X\n", nbytes);
    }
    dataofs += nbytes;
}

//-------------------------------------------------------------------------

void gen_labref(int n)
/*
 * Generate a reference to a label
 */
{
    if (gentype == longgen && outcol < 58)
    {
        oprintf(icdFile, ",L_%d", n);
        outcol += 6;
    }
    else
    {
        nl();
        oprintf(icdFile, "\tDC.A\tL_%d", n);
        outcol = 22;
        gentype = longgen;
    }
    dataofs += 4;
}

void gen_labdifref(int n1, int n2)
{
    {
        if (gentype == longgen && outcol < 58)
        {
            oprintf(icdFile, ",L_%d-L_%d", n1, n2);
            outcol += 6;
        }
        else
        {
            if (!newlabel)
                nl();
            else
                newlabel = FALSE;
            oprintf(icdFile, "\tDC.A\tL_%d-L_%d", n1, n2);
            outcol = 22;
            gentype = longgen;
        }
    }
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
/*
 *      dump the string literal pool.
 */
{
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
            strtab = strtab->next;
        } nl();
    }
}

//-------------------------------------------------------------------------

void nl(void)
/*
 * New line
 */
{
    {
        if (outcol > 0)
        {
            oputc('\n', icdFile);
            outcol = 0;
            gentype = nogen;
        }
    }
}

/*
 * Switch to cseg 
 */
void cseg(void)
{
    if (curseg != codeseg)
    {
        nl();
        oprintf(icdFile, "\tSECTION\tcode\n");
        curseg = codeseg;
    }
}

/*
 * Switch to dseg
 */
void dseg(void)
{
    if (curseg != dataseg)
    {
        nl();
        oprintf(icdFile, "\tSECTION\tdata\n");
        curseg = dataseg;
    }
}

/*
 * Switch to bssseg
 */
void bssseg(void)
{
    if (curseg != bssxseg)
    {
        nl();
        oprintf(icdFile, "\tSECTION\tbss\n");
        curseg = bssxseg;
    }
}
/*
 * Switch to const seg
 */
void xconstseg(void)
{
    if (curseg != constseg)
    {
        nl();
        oprintf(icdFile, "\tSECTION\tconst\n");
        curseg = constseg;
    }
}
/*
 * Switch to string seg
 */
void xstringseg(void)
{
    if (curseg != stringseg)
    {
        nl();
        oprintf(icdFile, "\tSECTION\tstring\n");
        curseg = stringseg;
    }
}

/*
 * Switch to startupseg
 */
void startupseg(void)
{
    if (curseg != startupxseg)
    {
        nl();
        oprintf(icdFile, "\tSECTION\tcstartup\n");
        curseg = startupxseg;
    }
}

/*
 * Switch to rundownseg
 */
void rundownseg(void)
{
    if (curseg != rundownxseg)
    {
        nl();
        oprintf(icdFile, "\tSECTION\tcrundown\n");
        curseg = rundownxseg;
    }
}

/*
 * Switch to cppseg
 */
void cppseg(void)
{
    if (curseg != cppxseg)
    {
        nl();
        oprintf(icdFile, "\tSECTION\tcppinit\n");
        curseg = cppxseg;
    }
}

//-------------------------------------------------------------------------

void cpprdseg(void)
{
    if (curseg != xcpprdseg)
    {
        nl();
        oprintf(icdFile, "\tSECTION\tcpprundown\n");
        curseg = xcpprdseg;
    }
}

//-------------------------------------------------------------------------


void exitseg(void)
{
}
void gen_virtual(SYM *sp, int data)
{
    {
        char buf[512];
		if (!data && curseg != codeseg)
            cseg();
        virtual_mode = data;
        curseg = virtseg;
        putsym(buf, sp, sp->name);

        nl();
        oprintf(icdFile, "@%s\tVIRTUAL", buf);
        oprintf(icdFile, "%s:\n", buf);
   }
}

//-------------------------------------------------------------------------

void gen_endvirtual(SYM *sp)
{
    {
        char buf[512];
        putsym(buf, sp, sp->name);
        nl();
        oprintf(icdFile, "@%s\tENDVIRTUAL", buf);
		if (virtual_mode)
			dseg();
		else
			cseg();
    }
}
void align(int size)
{
    if (curseg == codeseg)
        return ;
	nl();
	oprintf("align %d\n",size);
}
//-------------------------------------------------------------------------

void asm_header(void)
{
    oprintf(icdFile, ";Icode test - %s\n\n", outfile);
}

//-------------------------------------------------------------------------

void globaldef(SYM *sp)
{
    char buf[100],  *q = buf,  *p = sp->name;
    if (curseg == codeseg && currentfunc->pascaldefn)
    {
        if (prm_cmangle)
            p++;
        while (*p)
            *q++ = toupper(*p++);
        *q++ = 0;
    }
    else
        strcpy(buf, p);
    oprintf(icdFile, "\tPUBLIC\t%s\n", buf);
}

//-------------------------------------------------------------------------

void output_alias(SYM *sp)
{
    char buf[256];
    putsym(buf, sp, sp->name);
    oprintf(icdFile, "%s EQU\t<%s>\n", buf, sp->alias);
}

//-------------------------------------------------------------------------

int put_exfunc(SYM *sp, int notyet)
{
    char buf[100],  *q = buf,  *p = sp->name;
    putsym(buf, sp, p);
    if (notyet)
    {
        oprintf(icdFile, "\n\t.CODE\n");
        notyet = FALSE;
    }
    if (sp->tp->type == bt_func || sp->tp->type == bt_ifunc)
        oprintf(icdFile, "\tEXTRN\t%s:PROC\n", buf);
    else
        oprintf(icdFile, "\tEXTRN\t%s:DATA\n", buf);
    if (sp->importable)
        oprintf(icdFile, "\timport %s %s\n", buf, sp->importfile);
    return notyet;
}

//-------------------------------------------------------------------------

int put_expfunc(SYM *sp, int notyet)
{
    char buf[256];
    if (notyet)
    {
        oprintf(icdFile, "\n.CODE\n");
        notyet = FALSE;
    }
    putsym(buf, sp, sp->name);
    oprintf(icdFile, "\texport %s\n", buf);
}

//-------------------------------------------------------------------------

void putexterns(void)
/*
 * Output the fixup tables and the global/external list
 */
{
    SYM *sp;
    int i;
    LIST *l;
    char buf[100];
    {
        int notyet = TRUE;
        nl();
        exitseg();
        for (i = 0; i < HASHTABLESIZE; i++)
        {
            if ((sp = (SYM*)gsyms[i]) != 0)
            {
                while (sp)
                {
                    if (sp->storage_class == sc_externalfunc && sp->mainsym
                        ->extflag && !(sp->tp->cflags &DF_INTRINS))
                        notyet = put_exfunc(sp, notyet);
                    if (sp->storage_class == sc_external && sp->mainsym
                        ->extflag)
                        notyet = put_exfunc(sp, notyet);
                    if (sp->storage_class == sc_defunc)
                    {
                        SYM *sp1 = sp->tp->lst.head;
                        while (sp1)
                        {
                            if (sp1->storage_class == sc_externalfunc && sp1
                                ->mainsym->extflag && !(sp1->tp->cflags
                                &DF_INTRINS))
                            {
                                notyet = put_exfunc(sp1, notyet);
                            }
                            sp1 = sp1->next;
                        }
                    }
                    if (sp->exportable)
                        notyet = put_expfunc(sp, notyet);
                    sp = sp->next;
                }
            }
        }

        while (localfuncs)
        {
            sp = localfuncs->data;
            if (sp->storage_class == sc_externalfunc && sp->mainsym->extflag &&
                !(sp->tp->cflags &DF_INTRINS))
                notyet = put_exfunc(sp, notyet);
            if (sp->exportable)
                notyet = put_expfunc(sp, notyet);
            localfuncs = localfuncs->link;
        }
        notyet = TRUE;

        if (libincludes)
        {
            while (libincludes)
            {
                oprintf(icdFile, "\tINCLUDELIB\t%s\n", libincludes->data);
                libincludes = libincludes->link;
            }
            oputc('\n', icdFile);
        }
        oprintf(icdFile, "\tEND\n");
    }
}
