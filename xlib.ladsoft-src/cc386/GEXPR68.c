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
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
02111-1307, USA.  

This program is derived from the cc68k complier by 
Matthew Brandt (mattb@walkingdog.net) 

You may contact the author of this derivative at:

mailto::camille@bluegrass.net

or by snail mail at:

David Lindauer
850 Washburn Ave Apt 99
Louisville, KY 40222
 */
#include <stdio.h>
#include <string.h>
#include "lists.h"
#include "expr.h"
#include "c.h"
#include "gen68.h"
#include "diag.h"

#define ABS(x) ( (x) < 0 ? -(x) : (x))
/*
 *      this module contains all of the code generation routines
 *      for evaluating expressions and conditions.
 */
extern SYM *declclass;
extern OCODE *peep_tail,  *frame_ins;
extern int stdinttype, stdunstype, stdintsize, stdldoublesize, stdaddrsize;
extern int cf_freeaddress, cf_freedata;
extern int linkreg, basereg, stackadd, stackmod;
extern long stackdepth, framedepth;
extern int prm_largedata, prm_68020, prm_phiform, prm_linkreg, prm_coldfire;
extern int prm_smallcode, prm_pcrel, prm_datarel, prm_smalldata;
extern AMODE push[], pop[];
extern int prm_68020, prm_cmangle;
extern SYM *currentfunc;
extern long lc_maxauto;
extern long nextlabel;
extern char regstack[], rsold[], rsodepth, rsdepth;
extern int retlab;
extern char dregs[3], aregs[3], fregs[3];
extern int floatstack_mode;
extern int pushcount;

int genning_inline;
static char opcomb[] = 
{
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 1, 1,
        1, 1, 1, 1, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1,
        0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
        1, 0, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 1, 1, 1,
        1, 1, 1, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 1, 1, 1, 1, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 1, 1, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1,
        0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 1, 1, 1, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 1, 1, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0,
        1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 1, 1, 1, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};
/*
 * Routine replaces move instructions the coldfire can't handle
 * with sequences it can
 */
void gen_move(int nsize, AMODE *ap2, AMODE *apr)
{
    if (!prm_coldfire)
        gen_codes(op_move, nsize, ap2, apr);
    else
    {
        if (opcomb[(ap2->mode *43) + apr->mode])
        /* Check here for ColdFire move restrictions */
            gen_codes(op_move, nsize, ap2, apr);
        else
        {
            /* Check for #0,<mode> and allow since clr will replace */
            if (ap2->mode == am_immed && ap2->offset->v.i == 0)
            {
                gen_codes(op_move, nsize, ap2, apr);
            }
            else
            {
                do_extend(ap2, nsize, F_DREG | F_VOL);
                gen_codes(op_move, nsize, ap2, apr);
            }
        }
    }
}

/* little ditty to allocate stack space for floating point
 * conversions and return a pointer to it
 * it will be at the bottom for standard stack frames; top for 
 * using ESP for frames
 * 0-7 used for fistp, 8-15 used for long intermediate values
 */
AMODE *cmpconvpos(void)
{
    AMODE *ap1;
    int constv = 16;
    //   if (prm_cplusplus)
    //      constv = 12;
    if (!floatstack_mode)
    {
        if (!lc_maxauto)
        {
            OCODE *new = xalloc(sizeof(OCODE));
            new->opcode = op_sub;
            new->oper2 = make_immed(constv);
            new->oper1 = makedreg(7);
            new->back = frame_ins;
            new->fwd = frame_ins->fwd;
            frame_ins->fwd->back = new;
            frame_ins->fwd = new;
        }
        else
        {
            frame_ins->oper2->offset->v.i -= constv;
        }
        floatstack_mode += constv;
    }
    ap1 = xalloc(sizeof(AMODE));
    ap1->mode = am_indx;
    ap1->preg = linkreg;
    ap1->offset = makeintnode(en_icon,  - lc_maxauto - constv);
    ap1->length = BESZ_DWORD;
    return ap1;
}

//-------------------------------------------------------------------------

AMODE *truncateFloat(AMODE *ap1, int size)
{
    do_extend(ap1, size, F_FREG);
    gen_codef(op_fmove, size, ap1, push);
    gen_codef(op_fmove, size, pop, ap1);
    return ap1;
}

//-------------------------------------------------------------------------

int chksize(int lsize, int rsize)
{
    int l, r;
    l = lsize;
    r = rsize;
    if (l < 0)
        l =  - l;
    if (r < 0)
        r =  - r;
    if (rsize == 5)
     /* 5 is used for bools, which are the smallest type now */
        if (lsize == 5)
            return FALSE;
        else
            return TRUE;
    if (lsize == 5)
        return FALSE;
    return (l > r);
}

//-------------------------------------------------------------------------

AMODE *make_label(int lab)
/*
 *      construct a reference node for an internal label number.
 */
{
    ENODE *lnode;
    AMODE *ap;
    lnode = xalloc(sizeof(ENODE));
    lnode->nodetype = en_labcon;
    lnode->v.i = lab;
    ap = xalloc(sizeof(AMODE));
    ap->mode = am_direct;
    ap->offset = lnode;
    return ap;
}

//-------------------------------------------------------------------------

AMODE *makebf(ENODE *node, AMODE *ap1, int size)
/*
 *      construct a bit field reference for 68020 bit field instructions
 */
{
    AMODE *ap;
    if (node->startbit ==  - 1)
        DIAG("Illegal bit field");
    ap = xalloc(sizeof(AMODE));
    ap->mode = am_bf;
    ap->preg = node->startbit;
    ap->sreg = node->bits;
    switch (size)
    {
        case BESZ_BYTE:
            case  - BESZ_BYTE: ap->preg = 8-node->startbit - node->bits;
            break;
        case BESZ_WORD:
            case  - BESZ_WORD: ap->preg = 16-node->startbit - node->bits;
            break;
        case BESZ_DWORD:
            case  - BESZ_DWORD: ap->preg = 32-node->startbit - node->bits;
            break;
    }
    return ap;
}

//-------------------------------------------------------------------------

AMODE *make_immed(long i)
/*
 *      make a node to reference an immediate value i.
 */
{
    AMODE *ap;
    ENODE *ep;
    ep = xalloc(sizeof(ENODE));
    ep->nodetype = en_icon;
    ep->v.i = i;
    ap = xalloc(sizeof(AMODE));
    ap->mode = am_immed;
    ap->offset = ep;
    return ap;
}

//-------------------------------------------------------------------------

AMODE *make_immedt(long i, int size)
/*
 *      make a node to reference an immediate value i.
 */
{
    switch (size)
    {
        case BESZ_BYTE:
            case  - BESZ_BYTE: i &= 0xff;
            break;
        case BESZ_WORD:
            case  - BESZ_WORD: i &= 0xffff;
            break;
        case BESZ_DWORD:
            case  - BESZ_DWORD: i &= 0xffffffff;
            break;
    }
    return make_immed(i);
}

//-------------------------------------------------------------------------

AMODE *make_offset(ENODE *node)
/*
 *      make a direct reference to a node.
 */
{
    AMODE *ap;
    ap = xalloc(sizeof(AMODE));
    ap->mode = am_direct;
    ap->offset = node;
    return ap;
}

//-------------------------------------------------------------------------

AMODE *make_stack(int number)
{
    AMODE *ap = xalloc(sizeof(AMODE));
    ENODE *ep = xalloc(sizeof(ENODE));
    ep->nodetype = en_icon;
    ep->v.i =  - number;
    ap->mode = am_indx;
    ap->preg = 7;
    ap->offset = ep;
    return (ap);
}

//-------------------------------------------------------------------------

void tofloat(AMODE *ap, int size)
{
    AMODE *ap2 = 0;
    int ss;
    if (ap->mode == am_freg)
        return ;
    if (size == 6 || size ==  - 6)
    {
        int pushed = FALSE;
        if (fregs[0])
        {
            pushed = TRUE;
            gen_codef(op_fmove, 10, makefreg(0), push);
        }
        ap2 = temp_float();
        if (ap->mode == am_doublereg)
        {
            gen_codes(op_move, BESZ_DWORD, makedreg(1), push);
            gen_codes(op_move, BESZ_DWORD, makedreg(0), push);
            gen_codes(op_move, BESZ_DWORD, makeareg(7), push);
            ss = 12;
        }
        else
        {
            AMODE *ap3 = direct_data(ap);
            if (ap3->mode == am_immed)
            {
                gen_codes(op_move, BESZ_DWORD, make_immed(ap3->offset->v.i &0xffffffff),
                    push);
                #if sizeof(ULLONG_TYPE) == 4
                    gen_codes(op_move, BESZ_DWORD, make_immed(ap3->offset->v.i < 0 ?  -
                        1: 0), push);
                #else 
                    gen_codes(op_move, BESZ_DWORD, make_immed(ap3->offset->v.i >> 32),
                        push);
                #endif 
                gen_codes(op_move, BESZ_DWORD, makeareg(7), push);
                ss = 12;
            }
            else
            {
                gen_codes(op_pea, BESZ_DWORD, ap, 0);
                ss = 4;
            }
        }
        if (size == 6)
            call_library2("__LQFU", ss);
        else
            call_library2("__LQFS", ss);
        if (pushed)
        {
            gen_codef(op_fmove, 10, makefreg(0), ap2);
            gen_codef(op_fmove, 10, pop, makefreg(0));
        }
        freeop(ap);
        ap->mode = am_doublereg;
        ap->length = size;
    }
    else
    {
        if (ap->mode == am_areg)
        {
            ap2 = temp_data();
            gen_codes(op_move, BESZ_DWORD, ap, ap2);
            freeop(ap);
            ap->preg = ap2->preg;
            ap->mode = ap2->mode;
        }
        ap2 = temp_float();
        gen_codef(op_fmove, ap->length, ap, ap2);
        freeop(ap);
    }
    ap->mode = am_freg;
    ap->preg = ap2->preg;
    ap->tempflag = 1;
    ap->length = 10;
    return ;
}

//-------------------------------------------------------------------------

AMODE *floatstore(ENODE *e1, AMODE *ap, AMODE *ap1, int novalue, int size)
{
    if (!ap1)
        ap1 = gen_expr(e1, FALSE, TRUE, BESZ_DWORD);
    if (e1 && isbit(e1))
    {
        do_extend(ap, BESZ_DWORD, F_DREG | F_VOL);
        bit_store(ap, ap1, e1, novalue);
        freeop(ap1);
    }
    else if (ap->mode != am_freg || ap1->mode != am_freg || ap1->preg != ap
        ->preg)
    if (size == 6 || size ==  - 6)
    {
        gen_codef(op_fmove, 10, ap, push);
        if (size == 6)
            call_library2("__LFQU", 16);
        else
            call_library2("__LFQS", 16);
        freeop(ap1);
        ap->mode = am_doublereg;
        ap->length = size;
    }
    else
    {
        gen_codef(op_fmove, size, ap, ap1);
        freeop(ap);
        ap = ap1;
    }
    if (novalue)
        freeop(ap);
    return ap;
}

//-------------------------------------------------------------------------

void doshift(int op, AMODE *ap2, AMODE *ap1, int div, int size)
{
    AMODE *apr = copy_addr(ap1),  *ap3;
    if (div)
    {
        // ap2 always a const if we get in here
        freeop(ap2);
        switch (size)
        {
            case BESZ_BYTE:
                case  - BESZ_BYTE: gen_codes(op_btst, BESZ_BYTE, make_immed(7), ap1);
                gen_code(op_beq, make_label(nextlabel), 0);
            case BESZ_WORD:
                case  - BESZ_WORD: do_extend(ap1, BESZ_DWORD, F_DREG | F_VOL);
                gen_codes(op_and, BESZ_DWORD, make_immed(0x8000), ap1);
                gen_code(op_beq, make_label(nextlabel), 0);
                freeop(ap1);
                break;
            case BESZ_DWORD:
            case  - BESZ_DWORD: default:
                if (ap2->mode == am_immed && ap2->offset->v.i == 1 && ap1->mode
                    == am_dreg)
                {
                    gen_codes(op_asr, BESZ_DWORD, ap2, ap1);
                    gen_code(op_bpl, make_label(nextlabel), 0);
                    gen_code(op_bcc, make_label(nextlabel), 0);
                    gen_codes(op_add, BESZ_DWORD, make_immed(1), ap1);
                    gen_label(nextlabel++);
                    return ;
                }
                else
                {
                    if (ap1->mode == am_dreg)
                    {
                        gen_codes(op_tst, BESZ_DWORD, ap1, 0);
                        gen_code(op_bpl, make_label(nextlabel), 0);
                    }
                    else
                    {
                        do_extend(ap1, BESZ_DWORD, F_DREG | F_VOL);
                        gen_codes(op_and, BESZ_DWORD, make_immed(0x80000000), ap1);
                        gen_code(op_beq, make_label(nextlabel), 0);
                        freeop(ap1);
                    }
                    break;
                }
        }
        ap3 = copy_addr(ap2);
        ap3->offset = makeintnode(en_icon, ((1 << ap2->offset->v.i) - 1));
        ap3->length = ap2->length;
        gen_code(op_add, ap3, ap1);
        gen_label(nextlabel++);
    }
    {
        do_extend(apr, size, F_DREG | F_VOL);
        if (ap2->mode == am_immed)
        {
            int temp = ap2->offset->v.i;
            while (temp > 8)
            {
                temp = temp - 8;
                gen_codes(op, size, make_immed(8), apr);
            }
            if (temp != 0)
                gen_codes(op, size, make_immed(temp), apr);
        }
        else
        {
            do_extend(ap2, size, F_DREG);
            gen_codes(op, size, ap2, apr);
            freeop(ap2);
        }
        ap2->mode = am_dreg;
        ap2->preg = apr->preg;
    }
}

//-------------------------------------------------------------------------

AMODE *do6shift(int op, ENODE *node, int div, int size)
{
    AMODE *ecx = makedreg(2),  *ap1 = 0,  *ap2,  *ap3;
    LLONG_TYPE t;
    ap2 = gen_expr(node->v.p[1], FALSE, FALSE, BESZ_DWORD);
    if (div)
    {
        // if we get in here ap2 is a const ...
        ap1 = gen_expr(node->v.p[0], FALSE, FALSE, size);
        do_extend(ap1, size, F_DOUBLEREG);
        gen_codes(op_btst, BESZ_DWORD, make_immed(31), makedreg(0));
        gen_1branch(op_beq, make_label(nextlabel));
        {
            AMODE *ap3 = copy_addr(ap2);
            AMODE *ap4 = copy_addr(ap2);
            t = (1 << ap2->offset->v.i) - 1;
            #if sizeof(ULLONG_TYPE) == 4
                ap3->offset = makeintnode(en_icon, t < 0 ?  - 1: 0);
            #else 
                ap3->offset = makeintnode(en_icon, t >> 32);
            #endif 
            ap4->offset = makeintnode(en_icon, t);
            ap3->length = ap2->length;
            ap4->length = ap2->length;
            gen_codes(op_add, BESZ_DWORD, ap3, makedreg(0));
            gen_codes(op_add, BESZ_DWORD, ap4, makedreg(1));
            gen_code(op_bcc, make_label(nextlabel), 0);
            gen_codes(op_add, BESZ_DWORD, make_immed(1), makedreg(0));
            gen_label(nextlabel++);
        }
        gen_label(nextlabel++);
    }
    {

        int xchg =  - 1;
        int pushed = FALSE;
        if (dregs[2] && ap2->mode != am_dreg && ap2->preg != 2)
        {
            pushed = TRUE;
            gen_push(2, am_dreg, 0);
        }
        if (ap2->mode == am_doublereg)
            gen_push(1, am_dreg, 0);
        else
        {
            if (ap2->length == 6 || ap2->length ==  - 6)
            {
                if (ap2->offset)
                    ap2->offset = makenode(en_add, ap2->offset, makeintnode
                        (en_icon, 4));
                else
                    ap2->offset = makeintnode(en_icon, 4);
                if (ap2->mode == am_ind)
                    ap2->mode = am_indx;
                ap2->length = BESZ_DWORD;
            }
            else
                do_extend(ap2, BESZ_DWORD, F_ALL);
            gen_codes(op_move, BESZ_DWORD, ap2, push);
        }
        freeop(ap2);
        if (!ap1)
            ap1 = gen_expr(node->v.p[0], FALSE, FALSE, size);
        if (ap1->mode == am_doublereg)
        {
            gen_push(1, am_dreg, 0);
            gen_push(0, am_dreg, 0);
        }
        else if (ap1->mode == am_immed)
        {
            gen_codes(op_move, BESZ_DWORD, make_immed(ap1->offset->v.i), push);
            #if sizeof(LLONG_TYPE) == 4
                gen_codes(op_move, BESZ_DWORD, make_immed(ap1->offset->v.i < 0 ?  - 1: 0)
                    , push);
            #else 
                gen_codes(op_move, BESZ_DWORD, make_immed(ap1->offset->v.i >> 32), push);
            #endif 
        }
        else
        {
            ap2 = copy_addr(ap1);
            if (ap2->offset)
                ap2->offset = makenode(en_add, ap2->offset, makeintnode(en_icon,
                    4));
            else
                ap2->offset = makeintnode(en_icon, 4);
            if (ap2->mode == am_ind)
                ap2->mode = am_indx;
            gen_codes(op_move, BESZ_DWORD, ap2, push);
            gen_codes(op_move, BESZ_DWORD, ap1, push);
        }
        freeop(ap1);
        if (op == op_lsr)
            call_library2("__LXSHR", 12);
        else if (op == op_asr)
            call_library2("__LXSAR", 12);
        else
            call_library2("__LXSHL", 12);
        if (pushed)
            gen_pop(2, am_dreg, 0);
        dregs[0]++;
        dregs[1]++;
    }
    ap1 = xalloc(sizeof(AMODE));
    ap1->mode = am_doublereg;
    ap1->length = 6;
    return ap1;
}

//-------------------------------------------------------------------------

void bit_store(AMODE *ap1, AMODE *ap2, ENODE *node, int novalue)
{
    if (prm_68020 && !prm_coldfire)
    {
        AMODE *ap3;
        if (ap1->mode != am_dreg)
            do_extend(ap1, BESZ_DWORD, F_DREG | F_VOL);
        gen_code3(op_bfins, 0, ap1, ap2, makebf(node, ap2, ap1->length));
        ap1->mode = ap3->mode;
        ap1->preg = ap3->preg;
        ap1->tempflag = ap3->tempflag;
    }
    else if (ap1->mode == am_immed)
    {
        int vb;
        AMODE *ap3;
        if (prm_coldfire)
            do_extend(ap2, BESZ_DWORD, F_DREG | F_VOL);
        ap3 = make_immed(ap1->offset->v.i &mod_mask(node->bits));
        //         if (node->bits == 1) {
        //            if (!ap1->offset->v.i)
        //               gen_codes(op_bclr,BESZ_DWORD,make_immed(node->startbit),ap2);
        //         } else
        if (mod_mask(node->bits) != ap3->offset->v.i)
            gen_codes(op_andi, ap2->length, make_immed(~(mod_mask(node->bits)
                << node->startbit)), ap2);
        //         if ((vb = single_bit(ap1->offset->v.i)) != -1) {
        //            gen_codes(op_bset,BESZ_DWORD,make_immed(vb+node->startbit),ap2);
        //         }else 
        if (ap3->offset->v.i)
        {
            ap3->offset->v.i <<= node->startbit;
            gen_codes(op_or, ap2->length, ap3, ap2);
        }
        ap1->mode = ap2->mode;
        ap1->preg = ap2->preg;
        ap1->tempflag = ap2->tempflag;
    }
    else
    {
        do_extend(ap1, ap2->length, F_DREG | F_VOL);
        gen_codes(op_andi, BESZ_DWORD, make_immed(mod_mask(node->bits)), ap1);
        if (!novalue)
            gen_codes(op_move, BESZ_DWORD, ap1, push);
        if (node->startbit)
            doshift(op_asl, make_immed(node->startbit), ap1, FALSE, ap1->length)
                ;
        gen_codes(op_andi, ap2->length, make_immed(~(mod_mask(node->bits) <<
            node->startbit)), ap2);
        gen_codes(op_or, ap2->length, ap1, ap2);
        if (!novalue)
            gen_codes(op_move, BESZ_DWORD, pop, ap1);
    }
}

//-------------------------------------------------------------------------

AMODE *bit_load(AMODE *ap, ENODE *node, int size)
{
    if (prm_68020 && !prm_coldfire)
    {
        AMODE *ap1 = temp_data();
        ap1->tempflag = TRUE;
        if (ap1->mode == am_immed)
            gen_codes(op_move, BESZ_DWORD, ap1, ap);
        else
            gen_code3(op_bfextu, 0, ap, makebf(node, ap, size), ap1);
        return ap1;
    }
    else
    {
        do_extend(ap, ap->length, F_DREG | F_VOL);
        if (node->startbit)
            doshift(op_asr, make_immed(node->startbit), ap, FALSE, ap->length);
        gen_codes(op_andi, ap->length, make_immed(mod_mask(node->bits)), ap);
        return ap;
    }
}

//-------------------------------------------------------------------------

void loaddoublereg(AMODE *ap)
{
    if (ap->mode == am_immed)
    {
        temp_doubledregs();
        gen_codes(op_move, BESZ_DWORD, make_immed(ap->offset->v.i &0xffffffff), makedreg
            (1));
        #if sizeof(ULLONG_TYPE) == 4
            gen_codes(op_move, BESZ_DWORD, make_immed(ap->offset->v.i < 0 ?  - 1: 0),
                makedreg(0));
        #else 
            gen_codes(op_move, BESZ_DWORD, make_immed(ap->offset->v.i >> 32), makedreg(0)
                );
        #endif 
    }
    else
    {
        AMODE *ap2;
        ap = direct_data(ap);
        temp_doubledregs();
        gen_codes(op_move, BESZ_DWORD, ap, makedreg(0));
        ap2 = copy_addr(ap);
        if (ap2->offset)
            ap2->offset = makenode(en_add, ap2->offset, makeintnode(en_icon, 4))
                ;
        else
            ap2->offset = makeintnode(en_icon, 4);
        if (ap2->mode == am_ind)
            ap2->mode = am_indx;
        gen_codes(op_move, BESZ_DWORD, ap2, makedreg(1));
        freeop(ap);
    }
    ap->mode = am_doublereg;
    ap->length = 6;
}

//-------------------------------------------------------------------------

void do_extend(AMODE *ap, int osize, int flags)
/*
 *      if isize is not equal to osize then the operand ap will be
 *      loaded into a register (if not already) and if osize is
 *      greater than isize it will be extended to match.
 */
{
    AMODE *ap2,  *ap1;

    int isize = ap->length;
    if (osize ==  - 5)
        osize = 5;
    if (isize && isize != osize && isize !=  - osize && ap->mode != am_immed)
    {
        if (ap->mode == am_dreg && (!(flags &F_VOL) || ap->tempflag))
            ap2 = ap;
        else
        {
            if (!(flags &F_NOREUSE))
                freeop(ap);
            ap2 = temp_data();
        }
        if (chksize(isize, osize))
        {
            /* moving to a lower type */
            if (isize > 6)
            {
                tofloat(ap, isize);
                if (osize <= 6)
                {
                    int pushed = FALSE;
                    ap2 = floatstore(0, ap, ap2, FALSE, osize);
                    ap->mode = ap2->mode;
                    ap->preg = ap2->preg;
                    ap->sreg = ap2->sreg;
                    ap->offset = ap2->offset;
                    ap->length = osize;
                    ap->tempflag = 1;

                }
            }
            else
            {
                if (ap->mode != am_dreg || ap->preg != ap2->preg)
                    if (isize == 6 || isize ==  - 6)
                if (ap->mode == am_doublereg)
                {
                    if (ap2->preg != 1)
                    {
                        dregs[ap2->preg]--;
                        dregs[1]++;
                    }
                    ap2->mode = am_dreg;
                    ap2->preg = 1;
                    ap2->tempflag = 1;
                }
                else
                    gen_codes(op_move, BESZ_DWORD, ap, ap2);
                else
                    gen_codes(op_move, isize, ap, ap2);
                ap->mode = ap2->mode;
                ap->preg = ap2->preg;
                ap->length = osize;
                ap->tempflag = ap2->tempflag;
            }
        }
        else
        {
            /* moving up in type */
            if (isize > 6)
            {
                tofloat(ap, isize);
                freeop(ap2);
            }
            else if (isize == 6 || isize ==  - 6 || osize > 6)
            {
                tofloat(ap, isize);
                ap->tempflag = 1;
            }
            else
            {
                int size6 = osize == 6 || osize ==  - 6;
                if (osize == 6 || osize ==  - 6)
                    osize = BESZ_DWORD;
                if (ap->mode == am_areg && (isize == BESZ_BYTE || isize ==  - BESZ_BYTE))
                    gen_codes(op_move, BESZ_DWORD, ap, ap2);
                if (size6 && (isize == BESZ_DWORD || isize ==  - BESZ_DWORD))
                    gen_codes(op_move, isize, ap, ap2);
                else if (isize < 0)
                {
                    if (!equal_address(ap, ap2))
                    {
                        gen_codes(op_move, isize, ap, ap2);
                        freeop(ap);
                    }
                    if ((isize == BESZ_BYTE || isize ==  - BESZ_BYTE) && osize ==  - BESZ_DWORD && 
                        (prm_68020 || prm_coldfire))
                        gen_codes(op_extb, BESZ_DWORD, ap2, 0);
                    else
                    {
                        if (isize == BESZ_BYTE || isize ==  - BESZ_BYTE)
                            gen_codes(op_ext, BESZ_WORD, ap2, 0);
                        if (osize ==  - BESZ_DWORD || osize == BESZ_DWORD)
                            gen_codes(op_ext, BESZ_DWORD, ap2, 0);
                    }
                }
                else
                {
                    if (equal_address(ap, ap2))
                        ap2 = temp_data();
                    gen_codes(op_moveq, 0, make_immed(0), ap2);
                    gen_codes(op_move, ap->length, ap, ap2);
                    freeop(ap);
                }
                if (size6)
                {
                    freeop(ap2);
                    temp_doubledregs();
                    if (ap2->preg != 1)
                        gen_codes(op_move, BESZ_DWORD, ap2, makedreg(1));
                    gen_codes(op_moveq, BESZ_DWORD, make_immed(0), makedreg(0));
                    if (isize < 0)
                    {
                        int label = nextlabel++;
                        gen_codes(op_tst, BESZ_DWORD, makedreg(1), 0);
                        gen_1branch(op_bpl, make_label(label));
                        gen_codes(op_subq, BESZ_DWORD, make_immed(1), makedreg(0));
                        gen_label(label);
                    }
                    ap->mode = am_doublereg;
                    ap->length = 6;
                    osize = 6;
                }
                else
                {
                    ap->mode = am_dreg;
                    ap->preg = ap2->preg;
                    ap->tempflag = ap2->tempflag;
                    ap->length = osize;
                }
            }
        }
    }
    if (((flags &F_VOL) == 0) || ap->tempflag)
    {
        switch (ap->mode)
        {
            case am_doublereg:
                if (flags &F_DOUBLEREG)
                    return ;
                break;
            case am_freg:
                if (flags &F_FREG)
                    return ;
                break;
            case am_immed:
                if (flags &F_IMMED)
                    return ;
                 /* mode ok */
                break;
            case am_areg:
                if (flags &F_AREG)
                    return ;
                break;
            case am_dreg:
                if (flags &F_DREG)
                    return ;
                break;
            case am_indx:
            case am_ind:
            case am_ainc:
            case am_adec:
                if (flags &F_REGI)
                    return ;
                break;
            case am_baseindxdata:
            case am_baseindxaddr:
                if (flags &F_REGIX)
                    return ;
                break;
            case am_adirect:
            case am_direct:
                if (flags &F_ABS)
                    return ;
            case am_pcindx:
                if (flags &(F_PCI | F_PCIX))
                    return ;
                break;
        }
    }
    if (flags &(F_FREG | F_DREG))
    {
        if (flags &F_DREG)
        {
            if (ap->mode != am_dreg || !ap->tempflag && (flags &F_VOL))
            if (isize == osize || isize ==  - osize || ap->mode == am_immed)
            {
                if (osize < 7)
                {
                    freeop(ap);
                    ap2 = temp_data();
                    ap2->length = osize;
                    gen_code(op_move, ap, ap2);
                    ap->mode = ap2->mode;
                    ap->preg = ap2->preg;
                    ap->tempflag = ap2->tempflag;
                }
                else
                {
                    tofloat(ap, isize);
                }
            }
        }
        else if (ap->mode != am_freg)
        {
            tofloat(ap, isize);
        }
        else if (flags &F_VOL)
        {
            ap2 = temp_float();
            ap2->length = osize;
            gen_codes(op_fmove, osize, ap, ap2);
            ap->mode = ap2->mode;
            ap->preg = ap2->preg;
            ap->tempflag = ap2->tempflag;
        }

    }
    if ((flags &F_DOUBLEREG) && !(flags &(F_MEM | F_IMMED)) && ap->mode !=
        am_doublereg)
        loaddoublereg(ap);
    if (flags &F_AREG)
    {
        if (isize ==  - BESZ_BYTE)
        {
            freeop(ap);
            ap2 = temp_data();
            gen_codes(op_move, BESZ_BYTE, ap, ap2);
            gen_codes(op_ext, 2, ap2, 0);
            ap->mode = ap2->mode;
            ap->preg = ap2->preg;
            ap->tempflag = 1;
            isize =  - 2;
        }
        if (isize == BESZ_BYTE)
        {
            freeop(ap);
            ap2 = temp_data();
            gen_codes(op_move, BESZ_BYTE, ap, ap2);
            gen_codes(op_and, 2, make_immed(0xff), ap2);
            ap->mode = ap2->mode;
            ap->preg = ap2->preg;
            ap->tempflag = 1;
            isize = 2;
        }
        freeop(ap);
        ap2 = temp_addr();
        gen_codes(op_move, isize, ap, ap2);
        ap->mode = am_areg;
        ap->preg = ap2->preg;
        ap->tempflag = 1;
    }
    if (osize != isize && osize == 5)
    {
        int lab = nextlabel++;
        ap2 = temp_data();
        gen_codes(op_moveq, BESZ_DWORD, make_immed(0), ap2);
        gen_codes(op_cmp, BESZ_BYTE, make_immed(0), ap);
        gen_code(op_beq, make_label(lab), 0);
        gen_codes(op_moveq, BESZ_BYTE, make_immed(1), ap2);
        gen_label(lab);
        freeop(ap);
        ap->mode = ap->mode;
        ap->tempflag = ap2->tempflag;
        ap->length = ap2->length;
        ap->preg = ap2->preg;
    }
    ap->length = osize;
    if (osize == 5)
        ap->length = 5;
}

//-------------------------------------------------------------------------

int isshort(ENODE *node)
/*
 *      return true if the node passed can be generated as a short
 *      offset.
 */
{
    return (isintconst(node->nodetype) || node->nodetype == en_absacon) && 
        (node->v.i >=  - 32768 && node->v.i <= 32767);
}

//-------------------------------------------------------------------------

int isbyte(ENODE *node)
/*
 *      return true if the node passed can be evaluated as a byte
 *      offset.
 */
{
    return isintconst(node->nodetype) && ( - 128 <= node->v.i && node->v.i <=
        127);
}

//-------------------------------------------------------------------------

int isamshort(AMODE *ap)
{
    LLONG_TYPE v;
    if (ap->offset->nodetype != en_icon)
        return TRUE;
    v = ap->offset->v.i;
    return (v >=  - 32768 && v < 32767);
}

//-------------------------------------------------------------------------

int isamshort2(AMODE *ap, AMODE *ap2)
{
    LLONG_TYPE v;
    if (ap->offset->nodetype != en_icon || ap2->offset->nodetype != en_icon)
        return TRUE;
    v = ap->offset->v.i + ap2->offset->v.i;
    return (v >=  - 32768 && v < 32767);
}

//-------------------------------------------------------------------------

int isambyte(AMODE *ap)
{
    long v;
    if (ap->offset->nodetype != en_icon)
        return FALSE;
    v = ap->offset->v.i;
    return (v >=  - 128 && v < 128);
}

//-------------------------------------------------------------------------

int isambyte2(AMODE *ap, AMODE *ap2)
{
    long v;
    if (ap->offset->nodetype != en_icon || ap2->offset->nodetype != en_icon)
        return FALSE;
    v = ap->offset->v.i + ap2->offset->v.i;
    return (v >=  - 128 && v < 128);
}

//-------------------------------------------------------------------------

static int depth(ENODE *node)
{
    int a, b;
    if (!node)
        return 0;
    switch (node->nodetype)
    {
        case en_structret:
            return 0;
        case en_cfi:
        case en_cfc:
        case en_cri:
        case en_crc:
        case en_clri:
        case en_clrc:
        case en_cll:
        case en_cull:
		case en_ci:
		case en_cui:
        case en_cl:
        case en_cul:
        case en_cp:
        case en_cbool:
        case en_cub:
        case en_cb:
        case en_cuw:
        case en_cw:
        case en_cd:
        case en_cld:
        case en_cf:
        case en_bits:
        case en_ll_ref:
        case en_ull_ref:
        case en_l_ref:
        case en_ul_ref:
		case en_a_ref:
		case en_ua_ref:
		case en_i_ref:
		case en_ui_ref:
        case en_ub_ref:
        case en_bool_ref:
        case en_b_ref:
        case en_uw_ref:
        case en_w_ref:
        case en_longdoubleref:
        case en_doubleref:
        case en_floatref:
        case en_fimaginaryref:
        case en_rimaginaryref:
        case en_lrimaginaryref:
        case en_fcomplexref:
        case en_rcomplexref:
        case en_lrcomplexref:
            return 1+depth(node->v.p[0]);
        case en_uminus:
        case en_moveblock:
        case en_stackblock:
        case en_movebyref:
        case en_clearblock:
            return 1+depth(node->v.p[0]);
        case en_fimaginarycon:
        case en_rimaginarycon:
        case en_lrimaginarycon:
        case en_fcomplexcon:
        case en_rcomplexcon:
        case en_lrcomplexcon:
        case en_llcon:
        case en_llucon:
        case en_icon:
        case en_lcon:
        case en_lucon:
        case en_iucon:
        case en_boolcon:
        case en_ccon:
        case en_cucon:
        case en_rcon:
        case en_lrcon:
        case en_fcon:
        case en_absacon:
        case en_trapcall:
        case en_labcon:
        case en_nacon:
        case en_autocon:
        case en_autoreg:
        case en_napccon:
        case en_nalabcon:
        case en_tempref:
        case en_regref:
            return 1;
        case en_not:
        case en_compl:
        case en_substack:
            return 1+depth(node->v.p[0]);
        case en_eq:
        case en_ne:
        case en_lt:
        case en_le:
        case en_gt:
        case en_ge:
        case en_ugt:
        case en_uge:
        case en_ult:
        case en_ule:
        case en_land:
        case en_lor:
        case en_div:
        case en_udiv:
        case en_pdiv:
        case en_mod:
        case en_umod:
        case en_assign:
        case en_refassign:
        case en_lassign:
        case en_asuminus:
        case en_ascompl:
        case en_add:
        case en_sub:
        case en_addstruc:
        case en_umul:
        case en_pmul:
		case en_arrayindex:
        case en_mul:
        case en_and:
        case en_or:
        case en_xor:
        case en_asalsh:
        case en_asarsh:
        case en_alsh:
        case en_arsh:
        case en_arshd:
        case en_asarshd:
        case en_lsh:
        case en_rsh:
        case en_asadd:
        case en_assub:
        case en_asmul:
        case en_asdiv:
        case en_asmod:
        case en_asand:
        case en_asumod:
        case en_asudiv:
        case en_asumul:
        case en_asor:
        case en_aslsh:
        case en_asxor:
        case en_asrsh:
        case en_repcons:
        case en_ainc:
        case en_adec:
            a = depth(node->v.p[0]);
            b = depth(node->v.p[1]);
            if (a > b)
                return 1+a;
            return 1+b;
		case en_array:
            a = depth(node->v.p[0]);
            b = depth(node->v.p[1])-1;
            if (a > b)
                return 1+a;
            return 1+b;
        case en_void:
        case en_cond:
        case en_voidnz:
        case en_dvoid:
            return 1+depth(node->v.p[1]);
        case en_sfcall:
        case en_sfcallb:
        case en_scallblock:
        case en_pfcall:
        case en_pfcallb:
        case en_fcall:
        case en_intcall:
        case en_callblock:
        case en_fcallb:
        case en_pcallblock:
        case en_thiscall:
            return 1;
        case en_cl_reg:
        case en_conslabel:
        case en_destlabel:
        case en_addcast:
            return depth(node->v.p[0]);
        default:
            DIAG("error in depth routine.");
            return 1;
    }
}

//-------------------------------------------------------------------------

void noids(AMODE *ap)
{
    switch (ap->mode)
    {
        case am_baseindxaddr:
        case am_baseindxdata:
        case am_pcindxdata:
        case am_pcindxaddr:
        case am_iiprepca:
        case am_iipostpca:
        case am_iiprepcd:
        case am_iipostpcd:
        case am_iiprea:
        case am_iipred:
        case am_iiposta:
        case am_iipostd:
            do_extend(ap, ap->length, F_DREG | F_VOL);
            break;
    }
}

//-------------------------------------------------------------------------

AMODE *direct_data(AMODE *ap)
{
    AMODE *ap3;
    int oldlen = ap->length;
    switch (ap->mode)
    {
        case am_ainc:
        case am_adec:
        case am_ind:
            ap->mode = am_indx;
            ap->offset = makeintnode(en_icon, 0);
            return ap;
        case am_adirect:
        case am_direct:
        case am_immed:
        case am_indx:
        case am_pcindx:
            return ap;
        case am_pc:
            ap->mode = am_indx;
            ap->offset = makeintnode(en_icon, 0);
            return ap;
        default:
            freeop(ap);
            ap3 = temp_addr();
            ap->length = BESZ_DWORD;
            gen_code(op_lea, ap, ap3);
            ap3->mode = am_indx;
            ap3->offset = makeintnode(en_icon, 0);
            ap3->length = oldlen;
            freeop(ap);
            return ap3;
    }
}

//-------------------------------------------------------------------------

AMODE *doindex(ENODE *node, enum e_node type)
{
    AMODE *ap,  *ap2,  *ap3;
    ENODE node2;
    int scale;
    switch (node->nodetype)
    {
        case en_icon:
            ap = gen_expr(node, FALSE, TRUE, BESZ_DWORD);
            break;
        case en_lsh:
            if ((prm_68020 && (scale = node->v.p[1]->v.i) < 4 && scale) && 
                (!prm_coldfire || (scale < 3)))
            {
                ap = gen_expr(node->v.p[0], FALSE, TRUE, BESZ_DWORD);
                if (node->v.p[0]->nodetype == en_bits)
                    ap = bit_load(ap, node->v.p[0], BESZ_DWORD);
                if (ap->mode != am_immed)
                    do_extend(ap, BESZ_DWORD, F_DREG);
                if (ap->mode == am_immed)
                {
                    while (--scale)
                        ap->offset->v.i <<= 1;
                }
                else
                {
                    do_extend(ap, BESZ_DWORD, F_DREG);
                    if (ap->mode == am_dreg)
                        ap->mode = am_baseindxdata;
                    else
                        ap->mode = am_baseindxaddr;
                    ap->sreg = ap->preg;
                    ap->preg =  - 1;
                    ap->scale = scale;
                    ap->offset = makeintnode(en_icon, 0);
                }
                break;
            }
        default:
            node2.v.p[0] = node;
            node2.nodetype = type;
            ap = gen_deref(&node2, BESZ_DWORD);
            switch (ap->mode)
            {
            case am_ainc:
            case am_adec:
                ap2 = temp_addr();
                gen_lea(ap->length, ap, ap2);
                freeop(ap);
                ap = ap2;
            case am_baseindxdata:
            case am_baseindxaddr:
                if (ap->sreg >= 0 && ap->preg >= 0)
                {
                    freeop(ap);
                    ap3 = temp_addr();
                    gen_lea(BESZ_DWORD, ap, ap3);
                    ap3->mode = am_ind;
                    ap = ap3;
                }

            }
            break;
    }
    return ap;
}

//-------------------------------------------------------------------------

AMODE *gen_index(ENODE *node)
/*
 *      generate code to evaluate an index node (^+) and return
 *      the addressing mode of the result. This routine takes no
 *      flags since it always returns either am_ind or am_indx.
 */
{
    AMODE *ap1,  *ap2,  *ap3,  *ap;
    ENODE node2;

    int a = depth(node->v.p[1]) - depth(node->v.p[0]);
    int nsize;
    if (a <= 2)
    {
        ap1 = doindex(node->v.p[0], node->nodetype);
        //               nsize = natural_size(node->v.p[1]) ;
        //               if (nsize == 6 || nsize == -6)
        //                  aprtocx(ap1) ;
        ap2 = doindex(node->v.p[1], node->nodetype);
    }
    else
    {
        ap2 = doindex(node->v.p[1], node->nodetype);
        //               nsize = natural_size(node->v.p[0]) ;
        //               if (nsize == 6 || nsize == -6)
        //                  aprtocx(ap2) ;
        ap1 = doindex(node->v.p[0], node->nodetype);
    }
    tryagain: switch (ap1->mode)
    {
        case am_areg:
            switch (ap2->mode)
            {
            case am_areg:
                ap1->sreg = ap2->preg;
                ap1->mode = am_baseindxaddr;
                ap1->offset = makeintnode(en_icon, 0);
                ap1->scale = 0;
                return ap1;
            case am_dreg:
                ap1->sreg = ap2->preg;
                ap1->mode = am_baseindxdata;
                ap1->offset = makeintnode(en_icon, 0);
                ap1->scale = 0;
                return ap1;
            case am_adirect:
                if ((!prm_largedata && (prm_datarel || !prm_smalldata)) ||
                    prm_68020)
                {
                    ap1->mode = am_indx;
                    ap1->offset = ap2->offset;
                    return ap1;
                }
                ap = temp_addr();
                gen_lea(0, ap2, ap);
                ap1->mode = am_baseindxaddr;
                ap1->offset = makeintnode(en_icon, 0);
                ap1->scale = 0;
                ap1->sreg = ap->preg;
                return ap1;
            case am_immed:
                if ((!prm_largedata && (prm_datarel || !prm_smalldata)) ||
                    prm_68020)
                {
                    ap1->mode = am_indx;
                    ap1->offset = ap2->offset;
                    return ap1;
                }
                ap = temp_data();
                gen_codes(op_move, BESZ_DWORD, ap2, ap);
                ap1->mode = am_baseindxdata;
                ap1->offset = makeintnode(en_icon, 0);
                ap1->scale = 0;
                ap1->sreg = ap->preg;
                return ap1;
            case am_indx:
                if (prm_68020 || isambyte(ap2))
                {
                    ap1->mode = am_baseindxaddr;
                    ap1->offset = ap2->offset;
                    ap1->scale = 0;
                    ap1->sreg = ap2->preg;
                    return ap1;
                }
                freeop(ap2);
                ap = temp_addr();
                gen_lea(0, ap2, ap);
                ap->mode = am_baseindxaddr;
                ap->sreg = ap1->preg;
                ap->scale = 0;
                ap->offset = makeintnode(en_icon, 0);
                return ap;
            case am_ind:
                ap1->mode = am_baseindxaddr;
                ap1->offset = makeintnode(en_icon, 0);
                ap1->scale = 0;
                ap1->sreg = ap2->preg;
                return ap1;
            case am_baseindxaddr:
            case am_baseindxdata:
                if (ap2->preg ==  - 1)
                {
                    ap2->preg = ap1->preg;
                    return ap2;
                }
                freeop(ap2);
                ap = temp_addr();
                gen_lea(0, ap2, ap);
                ap1->mode = am_baseindxaddr;
                ap1->scale = 0;
                ap1->offset = makeintnode(en_icon, 0);
                ap1->sreg = ap->preg;
                return ap1;
            }
            break;
        case am_dreg:
            switch (ap2->mode)
            {
            case am_areg:
                ap2->sreg = ap1->preg;
                ap2->mode = am_baseindxdata;
                ap2->offset = makeintnode(en_icon, 0);
                ap2->scale = 0;
                return ap2;
            case am_dreg:
                freeop(ap2);
                ap = temp_addr();
                gen_lea(0, ap1, ap);
                ap->sreg = ap1->preg;
                ap->mode = am_baseindxdata;
                ap->offset = makeintnode(en_icon, 0);
                ap->scale = 0;
                return ap;
            case am_adirect:
                if ((!prm_largedata && (prm_datarel || !prm_smalldata)) ||
                    prm_68020)
                {
                    freeop(ap1);
                    ap = temp_addr();
                    gen_codes(op_move, BESZ_DWORD, ap1, ap);
                    ap->mode = am_indx;
                    ap->offset = ap2->offset;
                    return ap;
                }
                ap = temp_addr();
                gen_lea(0, ap2, ap);
                ap->mode = am_baseindxdata;
                ap->offset = makeintnode(en_icon, 0);
                ap->scale = 0;
                ap->sreg = ap1->preg;
                return ap;
            case am_immed:
                if (prm_68020 || isamshort(ap2))
                {
                    freeop(ap1);
                    ap = temp_addr();
                    gen_codes(op_move, BESZ_DWORD, ap1, ap);
                    ap->mode = am_indx;
                    ap->offset = ap2->offset;
                    return ap;
                }
                ap = temp_addr();
                gen_code(op_move, ap2, ap);
                ap->mode = am_baseindxdata;
                ap->offset = makeintnode(en_icon, 0);
                ap->scale = 0;
                ap->sreg = ap1->preg;
                return ap;
            case am_indx:
                if (prm_68020 || isambyte(ap2))
                {
                    ap2->mode = am_baseindxdata;
                    ap2->scale = 0;
                    ap2->sreg = ap1->preg;
                    return ap2;
                }
                freeop(ap2);
                ap = temp_addr();
                gen_lea(0, ap2, ap);
                ap->mode = am_baseindxdata;
                ap->sreg = ap1->preg;
                ap->scale = 0;
                ap->offset = makeintnode(en_icon, 0);
                return ap;
            case am_ind:
                ap2->mode = am_baseindxdata;
                ap2->offset = makeintnode(en_icon, 0);
                ap2->scale = 0;
                ap2->sreg = ap1->preg;
                return ap2;
            case am_baseindxaddr:
            case am_baseindxdata:
                if (ap2->preg ==  - 1)
                {
                    ap = temp_addr();
                    gen_codes(op_move, BESZ_DWORD, ap1, ap);
                    ap2->preg = ap->preg;
                    return ap2;
                }
                freeop(ap2);
                ap = temp_addr();
                gen_lea(0, ap2, ap);
                ap->mode = am_baseindxdata;
                ap->scale = 0;
                ap->offset = makeintnode(en_icon, 0);
                ap->sreg = ap1->preg;
                return ap;
            }
            break;
        case am_adirect:
            switch (ap2->mode)
            {
            case am_areg:
                if ((!prm_largedata && (prm_datarel || !prm_smalldata)) ||
                    prm_68020)
                {
                    ap2->mode = am_indx;
                    ap2->offset = ap1->offset;
                    return ap2;
                }
                ap = temp_addr();
                gen_lea(0, ap1, ap);
                ap2->mode = am_baseindxaddr;
                ap2->offset = makeintnode(en_icon, 0);
                ap2->scale = 0;
                ap2->sreg = ap->preg;
                return ap2;
            case am_dreg:
                if ((!prm_largedata && (prm_datarel || !prm_smalldata)) ||
                    prm_68020)
                {
                    freeop(ap1);
                    ap = temp_addr();
                    gen_codes(op_move, BESZ_DWORD, ap2, ap);
                    ap->mode = am_indx;
                    ap->offset = ap1->offset;
                    return ap;
                }
                ap = temp_addr();
                gen_lea(0, ap1, ap);
                ap->mode = am_baseindxdata;
                ap->offset = makeintnode(en_icon, 0);
                ap->scale = 0;
                ap->sreg = ap2->preg;
                return ap;
            case am_adirect:
                ap1->offset = makenode(en_add, ap1->offset, ap2->offset);
                return ap1;
            case am_immed:
                ap1->offset = makenode(en_add, ap1->offset, ap2->offset);
                return ap1;
            case am_indx:
                if ((!prm_largedata && (prm_datarel || !prm_smalldata)) ||
                    prm_68020)
                {
                    ap2->offset = makenode(en_add, ap2->offset, ap1->offset);
                    return ap2;
                }
                ap = temp_addr();
                gen_lea(0, ap1, ap);
                ap2->mode = am_baseindxaddr;
                ap2->sreg = ap->preg;
                ap2->scale = 0;
                return ap2;
            case am_ind:
                if ((!prm_largedata && (prm_datarel || !prm_smalldata)) ||
                    prm_68020)
                {
                    ap2->offset = ap1->offset;
                    ap2->mode = am_indx;
                    return ap2;
                }
                ap = temp_addr();
                gen_lea(0, ap1, ap);
                ap2->mode = am_baseindxaddr;
                ap2->sreg = ap->preg;
                ap2->scale = 0;
                ap2->offset = makeintnode(en_icon, 0);
                return ap2;
            case am_baseindxaddr:
            case am_baseindxdata:
                if (prm_68020)
                {
                    ap2->offset = makenode(en_add, ap2->offset, ap1->offset);
                    return ap2;
                }
                if (ap2->preg ==  - 1)
                {
                    ap = temp_addr();
                    ap2->preg = ap->preg;
                    gen_lea(0, ap1, ap);
                    return ap2;
                }
                ap = temp_addr();
                gen_lea(0, ap1, ap);
                ap2->mode = am_baseindxaddr;
                ap2->sreg = ap->preg;
                ap2->scale = 0;
                ap2->offset = makeintnode(en_icon, 0);
                return ap2;
            }
            break;
        case am_immed:
            switch (ap2->mode)
            {
            case am_areg:
                if (prm_68020 || isamshort(ap1))
                {
                    ap2->mode = am_indx;
                    ap2->offset = ap1->offset;
                    return ap2;
                }
                ap = temp_data();
                gen_codes(op_move, BESZ_DWORD, ap1, ap);
                ap2->mode = am_baseindxdata;
                ap2->offset = makeintnode(en_icon, 0);
                ap2->scale = 0;
                ap2->sreg = ap->preg;
                return ap2;
            case am_dreg:
                if (prm_68020 || isamshort(ap1))
                {
                    freeop(ap2);
                    ap = temp_addr();
                    gen_codes(op_move, BESZ_DWORD, ap2, ap);
                    ap->mode = am_indx;
                    ap->offset = ap1->offset;
                    return ap;
                }
                ap = temp_addr();
                gen_codes(op_move, BESZ_DWORD, ap1, ap);
                ap->mode = am_baseindxdata;
                ap->offset = makeintnode(en_icon, 0);
                ap->scale = 0;
                ap->sreg = ap2->preg;
                return ap;
            case am_adirect:
                ap2->offset = makenode(en_add, ap2->offset, ap1->offset);
                return ap2;
            case am_immed:
                if (prm_68020 || isamshort2(ap1, ap2))
                {
                    ap1->offset->v.i += ap2->offset->v.i;
                    return ap1;
                }
                if (isamshort(ap1))
                {
                    ap = temp_addr();
                    gen_codes(op_move, BESZ_DWORD, ap2, ap);
                    ap->mode = am_indx;
                    ap->offset = ap1->offset;
                    return ap;
                }
                if (isamshort(ap2))
                {
                    ap = temp_addr();
                    gen_codes(op_move, BESZ_DWORD, ap1, ap);
                    ap->mode = am_indx;
                    ap->offset = ap2->offset;
                    return ap;
                }
                ap = temp_addr();
                ap3 = temp_data();
                gen_codes(op_move, BESZ_DWORD, ap1, ap);
                gen_codes(op_move, BESZ_DWORD, ap2, ap3);
                ap->mode = am_baseindxdata;
                ap->sreg = ap3->preg;
                ap->scale = 0;
                ap->offset = makeintnode(en_icon, 0);
                return ap;
            case am_indx:
                if (prm_68020 || isamshort2(ap1, ap2))
                {
                    if (ap2->offset->nodetype == en_icon)
                        ap2->offset->v.i += ap1->offset->v.i;
                    else
                        ap2->offset = makenode(en_add, ap2->offset, ap1->offset)
                            ;
                    return ap2;
                }
                ap = temp_data();
                gen_codes(op_move, BESZ_DWORD, ap2, ap);
                ap1->mode = am_baseindxdata;
                ap1->scale = 0;
                ap1->sreg = ap->preg;
                return ap1;
            case am_baseindxaddr:
            case am_baseindxdata:
                if (ap2->preg ==  - 1 && !prm_68020)
                {
                    ap = temp_addr();
                    gen_codes(op_move, BESZ_DWORD, ap1, ap);
                    ap2->preg = ap->preg;
                    return ap2;
                }
                if (prm_68020 || isambyte2(ap1, ap2))
                {
                    if (ap2->offset->nodetype == am_immed)
                        ap2->offset->v.i += ap1->offset->v.i;
                    else
                        ap2->offset = makenode(en_add, ap2->offset, ap1->offset)
                            ;
                    return ap2;
                }
                freeop(ap2);
                ap = temp_addr();
                gen_lea(0, ap2, ap);
                ap->mode = am_ind;
                ap2 = ap;
                /* drop through */
            case am_ind:
                if (prm_68020 || isamshort(ap1))
                {
                    ap2->offset = ap1->offset;
                    ap2->mode = am_indx;
                    return ap2;
                }
                ap = temp_data();
                gen_codes(op_move, BESZ_DWORD, ap1, ap);
                ap2->mode = am_baseindxdata;
                ap2->scale = 0;
                ap2->sreg = ap->preg;
                ap2->offset = makeintnode(en_icon, 0);
                return ap2;
            }
            break;
        case am_indx:
            switch (ap2->mode)
            {
            case am_areg:
                if (prm_68020 || isambyte(ap1))
                {
                    ap2->mode = am_baseindxaddr;
                    ap2->offset = ap1->offset;
                    ap2->scale = 0;
                    ap2->sreg = ap2->preg;
                    return ap2;
                }
                freeop(ap1);
                ap = temp_addr();
                gen_lea(0, ap1, ap);
                ap->mode = am_baseindxaddr;
                ap->sreg = ap2->preg;
                ap->scale = 0;
                ap->offset = makeintnode(en_icon, 0);
                return ap;
            case am_dreg:
                if (prm_68020 || isambyte(ap1))
                {
                    ap1->mode = am_baseindxdata;
                    ap1->scale = 0;
                    ap1->sreg = ap2->preg;
                    return ap1;
                }
                freeop(ap1);
                ap = temp_addr();
                gen_lea(0, ap1, ap);
                ap->mode = am_baseindxdata;
                ap->sreg = ap2->preg;
                ap->scale = 0;
                ap->offset = makeintnode(en_icon, 0);
                return ap;
            case am_adirect:
                if ((!prm_largedata && (prm_datarel || !prm_smalldata)) ||
                    prm_68020)
                {
                    ap1->offset = makenode(en_add, ap1->offset, ap2->offset);
                    return ap1;
                }
                ap = temp_addr();
                gen_lea(0, ap2, ap);
                ap1->mode = am_baseindxaddr;
                ap1->sreg = ap->preg;
                ap1->scale = 0;
                return ap1;
            case am_immed:
                if (prm_68020 || isamshort2(ap1, ap2))
                {
                    if (ap1->offset->nodetype == am_immed)
                        ap1->offset->v.i += ap2->offset->v.i;
                    else
                        ap1->offset = makenode(en_add, ap1->offset, ap2->offset)
                            ;
                    return ap1;
                }
                ap = temp_data();
                gen_codes(op_move, BESZ_DWORD, ap2, ap);
                ap1->mode = am_baseindxdata;
                ap1->scale = 0;
                ap1->sreg = ap->preg;
                return ap1;
            case am_indx:
                if (isambyte2(ap1, ap2) || prm_68020)
                {
                    if (ap1->offset->nodetype == am_immed && ap2->offset
                        ->nodetype)
                        ap1->offset->v.i += ap2->offset->v.i;
                    else
                        ap1->offset = makenode(en_add, ap1->offset, ap2->offset)
                            ;
                    ap1->mode = am_baseindxaddr;
                    ap1->sreg = ap2->preg;
                    ap1->scale = 0;
                    ap1->offset = makeintnode(en_icon, 0);
                    return ap1;
                }
                if (isambyte(ap1))
                {
                    freeop(ap2);
                    ap = temp_addr();
                    gen_lea(0, ap2, ap);
                    ap1->mode = am_baseindxaddr;
                    ap1->sreg = ap->preg;
                    ap1->scale = 0;
                    ap1->offset = makeintnode(en_icon, 0);
                    return ap1;
                }
                if (isambyte(ap2))
                {
                    freeop(ap1);
                    ap = temp_addr();
                    gen_lea(0, ap1, ap);
                    ap2->mode = am_baseindxaddr;
                    ap2->sreg = ap->preg;
                    ap2->scale = 0;
                    ap2->offset = makeintnode(en_icon, 0);
                    return ap2;
                }
                freeop(ap1);
                ap = temp_addr();
                gen_lea(0, ap1, ap);
                freeop(ap2);
                ap1 = temp_addr();
                gen_lea(0, ap2, ap1);
                ap1->sreg = ap->preg;
                ap1->mode = am_baseindxaddr;
                ap1->scale = 0;
                ap1->offset = makeintnode(en_icon, 0);
                return ap1;
            case am_ind:
                if (isambyte(ap1) || prm_68020)
                {
                    ap1->mode = am_baseindxaddr;
                    ap1->sreg = ap2->preg;
                    ap1->scale = 0;
                    return ap1;
                }
                freeop(ap1);
                ap = temp_addr();
                gen_lea(0, ap1, ap);
                ap2->sreg = ap->preg;
                ap2->mode = am_baseindxaddr;
                ap2->scale = 0;
                ap2->offset = makeintnode(en_icon, 0);
                return ap2;
            case am_baseindxaddr:
            case am_baseindxdata:
                if (prm_68020 || isambyte2(ap1, ap2))
                {
                    if (ap2->preg ==  - 1)
                    {
                        ap2->preg = ap1->preg;
                        if (ap2->offset->nodetype == am_immed && ap1->offset
                            ->nodetype == am_immed)
                            ap2->offset->v.i += ap1->offset->v.i;
                        else
                            ap2->offset = makenode(en_add, ap2->offset, ap1
                                ->offset);
                        return ap2;
                    }
                    freeop(ap2);
                    ap = temp_addr();
                    gen_lea(0, ap2, ap);
                    ap->mode = am_baseindxaddr;
                    ap->sreg = ap1->preg;
                    ap->scale = 0;
                    ap->offset = ap1->offset;
                    return ap;
                }
                if (ap2->preg ==  - 1)
                {
                    ap3 = xalloc(sizeof(AMODE));
                    ap3->preg = ap2->sreg;
                    if (ap2->mode == am_baseindxdata)
                    {
                        ap3->mode = F_DREG;
                        if (ap3->preg < cf_freedata)
                            ap3->tempflag = 1;
                    }
                    else
                    {
                        ap3->mode = F_AREG;
                        if (ap3->preg < cf_freeaddress)
                            ap3->tempflag = 1;
                    }
                    if (ap2->scale)
                    {
                        do_extend(ap3, BESZ_DWORD, F_DREG | F_VOL);
                        gen_codes(op_asl, BESZ_DWORD, ap3, make_immed(ap2->scale));
                        do_extend(ap3, BESZ_DWORD, F_AREG | F_VOL);
                    }
                    else
                        do_extend(ap3, BESZ_DWORD, F_AREG);
                    ap2->preg = ap3->preg;
                    ap2->mode = am_indx;
                    return ap2;
                }
                freeop(ap2);
                ap = temp_addr();
                gen_lea(0, ap2, ap);
                ap2 = temp_data();
                gen_codes(op_move, BESZ_DWORD, ap, ap2);
                freeop(ap1);
                gen_lea(0, ap1, ap);
                ap->mode = am_baseindxdata;
                ap->sreg = ap2->preg;
                ap->scale = 0;
                ap->offset = makeintnode(en_icon, 0);
                return ap;
            }
            break;
        case am_ind:
            switch (ap2->mode)
            {
            case am_areg:
                ap2->mode = am_baseindxaddr;
                ap2->offset = makeintnode(en_icon, 0);
                ap2->scale = 0;
                ap2->sreg = ap1->preg;
                return ap2;
            case am_dreg:
                ap1->mode = am_baseindxdata;
                ap1->offset = makeintnode(en_icon, 0);
                ap1->scale = 0;
                ap1->sreg = ap2->preg;
                return ap1;
            case am_adirect:
                if ((!prm_largedata && (prm_datarel || !prm_smalldata)) ||
                    prm_68020)
                {
                    ap1->offset = ap2->offset;
                    ap1->mode = am_indx;
                    return ap1;
                }
                ap = temp_addr();
                gen_lea(0, ap2, ap);
                ap1->mode = am_baseindxaddr;
                ap1->sreg = ap->preg;
                ap1->scale = 0;
                ap1->offset = makeintnode(en_icon, 0);
                return ap1;
            case am_immed:
                if (prm_68020 || isamshort(ap2))
                {
                    ap1->offset = ap2->offset;
                    ap1->mode = am_indx;
                    return ap1;
                }
                ap = temp_data();
                gen_codes(op_move, BESZ_DWORD, ap2, ap);
                ap1->mode = am_baseindxdata;
                ap1->scale = 0;
                ap1->sreg = ap->preg;
                ap1->offset = makeintnode(en_icon, 0);
                return ap1;
            case am_indx:
                if (isambyte(ap2) || prm_68020)
                {
                    ap2->mode = am_baseindxaddr;
                    ap2->sreg = ap1->preg;
                    return ap2;
                }
                freeop(ap2);
                ap = temp_addr();
                gen_lea(0, ap2, ap);
                ap1->sreg = ap->preg;
                ap1->mode = am_baseindxaddr;
                ap1->scale = 0;
                ap1->offset = makeintnode(en_icon, 0);
                return ap1;
            case am_ind:
                ap1->mode = am_baseindxaddr;
                ap1->sreg = ap2->preg;
                ap1->scale = 0;
                ap1->offset = makeintnode(en_icon, 0);
                return ap1;
            case am_baseindxaddr:
            case am_baseindxdata:
                if (ap2->preg ==  - 1)
                {
                    ap2->preg = ap1->preg;
                    return ap2;
                }
                freeop(ap2);
                ap = temp_addr();
                gen_lea(0, ap2, ap);
                ap->mode = am_baseindxaddr;
                ap->sreg = ap1->preg;
                ap->scale = 0;
                ap->offset = makeintnode(en_icon, 0);
                return ap;
            }
            break;
        case am_baseindxaddr:
            switch (ap2->mode)
            {
            case am_areg:
                if (ap1->preg ==  - 1)
                {
                    ap1->preg = ap2->preg;
                    return ap1;
                }
                freeop(ap1);
                ap = temp_addr();
                gen_lea(0, ap1, ap);
                ap2->mode = am_baseindxaddr;
                ap2->scale = 0;
                ap2->offset = makeintnode(en_icon, 0);
                ap2->sreg = ap->preg;
                return ap2;
            case am_dreg:
                if (ap1->preg ==  - 1)
                {
                    ap = temp_addr();
                    gen_codes(op_move, BESZ_DWORD, ap2, ap);
                    ap1->preg = ap->preg;
                    return ap1;
                }
                freeop(ap1);
                ap = temp_addr();
                gen_lea(0, ap1, ap);
                ap->mode = am_baseindxdata;
                ap->scale = 0;
                ap->offset = makeintnode(en_icon, 0);
                ap->sreg = ap2->preg;
                return ap;
            case am_adirect:
                if (prm_68020)
                {
                    ap1->offset = makenode(en_add, ap1->offset, ap2->offset);
                    return ap1;
                }
                if (ap1->preg ==  - 1)
                {
                    ap = temp_addr();
                    ap1->preg = ap->preg;
                    gen_lea(0, ap2, ap);
                    return ap1;
                }
                ap = temp_addr();
                gen_lea(0, ap2, ap);
                ap1->mode = am_baseindxaddr;
                ap1->sreg = ap->preg;
                ap1->scale = 0;
                ap1->offset = makeintnode(en_icon, 0);
                return ap1;
            case am_immed:
                if (!prm_68020 && ap1->preg ==  - 1)
                {
                    ap = temp_addr();
                    gen_codes(op_move, BESZ_DWORD, ap2, ap);
                    ap1->preg = ap->preg;
                    return ap1;
                }
                if (prm_68020 || isambyte2(ap1, ap2))
                {
                    if (ap1->offset->nodetype == am_immed)
                        ap1->offset->v.i += ap2->offset->v.i;
                    else
                        ap1->offset = makenode(en_add, ap1->offset, ap2->offset)
                            ;
                    return ap1;
                }
                freeop(ap1);
                ap = temp_addr();
                gen_lea(0, ap1, ap);
                if ((!prm_largedata && (prm_datarel || !prm_smalldata)) ||
                    prm_68020)
                {
                    ap->mode = am_indx;
                    ap->offset = ap2->offset;
                    return ap;
                }
                ap1 = temp_data();
                ap1->mode = am_baseindxdata;
                ap1->sreg = ap->preg;
                ap1->scale = 0;
                ap1->offset = makeintnode(en_icon, 0);
                return ap1;
            case am_indx:
                if (prm_68020 || isambyte2(ap1, ap2))
                {
                    if (ap1->preg ==  - 1)
                    {
                        ap1->preg = ap2->preg;
                        if (ap2->offset->nodetype == am_immed && ap1->offset
                            ->nodetype == am_immed)
                            ap1->offset->v.i += ap2->offset->v.i;
                        else
                            ap1->offset = makenode(en_add, ap1->offset, ap2
                                ->offset);
                        return ap1;
                    }
                    freeop(ap1);
                    ap = temp_addr();
                    gen_lea(0, ap1, ap);
                    ap->mode = am_baseindxaddr;
                    ap->sreg = ap2->preg;
                    ap->scale = 0;
                    ap->offset = ap2->offset;
                    return ap;
                }
                if (ap1->preg ==  - 1)
                {
                    ap3 = xalloc(sizeof(AMODE));
                    ap3->preg = ap1->sreg;
                    if (ap1->mode == am_baseindxdata)
                    {
                        ap3->mode = F_DREG;
                        if (ap3->preg < cf_freedata)
                            ap3->tempflag = 1;
                    }
                    else
                    {
                        ap3->mode = F_AREG;
                        if (ap3->preg < cf_freeaddress)
                            ap3->tempflag = 1;
                    }
                    if (ap1->scale)
                    {
                        do_extend(ap3, BESZ_DWORD, F_DREG | F_VOL);
                        gen_codes(op_asl, BESZ_DWORD, ap3, make_immed(ap1->scale));
                        do_extend(ap3, BESZ_DWORD, F_AREG | F_VOL);
                    }
                    else
                        do_extend(ap3, BESZ_DWORD, F_AREG);
                    ap1->preg = ap3->preg;
                    ap1->mode = am_indx;
                    return ap1;
                }
                freeop(ap1);
                ap = temp_addr();
                gen_lea(0, ap1, ap);
                ap1 = temp_data();
                gen_codes(op_move, BESZ_DWORD, ap, ap1);
                freeop(ap2);
                ap = temp_addr();
                gen_lea(0, ap2, ap);
                ap->mode = am_baseindxdata;
                ap->sreg = ap1->preg;
                ap->scale = 0;
                ap->offset = makeintnode(en_icon, 0);
                return ap;
            case am_ind:
                if (ap1->preg ==  - 1)
                {
                    ap1->preg = ap2->preg;
                    return ap1;
                }
                freeop(ap1);
                ap = temp_addr();
                gen_lea(0, ap1, ap);
                ap->mode = am_baseindxaddr;
                ap->sreg = ap2->preg;
                ap->scale = 0;
                ap->offset = ap1->offset;
                return ap;
            case am_baseindxaddr:
            case am_baseindxdata:
                if (prm_68020 || (ap1->preg !=  - 1 && ap2->preg !=  - 1))
                {
                    freeop(ap1);
                    ap = temp_addr();
                    gen_lea(0, ap1, ap);
                    ap1 = temp_data();
                    gen_codes(op_move, BESZ_DWORD, ap, ap1);
                    freeop(ap2);
                    gen_lea(0, ap2, ap);
                    ap->mode = am_baseindxdata;
                    ap->sreg = ap1->preg;
                    ap->scale = 0;
                    ap->offset = makeintnode(en_icon, 0);
                    return ap;
                }
                if (ap1->preg ==  - 1)
                {
                    ap3 = xalloc(sizeof(AMODE));
                    ap3->preg = ap1->sreg;
                    if (ap1->mode == am_baseindxdata)
                    {
                        ap3->mode = F_DREG;
                        if (ap3->preg < cf_freedata)
                            ap3->tempflag = 1;
                    }
                    else
                    {
                        ap3->mode = F_AREG;
                        if (ap3->preg < cf_freeaddress)
                            ap3->tempflag = 1;
                    }
                    if (ap1->scale)
                    {
                        do_extend(ap3, BESZ_DWORD, F_DREG | F_VOL);
                        gen_codes(op_asl, BESZ_DWORD, ap3, make_immed(ap1->scale));
                        do_extend(ap3, BESZ_DWORD, F_AREG | F_VOL);
                    }
                    else
                        do_extend(ap3, BESZ_DWORD, F_AREG);
                    ap1->preg = ap3->preg;
                    ap1->mode = am_indx;
                }
                if (ap2->preg ==  - 1)
                {
                    ap3 = xalloc(sizeof(AMODE));
                    ap3->preg = ap2->sreg;
                    if (ap2->mode == am_baseindxdata)
                    {
                        ap3->mode = F_DREG;
                        if (ap3->preg < cf_freedata)
                            ap3->tempflag = 1;
                    }
                    else
                    {
                        ap3->mode = F_AREG;
                        if (ap3->preg < cf_freeaddress)
                            ap3->tempflag = 1;
                    }
                    if (ap2->scale)
                    {
                        do_extend(ap3, BESZ_DWORD, F_DREG | F_VOL);
                        gen_codes(op_asl, BESZ_DWORD, ap3, make_immed(ap2->scale));
                        do_extend(ap3, BESZ_DWORD, F_AREG | F_VOL);
                    }
                    else
                        do_extend(ap3, BESZ_DWORD, F_AREG);
                    ap2->preg = ap3->preg;
                    ap2->mode = am_indx;
                }
                goto tryagain;
            }
            break;
        case am_baseindxdata:
            switch (ap2->mode)
            {
            case am_areg:
                if (ap1->preg ==  - 1)
                {
                    ap1->preg = ap2->preg;
                    return ap1;
                }
                freeop(ap1);
                ap = temp_addr();
                gen_lea(0, ap1, ap);
                ap2->mode = am_baseindxaddr;
                ap2->scale = 0;
                ap2->offset = makeintnode(en_icon, 0);
                ap2->sreg = ap->preg;
                return ap2;
            case am_dreg:
                if (ap1->preg ==  - 1)
                {
                    ap = temp_addr();
                    gen_codes(op_move, BESZ_DWORD, ap2, ap);
                    ap1->preg = ap->preg;
                    return ap1;
                }
                freeop(ap1);
                ap = temp_addr();
                gen_lea(0, ap1, ap);
                ap->mode = am_baseindxdata;
                ap->scale = 0;
                ap->offset = makeintnode(en_icon, 0);
                ap->sreg = ap2->preg;
                return ap;
            case am_adirect:
                if (prm_68020)
                {
                    ap1->offset = makenode(en_add, ap1->offset, ap2->offset);
                    return ap1;
                }
                if (ap1->preg ==  - 1)
                {
                    ap = temp_addr();
                    ap1->preg = ap->preg;
                    gen_lea(0, ap2, ap);
                    return ap1;
                }
                ap = temp_addr();
                gen_lea(0, ap2, ap);
                ap1->mode = am_baseindxaddr;
                ap1->sreg = ap->preg;
                ap1->scale = 0;
                ap1->offset = makeintnode(en_icon, 0);
                return ap1;
            case am_immed:
                if (!prm_68020 && ap1->preg ==  - 1)
                {
                    ap = temp_addr();
                    gen_codes(op_move, BESZ_DWORD, ap2, ap);
                    ap1->preg = ap->preg;
                    return ap1;
                }
                if (prm_68020 || isambyte2(ap1, ap2))
                {
                    if (ap1->offset->nodetype == am_immed)
                        ap1->offset->v.i += ap2->offset->v.i;
                    else
                        ap1->offset = makenode(en_add, ap1->offset, ap2->offset)
                            ;
                    return ap1;
                }
                freeop(ap1);
                ap = temp_addr();
                gen_lea(0, ap1, ap);
                if ((!prm_largedata && (prm_datarel || !prm_smalldata)) ||
                    prm_68020)
                {
                    ap->mode = am_indx;
                    ap->offset = ap2->offset;
                    return ap;
                }
                ap1 = temp_data();
                ap1->mode = am_baseindxdata;
                ap1->sreg = ap->preg;
                ap1->scale = 0;
                ap1->offset = makeintnode(en_icon, 0);
                return ap1;
            case am_indx:
                if (prm_68020 || isambyte2(ap1, ap2))
                {
                    if (ap1->preg ==  - 1)
                    {
                        ap1->preg = ap2->preg;
                        if (ap2->offset->nodetype == am_immed && ap1->offset
                            ->nodetype == am_immed)
                            ap1->offset->v.i += ap2->offset->v.i;
                        else
                            ap1->offset = makenode(en_add, ap1->offset, ap2
                                ->offset);
                        return ap1;
                    }
                    freeop(ap1);
                    ap = temp_addr();
                    gen_lea(0, ap1, ap);
                    ap->mode = am_baseindxaddr;
                    ap->sreg = ap2->preg;
                    ap->scale = 0;
                    ap->offset = ap2->offset;
                    return ap;
                }
                if (ap1->preg ==  - 1)
                {
                    ap3 = xalloc(sizeof(AMODE));
                    ap3->preg = ap1->sreg;
                    if (ap1->mode == am_baseindxdata)
                    {
                        ap3->mode = F_DREG;
                        if (ap3->preg < cf_freedata)
                            ap3->tempflag = 1;
                    }
                    else
                    {
                        ap3->mode = F_AREG;
                        if (ap3->preg < cf_freeaddress)
                            ap3->tempflag = 1;
                    }
                    if (ap3->preg < cf_freedata)
                        ap3->tempflag = 1;
                    if (ap1->scale)
                    {
                        do_extend(ap3, BESZ_DWORD, F_DREG | F_VOL);
                        gen_codes(op_asl, BESZ_DWORD, ap3, make_immed(ap1->scale));
                        do_extend(ap3, BESZ_DWORD, F_AREG | F_VOL);
                    }
                    else
                        do_extend(ap3, BESZ_DWORD, F_AREG);
                    ap1->preg = ap3->preg;
                    ap1->mode = am_indx;
                    return ap1;
                }
                freeop(ap1);
                ap = temp_addr();
                gen_lea(0, ap1, ap);
                ap1 = temp_data();
                gen_codes(op_move, BESZ_DWORD, ap, ap1);
                freeop(ap2);
                gen_lea(0, ap2, ap);
                ap->mode = am_baseindxdata;
                ap->sreg = ap1->preg;
                ap->scale = 0;
                ap->offset = makeintnode(en_icon, 0);
                return ap;
            case am_ind:
                if (ap1->preg ==  - 1)
                {
                    ap1->preg = ap2->preg;
                    return ap1;
                }
                freeop(ap1);
                ap = temp_addr();
                gen_lea(0, ap1, ap);
                ap->mode = am_baseindxaddr;
                ap->sreg = ap2->preg;
                ap->scale = 0;
                ap->offset = ap1->offset;
                return ap;
            case am_baseindxaddr:
            case am_baseindxdata:
                if (prm_68020 || (ap1->preg !=  - 1 && ap2->preg !=  - 1))
                {
                    freeop(ap1);
                    ap = temp_addr();
                    gen_lea(0, ap1, ap);
                    ap1 = temp_data();
                    gen_codes(op_move, BESZ_DWORD, ap, ap1);
                    freeop(ap2);
                    gen_lea(0, ap2, ap);
                    ap->mode = am_baseindxdata;
                    ap->sreg = ap->preg;
                    ap->scale = 0;
                    ap->offset = makeintnode(en_icon, 0);
                    return ap;
                }
                if (ap1->preg ==  - 1)
                {
                    ap3 = xalloc(sizeof(AMODE));
                    ap3->preg = ap1->sreg;
                    if (ap1->mode == am_baseindxdata)
                    {
                        ap3->mode = F_DREG;
                        if (ap3->preg < cf_freedata)
                            ap3->tempflag = 1;
                    }
                    else
                    {
                        ap3->mode = F_AREG;
                        if (ap3->preg < cf_freeaddress)
                            ap3->tempflag = 1;
                    }
                    if (ap1->scale)
                    {
                        do_extend(ap3, BESZ_DWORD, F_DREG | F_VOL);
                        gen_codes(op_asl, BESZ_DWORD, ap3, make_immed(ap1->scale));
                        do_extend(ap3, BESZ_DWORD, F_AREG | F_VOL);
                    }
                    else
                        do_extend(ap3, BESZ_DWORD, F_AREG);
                    ap1->preg = ap3->preg;
                    ap1->mode = am_indx;
                }
                if (ap2->preg ==  - 1)
                {
                    ap3 = xalloc(sizeof(AMODE));
                    ap3->preg = ap2->sreg;
                    if (ap2->mode == am_baseindxdata)
                    {
                        ap3->mode = F_DREG;
                        if (ap3->preg < cf_freedata)
                            ap3->tempflag = 1;
                    }
                    else
                    {
                        ap3->mode = F_AREG;
                        if (ap3->preg < cf_freeaddress)
                            ap3->tempflag = 1;
                    }
                    if (ap2->scale)
                    {
                        do_extend(ap3, BESZ_DWORD, F_DREG | F_VOL);
                        gen_codes(op_asl, BESZ_DWORD, ap3, make_immed(ap2->scale));
                        do_extend(ap3, BESZ_DWORD, F_AREG | F_VOL);
                    }
                    else
                        do_extend(ap3, BESZ_DWORD, F_AREG);
                    ap2->preg = ap3->preg;
                    ap2->mode = am_indx;
                }
                goto tryagain;
            }
            break;
    }
    DIAG("invalid index conversion");
}

//-------------------------------------------------------------------------

AMODE *gen_deref(ENODE *node, int size)
/*
 *      return the addressing mode of a dereferenced node.
 */
{
    AMODE *ap1,  *ap2;
    int siz1;
    switch (node->nodetype) /* get load size */
    {
        case en_substack:
        case en_structret:
            siz1 = BESZ_DWORD;
            break;
        case en_ll_ref:
            siz1 =  - 6;
            break;
        case en_ull_ref:
            siz1 = 6;
            break;
        case en_ub_ref:
            siz1 = BESZ_BYTE;
            break;
        case en_b_ref:
            siz1 =  - BESZ_BYTE;
            break;
        case en_bool_ref:
            siz1 = 5;
            break;
        case en_uw_ref:
            siz1 = 2;
            break;
        case en_w_ref:
            siz1 =  - 2;
            break;
        case en_l_ref:
		case en_i_ref:
		case en_a_ref:
            siz1 =  - BESZ_DWORD;
            break;
        case en_add:
        case en_addstruc:
        case en_ul_ref:
		case en_ui_ref:
		case en_ua_ref:
            siz1 = BESZ_DWORD;
            break;
        case en_floatref:
            siz1 = 7;
            break;
        case en_doubleref:
            siz1 = 8;
            break;
        case en_longdoubleref:
            siz1 = 10;
            break;
        case en_fimaginaryref:
            siz1 = 15;
            break;
        case en_rimaginaryref:
            siz1 = 16;
            break;
        case en_lrimaginaryref:
            siz1 = 17;
            break;
        case en_fcomplexref:
            siz1 = 20;
            break;
        case en_rcomplexref:
            siz1 = 21;
            break;
        case en_lrcomplexref:
            siz1 = 22;
            break;
        default:
            siz1 = BESZ_DWORD;
    }
    if (node->v.p[0]->nodetype == en_add || node->v.p[0]->nodetype ==
        en_addstruc)
    {
        ap1 = gen_index(node->v.p[0]);
        ap1->length = siz1;
        return ap1;
    }
    else if (node->v.p[0]->nodetype == en_autocon || node->v.p[0]->nodetype ==
        en_autoreg)
    {
        ap1 = xalloc(sizeof(AMODE));
        ap1->mode = am_indx;
        if (prm_linkreg && !currentfunc->intflag)
        {
            ap1->preg = linkreg;
            ap1->offset = makeintnode(en_icon, ((SYM*)node->v.p[0]->v.p[0])
                ->value.i);
        }
        else if (((SYM*)node->v.p[0]->v.p[0])->funcparm)
        {
            if (prm_phiform || currentfunc->intflag)
            {
                ap1->preg = linkreg;
                ap1->offset = makeintnode(en_icon, ((SYM*)node->v.p[0]->v.p[0])
                    ->value.i);
            }
            else
            {
                ap1->preg = 7;
                ap1->offset = makeintnode(en_icon, (((SYM*)node->v.p[0]->v.p[0])
                    ->value.i + framedepth + stackdepth));
            }
                //									if ((currentfunc->value.classdata.cppflags & PF_MEMBER) &&
                //												!(currentfunc->value.classdata.cppflags & PF_STATIC) && ap1->offset->v.i > 0)
                //										ap1->offset->v.i += 4;
        }
        else
        {
            ap1->preg = 7;
            ap1->offset = makeintnode(en_icon, (((SYM*)node->v.p[0]->v.p[0])
                ->value.i + stackdepth + lc_maxauto));
        }
        ap1->length = siz1;
        return ap1;
    }
    else if (node->v.p[0]->nodetype == en_nacon)
    {
        ap1 = xalloc(sizeof(AMODE));
        if (prm_datarel)
        {
            ap1->preg = basereg;
            ap1->offset = makenode(node->v.p[0]->nodetype, ((SYM*)node->v.p[0]
                ->v.sp), 0);
            if (prm_largedata)
            {
                ap2 = temp_addr();
                ap1->mode = am_areg;
                gen_codes(op_move, BESZ_DWORD, ap1, ap2);
                ap1->mode = am_immed;
                gen_codes(op_add, BESZ_DWORD, ap1, ap2);
                ap1 = ap2;
                ap1->mode = am_ind;
            }
            else
            {
                ap1->mode = am_indx;
            }
        }
        else
        {
            ap1->mode = am_adirect;
            ap1->offset = makenode(node->v.p[0]->nodetype, ((SYM*)node->v.p[0]
                ->v.sp), 0);
            if (prm_smalldata)
                ap1->preg = 2;
            else
                ap1->preg = 4;
        }
        ap1->length = siz1;
        return ap1;
    }
    else if (node->v.p[0]->nodetype == en_nalabcon)
    {
        ap1 = xalloc(sizeof(AMODE));
        if (prm_datarel)
        {
            ap1->preg = basereg;
            ap1->offset = makenode(node->v.p[0]->nodetype, node->v.p[0]->v.sp,
                0);
            if (prm_largedata)
            {
                ap2 = temp_addr();
                ap1->mode = am_areg;
                gen_codes(op_move, BESZ_DWORD, ap1, ap2);
                ap1->mode = am_immed;
                gen_codes(op_add, BESZ_DWORD, ap1, ap2);
                ap1 = ap2;
                ap1->mode = am_ind;
            }
            else
            {
                ap1->mode = am_indx;
            }
        }
        else
        {
            ap1->mode = am_adirect;
            ap1->offset = makeintnode(node->v.p[0]->nodetype, node->v.p[0]->v.i)
                ;
            if (prm_smalldata)
                ap1->preg = 2;
            else
                ap1->preg = 4;
        }
        ap1->length = siz1;
        return ap1;
    }
    else if (node->v.p[0]->nodetype == en_labcon || node->v.p[0]->nodetype ==
        en_napccon)
    {
        ap1 = xalloc(sizeof(AMODE));
        if (prm_pcrel)
            ap1->mode = am_pcindx;
        else
        {
            ap1->mode = am_adirect;
            if (prm_smallcode)
                ap1->preg = 2;
            else
                ap1->preg = 4;
        }
        if (node->v.p[0]->nodetype == en_labcon)
            ap1->offset = makeintnode(node->v.p[0]->nodetype, node->v.p[0]->v.i)
                ;
        else
            ap1->offset = makenode(node->v.p[0]->nodetype, node->v.p[0]->v.sp,
                0);
        ap1->length = siz1;
        return ap1;
    }
    else if (node->v.p[0]->nodetype == en_absacon)
    {
        ap1 = xalloc(sizeof(AMODE));
        ap1->mode = am_adirect;
        ap1->preg = isshort(node->v.p[0]) ? 2 : 4;
        ap1->offset = makeintnode(en_absacon, ((SYM*)node->v.p[0]->v.p[0])
            ->value.i);
        ap1->length = siz1;
        return ap1;

    }
    else if (node->v.p[0]->nodetype == en_regref)
    {
        ap1 = gen_expr(node->v.p[0], FALSE, TRUE, BESZ_DWORD);
        return ap1;
    }

    ap1 = gen_expr(node->v.p[0], FALSE, TRUE, BESZ_DWORD); /* generate address */
    do_extend(ap1, BESZ_DWORD, F_AREG | F_IMMED);
    /* AINCDEC for example may return an indirect mode already */
    ap1->length = siz1;
    if (ap1->mode == am_areg || ap1->mode == am_immed)
    {
        if (ap1->mode == am_areg)
        {
            ap1->mode = am_ind;
            return ap1;
        }
        ap1->mode = am_direct;
    }
    return ap1;
}

//-------------------------------------------------------------------------

void get_size(AMODE *ap1, AMODE *ap2, int size, int flags)
{
    if (size > 6)
        size = BESZ_DWORD;
    if (chksize(size, ap1->length))
    {
        if (!chksize(size, ap2->length))
            size = ap2->length;
    }
    else
        if (chksize(ap1->length, ap2->length))
            size = ap1->length;
        else
            size = ap2->length;
    if (((ap1->length > 0 || ap2->length > 0) && size < 0) && size !=  - 11)
        size =  - size;
    if (size)
    {
        if (ap1->mode == am_immed && (ap1->length <= BESZ_DWORD || ap1->length == 6))
            ap1->length = size;
        else
            do_extend(ap1, size, flags);
        if (ap2->mode == am_immed && (ap2->length <= BESZ_DWORD || ap2->length == 6))
            ap2->length = size;
        else
            do_extend(ap2, size, flags);
    }
}

//-------------------------------------------------------------------------

void small_size(AMODE *ap1, AMODE *ap2, int size, int flags)
{
    if (ap1->length <= 6 && ap2->length <= 6 && chksize(ap2->length, ap1
        ->length))
    {
        if ((ap2->mode != am_dreg || ap2->preg < 4) && (ap2->length != 6 && ap2
            ->length !=  - 6))
            ap2->length = ap1->length;
        else
            do_extend(ap2, ap1->length, flags);
    }
    else
        get_size(ap1, ap2, size, flags);
}

//-------------------------------------------------------------------------

int resultsize(int n1, int n2)
{
    if (n1 < 0)
        n1 =  - n1;
    if (n2 < 0)
        n1 =  - n1;
}

//-------------------------------------------------------------------------

int assign_size(ENODE *node, int size)
{
    int rsize = natural_size(node);
    int neg = 0;
    if (rsize < 0)
        neg =  - 1;
    if (chksize(size, rsize))
        if (size < 6 && size !=  - 6)
            rsize = size;
        else if (rsize < 6 && rsize !=  - 6)
            rsize = 4 * neg;
        else if (rsize != 6 && rsize !=  - 6)
            rsize = size;

    return rsize;
}

//-------------------------------------------------------------------------

void resolve_binary(ENODE *node, AMODE **ap1, AMODE **ap2, int size)
{
    *ap2 = gen_expr(node->v.p[1], FALSE, FALSE, size);
    noids(*ap2);
    *ap1 = gen_expr(node->v.p[0], FALSE, FALSE, size);
    get_size(*ap1,  *ap2, size, 0);
}

//-------------------------------------------------------------------------

static int isbit(ENODE *node)
{
    return node->nodetype == en_bits;
}

//-------------------------------------------------------------------------

static int as_args(ENODE *node, AMODE **apr, AMODE **ap1, AMODE **ap2, int size)
{
    int rv = isbit(node->v.p[0]);
    int rsize = assign_size(node->v.p[1], size);
    *ap2 = gen_expr(node->v.p[1], FALSE, FALSE, rsize);
    noids(ap2);
    *apr = gen_expr(node->v.p[0], FALSE, TRUE, size);
    if (rv)
        *ap1 = gen_expr(node->v.p[0], FALSE, FALSE, size);
    else
        *ap1 = copy_addr(*apr);
    return rv;
}

//-------------------------------------------------------------------------

static int prefer(ENODE *node, AMODE **ap2, AMODE **ap3, int op, int dosizing,
    int size)
{
    AMODE *ap2x,  *ap3x;
    int rv = 0;
    int a = depth(node->v.p[1]) - depth(node->v.p[0]);
    if (a <= 2)
    {
        ap2x = gen_expr(node->v.p[0], FALSE, FALSE, size);
        noids(ap2x);
        ap3x = gen_expr(node->v.p[1], FALSE, FALSE, size);
        if (dosizing)
            get_size(ap2x, ap3x, size, 0);
    }
    else
    {
        ap3x = gen_expr(node->v.p[1], FALSE, FALSE, size);
        noids(ap3x);
        ap2x = gen_expr(node->v.p[0], FALSE, FALSE, size);
        if (dosizing)
            get_size(ap2x, ap3x, size, 0);
    }
    *ap3 = ap3x;
    *ap2 = ap2x;
    if (ap2x->mode != am_dreg || !ap2x->tempflag)
    {
        if (op != op_sub && op != op_divs && op != op_divu && op != op_divsl &&
            op != op_divul)
        {
            if (ap3x->mode == am_dreg && ap3x->tempflag)
            {
                *ap3 = ap2x;
                *ap2 = ap3x;
                rv = !rv;
            }
            else
            if (ap2x->mode == am_immed)
            {
                *ap3 = ap2x;
                *ap2 = ap3x;
                rv = !rv;
            }
        }
    }
    return rv;
}

//-------------------------------------------------------------------------

AMODE *gen_unary(ENODE *node, int op, int fop, int size)
/*
 *      generate code to evaluate a unary minus or complement.
 */
{
    AMODE *ap;
    int xchg;
    ap = gen_expr(node->v.p[0], FALSE, FALSE, size);
    if (size > 6)
    {
        do_extend(ap, 10, F_FREG | F_VOL);
        gen_code(fop, ap, 0);
    }
    else
    {
        if (!size)
            size = natural_size(node);
        if (size == 6 || size ==  - 6)
        {
            do_extend(ap, size, F_DOUBLEREG);
            gen_codes(op, BESZ_DWORD, makedreg(0), 0);
            gen_codes(op, BESZ_DWORD, makedreg(1), 0);
            if (op == op_neg)
            {
                int label = nextlabel++;
                gen_1branch(op_bcc, make_label(label));
                gen_code(op_subq, make_immed(1), makedreg(0));
                gen_label(label);
            }
        }
        else
        {
            do_extend(ap, prm_coldfire ? 4 : size, F_DREG | F_VOL);
            gen_code(op, ap, 0);
        }
    }
    return ap;
}

//-------------------------------------------------------------------------

AMODE *gen_fbinary(ENODE *node, int fop, int size, AMODE *apr)
{
    AMODE *ap1,  *ap2;
    int bits;
    if (apr)
        bits = as_args(node, apr, &ap1, &ap2, natural_size(node->v.p[0]));
    else
        prefer(node, &ap1, &ap2, fop, FALSE, size);
    tofloat(ap1, ap1->length);
    tofloat(ap2, ap2->length);
    do_extend(ap1, ap1->length, F_FREG | F_VOL);
    gen_code(fop, ap2, ap1);
    freeop(ap2);
    return ap1;
}

//-------------------------------------------------------------------------

AMODE *gen_binary(ENODE *node, int op, int fop, int size)
/*
 *      generate code to evaluate a binary node and return 
 *      the addressing mode of the result.
 */
{
    AMODE *ap1,  *ap2,  *ap3;

    if (natural_size(node->v.p[0]) <= 6 && natural_size(node->v.p[1]) <= 6)
    {
        ap1 = gen_xbin(node, op, 0, size);

    }
    else
    {
        ap1 = gen_fbinary(node, fop, size, 0);
    }
    return (ap1);
}

//-------------------------------------------------------------------------

AMODE *gen_6bin(ENODE *node, int op, int size)
{
    AMODE *ap1,  *ap2,  *ap3,  *ap4;
    int pushed = FALSE;

    ap1 = gen_expr(node->v.p[0], FALSE, FALSE, size);
    if (size < 6 && size >  - 6 && ap1->length != 6 && ap1->length !=  - 6)
        do_extend(ap1, size, F_DREG | F_VOL);
    else
        do_extend(ap1, 6, F_DOUBLEREG);
    ap2 = gen_expr(node->v.p[1], FALSE, FALSE, natural_size(node->v.p[1]));
    do_extend(ap2, size, 0);

    if (ap2->mode == am_immed)
    {
        ap4 = make_immed(ap2->offset->v.i &0xffffffffL);
        #if sizeof(ULLONG_TYPE) == 4
            ap3 = make_immed(ap2->offset->v.i < 0 ?  - 1: 0);
        #else 
            ap3 = make_immed(ap2->offset->v.i >> 32);
        #endif 
    }
    else if (ap2->mode == am_doublereg && ap1->mode == am_doublereg)
    {
        ap3 = make_stack(0);
        ap4 = make_stack( - 4);
    }
    else
    {
        ap2->length = 4;
        ap3 = ap2;
        ap4 = copy_addr(ap2);
        if (ap4->offset)
            ap4->offset = makenode(en_add, ap4->offset, makeintnode(en_icon, 4))
                ;
        else
            ap4->offset = makeintnode(en_icon, 4);
        if (ap4->mode == am_ind)
            ap4->mode = am_indx;
    }
    if (size && size < 6 && size >  - 6)
    {
        if (op == op_eor && ap2->mode != am_immed && ap2->mode != am_dreg)
            do_extend(ap2, size, F_DREG | F_VOL);
        gen_code(op, ap2, ap1);
    }
    else if (ap2->mode == am_doublereg && ap1->mode == am_doublereg)
    {
        gen_codes(op, BESZ_DWORD, makedreg(0), ap3);
        gen_codes(op, BESZ_DWORD, makedreg(1), ap4);
        if (op == op_add)
        {
            int label = nextlabel++;
            gen_1branch(op_bcc, make_label(label));
            gen_code(op_addq, make_immed(1), ap3);
            gen_label(label);
        }
        else if (op == op_sub)
        {
            int label = nextlabel++;
            gen_1branch(op_bcc, make_label(label));
            gen_code(op_subq, make_immed(1), ap3);
            gen_label(label);
        }
    }
    else
    {
        if (op == op_eor && ap3->mode != am_immed)
        {
            ap3->length = size;
            if ((ap3->mode == am_indx) && ap3->preg == 7)
            {
                ap3->offset = makenode(en_add, ap3->offset, makeintnode(en_icon,
                    8));
                ap4->offset = makenode(en_add, ap4->offset, makeintnode(en_icon,
                    8));
            }
            do_extend(ap3, size, F_DOUBLEREG);
            gen_codes(op, BESZ_DWORD, makedreg(0), make_stack(0));
            gen_codes(op, BESZ_DWORD, makedreg(1), make_stack( - 4));
            dregs[0]--;
            dregs[1]--;
            pushcount--;
            gen_pop(0, am_dreg, 0);
            gen_pop(1, am_dreg, 0);
            ap1 = ap2;
            ap2 = make_immed(0);
        }
        else
        {
            gen_codes(op, BESZ_DWORD, ap3, makedreg(0));
            gen_codes(op, BESZ_DWORD, ap4, makedreg(1));
        }
        if (op == op_add)
        {
            int label = nextlabel++;
            gen_1branch(op_bcc, make_label(label));
            gen_code(op_addq, make_immed(1), makedreg(0));
            gen_label(label);
        }
        else if (op == op_sub)
        {
            int label = nextlabel++;
            gen_1branch(op_bcc, make_label(label));
            gen_code(op_subq, make_immed(1), makedreg(0));
            gen_label(label);
        }
    }
    freeop(ap2);
    return ap1;
}

//-------------------------------------------------------------------------

AMODE *gen_xbin(ENODE *node, int op, int op2, int size)
/*
 *      generate code to evaluate a restricted binary node and return 
 *      the addressing mode of the result.
 */
{
    AMODE *ap1,  *ap2,  *ap3;
    int nsize = natural_size(node);
    if (nsize == 6 || nsize ==  - 6)
        return gen_6bin(node, op, size);
    prefer(node, &ap1, &ap2, op, TRUE, size);

    if (op == op_sub)
    {
        do_extend(ap1, ap1->length, F_DREG | F_VOL);
    }
    else
    {
        if ((ap1->mode != am_dreg) || !ap1->tempflag)
        {
            if (ap2->mode == am_dreg && ap2->tempflag)
            {
                ap3 = ap1;
                ap1 = ap2;
                ap2 = ap3;
            }
            else
            {
                int vb;
                do_extend(ap1, ap1->length, F_DREG | F_VOL);
                if (ap2->mode == am_immed)
                {
                    if (op == op_and && (vb = single_bit(~ap2->offset->v.i)) !=
                        - 1)
                    {
                        gen_codes(op2, BESZ_DWORD, make_immed(vb), ap1);
                        return ap1;
                    }
                    else if ((op == op_or || op == op_eor) && (vb = single_bit
                        (ap2->offset->v.i)) !=  - 1)
                    {
                        gen_codes(op2, BESZ_DWORD, make_immed(vb), ap1);
                        return ap1;
                    }
                }
            }
        }
    }
    if (prm_coldfire)
    {
        do_extend(ap1, ap1->length < 0 ?  - BESZ_DWORD: BESZ_DWORD, F_ALL &~F_AREG);
        do_extend(ap2, ap2->length < 0 ?  - BESZ_DWORD: BESZ_DWORD, F_ALL &~F_AREG);
    }
	if (op == op_eor)
	    do_extend(ap2, ap2->length, F_DREG);
    gen_codes(op, prm_coldfire ? BESZ_DWORD : ap1->length, ap2, ap1);
    freeop(ap2);
    return ap1;
}

//-------------------------------------------------------------------------

AMODE *gen_shift(ENODE *node, int op, int size, int div)
/*
 *      generate code to evaluate a shift node and return the
 *      address mode of the result.
 */
{
    AMODE *ap1,  *ap2;
    if (size == 6 || size ==  - 6)
        return do6shift(op, node, div, size);
    ap1 = gen_expr(node->v.p[0], FALSE, FALSE, size);
    noids(ap1);
    ap2 = gen_expr(node->v.p[1], FALSE, FALSE, size);
    if (prm_coldfire)
    {
        do_extend(ap1, ap1->length < 0 ?  - BESZ_DWORD: BESZ_DWORD, (F_ALL &~F_AREG) | F_VOL);
        do_extend(ap2, ap2->length < 0 ?  - BESZ_DWORD: BESZ_DWORD, F_ALL &~F_AREG);
    }
    else
        do_extend(ap1, size, F_DREG | F_VOL);
    doshift(op, ap2, ap1, div, ap1->length);
    if (size == 6 || size ==  - 6)
    {
        ap1->mode = am_doublereg;
        ap1->length = size;
    }
    return ap1;
}

//-------------------------------------------------------------------------

AMODE *do6div(ENODE *node, int op, int modflag, int size)
{
    AMODE *ap1;
    int regax, regdx, regcx;
    int pushed = 0;
    temp_doubledregs();
    if (dregs[2])
    {
        gen_push(2, am_dreg, FALSE);
        pushed = 1;
    }
    regax = dregs[0];
    regdx = dregs[1];
    regcx = dregs[2];
    //   set_func_mode(1) ;
    dregs[0] = dregs[1] = dregs[2] = 0;
    ap1 = gen_expr(node->v.p[1], FALSE, FALSE, size);
    if (ap1->mode != am_immed)
        do_extend(ap1, size, 0);
    noids(ap1);
    if (ap1->mode == am_doublereg)
    {
        gen_codes(op_move, BESZ_DWORD, makedreg(1), push);
        gen_codes(op_move, BESZ_DWORD, makedreg(0), push);
        dregs[1]--;
        dregs[0]--;
    }
    else if (ap1->mode == am_immed)
    {
        gen_codes(op_move, BESZ_DWORD, make_immed(ap1->offset->v.i &0xffffffffL), push);
        #if sizeof(ULLONG_TYPE) == 4
            gen_codes(op_move, BESZ_DWORD, make_immed(ap1->offset->v.i < 0 ?  - 1: 0),
                push);
        #else 
            gen_codes(op_move, BESZ_DWORD, make_immed(ap1->offset->v.i >> 32), push);
        #endif 
    }
    else
    {
        AMODE *ap2;
        ap1 = direct_data(ap1);
        ap2 = copy_addr(ap1);
        if (ap2->offset)
            ap2->offset = makenode(en_add, ap2->offset, makeintnode(en_icon, 4))
                ;
        else
            ap2->offset = makeintnode(en_icon, 4);
        if (ap2->mode == am_ind)
            ap2->mode = am_indx;
        gen_codes(op_move, BESZ_DWORD, ap2, push);
        gen_codes(op_move, BESZ_DWORD, ap1, push);
        freeop(ap1);
    }
    ap1 = gen_expr(node->v.p[0], FALSE, FALSE, size);
    if (ap1->mode != am_immed)
        do_extend(ap1, size, 0);
    noids(ap1);
    if (ap1->mode == am_doublereg)
    {
        gen_codes(op_move, BESZ_DWORD, makedreg(1), push);
        gen_codes(op_move, BESZ_DWORD, makedreg(0), push);
        dregs[1]--;
        dregs[0]--;
    }
    else if (ap1->mode == am_immed)
    {
        gen_codes(op_move, BESZ_DWORD, make_immed(ap1->offset->v.i &0xffffffffL), push);
        #if sizeof(ULLONG_TYPE) == 4
            gen_codes(op_move, BESZ_DWORD, make_immed(ap1->offset->v.i < 0 ?  - 1: 0),
                push);
        #else 
            gen_codes(op_move, BESZ_DWORD, make_immed(ap1->offset->v.i >> 32), push);
        #endif 
    }
    else
    {
        AMODE *ap2;
        ap1 = direct_data(ap1);
        ap2 = copy_addr(ap1);
        if (ap2->offset)
            ap2->offset = makenode(en_add, ap2->offset, makeintnode(en_icon, 4))
                ;
        else
            ap2->offset = makeintnode(en_icon, 4);
        if (ap2->mode == am_ind)
            ap2->mode = am_indx;
        gen_codes(op_move, BESZ_DWORD, ap2, push);
        gen_codes(op_move, BESZ_DWORD, ap1, push);
        freeop(ap1);
    }
    if (op == op_divs)
        if (modflag)
            call_library2("__LXMODS", 16);
        else
            call_library2("__LXDIVS", 16);
        else
            if (modflag)
                call_library2("__LXMODU", 16);
            else
                call_library2("__LXDIVU", 16);
    dregs[0] = regax;
    dregs[1] = regdx;
    dregs[2] = regcx;
    //   set_func_mode(0) ;
    if (pushed)
        gen_pop(2, am_dreg, FALSE);
    ap1 = xalloc(sizeof(AMODE));
    ap1->mode = am_doublereg;
    ap1->length = size;
    return ap1;
}

//-------------------------------------------------------------------------

AMODE *gen_modiv(ENODE *node, int op, int fop, int size, int modflag)
/*
 *      generate code to evaluate a mod operator or a divide
 *      operator. these operations are done on only long
 *      divisors and word dividends so that the 68000 div
 *      instruction can be used.
 */
{
    AMODE *ap1,  *ap2,  *ap3;
    int temp;
    int nsize = natural_size(node);
    if (size > 6)
    {
        return gen_fbinary(node, fop, size, 0);
    }
    if (size == 6 || size ==  - 6)
        return do6div(node, op, modflag, size);

    if (prm_68020 || prm_coldfire || nsize == 2 || nsize ==  - 2)
    {
        resolve_binary(node, &ap1, &ap2, size);
        if (prm_coldfire || nsize == BESZ_DWORD || nsize ==  - BESZ_DWORD)
        {
            do_extend(ap1, ap1->length < 0 ?  - BESZ_DWORD: BESZ_DWORD, F_DREG | F_VOL);
            do_extend(ap2, ap2->length < 0 ?  - BESZ_DWORD: BESZ_DWORD, prm_coldfire ? F_DIVL :
                F_ALL &~F_AREG);
            if (prm_coldfire)
            {
                if (modflag)
                    if (op == op_divs)
                        op = op_rems;
                    else
                        op = op_remu;
                gen_codes(op, BESZ_DWORD, ap2, ap1);
            }
            else
            {
                if (modflag)
                {
                    ap3 = temp_data();
                    ap1->mode = am_divsl;
                    ap1->sreg = ap3->preg;
                    gen_codes(op, BESZ_DWORD, ap2, ap1);
                    ap1->mode = am_dreg;
                    freeop(ap3);
                }
                else
                    gen_codes(op, BESZ_DWORD, ap2, ap1);
            }
        }
        else
        {
            do_extend(ap1, 2, F_DREG | F_VOL);
            do_extend(ap2, 2, prm_coldfire ? F_DIVL : F_ALL &~F_AREG);
            gen_codes(op, nsize, ap2, ap1);
            if (modflag)
                gen_codes(op_swap, 0, ap1, 0);
        }

        freeop(ap2);
        return ap1;

    }
    flush_for_libcall();
    ap1 = gen_expr(node->v.p[0], FALSE, FALSE, size);
    do_extend(ap1, ap1->length < 0 ?  - BESZ_DWORD: BESZ_DWORD, F_ALL);
    gen_codes(op_move, BESZ_DWORD, ap1, push);
    freeop(ap1);
    ap2 = gen_expr(node->v.p[1], FALSE, FALSE, size);
    do_extend(ap2, ap2->length < 0 ?  - BESZ_DWORD: BESZ_DWORD, F_ALL);
    gen_codes(op_move, BESZ_DWORD, ap2, push);
    freeop(ap2);
    if (op == op_divs)
    if (modflag)
    {
        ap1 = call_library("__lmods", 8);
    }
    else
    {
        ap1 = call_library("__ldivs", 8);
    }
    else
    if (modflag)
    {
        ap1 = call_library("__lmodu", 8);
    }
    else
    {
        ap1 = call_library("__ldivu", 8);
    }
    return ap1;
}

//-------------------------------------------------------------------------

void swap_nodes(ENODE *node)
/*
 *      exchange the two operands in a node.
 */
{
    ENODE *temp;
    temp = node->v.p[0];
    node->v.p[0] = node->v.p[1];
    node->v.p[1] = temp;
}

//-------------------------------------------------------------------------

AMODE *gen_pdiv(ENODE *node, int size)
{
    if (prm_68020)
        return gen_modiv(node, op_divul, op_fdiv, size, FALSE);
    else
        return gen_modiv(node, op_divu, op_fdiv, size, FALSE);
}

//-------------------------------------------------------------------------

AMODE *gen_pmul(ENODE *node, int size)
{
    AMODE *ap1,  *ap2;
    if (isintconst(node->v.p[0]->nodetype))
        swap_nodes(node);
    prefer(node, &ap1, &ap2, op_mulu, FALSE, prm_68020 || prm_coldfire ? size :
        2);
    do_extend(ap1, prm_68020 ? 4 : 2, F_DREG | F_VOL);
    gen_codes(op_muls, prm_68020 ? 4 : 2, ap2, ap1);
    freeop(ap2);
    return ap1;
}

//-------------------------------------------------------------------------

int do6mul(ENODE *node, int size)
{
    AMODE *ap1;
    int regax, regdx, regcx;
    int pushed = 0;
    temp_doubledregs();
    if (dregs[2])
    {
        gen_push(2, am_dreg, FALSE);
        pushed = 1;
    }
    regax = dregs[0];
    regdx = dregs[1];
    regcx = dregs[2];
    //   set_func_mode(1) ;
    dregs[0] = dregs[1] = dregs[2] = 0;
    ap1 = gen_expr(node->v.p[1], FALSE, FALSE, size);
    if (ap1->mode != am_immed)
        do_extend(ap1, size, 0);
    noids(ap1);
    if (ap1->mode == am_doublereg)
    {
        gen_codes(op_move, BESZ_DWORD, makedreg(1), push);
        gen_codes(op_move, BESZ_DWORD, makedreg(0), push);
        dregs[0]--;
        dregs[1]--;
    }
    else if (ap1->mode == am_immed)
    {
        gen_codes(op_move, BESZ_DWORD, make_immed(ap1->offset->v.i &0xffffffffL), push);
        #if sizeof(ULLONG_TYPE) == 4
            gen_codes(op_move, BESZ_DWORD, make_immed(ap1->offset->v.i < 0 ?  - 1: 0),
                push);
        #else 
            gen_codes(op_move, BESZ_DWORD, make_immed(ap1->offset->v.i >> 32), push);
        #endif 
    }
    else
    {
        AMODE *ap2;
        ap1 = direct_data(ap1);
        ap2 = copy_addr(ap1);
        if (ap2->offset)
            ap2->offset = makenode(en_add, ap2->offset, makeintnode(en_icon, 4))
                ;
        else
            ap2->offset = makeintnode(en_icon, 4);
        if (ap2->mode == am_ind)
            ap2->mode = am_indx;
        gen_codes(op_move, BESZ_DWORD, ap2, push);
        gen_codes(op_move, BESZ_DWORD, ap1, push);
        freeop(ap1);
    }
    ap1 = gen_expr(node->v.p[0], FALSE, FALSE, size);
    if (ap1->mode != am_immed)
        do_extend(ap1, size, 0);
    if (ap1->mode == am_doublereg)
    {
        gen_codes(op_move, BESZ_DWORD, makedreg(1), push);
        gen_codes(op_move, BESZ_DWORD, makedreg(0), push);
        dregs[0]--;
        dregs[1]--;
    }
    else if (ap1->mode == am_immed)
    {
        gen_codes(op_move, BESZ_DWORD, make_immed(ap1->offset->v.i &0xffffffffL), push);
        #if sizeof(ULLONG_TYPE) == 4
            gen_codes(op_move, BESZ_DWORD, make_immed(ap1->offset->v.i < 0 ?  - 1: 0),
                push);
        #else 
            gen_codes(op_move, BESZ_DWORD, make_immed(ap1->offset->v.i >> 32), push);
        #endif 
    }
    else
    {
        AMODE *ap2;
        ap1 = direct_data(ap1);
        ap2 = copy_addr(ap1);
        if (ap2->offset)
            ap2->offset = makenode(en_add, ap2->offset, makeintnode(en_icon, 4))
                ;
        else
            ap2->offset = makeintnode(en_icon, 4);
        if (ap2->mode == am_ind)
            ap2->mode = am_indx;
        gen_codes(op_move, BESZ_DWORD, ap2, push);
        gen_codes(op_move, BESZ_DWORD, ap1, push);
        freeop(ap1);
    }
    call_library2("__LXMUL", 16);
    dregs[0] = regax;
    dregs[1] = regdx;
    dregs[2] = regcx;
    //   set_func_mode(0) ;
    if (pushed)
        gen_pop(2, am_dreg, FALSE);
    ap1 = xalloc(sizeof(AMODE));
    ap1->mode = am_doublereg;
    ap1->length = size;
    return ap1;
}

//-------------------------------------------------------------------------

AMODE *gen_mul(ENODE *node, int op, int fop, int size)
/*
 *      generate code to evaluate a multiply node. both operands
 *      are treated as words and the result is long and is always
 *      in a register so that the 68000 mul instruction can be used.
 */
{
    AMODE *ap1,  *ap2;
    int nsize = natural_size(node);
    if (size > 6)
    {
        return gen_fbinary(node, fop, size, 0);
    }
    if (size == 6 || size ==  - 6)
        return do6mul(node, size);
    if (prm_68020 || prm_coldfire || nsize ==  - 2 || nsize == 2)
    {
        if (isintconst(node->v.p[0]->nodetype))
            swap_nodes(node);
        prefer(node, &ap1, &ap2, op, FALSE, size);
        do_extend(ap1, prm_coldfire ? (ap1->length < 0 ?  - BESZ_DWORD: BESZ_DWORD): size, F_DREG
            | F_VOL);
        do_extend(ap2, prm_coldfire ? (ap2->length < 0 ?  - BESZ_DWORD: BESZ_DWORD): size,
            prm_coldfire ? F_DIVL : F_DALT);
		if (size == 1 || size == -1)
			size = 2;
        gen_codes(op, prm_coldfire ? 4 : size, ap2, ap1);
        freeop(ap2);
        return ap1;
    }
    flush_for_libcall();
    if (isintconst(node->v.p[0]->nodetype))
        swap_nodes(node);
    ap1 = gen_expr(node->v.p[0], FALSE, FALSE, size);
    do_extend(ap1, ap1->length < 0 ?  - BESZ_DWORD: BESZ_DWORD, F_ALL);
    gen_codes(op_move, BESZ_DWORD, ap1, push);
    freeop(ap1);
    ap2 = gen_expr(node->v.p[1], FALSE, FALSE, size);
    do_extend(ap2, ap2->length < 0 ?  - BESZ_DWORD: BESZ_DWORD, F_ALL);
    gen_codes(op_move, BESZ_DWORD, ap2, push);
    freeop(ap2);
    if (op == op_muls)
    {
        ap1 = call_library("__lmuls", 8);
    }
    else
    {
        ap1 = call_library("__lmulu", 8);
    }
    return ap1;
}

//-------------------------------------------------------------------------

AMODE *gen_hook(ENODE *node, int size)
/*
 *      generate code to evaluate a condition operator node (?:)
 */
{
    AMODE *ap1,  *ap2;
    int false_label, end_label;
    int sizl, sizr, xsiz;
    sizr = natural_size(node->v.p[1]->v.p[1]);
    sizl = natural_size(node->v.p[1]->v.p[0]);
    sizl = sizl < 0 ?  - sizl: sizl;
    sizr = sizr < 0 ?  - sizr: sizr;
    if (sizl < sizr)
        xsiz = sizr;
    else
        xsiz = sizl;
    false_label = nextlabel++;
    end_label = nextlabel++;
    falsejp(node->v.p[0], false_label);
    node = node->v.p[1];
    ap1 = gen_expr(node->v.p[0], FALSE, FALSE, size);
    do_extend(ap1, xsiz, F_DREG | F_VOL);
    freeop(ap1);
    gen_codes(op_jmp, 0, make_label(end_label), 0);
    gen_label(false_label);
    ap2 = gen_expr(node->v.p[1], FALSE, FALSE, size);
    do_extend(ap2, xsiz, F_DREG | F_VOL);
    if (!equal_address(ap1, ap2))
    {
        /* if both are floating they won't get here */
        if (ap1->mode == am_freg)
        {
            do_extend(ap2, 10, F_FREG | F_VOL);
        }
        else
        {
            if (ap2->mode == am_freg)
            {
                if (ap1->length == 5 || ap1->length != 4 && ap1->length !=  - 4)
                    do_extend(ap1, BESZ_DWORD, F_DREG | F_VOL);
                gen_codef(op_fmove, ap1->length, ap2, ap1);
            }
            else
                gen_code(op_move, ap2, ap1);
            freeop(ap2);
            ap2->mode = ap1->mode;
            ap2->preg = ap1->preg;
            dregs[ap2->preg]++;
        }
    }
    gen_label(end_label);
    return ap2;
}

//-------------------------------------------------------------------------

AMODE *gen_asunary(ENODE *node, int novalue, int op, int fop, int size)
/*
 *      generate code to evaluate a unary minus or complement.
 */
{
    AMODE *ap,  *ap1;
    int nsize = natural_size(node->v.p[0]);
    ap = gen_expr(node->v.p[0], novalue, TRUE, nsize);
    if (nsize > 6)
    {
        AMODE ap1 =  *ap;
        tofloat(ap, ap->length);
        gen_codef(fop, ap->length, ap, 0);
        gen_codes(op_fmove, ap1.length, ap, &ap1);
    }
    else
    {
        if (ap->length == 6 || ap->length ==  - 6)
        {
            int oldlen = ap->length;
            AMODE *ap2 = copy_addr(ap);
            if (op == op_neg || op == op_not)
            {
                do_extend(ap, size, F_DOUBLEREG);
                gen_codes(op, BESZ_DWORD, makedreg(0), 0);
                gen_codes(op, BESZ_DWORD, makedreg(1), 0);
            }
            else
            {
                ap = direct_data(ap);
                gen_code(op, ap, 0);
                ap1 = copy_addr(ap);
                if (ap1->offset)
                    ap1->offset = makenode(en_add, ap1->offset, makeintnode
                        (en_icon, 4));
                else
                    ap1->offset = makeintnode(en_icon, 4);
                if (ap1->mode == am_ind)
                    ap1->mode = am_indx;
                gen_code(op, ap1, 0);
            }
            if (op == op_neg)
            {
                int label = nextlabel++;
                gen_1branch(op_bcc, make_label(label));
                gen_code(op_subq, make_immed(1), makedreg(0));
                gen_label(label);
            }
            if (op == op_neg || op == op_not)
            {
                gen_codes(op_move, BESZ_DWORD, makedreg(0), ap2);
                ap2->offset = makenode(en_add, ap2->offset, makeintnode(en_icon,
                    4));
                gen_codes(op_move, BESZ_DWORD, makedreg(1), ap2);
            }

            ap->length = oldlen;
        }
        else
        if (prm_coldfire && nsize != 4 && nsize !=  - 4)
        {
            ap1 = copy_addr(ap);
            do_extend(ap, BESZ_DWORD, F_DREG | F_VOL);
            gen_codes(op, BESZ_DWORD, ap1, 0);
            gen_codes(op_move, nsize, ap1, ap);
        }
        else
            gen_codes(op, nsize, ap, 0);
    }
    if (novalue)
        freeop(ap);
    return ap;
}

//-------------------------------------------------------------------------

AMODE *gen_asadd(ENODE *node, int novalue, int op, int fop, int size)
/*
 *      generate a plus equal or a minus equal node.
 */
{
    AMODE *ap1,  *ap2,  *apr;
    int bits;
    int label;
    int nsize = natural_size(node->v.p[0]);
    if (natural_size(node) > 6)
    {
        ap1 = gen_fbinary(node, fop, nsize, &apr);
        return floatstore(node->v.p[0], ap1, apr, novalue, natural_size(node));
    }
    if (nsize == 6 || nsize ==  - 6)
    {
        int pushed = FALSE, oldsize;
        int rsize = assign_size(node->v.p[1], size);
        apr = gen_expr(node->v.p[0], FALSE, TRUE, size);
        apr = direct_data(apr);
        ap2 = gen_expr(node->v.p[1], FALSE, FALSE, rsize);
        if (ap2->mode != am_immed)
            do_extend(ap2, 6, F_DOUBLEREG);
        oldsize = apr->length;
        apr->length = 4;
        if (ap2->mode == am_immed)
        {
            label = nextlabel++;
            #if sizeof(ULLONG_TYPE) == 4      
                gen_code(op, make_immed(ap2->offset->v.i < 0 ?  - 1: 0), apr);
            #else 
                gen_code(op, make_immed(ap2->offset->v.i >> 32), apr);
            #endif 
            ap1 = copy_addr(apr);
            ap1->offset = makenode(en_add, ap1->offset, makeintnode(en_icon, 4))
                ;
            gen_codes(op, BESZ_DWORD, make_immed(ap2->offset->v.i &0xffffffffUL), ap1);
            gen_1branch(op_bcc, make_label(label));
            gen_codes(op, BESZ_DWORD, make_immed(1), apr);
            gen_label(label);
        }
        else
        {
            label = nextlabel++;
            gen_codes(op, BESZ_DWORD, makedreg(0), apr);
            apr->mode = am_indx;
            ap1 = copy_addr(apr);
            gen_codes(op, BESZ_DWORD, makedreg(1), ap1);
            gen_1branch(op_bcc, make_label(label));
            gen_codes(op, BESZ_DWORD, make_immed(1), apr);
            gen_label(label);
        }
        apr->length = oldsize;
        if (novalue)
            freeop(apr);
        return apr;
    }
    bits = as_args(node, &apr, &ap1, &ap2, nsize);
    small_size(ap1, ap2, nsize, F_NOREUSE);
    if (!bits)
    {
        if (ap1->mode != am_dreg && ap2->mode != am_dreg && ap2->mode !=
            am_immed)
            do_extend(ap2, prm_coldfire ? 4 : ap1->length, F_DREG);
        if (prm_coldfire && nsize < 4 && nsize !=  - 4)
        {
            AMODE *ap3 = copy_addr(apr);
            do_extend(ap2, BESZ_DWORD, F_DREG | F_VOL);
            do_extend(ap3, BESZ_DWORD, F_DREG | F_VOL);
            gen_code(op, ap2, ap3);
            ap3->length = nsize;
            gen_code(op_move, ap3, apr);
        }
        else
            gen_code(op, ap2, apr);
        freeop(ap1);
        freeop(ap2);
        if (novalue)
            freeop(apr);
        return apr;
    }
    else
    {
        gen_code(op, ap2, ap1);
        bit_store(ap1, apr, node->v.p[0], novalue);
        freeop(ap2);
        freeop(apr);
        if (novalue)
            freeop(ap1);
        return ap1;
    }
}

//-------------------------------------------------------------------------

AMODE *gen_aslogic(ENODE *node, int novalue, int op, int op2, int size)
/*
 *      generate a and equal or a or equal node.
 */
{
    AMODE *ap1,  *ap2,  *apr,  *ap3;
    int bits = isbit(node->v.p[0]);
    int nsize = natural_size(node->v.p[0]);
    if (nsize == 6 || nsize ==  - 6)
    {
        int pushed = FALSE, oldsize;
        int rsize = assign_size(node->v.p[1], size);
        apr = gen_expr(node->v.p[0], FALSE, TRUE, size);
        apr = direct_data(apr);
        ap2 = gen_expr(node->v.p[1], FALSE, FALSE, rsize);
        if (ap2->mode != am_immed)
            do_extend(ap2, 6, F_DOUBLEREG);
        oldsize = apr->length;
        apr->length = 4;
        if (ap2->mode == am_immed)
        {
            #if sizeof(ULLONG_TYPE) == 4      
                gen_codes(op, BESZ_DWORD, make_immed(ap2->offset->v.i < 0 ?  - 1: 0),
                    apr);
            #else 
                gen_codes(op, BESZ_DWORD, make_immed(ap2->offset->v.i >> 32), apr);
            #endif 
            ap1 = copy_addr(apr);
            ap1->offset = makenode(en_add, ap1->offset, makeintnode(en_icon, 4))
                ;
            gen_codes(op, BESZ_DWORD, make_immed(ap2->offset->v.i &0xffffffffUL), ap1);
            apr->mode = am_ind;
        }
        else
        {
            gen_codes(op, BESZ_DWORD, makedreg(0), apr);
            ap1 = copy_addr(apr);
            if (ap1->offset)
                ap1->offset = makenode(en_add, ap1->offset, makeintnode(en_icon,
                    4));
            else
                ap1->offset = makeintnode(en_icon, 4);
            if (ap1->mode == am_ind)
                ap1->mode = am_indx;
            gen_codes(op, BESZ_DWORD, makedreg(1), apr);
        }
        freeop(ap2);
        apr->length = oldsize;
        if (novalue)
            freeop(apr);
        return apr;
    }
    if (bits)
    {
        ap2 = gen_expr(node->v.p[1], FALSE, FALSE, nsize);
        noids(ap2);
        apr = gen_expr(node->v.p[0], FALSE, TRUE, nsize);
        if (ap2->mode == am_immed)
        {
            ENODE *node1 = node->v.p[0];
            int vb;
            ap2->offset->v.i &= mod_mask(node1->bits);
            ap3 = make_immed(ap2->offset->v.i << node1->startbit);
            if (op == op_and)
            {
                ap3->offset->v.i |= ~(mod_mask(node1->bits) << node1->startbit);
                ap3->offset->v.i = ~ap3->offset->v.i;
                vb = single_bit(ap3->offset->v.i);
                if (!prm_coldfire && vb !=  - 1 && apr->mode == am_dreg)
                    gen_codes(op_bclr, BESZ_DWORD, make_immed(vb), apr);
                else
                {
                    ap3->offset->v.i = ~ap3->offset->v.i;
                    if (prm_coldfire && nsize < 4 && nsize !=  - 4)
                    {
                        AMODE *ap4 = copy_addr(apr);
                        do_extend(ap4, BESZ_DWORD, F_DREG | F_VOL);
                        gen_code(op_and, ap3, ap4);
                        ap4->length = nsize;
                        gen_code(op_move, ap4, apr);
                    }
                    else
                        gen_code(op_and, ap3, apr);
                }
            }
            else
            {
                vb = single_bit(ap3->offset->v.i);
                if (!prm_coldfire && vb !=  - 1 && apr->mode == am_dreg)
                    gen_codes(op2, BESZ_DWORD, make_immed(vb), apr);
                else
                if (prm_coldfire && nsize < 4 && nsize !=  - 4)
                {
                    AMODE *ap4 = copy_addr(apr);
                    do_extend(ap4, BESZ_DWORD, F_DREG | F_VOL);
                    gen_codes(op, BESZ_DWORD, ap3, ap4);
                    ap4->length = nsize;
                    gen_code(op_move, ap4, apr);
                }
                else
                    gen_code(op, ap3, apr);
            }
            if (novalue)
                freeop(ap2);
            return ap2;
        }
        else
        {
            do_extend(ap2, prm_coldfire ? 4 : ap2->length, F_DREG | F_VOL);
            gen_code(op_and, make_immed(mod_mask(node->v.p[0]->bits)), ap2);
            if (chksize(ap2->length, apr->length))
                ap2->length = apr->length;
            if (!novalue)
                gen_codes(op_move, BESZ_DWORD, ap2, push);
            if (node->v.p[0]->startbit)
                doshift(op_asl, make_immed(node->v.p[0]->startbit), ap2, FALSE,
                    ap2->length);
            if (op == op_and)
                gen_code(op_or, make_immed(~(mod_mask(node->v.p[0]->bits) <<
                    node->v.p[0]->startbit)), ap2);
            if (prm_coldfire && nsize < 4 && nsize !=  - 4)
            {
                AMODE *ap4 = copy_addr(apr);
                do_extend(ap4, BESZ_DWORD, F_DREG | F_VOL);
                gen_code(op, ap2, ap4);
                ap4->length = nsize;
                gen_code(op_move, ap4, apr);
            }
            else
                gen_code(op, ap2, apr);
            freeop(apr);
            if (!novalue)
                gen_codes(op_move, BESZ_DWORD, pop, ap2);
            else
                freeop(ap2);
            return ap2;
        }
    }
    else
    {
        int vb;
        as_args(node, &apr, &ap1, &ap2, nsize);
        small_size(ap1, ap2, nsize, F_NOREUSE);
        if (ap1->mode != am_dreg && ap2->mode != am_dreg && ap2->mode !=
            am_immed)
            do_extend(ap2, prm_coldfire ? 4 : ap2->length, F_DREG | F_VOL);
        else if (prm_coldfire)
            do_extend(ap2, BESZ_DWORD, F_DREG);
        if (ap2->mode == am_immed)
        {
            if (op == op_and && (vb = single_bit(~ap2->offset->v.i)) !=  - 1 &&
                ap1->mode == am_dreg)
            {
                gen_codes(op2, BESZ_DWORD, make_immed(vb), ap1);
                freeop(ap1);
                return apr;
            }
            else if ((vb = single_bit(ap2->offset->v.i)) !=  - 1 && ap1->mode 
                == am_dreg)
            {
                gen_codes(op2, BESZ_DWORD, make_immed(vb), ap1);
                freeop(ap1);
                return apr;
            }
        }
        if (prm_coldfire && nsize < 4 && nsize !=  - 4)
        {
            AMODE *ap4 = copy_addr(apr);
            do_extend(ap4, BESZ_DWORD, F_DREG | F_VOL);
            gen_code(op, ap2, ap4);
            ap4->length = nsize;
            gen_code(op_move, ap4, apr);
        }
        else
            gen_code(op, ap2, apr);
        freeop(ap1);
        freeop(ap2);
        if (novalue)
            freeop(apr);
        return apr;
    }
}

//-------------------------------------------------------------------------

AMODE *gen_asshift(ENODE *node, int novalue, int op, int size, int div)
/*
 *      generate shift equals operators.
 */
{
    AMODE *ap1,  *ap2,  *apr;
    int nsize = natural_size(node->v.p[0]), bits;

    //            if (op == op_asl || op == op_lsl)
    //               small_size(ap1,ap2,nsize);
    //            else
    //               get_size(ap1,ap2,nsize);
    if (nsize == 6 || nsize ==  - 6)
    {
        ap1 = do6shift(op, node, div, nsize);
        apr = gen_expr(node->v.p[0], FALSE, TRUE, size);
        apr = direct_data(apr);
        gen_codes(op_move, BESZ_DWORD, makedreg(0), apr);
        ap1 = copy_addr(apr);
        if (ap1->offset)
            ap1->offset = makenode(en_add, ap1->offset, makeintnode(en_icon, 4))
                ;
        else
            ap1->offset = makeintnode(en_icon, 4);
        if (ap1->mode == am_ind)
            ap1->mode = am_indx;
        gen_codes(op_move, BESZ_DWORD, makedreg(1), ap1);
        if (novalue)
            freeop(ap1);
        apr->mode = am_doublereg;
        apr->length = 6;
        return apr;
    }
    else
        bits = as_args(node, &apr, &ap1, &ap2, nsize);
    if (!bits)
    {
        if (prm_coldfire && nsize < 4 && nsize >  - 4)
        {
            AMODE *ap4 = copy_addr(apr);
            do_extend(ap4, BESZ_DWORD, F_DREG | F_VOL);
            gen_codes(op, BESZ_DWORD, ap2, ap4);
            ap4->length = nsize;
            gen_code(op_move, ap4, apr);
        }
        else
        {
            do_extend(ap1, BESZ_DWORD, F_DREG | F_VOL);
            doshift(op, ap2, ap1, div, nsize);
            gen_codes(op_move, BESZ_DWORD, ap1, apr);
        }
        freeop(ap2);
        return apr;
    }
    else
    {
        do_extend(ap1, BESZ_DWORD, F_DREG | F_VOL);
        doshift(op, ap2, ap1, div, nsize);
        bit_store(ap1, apr, node->v.p[0], novalue);
        freeop(apr);
        if (novalue)
            freeop(ap1);
        return ap1;
    }
}

//-------------------------------------------------------------------------

AMODE *gen_asmul(ENODE *node, int novalue, int op, int fop, int size)
/*
 *      generate a *= node.
 */
{
    AMODE *ap1,  *ap2,  *ap3,  *ap4,  *apr;
    int bits;
    int nsize = natural_size(node->v.p[0]);
    if (natural_size(node) > 6)
    {
        ap1 = gen_fbinary(node, fop, nsize, &apr);
        return floatstore(node->v.p[0], ap1, apr, novalue, natural_size(node));
    }
    else if (natural_size(node) == 6 || natural_size(node) ==  - 6)
    {
        int oldlen;
        apr = gen_expr(node->v.p[0], FALSE, TRUE, size);
        apr = direct_data(apr);
        ap1 = do6mul(node, 6);
        if (apr->length < 6 && apr->length !=  - 6)
        {
            gen_codes(op_move, apr->length, makedreg(1), apr);
            freeop(ap1);
            if (novalue)
                freeop(apr);
            return apr;
        }
        gen_codes(op_move, BESZ_DWORD, makedreg(0), apr);
        ap1 = copy_addr(apr);
        if (ap1->offset)
            ap1->offset = makenode(en_add, ap1->offset, makeintnode(en_icon, 4))
                ;
        else
            ap1->offset = makeintnode(en_icon, 4);
        if (ap1->mode == am_ind)
            ap1->mode = am_indx;
        gen_codes(op_move, BESZ_DWORD, makedreg(1), ap1);
        apr->length = oldlen;
        freeop(apr);
        if (novalue)
            freeop(ap1);
        return ap1;
    }
    if (prm_coldfire || prm_68020 || nsize == 2 || nsize ==  - 2)
    {
        bits = as_args(node, &apr, &ap1, &ap2, nsize);
        small_size(ap1, ap2, nsize, F_NOREUSE);
        if (!bits)
        {
            if (ap1->mode != am_dreg)
            {
                do_extend(ap2, prm_coldfire ? (ap2->length < 0 ?  - BESZ_DWORD: BESZ_DWORD): ap1
                    ->length, F_DREG);
                do_extend(ap1, prm_coldfire ? (ap1->length < 0 ?  - BESZ_DWORD: BESZ_DWORD): ap1
                    ->length, prm_coldfire ? F_DIVL : F_ALL &~F_AREG);
                gen_codes(op_mulu, ap1->length == 1 || ap1->length == -1 ? ap1->length * 2 : ap1->length, ap1,ap2);
                freeop(ap1);
                gen_code(op_move, ap2, apr);
                if (novalue)
                    freeop(ap2);
                return ap2;
            }
            else
            {
                if (prm_coldfire)
                    do_extend(ap1, ap1->length < 0 ?  - BESZ_DWORD: BESZ_DWORD, F_DREG);
                do_extend(ap2, prm_coldfire ? (ap2->length < 0 ?  - BESZ_DWORD: BESZ_DWORD): ap2
                    ->length, prm_coldfire ? F_DIVL : F_ALL &~F_AREG);
                gen_codes(op_mulu, ap1->length == 1 || ap1->length == -1 ? ap1->length * 2 : ap1->length, ap1, ap2);
                freeop(ap2);
                if (!equal_address(ap1, apr))
                {
                    gen_code(op_move, ap1, apr);
                    if (novalue)
                        freeop(ap1);
                    return ap1;
                }
                freeop(ap1);
            }
            if (novalue)
                freeop(apr);
            return apr;
        }
        else
        {
            if (prm_coldfire)
                do_extend(ap1, ap1->length < 0 ?  - BESZ_DWORD: BESZ_DWORD, F_DREG);
            do_extend(ap2, prm_coldfire ? 4 : ap2->length, prm_coldfire ?
                F_DIVL : F_ALL &~F_AREG);
            gen_code(op_mulu, ap2, ap1);
            bit_store(ap1, apr, node->v.p[0], novalue);
            freeop(ap2);
            freeop(apr);
            if (novalue)
                freeop(ap1);
            return ap1;
        }
    }
    flush_for_libcall();
    bits = as_args(node, &apr, &ap1, &ap2, nsize);
    small_size(ap1, ap2, nsize, F_NOREUSE);

    do_extend(ap2, ap2->length < 0 ?  - BESZ_DWORD: BESZ_DWORD, F_ALL);
    gen_codes(op_move, BESZ_DWORD, ap2, push);
    freeop(ap2);
    do_extend(ap1, ap1->length < 0 ?  - BESZ_DWORD: BESZ_DWORD, F_ALL);
    gen_codes(op_move, BESZ_DWORD, ap1, push);
    freeop(ap1);
    if (op == op_muls)
    {
        ap1 = call_library("__lmuls", 8);
    }
    else
    {
        ap1 = call_library("__lmulu", 8);
    }
    if (node->v.p[0]->nodetype == en_bits)
        bit_store(ap1, apr, node->v.p[0], novalue);
    else
        gen_codes(op_move, nsize, ap1, apr);
    if (novalue)
        freeop(ap1);
    return apr;
}

//-------------------------------------------------------------------------

AMODE *gen_asmodiv(ENODE *node, int novalue, int op, int fop, int size, int
    modflag)
/*
 *      generate /= and %= nodes.
 */
{
    AMODE *ap1,  *ap2,  *ap3 = 0,  *ap4,  *ap5,  *apr;
    int siz1, temp;
    int bits;
    int nsize = natural_size(node->v.p[0]);
    if (natural_size(node) > 6)
    {
        ap1 = gen_fbinary(node, fop, nsize, &apr);
        return floatstore(node->v.p[0], ap1, apr, novalue, natural_size(node));
    }
    else if (natural_size(node) == 6 || natural_size(node) ==  - 6)
    {
        int oldlen;
        apr = gen_expr(node->v.p[0], FALSE, TRUE, size);
        apr = direct_data(apr);
        ap1 = do6div(node, op, modflag, natural_size(node));
        if (apr->length < 6 && apr->length !=  - 6)
        {
            gen_codes(op_move, apr->length, makedreg(1), apr);
            freeop(ap1);
            if (novalue)
                freeop(apr);
            return apr;
        }
        else
        {
            oldlen = apr->length;
            apr->length = 4;
            gen_codes(op_move, BESZ_DWORD, makedreg(0), apr);
            ap1 = copy_addr(apr);
            if (ap1->offset)
                ap1->offset = makenode(en_add, ap1->offset, makeintnode(en_icon,
                    4));
            else
                ap1->offset = makeintnode(en_icon, 4);
            if (ap1->mode == am_ind)
                ap1->mode = am_indx;
            gen_codes(op_move, BESZ_DWORD, makedreg(1), ap1);
            freeop(apr);
            apr->length = oldlen;
            if (novalue)
                freeop(ap1);
            return ap1;
        }
    }
    if (prm_coldfire || prm_68020 || nsize == 2 || nsize ==  - 2)
    {
        bits = as_args(node, &apr, &ap1, &ap2, nsize);
        small_size(ap1, ap2, nsize, F_NOREUSE);
//        if (!prm_coldfire && ap1->length != 2 && ap1->length !=  - 2)
//            if (op == op_divs)
//                op = op_divsl;
//            else
//                op = op_divul;
        if (!bits)
        {
            if (prm_coldfire || prm_68020 || nsize == 4 || nsize ==  - 4)
            {
                do_extend(ap1, BESZ_DWORD, F_DREG);
                do_extend(ap2, BESZ_DWORD, F_ALL &~F_AREG);
                if (prm_coldfire)
                {
                    if (modflag)
                        if (op == op_divs)
                            op = op_rems;
                        else
                            op = op_remu;
                    gen_codes(op, BESZ_DWORD, ap2, ap1);
                }
                else
                {
                    if (modflag)
                    {
                        ap3 = temp_data();
                        ap1->mode = am_divsl;
                        ap1->sreg = ap3->preg;
                        gen_codes(op_divsl, BESZ_DWORD, ap2, ap1);
                        ap1->mode = am_dreg;
                        freeop(ap3);
                    }
                    else
                        gen_codes(op, BESZ_DWORD, ap2, ap1);
                }
            }
            else
            {
                if (nsize == 2 || nsize == -2)
                    do_extend(ap1, ap1->length < 0 ?  - BESZ_DWORD: BESZ_DWORD, F_DREG);
                do_extend(ap2, 2, F_ALL &~F_AREG);
                gen_codes(op, nsize, ap2, ap1);
                if (modflag)
                    gen_codes(op_swap, 0, ap1, 0);
                if (!equal_address(ap1, apr))
                {
                    gen_code(op_move, ap1, apr);
                    if (novalue)
                        freeop(ap1);
                    return ap1;
                }
                if (novalue)
                    freeop(apr);
                return apr;
            }
        }
        if (node->v.p[0]->nodetype == en_bits)
            bit_store(ap1, apr, node->v.p[0], size);
        else if (!equal_address(ap1, apr))
            gen_codes(op_move, nsize, ap1, apr);
        if (novalue)
            freeop(ap1);
        return ap1;
    }
    flush_for_libcall();
    bits = as_args(node, &apr, &ap1, &ap2, nsize);
    // should it bet get_size()?
    small_size(ap1, ap2, nsize, F_NOREUSE);
    do_extend(ap1, ap1->length < 0 ?  - BESZ_DWORD: BESZ_DWORD, F_ALL);
    gen_codes(op_move, BESZ_DWORD, ap1, push);
    freeop(ap1);
    do_extend(ap2, ap2->length < 0 ?  - BESZ_DWORD: BESZ_DWORD, F_ALL);
    gen_codes(op_move, BESZ_DWORD, ap2, push);
    freeop(ap2);
    if (modflag)
        if (op == op_divs)
            ap1 = call_library("__lmods", 8);
        else
            ap1 = call_library("__lmodu", 8);
        else
            if (op == op_divs)
                ap1 = call_library("__ldivs", 8);
            else
                ap1 = call_library("__ldivu", 8);
    if (node->v.p[0]->nodetype == en_bits)
        bit_store(ap1, apr, node->v.p[0], nsize);
    else
        gen_codes(op_move, nsize, ap1, apr);
    if (novalue)
        freeop(ap1);
    return apr;
}

//-------------------------------------------------------------------------

void mov1(AMODE *src, AMODE *dst, int size)
{
    gen_codes(op_move, size, src, dst);
    freeop(src);

}

//-------------------------------------------------------------------------

void mov2(AMODE *src, AMODE *dst, int size1, int size2)
{
    AMODE *dst1,  *src1;
    src1 = copy_addr(src);
    dst1 = copy_addr(dst);
    gen_move(size1, src1, dst1);
    if (src1->offset)
        src1->offset = makenode(en_add, src1->offset, makeintnode(en_icon,
            size1));
    else
        src1->offset = makeintnode(en_icon, size1);
    if (src1->mode == am_ind)
        src1->mode = am_indx;
    if (dst1->offset)
        dst1->offset = makenode(en_add, dst1->offset, makeintnode(en_icon,
            size1));
    else
        dst1->offset = makeintnode(en_icon, size1);
    if (dst1->mode == am_ind)
        dst1->mode = am_indx;
    gen_move(size2, src1, dst1);
    freeop(src1);
}

//-------------------------------------------------------------------------

void mov3(AMODE *src, AMODE *dst, int size1, int size2, int size3)
{
    AMODE *dst1,  *src1;
    src1 = copy_addr(src);
    dst1 = copy_addr(dst);
    gen_move(size1, src1, dst1);
    if (src1->offset)
        src1->offset = makenode(en_add, src1->offset, makeintnode(en_icon,
            size1));
    else
        src1->offset = makeintnode(en_icon, size1);
    if (src1->mode == am_ind)
        src1->mode = am_indx;
    if (dst1->offset)
        dst1->offset = makenode(en_add, dst1->offset, makeintnode(en_icon,
            size1));
    else
        dst1->offset = makeintnode(en_icon, size1);
    if (dst1->mode == am_ind)
        dst1->mode = am_indx;
    gen_move(size2, src1, dst1);
    src1->offset = makenode(en_add, src1->offset, makeintnode(en_icon, size2));
    dst1->offset = makenode(en_add, dst1->offset, makeintnode(en_icon, size2));
    gen_move(size3, src1, dst1);
    freeop(src1);
}

//-------------------------------------------------------------------------

AMODE *amode_moveblock(AMODE *ap1, AMODE *ap2, int size)
{
    int lbl;
    long v, tp, sz;
    int t = size &3, q = size >> 2;
    AMODE *ap3,  *ap4, *ap5, apcount;

    switch (size)
    {
        case 1:
        case 2:
        case 4:
            mov1(ap1, ap2, size);
            break;
        case 3:
            mov2(ap1, ap2, 2, 1);
            break;
        case 5:
            mov2(ap1, ap2, BESZ_DWORD, 1);
            break;
        case 6:
            mov2(ap1, ap2, BESZ_DWORD, 2);
            break;
        case 7:
            mov3(ap1, ap2, BESZ_DWORD, 2, 1);
            break;
        case 8:
            mov2(ap1, ap2, BESZ_DWORD, 4);
            break;
        case 9:
            mov3(ap1, ap2, BESZ_DWORD, BESZ_DWORD, 1);
            break;
        case 10:
            mov3(ap1, ap2, BESZ_DWORD, BESZ_DWORD, 2);
            break;
        case 12:
            mov3(ap1, ap2, BESZ_DWORD, BESZ_DWORD, 4);
            break;
        default:
            if (!(sz = size))
                return (0);
            if (sz &1)
            {
                tp = 1;
            }
            else if (sz &2)
            {
                tp = 2;
            }
            else
            {
                tp = 4;
            }
            v = sz / tp;
            if (!prm_coldfire && v < 65536)
                v--;
            lbl = nextlabel++;
            if (ap1->mode != am_ind && (ap1->mode != am_indx || ap1->offset &&
                ap1->offset->v.i))
            {
                freeop(ap1);
                ap3 = temp_addr();
                gen_code(op_lea, ap1, ap3);
                ap1->preg = ap3->preg;
                ap1->length = 4;
            }
            if (ap2->mode != am_ind && (ap2->mode != am_indx || ap2->offset &&
                ap2->offset->v.i))
            {
                freeop(ap2);
                ap3 = temp_addr();
                gen_code(op_lea, ap2, ap3);
                ap2->preg = ap3->preg;
                ap2->length = 4;
            }
            ap3 = temp_data();
            ap4 = copy_addr(ap2);
            ap4->mode = am_areg;
            ap2->mode = am_ainc;
            ap1->mode = am_ainc;
            gen_codes(op_move, v < 65536 ? 2 : BESZ_DWORD, make_immed(v), ap3);
            gen_codes(op_move, BESZ_DWORD, ap4, push);
			if (ap1->preg >= 2)
			{
				ap5 = makeareg(ap1->preg);
	            gen_codes(op_move, BESZ_DWORD, ap5, push);
			}
            gen_label(lbl);
            gen_codes(op_move, tp, ap1, ap2);
            if (!prm_coldfire && v < 65536)
                gen_codes(op_dbra, 0, ap3, make_label(lbl));
            else
            {
                gen_codes(op_sub, BESZ_DWORD, make_immed(1), ap3);
                gen_codes(op_bne, 0, make_label(lbl), 0);
            }
			if (ap1->preg >= 2)
			{
	            gen_codes(op_move, BESZ_DWORD, pop, ap5);
			}
            gen_codes(op_move, BESZ_DWORD, pop, ap4);
            freeop(ap3);
            break;
    }
    freeop(ap1);
    return ap2;

}

//-------------------------------------------------------------------------

AMODE *gen_moveblock(ENODE *node)
{
    AMODE *ap1,  *ap2;
    ENODE ep1;
    if (!node->size)
        return (0);
    if (node->v.p[1]->nodetype == en_moveblock)
        ep1 =  *node->v.p[1];
    else if (node->v.p[1]->nodetype == en_movebyref)
        ep1 =  *node->v.p[1]->v.p[0];
    else
    {
        ep1.nodetype = en_a_ref;
        ep1.cflags = 0;
        ep1.v.p[0] = node->v.p[1];
    }
    ap2 = gen_expr(&ep1, FALSE, TRUE, 0);
    ap2 = direct_data(ap2);
    if (node->v.p[0]->nodetype == en_moveblock)
        ep1 =  *node->v.p[0];
    else if (node->v.p[0]->nodetype == en_movebyref)
        ep1 =  *node->v.p[0]->v.p[0];
    else
    {
        ep1.nodetype = en_a_ref;
        ep1.cflags = 0;
        ep1.v.p[0] = node->v.p[0];
    }
    ap1 = gen_expr(&ep1, FALSE, TRUE, 0);
    return amode_moveblock(ap2, ap1, node->size);
}

//-------------------------------------------------------------------------

AMODE *amode_clearblock(AMODE *ap1, int size)
{
    int lbl;
    long v, tp, sz;
    int t = size &3, q = size >> 2;
    AMODE *ap3,  *ap4, apcount;

    if (!(sz = size))
        return (0);
    if (sz &1)
    {
        tp = 1;
    }
    else if (sz &2)
    {
        tp = 2;
    }
    else
    {
        tp = 4;
    }
    v = sz / tp;
    if (!prm_coldfire && v < 65536)
        v--;
    lbl = nextlabel++;
	do_extend(ap1, 4, F_AREG | F_VOL);
    ap3 = temp_data();
    ap1->mode = am_ainc;
    gen_codes(op_move, v < 65536 ? 2 : BESZ_DWORD, make_immed(v), ap3);
    gen_label(lbl);
    gen_codes(op_move, tp, make_immed(0),ap1);
    if (!prm_coldfire && v < 65536)
        gen_codes(op_dbra, 0, ap3, make_label(lbl));
    else
    {
        gen_codes(op_sub, BESZ_DWORD, make_immed(1), ap3);
        gen_codes(op_bne, 0, make_label(lbl), 0);
    }
    freeop(ap3);
    return ap1;

}

//-------------------------------------------------------------------------

AMODE *gen_clearblock(ENODE *node)
{
    AMODE *ap1 ;
    ENODE ep1;
    if (!node->size)
        return (0);
    ap1 = gen_expr(node->v.p[0], FALSE, TRUE, 0);
    return amode_clearblock(ap1, node->size);
}

//-------------------------------------------------------------------------

int count_regs(AMODE *ap1, AMODE *ap2)
{
    int r = 0;
    switch (ap1->mode)
    {
        case am_baseindxaddr:
            if (ap1->sreg < cf_freeaddress && ap1->sreg !=  - 1)
                r++;
        case am_baseindxdata:
        case am_ind:
        case am_indx:
        case am_areg:
            if (ap1->preg < cf_freeaddress && ap1->preg !=  - 1)
                r++;
            break;
    }
    switch (ap2->mode)
    {
        case am_baseindxaddr:
            if (ap2->sreg < cf_freeaddress && ap2->sreg !=  - 1)
                r++;
        case am_baseindxdata:
        case am_ind:
        case am_indx:
        case am_areg:
            if (ap2->preg < cf_freeaddress && ap2->preg !=  - 1)
                r++;
            break;
    }
    return r;
}

//-------------------------------------------------------------------------

AMODE *gen_assign(ENODE *node, int novalue, int size, int stdc)
/*
 *      generate code for an assignment node. if the size of the
 *      assignment destination is larger than the size passed then
 *      everything below this node will be evaluated with the
 *      assignment size.
 */
{
    AMODE *ap1,  *ap2,  *apr,  *ap3,  *aps = 0;
    int bits;
    int nsize = natural_size(node->v.p[0]);
    int rsize = assign_size(node->v.p[1], nsize);
    if (nsize == 6 || nsize ==  - 6)
    {
        apr = gen_expr(node->v.p[0], FALSE, TRUE, size);
        apr = direct_data(apr);
        if (!novalue && !stdc)
        {
            aps = copy_addr(apr);
            do_extend(aps, nsize, F_DOUBLEREG);
        }
        ap2 = gen_expr(node->v.p[1], FALSE, FALSE, rsize);
    }
    else
    {
        if (isfloatconst(node->v.p[1]->nodetype))
        {
            ap2 = xalloc(sizeof(AMODE));
            ap2->mode = am_immed;
            ap2->offset = node->v.p[1];
            ap2->length = natural_size(ap2->offset);
        }
        else
            ap2 = gen_expr(node->v.p[1], FALSE, FALSE, rsize);
        noids(ap2);
        if (!novalue && !stdc)
        {
            aps = gen_expr(node->v.p[0], FALSE, TRUE, nsize);
            do_extend(aps, nsize, F_DREG | F_FREG);
        }
        apr = gen_expr(node->v.p[0], FALSE, TRUE, nsize);
    }
    if (natural_size(node->v.p[0]) > 6)
    {
        if (ap2->mode != am_freg && ap2->length == apr->length)
        {
            amode_moveblock(ap2, apr, apr->length == 7 ? 4 : apr->length);
            if (aps)
                return aps;
            return apr;
        }
        else
        {
            do_extend(ap2, 10, F_FREG);
            floatstore(node->v.p[0], ap2, apr, novalue, nsize);
            freeop(apr);
            if (aps)
            {
                freeop(ap2);
                return aps;
            }
            return ap2;
        }
    }
    do_extend(ap2, resultsize(apr->length, natural_size(node->v.p[1])), 0);
    ap2->length = apr->length;
    bits = isbit(node->v.p[0]);
    if (!equal_address(apr, ap2))
    {
        if (!bits)
        {
            if (apr->length > 6)
            {
                do_extend(ap2, apr->length, F_FREG);
                gen_codef(op_fmove, apr->length, ap2, apr);
                freeop(ap2);
                if (aps)
                    return aps;
                return apr;
            }
            if (apr->length == 6 || apr->length ==  - 6)
            {
                int oldlen = apr->length, oldlen2 = ap2->length;
                do_extend(ap2, resultsize(ap2->length, natural_size(node
                    ->v.p[1])), F_MEM | F_DOUBLEREG);
                ap2->length = oldlen2;
                if (ap2->mode == am_doublereg)
                {
                    gen_codes(op_move, BESZ_DWORD, makedreg(0), apr);
                    ap1 = copy_addr(apr);
                    ap1->offset = makenode(en_add, ap1->offset, makeintnode
                        (en_icon, 4));
                    gen_codes(op_move, BESZ_DWORD, makedreg(1), ap1);
                    apr->mode = am_ind;
                }
                else
                {
                    if (ap2->mode == am_immed)
                    {
                        #if sizeof(ULLONG_TYPE) == 4
                            gen_codes(op_move, BESZ_DWORD, make_immed(ap2->offset->v.i <
                                0 ?  - 1: 0), apr);
                        #else 
                            gen_codes(op_move, BESZ_DWORD, make_immed(ap2->offset->v.i
                                >> 32), apr);
                        #endif 
                        ap1 = copy_addr(apr);
                        ap1->offset = makenode(en_add, ap1->offset, makeintnode
                            (en_icon, 4));
                        gen_codes(op_move, BESZ_DWORD, make_immed(ap2->offset->v.i
                            &0xffffffff), ap1);
                        apr->mode = am_ind;
                    }
                    else
                    {
                        int oldlen = apr->length;
                        apr->length = 4;
                        ap2->length = 4;
                        mov2(ap2, apr, BESZ_DWORD, 4);
                        apr->length = oldlen;
                    }
                }
                apr->length = oldlen;
                freeop(ap2);
                if (novalue || aps)
                    freeop(apr);
                if (aps)
                    return aps;
                return (apr);
            }
            else
            {
                do_extend(ap2, resultsize(apr->length, natural_size(node
                    ->v.p[1])), F_ALL);
                ap2->length = apr->length;
                gen_move(apr->length, ap2, apr);
                if (!novalue && (ap2->mode == am_dreg || ap2->mode == am_immed))
                {
                    freeop(apr);
                    if (aps)
                    {
                        freeop(ap2);
                        return aps;
                    }
                    return ap2;
                }
                freeop(ap2);
                if (aps)
                {
                    freeop(apr);
                    return aps;
                }
                return apr;
            }
        }
        else
        {
            bit_store(ap2, apr, node->v.p[0], novalue);
            freeop(apr);
            if (aps)
            {
                freeop(ap2);
                return aps;
            }
            return ap2;
        }
    }
    if (aps)
    {
        freeop(apr);
        return aps;
    }
    return apr;
}

//-------------------------------------------------------------------------

AMODE *gen_refassign(ENODE *node, int novalue, int size)
/*
 *      generate code for an assignment node. if the size of the
 *      assignment destination is larger than the size passed then
 *      everything below this node will be evaluated with the
 *      assignment size.
 */
{
    AMODE *ap1,  *ap2,  *apr,  *ap4,  *ap3,  *ap5;
    int nsize = natural_size(node->v.p[0]);
    int bits = as_args(node, &apr, &ap1, &ap2, nsize);
    if (natural_size(node->v.p[0]) > 6)
    {
        do_extend(ap2, ap1->length, F_FREG);
        if (isbit(node->v.p[0]))
        {
            ap3 = temp_data();
            gen_codef(op_fmove, BESZ_DWORD, ap2, ap3);
            bit_store(ap3, apr, node->v.p[1], natural_size(apr));
            freeop(ap3);

        }
        else
            gen_codef(op_fmove, apr->length, ap2, apr);
        ap4 = temp_addr();
        gen_code(op_lea, apr, ap4);
        freeop(apr);
        return ap4;
    }
    if (!equal_address(apr, ap2))
    {
        if (!bits)
        {
            if (apr->length > 6)
            {
                gen_codef(op_fmove, apr->length, ap2, apr);
                ap4 = temp_data();
                gen_code(op_lea, apr, ap2);
                return ap4;
            }
            if (apr->length == 6 || apr->length ==  - 6)
            {
                apr = direct_data(apr);
                do_extend(ap2, 6, F_DOUBLEREG);
                gen_codes(op_move, BESZ_DWORD, makedreg(0), apr);
                ap5 = copy_addr(apr);
                if (ap5->offset)
                    ap5->offset = makenode(en_add, ap5->offset, makeintnode
                        (en_icon, 4));
                else
                    ap5->offset = makeintnode(en_icon, 4);
                if (ap5->mode == am_ind)
                    ap5->mode = am_indx;
                gen_codes(op_move, BESZ_DWORD, makedreg(1), ap5);
                apr->mode = am_ind;
            }
            else
            {
                do_extend(ap2, resultsize(ap1->length, natural_size(node
                    ->v.p[1])), F_ALL);
                ap2->length = ap1->length;
                gen_code(op_move, ap2, apr);
            }
            ap4 = temp_addr();
            gen_code(op_lea, apr, ap4);
            freeop(ap1);
            freeop(ap2);
            freeop(apr);
            return ap4;
        }
        else
        {
            bit_store(ap2, apr, node->v.p[0], novalue);
            freeop(ap1);
            freeop(apr);
            return ap2;
        }
    }
    freeop(ap1);
    freeop(ap2);
    return apr;
}

//-------------------------------------------------------------------------

AMODE *gen_aincdec(ENODE *node, int novalue, int op, int fop, int size)
/*
 *      generate an auto increment or decrement node. op should be
 *      either op_add (for increment) or op_sub (for decrement).
 */
{
    AMODE *ap1,  *ap2,  *ap3 = 0,  *ap4;
    int pushed = FALSE, sz;
    int nsize = natural_size(node->v.p[0]);
    ap2 = gen_expr(node->v.p[1], FALSE, FALSE, 4);
    ap1 = gen_expr(node->v.p[0], FALSE, TRUE, nsize);
    if (ap1->mode != am_dreg && ap2->mode != am_dreg && ap2->mode != am_immed)
        do_extend(ap2, BESZ_DWORD, F_DREG | F_VOL);
    sz = ap1->length;
    if (sz > 6)
    {
        tofloat(ap1, sz);
        ap3 = temp_float();
        ap3->length = ap1->length;
        if (!novalue)
            gen_codef(op_fmove, 10, ap1, ap3);
        else
            freeop(ap3);
        gen_codef(fop, 10, ap2, ap1);
        floatstore(node->v.p[0], ap3, ap1, TRUE, natural_size(node->v.p[0]));
        return ap3;
    }
    else if (nsize == 6 || nsize ==  - 6)
    {
        int label = nextlabel++;
        sz = ap1->length;
        ap3 = copy_addr(ap1);
        ap3 = direct_data(ap3);
        if (!novalue)
        {
            do_extend(ap1, nsize, F_DOUBLEREG);
        }
        ap4 = copy_addr(ap3);
        if (ap4->offset)
            ap4->offset = makenode(en_add, ap4->offset, makeintnode(en_icon, 4))
                ;
        else
            ap4->offset = makeintnode(en_icon, 4);
        if (ap4->mode == am_ind)
            ap4->mode = am_indx;
        gen_codes(op, BESZ_DWORD, ap2, ap4);
        gen_1branch(op_bcc, make_label(label));
        gen_codes(op, BESZ_DWORD, make_immed(1), ap3);
        gen_label(label);
        freeop(ap3);
        freeop(ap2);
        ap3 = ap1;
    }
    else if (isbit(node->v.p[0]))
    {
        ap3 = gen_expr(node->v.p[0], FALSE, FALSE, sz);
        gen_codes(op, prm_coldfire ? 4 : sz, ap2, ap3);
        bit_store(ap3, ap1, node->v.p[0], novalue);
        freeop(ap1);
        if (novalue)
            freeop(ap3);
        return ap3;
    }
    else
    {
        freeop(ap1);
        if (!novalue)
        {
            int reg = next_dreg();
            /*FIXME*/
            if ((ap1->mode == am_ind || ap1->mode == am_indx || ap1->mode ==
                am_ainc || ap1->mode == am_adec) && ap1->preg == reg || (ap1
                ->mode == am_baseindxdata || ap1->mode == am_baseindxaddr) && 
                (ap1->preg == reg || ap1->sreg == reg) || (ap1->mode ==
                am_pcindxdata || ap1->mode == am_pcindxaddr) && ap1->preg ==
                reg)
            {
                gen_codes(op_move, BESZ_DWORD, ap1, push);
                pushed = TRUE;
            }
            else
            {
                ap3 = temp_data();
                gen_codes(op_move, sz, ap1, ap3);
            }
        }
        gen_codes(op, prm_coldfire ? 4 : sz, ap2, ap1);
        if (pushed)
        {
            ap3 = temp_data();
            gen_codes(op_move, BESZ_DWORD, pop, ap3);
        }
    }
    if (!novalue)
        ap3->length = sz;
    return ap3;
}

//-------------------------------------------------------------------------

int push_param(ENODE *ep, int size)
/*
 *      push the operand expression onto the stack.
 */
{
    AMODE *ap = 0,  *ap1;
    float cc;
    double bb;
    long double aa;
    int rv = size;
    if (rv < 0)
        rv =  - rv;
    switch (ep->nodetype)
    {
        case en_napccon:
            ep->v.p[0] = ep->v.sp;
        case en_labcon:
            ap = xalloc(sizeof(AMODE));
            if (prm_pcrel)
                ap->mode = am_pcindx;
            else
            {
                ap->mode = am_adirect;
                if (prm_smallcode)
                    ap->preg = 2;
                else
                    ap->preg = 4;
            }
            ap->offset = ep; /* use as constant node */
            gen_codes(op_pea, 0, ap, 0);
            gen_code(op_void, 0, 0);
            rv = 4;
            break;
        case en_absacon:
            ap = xalloc(sizeof(AMODE));
            ap->mode = am_adirect;
            ep->v.i = ((SYM*)ep->v.p[0])->value.i;
            ap->preg = isshort(ep) ? 2 : 4;
            ap->offset = ep; /* use as constant node */
            gen_codes(op_pea, 0, ap, 0);
            gen_code(op_void, 0, 0);
            rv = 4;
            break;
        case en_nacon:
            ep->v.p[0] = ep->v.sp;
        case en_nalabcon:
            ap = xalloc(sizeof(AMODE));
            if (prm_datarel)
            {
                ap->preg = basereg; /* frame pointer */
                if (prm_largedata)
                {
                    ap1 = temp_addr();
                    ap->mode = am_areg;
                    gen_codes(op_move, BESZ_DWORD, ap, ap1);
                    ap->mode = am_immed;
                    ap->offset = ep;
                    gen_codes(op_add, BESZ_DWORD, ap, ap1);
                    ap = ap1;
                    ap->mode = am_ind;
                    gen_codes(op_move, BESZ_DWORD, ap, push);
                    ap->mode = am_areg;
                    freeop(ap);
                    gen_code(op_void, 0, 0);
                }
                else
                {
                    ap->mode = am_indx;
                    ap->offset = ep; /* use as constant node */
                    gen_codes(op_pea, BESZ_DWORD, ap, 0);
                    gen_code(op_void, 0, 0);
                }
            }
            else
            {
                ap->mode = am_adirect;
                ap->offset = ep;
                if (prm_smalldata)
                {
                    ap->preg = 2;
                }
                else
                {
                    ap->preg = 4;
                }
                gen_codes(op_pea, BESZ_DWORD, ap, 0);
                gen_code(op_void, 0, 0);
            }
            rv = 4;
            break;
        case en_cfc:
        case en_fcomplexref:
        case en_fcomplexcon:
        case en_crc:
        case en_rcomplexref:
        case en_rcomplexcon:
        case en_clrc:
        case en_lrcomplexref:
        case en_lrcomplexcon:
            rv = 4;
            break;
        case en_fimaginarycon:
        case en_fcon:
        case en_cfi:
        case en_cf:
        case en_floatref:
        case en_fimaginaryref:
            ap = gen_expr(ep, FALSE, TRUE, 7);
            do_extend(ap, 7, F_FREG | F_MEM);
            if (ap->mode == am_freg)
                gen_codef(op_fmove, 7, ap, push);
            else
                gen_codes(op_move, BESZ_DWORD, ap, push);
            gen_code(op_void, 0, 0);
            rv = 4;
            break;
        case en_rimaginarycon:
        case en_rcon:
        case en_cri:
        case en_cd:
        case en_doubleref:
        case en_rimaginaryref:
            ap = gen_expr(ep, FALSE, TRUE, 8);
            do_extend(ap, 8, F_FREG);
            gen_codef(op_fmove, 8, ap, push);
            gen_code(op_void, 0, 0);
            rv = 8;
            break;
        case en_lrcon:
        case en_lrimaginarycon:
        case en_clri:
        case en_cld:
        case en_lrimaginaryref:
        case en_longdoubleref:
            ap = gen_expr(ep, FALSE, TRUE, 10);
            do_extend(ap, 10, F_FREG);
            gen_codef(op_fmove, 10, ap, push);
            gen_code(op_void, 0, 0);
            rv = 12;
            break;
        default:
            ap = gen_expr(ep, FALSE, TRUE, size);
            if (ep->nodetype == en_thiscall && ep->v.p[0]->nodetype ==
                en_substack)
            {
                freeop(ap);
                rv = ep->v.p[0]->v.p[0]->v.p[0]->v.i; // assume icon node
                break;
            }
            do_extend(ap, size, F_ALL | F_DOUBLEREG);
            if (size <= 6)
            {
                while (castvalue(ep))
                    ep = ep->v.p[0];
                if (isbit(ep))
                    ap = bit_load(ap, ep, 4);
            }
            if ((size < 4 || size == 5) && size >  - 4 && ap->mode != am_immed)
            {
                do_extend(ap, BESZ_DWORD, F_DREG);
                rv = 4;
            }
            if (size == 6 || size ==  - 6)
            {
                do_extend(ap, size, F_DOUBLEREG | F_MEM);
                if (ap->mode == am_doublereg)
                {
                    gen_codes(op_move, BESZ_DWORD, makedreg(1), push);
                    gen_codes(op_move, BESZ_DWORD, makedreg(0), push);
                }
                else if (ap->mode == am_immed)
                {
                    gen_codes(op_pea, BESZ_DWORD, make_direct(ap->offset->v.i), 0);
                    #if sizeof(LLONG_TYPE) == 4
                        gen_codes(op_pea, BESZ_DWORD, make_direct(ap->offset->v.i < 0 ? 
                            - 1: 0), 0);
                    #else 
                        gen_codes(op_pea, BESZ_DWORD, make_direct(ap->offset->v.i >> 32),
                            0);
                    #endif 
                }
                else
                {
                    AMODE *ap2 = copy_addr(ap);
                    if (ap2->offset)
                        ap2->offset = makenode(en_add, ap2->offset, makeintnode
                            (en_icon, 4));
                    else
                        ap2->offset = makeintnode(en_icon, 4);
                    if (ap2->mode == am_ind)
                        ap2->mode = am_indx;
                    gen_codes(op_move, BESZ_DWORD, ap2, push);
                    gen_codes(op_move, BESZ_DWORD, ap, push);
                }
                rv = 8;
            }
            else if (size <= 5)
            {
                if (ap->mode == am_doublereg)
                    gen_codes(op_move, rv, makedreg(1), push);
                else
                    gen_codes(op_move, rv, ap, push);
                rv = 4;
            }
            else
            {
                if (ap->mode != am_freg)
                    do_extend(ap, size, F_FREG);
                if (size == 7)
                    rv = 4;
                else if (size == 10)
                    rv = 12;
                else
                    rv = 8;
                gen_codef(op_fmove, size, ap, push);
            }
            gen_code(op_void, 0, 0);
            break;
    }
    if (ap)
        freeop(ap);
    stackdepth += rv;
    return (rv);
}

//-------------------------------------------------------------------------

int push_stackblock(ENODE *ep)
{
    AMODE *ap,  *ap1,  *ap2;
    SYM *sp;
    int x, lbl, v;
    int sz = (ep->size + stackadd) &stackmod;
    if (!sz)
        return (0);
    if (sz > 24)
    {
        switch (ep->nodetype)
        {
            case en_napccon:
                ep->v.p[0] = ep->v.sp;
            case en_labcon:
                ap = xalloc(sizeof(AMODE));
                if (prm_pcrel)
                    ap->mode = am_pcindx;
                else
                {
                    ap->mode = am_adirect;
                    if (prm_smallcode)
                        ap->preg = 2;
                    else
                        ap->preg = 4;
                }
                ap->offset = makenode(en_add, ep, makeintnode(en_icon, sz)); /*
                    use as constant node */
                gen_lea(0, ap, ap2 = temp_addr());
                break;
            case en_absacon:
                ep->v.i = ep->v.sp->value.i;
                ap = xalloc(sizeof(AMODE));
                ap->mode = am_adirect;
                ap->preg = isshort(ep) ? 2 : 4;
                ap->offset = makenode(en_add, ep, makeintnode(en_icon, sz)); /*
                    use as constant node */
                gen_lea(0, ap, ap2 = temp_addr());
                break;
            case en_nacon:
                ep->v.p[0] = ep->v.sp;
            case en_nalabcon:
                ap1 = 0;
                ap = xalloc(sizeof(AMODE));
                if (prm_datarel)
                {
                    ap->preg = basereg; /* frame pointer */

                    if (prm_largedata)
                    {
                        ap1 = temp_addr();
                        ap->mode = am_areg;
                        gen_codes(op_move, BESZ_DWORD, ap, ap1);
                        ap->mode = am_immed;
                        ap->offset = ep;
                        gen_codes(op_add, BESZ_DWORD, ap, ap1);
                        ap = ap1;
                        ap->mode = am_ind;
                    }
                    else
                    {
                        ap->mode = am_indx;
                        ap->offset = ep; /* use as constant node */
                    }
                }
                else
                {
                    ap->mode = am_adirect;
                    ap->offset = ep;
                    if (prm_smalldata)
                    {
                        ap->preg = 2;
                    }
                    else
                    {
                        ap->preg = 4;
                    }
                }
                ap->offset = makenode(en_add, ep, makeintnode(en_icon, sz)); /*
                    use as constant node */
                gen_lea(0, ap, ap2 = temp_addr());
                if (ap1)
                {
                    ap1->mode = am_areg;
                    freeop(ap1);
                }
                break;
            case en_autocon:
            case en_autoreg:
                ep->v.i = ep->v.sp->value.i;
                ap = xalloc(sizeof(AMODE));
                ap->mode = am_indx;
                if (prm_linkreg && !currentfunc->intflag)
                {
                    ap->preg = linkreg;
                    ap->offset = makenode(en_add, ep, makeintnode(en_icon, sz))
                        ; /* use as constant node */
                }
                else if (ep->v.sp->funcparm)
                {
                    if (prm_phiform || currentfunc->intflag)
                    {
                        ap->preg = linkreg; /* frame pointer */
                        ap->offset = makenode(en_add, ep, makeintnode(en_icon,
                            sz)); /* use as constant node */
                    }
                    else
                    {
                        ap->preg = 7;
                        ap->offset = makenode(en_add, ep, makeintnode(en_icon, 
                            (sz + stackdepth + framedepth))); /* use as
                            constant node */
                    }
                }
                else
                {
                    ap->preg = 7;
                    ap->offset = makenode(en_add, ep, makeintnode(en_icon, (sz 
                        + stackdepth + lc_maxauto))); /* use as constant node */
                }
                gen_lea(0, ap, ap2 = temp_addr());
                break;
            default:
                ap = 0;
                ap2 = gen_expr(ep, FALSE, TRUE, 0);
                do_extend(ap2, BESZ_DWORD, F_AREG | F_VOL);
                gen_codes(op_add, BESZ_DWORD, make_immed(sz), ap2);
                break;
        }
        lbl = nextlabel++;
        ap1 = temp_data();
        v = sz / 4;
        if (v < 65536)
            v--;
        gen_codes(op_move, v < 65536 ? 2 : BESZ_DWORD, make_immed(v), ap1);
        gen_label(lbl);
        ap2->mode = am_adec;
        gen_codes(op_move, BESZ_DWORD, ap2, push);
        if (!prm_coldfire && v < 65536)
        {
            gen_codes(op_dbra, 0, ap1, make_label(lbl));
        }
        else
        {
            gen_codes(op_sub, BESZ_DWORD, make_immed(1), ap1);
            gen_codes(op_bne, 0, make_label(lbl), 0);
        }
        freeop(ap1);
        freeop(ap2);
        if (ap)
            freeop(ap);
    }
    else
    {
        int i = ((sz + 3) / 4) *4;
        switch (ep->nodetype)
        {
            case en_napccon:
                ep->v.p[0] = ep->v.sp;
            case en_labcon:
                ap = xalloc(sizeof(AMODE));
                if (prm_pcrel)
                    ap->mode = am_pcindx;
                else
                {
                    ap->mode = am_adirect;
                    if (prm_smallcode)
                        ap->preg = 2;
                    else
                        ap->preg = 4;
                }
                ap->offset = ep;
                break;
            case en_absacon:
                ep->v.i = ep->v.sp->value.i;
                ap = xalloc(sizeof(AMODE));
                ap->mode = am_adirect;
                ap->preg = isshort(ep) ? 2 : 4;
                ap->offset = ep;
                break;
            case en_nacon:
                ep->v.p[0] = ep->v.sp;
            case en_nalabcon:
                ap1 = 0;
                ap = xalloc(sizeof(AMODE));
                if (prm_datarel)
                {
                    ap->preg = basereg; /* frame pointer */

                    if (prm_largedata)
                    {
                        ap1 = temp_addr();
                        ap->mode = am_areg;
                        gen_codes(op_move, BESZ_DWORD, ap, ap1);
                        ap->mode = am_immed;
                        ap->offset = ep;
                        gen_codes(op_add, BESZ_DWORD, ap, ap1);
                        ap = ap1;
                        ap->mode = am_ind;
                    }
                    else
                    {
                        ap->mode = am_indx;
                        ap->offset = ep; /* use as constant node */
                    }
                }
                else
                {
                    ap->mode = am_adirect;
                    ap->offset = ep;
                    if (prm_smalldata)
                    {
                        ap->preg = 2;
                    }
                    else
                    {
                        ap->preg = 4;
                    }
                }
                ap->offset = ep;
                if (ap1)
                {
                    ap1->mode = am_areg;
                    freeop(ap1);
                }
                break;
            case en_autocon:
            case en_autoreg:
                ep->v.i = ep->v.sp->value.i;
                ap = xalloc(sizeof(AMODE));
                ap->mode = am_indx;
                if (prm_linkreg && !currentfunc->intflag)
                {
                    ap->preg = linkreg;
                    ap->offset = ep; /* use as constant node */
                }
                else if (ep->v.sp->funcparm)
                {
                    if (prm_phiform || currentfunc->intflag)
                    {
                        ap->preg = linkreg; /* frame pointer */
                        ap->offset = ep;
                    }
                    else
                    {
                        ap->preg = 7;
                        ap->offset = makenode(en_add, ep, makeintnode(en_icon, 
                            (stackdepth + framedepth))); /* use as constant
                            node */
                    }
                }
                else
                {
                    ap->preg = 7;
                    ap->offset = makenode(en_add, ep, makeintnode(en_icon, 
                        (stackdepth + lc_maxauto))); /* use as constant node */
                }
                break;
            default:
                ap = 0;
                ep = makenode(en_a_ref, ep, 0);
                ap = gen_expr(ep, FALSE, TRUE, 4);
                do_extend(ap, BESZ_DWORD, F_ALL);
                if (ap->mode == am_ind)
                    ap->mode = am_indx;
                break;
        }
        if (!ap->offset)
            ap->offset = makeintnode(en_icon, 0);
        while (i > 0)
        {
            AMODE *ap1 = xalloc(sizeof(AMODE));
            memcpy(ap1, ap, sizeof(AMODE));
            ap1->offset = makenode(en_addstruc, ap->offset, makeintnode(en_icon,
                (i - 4)));
            gen_codes(op_move, BESZ_DWORD, ap1, push);
            i = i - 4;
        }

    }
    return (sz);
}

//-------------------------------------------------------------------------

int gen_parms(ENODE *plist, int size)
/*
 *      push a list of parameters onto the stack and return the
 *      size of parameters pushed.
 */
{
    int i;
    i = 0;
    while (plist != 0)
    {
        if (plist->nodetype == en_stackblock)
            i += push_stackblock(plist->v.p[0]);
        else
            i += push_param(plist->v.p[0], natural_size(plist->v.p[0]));
        plist = plist->v.p[1];
    }
    return i;
}

//-------------------------------------------------------------------------

AMODE *inlinecall(ENODE *node)
{
    ENODE *nameref = node,  *thisn = 0;
    SYM *sp;
    int size;
    int regs[3];
    int selreg;
    int i;
    memcpy(regs, dregs, sizeof(dregs));

    if (nameref->nodetype == en_thiscall)
    {
        thisn = nameref->v.p[0];
        nameref = nameref->v.p[1];
    }

    if (nameref->nodetype == en_callblock || nameref->nodetype == en_scallblock)
    {
        size = 4;
        nameref = nameref->v.p[1];
    }
    else
        size = nameref->v.p[0]->v.i;
    nameref = nameref->v.p[1]->v.p[0]->v.p[0];
    if (nameref->nodetype == en_nacon || nameref->nodetype == en_napccon)
    {
        sp = nameref->v.sp;
        genning_inline++;
        if (size == 6 || size ==  - 6)
            temp_doubledregs();
        if (sp && (sp->value.classdata.cppflags &PF_INLINE))
        {
            int oldretlab = retlab;
            AMODE *ap;
            SYM *oldcurfunc = currentfunc;
            currentfunc = sp;
            retlab = nextlabel++;
            #ifdef XXXXX
                if (sp->value.classdata.cppflags &PF_CONSTRUCTOR)
                {
                    SYM *psp = sp->parentclass;
                    if (psp && psp->value.classdata.baseclass->vtabsp)
                    {
                        ENODE *ts = makenode(en_nacon, psp
                            ->value.classdata.baseclass->vtabsp, 0);
                        thisn = makenode(en_a_ref, thisn, 0);
                        ts = makenode(en_assign, thisn, ts);
                        gen_expr(ts, FALSE, TRUE, 4);
                    }
                }
            #endif 
            genstmt(sp->value.classdata.inlinefunc->stmt);
            genreturn(0, 3);
            genning_inline--;
            selreg = 0;
            for (i = 0; i < 3; i++)
                regs[i] -= dregs[i];
            for (i = 0; i < 3; i++)
                if (regs[i] < regs[selreg])
                    selreg = i;
            if (size > 100)
            {
                ap = makedreg(selreg);
                ap->length = size - 100000;
                ap->mode = am_ind;
            }
            else
            if (size == 6 || size ==  - 6)
            {
                ap = xalloc(sizeof(AMODE));
                ap->length = size < 0 ?  - 6: 6;
                ap->mode = am_doublereg;
                dregs[0]++;
                dregs[1]++;
            }
            else if (size >= 6)
            {
                ap = temp_float();
                if (ap->preg != 0)
                    gen_codef(op_fmove, 8, makefreg(0), ap);
            }
            else
            {
                if (size != 0)
                {
                    ap = makedreg(selreg);
                }
                else
                    ap = makedreg(0);
            }
            currentfunc = oldcurfunc;
            retlab = oldretlab;
            return ap;
        }
    }
    return 0;
}

//-------------------------------------------------------------------------

AMODE *gen_fcall(ENODE *node, int novalue)
/*
 *      generate a function call node and return the address mode
 *      of the result.
 */
{
    AMODE *ap,  *result,  *ap1,  *app = 0;
    ENODE *pushthis = 0;
    int refret = FALSE;
    int i, siz1;
    char xdregs[3], xaregs[3], xfregs[3];
    if (node->nodetype != en_trapcall)
	{
		if (ap = inlinecall(node))
	        return ap;
	    if (node->nodetype != en_callblock && node->nodetype != en_scallblock)
    	    if (ap = HandleIntrins(node, novalue))
        	    return ap;
	}

        if (node->nodetype == en_thiscall)
        {
            pushthis = node->v.p[0];
            node = node->v.p[1];
            i = 4;
        }
        else
            i = 0;
    if (node->nodetype == en_callblock || node->nodetype == en_scallblock)
    {
        siz1 = 4;
    }
    else
    {
        siz1 = node->v.p[0]->v.i;
        if (siz1 > 100)
        {
            siz1 -= 100000;
            refret = TRUE;
        }
    }
    xdregs[0] = dregs[0];
    xdregs[1] = dregs[1];
    if ((siz1 == 6 || siz1 ==  - 6) && (xdregs[0] && xdregs[1]))
        temp_doubledregs();
    else
    {
        if (xdregs[1])
            gen_push(1, am_dreg, 0);
        if (xdregs[0])
            gen_push(0, am_dreg, 0);
    }
    if (xdregs[2] = dregs[2])
        gen_push(2, am_dreg, 0);
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
    dregs[0] = dregs[1] = dregs[2] = 0;
    aregs[0] = aregs[1] = 0;
    set_func_mode(1);
    if (pushthis && pushthis->nodetype == en_substack)
        app = gen_expr(pushthis, FALSE, TRUE, 4);
    if (node->nodetype == en_callblock || node->nodetype == en_scallblock)
    {
        i = gen_parms(node->v.p[1]->v.p[1]->v.p[1]->v.p[0], 0);
        ap = gen_expr(node->v.p[0], FALSE, TRUE, 4);
        gen_codes(op_move, BESZ_DWORD, ap, push);
        i += 4;
        stackdepth += 4;
        node = node->v.p[1];
        freeop(ap);
    }
    else
    {
		if (node->nodetype != en_trapcall)
		        i = gen_parms(node->v.p[1]->v.p[1]->v.p[0], 0); /* generate parameters */
		else
			i = 0;
    }
    if ((prm_phiform || node->nodetype == en_trapcall || node->nodetype ==
        en_intcall) && i)
        gen_codes(op_move, BESZ_DWORD, makeareg(7), makeareg(0));
    if (node->nodetype == en_trapcall)
    {
        gen_codes(op_trap, 0, make_immed(node->v.p[0]->v.i), 0);
    }
    else if (node->nodetype == en_intcall)
    {
        /* This is one case where we can generate code that will run
         * on a 68000 but not on a 68020 or vice versa*/
        int curlabel = nextlabel++;
        DIAG("Direct calls to interrups not portable across processors");
        if (prm_68020)
        {
            gen_codes(op_clr, 2, push, 0);
        }
        ap1 = make_label(curlabel);
        if (prm_pcrel)
            ap1->mode = am_pcindx;
        else
        {
            ap1->mode = am_adirect;
            if (prm_smallcode)
                ap1->preg = 2;
            else
                ap1->preg = 4;
        }
        gen_codes(op_pea, BESZ_DWORD, ap1, 0);
        ap1 = xalloc(sizeof(AMODE));
        ap1->mode = am_sr;
        gen_codes(op_move, 2, ap1, push); /* May cause an exception on an 020 */
        gen_codes(op_bra, 0, make_offset(node->v.p[1]->v.p[0]->v.p[0]), 0);
        gen_label(curlabel);
        freeop(ap1);
    }
    else
    {
        if (pushthis)
        {
            if (!app)
                app = gen_expr(pushthis, FALSE, TRUE, 4);
            gen_codes(op_move, BESZ_DWORD, app, push);
            freeop(app);
        }
        if (node->v.p[1]->v.p[0]->v.p[0]->nodetype == en_nacon || node->v.p[1]
            ->v.p[0]->v.p[0]->nodetype == en_napccon)
        {
            SYM *sp = node->v.p[1]->v.p[0]->v.p[0]->v.sp;
            if (sp->inreg)
            {
                if (sp->value.i < 8)
                    ap1 = makedreg(sp->value.i);
                else
                    ap1 = makeareg(sp->value.i &7);
                do_extend(ap1, BESZ_DWORD, F_AREG);
                ap1->mode = am_ind;
                gen_codes(op_jsr, 0, ap1, 0);
            }
            else
            {
                node->v.p[1]->v.p[0]->v.p[0]->v.sp = sp;
                ap1 = xalloc(sizeof(AMODE));
                ap1->offset = node->v.p[1]->v.p[0]->v.p[0];
                if (prm_68020)
                {
                    ap1->preg = 4;
                }
                else
                {
                    ap1->preg = 2;
                }
                if (prm_pcrel)
                    ap1->mode = am_pcindx;
                else
                    ap1->mode = am_adirect;
                gen_codes(op_jsr, 0, ap1, 0);
            }
        }
        else
        {
            ap = gen_expr(node->v.p[1]->v.p[0]->v.p[0], FALSE, TRUE, 4);
            do_extend(ap, BESZ_DWORD, F_AREG);
            ap->mode = am_ind;
            freeop(ap);
            gen_codes(op_jsr, 0, ap, 0);
        }
    }
    dregs[0] = xdregs[0];
    dregs[1] = xdregs[1];
    dregs[2] = xdregs[2];
    aregs[0] = xaregs[0];
    aregs[1] = xaregs[1];
    set_func_mode(0);
    if (node->nodetype == en_fcall || node->nodetype == en_fcallb || node
        ->nodetype == en_callblock)
    if (i != 0)
    {
        if (i > 8)
        {
            AMODE *ap = xalloc(sizeof(AMODE));
            ap->mode = am_indx;
            ap->offset = makeintnode(en_icon, i);
            ap->preg = 7;
            gen_lea(0, ap, makeareg(7));
        }
        else
            gen_codes(op_add, BESZ_DWORD, make_immed(i), makeareg(7));
        stackdepth -= i;
    }
    if (refret)
    {
        result = temp_data();
        result->length = siz1;
        if (result->preg != 0)
            gen_codes(op_move, siz1, result, makedreg(0));
        result->mode = am_ind;
    }
    else
    if (siz1 == 6 || siz1 ==  - 6)
    {
        result = xalloc(sizeof(AMODE));
        result->length = siz1 < 0 ?  - 6: 6;
        result->mode = am_doublereg;
        dregs[0]++;
        dregs[1]++;
    }
    else if (siz1 >= 6)
    {
        result = temp_float();
        if (result->preg != 0)
            gen_codef(op_fmove, 8, makefreg(0), result);
    }
    else
    {
        if (siz1 != 0)
        {
            result = temp_data();
            if (result->preg != 0)
                gen_codes(op_move, BESZ_DWORD, makedreg(0), result);
        }
        else
            result = makedreg(0);
    }
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
    if (xdregs[2])
        gen_pop(2, am_dreg, 0);
    if (siz1 != 6 && siz1 !=  - 6)
    {
        if (xdregs[1])
            gen_pop(1, am_dreg, 0);
        if (xdregs[0])
            gen_pop(0, am_dreg, 0);
    }
    result->tempflag = 1;
    return result;
}

//-------------------------------------------------------------------------

AMODE *gen_repcons(ENODE *node)
/*
 *      generate a function call node and return the address mode
 *      of the result.
 */
{
    AMODE *ax,  *cx,  *ap,  *ap1,  *ap2;
    ENODE *pushthis = 0,  *onode = node;
    int i = 0, siz1;
    int lab;
    int regax, regdx, regcx;
    node = node->v.p[1];
        if (node->nodetype == en_thiscall)
        {
            pushthis = node->v.p[0];
            node = node->v.p[1];
            i = 4;
        }
    ap2 = gen_expr(onode->v.p[0]->v.p[0], FALSE, FALSE, 4);
    gen_codes(op_move, BESZ_DWORD, ap2, cx = makedreg(0));
    if (pushthis)
    {
        AMODE *ap2 = gen_expr(pushthis, FALSE, TRUE, 4);
        gen_codes(op_move, BESZ_DWORD, ap2, ax = makedreg(0));
        freeop(ap2);
    }
    gen_label(lab = nextlabel++);

    gen_codes(op_move, BESZ_DWORD, cx, push);
    gen_codes(op_move, BESZ_DWORD, ax, push);

    if (node->v.p[1]->v.p[0]->v.p[0]->nodetype == en_nacon || node->v.p[1]
        ->v.p[0]->v.p[0]->nodetype == en_napccon)
    {
        SYM *sp = node->v.p[1]->v.p[0]->v.p[0]->v.sp;
        if (sp->inreg)
        {
            if (sp->value.i < 8)
                ap1 = makedreg(sp->value.i);
            else
                ap1 = makeareg(sp->value.i &7);
            do_extend(ap1, BESZ_DWORD, F_AREG);
            ap1->mode = am_ind;
            gen_codes(op_jsr, 0, ap1, 0);
        }
        else
        {
            node->v.p[1]->v.p[0]->v.p[0]->v.sp = sp;
            ap1 = xalloc(sizeof(AMODE));
            ap1->offset = node->v.p[1]->v.p[0];
            if (prm_68020)
            {
                ap1->preg = 4;
            }
            else
            {
                ap1->preg = 2;
            }
            if (prm_pcrel)
                ap1->mode = am_pcindx;
            else
                ap1->mode = am_adirect;
            gen_codes(op_jsr, 0, ap1, 0);
        }
    }
    else
    {
        ap = gen_expr(node->v.p[1]->v.p[0]->v.p[0], FALSE, TRUE, 4);
        do_extend(ap, BESZ_DWORD, F_AREG);
        ap->mode = am_ind;
        freeop(ap);
        gen_codes(op_jsr, 0, ap, 0);
    }
    gen_codes(op_move, BESZ_DWORD, pop, ax);
    gen_codes(op_move, BESZ_DWORD, pop, cx);
    gen_codes(op_add, BESZ_DWORD, make_immed((int)onode->v.p[0]->v.p[1]->v.p[0]), ax);
    gen_codes(op_sub, BESZ_DWORD, make_immed(1), cx);
    gen_code(op_bne, make_label(lab), 0);
}

//-------------------------------------------------------------------------

AMODE *gen_pfcall(ENODE *node, int size)
/*
 *      generate a function call node for pascal function calls
 *			and return the address mode of the result.
 */
{
    AMODE *ap,  *result,  *ap1;
    int i, siz1;
    int refret = FALSE;
    ENODE *invnode = 0,  *anode,  *pushthis = 0;

    char xdregs[3], xaregs[3], xfregs[3];
    if (ap = inlinecall(node))
        return ap;
        if (node->nodetype == en_thiscall)
        {
            pushthis = node->v.p[0];
            node = node->v.p[1];
            i = 4;
        }
    if (node->nodetype == en_pcallblock || node->nodetype == en_scallblock)
    {
        siz1 = 4;
    }
    else
    {
        siz1 = node->v.p[0]->v.i;
        if (siz1 > 100)
        {
            siz1 -= 100000;
            refret = TRUE;
        }
    }
    xdregs[0] = dregs[0];
    xdregs[1] = dregs[1];
    if ((siz1 == 6 || siz1 ==  - 6) && (xdregs[0] && xdregs[1]))
        temp_doubledregs();
    else
    {
        if (xdregs[0])
            gen_push(0, am_dreg, 0);
        if (xdregs[1])
            gen_push(1, am_dreg, 0);
    }
    if (xdregs[2] = dregs[2])
        gen_push(2, am_dreg, 0);
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
    /* invert the parameter list */
    if (node->nodetype == en_pcallblock || node->nodetype == en_scallblock)
        anode = node->v.p[1]->v.p[1]->v.p[1]->v.p[0];
    else
        anode = node->v.p[1]->v.p[1]->v.p[0];
    while (anode)
    {
        invnode = makenode(anode->nodetype, anode->v.p[0], invnode);
        anode = anode->v.p[1];
    }
    if (pushthis)
    {
        AMODE *app = gen_expr(pushthis, FALSE, TRUE, 4);
        gen_codes(op_move, BESZ_DWORD, app, push);
        freeop(app);
    }
    if (node->nodetype == en_pcallblock || node->nodetype == en_scallblock)
    {
        ap = gen_expr(node->v.p[0], FALSE, TRUE, 4);
        gen_codes(op_move, BESZ_DWORD, ap, push);
        freeop(ap);
        i = 4;
        stackdepth += 4;
        i += gen_parms(invnode, size);
        node = node->v.p[1];
    }
    else
    {
        i = gen_parms(invnode, size); /* generate parameters */
    }
    if ((prm_phiform || node->nodetype == en_trapcall || node->nodetype ==
        en_intcall) && i)
        gen_codes(op_move, BESZ_DWORD, makeareg(7), makeareg(0));
    if (node->v.p[1]->v.p[0]->v.p[0]->nodetype == en_nacon || node->v.p[1]
        ->v.p[0]->v.p[0]->nodetype == en_napccon)
    {
        SYM *sp = node->v.p[1]->v.p[0]->v.p[0]->v.sp;
        if (sp->inreg)
        {
            if (sp->value.i < 8)
                ap1 = makedreg(sp->value.i);
            else
                ap1 = makeareg(sp->value.i &7);
            do_extend(ap1, size, F_AREG);
            ap1->mode = am_ind;
            gen_codes(op_jsr, 0, ap1, 0);
        }
        else
        {
            node->v.p[1]->v.p[0]->v.p[0]->v.sp = sp;
            ap1 = xalloc(sizeof(AMODE));
            ap1->offset = node->v.p[1]->v.p[0]->v.p[0];
            if (prm_68020)
            {
                ap1->preg = 4;
            }
            else
            {
                ap1->preg = 2;
            }
            if (prm_pcrel)
                ap1->mode = am_pcindx;
            else
                ap1->mode = am_adirect;
            gen_codes(op_jsr, 0, ap1, 0);
        }
    }
    else
    {
        ap = gen_expr(node->v.p[1]->v.p[0]->v.p[0], FALSE, TRUE, 4);
        do_extend(ap, BESZ_DWORD, F_AREG);
        ap->mode = am_ind;
        freeop(ap);
        gen_codes(op_jsr, 0, ap, 0);
    }
    stackdepth -= i;
    if (refret)
    {
        result = temp_data();
        result->length = siz1;
        if (result->preg != 0)
            gen_codes(op_move, siz1, result, makedreg(0));
        //               result->mode = am_ind ;
    }
    else
    if (siz1 ==  - 6 || siz1 == 6)
    {
        result = xalloc(sizeof(AMODE));
        result->length = siz1 < 0 ?  - 6: 6;
        ;
        result->mode = am_doublereg;
        dregs[0]++;
        dregs[1]++;
    }
    else if (siz1 >= 6)
    {
        result = temp_float();
        if (result->preg != 0)
            gen_codef(op_fmove, 8, makefreg(0), result);
    }
    else
    {
        if (siz1 != 0)
        {
            result = temp_data();
            if (result->preg != 0)
                gen_codes(op_move, BESZ_DWORD, makedreg(0), result);
        }
        else
            result = makedreg(0);
    }
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
    if (xdregs[2])
        gen_pop(2, am_dreg, 0);
    if (siz1 != 6 && siz1 !=  - 6)
    {
        if (xdregs[1])
            gen_pop(1, am_dreg, 0);
        if (xdregs[0])
            gen_pop(0, am_dreg, 0);
    }
    result->tempflag = 1;
    return result;
}

//-------------------------------------------------------------------------

AMODE *gen_expr(ENODE *node, int novalue, int adronly, int size)
/*
 *      general expression evaluation. returns the addressing mode
 *      of the result.
 */
{
    AMODE *ap1,  *ap2;
    int lab0, lab1;
    int natsize, fconst = 0;
    SYM *sp;
    if (node == 0)
    {
        // CPP generates null nodes for cons & des sometimes
        //                DIAG("null node in gen_expr.");
        return 0;
    }
    switch (node->nodetype)
    {
        case en_bits:
            ap1 = gen_expr(node->v.p[0], novalue, adronly, size);
            if (!adronly)
                ap1 = bit_load(ap1, node, natural_size(node));
            return ap1;
        case en_cl_reg:
            return gen_expr(node->v.p[0], novalue, adronly, natural_size(node
                ->v.p[0]));
        case en_cbool:
        case en_cb:
        case en_cub:
        case en_cw:
        case en_cuw:
        case en_cl:
        case en_cul:
		case en_ci:
		case en_cui:
        case en_cf:
        case en_cd:
        case en_cld:
        case en_cp:
        case en_cll:
        case en_cull:
        case en_cfc:
        case en_cfi:
        case en_crc:
        case en_cri:
        case en_clrc:
        case en_clri:
            ap1 = gen_expr(node->v.p[0], novalue, adronly, natural_size(node
                ->v.p[0]));
            if (!adronly)
                do_extend(ap1, natural_size(node), F_VOL);
            return ap1;
        case en_napccon:
            node->v.p[0] = node->v.sp;
        case en_labcon:
            ap1 = temp_addr();
            ap1->tempflag = TRUE;
            ap2 = xalloc(sizeof(AMODE));
            if (prm_pcrel)
                ap2->mode = am_pcindx;
            else
            {
                ap2->mode = am_adirect;
                if (prm_smallcode)
                    ap2->preg = 2;
                else
                    ap2->preg = 4;
            }
            ap2->offset = node; /* use as constant node */
            gen_lea(0, ap2, ap1);
            ap1->length = 4;
            return ap1; /* return reg */
        case en_absacon:
            node->v.i = node->v.sp->value.i;
            ap1 = temp_addr();
            ap1->tempflag = TRUE;
            ap2 = xalloc(sizeof(AMODE));
            ap2->mode = am_adirect;
            ap2->preg = isshort(node) ? 2 : 4;
            ap2->offset = node; /* use as constant node */
            gen_lea(0, ap2, ap1);
            ap1->length = 4;
            return ap1; /* return reg */
        case en_nacon:
            node->v.p[0] = node->v.sp;
        case en_nalabcon:
            ap1 = temp_addr();
            ap1->tempflag = TRUE;
            ap2 = xalloc(sizeof(AMODE));
            if (prm_datarel)
            {
                ap2->preg = basereg; /* frame pointer */
                if (prm_largedata)
                {
                    ap2->mode = am_areg;
                    gen_codes(op_move, 0, ap2, ap1);
                    ap2->mode = am_immed;
                    ap2->offset = node;
                    gen_codes(op_add, 0, ap2, ap1);
                }
                else
                {
                    ap2->mode = am_indx;
                    ap2->offset = node; /* use as constant node */
                    gen_lea(0, ap2, ap1);
                }
            }
            else
            {
                ap2->mode = am_adirect;
                ap2->offset = node;
                if (prm_smalldata)
                {
                    ap2->preg = 2;
                }
                else
                {
                    ap2->preg = 4;
                }
                gen_lea(0, ap2, ap1);
            }
            ap1->length = 4;
            return ap1; /* return reg */
        case en_rcon:
        case en_lrcon:
        case en_fcon:
        case en_fimaginarycon:
        case en_rimaginarycon:
        case en_lrimaginarycon:
        case en_fcomplexcon:
        case en_rcomplexcon:
        case en_lrcomplexcon:
            ap1 = xalloc(sizeof(AMODE));
            ap1->mode = am_immed;
            ap1->offset = node;
            ap1->length = natural_size(node);
            return ap1;
        case en_llcon:
        case en_llucon:
        case en_icon:
        case en_lcon:
        case en_lucon:
        case en_iucon:
        case en_boolcon:
        case en_ccon:
        case en_cucon:
            ap1 = xalloc(sizeof(AMODE));
            ap1->mode = am_immed;
            ap1->offset = node;
            ap1->length = natural_size(node);
            return ap1;
        case en_substack:
            ap1 = gen_expr(node->v.p[0], FALSE, FALSE, size);
            gen_codes(op_sub, BESZ_DWORD, ap1, makeareg(7));
            ap1 = temp_data();
            gen_codes(op_move, BESZ_DWORD, makeareg(7), ap1);
            return ap1;
        case en_structret:
            ap1 = xalloc(sizeof(AMODE));
            if (prm_linkreg && !currentfunc->intflag)
            {
                if ((currentfunc->pascaldefn || currentfunc->isstdcall) &&
                    currentfunc->tp->lst.head && currentfunc->tp->lst.head != 
                    (SYM*) - 1)
                {
                    ap1->preg = linkreg;
                    ap1->mode = am_indx;
                    ap1->offset = makeintnode(en_icon, (currentfunc->tp
                        ->lst.head->value.i + ((currentfunc->tp->lst.head->tp
                        ->size + 3) &0xFFFFFFFCL)));
                }
                else
                {
                    ap1->preg = linkreg;
                    ap1->mode = am_indx;
                    ap1->offset = makeintnode(en_icon, 8);
                }
            }
            else if (prm_phiform || currentfunc->intflag)
            {
                if ((currentfunc->pascaldefn || currentfunc->isstdcall) &&
                    currentfunc->tp->lst.head && currentfunc->tp->lst.head != 
                    (SYM*) - 1)
                {
                    ap1->preg = linkreg;
                    ap1->mode = am_indx;
                    ap1->offset = makeintnode(en_icon, (currentfunc->tp
                        ->lst.head->value.i + ((currentfunc->tp->lst.head->tp
                        ->size + 3) &0xFFFFFFFCL)));
                }
                else
                {
                    ap1->preg = linkreg;
                    ap1->mode = am_ind;
                }
            }
            else
            {
                if (currentfunc->pascaldefn && currentfunc->tp->lst.head &&
                    currentfunc->tp->lst.head != (SYM*) - 1)
                {
                    ap1->preg = 7;
                    ap1->mode = am_indx;
                    ap1->offset = makeintnode(en_icon, (framedepth + stackdepth
                        + currentfunc->tp->lst.head->value.i + ((currentfunc
                        ->tp->lst.head->tp->size + 3) &0xfffffffcL)));
                }
                else
                {
                    ap1->preg = 7;
                    ap1->mode = am_indx;
                    ap1->offset = makeintnode(en_icon, (framedepth + stackdepth)
                        );
                }
            }
            ap2 = temp_data();
            gen_codes(op_move, BESZ_DWORD, ap1, ap2);
            return ap2;
        case en_autocon:
        case en_autoreg:
            ap1 = temp_addr();
            ap1->tempflag = TRUE;
            ap2 = xalloc(sizeof(AMODE));
            ap2->mode = am_indx;
            if (prm_linkreg && !currentfunc->intflag)
            {
                ap2->preg = linkreg;
                ap2->offset = makeintnode(en_icon, ((SYM*)node->v.p[0])
                    ->value.i);
            }
            else if (((SYM*)node->v.p[0])->funcparm)
            {
                int i = 0;
                    //													if ((currentfunc->value.classdata.cppflags & PF_MEMBER) &&
                    //															!(currentfunc->value.classdata.cppflags & PF_STATIC))
                    //														i += 4;
                if (prm_phiform || currentfunc->intflag)
                {
                    ap2->preg = linkreg; /* frame pointer */
                    ap2->offset = makeintnode(en_icon, (((SYM*)node->v.p[0])
                        ->value.i + 4));
                }
                else
                {
                    ap2->preg = 7;
                    ap2->offset = makeintnode(en_icon, (((SYM*)node->v.p[0])
                        ->value.i + 4+stackdepth + framedepth));
                }
            }
            else
            {
                ap2->preg = 7;
                ap2->offset = makeintnode(en_icon, (((SYM*)node->v.p[0])
                    ->value.i + stackdepth + lc_maxauto));
            }
            gen_lea(0, ap2, ap1);
            ap1->length = 4;
            return ap1; /* return reg */
        case en_bool_ref:
        case en_b_ref:
        case en_w_ref:
        case en_ub_ref:
        case en_uw_ref:
        case en_l_ref:
        case en_ul_ref:
		case en_a_ref:
		case en_ua_ref:
		case en_i_ref:
		case en_ui_ref:
        case en_fimaginaryref:
        case en_rimaginaryref:
        case en_lrimaginaryref:
        case en_fcomplexref:
        case en_rcomplexref:
        case en_lrcomplexref:
        case en_floatref:
        case en_doubleref:
        case en_longdoubleref:
        case en_ll_ref:
        case en_ull_ref:
            ap1 = gen_deref(node, 4);
            return ap1;
        case en_tempref:
        case en_regref:
            ap1 = xalloc(sizeof(AMODE));
            if ((node->v.i &0xff) < 16)
            {
                ap1->mode = am_dreg;
                ap1->preg = node->v.i &0xff;
            }
            else
            if ((node->v.i &0xff) < 32)
            {
                ap1->mode = am_areg;
                ap1->preg = (node->v.i &0xff) - 16;
            }
            else
            {
                ap1->mode = am_freg;
                ap1->preg = (node->v.i &0xff) - 32;
            }
            ap1->tempflag = 0; /* not a temporary */
            ap1->length = node->v.i >> 16;
            return ap1;
        case en_asuminus:
            return gen_asunary(node, novalue, op_neg, op_fneg, size);
        case en_ascompl:
            return gen_asunary(node, novalue, op_not, op_not, size);
        case en_uminus:
            return gen_unary(node, op_neg, op_fneg, size);
        case en_compl:
            return gen_unary(node, op_not, op_not, size);
        case en_add:
        case en_addstruc:
		case en_array:
            return gen_binary(node, op_add, op_fadd, size);
        case en_addcast:
            ap1 = gen_expr(node->v.p[0], FALSE, FALSE, size);
            gen_codes(op_tst, BESZ_DWORD, ap1, 0);
            lab0 = nextlabel++;
            gen_branch(op_beq, make_label(lab0));
            ap2 = gen_expr(node->v.p[1], FALSE, FALSE, size); 
                // will always be constant
            gen_codes(op_add, BESZ_DWORD, ap1, ap2);
            gen_label(lab0);
            return ap1;
        case en_sub:
            return gen_binary(node, op_sub, op_fsub, size);
        case en_and:
            return gen_xbin(node, op_and, op_and, size);
        case en_or:
            return gen_xbin(node, op_or, op_or, size);
        case en_xor:
            return gen_xbin(node, op_eor, op_eor, size);
        case en_pmul:
		case en_arrayindex:
            return gen_pmul(node, size);
        case en_pdiv:
            return gen_pdiv(node, size);
        case en_mul:
            return gen_mul(node, op_muls, op_fmul, size);
        case en_umul:
            return gen_mul(node, op_mulu, op_fmul, size);
        case en_div:
            return gen_modiv(node, op_divs, op_fdiv, size, 0);
        case en_udiv:
            return gen_modiv(node, op_divu, op_fdiv, size, 0);
        case en_mod:
            return gen_modiv(node, op_divs, op_fdiv, size, 1);
        case en_umod:
            return gen_modiv(node, op_divu, op_fdiv, size, 1);
        case en_alsh:
            return gen_shift(node, op_asl, size, FALSE);
        case en_arsh:
            return gen_shift(node, op_asr, size, FALSE);
        case en_arshd:
            return gen_shift(node, op_asr, size, TRUE);
        case en_lsh:
            return gen_shift(node, op_lsl, size, FALSE);
        case en_rsh:
            return gen_shift(node, op_lsr, size, FALSE);
        case en_asadd:
            return gen_asadd(node, novalue, op_add, op_fadd, size);
        case en_assub:
            return gen_asadd(node, novalue, op_sub, op_fsub, size);
        case en_asand:
            return gen_aslogic(node, novalue, op_and, op_bclr, size);
        case en_asor:
            return gen_aslogic(node, novalue, op_or, op_bset, size);
        case en_asxor:
            return gen_aslogic(node, novalue, op_eor, op_bchg, size);
        case en_aslsh:
            return gen_asshift(node, novalue, op_lsl, size, FALSE);
        case en_asrsh:
            return gen_asshift(node, novalue, op_lsr, size, FALSE);
        case en_asalsh:
            return gen_asshift(node, novalue, op_asl, size, FALSE);
        case en_asarsh:
            return gen_asshift(node, novalue, op_asr, size, FALSE);
        case en_asarshd:
            return gen_asshift(node, novalue, op_asr, size, TRUE);
        case en_asmul:
            return gen_asmul(node, novalue, op_muls, op_fmul, size);
        case en_asumul:
            return gen_asmul(node, novalue, op_mulu, op_fmul, size);
        case en_asdiv:
            return gen_asmodiv(node, novalue, op_divs, op_fdiv, size, FALSE);
        case en_asudiv:
            return gen_asmodiv(node, novalue, op_divu, op_fdiv, size, FALSE);
        case en_asmod:
            return gen_asmodiv(node, novalue, op_divs, op_fdiv, size, TRUE);
        case en_asumod:
            return gen_asmodiv(node, novalue, op_divu, op_fdiv, size, TRUE);
        case en_assign:
            return gen_assign(node, novalue, size, TRUE);
        case en_lassign:
            return gen_assign(node, novalue, size, FALSE);
        case en_refassign:
            return gen_refassign(node, novalue, size);
        case en_moveblock:
            return gen_moveblock(node);
        case en_clearblock:
            return gen_clearblock(node);
        case en_ainc:
            return gen_aincdec(node, novalue, op_add, op_fadd, size);
        case en_adec:
            return gen_aincdec(node, novalue, op_sub, op_fsub, size);
        case en_land:
        case en_lor:
        case en_eq:
        case en_ne:
        case en_lt:
        case en_le:
        case en_gt:
        case en_ge:
        case en_ult:
        case en_ule:
        case en_ugt:
        case en_uge:
        case en_not:
            return gen_relat(node);
        case en_cond:
            return gen_hook(node, size);
        case en_voidnz:
            lab0 = nextlabel++;
            falsejp(node->v.p[0]->v.p[0], lab0);
            initstack();
            gen_void(node->v.p[1]);
            gen_label(lab0);
            return gen_expr(node->v.p[0]->v.p[1], FALSE, FALSE, 4); /* will
                typically be part of a void tree, or top of tree */
        case en_void:
            natsize = natural_size(node->v.p[0]);
            gen_void(node->v.p[0]);
            return gen_expr(node->v.p[1], TRUE, FALSE, natural_size(node));		
        case en_dvoid:
            ap1 = gen_expr(node->v.p[0], TRUE, FALSE, natural_size(node->v.p[0])
                );
            gen_expr(node->v.p[1], TRUE, FALSE, natural_size(node->v.p[1]));
            return ap1;
        case en_pfcall:
        case en_pfcallb:
        case en_pcallblock:
            return gen_pfcall(node, novalue);
        case en_sfcall:
        case en_sfcallb:
        case en_scallblock:
        case en_fcall:
        case en_callblock:
        case en_fcallb:
        case en_intcall:
        case en_thiscall:
        case en_trapcall:
            return gen_fcall(node, novalue);
        case en_conslabel:
            if (node->v.p[1]->v.sp->nullsym)
                return 0;
            node->v.p[1]->v.sp->value.classdata.conslabel = lab0 = nextlabel++;
            ap1 = gen_expr(node->v.p[0], novalue, adronly, size);
            gen_label(lab0);
            return ap1;
        case en_destlabel:
            if (node->v.p[1]->v.sp->nullsym)
                return 0;
            node->v.p[1]->v.sp->value.classdata.destlabel = nextlabel;
            gen_label(nextlabel++);
            return gen_expr(node->v.p[0], novalue, adronly, size);
        case en_movebyref:
             /* explicitly uncoded*/
        default:
        case en_repcons:
            DIAG("uncoded node in gen_expr.");
            return 0;
    }
}

//-------------------------------------------------------------------------

AMODE *gen_void(ENODE *node)
{
    gen_expr(node, TRUE, FALSE, natural_size(node));
    gen_code(op_void, 0, 0);
    return 0;
}

//-------------------------------------------------------------------------

int natural_size(ENODE *node)
/*
 *      return the natural evaluation size of a node.
 */
{
    int siz0, siz1;
    if (node == 0)
        return 0;
    switch (node->nodetype)
    {
        case en_bits:
        case en_cl_reg:
            return natural_size(node->v.p[0]);
        case en_icon:
        case en_lcon:
            return  - 4;
        case en_lucon:
        case en_iucon:
            return 4;
        case en_ccon:
        case en_cucon:
            return 1;
        case en_rcon:
        case en_doubleref:
        case en_cd:
            return 8;

        case en_cld:
        case en_lrcon:
        case en_longdoubleref:
            return 10;
        case en_cf:
        case en_fcon:
        case en_floatref:
            return 7;
        case en_fimaginaryref:
        case en_fimaginarycon:
        case en_cfi:
            return 15;
        case en_rimaginaryref:
        case en_rimaginarycon:
        case en_cri:
            return 16;
        case en_lrimaginaryref:
        case en_lrimaginarycon:
        case en_clri:
            return 17;
        case en_fcomplexref:
        case en_fcomplexcon:
        case en_cfc:
            return 20;
        case en_rcomplexref:
        case en_rcomplexcon:
        case en_crc:
            return 21;
        case en_lrcomplexref:
        case en_lrcomplexcon:
        case en_clrc:
            return 22;
        case en_boolcon:
            return 5;
        case en_llcon:
            return  - 6;
        case en_llucon:
            return 6;
        case en_trapcall:
        case en_labcon:
        case en_nacon:
        case en_autocon:
        case en_autoreg:
        case en_napccon:
        case en_absacon:
        case en_nalabcon:
            return stdaddrsize;
        case en_l_ref:
		case en_i_ref:
		case en_a_ref:
        case en_cl:
		case en_ci:
            return  - 4;
        case en_substack:
        case en_structret:
            return 4;
        case en_ll_ref:
        case en_cll:
            return  - 6;
        case en_ull_ref:
        case en_cull:
            return 6;
        case en_thiscall:
            return natural_size(node->v.p[1]);
        case en_scallblock:
        case en_sfcallb:
        case en_sfcall:
        case en_pfcall:
        case en_pfcallb:
             /* ignore pascal style now */
        case en_pcallblock:
        case en_fcall:
        case en_callblock:
        case en_fcallb:
        case en_intcall:
            return natural_size(node->v.p[1]);
        case en_tempref:
        case en_regref:
            return node->v.i >> 16;
        case en_ul_ref:
        case en_cul:
		case en_cui:
		case en_ui_ref:
		case en_ua_ref:
            return 4;
        case en_cp:
            return stdaddrsize;
        case en_ub_ref:
        case en_cub:
            return 1;
        case en_bool_ref:
        case en_cbool:
            return 5;
        case en_b_ref:
        case en_cb:
            return  - 1;
        case en_uw_ref:
        case en_cuw:
            return 2;
        case en_cw:
        case en_w_ref:
            return  - 2;
        case en_not:
        case en_compl:
        case en_uminus:
        case en_assign:
        case en_refassign:
        case en_lassign:
        case en_ainc:
        case en_adec:
        case en_asuminus:
        case en_ascompl:
        case en_moveblock:
        case en_stackblock:
        case en_movebyref:
        case en_clearblock:
            return natural_size(node->v.p[0]);
        case en_add:
        case en_sub:
        case en_addstruc:
        case en_umul:
        case en_udiv:
        case en_umod:
        case en_pmul:
		case en_arrayindex:
        case en_mul:
        case en_div:
        case en_pdiv:
        case en_mod:
        case en_and:
        case en_or:
        case en_xor:
        case en_asalsh:
        case en_asarsh:
        case en_alsh:
        case en_arsh:
        case en_arshd:
        case en_asarshd:
        case en_lsh:
        case en_rsh:
        case en_eq:
        case en_ne:
        case en_lt:
        case en_le:
        case en_gt:
        case en_ge:
        case en_ugt:
        case en_uge:
        case en_ult:
        case en_ule:
        case en_land:
        case en_lor:
        case en_asadd:
        case en_assub:
        case en_asmul:
        case en_asdiv:
        case en_asmod:
        case en_asand:
        case en_asumod:
        case en_asudiv:
        case en_asumul:
        case en_asor:
        case en_aslsh:
        case en_asxor:
        case en_asrsh:
            siz0 = natural_size(node->v.p[0]);
            siz1 = natural_size(node->v.p[1]);
            if (chksize(siz1, siz0))
                return siz1;
            else
                return siz0;
        case en_void:
        case en_cond:
        case en_repcons:
            return natural_size(node->v.p[1]);
		case en_array:
			return BESZ_DWORD;
        case en_voidnz:
        case en_dvoid:
        case en_conslabel:
        case en_destlabel:
            return natural_size(node->v.p[0]);
        default:
            DIAG("natural size error.");
            break;

    }
    return 0;
}

/*
 * subroutine evaluates a node determining how to test it for
 * non-zero
 */
static int defcond(ENODE *node, AMODE **apn)
{
    AMODE *ap1;
    int rv = 0;
    int size;
    if ((size = natural_size(node)) > 6)
    {
        ap1 = gen_expr(node, FALSE, FALSE, BESZ_DWORD);
        do_extend(ap1, 10, F_FREG);
        if (apn)
        {
            *apn = temp_data();
            gen_codes(op_moveq, BESZ_DWORD, make_immed(0),  *apn);
        }
        gen_codef(op_fcmp, size, make_immed(0), ap1);
    }
    else
    {
        if (isbit(node))
        {
            ap1 = gen_expr(node, FALSE, TRUE, 0);
            if (size == 6 || size ==  - 6)
            {
                DIAG("Bit wise compare on long long");
            }
            else
            {
                #ifdef xxxxx
                    if (node->bits == 1)
                    {
                        rv = 1;
                        do_extend(ap1, BESZ_DWORD, F_DREG);
                        if (apn)
                        {
                            *apn = temp_data();
                            gen_codes(op_moveq, BESZ_DWORD, make_immed(0),  *apn);
                        }
                        gen_codes(op_btst, BESZ_DWORD, make_immed(node->startbit), ap1);
                    }
                    else
                #endif 
                {
                    do_extend(ap1, BESZ_DWORD, F_DREG);
                    gen_codes(op_and, BESZ_DWORD, make_immed(mod_mask(node->bits) <<
                        node->startbit), ap1);
                }
            }
        }
        else
        {
            ap1 = gen_expr(node, FALSE, FALSE, 0);
            if (size == 6 || size ==  - 6)
            {
                do_extend(ap1, 6, F_DOUBLEREG);
                if (apn)
                {
                    *apn = temp_data();
                    gen_codes(op_moveq, BESZ_DWORD, make_immed(0),  *apn);
                }
                gen_codes(op_or, BESZ_DWORD, makedreg(0), makedreg(1));
            }
            else
            {
                if (apn)
                {
                    *apn = temp_data();
                    gen_codes(op_moveq, BESZ_DWORD, make_immed(0),  *apn);
                }
                gen_codes(op_cmp, size, make_immed(0), ap1);
            }
        }
    }
    freeop(ap1);
    return rv;
}

//-------------------------------------------------------------------------

static AMODE *truerelat(ENODE *node)
{
    AMODE *ap1;
    if (node == 0)
        return 0;
    switch (node->nodetype)
    {
        case en_eq:
            ap1 = gen_compare(node, op_seq, op_seq, op_seq, op_seq, op_seq,
                op_fseq, op_fseq, 0);
            break;
        case en_ne:
            ap1 = gen_compare(node, op_sne, op_sne, op_sne, op_sne, op_sne,
                op_fsne, op_fsne, 0);
            break;
        case en_lt:
            ap1 = gen_compare(node, op_slt, op_sgt, op_slt, op_sgt, op_slo,
                op_fslt, op_fsnle, 0);
            break;
        case en_le:
            ap1 = gen_compare(node, op_sle, op_sge, op_slt, op_sgt, op_sls,
                op_fsle, op_fsnlt, 0);
            break;
        case en_gt:
            ap1 = gen_compare(node, op_sgt, op_slt, op_sgt, op_slt, op_shi,
                op_fsgt, op_fsnge, 0);
            break;
        case en_ge:
            ap1 = gen_compare(node, op_sge, op_sle, op_sgt, op_slt, op_shs,
                op_fsge, op_fsngt, 0);
            break;
        case en_ult:
            ap1 = gen_compare(node, op_slo, op_shi, op_slo, op_shi, op_slo,
                op_fslt, op_fsnle, 0);
            break;
        case en_ule:
            ap1 = gen_compare(node, op_sls, op_shs, op_slo, op_shi, op_sls,
                op_fsle, op_fsnlt, 0);
            break;
        case en_ugt:
            ap1 = gen_compare(node, op_shi, op_slo, op_shi, op_slo, op_shi,
                op_fsgt, op_fsnge, 0);
            break;
        case en_uge:
            ap1 = gen_compare(node, op_shs, op_sls, op_shi, op_slo, op_shs,
                op_fsge, op_fsngt, 0);
            break;
        case en_not:
            if (isintconst(node->nodetype))
            {
                ap1 = gen_expr(node, FALSE, FALSE, 0);
                ap1->offset->v.i = !ap1->offset->v.i;
                break;
            }
            if (defcond(node->v.p[0], &ap1))
            {
                gen_codes(op_scc, 1, ap1, 0);
            }
            else
            {
                gen_codes(op_seq, 1, ap1, 0);
            }
			gen_codes(op_neg, 1, ap1, 0);
            break;
        default:
            DIAG("True-relat error");
            break;
    }
    ap1->length = BESZ_DWORD;
    return ap1;
}

//-------------------------------------------------------------------------

static int complex_relat(ENODE *node)
{
    if (!node)
        return 0;
    switch (node->nodetype)
    {
        case en_substack:
        case en_structret:
            return 0;
        case en_cl_reg:
            return complex_relat(node->v.p[0]);
        case en_ull_ref:
        case en_ll_ref:
        case en_cull:
        case en_cll:
        case en_llcon:
        case en_llucon:
            return 1;
        case en_bits:		
		case en_a_ref:
		case en_ua_ref:
		case en_i_ref:
		case en_ui_ref:

        case en_l_ref:
        case en_cl:
        case en_ul_ref:
        case en_cul:
		case en_ci:
		case en_cui:
        case en_cp:
        case en_ub_ref:
        case en_cub:
        case en_bool_ref:
        case en_cbool:
        case en_b_ref:
        case en_cb:
        case en_uw_ref:
        case en_cuw:
        case en_cw:
        case en_w_ref:
        case en_cd:
        case en_cld:
        case en_cf:
        case en_cfc:
        case en_cfi:
        case en_crc:
        case en_cri:
        case en_clrc:
        case en_clri:
        case en_uminus:
        case en_ainc:
        case en_adec:
        case en_moveblock:
        case en_stackblock:
        case en_movebyref:
        case en_clearblock:
            return complex_relat(node->v.p[0]);
        case en_icon:
        case en_lcon:
        case en_lucon:
        case en_iucon:
        case en_boolcon:
        case en_ccon:
        case en_cucon:
        case en_rcon:
        case en_doubleref:
        case en_lrcon:
        case en_longdoubleref:
        case en_fcon:
        case en_floatref:
        case en_fimaginaryref:
        case en_rimaginaryref:
        case en_lrimaginaryref:
        case en_fcomplexref:
        case en_rcomplexref:
        case en_lrcomplexref:
        case en_fimaginarycon:
        case en_rimaginarycon:
        case en_lrimaginarycon:
        case en_fcomplexcon:
        case en_rcomplexcon:
        case en_lrcomplexcon:
        case en_absacon:
        case en_trapcall:
        case en_labcon:
        case en_nacon:
        case en_autocon:
        case en_autoreg:
        case en_napccon:
        case en_nalabcon:
        case en_tempref:
        case en_regref:
            return 0;
        case en_eq:
        case en_ne:
        case en_lt:
        case en_le:
        case en_gt:
        case en_ge:
        case en_ugt:
        case en_uge:
        case en_ult:
        case en_ule:
        case en_land:
        case en_lor:
        case en_not:
        case en_compl:
            return 1;
        case en_div:
        case en_udiv:
        case en_pdiv:
        case en_mod:
        case en_umod:
        case en_assign:
        case en_refassign:
        case en_lassign:
        case en_asuminus:
        case en_ascompl:
        case en_add:
        case en_sub:
        case en_addstruc:
        case en_umul:
        case en_pmul:
		case en_arrayindex:
        case en_mul:
        case en_and:
        case en_or:
        case en_xor:
        case en_asalsh:
        case en_asarsh:
        case en_alsh:
        case en_arsh:
        case en_arshd:
        case en_asarshd:
        case en_lsh:
        case en_rsh:
        case en_asadd:
        case en_assub:
        case en_asmul:
        case en_asdiv:
        case en_asmod:
        case en_asand:
        case en_asumod:
        case en_asudiv:
        case en_asumul:
        case en_asor:
        case en_aslsh:
        case en_asxor:
        case en_asrsh:
        case en_repcons:
		case en_array:
            return complex_relat(node->v.p[0]) || complex_relat(node->v.p[1]);
        case en_void:
        case en_cond:
        case en_voidnz:
        case en_dvoid:
            return complex_relat(node->v.p[1]);
        case en_sfcall:
        case en_sfcallb:
        case en_scallblock:
        case en_pfcall:
        case en_pfcallb:
        case en_thiscall:
        case en_fcall:
        case en_intcall:
        case en_callblock:
        case en_fcallb:
        case en_pcallblock:
            return 0;
        case en_conslabel:
        case en_destlabel:
            return complex_relat(node->v.p[0]);
        default:
            DIAG("error in complex_relat routine.");
            return 1;
    }
}

//-------------------------------------------------------------------------

AMODE *gen_relat(ENODE *node)
{
    long lab1;
    AMODE *ap1;
    int size = natural_size(node);
    if (size > 6)
        size = BESZ_DWORD;
    if (node->nodetype != en_land && node->nodetype != en_lor && !complex_relat
        (node->v.p[0]) && !complex_relat(node->v.p[1]))
    {
        ap1 = truerelat(node);
    }
    else
    {
        lab1 = nextlabel++;
        truejp(node, lab1);
        ap1 = temp_data();
        if (ap1->mode != am_dreg)
        {
            DIAG("gen_relat: No free temp regs");
        }
        else
        {
            int lab2 = nextlabel++;
            gen_codes(op_moveq, 0, make_immed(0), ap1);
            gen_code(op_bra, make_label(lab2), 0);
            gen_label(lab1);
            gen_codes(op_moveq, 0, make_immed(1), ap1);
            gen_label(lab2);
            ap1->length = BESZ_DWORD;
        }
    }
    return ap1;
}

//-------------------------------------------------------------------------

AMODE *gen_compare(ENODE *node, int btype1, int btype2, int btype3, int btypeBESZ_DWORD,
    int babst, int fbtype1, int fbtype2, int label)
/*
 *      generate code to do a comparison of the two operands of
 *      node.
 */
{
    AMODE *ap1,  *ap2,  *ap3,  *apx = 0;
    int size;
    int btype = btype1, bitted = FALSE;
    ap3 = 0;
    size = natural_size(node);
    if (size > 6)
    {
        resolve_binary(node, &ap1, &ap2, natural_size(node));
        do_extend(ap1, ap1->length, F_FREG);
        do_extend(ap2, ap2->length, F_FREG);
        if (!label)
        {
            apx = temp_data();
            gen_codes(op_moveq, 0, make_immed(0), apx);
        }
        gen_codef(op_fcmp, size, ap2, ap1);
        if (label)
        {
            gen_branch(fbtype1, make_label(label));
        }
        else
        {
            apx->length = BESZ_DWORD;
            gen_codes(fbtype1, 1, apx, 0);
            gen_codes(op_neg, 1, apx, 0);

        }
        freeop(ap2);
        freeop(ap1);
        return apx;
    }
    if (size == 6 || size ==  - 6)
    {
        ap2 = gen_expr(node->v.p[1], FALSE, FALSE, size);
        do_extend(ap2, 6, F_MEM | F_DOUBLEREG);
        if (ap2->mode == am_doublereg)
        {
            AMODE *ap4;
            ap4 = cmpconvpos();
            ap3 = copy_addr(ap4);
            ap3->offset = makenode(en_add, ap3->offset, makeintnode(en_icon, 4))
                ;
            if (ap3->offset)
                ap3->offset = makenode(en_add, ap3->offset, makeintnode(en_icon,
                    4));
            else
                ap3->offset = makeintnode(en_icon, 4);
            if (ap3->mode == am_ind)
                ap3->mode = am_indx;
            gen_codes(op_move, BESZ_DWORD, makedreg(0), ap4);
            gen_codes(op_move, BESZ_DWORD, makedreg(1), ap3);
            freeop(ap2);
            ap2 = ap4;
        }
        else
        {
            ap3 = copy_addr(ap2);
            if (ap2->mode == am_immed)
            {
                ap3->offset = copy_enode(ap2->offset);
                ap2->offset->v.i >>= 16;
                ap3->offset->v.i &= 0xffffffff;
            }
            else
            {
                if (ap3->offset)
                    ap3->offset = makenode(en_add, ap2->offset, makeintnode
                        (en_icon, 4));
                else
                    ap3->offset = makeintnode(en_icon, 4);
                if (ap3->mode == am_ind)
                    ap3->mode = am_indx;
            }
        }
        ap1 = gen_expr(node->v.p[0], FALSE, FALSE, size);
        do_extend(ap1, 6, F_DOUBLEREG);
        if (btype1 == op_beq || btype1 == op_seq || btype1 == op_bne || btype1 
            == op_sne)
        {
            int lbl1 = nextlabel++;
            ap2->length = ap3->length = BESZ_DWORD;
            gen_codes(op_sub, BESZ_DWORD, ap3, makedreg(0));
            gen_codes(op_sub, BESZ_DWORD, ap2, makedreg(1));
            gen_1branch(op_bcc, make_label(lbl1));
            gen_codes(op_sub, BESZ_DWORD, make_immed(1), makedreg(0));
            gen_label(lbl1);
            gen_code(op_or, makedreg(0), makedreg(1));
            freeop(ap1);
            freeop(ap2);
            if (label)
            {
                gen_branch(btype1, make_label(label));
            }
            else
            {
                ap1 = temp_data();
                ap1->length = 1;
                gen_code(fbtype1, ap1, 0);
            }
        }
        else
        {
            int templab = nextlabel++;
            if (label)
            {
                gen_codes(op_cmp, BESZ_DWORD, ap3, makedreg(0));
                gen_branch(btype3, make_label(label));
                gen_1branch(btypeBESZ_DWORD, make_label(templab));
                gen_codes(op_cmp, BESZ_DWORD, ap2, makedreg(1));
                gen_branch(babst, make_label(label));
                gen_label(templab);
                freeop(ap1);
                freeop(ap2);
            }
            else
            {
                AMODE *ap5;
                AMODE *ap4;
                ap4 = cmpconvpos();
                ap5 = copy_addr(ap4);
                if (ap4->offset)
                    ap5->offset = makenode(en_add, ap5->offset, makeintnode
                        (en_icon, 4));
                else
                    ap5->offset = makeintnode(en_icon, BESZ_DWORD);
                if (ap5->mode == am_ind)
                    ap5->mode = am_indx;
                gen_codes(op_cmp, BESZ_DWORD, ap3, makedreg(0));
                gen_codes(btype1, 1, ap4, 0);
                gen_codes(op_cmp, BESZ_DWORD, ap2, makedreg(1));
                gen_codes(btype1, 1, ap5, 0);
                freeop(ap1);
                freeop(ap2);
                ap1 = temp_data();
                ap1->length = 1;
                gen_codes(op_move, 1, ap4, ap1);
                gen_1branch(op_beq, make_label(templab));
                gen_codes(op_move, 1, ap5, ap1);
                gen_label(templab);

            }
        }
        return ap1;
    }
    if ((isbit(node->v.p[0]) || isbit(node->v.p[1])) && (btype1 == op_beq ||
        btype1 == op_bne || btype1 == op_seq || btype1 == op_sne))
    {
        ENODE *xnode;
        bitted = TRUE;
        if (isbit(node->v.p[0]))
        {
            ap2 = gen_expr(node->v.p[1], FALSE, FALSE, BESZ_DWORD);
            noids(ap2);
            ap1 = gen_expr(xnode = node->v.p[0], FALSE, TRUE, BESZ_DWORD);
        }
        else
        {
            ap2 = gen_expr(node->v.p[0], FALSE, FALSE, BESZ_DWORD);
            noids(ap2);
            ap1 = gen_expr(xnode = node->v.p[1], FALSE, TRUE, BESZ_DWORD);
        }
        #ifdef XXXXX
            if (ap2->mode == am_immed && (ap1->mode == am_dreg || xnode
                ->startbit < 8))
            {
                if (xnode->bits == 1)
                {
                    if (!(ap2->offset->v.i &1))
                    {
                        if (!label)
                        {
                            apx = temp_data();
                            gen_codes(op_moveq, 0, make_immed(0), apx);
                        }
                        gen_codes(op_btst, BESZ_DWORD, make_immed(xnode->startbit), ap1);
                        switch (btype1)
                        {
                            case op_beq:
                                btype = op_bcc;
                                break;
                            case op_bne:
                                btype = op_bcs;
                                break;
                            case op_seq:
                                btype = op_scc;
                                break;
                            case op_sne:
                                btype = op_scs;
                                break;
                        }
                    }
                    else
                    {
                        if (!label)
                        {
                            apx = temp_data();
                            gen_codes(op_moveq, 0, make_immed(0), apx);
                        }
                        gen_codes(op_btst, BESZ_DWORD, make_immed(xnode->startbit), ap1);
                        switch (btype1)
                        {
                            case op_beq:
                                btype = op_bcs;
                                break;
                            case op_bne:
                                btype = op_bcc;
                                break;
                            case op_seq:
                                btype = op_scs;
                                break;
                            case op_sne:
                                btype = op_scc;
                                break;
                        }
                    }
                }
                else
                    goto join;
            }
            else
        #endif 
        {
            join: ap1 = bit_load(ap1, xnode, BESZ_DWORD);
            goto join2;
        }
    }
    else
    {
        resolve_binary(node, &ap1, &ap2, prm_coldfire ? 4 : 0);
        join2: if (ap1->mode != am_dreg && ap1->mode != am_areg)
        {
            if (ap2->mode == am_immed)
            {
                if (ap1->mode == am_immed)
                    goto cmp2;
            }
            else
            {
                if (ap2->mode == am_dreg || ap2->mode == am_areg)
                {
                    swapit: ap3 = ap2;
                    ap2 = ap1;
                    ap1 = ap3;
                    ap3 = 0;
                    btype = btype2;
                }
                else
                if (ap1->mode == am_immed)
                {
                    goto swapit;
                }
                else
                {
                    cmp2: ap3 = ap1;
                    ap1 = temp_data();
                    gen_codes(op_move, size, ap3, ap1);
                    freeop(ap3);
                    do_extend(ap1, size, F_DREG);
                }
            }
        }
        if (!label)
        {
            apx = temp_data();
            gen_codes(op_moveq, 0, make_immed(0), apx);
        }
        if (prm_coldfire && size < BESZ_DWORD && size >  - BESZ_DWORD)
        {
            do_extend(ap1, BESZ_DWORD, F_DREG);
            do_extend(ap2, BESZ_DWORD, F_DREG);
        }
        gen_codes(op_cmp, prm_coldfire ? BESZ_DWORD : size, ap2, ap1);
    }
    freeop(ap1);
    freeop(ap2);
    if (!bitted && ap2->mode == am_immed && ap2->offset->v.i == 0)
    {
        switch (btype)
        {
            case op_shi:
                btype = op_sne;
                break;
            case op_scc:
                gen_code(op_move, makedreg(0), makedreg(0));
                break;
            case op_scs:
                gen_code(op_move, makedreg(0), makedreg(0));
                break;
            case op_slo:
                btype = op_seq;
                break;
            case op_bhi:
                btype = op_beq;
                break;
            case op_bcc:
                btype = op_bra; /* Following code never gets executed */
                break;
            case op_bcs:
                 /* Following code always gets executed */
                return ap1;
            case op_blo:
                btype = op_beq;
                break;
        }
    }
    if (label)
    {
        gen_branch(btype, make_label(label));
    }
    else
    {
        apx->length = BESZ_DWORD;
        gen_codes(btype, 1, apx, 0);
        gen_codes(op_neg, 1, apx, 0);

    }
    freeop(ap2);
    freeop(ap1);
    return apx;
}

//-------------------------------------------------------------------------

void truejp(ENODE *node, int label)
/*
 *      generate a jump to label if the node passed evaluates to
 *      a true condition.
 */
{
    AMODE *ap1;
    int siz1;
    int lab0;
    if (node == 0)
        return ;
    switch (node->nodetype)
    {
        case en_eq:
            gen_compare(node, op_beq, op_beq, op_beq, op_beq, op_beq, op_fbeq,
                op_fbeq, label);
            break;
        case en_ne:
            gen_compare(node, op_bne, op_bne, op_bne, op_bne, op_bne, op_fbne,
                op_fbne, label);
            break;
        case en_lt:
            gen_compare(node, op_blt, op_bgt, op_blt, op_bgt, op_blo, op_fblt,
                op_fbnle, label);
            break;
        case en_le:
            gen_compare(node, op_ble, op_bge, op_blt, op_bgt, op_bls, op_fble,
                op_fbnlt, label);
            break;
        case en_gt:
            gen_compare(node, op_bgt, op_blt, op_bgt, op_blt, op_bhi, op_fbgt,
                op_fbnge, label);
            break;
        case en_ge:
            gen_compare(node, op_bge, op_ble, op_bgt, op_blt, op_bhs, op_fbge,
                op_fbngt, label);
            break;
        case en_ult:
            gen_compare(node, op_blo, op_bhi, op_blo, op_bhi, op_blo, op_fblt,
                op_fbnle, label);
            break;
        case en_ule:
            gen_compare(node, op_bls, op_bhs, op_blo, op_bhi, op_bls, op_fble,
                op_fbnlt, label);
            break;
        case en_ugt:
            gen_compare(node, op_bhi, op_blo, op_bhi, op_blo, op_bhi, op_fbgt,
                op_fbnge, label);
            break;
        case en_uge:
            gen_compare(node, op_bhs, op_bls, op_bhi, op_blo, op_bhs, op_fbge,
                op_fbngt, label);
            break;
        case en_land:
            lab0 = nextlabel++;
            falsejp(node->v.p[0], lab0);
            truejp(node->v.p[1], label);
            gen_label(lab0);
            break;
        case en_lor:
            truejp(node->v.p[0], label);
            truejp(node->v.p[1], label);
            break;
        case en_not:
            falsejp(node->v.p[0], label);
            break;
        default:
            siz1 = natural_size(node);
            if (isintconst(node->nodetype))
            {
                if (node->v.i != 0)
                    gen_code(op_bra, make_label(label), 0);
                break;
            }
            if (siz1 > 6)
                if (defcond(node, 0))
                    gen_branch(op_fbge, make_label(label));
                else
                    gen_branch(op_fbne, make_label(label));
                else
                    if (defcond(node, 0))
                        gen_branch(op_bcc, make_label(label));
                    else
                        gen_branch(op_bne, make_label(label));
            break;
    }
}

//-------------------------------------------------------------------------

void falsejp(ENODE *node, int label)
/*
 *      generate code to execute a jump to label if the expression
 *      passed is false.
 */
{
    AMODE *ap;
    int siz1;
    int lab0;
    if (node == 0)
        return ;
    switch (node->nodetype)
    {
        case en_eq:
            gen_compare(node, op_bne, op_bne, op_bne, op_bne, op_bne, op_fbne,
                op_fbne, label);
            break;
        case en_ne:
            gen_compare(node, op_beq, op_beq, op_beq, op_beq, op_beq, op_fbeq,
                op_fbeq, label);
            break;
        case en_lt:
            gen_compare(node, op_bge, op_ble, op_bgt, op_blt, op_bhs, op_fbge,
                op_fbngt, label);
            break;
        case en_le:
            gen_compare(node, op_bgt, op_blt, op_bgt, op_blt, op_bhi, op_fbgt,
                op_fbnge, label);
            break;
        case en_gt:
            gen_compare(node, op_ble, op_bge, op_blt, op_bgt, op_bls, op_fble,
                op_fbnlt, label);
            break;
        case en_ge:
            gen_compare(node, op_blt, op_bgt, op_blt, op_bgt, op_blo, op_fblt,
                op_fbnle, label);
            break;
        case en_ult:
            gen_compare(node, op_bhs, op_bls, op_bhi, op_blo, op_bhs, op_fbge,
                op_fbngt, label);
            break;
        case en_ule:
            gen_compare(node, op_bhi, op_blo, op_bhi, op_blo, op_bhi, op_fbgt,
                op_fbnge, label);
            break;
        case en_ugt:
            gen_compare(node, op_bls, op_bhs, op_blo, op_bhi, op_bls, op_fble,
                op_fbnlt, label);
            break;
        case en_uge:
            gen_compare(node, op_blo, op_bhi, op_blo, op_bhi, op_blo, op_fblt,
                op_fbnle, label);
            break;
        case en_land:
            falsejp(node->v.p[0], label);
            falsejp(node->v.p[1], label);
            break;
        case en_lor:
            lab0 = nextlabel++;
            truejp(node->v.p[0], lab0);
            falsejp(node->v.p[1], label);
            gen_label(lab0);
            break;
        case en_not:
            truejp(node->v.p[0], label);
            break;
        default:
            siz1 = natural_size(node);
            if (isintconst(node->nodetype))
            {
                if (node->v.i == 0)
                    gen_code(op_bra, make_label(label), 0);
                break;
            }
            if (siz1 > 6)
                if (defcond(node, 0))
                    gen_branch(op_fbge, make_label(label));
                else
                    gen_branch(op_fbeq, make_label(label));
                else
                    if (defcond(node, 0))
                        gen_branch(op_bcc, make_label(label));
                    else
                        gen_branch(op_beq, make_label(label));
            break;
    }
}
