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
 * istmt.c
 *
 * channge the statement list to icode
 */
#include <stdio.h>
#include <limits.h>
#include "utype.h"
#include "lists.h"
#include "expr.h"
#include "c.h"
#include "iexpr.h"
#include "iopt.h"
#include "diag.h"

extern IMODE *structret_imode ;
extern int global_flag;
extern TABLE gsyms;
extern long nextlabel;
extern TYP stdfunc;
extern SYM *currentfunc;
extern int prm_cplusplus;
extern int prm_locsub, prm_globsub, prm_copyprop, prm_foldconst;
extern int prm_optcode, prm_optdead, prm_optlive;
extern BLOCKLIST *blockhead;
extern int blocknum;
extern QUAD *intermed_head;
extern TABLE tempsyms;
extern int firstlabel;
#ifdef DUMP_GCSE_INFO
    extern FILE *icdFile;
#endif 
/*

 * virtual registers
 */
IMODE rlink = 
{
    i_rlink, 0, 0
};
IMODE rsp = 
{
    i_rsp, 0, 0
};
IMODE rstruct = 
{
    i_rstruct, 0, 0
};
IMODE rstructstar = 
{
    i_rstructstar, 0, 0
};
IMODE rret = 
{
    i_rret, 0, 0
};
int tempnum;
static int diddef; /* True if did default */
/*
 * current label values 
 */
static int breaklab;
static int contlab;
int retlab;
static int deflab;

void genstmtini(void)
{
    rsp.size = rlink.size = ISZ_ADDR;
}

//-------------------------------------------------------------------------

IMODE *tempreg(int size, int mode)
/*
 * create a temporary register
 */
{
    SYM *sp;
    IMODE *ap;
    char buf[10];
    ap = xalloc(sizeof(IMODE));
    sp = makesym(sc_temp);
    sprintf(buf, "$$t%d", tempnum);
    sp->name = litlate(buf);
    sp->value.i = tempnum++;
    if (prm_locsub)
        insert(sp, &tempsyms);
    ap->offset = makenode(en_tempref, sp, 0);
    ap->size = size;
    if (mode)
    {
        ap->mode = i_immed;
        sp->imaddress = ap;
    }
    else
    {
        ap->mode = i_direct;
        sp->imvalue = ap;
    }
    return ap;
}

//-------------------------------------------------------------------------

IMODE *make_direct(int i)
/*
 *      make a direct reference to an immediate value.
 */
{
    return make_offset(makenode(en_icon, (char*)i, 0));
}

//-------------------------------------------------------------------------

IMODE *make_strlab(char *s)
/*
 *      generate a direct reference to a string label.
 */
{
    IMODE *ap;
    ap = xalloc(sizeof(IMODE));
    ap->size = ISZ_ADDR;
    ap->mode = i_direct;
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
    lab1 = contlab; /* save old continue label */
    contlab = nextlabel++; /* new continue label */
    if (stmt->s1 != 0)
     /* has block */
    {
        lab2 = breaklab; /* save old break label */
        breaklab = nextlabel++;
        gen_igoto(contlab);
        lab3 = nextlabel++;
        gen_label(lab3);
        genstmt(stmt->s1);
        gen_label(contlab);
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
    if (stmt->label != 0)
        gen_expr(stmt->label, F_NOVALUE, natural_size(stmt->label));

    gen_igoto(start_label);
    gen_label(loop_label);
    if (stmt->s1 != 0)
    {
        breaklab = exit_label;
        genstmt(stmt->s1);
    }
    gen_label(contlab);
    if (stmt->s2 != 0)
        gen_expr(stmt->s2, F_NOVALUE, natural_size(stmt->s2));
    gen_label(start_label);
    if (stmt->lst)
        gen_line(stmt->lst);
    if (stmt->exp != 0)
        truejp(stmt->exp, loop_label);
    else
        gen_igoto(loop_label);
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
    falsejp(stmt->exp, lab1);
    genstmt(stmt->s1);
    if (stmt->s2 != 0)
     /* else part exists */
    {
        gen_igoto(lab2);
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
	gen_label(looplab);
    if (stmt->s1 != 0)
    {
		breaklab = nextlabel++;
	    contlab = nextlabel++;
        genstmt(stmt->s1); /* generate body */
	    gen_label(contlab);
        truejp(stmt->exp, looplab);
        gen_label(breaklab);
    }
    else
    {
        genstmt(stmt->s1);
        truejp(stmt->exp, looplab);
    }
    breaklab = oldbreak;
    contlab = oldcont;
}

//-------------------------------------------------------------------------

void gen_genword(SNODE *stmt)
/*
 * generate data in the code seg
 */
{
    gen_icode(i_genword, 0, make_immed((int)stmt->exp), 0);
}

//-------------------------------------------------------------------------

IMODE *set_symbol(char *name, int isproc)
/*
 *      generate a call to a library routine.
 */
{
    SYM *sp;
    IMODE *result;
    sp = gsearch(name);
    if (sp == 0)
    {
        ++global_flag;
        sp = makesym(sc_external);
        sp->tp = &stdfunc;
        sp->name = name;
        sp->extflag = TRUE;
        insert(sp, &gsyms);
        --global_flag;
    }
    result = make_strlab(name);
    return result;
}

//-------------------------------------------------------------------------

IMODE *call_library(char *lib_name, int size)
/*
 *      generate a call to a library routine.
 */
{
    IMODE *result;
    result = set_symbol(lib_name, 1);
    gen_icode(i_gosub, 0, result, 0);
    if (size != 0)
    {
//        gen_icode(i_parmadj, 0, make_immed(size), 0);
    }
    result = tempreg(ISZ_UINT, 0);
    gen_icode(i_assn, result, &rret, 0);
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
        cs->ptrs = xalloc((cs->top - cs->bottom) * sizeof(struct cases));
        if (compact) {
            int i;
            for (i = cs->bottom; i < cs->top; i++)
                cs->ptrs[i - cs->bottom].label = cs->deflab;
        }
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

void gencompactswitch(SNODE *stmt, struct cases *cs)
{
    int tablab, curlab, i, size;
    IMODE *ap,  *ap2;
    ap = gen_expr(stmt->exp, F_VOL, ISZ_UINT);
    gen_icode2(i_coswitch, ap, make_immed(cs->count), make_immed(cs->top - cs->bottom), cs->deflab);
	gather_cases(stmt->s1,cs,TRUE);
    for (i = 0; i < cs->top - cs->bottom; i++)
		if (cs->ptrs[i].label != cs->deflab)
			gen_icode2(i_swbranch,0,make_immed(i+cs->bottom),0,cs->ptrs[i].label);
}
//-------------------------------------------------------------------------

void genxswitch(SNODE *stmt)
/*
 *      analyze and generate best switch statement.
 */
{
    int oldbreak;
    int a = 1;
    struct cases cs;
    oldbreak = breaklab;
    breaklab = nextlabel++;
    memset(&cs,0,sizeof(cs));
#if sizeof(LLONG_TYPE) == 4
    cs.top = INT_MIN;
    cs.bottom = INT_MAX;
#else
    cs.top = (a << 63); // LLONG_MIN
    cs.bottom = cs.top - 1; // LLONG_MAX
#endif
    cs.deflab = nextlabel++;
    count_cases(stmt->s1,&cs) ;
    cs.top++;
	gencompactswitch(stmt, &cs);
    genstmt(stmt->s1);
    if (!cs.diddef)
        gen_label(cs.deflab);
    gen_label(breaklab);
    breaklab = oldbreak;
}

//-------------------------------------------------------------------------

void genreturn(SNODE *stmt, int flag)
/*
 *      generate a return statement.
 */
{
    IMODE *ap,  *ap1;
    ENODE ep;
    int size;
    /* returns a value? */
    if (stmt != 0 && stmt->exp != 0)
    {
        if (currentfunc->tp->btp && isstructured(currentfunc->tp->btp))
        {
            size = currentfunc->tp->btp->size;
            ep.nodetype = en_ul_ref;
            ep.v.p[0] = stmt->exp;
            ap = gen_expr(&ep, F_VOL, ISZ_UINT);
        }
        else if (currentfunc->tp->btp && currentfunc->tp->btp->type ==
            bt_memberptr)
        {
            size = ISZ_UINT;
            ap = gen_expr(stmt->exp, F_VOL, ISZ_UINT);
        }
        else
        {
            size = natural_size(stmt->exp);
            ap = gen_expr(stmt->exp, 0, size);
            gen_icode(i_assn, &rret, ap, 0);
        }
    }
    /* create the return or a branch to the return
     * return is put at end of function...
     */
    if (flag)
    {
        if (retlab !=  - 1)
            gen_label(retlab);
		gen_icode(i_epilogue,0,0,0);
        if (currentfunc->intflag)
            gen_icode(i_rett, 0, 0, 0);
        else
            gen_icode(i_ret, 0, 0, 0);
    }
    else
    {
        if (retlab ==  - 1)
            retlab = nextlabel++;
        gen_igoto(retlab);
    }
}

//-------------------------------------------------------------------------

void gen_throw(TYP *tp, ENODE *exp)
{
    if (!tp || !exp)
    {
        call_library("@_RethrowException$qv", 0);
    }
    else
    {
        IMODE *ap1 = xalloc(sizeof(IMODE));
        ap1->mode = i_immed;
        ap1->offset = makenode(en_napccon, (void *)getxtsym(tp),0);
        gen_icode(i_parm, 0, exp, 0);
        gen_icode(i_parm, 0, ap1, 0);
        call_library("@_ThrowException$qpvpv", 8);
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
            case st_tryblock:
                DIAG("tryblock not implemented");
                break;
            case st_block:
                gen_icode2(i_blockstart, 0, 0, 0, (int)stmt->label);
                genstmt(stmt->exp);
                gen_icode2(i_blockend, 0, 0, 0, (int)stmt->label);
                break;
            case st_label:
                gen_label((int)stmt->label);
                break;
            case st_goto:
                gen_igoto((int)stmt->label);
                break;
            case st_throw:
                gen_throw(stmt->lst, stmt->exp);
                break;
            case st_expr:
                gen_expr(stmt->exp, F_NOVALUE, natural_size(stmt->exp));
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
                gen_igoto(contlab);
                break;
            case st_break:
                gen_igoto(breaklab);
                break;
            case st_switch:
                genxswitch(stmt);
                break;
            case st__genword:
                gen_genword(stmt);
                break;
            case st_passthrough:
                gen_asm(stmt);
                break;
            default:
                DIAG("unknown statement.");
                break;
        }
        stmt = stmt->next;
    }
}

//-------------------------------------------------------------------------

SYM *gen_mp_virtual_thunk(SYM *vsp)
{
#ifdef XXXXX

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
        gen_virtual(sp, FALSE);
        ap1 = makedreg(ESP);
        ap1->mode = am_indisp;
        ap1->offset = makeintnode(en_icon, isstructured(sp->tp->btp) ? 8 : 4);
        gen_codes(op_mov, BESZ_DWORD , makedreg(EAX), ap1);
        ap1->preg = 0;
        if (sp->value.classdata.vtaboffs)
            ap1->offset = makeintnode(en_icon, sp->value.classdata.vtaboffs);
        else
            ap1->offset = makeintnode(en_icon, 0);
        gen_codes(op_mov, BESZ_DWORD , makedreg(EAX), ap1);
        ap1->offset = makeintnode(en_icon, sp->value.i);
        gen_codes(op_jmp, BESZ_DWORD , ap1, 0);
        flush_peep();
        gen_endvirtual(sp);
        return sp;
#endif
		return 0;
}

//-------------------------------------------------------------------------

SYM *gen_vsn_virtual_thunk(SYM *func, int ofs)
{
#ifdef XXXXX
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
        ap1 = makedreg(ESP);
        ap1->mode = am_indisp;
        ap1->offset = makeintnode(en_icon, BESZ_DWORD );
        gen_codes(op_add, BESZ_DWORD , ap1, make_immed( - ofs));
        ap2 = xalloc(sizeof(AMODE));
        ap2->offset = makenode(en_napccon, (void*)func, 0);
        ap2->mode = am_immed;
        gen_codes(op_jmp, BESZ_DWORD , ap2, 0);
        flush_peep();
        gen_endvirtual(sp);
        return sp;
#endif
}

//-------------------------------------------------------------------------

void genfunc(SNODE *stmt)
/*
 *      generate a function body and dump the icode
 */
{
    retlab = contlab = breaklab =  - 1;
    intermed_head = 0;
    structret_imode = 0 ;
    tempnum = 0;
    blocknum = 0;
    blockhead = 0;
    addblock( - 1);
    memset(&tempsyms, 0, sizeof(tempsyms));
    //      firstlabel = nextlabel;
        if (prm_cplusplus)
    if (stmt->stype == st_line)
    {
        gen_line(stmt);
        stmt = stmt->next;
    }
	gen_icode(i_prologue,0,0,0);
    /* We do constant folding while we still have the enode list */
    if (prm_foldconst)
        constscan(stmt);
    /*          switchbottom = switchtop = 0;
     */

    /* Generate the icode */
    /* LCSE is done while code is generated */
    genstmt(stmt);
    genreturn(0, 1);
    #ifdef DUMP_GCSE_INFO
        if (icdFile)
            fprintf(icdFile, 
                "\n*************************FUNCTION %s********************************\n", currentfunc->name);
    #endif 

    /*
     * icode optimizations goes here.  Note that LCSE is done through
     * DAG construction during the actual construction of the blocks
     * so it is already done at this point.
     *
     * Order IS important!!!!!!!!! be careful!!!!!
     */
    /* Global opts */
//    flows_and_doms();
//    CalculatePointers();
//    CalculateEquations(intermed_head);
//    if (prm_globsub)
//        gcse();
//    if (prm_copyprop)
//        CopyPropagation();

    if (prm_optcode)
        commondelete(intermed_head);
     /* reuse common code frags */

    /* Local opts */
    if (prm_optdead)
        peep_icode(intermed_head);
     /* peepcode optimizer for icode */

//    if (prm_optlive)
//        LiveAnalysis();
    /* Code gen from icode */
    gen_strlab(currentfunc); /* name of function */
    rewrite_icode(); /* Translate to machine code & dump */
//    gcseRundown();
//    RundownUse();
//    RundownPointers();
}
