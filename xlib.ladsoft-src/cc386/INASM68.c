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
 * inline assembler (68k)  not implemented
 */
#include <stdio.h> 
#include <string.h>
#include "lists.h"
#include "expr.h" 
#include "c.h" 
#include "ccerr.h"
#include "gen68.h"

extern long framedepth, stackdepth, lc_maxauto;
extern int prm_68020, prm_68010, prm_smalldata;

extern TABLE labelsyms;
extern int asmline;
extern int lastch;
extern int prm_linkreg;
extern enum e_sym lastst;
extern short *lptr;
extern TABLE lsyms;
extern int nextlabel;
extern char lastid[];
extern int linkreg;

ASMREG *keyimage;
ASMREG *regimage;
int asmMask, asmRMask; /* unused */
static SYM *lastsym;
static enum e_op op;
static int theSize;

ASMREG reglst[] = 
{
    {
        "d0", am_dreg, 0, 1
    } , 
    {
        "d1", am_dreg, 1, 1
    }
    , 
    {
        "d2", am_dreg, 2, 1
    }
    , 
    {
        "d3", am_dreg, 3, 1
    }
    , 
    {
        "d4", am_dreg, 4, 1
    }
    , 
    {
        "d5", am_dreg, 5, 1
    }
    , 
    {
        "d6", am_dreg, 6, 1
    }
    , 
    {
        "d7", am_dreg, 7, 1
    }
    , 
    {
        "a0", am_areg, 0, 2
    }
    , 
    {
        "a1", am_areg, 1, 2
    }
    , 
    {
        "a2", am_areg, 2, 2
    }
    , 
    {
        "a3", am_areg, 3, 2
    }
    , 
    {
        "a4", am_areg, 4, 2
    }
    , 
    {
        "a5", am_areg, 5, 2
    }
    , 
    {
        "a6", am_areg, 6, 2
    }
    , 
    {
        "a7", am_areg, 7, 2
    }
    , 
    {
        "sp", am_areg, 7, 2
    }
    , 
    {
        "fp0", am_freg, 0, 3
    }
    , 
    {
        "fp1", am_freg, 1, 3
    }
    , 
    {
        "fp2", am_freg, 2, 3
    }
    , 
    {
        "fp3", am_freg, 3, 3
    }
    , 
    {
        "fp4", am_freg, 4, 3
    }
    , 
    {
        "fp5", am_freg, 5, 3
    }
    , 
    {
        "fp6", am_freg, 6, 3
    }
    , 
    {
        "fp7", am_freg, 7, 3
    }
    , 
    {
        "fpcr", am_fpcr, 0, 5
    }
    , 
    {
        "fpsr", am_fpsr, 0, 5
    }
    , 
    {
        "sfc", am_sfc, 0, 4
    }
    , 
    {
        "dfc", am_dfc, 0, 4
    }
    , 
    {
        "usp", am_usp, 0, 4
    }
    , 
    {
        "vbr", am_vbr, 0, 4
    }
    , 
    {
        "cacr", am_cacr, 0, 4
    }
    , 
    {
        "caar", am_caar, 0, 4
    }
    , 
    {
        "msp", am_msp, 0, 4
    }
    , 
    {
        "isp", am_isp, 0, 4
    }
    , 
    {
        "sr", am_sr, 0, 6
    }
    , 
    {
        "ccr", am_ccr, 0, 6
    }
    , 
    {
        "pc", am_pc, 0, 7
    }
    , 
    {
        0, 0, 0
    }
    , 
};

static AMODE *asm_amode(void);

static void asm_err(int errnum)
{
    *lptr = 0;
    lastch = ' ';
    generror(errnum, 0, 0);
    getsym();
}

//-------------------------------------------------------------------------

int need_010(void)
{
    if (!prm_68010 && !prm_68020)
    {
        asm_err(ERR_AINVSELPROC);
    }
    return prm_68020 || prm_68010;
}

//-------------------------------------------------------------------------

int need_020(void)
{
    if (!prm_68020)
    {
        asm_err(ERR_AINVSELPROC);
    }
    return prm_68020;
}

//-------------------------------------------------------------------------

static ENODE *asm_ident(void)
{
    ENODE *node = 0;
    char *nm;
    int fn = FALSE;
    if (lastst != id)
        asm_err(ERR_IDEXPECT);
    else
    {
        SYM *sp;
        ENODE *qnode = 0;
        nm = litlate(lastid);
        getsym();
        /* No such identifier */
        /* label, put it in the symbol table */
        if ((sp = gsearch(nm)) == 0 && (sp = search(nm, &labelsyms)) == 0)
        {
            sp = makesym(sc_ulabel);
            sp->name = nm;
            sp->tp = xalloc(sizeof(TYP));
            sp->tp->type = bt_unsigned;
            sp->tp->uflags = UF_USED;
            sp->tp->bits = sp->tp->startbit =  - 1;
            sp->value.i = nextlabel++;
            insert(sp, &lsyms);
            node = xalloc(sizeof(ENODE));
            node->nodetype = en_labcon;
            node->v.i = sp->value.i;
        }
        else
        {
            /* If we get here the symbol was already in the table
             */
            foundsp: sp->tp->uflags |= UF_USED;
            switch (sp->storage_class)
            {
                case sc_static:
                case sc_global:
                case sc_external:
                case sc_externalfunc:
                case sc_abs:
                    sp->mainsym->extflag = TRUE;
                        if (sp->value.classdata.gdeclare)
                            sp->value.classdata.gdeclare->mainsym->extflag =
                                TRUE;
                    if (sp->tp->type == bt_func || sp->tp->type == bt_ifunc)
                    {
                        /* make a function node */
                        node = makenode(en_napccon, sp, 0);
                        isfunc: 
                    }
                    else
                    /* otherwise make a node for a regular variable */
                        if (sp->absflag)
                            node = makenode(en_absacon, sp, 0);
                        else
                    if (sp->tp->type == bt_func || sp->tp->type == bt_ifunc)
                    {
                        fn = TRUE;
                        node = makenode(en_napccon, sp, 0);
                    }
                        else
                            if (sp->staticlabel)
                                node = makenode(en_nalabcon, sp, 0);
                            else
                                node = makenode(en_nacon, sp, 0);
                    break;
                case sc_const:
                    /* constants and enums */
                    node = makeintnode(en_icon, sp->value.i);
                    break;
                case sc_label:
                case sc_ulabel:
                    node = xalloc(sizeof(ENODE));
                    node->nodetype = en_labcon;
                    node->v.i = sp->value.i;
                    break;
                default:
                     /* auto and any errors */
                    if (sp->storage_class != sc_auto && sp->storage_class !=
                        sc_autoreg)
                    {
                        gensymerror(ERR_ILLCLASS2, sp->name);
                    }
                    else
                    {
                        /* auto variables */
                        if (sp->storage_class == sc_auto)
                            node = makenode(en_autocon, sp, 0);
                        else if (sp->storage_class == sc_autoreg)
                            node = makenode(en_autoreg, sp, 0);
                        if (fn)
                            goto isfunc;
                    }
                    break;
            }

            (node)->cflags = 0;
        }
        lastsym = sp;
    }
    return node;
}

//-------------------------------------------------------------------------

static ENODE *asm_label(void)
{
    char *nm = litlate(lastid);
    ENODE *node;
    SYM *sp;
    getsym();
    /* No such identifier */
    /* label, put it in the symbol table */
    if ((sp = search(lastid, &lsyms)) == 0)
    {
        sp = makesym(sc_label);
        sp->name = litlate(lastid);
        sp->tp = xalloc(sizeof(TYP));
        sp->tp->type = bt_unsigned;
        sp->tp->uflags = 0;
        sp->tp->bits = sp->tp->startbit =  - 1;
        sp->value.i = nextlabel++;
        insert(sp, &lsyms);
    }
    else
    {
        if (sp->storage_class == sc_label)
        {
            asm_err(ERR_DUPLABEL);
            return 0;
        }
        if (sp->storage_class != sc_ulabel)
        {
            asm_err(ERR_ALABEXPECT);
            return 0;
        }
        sp->storage_class = sc_label;
    }
    if (lastst != colon)
    {
        asm_err(ERR_ALABEXPECT);
        return 0;
    }
    getsym();
    node = xalloc(sizeof(ENODE));
    node->nodetype = en_labcon;
    node->v.i = sp->value.i;
    return node;
}

//-------------------------------------------------------------------------

static int asm_getsize(void)
{
    int theSize = 0;
    if (lastst == dot)
    {
        switch (lastch)
        {
            case 'b':
            case 'B':
                theSize = 1;
                break;
            case 'w':
            case 'W':
                theSize = 2;
                break;
            case 'l':
            case 'L':
                theSize = 4;
                break;
            case 's':
            case 'S':
                theSize = 7;
                break;
            case 'd':
            case 'D':
                theSize = 8;
                break;
            case 'x':
            case 'X':
                theSize = 10;
                break;
            default:
                asm_err(ERR_AINVSIZE);
                return 0;
        }
    }
    else
        return 0;

    getch();
    getsym();
    return theSize;
}

//-------------------------------------------------------------------------

static int needsize(int size)
{
    if (theSize != size && theSize != 0)
    {
        asm_err(ERR_AINVSIZE);
        return 0;
    }
    return 1;
}

//-------------------------------------------------------------------------

static int intsize(void)
{
    if (theSize > 4)
    {
        asm_err(ERR_AINVSIZE);
        return 0;
    }
    return 1;
}

//-------------------------------------------------------------------------

static int wordsize(void)
{
    if (theSize > 4 || theSize == 1)
    {
        asm_err(ERR_AINVSIZE);
        return 0;
    }
    return 1;
}

//-------------------------------------------------------------------------

static int need_freg(AMODE **v)
{

    *v = asm_amode();
    if (! *v || (*v)->mode != am_freg)
    {
        asm_err(ERR_ANEEDFP);
        return 0;
    }
    return 1;
}

//-------------------------------------------------------------------------

static int need_adreg(AMODE **v)
{
    *v = asm_amode();
    if (! *v || (*v)->mode != am_areg && (*v)->mode != am_dreg)
    {
        asm_err(ERR_ANEEDAD);
        return 0;
    }
    return 1;
}

//-------------------------------------------------------------------------

static int need_dreg(AMODE **v)
{
    *v = asm_amode();
    if (! *v || (*v)->mode != am_dreg)
    {
        asm_err(ERR_AILLADDRESS);
        return 0;
    }
    return 1;
}

//-------------------------------------------------------------------------

static int need_areg(AMODE **v)
{
    *v = asm_amode();
    if ((*v)->mode != am_areg)
    {
        asm_err(ERR_AILLADDRESS);
        return 0;
    }
    return 1;
}

//-------------------------------------------------------------------------

int asm_enterauto(ENODE *node, int inbr, int *rinbr, int *reg1, int *reg2, int
    *rt1, int *rt2)
{
    if (node && ((node)->nodetype == en_autocon || (node)->nodetype ==
        en_autoreg))
    {
        if (*reg1 >= 0 &&  *reg2 >= 0)
        {
            asm_err(ERR_AINVINDXMODE);
            return 0;
        }
        if (reg1 < 0)
        {
            *reg1 = 7;
            if (prm_linkreg)
                *reg1 = linkreg;
            *rt1 = am_areg;
        }
        else
        {
            if (inbr)
                *rinbr = 1;
            *reg2 = 7;
            if (prm_linkreg)
                *reg2 = linkreg;
            *rt2 = am_areg;
        }
        return 1;
    }
    return 2;
}

//-------------------------------------------------------------------------

static AMODE *asm_mem(void)
{
    int reg1 =  - 1, reg2 =  - 1, scale = 1, temp, inbr = 0, usebr = 0;
    int rinbr = 0, rg1t = 0, rg2t = 0, thisscale = 0;
    int scalesize = 0;
    ENODE *node = 0,  *node0 = 0,  *node1 = 0;
    AMODE *rv;
    int gotident = FALSE, autonode = FALSE;
    if (lastst == minus)
    {
        getsym();
        if (lastst != openpa)
        {
            asm_err(ERR_AMODEEXPECT);
            return 0;
        }
        getsym();
        if (lastst != kw_asmreg || keyimage->regtype != am_areg)
        {
            asm_err(ERR_AAREGEXPECT);
            return 0;
        }
        regimage = keyimage;
        getsym();
        if (lastst != closepa)
        {
            asm_err(ERR_AMODEEXPECT);
            return 0;
        }
        rv = xalloc(sizeof(AMODE));
        rv->mode = am_adec;
        rv->preg = regimage->regnum;
        getsym();
        return rv;
    }
    getsym();
    if (lastst == openbr)
    {
        if (!prm_68020)
        {
            asm_err(ERR_AINVSELPROC);
            return 0;
        }
        inbr = 1;
        usebr = 1;
        getsym();
    }
    while (TRUE)
    {
        int rg;
        regimage = keyimage;
        if (regimage)
            rg = regimage->regnum;
        switch (lastst)
        {
            case closebr:
                if (!inbr)
                {
                    asm_err(ERR_AINVINDXMODE);
                    return 0;
                }
                inbr = 0;
                getsym();
                break;

            case kw_asmreg:
                getsym();
                scalesize = asm_getsize();
                if (scalesize != 0 && scalesize != 2 && scalesize != 4)
                {
                    asm_err(ERR_AINVINDXMODE);
                    return 0;
                }

                if (lastst == star)
                {
                    getsym();
                    scale = intexpr(0);

                    thisscale = 1;
                }
                if (inbr)
                {
                    if (reg1 ==  - 1 && thisscale)
                    {
                        asm_err(ERR_AMODEEXPECT);
                        return 0;
                    }
                    if (reg2 !=  - 1)
                    {
                        asm_err(ERR_ATOOMANYREGS);
                        return 0;
                    }
                    if (reg1 ==  - 1)
                    {
                        if (regimage->regtype != am_areg && regimage->regtype 
                            != am_pc)
                        {
                            asm_err(ERR_AAREGEXPECT);
                            return 0;
                        }
                        reg1 = rg;
                        rg1t = regimage->regtype;
                    }
                    else
                    {
                        rinbr = 1;
                        reg2 = rg;
                        rg2t = regimage->regtype;
                    }
                }
                else
                {
                    if (reg1 ==  - 1 && !usebr)
                    {
                        if (thisscale)
                        {
                            if (reg2 !=  - 1)
                            {
                                asm_err(ERR_ATOOMANYREGS);
                                return 0;
                            }
                            reg2 = rg;
                            rg2t = regimage->regtype;

                        }
                        else
                        {
                            reg1 = rg;
                            rg1t = regimage->regtype;
                        }
                    }
                    else
                    {
                        if (reg2 !=  - 1)
                        {
                            asm_err(ERR_ATOOMANYREGS);
                            return 0;
                        }
                        rg2t = regimage->regtype;
                        reg2 = rg;
                    }
                }
                break;
            case plus:
            case minus:
            case iconst:
                if (usebr)
                {
                    if (inbr)
                    {
                        if (node0)
                        {
                            asm_err(ERR_AINVINDXMODE);
                            return 0;
                        }
                        node0 = makeintnode(en_icon, intexpr(0));
                    }
                    else
                    {
                        if (node1)
                        {
                            asm_err(ERR_AINVINDXMODE);
                            return 0;
                        }
                        node1 = makeintnode(en_icon, intexpr(0));
                    }
                }
                else
                    if (!node0)
                        node0 = makeintnode(en_icon, intexpr(0));
                    else
                        if (!node1)
                            node1 = makeintnode(en_icon, intexpr(0));
                        else
                {
                    asm_err(ERR_AINVINDXMODE);
                    return 0;
                }
                break;
            case id:
                if (gotident)
                {
                    asm_err(ERR_AINVINDXMODE);
                    return 0;
                }
                node = asm_ident();
                if (lastst == plus)
                {
                    getsym();
                    node = makenode(en_add, node, makeintnode(en_icon, intexpr
                        (0)));
                }
                gotident = TRUE;
                switch (asm_enterauto(node, inbr, &rinbr, &reg1, &reg2, &rg1t,
                    &rg2t))
                {
                case 0:
                    return 0;
                case 1:
                    autonode = TRUE;
                    break;
                case 2:
                    autonode = FALSE;
                    break;
                }
                if (usebr)
                {
                    if (inbr)
                    {
                        if (node0)
                        {
                            asm_err(ERR_AINVINDXMODE);
                            return 0;
                        }
                        node0 = node;
                    }
                            else
                    {
                        if (node1)
                        {
                            asm_err(ERR_AINVINDXMODE);
                            return 0;
                        }
                        node1 = node;
                    }
                }
                        else
                            if (!node0)
                                node0 = node;
                            else
                                if (!node1)
                                    node1 = node;
                                else
                {
                    asm_err(ERR_AINVINDXMODE);
                    return 0;
                }
                break;
            default:
                asm_err(ERR_AILLADDRESS);
                return 0;
        }
        if (lastst == begin)
            break;
        if (lastst == closepa)
        {
            getsym();
            break;
        }
        if (lastst == closebr)
            continue;
        if (lastst != comma)
        {
            asm_err(ERR_AINVINDXMODE);
            return 0;
        }
        getsym();
    }
    if (rg2t == am_pc && scale != 1)
    {
        asm_err(ERR_AINVINDXMODE);
        return 0;
    }
    /* swap to get dreg in second pos */
    if (((reg1 !=  - 1 && rg1t == am_dreg) && reg2 !=  - 1 && scale == 1) ||
        rg2t == am_pc)
    {
        int rg3t = rg1t;
        int reg3 = reg1;
        reg2 = reg1;
        rg2t = rg1t;
        reg1 = reg3;
        rg1t = rg3t;
    }
    if (!prm_68020)
    {
        if (reg1 ==  - 1 && reg2 ==  - 1 && (node1 || !node0))
        {
            asm_err(ERR_AINVSELPROC);
            return 0;
        }
        if (reg1 ==  - 1 && reg2 !=  - 1 && (rg2t != am_areg || scale != 1))
        {
            asm_err(ERR_AINVSELPROC);
            return 0;
        }
    }
    if (reg1 ==  - 1 && reg2 ==  - 1 && !node0)
    {
        asm_err(ERR_AINVINDXMODE);
        return 0;
    }

    if (scale != 1 && scale != 2 && scale != 4 && scale != 8)
    {
        asm_err(ERR_ASCALE);
        return 0;
    }
    if (scale == 1)
        scale = 0;
    else if (scale == 2)
        scale = 1;
    else if (scale == 4)
        scale = 2;
    else if (scale == 8)
        scale = 3;
    if (rg1t == am_dreg)
    {
        asm_err(ERR_AAREGEXPECT);
        return 0;
    }
    if (node1 && !usebr)
    {
        asm_err(ERR_AINVINDXMODE);
        return 0;
    }
    rv = xalloc(sizeof(AMODE));
    if (node1)
    {
        rv->offset = node1;
    }
    rv->preg = reg1;
    rv->sreg = reg2;
    rv->scale = scale;
    if (usebr)
    {
        if (!node0)
            rv->offset = makeintnode(en_icon, 0);
        else
            rv->offset = node0;
        if (!node1)
            rv->offset2 = makeintnode(en_icon, 0);
        else
            rv->offset2 = node1;
        rv->sz = scalesize;
        if (rinbr)
        {
            if (reg1 ==  - 1 && reg2 ==  - 1)
                rv->mode = am_iimem;
            else
            if (reg1 !=  - 1)
            {
                if (reg2 !=  - 1)
                {
                    if (rg1t == am_pc)
                    {
                        if (rg2t == am_areg)
                            rv->mode = am_iiprepca;
                        else
                            rv->mode = am_iiprepcd;
                    }
                    else
                        if (rg2t == am_areg)
                            rv->mode = am_iiprea;
                        else
                            rv->mode = am_iipred;

                }
                else
                    if (rg1t == am_pc)
                        rv->mode = am_iiprepca;
                    else
                        rv->mode = am_iiprea;
            }
            else
                if (rg2t == am_areg)
                    rv->mode = am_iiprea;
                else
                    rv->mode = am_iipred;
        }
        else
        {
            if (reg1 ==  - 1 && reg2 ==  - 1)
                rv->mode = am_iimem;
            else
            if (reg1 !=  - 1)
            {
                if (reg2 !=  - 1)
                {
                    if (rg1t == am_pc)
                    {
                        if (rg2t == am_areg)
                            rv->mode = am_iipostpca;
                        else
                            rv->mode = am_iipostpcd;
                    }
                    else
                        if (rg2t == am_areg)
                            rv->mode = am_iiposta;
                        else
                            rv->mode = am_iipostd;
                }
                else
                    if (rg1t == am_pc)
                        rv->mode = am_iipostpca;
                    else
                        rv->mode = am_iiposta;
            }
            else
                if (rg2t == am_areg)
                    rv->mode = am_iiposta;
                else
                    rv->mode = am_iipostd;
        }
    }
    else
    {
        if (reg1 !=  - 1)
        {
            if (reg2 !=  - 1)
            {
                if (!node0)
                    rv->offset = makeintnode(en_icon, 0);
                else
                    rv->offset = node0;
                if (rg1t == am_pc)
                {
                    if (rg2t == am_areg)
                        rv->mode = am_pcindxaddr;
                    else
                        rv->mode = am_pcindxdata;
                }
                else
                    if (rg2t == am_areg)
                        rv->mode = am_baseindxaddr;
                    else
                        rv->mode = am_baseindxdata;
                rv->sz = scalesize;
                if (rv->mode == am_immed && (rv->offset->v.i &0xffff0000) &&
                    !prm_68020)
                    generror(ERR_AINVSELPROC, 0, 0);
            }
            else
            {
                if (!node0)
                {
                    rv->offset = makeintnode(en_icon, 0);
                    if (rg1t == am_pc)
                    {
                        rv->mode = am_pcindx;
                    }
                    else
                    if (lastst == plus)
                    {
                        getsym();
                        rv->mode = am_ainc;
                    }
                    else
                        rv->mode = am_ind;
                }
                else
                {
                    rv->offset = node0;
                    if (rg1t == am_pc)
                        rv->mode = am_pcindx;
                    else
                    {
                        rv->mode = am_indx;
                    }
                    if (rv->mode == am_immed && (rv->offset->v.i &0xffff0000)
                        && !prm_68020)
                        generror(ERR_AINVSELPROC, 0, 0);
                }
            }
        }
        else if (reg2 !=  - 1)
        {
            if (!node0)
                rv->offset = makeintnode(en_icon, 0);
            else
                rv->offset = node0;
            if (rg2t == am_areg)
                rv->mode = am_baseindxaddr;
            else
                rv->mode = am_baseindxdata;
            rv->sz = scalesize;
            if (rv->mode == am_immed && (rv->offset->v.i &0xffff0000) &&
                !prm_68020)
                generror(ERR_AINVSELPROC, 0, 0);

        }
        else
        {
            rv->mode = am_adirect;
            rv->preg = asm_getsize();
            rv->offset = node0;
            if (!rv->length)
                if (prm_smalldata)
                    rv->length = 2;
                else
                    rv->length = 4;
        }
    }
    return rv;
}

//-------------------------------------------------------------------------

static AMODE *asm_movemarg(AMODE *rv)
{
    AMODE old =  *rv;
    int start, val = 0, i;
    if (rv->mode == am_freg)
    {
        rv->mode = am_fmask;
        while (TRUE)
        {
            if (lastst != minus)
            {
                val |= 1 << old.preg;
                if (lastst != divide)
                    break;
                getsym();
                if (lastst != kw_asmreg)
                {
                    asm_err(ERR_ANEEDFP);
                    return 0;
                }
                regimage = keyimage;
                old.preg = regimage->regnum;
                old.mode = regimage->regtype;
                getsym();
            }
            else
            {
                getsym();
                start = old.preg;
                if (lastst != kw_asmreg)
                {
                    asm_err(ERR_ANEEDFP);
                    return 0;
                }
                regimage = keyimage;
                old.preg = regimage->regnum;
                old.mode = regimage->regtype;
                getsym();
                if (old.mode != am_freg)
                {
                    asm_err(ERR_ANEEDFP);
                    return 0;
                }
                if (start <= old.preg)
                    for (i = start; i <= old.preg; i++)
                        val |= 1 << i;
            }
        }
    }
    else
    {
        rv->mode = am_mask;
        rv->offset = 0;
        while (TRUE)
        {
            if (lastst != minus)
            {
                if (old.mode == am_areg)
                    val |= 0x100 << old.preg;
                else
                    val |= 1 << old.preg;
                if (lastst != divide)
                    break;
                getsym();
                if (lastst != kw_asmreg)
                {
                    asm_err(ERR_ANEEDAD);
                    return 0;
                }
                regimage = keyimage;
                old.preg = regimage->regnum;
                old.mode = regimage->regtype;
                getsym();
                if (old.mode != am_dreg && old.mode != am_areg)
                {
                    asm_err(ERR_ANEEDAD);
                    return 0;
                }
            }
            else
            {
                int modetype = old.mode;
                getsym();
                start = old.preg;
                if (lastst != kw_asmreg)
                {
                    asm_err(ERR_ANEEDAD);
                    return 0;
                }
                regimage = keyimage;
                old.preg = regimage->regnum;
                old.mode = regimage->regtype;
                getsym();
                if (old.mode != am_dreg && old.mode != am_areg)
                {
                    asm_err(ERR_ANEEDAD);
                    return 0;
                }
                if (modetype != old.mode)
                {
                    asm_err(ERR_AREGMISMATCH);
                    return 0;
                }
                if (start <= old.preg)
                    for (i = start; i <= old.preg; i++)
                        if (old.mode == am_areg)
                            val |= 0x100 << i;
                        else
                            val |= 1 << i;
            }
        }
    }
    rv->offset = (void*)val;
    return rv;

}

//-------------------------------------------------------------------------

static AMODE *asm_amode(void)
{
    AMODE *rv = xalloc(sizeof(AMODE));
    int sz = 0, seg = 0;
    lastsym = 0;
    switch (lastst)
    {
        case kw_asmreg:
            regimage = keyimage;
            rv->preg = regimage->regnum;
            rv->mode = regimage->regtype;
            sz = regimage->size;
            getsym();
            if (lastst == divide || lastst == minus)
            {
                if (!asm_movemarg(rv))
                {
                    asm_err(ERR_AILLADDRESS);
                    return 0;
                }
            }
            else if (lastst == colon)
            {
                AMODE *temp;
                getsym();
                if (rv->mode != am_dreg)
                {
                    asm_err(ERR_AILLADDRESS);
                    return 0;
                }
                if (!need_dreg(&temp))
                    return 0;
                rv->mode = am_divsl;
                rv->sreg = temp->preg;
            }
            break;
        case openpa:
        case minus:
            rv = asm_mem();
            break;
        case id:
            doid: rv->mode = am_immed;
            rv->offset = asm_ident();
            if (rv->offset->nodetype == en_autocon || rv->offset->nodetype ==
                en_autoreg)
            {
                asm_err(ERR_AUSELEA);
                return 0;
            }
            break;
        case hash:
            getsym();
            if (lastst == id)
                goto doid;
            if (lastst != minus && lastst != iconst && lastst != lconst &&
                lastst != luconst && lastst != cconst)
            {
                asm_err(ERR_AMODEEXPECT);
                return 0;
            }
            // FALL THROUGH
        case iconst:
        case iuconst:
        case lconst:
        case luconst:
        case cconst:
            rv->mode = am_immed;
            rv->offset = makeintnode(en_icon, intexpr(0));
            ;
            break;
        default:
            asm_err(ERR_AMODEEXPECT);
            return 0;
    }
    if (rv)
        rv->length = theSize;
    return rv;
}

//-------------------------------------------------------------------------

static AMODE *getbf(void)
{
    AMODE *rv = xalloc(sizeof(AMODE));
    if (lastst != begin)
        return 0;
    getsym();
    if (lastst == kw_asmreg)
    {
        rv->mode = am_bfreg;
        if (keyimage->regtype != am_dreg)
            return 0;
        rv->preg = keyimage->regnum;
        getsym();
        if (lastst != colon)
            return 0;
        getsym();
        if (lastst != kw_asmreg)
            return 0;
        if (keyimage->regtype != am_dreg)
            return 0;
        rv->sreg = keyimage->regnum;
        getsym();
        if (lastst != end)
            return 0;
        getsym();
    }
    else
    {
        rv->mode = am_bf;
        rv->preg = intexpr(0);
        if (lastst != colon)
            return 0;
        getsym();
        rv->sreg = intexpr(0);
        if (lastst != end)
            return 0;
        getsym();
    }
    return rv;
}

//-------------------------------------------------------------------------

enum e_op asm_op(void)
{
    int op;
    if (lastst != kw_asminst)
    {
        asm_err(ERR_AINVOP);
        return  - 1;
    } op = keyimage->regtype;
    getsym();
    return op;
}

//-------------------------------------------------------------------------

static OCODE *make_ocode(AMODE *ap1, AMODE *ap2, AMODE *ap3)
{
    OCODE *o = xalloc(sizeof(OCODE));
    o->oper1 = ap1;
    o->oper2 = ap2;
    o->oper3 = ap3;
    return o;
}

//-------------------------------------------------------------------------

static int noincdec(AMODE *ap)
{
    if (ap->mode == am_ainc || ap->mode == am_adec)
    {
        asm_err(ERR_AILLADDRESS);
        return 0;
    }
    return 1;
}

//-------------------------------------------------------------------------

static int dreg(AMODE *ap)
{
    if (ap->mode != am_dreg)
    {
        asm_err(ERR_AILLADDRESS);
        return 0;
    }
    return 1;
}

//-------------------------------------------------------------------------

static int vermode(AMODE *ap, int dok, int aok, int pcok, int immok)
{
    switch (ap->mode)
    {
        case am_dreg:
            if (dok)
                return 1;
            break;
        case am_areg:
            if (aok)
                return 1;
            break;
        case am_pcindx:
        case am_pcindxaddr:
        case am_pcindxdata:
        case am_iiprepca:
        case am_iiprepcd:
        case am_iipostpca:
        case am_iipostpcd:
            if (pcok)
                return 1;
            break;
        case am_immed:
            if (immok)
                return 1;
            break;
        case am_baseindxaddr:
        case am_baseindxdata:
        case am_iimem:
        case am_iiprea:
        case am_iipred:
        case am_iiposta:
        case am_iipostd:
        case am_indx:
        case am_ainc:
        case am_adec:
        case am_direct:
        case am_adirect:
        case am_ind:
            return 1;
        default:
            break;
    }
    asm_err(ERR_AILLADDRESS);
    return 0;
}

//-------------------------------------------------------------------------

static OCODE *ope_negbcd(void)
{
    AMODE *ap1,  *ap2;
    if (theSize && theSize != 1)
    {
        asm_err(ERR_AINVSIZE);
        return 0;
    }
    ap1 = asm_amode();
    if (!ap1)
        return (OCODE*) - 1;
    needpunc(comma, 0);
    ap2 = asm_amode();
    if (!ap2)
        return (OCODE*) - 1;
    if (ap1->mode != ap2->mode || (ap1->mode != am_dreg && ap1->mode != am_adec)
        )
        return (OCODE*) - 1;
    return make_ocode(ap1, ap2, 0);
}

//-------------------------------------------------------------------------

static OCODE *ope_addx(void)
{
    AMODE *ap1,  *ap2;
    if (!intsize())
    {
        return 0;
    }
    if (!theSize)
        theSize = 2;
    ap1 = asm_amode();
    if (!ap1)
        return (OCODE*) - 1;
    needpunc(comma, 0);
    ap2 = asm_amode();
    if (!ap2)
        return (OCODE*) - 1;
    if (ap1->mode != ap2->mode || (ap1->mode != am_dreg && ap1->mode != am_adec)
        )
        return (OCODE*) - 1;
    return make_ocode(ap1, ap2, 0);
}

//-------------------------------------------------------------------------

static OCODE *ope_math(void)
{
    AMODE *ap1,  *ap2;
    if (!intsize())
        return 0;
    if (!theSize)
        theSize = 2;
    ap1 = asm_amode();
    if (!ap1)
        return (OCODE*) - 1;
    needpunc(comma, 0);
    ap2 = asm_amode();
    if (!ap2)
        return (OCODE*) - 1;
    if (ap2->mode == am_areg)
        if (!wordsize())
            return 0;
    if (ap2->mode == am_dreg || ap2->mode == am_areg)
    {
        if (!vermode(ap1, TRUE, TRUE, TRUE, TRUE))
            return 0;
    }
    else if (ap1->mode == am_dreg || ap1->mode == am_immed)
    {
        if (!vermode(ap2, TRUE, TRUE, FALSE, FALSE))
            return 0;
    }
    else
    {
        return (OCODE*) - 1;
    }
    return make_ocode(ap1, ap2, 0);

}

//-------------------------------------------------------------------------

static OCODE *ope_amath(void)
{
    AMODE *ap1,  *ap2;
    if (!wordsize())
        return 0;
    ap1 = asm_amode();
    if (!ap1)
        return (OCODE*) - 1;
    needpunc(comma, 0);
    ap2 = asm_amode();
    if (!ap2)
        return (OCODE*) - 1;
    if (ap2->mode != am_areg)
    {
        asm_err(ERR_AAREGEXPECT);
        return 0;
    }
    if (!vermode(ap1, TRUE, TRUE, TRUE, TRUE))
        return 0;
    return make_ocode(ap1, ap2, 0);
}

//-------------------------------------------------------------------------

static OCODE *ope_imath(void)
{
    AMODE *ap1,  *ap2;
    if (!intsize())
        return 0;
    if (!theSize)
        theSize = 2;
    ap1 = asm_amode();
    if (!ap1)
        return (OCODE*) - 1;
    needpunc(comma, 0);
    ap2 = asm_amode();
    if (!ap2)
        return (OCODE*) - 1;
    if (ap1->mode != am_immed)
    {
        return (OCODE*) - 1;
    }
    if (!vermode(ap2, TRUE, FALSE, FALSE, FALSE))
        return 0;
    return make_ocode(ap1, ap2, 0);
}

//-------------------------------------------------------------------------

static OCODE *ope_qmath(void)
{
    OCODE *oc = ope_imath();
    if (!oc)
        return (OCODE*) - 1;
    if (!intsize())
        return 0;
    if (oc->oper1->offset->v.i < 1 || oc->oper1->offset->v.i > 8)
    {
        return (OCODE*) - 1;
    }
    return oc;
}

//-------------------------------------------------------------------------

static OCODE *ope_log(void)
{
    AMODE *ap1,  *ap2;
    if (!intsize())
        return 0;
    if (!theSize)
        theSize = 2;
    ap1 = asm_amode();
    if (!ap1)
        return (OCODE*) - 1;
    needpunc(comma, 0);
    ap2 = asm_amode();
    if (!ap2)
        return (OCODE*) - 1;
    if (ap2->mode == am_dreg)
    {
        if (!vermode(ap1, TRUE, FALSE, TRUE, TRUE))
            return 0;
    }
    else if (ap1->mode == am_dreg || ap1->mode == am_immed)
    {
        if (!vermode(ap2, FALSE, FALSE, FALSE, FALSE))
            return 0;
    }
    else
    {
        return (OCODE*) - 1;
    }
    return make_ocode(ap1, ap2, 0);
}

//-------------------------------------------------------------------------

static OCODE *ope_ilog(void)
{
    AMODE *ap1,  *ap2;
    if (!intsize())
        return 0;
    if (!theSize)
        theSize = 2;
    ap1 = asm_amode();
    if (!ap1)
        return (OCODE*) - 1;
    needpunc(comma, 0);
    ap2 = asm_amode();
    if (!ap2)
        return (OCODE*) - 1;
    if (ap1->mode == am_immed)
    {
        if (ap2->mode != am_ccr && ap2->mode != am_sr)
            if (!vermode(ap2, TRUE, FALSE, TRUE, TRUE))
                return 0;
    }
    else
    {
        return (OCODE*) - 1;
    }
    return make_ocode(ap1, ap2, 0);
}

//-------------------------------------------------------------------------

static OCODE *ope_shift(void)
{
    AMODE *ap1,  *ap2;
    ap1 = asm_amode();
    if (!ap1)
        return (OCODE*) - 1;
    if (!theSize)
        theSize = 2;
    if (lastst == comma)
    {
        getsym();
        if (!intsize())
            return 0;
        ap2 = asm_amode();
        if (!ap2)
            return (OCODE*) - 1;
        if (ap2->mode != am_dreg || (ap1->mode != am_dreg && ap1->mode !=
            am_immed))
        {
            return (OCODE*) - 1;

        }
        return make_ocode(ap1, ap2, 0);

    }
    else
    {
        if (!vermode(ap1, FALSE, FALSE, FALSE, FALSE))
            return 0;
        if (theSize != 2 && theSize != 0)
            asm_err(ERR_AINVSIZE);
        return make_ocode(ap1, 0, 0);
    }
}

//-------------------------------------------------------------------------

static OCODE *ope_bra(void)
{
    ENODE *node = asm_ident();
    AMODE *ap1;
    if (!node)
        return (OCODE*) - 1;
    ap1 = xalloc(sizeof(AMODE));
    ap1->mode = am_direct;
    ap1->offset = node;
    return make_ocode(ap1, 0, 0);
}

//-------------------------------------------------------------------------

static OCODE *ope_bit(void)
{
    AMODE *ap1,  *ap2;
    ap1 = asm_amode();
    if (!ap1)
        return (OCODE*) - 1;
    if (!theSize)
        theSize = 2;
    needpunc(comma, 0);
    ap2 = asm_amode();
    if (!ap2)
        return (OCODE*) - 1;
    if (ap1->mode != am_dreg && ap1->mode != am_immed)
    {
        return (OCODE*) - 1;
    }
    else
        if (!vermode(ap2, TRUE, FALSE, FALSE, FALSE))
            return 0;

    if (ap2->mode == am_dreg)
        needsize(4);
    else
        needsize(1);
    return make_ocode(ap1, ap2, 0);

}

//-------------------------------------------------------------------------

static OCODE *ope_bf1(void)
{
    AMODE *ap1,  *ap2;
    if (!need_020())
        return 0;
    ap1 = asm_amode();
    if (!ap1)
        return (OCODE*) - 1;

    if (!vermode(ap1, TRUE, FALSE, FALSE, FALSE))
        return 0;
    if (!noincdec(ap1))
        return 0;
    ap2 = getbf();
    if (!ap2)
        return (OCODE*) - 1;
    if (theSize)
    {
        asm_err(ERR_AINVSIZE);
        return 0;
    }
    return make_ocode(ap1, ap2, 0);
}

//-------------------------------------------------------------------------

static OCODE *ope_bf2(void)
{
    AMODE *ap1,  *ap2,  *ap3;
    if (!need_020())
        return 0;
    needsize(0);
    ap1 = asm_amode();
    if (!ap1)
        return (OCODE*) - 1;
    if (!vermode(ap1, TRUE, FALSE, FALSE, FALSE))
        return 0;
    if (!noincdec(ap1))
        return 0;
    if (!(ap3 = getbf()))
        return (OCODE*) - 1;
    needpunc(comma, 0);
    ap2 = asm_amode();
    if (!dreg(ap2))
        return 0;
    if (theSize)
    {
        asm_err(ERR_AINVSIZE);
        return 0;
    }
    return make_ocode(ap1, ap3, ap2);
}

//-------------------------------------------------------------------------

static OCODE *ope_bf3(void)
{
    AMODE *ap1,  *ap2,  *ap3;
    if (!need_020())
        return 0;
    needsize(0);
    ap2 = asm_amode();
    if (!ap2)
        return (OCODE*) - 1;
    if (!dreg(ap2))
        return 0;
    needpunc(comma, 0);
    ap1 = asm_amode();
    if (!ap1)
        return (OCODE*) - 1;
    if (!vermode(ap1, TRUE, FALSE, FALSE, FALSE))
        return 0;
    if (!noincdec(ap1))
        return 0;
    if (!(ap3 = getbf()))
        return (OCODE*) - 1;
    if (theSize)
    {
        asm_err(ERR_AINVSIZE);
        return 0;
    }
    return make_ocode(ap2, ap1, ap3);
}

//-------------------------------------------------------------------------

static OCODE *ope_ead(void)
{
    AMODE *ap1,  *ap2;
    if (prm_68020)
    {
        if (!wordsize())
            return 0;
    }
    else
        if (!needsize(2))
            return 0;
    if (!theSize)
        theSize = 2;
    ap1 = asm_amode();
    if (!ap1)
        return (OCODE*) - 1;
    if (!vermode(ap1, TRUE, FALSE, TRUE, TRUE))
        return 0;
    needpunc(comma, 0);
    ap2 = asm_amode();
    if (!ap2)
        return (OCODE*) - 1;
    if (!dreg(ap2))
        return 0;
    return make_ocode(ap1, ap2, 0);
}

//-------------------------------------------------------------------------

static OCODE *ope_ear(void)
{
    AMODE *ap1,  *ap2;
    if (!need_020() || !intsize())
        return 0;
    if (!theSize)
        theSize = 4;
    ap1 = asm_amode();
    if (!ap1)
        return (OCODE*) - 1;
    if (!vermode(ap1, FALSE, FALSE, TRUE, FALSE))
        return 0;
    needpunc(comma, 0);
    ap2 = asm_amode();
    if (!ap2)
        return (OCODE*) - 1;
    if (ap2->mode != am_dreg && ap2->mode != am_areg)
    {
        return (OCODE*) - 1;
    }
    return make_ocode(ap1, ap2, 0);
}

//-------------------------------------------------------------------------

static OCODE *ope_ea(void)
{
    AMODE *ap1;
    if (!intsize())
        return 0;
    if (!theSize)
        theSize = 2;
    ap1 = asm_amode();
    if (!ap1)
        return (OCODE*) - 1;
    if (!vermode(ap1, TRUE, FALSE, FALSE, FALSE))
        return 0;
    return make_ocode(ap1, 0, 0);
}

//-------------------------------------------------------------------------

static OCODE *ope_jea(void)
{
    AMODE *ap1;
    if (!intsize())
        return 0;
    needsize(0);
    ap1 = asm_amode();
    if (!ap1)
        return (OCODE*) - 1;
    if (!vermode(ap1, FALSE, FALSE, TRUE, FALSE))
        return 0;
    if (ap1->mode == am_ainc || ap1->mode == am_adec)
    {
        asm_err(ERR_AINVINDXMODE);
        return 0;
    }
    return make_ocode(ap1, 0, 0);
}

//-------------------------------------------------------------------------

static OCODE *ope_cmp(void)
{
    AMODE *ap1,  *ap2;

    if (!intsize())
        return 0;
    if (!theSize)
        theSize = 2;
    ap1 = asm_amode();
    if (!ap1)
        return (OCODE*) - 1;
    if (!vermode(ap1, TRUE, TRUE, TRUE, TRUE))
        return 0;
    if (ap1->mode == am_areg)
        if (!wordsize())
            return 0;
    needpunc(comma, 0);
    ap2 = asm_amode();
    if (!ap2)
        return (OCODE*) - 1;
    if (!dreg(ap2))
        return 0;
    return make_ocode(ap1, ap2, 0);
}

//-------------------------------------------------------------------------

static OCODE *ope_eaa(void)
{
    AMODE *ap1,  *ap2;
    if (!wordsize())
        return 0;
    if (!theSize)
        theSize = 2;
    ap1 = asm_amode();
    if (!ap1)
        return (OCODE*) - 1;
    if (!vermode(ap1, TRUE, TRUE, TRUE, TRUE))
        return 0;
    needpunc(comma, 0);
    ap2 = asm_amode();
    if (!ap2)
        return (OCODE*) - 1;
    if (ap2->mode != am_areg)
    {
        return (OCODE*) - 1;
    }
    return make_ocode(ap1, ap2, 0);
}

//-------------------------------------------------------------------------

static OCODE *ope_tea(void)
{
    AMODE *ap1,  *ap2;
    if (!intsize())
        return 0;
    if (!theSize)
        theSize = 2;
    ap1 = asm_amode();
    if (!ap1)
        return (OCODE*) - 1;
    if (!vermode(ap1, TRUE, TRUE, TRUE, TRUE))
        return 0;
    return make_ocode(ap1, 0, 0);
}

//-------------------------------------------------------------------------

static OCODE *ope_lea(void)
{
    AMODE *ap1,  *ap2;
    if (!wordsize())
        return 0;
    if (!theSize)
        theSize = 2;
    ap1 = asm_amode();
    if (!ap1)
        return (OCODE*) - 1;
    if (!vermode(ap1, FALSE, FALSE, TRUE, FALSE))
        return 0;
    if (ap1->mode == am_ainc || ap1->mode == am_adec)
    {
        asm_err(ERR_AINVINDXMODE);
        return 0;
    }
    needpunc(comma, 0);
    ap2 = asm_amode();
    if (!ap2)
        return (OCODE*) - 1;
    if (ap2->mode != am_areg)
    {
        return (OCODE*) - 1;
    }
    return make_ocode(ap1, ap2, 0);
}

//-------------------------------------------------------------------------

static OCODE *ope_iea(void)
{
    AMODE *ap1,  *ap2;
    if (!intsize())
        return 0;
    if (!theSize)
        theSize = 2;
    ap2 = asm_amode();
    if (!ap2)
        return (OCODE*) - 1;
    if (ap2->mode != am_immed)
    {
        return (OCODE*) - 1;
    }
    needpunc(comma, 0);
    ap1 = asm_amode();
    if (!ap1)
        return (OCODE*) - 1;
    if (!vermode(ap1, TRUE, FALSE, TRUE, FALSE))
        return 0;
    return make_ocode(ap2, ap1, 0);
}

//-------------------------------------------------------------------------

static OCODE *ope_posbcd(void)
{
    AMODE *ap1,  *ap2;
    if (!intsize())
        return 0;
    if (!theSize)
        theSize = 2;
    ap1 = asm_amode();
    if (!ap1)
        return (OCODE*) - 1;
    if (ap1->mode != am_ainc)
    {
        return (OCODE*) - 1;
    }
    needpunc(comma, 0);
    ap2 = asm_amode();
    if (!ap2)
        return (OCODE*) - 1;
    if (ap2->mode != am_ainc)
    {
        return (OCODE*) - 1;
    }
    return make_ocode(ap1, ap2, 0);

}

//-------------------------------------------------------------------------

static OCODE *ope_dbr(void)
{
    AMODE *ap1,  *ap2;
    if (!wordsize())
        return 0;
    if (!need_dreg(&ap1))
        return 0;
    needpunc(comma, 0);
    ap2 = asm_amode();
    if (!ap2)
        return (OCODE*) - 1;
    if (ap2->mode != am_immed)
        return (OCODE*) - 1;
    if (ap2->offset->nodetype != en_nalabcon && ap2->offset->nodetype !=
        en_labcon)
        return (OCODE*) - 1;
    return make_ocode(ap1, ap2, 0);
}

//-------------------------------------------------------------------------

static OCODE *ope_div(void)
{
    AMODE *ap1,  *ap2,  *ap3;
    if (theSize == 4)
    {
        if (!need_020())
            return 0;
    }
    else
        if (!needsize(2))
            return 0;
    if (!theSize)
        theSize = 2;
    ap1 = asm_amode();
    if (!ap1)
        return (OCODE*) - 1;
    needpunc(comma, 0);
    if (!ap1)
        return 0;
    if (!vermode(ap1, TRUE, FALSE, TRUE, TRUE))
        return 0;
    ap2 = asm_amode();
    if (!ap2)
        return (OCODE*) - 1;
    if (ap2->mode != am_divsl && !dreg(ap2))
        return (OCODE*) - 1;
    if (ap2->mode == am_divsl)
    {
        if (!need_020())
            return 0;
        if (theSize == 2)
        {
            asm_err(ERR_AINVSIZE);
            return 0;
        }
    }
    return make_ocode(ap1, ap2, 0);

}

//-------------------------------------------------------------------------

static OCODE *ope_divl(void)
{
    AMODE *ap1,  *ap2,  *ap3;
    if (!need_020())
        return 0;
    if (!needsize(4))
        return 0;
    ap1 = asm_amode();
    if (!ap1)
        return (OCODE*) - 1;
    needpunc(comma, 0);
    if (!ap1)
        return 0;
    if (!vermode(ap1, TRUE, FALSE, TRUE, TRUE))
        return 0;
    ap2 = asm_amode();
    if (!ap2)
        return (OCODE*) - 1;
    if (ap2->mode != am_divsl)
        return (OCODE*) - 1;
    return make_ocode(ap1, ap2, 0);
}

//-------------------------------------------------------------------------

static OCODE *ope_eor(void)
{
    AMODE *ap1,  *ap2;
    if (!intsize())
        return 0;
    if (!theSize)
        theSize = 2;
    ap2 = asm_amode();
    if (!ap2)
        return (OCODE*) - 1;
    if (!dreg(ap2))
        return 0;
    needpunc(comma, 0);
    ap1 = asm_amode();
    if (!ap1)
        return (OCODE*) - 1;
    if (!vermode(ap1, TRUE, FALSE, FALSE, FALSE))
        return 0;
    return make_ocode(ap2, ap1, 0);
}

//-------------------------------------------------------------------------

static OCODE *ope_eai(void)
{
    fatal("unimplemented");
}

//-------------------------------------------------------------------------

static OCODE *ope_rrl(void)
{
    AMODE *ap1,  *ap2;
    if (!needsize(4))
        return 0;
    ap1 = asm_amode();
    if (!ap1)
        return (OCODE*) - 1;
    needpunc(comma, 0);
    ap2 = asm_amode();
    if (!ap2)
        return (OCODE*) - 1;
    if ((ap1->mode != am_areg && ap1->mode != am_dreg) || (ap2->mode != am_areg
        && ap2->mode != am_dreg))
    {
        asm_err(ERR_AILLADDRESS);
        return 0;
    }
    return make_ocode(ap1, ap2, 0);
}

//-------------------------------------------------------------------------

static OCODE *ope_ext(void)
{
    AMODE *ap1;
    if (!wordsize())
        return 0;
    if (!theSize)
        theSize = 2;
    ap1 = asm_amode();
    if (!ap1)
        return (OCODE*) - 1;
    if (!dreg(ap1))
        return 0;
    return make_ocode(ap1, 0, 0);
}

//-------------------------------------------------------------------------

static OCODE *ope_extb(void)
{
    AMODE *ap1;
    if (!needsize(4))
        return 0;
    ap1 = asm_amode();
    if (!ap1)
        return (OCODE*) - 1;
    if (!dreg(ap1))
        return 0;
    return make_ocode(ap1, 0, 0);
}

//-------------------------------------------------------------------------

static OCODE *ope_ai(void)
{
    AMODE *ap1,  *ap2;
    if (!prm_68020)
    {
        if (!needsize(2))
            return 0;
    }
    else
        if (!wordsize())
            return 0;
    ap1 = asm_amode();
    if (!ap1)
        return (OCODE*) - 1;
    if (ap1->mode != am_areg)
        return (OCODE*) - 1;
    needpunc(comma, 0);
    ap2 = asm_amode();
    if (!ap2)
        return (OCODE*) - 1;
    if (ap2->mode != am_immed)
        return (OCODE*) - 1;
    return make_ocode(ap1, ap2, 0);
}

//-------------------------------------------------------------------------

static OCODE *ope_move(void)
{
    AMODE *ap1,  *ap2;
    if (!intsize())
        return 0;
    if (!theSize)
        theSize = 2;
    ap1 = asm_amode();
    if (!ap1)
        return (OCODE*) - 1;
    needpunc(comma, 0);
    ap2 = asm_amode();
    if (!ap2)
        return (OCODE*) - 1;
    if (ap1->mode == am_usp)
    {
        if (!needsize(4))
            return 0;
        if (ap2->mode != am_areg)
        {
            return (OCODE*) - 1;
        }
    }
    else
    if (ap2->mode == am_usp)
    {
        if (!needsize(4))
            return 0;
        if (ap1->mode != am_areg)
        {
            return (OCODE*) - 1;
        }
    }
    else
    if (ap1->mode == am_sr || ap1->mode == am_ccr)
    {
        if (!needsize(ap1->mode == am_sr ? 2 : 1))
            return 0;
        if (!vermode(ap2, TRUE, FALSE, FALSE, FALSE))
            return 0;
    }
    else
    if (ap2->mode == am_sr || ap2->mode == am_ccr)
    {
        if (!needsize(ap2->mode == am_sr ? 2 : 1))
            return 0;
        if (!vermode(ap1, TRUE, FALSE, TRUE, TRUE))
            return 0;
    }
    else
    {
        if (ap1->mode == am_areg || ap2->mode == am_areg)
        {
            if (!wordsize())
                return 0;
        }
        else
            if (!intsize())
                return 0;
        if (!vermode(ap1, TRUE, TRUE, TRUE, TRUE))
            return 0;
        if (!vermode(ap2, TRUE, TRUE, FALSE, FALSE))
            return 0;
    }
    return make_ocode(ap1, ap2, 0);
}

//-------------------------------------------------------------------------

static OCODE *ope_movea(void)
{
    AMODE *ap1,  *ap2;
    if (!wordsize())
        return 0;
    if (!theSize)
        theSize = 2;
    ap1 = asm_amode();
    if (!ap1)
        return (OCODE*) - 1;
    needpunc(comma, 0);
    ap2 = asm_amode();
    if (!ap2)
        return (OCODE*) - 1;
    if (ap2->mode != am_areg)
    {
        return (OCODE*) - 1;
    }
    if (!vermode(ap1, TRUE, TRUE, TRUE, TRUE))
        return 0;
    return make_ocode(ap1, ap2, 0);
}

//-------------------------------------------------------------------------

static OCODE *ope_movec(void)
{
    AMODE *ap1,  *ap2,  *ap3;
    if (!needsize(4))
        return 0;
    ap1 = asm_amode();
    if (!ap1)
        return (OCODE*) - 1;
    needpunc(comma, 0);
    ap2 = asm_amode();
    if (!ap2)
        return (OCODE*) - 1;
    if (ap1->mode == am_areg || ap1->mode == am_dreg)
        ap3 = ap2;
    else if (ap2->mode == am_areg || ap2->mode == am_dreg)
        ap3 = ap1;
    else
    {
        return (OCODE*) - 1;
    }
    switch (ap3->mode)
    {
        case am_sfc:
        case am_dfc:
        case am_usp:
        case am_vbr:
            if (prm_68010)
                break;
        case am_cacr:
        case am_caar:
        case am_msp:
        case am_isp:
            if (prm_68020)
                break;
        default:
            return (OCODE*) - 1;
    }
    return make_ocode(ap1, ap2, 0);


}

//-------------------------------------------------------------------------

static OCODE *ope_movem(void)
{
    AMODE *ap1,  *ap2;
    if (!wordsize())
        return 0;
    ap1 = asm_amode();
    if (!ap2)
        return (OCODE*) - 1;
    needpunc(comma, 0);
    ap2 = asm_amode();
    if (!ap2)
        return (OCODE*) - 1;
    if (ap1->mode == am_dreg || ap1->mode == am_areg || ap1->mode == am_mask)
    {
        if (ap1->mode == am_dreg)
        {
            ap1->offset = (void*)(1 << ap1->preg);
            ap1->mode = am_mask;
        }
        else if (ap1->mode == am_areg)
        {
            ap1->offset = (void*)(0x100 << ap1->preg);
            ap1->mode = am_mask;
        }
        if (!vermode(ap2, FALSE, FALSE, FALSE, FALSE))
            return 0;
        if (ap2->mode == am_ainc)
        {
            return (OCODE*) - 1;
        }

    }
    else
    if (ap2->mode == am_dreg || ap2->mode == am_areg || ap2->mode == am_mask)
    {
        if (ap2->mode == am_dreg)
        {
            ap2->offset = (void*)(1 << ap2->preg);
            ap2->mode = am_mask;
        }
        else if (ap2->mode == am_areg)
        {
            ap2->offset = (void*)(0x100 << ap2->preg);
            ap2->mode = am_mask;
        }
        if (!vermode(ap1, FALSE, FALSE, FALSE, FALSE))
            return 0;
        if (ap1->mode == am_adec)
        {
            return (OCODE*) - 1;
        }
    }
    else
        return 0;
    return make_ocode(ap1, ap2, 0);
}

//-------------------------------------------------------------------------

static OCODE *ope_movep(void)
{
    AMODE *ap1,  *ap2;
    if (!wordsize())
        return 0;
    if (!theSize)
        theSize = 2;
    ap1 = asm_amode();
    if (!ap1)
        return (OCODE*) - 1;
    needpunc(comma, 0);
    ap2 = asm_amode();
    if (!ap2)
        return (OCODE*) - 1;
    if (ap1->mode == am_dreg && ap2->mode == am_indx || ap2->mode == am_dreg &&
        ap1->mode == am_indx)
    {
        return make_ocode(ap1, ap2, 0);

    }
    else
    {
        return (OCODE*) - 1;
    }
}

//-------------------------------------------------------------------------

static OCODE *ope_moveq(void)
{
    AMODE *ap1,  *ap2;
    if (!intsize())
        return 0;
    ap1 = asm_amode();
    if (!ap1)
        return (OCODE*) - 1;
    needpunc(comma, 0);
    ap2 = asm_amode();
    if (!ap2)
        return (OCODE*) - 1;
    if (ap1->mode == am_immed && ap2->mode == am_dreg)
        if (ap1->offset->v.i >=  - 128 && ap1->offset->v.i <= 127)
            return make_ocode(ap1, ap2, 0);
        else
    {
        asm_err(ERR_CONSTTOOLARGE);
        return 0;
    }
    else
        return (OCODE*) - 1;
    return (OCODE*) - 1;


}

//-------------------------------------------------------------------------

static OCODE *ope_moves(void)
{
    AMODE *ap1,  *ap2;
    if (!need_010())
        return 0;
    if (!intsize())
        return 0;
    if (!theSize)
        theSize = 2;
    ap1 = asm_amode();
    if (!ap1)
        return (OCODE*) - 1;
    needpunc(comma, 0);
    ap2 = asm_amode();
    if (!ap2)
        return (OCODE*) - 1;
    if (ap1->mode == am_dreg)
    {
        if (!vermode(ap2, FALSE, FALSE, FALSE, FALSE))
            return 0;
    }
    else
    if (ap2->mode == am_dreg)
    {
        if (!vermode(ap1, FALSE, FALSE, FALSE, FALSE))
            return 0;
    }
    else
        return (OCODE*) - 1;

    return make_ocode(ap1, ap2, 0);
}

//-------------------------------------------------------------------------

static OCODE *ope_mul(void)
{
    AMODE *ap1,  *ap2;
    if (!wordsize())
        return 0;
    if (theSize == 4 && !need_020())
        return 0;
    if (!theSize)
        theSize = 2;
    ap1 = asm_amode();
    if (!ap1)
        return (OCODE*) - 1;
    needpunc(comma, 0);
    ap2 = asm_amode();
    if (!ap2)
        return (OCODE*) - 1;
    if (!vermode(ap1, TRUE, FALSE, TRUE, TRUE))
        return 0;
    if (ap2->mode != am_dreg && (ap2->mode != am_divsl || theSize != 4))
    {
        return (OCODE*) - 1;
    }
    return make_ocode(ap1, ap2, 0);
}

//-------------------------------------------------------------------------

static OCODE *ope_eab(void)
{
    AMODE *ap1;
    if (theSize > 1)
    {
        return (OCODE*) - 2;
    }
    if (theSize == 4 && !need_020())
        return 0;
    ap1 = asm_amode();
    if (!ap1)
        return (OCODE*) - 1;
    if (!vermode(ap1, TRUE, FALSE, FALSE, FALSE))
        return 0;
    return make_ocode(ap1, 0, 0);
}

//-------------------------------------------------------------------------

static OCODE *ope_pack(void)
{
    AMODE *ap1,  *ap2,  *ap3;
    if (theSize)
    {
        return (OCODE*) - 2;
    }
    if (!need_020())
        return 0;
    ap1 = asm_amode();
    if (!ap1)
        return (OCODE*) - 1;
    needpunc(comma, 0);
    ap2 = asm_amode();
    if (!ap2)
        return (OCODE*) - 1;
    needpunc(comma, 0);
    ap3 = asm_amode();
    if (!ap3)
        return (OCODE*) - 1;
    if (ap1->mode != ap2->mode || (ap1->mode != am_dreg && ap1->mode != am_adec)
        || (ap3->mode != am_immed) || (ap3->offset->v.i &0xffff0000))
    {
        return (OCODE*) - 1;
    }
    return make_ocode(ap1, ap2, ap3);
}

//-------------------------------------------------------------------------

static OCODE *ope_i(void)
{
    AMODE *ap1;
    if (theSize)
    {
        return (OCODE*) - 2;
    }
    //   if (!need_010())
    //      return 0;
    ap1 = asm_amode();
    if (!ap1)
        return (OCODE*) - 1;
    if (ap1->mode != am_immed || (ap1->offset->v.i &0xffff0000))
    {
        asm_err(ERR_CONSTTOOLARGE);
        return 0;
    }
    return make_ocode(ap1, 0, 0);


}

//-------------------------------------------------------------------------

static OCODE *ope_set(void)
{
    return (ope_eab());
}

//-------------------------------------------------------------------------

static OCODE *ope_d(void)
{
    AMODE *ap1;
    if (!need_dreg(&ap1))
        return 0;
    if (theSize)
    {
        return (OCODE*) - 2;
    }
    return make_ocode(ap1, 0, 0);
}

//-------------------------------------------------------------------------

static OCODE *ope_trapcc(void)
{
    AMODE *ap1 = 0;
    if (!need_020())
        return 0;
    if (!wordsize())
        return 0;
    if (!theSize)
        theSize = 2;
    if (lastst != hash)
        return make_ocode(0, 0, 0);
    ap1 = asm_amode();
    if (!ap1)
    {
        return (OCODE*) - 1;
    }
    if (ap1->mode != am_immed)
    {
        return (OCODE*) - 1;
    }
    if (theSize == 2 && (ap1->offset->v.i &0xffff0000))
    {
        asm_err(ERR_CONSTTOOLARGE);
        return 0;
    }
    return make_ocode(ap1, 0, 0);
}

//-------------------------------------------------------------------------

static OCODE *ope_a(void)
{
    AMODE *ap1;
    if (!need_areg(&ap1))
        return 0;
    if (theSize)
    {
        return (OCODE*) - 2;
    }
    return make_ocode(ap1, 0, 0);
}

//-------------------------------------------------------------------------

static OCODE *ope_fmath(void)
{
    AMODE *ap1,  *ap2;
    ap1 = asm_amode();
    if (!ap1)
        return 0;
    if (lastst != comma)
    {
        if (theSize != 10 && theSize)
        {
            return (OCODE*) - 2;
        }
        if (ap1->mode != am_freg)
        {
            return (OCODE*) - 1;
        }
        return make_ocode(ap1, 0, 0);
    }
    getsym();
    ap2 = asm_amode();
    if (!ap2)
        return (OCODE*) - 1;
    if (ap2->mode != am_freg || (ap1->mode != am_freg && !vermode(ap1, TRUE,
        FALSE, TRUE, TRUE)))
    {
        return (OCODE*) - 1;
    }
    if (ap1->mode == am_freg && ap2->length != 10 && ap2->length)
    {
        return (OCODE*) - 2;
    }
    return make_ocode(ap1, ap2, 0);
}

//-------------------------------------------------------------------------

static OCODE *ope_fmathx(void)
{
    AMODE *ap1,  *ap2;
    ap1 = asm_amode();
    if (!ap1)
        return (OCODE*) - 1;
    needpunc(comma, 0);
    ap2 = asm_amode();
    if (!ap2)
        return (OCODE*) - 1;
    if (ap2->mode != am_freg || (ap1->mode != am_freg && !vermode(ap1, TRUE,
        FALSE, TRUE, TRUE)))
    {
        return (OCODE*) - 1;
    }
    if (ap1->mode == am_freg && theSize != 10 && theSize)
    {
        return (OCODE*) - 2;
    }
    return make_ocode(ap1, ap2, 0);
}

//-------------------------------------------------------------------------

static OCODE *ope_fmove(void)
{
    AMODE *ap1,  *ap2;
    ap1 = asm_amode();
    if (!ap1)
        return (OCODE*) - 1;
    needpunc(comma, 0);
    ap2 = asm_amode();
    if (!ap2)
        return (OCODE*) - 1;

    if (ap1->mode == am_freg)
    {
        if (ap2 == am_freg)
        {
            if (theSize != 10 && theSize)
            {
                return (OCODE*) - 2;
            }
        }
        else
        {
            if (!vermode(ap2, TRUE, FALSE, FALSE, FALSE))
                return 0;
            if (theSize > 7 && ap2->mode == am_dreg)
            {
                return (OCODE*) - 2;
            }
        }
    }
    else if (ap1->mode == am_fpsr || ap1->mode == am_fpcr)
    {
        if (!vermode(ap2, TRUE, FALSE, FALSE, FALSE))
            return 0;
        if (theSize && theSize != 4)
        {
            return (OCODE*) - 2;
        }

    }
    else
    {
        if (!vermode(ap1, TRUE, FALSE, TRUE, TRUE))
            return 0;
        if (ap2->mode == am_freg)
        {
            if (theSize > 7 && ap1->mode == am_dreg)
            {
                return (OCODE*) - 2;
            }
        }
        else if (ap2->mode == am_fpsr || ap2->mode == am_fpcr)
        {
            if (theSize && theSize != 4)
            {
                return (OCODE*) - 2;
            }
        }
        else
        {
            return (OCODE*) - 1;
        }

    }
    return make_ocode(ap1, ap2, 0);

}

//-------------------------------------------------------------------------

static OCODE *ope_fmovecr(void)
{
    AMODE *ap1,  *ap2;
    ap1 = asm_amode();
    if (!ap1)
        return (OCODE*) - 1;
    needpunc(comma, 0);
    ap2 = asm_amode();
    if (!ap2)
        return (OCODE*) - 1;
    if (ap1->mode != am_immed || ap2->mode != am_freg)
    {
        return (OCODE*) - 1;
    }
    if (theSize != 10 && theSize)
    {
        return (OCODE*) - 2;
    }
    if (ap1->offset->v.i &0xffffffc0)
    {
        asm_err(ERR_CONSTTOOLARGE);
        return 0;
    }
    return make_ocode(ap1, ap2, 0);
}

//-------------------------------------------------------------------------

static OCODE *ope_fmovem(void)
{
    AMODE *ap1,  *ap2;
    if (!wordsize())
        return 0;
    ap1 = asm_amode();
    if (!ap1)
        return (OCODE*) - 1;
    needpunc(comma, 0);
    ap2 = asm_amode();
    if (!ap2)
        return (OCODE*) - 1;
    if (ap1->mode == am_freg || ap1->mode == am_fmask)
    {
        if (ap1->mode == am_freg)
        {
            ap1->offset = (void*)(1 << ap1->preg);
            ap1->mode = am_fmask;
        }
        if (!vermode(ap2, FALSE, FALSE, FALSE, FALSE))
            return 0;
        if (ap2->mode == am_ainc)
        {
            return (OCODE*) - 1;
        }

    }
    else
    if (ap2->mode == am_freg || ap2->mode == am_fmask)
    {
        if (ap2->mode == am_freg)
        {
            ap2->offset = (void*)(1 << ap2->preg);
            ap2->mode = am_fmask;
        }
        if (!vermode(ap1, FALSE, FALSE, FALSE, FALSE))
            return 0;
        if (ap1->mode == am_adec)
        {
            return (OCODE*) - 1;
        }
    }
    else
        return (OCODE*) - 1;
    return make_ocode(ap1, ap2, 0);
}

//-------------------------------------------------------------------------

static OCODE *ope_fsincos(void)
{
    AMODE *ap1,  *ap2,  *ap3;
    ap1 = asm_amode();
    if (!ap1)
        return (OCODE*) - 1;
    needpunc(comma, 0);
    ap2 = asm_amode();
    if (!ap2)
        return (OCODE*) - 1;
    needpunc(comma, 0);
    ap3 = asm_amode();
    if (!ap3)
        return (OCODE*) - 1;
    if (ap2->mode != am_freg || ap3->mode != am_freg || (ap1->mode != am_freg
        && !vermode(ap1, TRUE, FALSE, TRUE, TRUE)))
    {
        return (OCODE*) - 1;
    }
    if (ap1->mode == am_freg && theSize != 10 && theSize)
    {
        return (OCODE*) - 2;
    }
    return make_ocode(ap1, ap2, ap3);
}

//-------------------------------------------------------------------------

static OCODE *ope_fone(void)
{
    AMODE *ap1;
    ap1 = asm_amode();
    if (!ap1)
        return (OCODE*) - 1;
    if (ap1->mode != am_freg && !vermode(ap1, TRUE, FALSE, TRUE, TRUE))
    {
        return (OCODE*) - 1;
    }
    if (ap1->mode == am_freg && theSize != 10 && theSize)
    {
        return (OCODE*) - 2;
    }
    return make_ocode(ap1, 0, 0);

}

//-------------------------------------------------------------------------

static OCODE *ope_fsave(void)
{
    AMODE *ap1 = asm_amode();
    if (!ap1)
        return (OCODE*) - 1;
    if (theSize)
        return (OCODE*) - 2;
    if (!vermode(ap1, FALSE, FALSE, FALSE, FALSE))
        ;
    return (OCODE*) - 1;
    if (ap1->mode == am_ainc)
        return (OCODE*) - 1;
    return make_ocode(ap1, 0, 0);
}

//-------------------------------------------------------------------------

static OCODE *ope_frestore(void)
{
    AMODE *ap1 = asm_amode();
    if (!ap1)
        return (OCODE*) - 1;
    if (theSize)
        return (OCODE*) - 2;
    if (!vermode(ap1, FALSE, FALSE, TRUE, FALSE))
        ;
    return (OCODE*) - 1;
    if (ap1->mode == am_adec)
        return (OCODE*) - 1;
    return make_ocode(ap1, 0, 0);
}

//-------------------------------------------------------------------------

OCODE *(*funcs[])(void) = 
{
    0, ope_negbcd, ope_math, ope_amath, ope_imath, ope_qmath, ope_log, ope_ilog,
        ope_shift, ope_bra, ope_bit, ope_bf1, ope_bf2, ope_bf3, ope_ead,
        ope_ear, ope_ea, ope_cmp, ope_eaa, ope_iea, ope_posbcd, ope_dbr,
        ope_div, ope_divl, ope_eor, ope_eai, ope_rrl, ope_ext, ope_extb, ope_ai,
        ope_move, ope_movea, ope_movec, ope_movem, ope_movep, ope_moveq,
        ope_moves, ope_mul, ope_eab, ope_pack, ope_i, ope_set, ope_d,
        ope_trapcc, ope_a, ope_fmath, ope_fmathx, ope_fmove, ope_fmovecr,
        ope_fmovem, ope_fsincos, ope_fone, ope_jea, ope_lea, ope_tea, ope_addx,
        ope_fsave, ope_frestore, 
};
void inasmini(void)
{
}
SNODE *asm_statement(int shortfin)
{
    #ifndef XXXXX
        SNODE *snp = 0,  **snpp = &snp;
        OCODE *rv;
        ENODE *node;
        ASMNAME *ki;
        int iserr = 0;
        theSize = 0;
        lastsym = 0;
        (*snpp) = xalloc(sizeof(SNODE));
        (*snpp)->stype = st_asm;
        (*snpp)->label = 0;
        (*snpp)->exp = 0;
        (*snpp)->next = 0;
        if (lastst != kw_asminst)
        {
            node = asm_label();
            if (!node)
                return (*snpp);
            asmline = shortfin;
            if (lastst == semicolon)
                getsym();
            (*snpp)->stype = st_label;
            (*snpp)->label = (SNODE*)node->v.i;
            return (*snpp);
        }
        ki = keyimage;
        op = asm_op();
        if (op ==  - 1)
            return (*snpp);
        if (ki->amode == 0)
        {
            rv = xalloc(sizeof(OCODE));
            rv->oper1 = rv->oper2 = rv->oper3 = rv->length = 0;
        }
        else
        {
            theSize = asm_getsize();
            rv = (*funcs[ki->amode])();
            if (rv && rv != (OCODE*) - 1 && rv != (OCODE*) - 2)
                rv->length = theSize;
            join: if (!rv || rv == (OCODE*) - 1 || rv == (OCODE*) - 2)
            {
                if (rv == (OCODE*) - 1)
                    asm_err(ERR_AILLADDRESS);
                if (rv == (OCODE*) - 2)
                    asm_err(ERR_AINVSIZE);
                iserr = 1;
                return (snp);
            }
        }
        rv->length = theSize;
        rv->noopt = TRUE;
        rv->opcode = op;
        rv->fwd = rv->back = 0;
        (*snpp)->exp = rv;
        snpp = &(*snpp)->next;
        asmline = shortfin;
        if (lastst == semicolon)
            getsym();
        return (snp);
    #else 
        return 0;
    #endif 
}
