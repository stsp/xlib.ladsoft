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
extern int prm_optmult;

enum e_gt
{
    nogen, bytegen, wordgen, longgen, floatgen, doublegen, longdoublegen,
        srrefgen
};
enum e_sg
{
    noseg, codeseg, dataseg, bssxseg, stringseg, constseg, startupxseg,
        rundownxseg, cppxseg, xcpprdseg
};

extern LIST *localfuncs;
extern long nextlabel;
extern int global_flag;
extern int prm_asmfile;
extern int prm_lines;
extern int phiused;
extern FILE *outputFile;
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

/* Init module */
void outcodeini(void)
{
    gentype = nogen;
    curseg = noseg;
    outcol = 0;
    phiput = FALSE;
	virtual_mode = 0;
} 
/*
 * ICODE op display handlers
 */
static void op_line(QUAD *q)
{
    if (!prm_lines)
        return ;
    oprintf(outputFile, ";\n; Line %d:\t%s\n;", (int)q->dc.label, (char*)q
        ->dc.left);
}

//-------------------------------------------------------------------------

static void op_passthrough(QUAD *q)
{
    oprintf(outputFile, "%s\n", (char*)q->dc.left);
}

//-------------------------------------------------------------------------

static void op_label(QUAD *q)
{
    oprintf(outputFile, "L_%d:", q->dc.label);
}

//-------------------------------------------------------------------------

static void putsingle(IMODE *ap, char *string)
{
    oprintf(outputFile, "\t%s", string);
    if (ap)
    {
        fputc('\t', outputFile);
        putamode(ap);
    }
}

//-------------------------------------------------------------------------

static void op_goto(QUAD *q)
{
    oprintf(outputFile, "\tGOTO\tL_%d:PC", q->dc.label);
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
    oprintf(outputFile, "\tRET\n");
}

//-------------------------------------------------------------------------

static void op_rett(QUAD *q)
{
    oprintf(outputFile, "\tRETT\n");
}

//-------------------------------------------------------------------------

static void putbin(QUAD *q, char *str)
{
    fputc('\t', outputFile);
    putamode(q->ans);
    oprintf(outputFile, " = ");
    putamode(q->dc.left);
    oprintf(outputFile, " %s ", str);
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
    putbin(q, "<<");
}

//-------------------------------------------------------------------------

static void op_lsr(QUAD *q)
{
    putbin(q, "U>>");
}

//-------------------------------------------------------------------------

static void op_asl(QUAD *q)
{
    putbin(q, "<<");
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
    fputc('\t', outputFile);
    putamode(q->ans);
    oprintf(outputFile, " = ");
    oprintf(outputFile, " %s ", str);
    putamode(q->dc.left);
}

//-------------------------------------------------------------------------

static void op_setnz(QUAD *q)
{
    putunary(q, "SETNZ");
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

static void op_genword(QUAD *q)
{
    putsingle(q->dc.left, "GENWORD");
}

//-------------------------------------------------------------------------

static void op_coswitch(QUAD *q)
{
    oprintf(outputFile, "\tCOSWITCH(");
    putamode(q->ans);
    fputc(',', outputFile);
    putamode(q->dc.left);
    fputc(',', outputFile);
    putamode(q->dc.right);
    fputc(',', outputFile);
    oprintf(outputFile, "L_%d:PC)", q->dc.label);
}

//-------------------------------------------------------------------------

static void op_dc(QUAD *q)
{
    oprintf(outputFile, "\tDC.L\tL_%d:PC", q->dc.label);
}

//-------------------------------------------------------------------------

static void op_assnblock(QUAD *q)
{
    fputc('\t', outputFile);
    putamode(q->ans);
    oprintf(outputFile, " BLOCK= ");
    putamode(q->dc.left);
    oprintf(outputFile, "(");
    putamode(q->dc.right);
    oprintf(outputFile, ")");
}

//-------------------------------------------------------------------------

static void putjmp(QUAD *q, char *str)
{
    oprintf(outputFile, "\tCONDGO\tL_%d:PC ; ", q->dc.label);
    if (q->dc.left)
        putamode(q->dc.left);
    oprintf(outputFile, " %s ", str);
    if (q->dc.right)
        putamode(q->dc.right);
}

//-------------------------------------------------------------------------

static void putset(QUAD *q, char *str)
{
    putamode(q->ans);
    oprintf(outputFile, " = ");
    putamode(q->dc.left);
    oprintf(outputFile, " %s ", str);
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
    oprintf(outputFile, "\tPARM\t");
    putamode(q->dc.left);
}

//-------------------------------------------------------------------------

static void op_parmadj(QUAD *q)
{
    oprintf(outputFile, "\tPARMADJ\t");
    putamode(q->dc.left);
}

//-------------------------------------------------------------------------

static void op_parmblock(QUAD *q)
{
    fputs("\tPARMBLOCK", outputFile);
    oprintf(outputFile, "{%d}=\t ", (int)q->dc.right);
    putamode(q->dc.left);
}

//-------------------------------------------------------------------------

static void op_cppini(QUAD *q)
{
    fputs("\tCPPINI", outputFile);
}

//-------------------------------------------------------------------------

static void op_block(QUAD *q)
{
    oprintf(outputFile, "\tBLOCK\t%d", q->dc.label + 1);
}

//-------------------------------------------------------------------------

static void op_livein(QUAD *q)
{
    DIAG("op_livein: propogated live-in node");
}

//-------------------------------------------------------------------------

static void op_icon(QUAD *q)
{
    fputc('\t', outputFile);
    putamode(q->ans);
    oprintf(outputFile, " = #%lX", q->dc.v.i);
}

//-------------------------------------------------------------------------

static void op_fcon(QUAD *q)
{
    fputc('\t', outputFile);
    putamode(q->ans);
    oprintf(outputFile, " = #%f", q->dc.v.f);
}

/* List of opcodes
 * This list MUST be in the same order as the op_ enums 
 */
static void(*oplst[])() = 
{
     /* NOPROTO */
    op_line, op_passthrough, op_label, op_goto, op_gosub, op_trap, op_int,
        op_ret, op_rett, op_add, op_sub, op_udiv, op_umod, op_sdiv, op_smod,
        op_umul, op_smul, op_lsl, op_lsr, op_asl, op_asr, op_neg, op_not,
        op_and, op_or, op_eor, op_setne, op_sete, op_setc, op_seta, op_setnc,
        op_setbe, op_setl, op_setg, op_setle, op_setge, op_assn, op_genword,
        op_coswitch, op_dc, op_assnblock, op_jc, op_ja, op_je, op_jnc, op_jbe,
        op_jne, op_jl, op_jg, op_jle, op_jge, op_parm, op_parmadj, op_parmblock,
        op_cppini, op_block, op_livein, op_icon, op_fcon 

};
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
            if (prm_cplusplus && prm_asmfile && !prm_nasm)
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
    switch (offset->nodetype)
    {
        case en_icon:
        case en_lcon:
        case en_iucon:
        case en_lucon:
        case en_ccon:
        case en_boolcon:
        case en_cucon:
            oprintf(outputFile, "%lX", offset->v.i);
            break;
        case en_llcon:
        case en_llucon:
            oprintf(outputFile, "%llX", offset->v.i);
            break;
        case en_fcon:
        case en_rcon:
        case en_lrcon:
        case en_fimaginarycon:
        case en_rimaginarycon:
        case en_lrimaginarycon:
            oprintf(outputFile, "%e", offset->v.f);
			break;
        case en_tempref:
            oprintf(outputFile, "TEMP%d", ((SYM*)offset->v.p[0])->value.i);
            break;
        case en_autoreg:
            oprintf(outputFile, "%s:REG", ((SYM*)offset->v.sp)->name);
            break;
        case en_autocon:
            oprintf(outputFile, "%s:LINK", ((SYM*)offset->v.sp)->name);
            break;
        case en_nalabcon:
            oprintf(outputFile, "L_%ld:RAM", offset->v.sp->value.i);
            break;
        case en_labcon:
            oprintf(outputFile, "L_%ld:PC", offset->v.i);
            break;
        case en_napccon:
            oprintf(outputFile, "%s:PC", ((SYM*)offset->v.sp)->name);
            break;
        case en_nacon:
            oprintf(outputFile, "%s:RAM", ((SYM*)offset->v.sp)->name);
            break;
        case en_absacon:
            oprintf(outputFile, "$%lX:ABS", ((SYM*)offset->v.sp)->name);
            break;
        case en_add:
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
            fputc('-', outputFile);
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
        case BESZ_NONE:
            oprintf(outputFile, ".0");
            break;
		case BESZ_FARPTR:
			oprintf(outputFile, ".FAR");
			break ;
        case BESZ_BOOL:
            oprintf(outputFile, ".BOOL");
            break;
        case BESZ_BYTE:
            oprintf(outputFile, ".UB");
            break;
            case  - BESZ_BYTE: oprintf(outputFile, ".B");
            break;
        case BESZ_SHORT:
            oprintf(outputFile, ".UW");
            break;
            case  - BESZ_SHORT: oprintf(outputFile, ".W");
            break;
        case BESZ_DWORD:
            oprintf(outputFile, ".UL");
            break;
            case  - BESZ_DWORD: oprintf(outputFile, ".L");
            break;
        case BESZ_QWORD:
            oprintf(outputFile, ".ULL");
            break;
        case  - BESZ_QWORD: oprintf(outputFile, ".LL");
            break;
        case BESZ_FLOAT:
            oprintf(outputFile, ".S");
            break;
        case BESZ_DOUBLE:
            oprintf(outputFile, ".D");
            break;
        case BESZ_LDOUBLE:
            oprintf(outputFile, ".X");
            break;
        case BESZ_IFLOAT:
            oprintf(outputFile, ".SI");
            break;
        case BESZ_IDOUBLE:
            oprintf(outputFile, ".DI");
            break;
        case BESZ_ILDOUBLE:
            oprintf(outputFile, ".XI");
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
        fputc('%', outputFile);
    switch (ap->mode)
    {
        case i_immed:
            fputc('#', outputFile);
            putconst(ap->offset);
            break;
        case i_ind:
            fputc('*', outputFile);
        case i_direct:
            putconst(ap->offset);
            break;
        case i_rsp:
            oprintf(outputFile, "SP");
            break;
        case i_rret:
            oprintf(outputFile, "RV");
            break;
        case i_rlink:
            oprintf(outputFile, "LINK");
            break;
        case i_rstruct:
            oprintf(outputFile, "STRUCTRET");
            return ;
        case i_rstructstar:
            oprintf(outputFile, "*STRUCTRET");
            return ;
        default:
            DIAG("putamode: illegal address mode.");
            break;
    }
    if (ap->bits)
        oprintf(outputFile, "{%d:%d}", ap->startbit, ap->bits);
    putlen(ap->size);
}

//-------------------------------------------------------------------------

void put_code(QUAD *q)
/*
 *      output a generic instruction.
 */
{
    if (!prm_asmfile)
        return ;
    (*oplst[q->dc.opcode])(q);
    fputc('\n', outputFile);
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
    if (prm_asmfile)
        oprintf(outputFile, "%s:\n", buf);
    dataofs = 0;
}

//-------------------------------------------------------------------------

void put_label(int lab)
/*
 *      outputFile a compiler generated label.
 */
{
    sprintf(dataname, "L_%d", lab);
    if (prm_asmfile)
        oprintf(outputFile, "%s:\n", dataname);
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
    if (prm_asmfile)
    if (gentype == floatgen && outcol < 60)
    {
        oprintf(outputFile, ",%f", val);
        outcol += 8;
    }
    else
    {
        nl();
        oprintf(outputFile, "\tDC.S\t%f", val);
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
    if (prm_asmfile)
    if (gentype == doublegen && outcol < 60)
    {
        oprintf(outputFile, ",%f", val);
        outcol += 8;
    }
    else
    {
        nl();
        oprintf(outputFile, "\tDC.D\t%f", val);
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
    if (prm_asmfile)
    if (gentype == longdoublegen && outcol < 60)
    {
        oprintf(outputFile, ",%f", val);
        outcol += 8;
    }
    else
    {
        nl();
        oprintf(outputFile, "\tDC.X\t%f", val);
        gentype = longdoublegen;
        outcol = 19;
    }
    dataofs += 8;
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
    dataofs += 1;
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
    dataofs += 4;
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
        #ifdef BCC32
            oprintf(outputFile, "\tDC.L\t0%lXH,0%lXH", val, val < 0 ?  - 1: 0);
        #else 
            oprintf(outputFile, "\tDC.L\t0%lXH,0%lXH", val, val >> 32);
        #endif 
        outcol += 10;
    }
    else
    {
        nl();
        #ifdef BCC32
            oprintf(outputFile, "\tDC.L\t0%lXH,0%lXH", val, val < 0 ?  - 1: 0);
        #else 
            oprintf(outputFile, "\tDC.L\t0%lXH,0%lXH", val, val >> 32);
        #endif 
        gentype = longgen;
        outcol = 25;
    }
}
void genlong(int val)
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
        oprintf(outputFile, "\tDC.I\t$%lX", val);
        gentype = longgen;
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
    if (prm_asmfile)
    if (gentype == longgen && outcol < 56)
    {
        oprintf(outputFile, ",%s", string);
        outcol += strlen(string);
    }
    else
    {
        nl();
        oprintf(outputFile, "\tDC.A\t%s", string);
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
    if (prm_asmfile)
    if (gentype == srrefgen && outcol < 56)
    {
        oprintf(outputFile, ",%s,%d", name, val);
        outcol += strlen(name) + 1;
    }
    else
    {
        nl();
        oprintf(outputFile, "\tDC.A\t%s,%d", name, val);
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
    if (prm_asmfile)
    {
        if (gentype == longgen && outcol < 55-strlen(sp->name))
        {
            oprintf(outputFile, ",%s", buf);
            outcol += (11+strlen(sp->name));
        }
        else
        {
            nl();
            oprintf(outputFile, "\tDC.A\t%s", buf);
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
    if (prm_asmfile)
    {
        if (gentype == longgen && outcol < 55-strlen(sp->name))
        {
            oprintf(outputFile, ",%s", buf);
            outcol += (11+strlen(sp->name));
        }
        else
        {
            nl();
            oprintf(outputFile, "\tDC.A\t%s", buf);
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
    if (prm_asmfile)
    {
        nl();
        oprintf(outputFile, "\tDS.B\t$%X\n", nbytes);
    }
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
        oprintf(outputFile, "\tDC.A\tL_%d", n);
        outcol = 22;
        gentype = longgen;
    }
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
/*
 *      dump the string literal pool.
 */
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
    if (prm_asmfile)
    {
        if (outcol > 0)
        {
            fputc('\n', outputFile);
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
    if (prm_asmfile)
    if (curseg != codeseg)
    {
        nl();
        oprintf(outputFile, "\tSECTION\tcode\n");
        curseg = codeseg;
    }
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
        curseg = dataseg;
    }
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
        curseg = bssxseg;
    }
}
/*
 * Switch to const seg
 */
void xconstseg(void)
{
    if (prm_asmfile)
    if (curseg != constseg)
    {
        nl();
        oprintf(outputFile, "\tSECTION\tconst\n");
        curseg = constseg;
    }
}
/*
 * Switch to string seg
 */
void xstringseg(void)
{
    if (prm_asmfile)
    if (curseg != stringseg)
    {
        nl();
        oprintf(outputFile, "\tSECTION\tstring\n");
        curseg = stringseg;
    }
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
        curseg = startupxseg;
    }
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
        curseg = rundownxseg;
    }
}

/*
 * Switch to cppseg
 */
void cppseg(void)
{
    if (prm_asmfile)
    if (curseg != cppxseg)
    {
        nl();
        oprintf(outputFile, "\tSECTION\tcppinit\n");
        curseg = cppxseg;
    }
}

//-------------------------------------------------------------------------

void cpprdseg(void)
{
    if (prm_asmfile)
    if (curseg != xcpprdseg)
    {
        nl();
        oprintf(outputFile, "\tSECTION\tcpprundown\n");
        curseg = xcpprdseg;
    }
}

//-------------------------------------------------------------------------


void exitseg(void)
{
}
void gen_virtual(char *name, int data)
{
    if (prm_asmfile)
    {
        char buf[512];
		if (!data && curseg != codeseg)
            cseg();
        virtual_mode = data;
        curseg = virtseg;
        putsym(buf, sp, sp->name);

        nl();
        oprintf(outputFile, "@%s\tVIRTUAL", buf);
        oprintf(outputFile, "%s:\n", buf);
   }
}

//-------------------------------------------------------------------------

void gen_endvirtual(char *name)
{
    if (prm_asmfile)
    {
        nl();
        oprintf(outputFile, "@%s\tENDVIRTUAL", name);
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
void dump_browsedata(unsigned char *data, int len)
{
}

//-------------------------------------------------------------------------

void asm_header(void)
{
    oprintf(outputFile, ";Icode test - %s\n\n", outfile);
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
    oprintf(outputFile, "\tPUBLIC\t%s\n", buf);
}

//-------------------------------------------------------------------------

void output_alias(SYM *sp)
{
    char buf[256];
    putsym(buf, sp, sp->name);
    oprintf(outputFile, "%s EQU\t<%s>\n", buf, sp->alias);
}

//-------------------------------------------------------------------------

int put_exdata(SYM *sp, int notyet)
{
    char buf[512],  *q = buf,  *p = sp->name;
    putsym(buf, sp, p);
    if (notyet)
    {
        oprintf(outputFile, "\n\t.CODE\n");
        notyet = FALSE;
    }
    oprintf(outputFile, "\tEXTRN\t%s:DATA\n", buf);
    if (sp->importable)
        oprintf(outputFile, "\timport %s %s\n", buf, sp->importfile);
    return notyet;
}
int put_exfunc(SYM *sp, int notyet)
{
    char buf[512],  *q = buf,  *p = sp->name;
    putsym(buf, sp, p);
    if (notyet)
    {
        oprintf(outputFile, "\n\t.CODE\n");
        notyet = FALSE;
    }
    oprintf(outputFile, "\tEXTRN\t%s:PROC\n", buf);
    if (sp->importable)
        oprintf(outputFile, "\timport %s %s\n", buf, sp->importfile);
    return notyet;
}

//-------------------------------------------------------------------------

int put_expfunc(SYM *sp, int notyet)
{
    char buf[512];
    if (notyet)
    {
        oprintf(outputFile, "\n.CODE\n");
        notyet = FALSE;
    }
    putsym(buf, sp, sp->name);
    oprintf(outputFile, "\texport %s\n", buf);
}

//-------------------------------------------------------------------------

//-------------------------------------------------------------------------

static int dumphashtable(int notyet, HASHREC **syms)
{
    int i;
    SYM *sp;
    char buf[100];
    for (i = 0; i < HASHTABLESIZE; i++)
    {
        if ((sp = (SYM*)syms[i]) != 0)
        {
            while (sp)
            {
                if (sp->storage_class == sc_externalfunc && sp->mainsym
                    ->extflag && !(sp->tp->cflags &DF_INTRINS))
                    notyet = put_exfunc(sp, notyet);
                if (sp->storage_class == sc_external && sp->mainsym->extflag)
                    notyet = put_exfunc(sp, notyet);
                if (sp->storage_class == sc_defunc)
                {
                    SYM *sp1 = sp->tp->lst.head;
                    while (sp1)
                    {
                        if (sp1->mainsym->storage_class == sc_externalfunc &&
                            sp1->mainsym->extflag && !(sp1->tp->cflags
                            &DF_INTRINS))
                        {
                            notyet = put_exfunc(sp1, notyet);
                        }
                        if (sp1->mainsym->exportable && !(sp1->value.classdata.cppflags & PF_INLINE))
                            notyet = put_expfunc(sp1, notyet);
                        sp1 = sp1->next;
                    }
                }
                if (sp->storage_class == sc_namespace && !sp
                    ->value.classdata.parentns->anonymous)
                    notyet = dumphashtable(notyet, sp->value.classdata.parentns
                        ->table);
                if (sp->mainsym->exportable && !(sp->value.classdata.cppflags & PF_INLINE))
                    notyet = put_expfunc(sp, notyet);
                sp = sp->next;
            }
        }
    }
    return notyet;
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
    if (prm_asmfile)
    {
        int notyet = TRUE;
        nl();
        exitseg();
        notyet = dumphashtable(notyet, gsyms);

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

        for (i = 0; i < HASHTABLESIZE; i++)
        {
            struct _templatelist *tl;
            if ((tl = (struct templatelist*)templateFuncs[i]) != 0)
            {
                while (tl)
                {
                    if (!tl->sp->value.classdata.templatedata->hasbody && tl
                        ->finalsp)
                        notyet = put_exfunc(tl->finalsp, notyet);
                    tl = tl->next;
                } 
            }
        }
        notyet = TRUE;
        exitseg();
        while (localdata)
        {
            sp = localdata->data;
            if (sp->mainsym->extflag)
            {
                notyet = put_exdata(sp, notyet);
            }
            localdata = localdata->link;
        }
            if (libincludes)
            {
                while (libincludes)
                {
                    oprintf(outputFile, "\tINCLUDELIB\t%s\n", libincludes->data)
                        ;
                    libincludes = libincludes->link;
                }
                oputc('\n', outputFile);
            }
            oprintf(outputFile, "\tEND\n");
    }
}
