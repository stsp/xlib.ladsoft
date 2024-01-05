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
#include <string.h>
#include <limits.h>
#include "lists.h"
#include "expr.h"
#include "c.h"
#include "gen68.h"
#include "diag.h"
 
extern ENODE *allocaSP;
extern int used_alloca;
extern int conscount;
extern int virtualfuncs;
extern SNODE *funcstmt,  *funcendstmt;
extern SYM *thissp;
extern long lc_maxauto;
extern int linkreg;
extern long framedepth;
extern TYP stdfunc;
extern struct amode push[], pop[], mvma7[], mvma7i[];
extern OCODE *peep_tail,  *peep_head,  *peep_insert;
extern long stackdepth;
extern SYM *currentfunc;
extern int prm_cplusplus, prm_linkreg, prm_phiform, prm_68020, prm_68010;
extern int prm_pcrel, prm_datarel, prm_smallcode, prm_profiler, prm_coldfire;
extern long firstlabel, nextlabel;
extern int global_flag;
extern int save_mask, fsave_mask;
extern TABLE *gsyms;
extern ENODE *thisenode;
extern ENODE *block_rundown;
extern char dregs[], aregs[], fregs[];
extern int funcfloat; /* unused, placeholder */
extern int rmaskcnt;
extern int prm_smartframes;
extern int xceptoffs;
extern int prm_xcept;
extern TRYBLOCK *try_block_list;
extern int prm_cplusplus, prm_xcept;
extern int genning_inline;

LIST *mpthunklist;

static int returndreg;
static int diddef;
static int breaklab;
static int contlab;
int retlab;
static long plabel;
static long retsize;

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

void genstmtini(void){}
AMODE *makedreg(int r)
/*
 *      make an address reference to a data register.
 */
{
    AMODE *ap;
    ap = xalloc(sizeof(AMODE));
    ap->mode = am_dreg;
    ap->preg = r;
    return ap;
} 

AMODE *makeareg(int r)
/*
 *      make an address reference to an address register.
 */
{
    AMODE *ap;
    ap = xalloc(sizeof(AMODE));
    ap->mode = am_areg;
    ap->preg = r;
    return ap;
}

//-------------------------------------------------------------------------

AMODE *makefreg(int r)
/*
 *      make an address reference to a data register.
 */
{
    AMODE *ap;
    ap = xalloc(sizeof(AMODE));
    ap->mode = am_freg;
    ap->preg = r;
    return ap;
}

//-------------------------------------------------------------------------

AMODE *make_mask(int mask, int floatflag)
/*
 *      generate the mask address structure.
 */
{
    AMODE *ap;
    ap = xalloc(sizeof(AMODE));
    if (floatflag)
        ap->mode = am_fmask;
    else
        ap->mode = am_mask;
    ap->offset = (ENODE*)mask;
    return ap;
}

//-------------------------------------------------------------------------

AMODE *make_direct(int i)
/*
 *      make a direct reference to an immediate value.
 */
{
    return make_offset(makeintnode(en_icon, i));
}

//-------------------------------------------------------------------------

AMODE *make_strlab(char *s)
/*
 *      generate a direct reference to a string label.
 */
{
    AMODE *ap;
    ap = xalloc(sizeof(AMODE));
    ap->mode = am_direct;
    ap->offset = makenode(en_nacon, s, 0);
    return ap;
}

//-------------------------------------------------------------------------

void genwhile(SNODE *stmt)
/*
 *      generate code to evaluate a while statement.
 */
{
    int lab1, lab2, lab3;
    initstack(); /* initialize temp registers */
    lab1 = contlab; /* save old continue label */
    contlab = nextlabel++; /* new continue label */
    if (stmt->s1 != 0)
     /* has block */
    {
        lab2 = breaklab; /* save old break label */
        breaklab = nextlabel++;
        gen_codes(op_bra, 0, make_label(contlab), 0);
        lab3 = nextlabel++;
        gen_label(lab3);
        genstmt(stmt->s1);
        gen_label(contlab);
        initstack();
        if (stmt->lst)
            gen_line(stmt->lst);
        truejp(stmt->exp, lab3);
        gen_label(breaklab);
        breaklab = lab2; /* restore old break label */
    }
    else
     /* no loop code */
    {
        if (stmt->lst)
            gen_line(stmt->lst);
        gen_label(contlab);
        initstack();
        truejp(stmt->exp, contlab);
    }
    contlab = lab1; /* restore old continue label */
}

//-------------------------------------------------------------------------

void gen_for(SNODE *stmt)
/*
 *      generate code to evaluate a for loop
 */
{
    int old_break, old_cont, exit_label, loop_label, start_label;
    old_break = breaklab;
    old_cont = contlab;
    loop_label = nextlabel++;
    contlab = nextlabel++;
    start_label = nextlabel++;
    exit_label = nextlabel++;
    initstack();
    if (stmt->label != 0)
        gen_void(stmt->label);

    gen_codes(op_bra, 0, make_label(start_label), 0);
    gen_label(loop_label);
    if (stmt->s1 != 0)
    {
        breaklab = exit_label;
        genstmt(stmt->s1);
    }
    if (stmt->lst)
        gen_line(stmt->lst);
    gen_label(contlab);
    initstack();
    if (stmt->s2 != 0)
        gen_void(stmt->s2);
    gen_label(start_label);
    initstack();
    if (stmt->exp != 0)
        truejp(stmt->exp, loop_label);
    else
        gen_codes(op_bra, 0, make_label(loop_label), 0);
    gen_label(exit_label);
    breaklab = old_break;
    contlab = old_cont;
}

//-------------------------------------------------------------------------

void genif(SNODE *stmt)
/*
 *      generate code to evaluate an if statement.
 */
{
    int lab1, lab2;
    lab1 = nextlabel++; /* else label */
    lab2 = nextlabel++; /* exit label */
    initstack(); /* clear temps */
    falsejp(stmt->exp, lab1);
    genstmt(stmt->s1);
    if (stmt->s2 != 0)
     /* else part exists */
    {
        gen_codes(op_bra, 0, make_label(lab2), 0);
        gen_label(lab1);
        genstmt(stmt->s2);
        gen_label(lab2);
    }
    else
     /* no else code */
        gen_label(lab1);
}

//-------------------------------------------------------------------------

void gendo(SNODE *stmt)
/*
 *      generate code for a do - while loop.
 */
{
    int oldcont, oldbreak, looplab;
    oldcont = contlab;
    oldbreak = breaklab;
    looplab = nextlabel++;
    contlab = nextlabel++;
    breaklab = nextlabel++;
    gen_label(looplab);
    genstmt(stmt->s1); /* generate body */
    gen_label(contlab);
    initstack();
    truejp(stmt->exp, looplab);
    gen_label(breaklab);
    breaklab = oldbreak;
    contlab = oldcont;
}

//-------------------------------------------------------------------------

void gen_genword(SNODE *stmt)
/*
 * Generate data in the code stream
 */
{
	AMODE *ap = make_immed((int)stmt->exp);
	ap->mode = am_direct;
    gen_codes(op_genword, 2, ap, 0);
}

//-------------------------------------------------------------------------

AMODE *set_symbol(char *name, int isproc)
/*
 *      generate a call to a library routine.
 */
{
    SYM *sp;
    AMODE *result = xalloc(sizeof(AMODE));
    sp = gsearch(name);
    if (sp == 0)
    {
        ++global_flag;
        sp = makesym(isproc ? sc_externalfunc : sc_external);
        sp->tp = &stdfunc;
        sp->name = name;
        sp->mainsym->extflag = TRUE;
        insert(sp, gsyms);
        --global_flag;
    }
    if (prm_68020)
    {
        result->preg = 4;
    }
    else
    {
        result->preg = 2;
    }
    if (prm_pcrel)
        result->mode = am_pcindx;
    else
        result->mode = am_adirect;
    result->offset = makenode(en_napccon, (void*)sp, 0);
    return result;
}

//-------------------------------------------------------------------------

static char xdregs[3], xaregs[3], xfregs[3];
AMODE *flush_for_libcall()
{
    int i;
    for (i = 0; i < 3; i++)
    {
        if (xdregs[i] = dregs[i])
            gen_push(i, am_dreg, 0);
    }
    for (i = 0; i < 3; i++)
    {
        if (xaregs[i] = aregs[i])
            gen_push(i, am_areg, 0);
    }
    for (i = 0; i < 3; i++)
    {
        if (xfregs[i] = fregs[i])
            gen_push(i, am_freg, 0);
    }
}

//-------------------------------------------------------------------------

void call_library2(char *lib_name, int size)
{
    AMODE *ap1 = set_symbol(lib_name, 1);
    gen_codes(op_jsr, 0, ap1, 0);
    if (size)
        gen_codes(op_add, BESZ_DWORD, make_immed(size), makeareg(7));
}

//-------------------------------------------------------------------------

AMODE *call_library(char *lib_name, int size)
/*
 *      generate a call to a library routine.
 */
{
    int i;
    AMODE *result;
    result = set_symbol(lib_name, 1);
    gen_codes(op_jsr, 0, result, 0);
    if (size)
        gen_codes(op_add, BESZ_DWORD, make_immed(size), makeareg(7));
    result = temp_data();
    if (result->preg != 0)
        gen_codes(op_move, BESZ_DWORD, makedreg(0), result);
    result->tempflag = TRUE;
    for (i = 2; i >= 0; i--)
    {
        if (xfregs[i])
            gen_pop(i, am_freg, 0);
    }
    for (i = 2; i >= 0; i--)
    {
        if (xaregs[i])
            gen_pop(i, am_areg, 0);
    }
    for (i = 2; i >= 0; i--)
    {
        if (xdregs[i])
            gen_pop(i, am_dreg, 0);
    }
    return result;
}
//-------------------------------------------------------------------------

void count_cases(SNODE *stmt, struct cases *cs)
{
    while (stmt)
    {
        switch(stmt->stype) {        
             case st_tryblock:
                break;
            case st_throw:
                break;
            case st_return:
            case st_expr:
                break;
            case st_while:
            case st_do:
                count_cases(stmt->s1,cs);
                break;
            case st_for:
                count_cases(stmt->s1,cs);
                break;
            case st_if:
                count_cases(stmt->s1,cs);
                count_cases(stmt->s2,cs);
                break;
            case st_switch:
                break;
            case st_block:
                count_cases(stmt->exp,cs);
                break;
            case st_asm:
                break;
            case st_case:
               if (!stmt->s2)
                {
                    cs->count++;
                    
                    if (stmt->switchid < cs->bottom)
                        cs->bottom = stmt->switchid;
                    if (stmt->switchid > cs->top)
                        cs->top = stmt->switchid;
                }
                break;
        }
        stmt = stmt->next;
    }
}
//-------------------------------------------------------------------------
void gather_cases(SNODE *stmt, struct cases *cs, int compact)
{
    if (!cs->ptrs) {
        global_flag++;
        if (compact) {
            int i;
            cs->ptrs = xalloc((cs->top - cs->bottom) * sizeof(struct cases));
            for (i = cs->bottom; i < cs->top; i++)
                cs->ptrs[i - cs->bottom].label = cs->deflab;
        } else
            cs->ptrs = xalloc((cs->count) * sizeof(struct cases));
        global_flag--;
    }
    while (stmt)
    {
        switch(stmt->stype) {        
             case st_tryblock:
                break;
            case st_throw:
                break;
            case st_return:
            case st_expr:
                break;
            case st_while:
            case st_do:
                gather_cases(stmt->s1,cs,compact);
                break;
            case st_for:
                gather_cases(stmt->s1,cs,compact);
                break;
            case st_if:
                gather_cases(stmt->s1,cs,compact);
                gather_cases(stmt->s2,cs,compact);
                break;
            case st_switch:
                break;
            case st_block:
                gather_cases(stmt->exp,cs,compact);
                break;
            case st_asm:
                break;
            case st_case:
                if (stmt->s2)
                 /* default case */
                {
                    cs->diddef = TRUE;
                    stmt->label = (SNODE*)cs->deflab;
                }
                else
                {
                    
                    if (compact) {
                        cs->ptrs[stmt->switchid - cs->bottom].label = nextlabel;
                        stmt->label = (SNODE*)nextlabel++;
                    } else {
                        cs->ptrs[cs->tablepos].label = nextlabel;
                        cs->ptrs[cs->tablepos].binlabel =  - 1;
                        cs->ptrs[cs->tablepos++].id = stmt->switchid;
                        stmt->label = (SNODE*)nextlabel++;
                    }
                }
                break;
        }
        stmt = stmt->next;
    }
}

//-------------------------------------------------------------------------

int analyzeswitch(SNODE *stmt, struct cases *cs)
/* 
 * Decide whitch type of switch statement to use
 */
{
    int size = natural_size(stmt->exp);
    count_cases(stmt->s1,cs) ;
    cs->top++;
    if (cs->count == 0)
        return (0);
    if (cs->count < 5)
        return 3;
    if (cs->count *10 / (cs->top - cs->bottom) >= 8)
        return (1);
    // do a simple switch instead of a binary if it is a long long
    if (size == BESZ_QWORD || size ==  - BESZ_QWORD)
        return 3;
    return (2);
}

//-------------------------------------------------------------------------

void bingen(int lower, int avg, int higher, AMODE *ap1, struct cases *cs)
/*
 * Recursively output the compare/jump tree for a type of binary search
 * on the case value
 */
{
    AMODE *ap2 = make_immed(cs->ptrs[avg].id);
    AMODE *ap3 = make_label(cs->ptrs[avg].label);
    if (cs->ptrs[avg].binlabel !=  - 1)
        gen_label(cs->ptrs[avg].binlabel);
    gen_codes(op_cmp, BESZ_DWORD, ap2, ap1);
    gen_codes(op_beq, 0, ap3, 0);
    if (avg == lower)
    {
        if (cs->deflab < 0)
            cs->deflab = nextlabel++;
        ap3 = make_label(cs->deflab);
        gen_codes(op_bra, 0, ap3, 0);
    }
    else
    {
        int avg1 = (lower + avg) / 2;
        int avg2 = (higher + avg + 1) / 2;
        if (avg + 1 < higher)
            ap3 = make_label(cs->ptrs[avg2].binlabel = nextlabel++);
        else
            ap3 = make_label(cs->deflab);
        if (ap1->length < 0)
            gen_codes(op_bgt, 0, ap3, 0);
        else
            gen_codes(op_bhi, 0, ap3, 0);
        bingen(lower, avg1, avg, ap1, cs);
        if (avg + 1 < higher)
            bingen(avg + 1, avg2, higher, ap1, cs);
    }
}

//-------------------------------------------------------------------------
int sortcmp(const void *one, const void *two)
{
    if (((struct caseptrs *)one)->id < ((struct caseptrs *)two)->id)
        return -1;
    if (((struct caseptrs *)one)->id > ((struct caseptrs *)two)->id)
        return 1;
    return 0;
}

void genbinaryswitch(SNODE *stmt, struct cases *cs)
/*
 * Main routine for handling the binary switch setup
 */
{
    AMODE *ap1;
    initstack();
    ap1 = gen_expr(stmt->exp, FALSE, FALSE, BESZ_DWORD);
    do_extend(ap1, ap1->length, F_DREG);
    gather_cases(stmt->s1,cs,FALSE);
    qsort(cs->ptrs,cs->count, sizeof(cs->ptrs[0]), sortcmp);
    bingen(0, (cs->count) / 2, cs->count, ap1, cs);
    freeop(ap1);
}

//-------------------------------------------------------------------------

void gencompactswitch(SNODE *stmt, struct cases *cs)
/*
 * Generate a table lookup mechanism if the switch table isn't too sparse
 */
{
    int tablab, i, size;
    AMODE *ap,  *ap1,  *ap2,  *ap3;
    tablab = nextlabel++;
    initstack();
    ap = gen_expr(stmt->exp, FALSE, FALSE, BESZ_DWORD);
    size = ap->length;
    do_extend(ap, BESZ_DWORD, F_DREG | F_VOL);
    initstack();
    if (size == BESZ_QWORD || size == - BESZ_QWORD)
    {
        if (cs->bottom)
        {
            int label = nextlabel++;
            #if sizeof(LLONG_TYPE) == 4
                gen_codes(op_sub, BESZ_DWORD, make_immed(cs->bottom < 0
                    ?  - 1: 0), makedreg(1));
            #else 
                gen_codes(op_sub, BESZ_DWORD, make_immed(cs->bottom >>
                    32), makedreg(1));
            #endif 
            if (size < 0)
                gen_1branch(op_blt, make_label(cs->deflab));
            else
            {
                peep_tail->noopt = TRUE;
                gen_1branch(op_bcs, make_label(cs->deflab));
            }
            gen_codes(op_sub, BESZ_DWORD, make_immed(cs->bottom), makedreg(0));
            gen_1branch(op_bcc, make_label(label));
            gen_code(op_subq, make_immed(1), makedreg(1));
            if (size < 0)
                gen_1branch(op_blt, make_label(cs->deflab));
            else
            {
                peep_tail->noopt = TRUE;
                gen_1branch(op_bcs, make_label(cs->deflab));
            }
            gen_label(label);
        }
        else
            if (size < 0)
            {
                gen_codes(op_tst, BESZ_DWORD, makedreg(1), makedreg(1));
                gen_1branch(op_blt, make_label(cs->deflab));
            }
            gen_codes(op_cmp, BESZ_DWORD, make_immed(cs->top - cs->bottom), makedreg(0));
    }
    else  {
        if (cs->bottom)
        {
            gen_codes(op_sub, BESZ_DWORD, make_immed(cs->bottom), ap);
            if (size < 0)
                gen_codes(op_blt, 0, make_label(cs->deflab), 0);
            else
            {
                peep_tail->noopt = TRUE;
                gen_codes(op_bcs, 0, make_label(cs->deflab), 0);
            }
        }
        else
        {
            if (size < 0)
            {
                gen_codes(op_tst, BESZ_DWORD, ap, 0);
                gen_codes(op_blt, 0, make_label(cs->deflab), 0);
            }
        }
        gen_codes(op_cmp, BESZ_DWORD, make_immed(cs->top - cs->bottom), ap);
    }
    if (size < 0)
        gen_codes(op_bhs, 0, make_label(cs->deflab), 0);
    else
        gen_codes(op_bcc, 0, make_label(cs->deflab), 0);
    ap1 = temp_addr();
    if (prm_68020 || prm_coldfire)
    {
        ap2 = xalloc(sizeof(AMODE));
        ap2->preg = ap1->preg;
        ap2->mode = am_pcindxdata;
        ap2->scale = 2;
        ap2->offset = makenode(en_labcon, (char*)tablab, 0);
        gen_codes(op_lea, BESZ_DWORD, ap2, ap1);
        ap3 = xalloc(sizeof(AMODE));
        ap3->sreg = ap->preg;
        ap3->preg = ap1->preg;
        ap3->scale = 0;
        ap3->offset = makeintnode(en_icon, 0);
        ap3->mode = am_ind;
        gen_codes(op_adda, BESZ_DWORD, ap3, ap1);
    }
    else
    {
        ap2 = xalloc(sizeof(AMODE));
        if (prm_pcrel)
        {
            ap2->preg = ap1->preg;
            ap2->mode = am_pcindx;
        }
        else
        {
            ap2->mode = am_adirect;
            if (prm_smallcode)
                ap2->preg = 2;
            else
                ap2->preg = 4;
        }
        ap2->offset = makeintnode(en_labcon, tablab);
        gen_codes(op_lea, 0, ap2, ap1);
        if (prm_datarel || prm_smallcode)
            gen_codes(op_asl, BESZ_DWORD, make_immed(1), ap);
        else
            gen_codes(op_asl, BESZ_DWORD, make_immed(2), ap);
        if (prm_datarel)
        {
            gen_codes(op_add, BESZ_DWORD, ap, ap1);
            ap2->mode = am_ind;
            gen_codes(op_add, 2, ap2, ap1);
        }
        else
        {
            ap3 = xalloc(sizeof(AMODE));
            ap3->sreg = ap->preg;
            ap3->preg = ap1->preg;
            ap3->scale = 0;
            ap3->offset = makeintnode(en_icon, 0);
            ap3->mode = am_baseindxdata;
            gen_codes(op_move, BESZ_DWORD, ap3, ap1);
        }
    }
    ap1->mode = am_ind;
    gen_codes(op_jmp, 0, ap1, 0);

    initstack();
    gen_label(tablab);
    gather_cases(stmt->s1,cs,TRUE);
    for (i = 0; i < cs->top - cs->bottom; i++)
        if (!prm_coldfire && (prm_smallcode || prm_pcrel))
            gen_codes(op_dcl, 2, make_label(cs->ptrs[i].label), 0);
        else
            gen_codes(op_dcl, BESZ_DWORD, make_label(cs->ptrs[i].label), 0);
}

//-------------------------------------------------------------------------

void gensimpleswitch(SNODE *stmt, struct cases *cs)
{
    int i = 0, j;
    AMODE *ap1,*ap2,*ap3,*ap4;
    int size = natural_size(stmt->exp);
    initstack();
    ap1 = gen_expr(stmt->exp, FALSE, FALSE, BESZ_DWORD);
    do_extend(ap1, BESZ_DWORD, F_DREG);
    gather_cases(stmt->s1,cs,FALSE);
    qsort(cs->ptrs,cs->count, sizeof(cs->ptrs[0]), sortcmp);
    for (i = 0; i < cs->count; i++)
    {
        if (size == BESZ_QWORD || size ==  - BESZ_QWORD)
        {
            int lab = nextlabel++;
            ap2 = make_immed(cs->ptrs[i].id);
            #if sizeof(LLONG_TYPE) == 4
                ap4 = make_immed(cs->ptrs[i].id < 0 ? 0xffffffff : 0);
            #else 
                ap4 = make_immed(cs->ptrs[i].id >> 32);
            #endif 
            ap3 = make_label(cs->ptrs[i].label);
            gen_coden(op_cmp, BESZ_DWORD, ap2, makedreg(0));
            gen_1branch(op_bne, make_label(lab));
            gen_codes(op_cmp, BESZ_DWORD, ap4, makedreg(1));
            gen_1branch(op_beq, ap3);
            gen_label(lab);
        }
        else {
            AMODE *ap2 = make_immed(cs->ptrs[i].id);
            AMODE *ap3 = make_label(cs->ptrs[i].label);
            gen_codes(op_cmp, ap1->length, ap2, ap1);
            gen_code(op_beq, ap3, 0);
        }
    }
    gen_codes(op_bra, 0, make_label(cs->deflab), 0);
    freeop(ap1);
}

//-------------------------------------------------------------------------

void genxswitch(SNODE *stmt)
/*
 *      analyze and generate best switch statement.
 */
{
    int oldbreak;
    struct cases cs;
    OCODE *result;
    oldbreak = breaklab;
    breaklab = nextlabel++;
    memset(&cs,0,sizeof(cs));
#if sizeof(LLONG_TYPE) == 4
    cs.top = INT_MIN;
    cs.bottom = INT_MAX;
#else
    cs.top = INT_MIN;
    cs.bottom = INT_MAX;
//    cs.top = (a << 63); // LLONG_MIN
//    cs.bottom = cs.top - 1; // LLONG_MAX
#endif
    cs.deflab = nextlabel++;
    switch (analyzeswitch(stmt,&cs))
    {
        case 3:
            gensimpleswitch(stmt, &cs);
            break;
        case 2:
            genbinaryswitch(stmt, &cs);
            break;
        case 1:
            gencompactswitch(stmt, &cs);
            break;
        case 0:
            break;
    }
    result = gen_codes(op_blockstart, 0, 0, 0);
    result->blocknum = (int)stmt->label;
    genstmt(stmt->s1);
    result = gen_codes(op_blockend, 0, 0, 0);
    result->blocknum = (int)stmt->label;
    if (!cs.diddef)
        gen_label(cs.deflab);
    gen_label(breaklab);
    breaklab = oldbreak;
}

//-------------------------------------------------------------------------

void AddProfilerData(void)
{
    char pname[256];
    if (prm_profiler)
    {
        sprintf(pname, "%s", currentfunc->name);
        plabel = stringlit(pname, FALSE, strlen(pname) + 1);
        gen_codes(op_pea, BESZ_DWORD, make_label(plabel), 0);
        call_library("__profile_in", 4);
    }
}

//-------------------------------------------------------------------------

void SubProfilerData(void)
{
    if (prm_profiler)
    {
        gen_codes(op_pea, BESZ_DWORD, make_label(plabel), 0);
        call_library("__profile_out", 4);
    }
}

//-------------------------------------------------------------------------

void genreturn(SNODE *stmt, int flag)
/*
 *      generate a return statement.
 */
{
    AMODE *ap,  *ap1,  *ap2,  *ap3;
    ENODE *mvn1;
    int size;
    SubProfilerData();
    if (stmt != 0 && stmt->exp != 0)
    {
        if (!genning_inline)
            initstack();
        if (currentfunc->tp->btp && isstructured(currentfunc->tp->btp))
        {
            size = currentfunc->tp->btp->size;
            ap = gen_expr(stmt->exp, FALSE, TRUE, BESZ_DWORD);
            returndreg = TRUE;
        }
        else if (currentfunc->tp->btp && currentfunc->tp->btp->type ==
            bt_memberptr)
        {
            size = BESZ_DWORD;
            ap = gen_expr(stmt->exp, FALSE, TRUE, BESZ_DWORD);
            returndreg = TRUE;

        }
        else
        {
            size = currentfunc->tp->btp->size;
            if (size == BESZ_DWORD && currentfunc->tp->btp->type == bt_float)
                size = BESZ_FLOAT;
            if (currentfunc->tp->btp->type == bt_longlong || currentfunc->tp
                ->btp->type == bt_unsignedlonglong)
            {
                size = currentfunc->tp->btp->type == bt_long ?  - BESZ_QWORD: BESZ_QWORD;
                ap = gen_expr(stmt->exp, FALSE, FALSE, size);
                do_extend(ap, size, F_DOUBLEREG);
            }
            else if (size > BESZ_QWORD)
            {
                retsize = BESZ_LDOUBLE;
                ap = gen_expr(stmt->exp, FALSE, FALSE, size);
                do_extend(ap, size, F_FREG);
            }
            else
            {
                returndreg = TRUE;
                retsize = BESZ_DWORD;
                ap = gen_expr(stmt->exp, FALSE, FALSE, size);
                do_extend(ap, size, F_DREG | F_VOL);
                if (!genning_inline && ap->preg != 0)
                {
                    gen_codes(op_move, size, makedreg(0), ap);
                    ap = makedreg(0);
                }
            }
        }
    }
    if (flag &1)
    {
        if (retlab !=  - 1)
            gen_label(retlab);
        if (flag &2)
            return ;
        if (currentfunc->value.classdata.eofdest)
        {
            if (returndreg)
            {
                if (currentfunc->tp->btp->type == bt_longlong || currentfunc
                    ->tp->btp->type == bt_unsignedlonglong)
                    gen_codes(op_move, BESZ_DWORD, makedreg(1), push);
                gen_codes(op_move, BESZ_DWORD, makedreg(0), push);
            }
            gen_expr(currentfunc->value.classdata.eofdest, TRUE, FALSE, 0);
            if (returndreg)
            {
                gen_codes(op_move, BESZ_DWORD, pop, makedreg(0));
                if (currentfunc->tp->btp->type == bt_longlong || currentfunc
                    ->tp->btp->type == bt_unsignedlonglong)
                    gen_codes(op_move, BESZ_DWORD, pop, makedreg(1));
            }

        }
        if ((conscount || try_block_list || currentfunc
            ->value.classdata.throwlist && currentfunc
            ->value.classdata.throwlist->data) && prm_xcept)
            call_library2("__RundownExceptBlock", 0);
		if (used_alloca)
		{
			AMODE *ap = gen_expr(allocaSP, FALSE, FALSE, 4);
			gen_codes(op_move, 4, ap, makeareg(7));
		}
        if ((!prm_linkreg || currentfunc->intflag) && (lc_maxauto))
        if (lc_maxauto > 8)
        {
            AMODE *ap = xalloc(sizeof(AMODE));
            ap->mode = am_indx;
            ap->offset = makeintnode(en_icon, lc_maxauto);
            ap->preg = 7;
            gen_codes(op_lea, 0, ap, makeareg(7));
        }
        else
            gen_codes(op_add, BESZ_DWORD, make_immed(lc_maxauto), makeareg(7));
        if (fsave_mask != 0)
            gen_codes(op_fmovem, BESZ_LDOUBLE, pop, make_mask(fsave_mask, 1));
        if (save_mask != 0)
            if (prm_coldfire)
        if (rmaskcnt > 1)
        {
            gen_codes(op_movem, BESZ_DWORD, mvma7, make_mask(save_mask, 0));
            mvn1 = xalloc(sizeof(struct enode));
            mvn1->nodetype = en_icon;
            mvn1->v.i = (rmaskcnt << 2);
            ap3 = xalloc(sizeof(struct amode));
            ap3->mode = am_indx;
            ap3->preg = 7;
            ap3->offset = mvn1;
            gen_codes(op_lea, BESZ_DWORD, ap3, mvma7i);
            freeop(ap3);
        }
        else
            gen_codes(op_move, BESZ_DWORD, pop, make_mask(save_mask, 0));
        else
            gen_codes(op_movem, BESZ_DWORD, pop, make_mask(save_mask, 0));
        if (prm_cplusplus && prm_xcept || prm_linkreg && !currentfunc->intflag
            && (currentfunc->tp->lst.head && currentfunc->tp->lst.head != (SYM*)
            - 1 || lc_maxauto) || !prm_smartframes || (currentfunc
            ->value.classdata.cppflags &PF_MEMBER) && !(currentfunc
            ->value.classdata.cppflags &PF_STATIC))
        {
            gen_codes(op_unlk, 0, makeareg(linkreg), 0);
        } if (currentfunc->intflag)
            gen_codes(op_rte, 0, 0, 0);
        else
        if (currentfunc->pascaldefn || currentfunc->isstdcall)
        {
            long retsize = 0;
            if (currentfunc->tp->lst.head && currentfunc->tp->lst.head != (SYM*)
                - 1)
            {
                retsize = currentfunc->paramsize;
                if (prm_linkreg)
                    retsize -= 8;
            }
            if (currentfunc->tp->btp && isstructured(currentfunc->tp->btp))
                retsize += 4;
            if (retsize)
            {
                if (prm_68020 || prm_68010)
                    gen_codes(op_rtd, 0, make_immed(retsize), 0);
                else
                {
                    ap = temp_addr();
                    freeop(ap);
                    gen_codes(op_move, BESZ_DWORD, pop, ap);
                    if (retsize > 8)
                    {
                        ap1 = xalloc(sizeof(AMODE));
                        ap1->mode = am_indx;
                        ap1->offset = makeintnode(en_icon, retsize);
                        ap1->preg = 7;
                        gen_codes(op_lea, 0, ap1, makeareg(7));
                    }
                    else
                        gen_codes(op_add, BESZ_DWORD, make_immed(retsize), makeareg(7));
                    ap->mode = am_ind;
                    gen_codes(op_jmp, 0, ap, 0);
                }
                return ;
            }
        }
        gen_codes(op_rts, 0, 0, 0);
    }
    else
    {
        if (retlab ==  - 1)
            retlab = nextlabel++;
        gen_codes(op_bra, 0, make_label(retlab), 0);
    }
}

//-------------------------------------------------------------------------

void gen_tryblock(void *val)
{
    AMODE *ap1 = xalloc(sizeof(AMODE));
    ap1->mode = am_indx;
    ap1->preg = linkreg;
    ap1->offset = makeintnode(en_icon,  - xceptoffs + 12); // ESP in XCEPTDATA
    // this is a little buggy, it doesn't push FP regs
    switch ((int)val)
    {
        case 0:
            // start of try block
            gen_move(BESZ_DWORD, makedreg(3), push);
            gen_move(BESZ_DWORD, makedreg(4), push);
            gen_move(BESZ_DWORD, makedreg(5), push);
            gen_move(BESZ_DWORD, makedreg(6), push);
            gen_move(BESZ_DWORD, makedreg(7), push);
            gen_move(BESZ_DWORD, ap1, push);
            gen_move(BESZ_DWORD, makeareg(7), ap1);
            break;
        case 1:
            //start of catch block
            gen_move(BESZ_DWORD, ap1, makeareg(7));
            gen_move(BESZ_DWORD, pop, ap1);
            gen_move(BESZ_DWORD, pop, makedreg(7));
            gen_move(BESZ_DWORD, pop, makedreg(6));
            gen_move(BESZ_DWORD, pop, makedreg(5));
            gen_move(BESZ_DWORD, pop, makedreg(4));
            gen_move(BESZ_DWORD, pop, makedreg(3));
            break;
        case 2:
            //end of try block
            gen_move(BESZ_DWORD, ap1, makeareg(7));
            gen_move(BESZ_DWORD, pop, ap1);
            gen_codes(op_add, BESZ_DWORD, make_immed(4 *5), makedreg(7));
            break;
    }
}

//-------------------------------------------------------------------------

void genstmt(SNODE *stmt)
/*
 *      genstmt will generate a statement and follow the next pointer
 *      until the block is generated.
 */
{
    while (stmt != 0)
    {
        switch (stmt->stype)
        {
            case st_case:
                if ((int)stmt->label >= 0)
                    gen_label((int)stmt->label);
                break;
            case st_block:
                genstmt(stmt->exp);
                break;
            case st_label:
                gen_label((int)stmt->label);
                break;
            case st_goto:
                gen_codes(op_bra, 0, make_label((int)stmt->label), 0);
                break;
            case st_tryblock:
                initstack(); // important!!!
                gen_tryblock(stmt->label);
                break;
            case st_expr:
                gen_void(stmt->exp);
                initstack();
                break;
            case st_return:
                genreturn(stmt, 0);
                break;
            case st_if:
                genif(stmt);
                break;
            case st_while:
                genwhile(stmt);
                break;
            case st_do:
                gendo(stmt);
                break;
            case st_for:
                gen_for(stmt);
                break;
            case st_line:
                gen_line(stmt);
                break;
            case st_continue:
                gen_codes(op_bra, 0, make_label(contlab), 0);
                break;
            case st_break:
                gen_codes(op_bra, 0, make_label(breaklab), 0);
                break;
            case st_switch:
                genxswitch(stmt);
                break;
            case st__genword:
                gen_genword(stmt);
                break;
            case st_asm:
                if (stmt->exp)
                    add_peep(stmt->exp);
                break;
            default:
                DIAG("unknown statement.");
                break;
        }
        stmt = stmt->next;
    }
}

//-------------------------------------------------------------------------

    void scppinit(void)
    /*
     * Call C++ reference variable and class initializers
     */
    {
        #ifdef XXXXX
            if (!strcmp(currentfunc->name, "_main"))
            {
                AMODE *ap1,  *ap2,  *ap3,  *ap4;
                int lbl = nextlabel++;
                initstack();
                ap1 = temp_addr();
                ap4 = xalloc(sizeof(AMODE));
                ap4->preg = ap1->preg;
                ap4->mode = am_ind;
                ap2 = set_symbol("CPPSTART", 0);
                ap3 = set_symbol("CPPEND", 0);
                gen_codes(op_lea, BESZ_DWORD, ap2, ap1);
                gen_label(lbl);
                gen_codes(op_move, BESZ_DWORD, ap1, push);
                gen_codes(op_jsr, BESZ_DWORD, ap4, 0);
                gen_codes(op_move, BESZ_DWORD, pop, ap1);
                gen_codes(op_add, BESZ_DWORD, make_immed(4), ap1);
                gen_codes(op_cmp, BESZ_DWORD, ap3, ap1);
                gen_codes(op_bhi, 0, make_label(lbl), 0);
                freeop(ap1);
            }
        #endif 
    }
    SYM *gen_mp_virtual_thunk(SYM *vsp)
    {
        LIST *v = mpthunklist;
        SYM *sp;
        AMODE *ap1,  *ap2;
        char buf[256];
        while (v)
        {
            sp = (SYM*)v->data;
            if (sp->value.i == vsp->value.classdata.vtabindex)
                if (isstructured(vsp->tp->btp) == isstructured(sp->tp->btp))
                    return sp;
            v = v->link;
        }
        global_flag++;
        sp = makesym(sc_static);
        sp->tp = vsp->tp;
        sp->value.i = vsp->value.classdata.vtabindex;
        sprintf(buf, "@$mpt$%d$%d", sp->value.i, isstructured(sp->tp->btp));
        sp->name = litlate(buf);
        sp->staticlabel = FALSE;
        v = xalloc(sizeof(LIST));
        v->data = sp;
        v->link = mpthunklist;
        mpthunklist = v;
        global_flag--;
        //   gen_codelab(sp) ;
        gen_virtual(sp, FALSE);
        ap1 = makedreg(7);
        ap1->mode = am_indx;
        ap1->offset = makeintnode(en_icon, isstructured(sp->tp->btp) ? 8 : 4);
        gen_codes(op_move, BESZ_DWORD, ap1, makedreg(0));
        ap1->preg = 0;
        ap1->mode = am_indx;
        if (sp->value.classdata.vtaboffs)
            ap1->offset = makeintnode(en_icon, sp->value.classdata.vtaboffs);
        else
            ap1->offset = makeintnode(en_icon, 0);
        gen_codes(op_move, BESZ_DWORD, ap1, makedreg(0));
        ap1->mode = am_ind;
        gen_codes(op_move, BESZ_DWORD, ap1, makedreg(0));
        ap1->offset = makeintnode(en_icon, sp->value.i);
        ap1->mode = am_indx;
        gen_codes(op_jmp, BESZ_DWORD, ap1, 0);
        flush_peep();
        gen_endvirtual(sp);
        return sp;
    }
    SYM *gen_vsn_virtual_thunk(SYM *func, int ofs)
    {
        SYM *sp;
        AMODE *ap1,  *ap2;
        char buf[256];
        global_flag++;
        sp = makesym(sc_static);
        sp->value.i = ofs;
        sprintf(buf, "@$vsn%s$%d", func->name, sp->value.i);
        sp->name = litlate(buf);
        sp->staticlabel = FALSE;
        sp->tp = func->mainsym; // strange use of the TP field here 
        global_flag--;
        gen_virtual(sp, FALSE);
        ap1 = makedreg(7);
        ap1->mode = am_indx;
        ap1->offset = makeintnode(en_icon, 4);
        gen_codes(op_add, BESZ_DWORD, make_immed( - ofs), ap1);
        ap2 = xalloc(sizeof(AMODE));
        ap2->offset = makenode(en_napccon, (void*)func, 0);
        ap2->mode = am_immed;
        gen_codes(op_bra, BESZ_DWORD, ap2, 0);
        flush_peep();
        gen_endvirtual(sp);
        return sp;

    }
void genfunc(SNODE *stmt)
/*
 *      generate a function body.
 */
{
    //int isconst = FALSE;
    cseg();
    currentfunc->gennedvirtfunc = (virtualfuncs || (currentfunc
        ->value.classdata.cppflags &(PF_HEADERFUNC | PF_INSTANTIATED)));
    if (currentfunc->storage_class == sc_global && !(currentfunc
        ->value.classdata.cppflags &PF_INLINE))
        if (!currentfunc->gennedvirtfunc)
            globaldef(currentfunc);
    returndreg = FALSE;
    retsize = 0;
    stackdepth = 0;
    contlab = breaklab =  - 1;
    retlab = nextlabel++;
    funcfloat = 0;
    if (funcstmt)
        gen_line(funcstmt);
    funcstmt = 0;
    #ifdef XXXXX
        if (stmt->stype == st_line)
        {
            gen_line(stmt);
            stmt = stmt->next;
        }
        if (!stmt)
        {
            if (currentfunc->gennedvirtfunc)
                gen_virtual(currentfunc, FALSE);
            else
                gen_codelab(currentfunc);
            gen_codes(op_rts, 0, 0, 0);
            flush_peep();
            if (currentfunc->gennedvirtfunc)
                gen_endvirtual(currentfunc);
            return ;
        }
    #endif 
        #ifdef XXXXX              
            if (currentfunc->value.classdata.cppflags &PF_CONSTRUCTOR)
            {
                isconst = TRUE;
            }
        #endif 
        if (currentfunc->gennedvirtfunc)
            gen_virtual(currentfunc, FALSE);
        else
        gen_codelab(currentfunc);
     /* name of function */

    opt1(stmt); /* push args & also subtracts SP */
        #ifdef XXXXX
            if (isconst)
            {
                SYM *psp = currentfunc->parentclass;
                if (currentfunc->value.classdata.basecons)
                    gen_void(currentfunc->value.classdata.basecons);
                if (psp->value.classdata.baseclass->vtabsp)
                {
                    CLASSLIST *l = psp->value.classdata.baseclass;
                    ENODE *thisn = copynode(thisenode);
                    ENODE *ts = makenode(en_nacon, psp
                        ->value.classdata.baseclass->vtabsp, 0);
                    ENODE *exp1,  *exp2;
                    thisn = makenode(en_a_ref, thisn, 0);
                    while (l)
                    {
                        if (l->flags &CL_VTAB)
                        {
                            exp1 = makenode(en_addstruc, ts, makeintnode
                                (en_icon, l->vtaboffs));
                            exp2 = makenode(en_addstruc, thisn, makeintnode
                                (en_icon, l->offset));
                            exp2 = makenode(en_a_ref, exp2, 0);
                            exp2 = makenode(en_assign, exp2, exp1);
                            initstack();
                            gen_expr(exp2, FALSE, TRUE, BESZ_DWORD);
                        }
                        l = l->link;
                    }
                }
            }
        #endif 
        if (prm_cplusplus)
            scppinit();
	if (used_alloca)
		{
			AMODE *ap = gen_expr(allocaSP, FALSE, FALSE, 4);
			gen_codes(op_move, 4, makeareg(7),ap);
		}
    initstack();
    genstmt(stmt);
        if (block_rundown)
            if (retsize)
        if (retsize > BESZ_DWORD)
        {
            if (retsize == BESZ_QWORD) {
                gen_codes(op_move, BESZ_DWORD, makedreg(0), push);
                gen_codes(op_move, BESZ_DWORD, makedreg(1), push);
                gen_void(block_rundown);
                gen_codes(op_move, BESZ_DWORD, pop, makedreg(1));
                gen_codes(op_move, BESZ_DWORD, pop, makedreg(0));
            } else {
                gen_codes(op_fmove, BESZ_LDOUBLE, makefreg(0), push);
                gen_void(block_rundown);
                gen_codes(op_fmove, BESZ_LDOUBLE, pop, makefreg(0));
            }
        }
        else
        {
            gen_codes(op_move, BESZ_DWORD, makedreg(0), push);
            gen_void(block_rundown);
            gen_codes(op_move, BESZ_DWORD, pop, makedreg(0));
        }
        else
            gen_void(block_rundown);
    if (funcendstmt)
        gen_line(funcendstmt);
    funcendstmt = 0;
    genreturn(0, 1);
        thissp->inreg = FALSE;
        thissp->value.i = 8;
    flush_peep();
        if (currentfunc->gennedvirtfunc)
            gen_endvirtual(currentfunc);
        if ((conscount || try_block_list || currentfunc
            ->value.classdata.throwlist && currentfunc
            ->value.classdata.throwlist->data) && prm_xcept)
            dumpXceptBlock(currentfunc);
}
