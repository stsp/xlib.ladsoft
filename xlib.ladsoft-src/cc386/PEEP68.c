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
 * peepcode optimizations
 * 26-Aug-1999 (SJS) Some changes for ColdFire instructions.
 * 28-Aug-1999 (SJS) 68020 compile now allowed with ColdFire restrictions on
 *	baseindex modes implemented in gen_code(). This is not the most elegant
 *	solution, but if someone wants to re-do gen_index() in gexpr68.c, please
 *	feel free...
 * 23-Oct-99 (SJS) Fixed enforcer hits with baseoffset checking in gen_code().
 * 07-jul-2001 (DAL) moved the 28 aug fixed to gen_index()
 */
#include <stdio.h>
#include <string.h>
#include "lists.h"
#include "expr.h"
#include "c.h"
#include "gen68.h"
#include "diag.h"

extern int prm_coldfire;
extern int prm_asmfile;
extern SYM *currentfunc;
extern int nextlabel;
extern int prm_peepopt;

OCODE *peep_head = 0, *peep_tail = 0, *peep_insert;

extern int prm_buggyclr;

void peepini(void)
{
    peep_head = peep_tail = peep_insert = 0;
}

//-------------------------------------------------------------------------

AMODE *copy_addr(AMODE *ap)
/*
 *      copy an address mode structure (these things dont last).
 */
{
    AMODE *newap;
    if (ap == 0)
        return 0;
    newap = xalloc(sizeof(AMODE));
    newap->mode = ap->mode;
    newap->preg = ap->preg;
    newap->sreg = ap->sreg;
    newap->scale = ap->scale;
    newap->length = ap->length;
    newap->tempflag = ap->tempflag;
    newap->offset = ap->offset;
    return newap;
}

//-------------------------------------------------------------------------

void gen_code(int op, AMODE *ap1, AMODE *ap2)
/*
 *      generate a code sequence into the peep list.
 */
{
    OCODE *new;
    new = xalloc(sizeof(OCODE));
    new->opcode = op;
    new->noopt = FALSE;
    if (ap1)
        if (ap1->length < 0)
            new->length =  - ap1->length;
        else
            new->length = ap1->length;
        else
            new->length = 0;
    new->oper1 = copy_addr(ap1);
    new->oper2 = copy_addr(ap2);
    new->oper3 = 0;
    add_peep(new);
}

//-------------------------------------------------------------------------

AMODE *gen_codes(int op, int len, AMODE *ap1, AMODE *ap2)
/*
 *      generate a code sequence into the peep list.
 */
{
    OCODE *new;
    new = xalloc(sizeof(OCODE));
    new->opcode = op;
    new->noopt = FALSE;
    if (len < 0)
        new->length =  - len;
    else
        new->length = len;
    new->oper1 = copy_addr(ap1);
    new->oper2 = copy_addr(ap2);
    new->oper3 = 0;
    add_peep(new);
    return new;
}

//-------------------------------------------------------------------------

void gen_coden(int op, int len, AMODE *ap1, AMODE *ap2)
{
    OCODE *new = gen_codes(op, len, ap1, ap2);
    new->noopt = TRUE;
}

//-------------------------------------------------------------------------

void gen_branch(int op, AMODE *ap1)
{
    gen_code(op, ap1, 0);
}

//-------------------------------------------------------------------------

void gen_1branch(int op, AMODE *ap1)
{
    gen_codes(op, 1, ap1, 0);
}

//-------------------------------------------------------------------------

void gen_codeb(int op, int len, AMODE *ap1, AMODE *ap2)
/*
 *   		move ap1 to dreg if we don't have a valid pair for math functions
 */
{
    AMODE *ap = ap1;
    if (ap1->mode != am_dreg && (ap2->mode != am_dreg && ap2->mode != am_areg))
    {
        ap = temp_data();
        gen_codes(op_move, len, ap1, ap);
    }
    gen_codes(op, len, ap, ap2);
    if (ap != ap1)
        freeop(ap);
}

//-------------------------------------------------------------------------

void gen_lea(int size, AMODE *ap1, AMODE *ap2)
{
    AMODE *ap3,  *ap4;
    if (ap1->mode == am_ainc || ap1->mode == am_adec)
    {
        enum e_am om = ap1->mode;
        if (!size)
        {
            size = 1;
            DIAG("Illegal len in autoinc lea");
        } if (size < 0)
            size =  - size;
        ap1->mode = am_ind;
        ap3 = copy_addr(ap1);
        ap3->mode = am_areg;
        ap4 = make_immed(size);
        gen_codes(op_move, 4, ap3, ap2);
        if (om == am_ainc)
            gen_codes(op_add, 4, ap4, ap3);
        else
            gen_codes(op_sub, 4, ap4, ap3);
        ap1->mode = om;
    }
    else
    if (ap2->mode == am_areg)
    {
        if (ap1->mode == am_baseindxaddr && !ap1->scale)
        {
            if (ap1->preg == ap2->preg && !ap1->offset->v.i)
            {
                ap3 = xalloc(sizeof(AMODE));
                ap3->mode = am_areg;
                ap3->preg = ap1->sreg;
                gen_codes(op_add, 4, ap3, ap2);
            }
            else if (ap1->sreg == ap2->preg && !ap1->offset->v.i)
            {
                ap3 = xalloc(sizeof(AMODE));
                ap3->mode = am_areg;
                ap3->preg = ap1->preg;
                gen_codes(op_add, 4, ap3, ap2);
            }
            else
                gen_codes(op_lea, 0, ap1, ap2);
        }
        else if (ap1->mode == am_baseindxdata && !ap1->offset->v.i && !ap1
            ->scale)
        {
            if (ap1->preg == ap2->preg)
            {
                ap3 = xalloc(sizeof(AMODE));
                ap3->mode = am_dreg;
                ap3->preg = ap1->sreg;
                gen_codes(op_add, 4, ap3, ap2);
            }
            else
                gen_codes(op_lea, 0, ap1, ap2);
        }
        else
            gen_codes(op_lea, 0, ap1, ap2);
    }
    else
        gen_codes(op_lea, 0, ap1, ap2);
}

//-------------------------------------------------------------------------

void gen_codelab(SYM *lab)
/*
 *      generate a code sequence into the peep list.
 */
{
    OCODE *new;
    new = xalloc(sizeof(OCODE));
    new->opcode = op_funclabel;
    new->length = 0;
    new->oper1 = lab;
    new->oper2 = 0;
    new->oper3 = 0;
    add_peep(new);
}

//-------------------------------------------------------------------------

void gen_line(SNODE *stmt)
{
    OCODE *new = xalloc(sizeof(OCODE));
    new->opcode = op_line;
    new->length = (int)stmt->exp;
    new->oper2 = (AMODE*)((int)stmt->s1);
    new->oper1 = (AMODE*)stmt->label;
    new->oper3 = 0;
    add_peep(new);
}

//-------------------------------------------------------------------------

void gen_codef(int op, int len, AMODE *ap1, AMODE *ap2)
{
    if (ap1->mode == am_freg)
    {
        if (!ap2 || ap2->mode == am_freg)
            len = 10;
    }
    else if (ap2 && ap2->mode == am_freg)
    {
        if (ap1->mode == am_immed)
            len = 10;
    }
    gen_codes(op, len, ap1, ap2);
}

//-------------------------------------------------------------------------

void gen_code3(int op, int len, AMODE *ap1, AMODE *ap2, AMODE *ap3)
{
    OCODE *new;
    new = xalloc(sizeof(OCODE));
    new->opcode = op;
    if (len < 0)
        new->length =  - len;
    else
        new->length = len;
    new->oper1 = copy_addr(ap1);
    new->oper2 = copy_addr(ap2);
    new->oper3 = copy_addr(ap3);
    add_peep(new);
}

//-------------------------------------------------------------------------

void add_peep(OCODE *new)
/*
 *      add the ocoderuction pointed to by new to the peep list.
 */
{
	if (new->opcode == op_btst)
		if ((new->length == 1 || new->length == -1) && new->oper2->mode == am_dreg)
			new->length = 0;
    if (peep_head == 0)
    {
        peep_head = peep_tail = peep_insert = new;
        new->fwd = 0;
        new->back = 0;
    }
    else
    {
        new->fwd = 0;
        new->back = peep_tail;
        peep_tail->fwd = new;
        peep_tail = new;
    }
}

//-------------------------------------------------------------------------

void gen_label(int labno)
/*
 *      add a compiler generated label to the peep list.
 */
{
    OCODE *new;
    new = xalloc(sizeof(OCODE));
    new->opcode = op_label;
    new->oper1 = (AMODE*)labno;
    add_peep(new);
}

//-------------------------------------------------------------------------

void flush_peep(void)
/*
 *      output all code and labels in the peep list.
 */
{
    opt3(); /* do the peephole optimizations */
    opt3();

    if (peep_head)
        outcode_gen(peep_head);
    if (prm_asmfile)
    {
        while (peep_head != 0)
        {
            switch (peep_head->opcode)
            {
                case op_label:
                    put_label((int)peep_head->oper1);
                    break;
                case op_funclabel:
                    gen_strlab(peep_head->oper1);
                    break;
                default:
                    put_code(peep_head);
                    break;

            }
            peep_head = peep_head->fwd;
        }
    }
    peep_head = 0;
}

//-------------------------------------------------------------------------

void peep_move(OCODE *ip)
/*
 *      peephole optimization for move instructions.
 *      makes quick immediates when possible.
 *      changes move #0,d to clr d.
 *      changes long moves to address registers to short when
 *              possible.
 *      changes move immediate to stack to pea.
 */
{
    ENODE *ep;
    /* SCREEN out moves to self */
    if (equal_address(ip->oper1, ip->oper2))
    {
        ip->fwd->back = ip->back;
        ip->back->fwd = ip->fwd;
        return ;
    }
    if (ip->oper2->mode == am_areg)
        ip->opcode = op_movea;
    /* get rid of extraneous temp regs */
    if (!prm_coldfire && ip->oper2->mode == am_dreg)
    if (ip->fwd && ip->fwd->opcode == op_move && ip->fwd->oper1->mode ==
        am_dreg && ip->fwd->oper1->preg == ip->oper2->preg && ip->length == ip
        ->fwd->length)
    {
        ip->oper2 = ip->fwd->oper2;
        ip->fwd->fwd->back = ip;
        ip->fwd = ip->fwd->fwd;
    }
    if (ip->oper1->mode != am_immed)
        return ;
    ep = ip->oper1->offset;
    if (!isintconst(ep->nodetype))
        return ;
    if (ip->oper2->mode == am_areg)
    {
        if ( - 32768L <= ep->v.i && ep->v.i <= 32768L)
            ip->length = 2;
    }
    else if (ip->oper2->mode == am_dreg)
    {
        if ( - 128 <= ep->v.i && ep->v.i <= 127)
        {
            ip->opcode = op_moveq;
            ip->length = 0;
        }
    }
    else
    {
        if (ep->v.i == 0 && !prm_buggyclr)
        {
            ip->opcode = op_clr;
            ip->oper1 = ip->oper2;
            ip->oper2 = 0;
        }
        else if (ip->oper2->mode == am_adec && ip->oper2->preg == 7)
        {
            ip->opcode = op_pea;
            ip->length = 0;
            ip->oper1->mode = am_direct;
            ip->oper2 = 0;
        }
    }
}

/*
 * get rid of a TST after any other instruction that sets flags if the
 * args match
 */
int peep_tst(OCODE *ip)
{
    if (ip->back->opcode == op_move || ip->back->opcode == op_and || ip->back
        ->opcode == op_or || ip->back->opcode == op_andi || ip->back->opcode ==
        op_ori || ip->back->opcode == op_add || ip->back->opcode == op_addi ||
        ip->back->opcode == op_addq || ip->back->opcode == op_sub || ip->back
        ->opcode == op_subi || ip->back->opcode == op_subq)
    {
        if (equal_address(ip->back->oper2, ip->oper1))
        {
            ip->back->fwd = ip->fwd;
            ip->fwd->back = ip->back;
        }
    }
}

/*
 * this returns the address of a single bit, or -1
 */
int single_bit(int val)
{
    unsigned long v = 1, t = val;
    int i;
    for (i = 0; i < 32; i++, v <<= 1)
    {
        if (v > t)
            return  - 1;
        if (v == t)
            return i;
    }
    return  - 1;

}

//-------------------------------------------------------------------------

int equal_address(AMODE *ap1, AMODE *ap2)
/*
 *      compare two address nodes and return true if they are
 *      equivalent.
 */
{
    if (ap1 == 0 || ap2 == 0)
        return 0;
    if (ap1->mode != ap2->mode)
        return 0;
    switch (ap1->mode)
    {
        case am_areg:
        case am_dreg:
        case am_ainc:
        case am_adec:
            return ap1->preg == ap2->preg;
        case am_baseindxaddr:
        case am_baseindxdata:
        case am_indx:
            if (ap1->preg != ap2->preg)
                return FALSE;
            if (ap1->sreg != ap2->sreg)
                return FALSE;
            return equalnode(ap1->offset, ap2->offset);
        case am_immed:
        case am_direct:
        case am_adirect:
        case am_pcindx:
            return equalnode(ap1->offset, ap2->offset);
    }
    return 0;
}

//-------------------------------------------------------------------------

void peep_add(OCODE *ip)
/*
 *      peephole optimization for add instructions.
 *      makes quick immediates out of small constants.
 */
{
    ENODE *ep;
    if (ip->oper1->mode == am_immed && ip->oper1->offset->v.i == 0)
    {
        ip->back->fwd = ip->fwd;
        ip->fwd->back = ip->back;
        return ;
    }
    if (ip->oper2->mode == am_areg)
        ip->opcode = op_adda;
    if (ip->oper1->mode != am_immed)
        return ;
    ep = ip->oper1->offset;
    if (ip->oper2->mode != am_areg)
        ip->opcode = op_addi;
    #ifdef XXXXX
        else
        {
            if (isshort(ep))
                ip->length = 2;
        }
    #endif 
    if (!(isintconst(ep->nodetype)))
        return ;
    if (1 <= ep->v.i && ep->v.i <= 8)
        ip->opcode = op_addq;
    else if ( - 8 <= ep->v.i && ep->v.i <=  - 1)
    {
        ip->opcode = op_subq;
        ep->v.i =  - ep->v.i;
    }
}

//-------------------------------------------------------------------------

void peep_sub(OCODE *ip)
/*
 *      peephole optimization for subtract instructions.
 *      makes quick immediates out of small constants.
 */
{
    ENODE *ep;
    if (ip->oper1->mode == am_immed && ip->oper1->offset->v.i == 0)
    {
        ip->back->fwd = ip->fwd;
        ip->fwd->back = ip->back;
        return ;
    }
    if (ip->oper2->mode == am_areg)
        ip->opcode = op_suba;
    if (ip->oper1->mode != am_immed)
        return ;
    ep = ip->oper1->offset;
    if (ip->oper2->mode != am_areg)
        ip->opcode = op_subi;
    #ifdef XXXXX
        else
        {
            if (isshort(ep))
                ip->length = 2;
        }
    #endif 
    if (!isintconst(ep->nodetype))
        return ;
    if (1 <= ep->v.i && ep->v.i <= 8)
    {
        ip->opcode = op_subq;
        if (ip->oper2->mode == am_areg && ip->length <= 4 && ip->fwd->opcode !=
            op_line && ip->fwd->opcode != op_label && ip->fwd->oper1 && ip->fwd
            ->oper1->mode == am_ind && ip->oper2->preg == ip->fwd->oper1->preg)
        {
            int sz1 = ip->fwd->length;
            int sz2 = ep->v.i;
            if (sz1 < 0)
                sz1 =  - sz1;
            if (sz2 < 0)
                sz2 =  - sz2;
            if (sz2 == sz1)
            {
                ip->back->fwd = ip->fwd;
                ip->fwd->back = ip->back;
                ip->fwd->oper1->mode = am_adec;
            }
        }
    }
    else if ( - 8 <= ep->v.i && ep->v.i <=  - 1)
    {
        ip->opcode = op_addq;
        ep->v.i =  - ep->v.i;
    }
}

//-------------------------------------------------------------------------

void peep_andi(OCODE *ip)
{
    OCODE *ipf = ip->fwd;
    if (ipf->opcode != op_andi)
        return ;
    if (ip->oper1->mode != am_immed || ipf->oper1->mode != am_immed)
        return ;
    if (!equal_address(ip->oper2, ipf->oper2))
        return ;
    ip->oper1->offset->v.i &= ipf->oper1->offset->v.i;
    ipf->back->fwd = ipf->fwd;
    ipf->fwd->back = ipf->back;
}

//-------------------------------------------------------------------------

void peep_cmp(OCODE *ip)
/*
 *      peephole optimization for compare instructions.
 *      changes compare #0 to tst and if previous instruction
 *      should have set the condition codes properly delete.
 *      return value is true if instruction was deleted.
 */
{
    OCODE *prev = 0;
    ENODE *ep;
    if (ip->oper2->mode == am_areg)
        ip->opcode = op_cmpa;
    if (ip->oper1->mode == am_immed)
    {
        ep = ip->oper1->offset;
        if (ip->oper2->mode == am_areg)
        {
            if (isshort(ep))
                ip->length = 2;
            return ;
        }
        ip->opcode = op_cmpi;
        if (isintconst(ep->nodetype) && ep->v.i == 0)
        {
            if (ip->fwd->opcode == op_bne || ip->fwd->opcode == op_beq)
            {
                ip->oper1 = ip->oper2;
                ip->oper2 = 0;
                ip->opcode = op_tst;
                prev = ip->back;
            }
        }
    }
    if (prev == 0)
        return ;
    if ((((prev->opcode == op_move || prev->opcode == op_moveq || prev->opcode 
        == op_add || prev->opcode == op_addi || prev->opcode == op_sub || prev
        ->opcode == op_subi || prev->opcode == op_addq || prev->opcode ==
        op_subq) && equal_address(prev->oper1, ip->oper1)) && prev->oper2->mode
        != am_areg) || (prev->opcode != op_label && equal_address(prev->oper2,
        ip->oper1)))
    {
        prev->fwd = ip->fwd;
        if (prev->fwd != 0)
            prev->fwd->back = prev;
    }
}

//-------------------------------------------------------------------------

void peep_muldiv(OCODE *ip, int op)
/*
 *      changes multiplies and divides by convienient values
 *      to shift operations. op should be either op_asl or
 *      op_asr (for divide).
 */
{
    long shcnt;
    if (ip->oper1->mode != am_immed)
        return ;
    if (!isintconst(ip->oper1->offset->nodetype))
        return ;
    shcnt = pwrof2(ip->oper1->offset->v.i);
    if (shcnt ==  - 1)
        return ;
    ip->oper1->offset->v.i = shcnt;
    ip->opcode = op;
    ip->length = 4;
}

//-------------------------------------------------------------------------

void peep_lea(OCODE *ip)
{
    OCODE *ip1 = ip->fwd;
    if (ip1->opcode == op_move && ip1->oper1->mode == am_areg && ip1->oper1
        ->preg == ip->oper2->preg)
    if (ip1->oper2->mode == am_adec && ip1->oper2->preg == 7)
    {
        ip->opcode = op_pea;
        ip->oper2 = 0;
        ip->fwd = ip1->fwd;
        ip1->fwd->back = ip;
    }
}

//-------------------------------------------------------------------------

void peep_uctran(OCODE *ip)
/*
 *      peephole optimization for unconditional transfers.
 *      deletes instructions which have no path.
 *      applies to bra, jmp, and rts instructions.
 */
{
    while (ip->fwd != 0 && ip->fwd->opcode != op_label)
    {
        ip->fwd = ip->fwd->fwd;
        if (ip->fwd != 0)
            ip->fwd->back = ip;
    }
}

//-------------------------------------------------------------------------

void peep_label(OCODE *ip)
/*
 *		peephole optimization for labels
 *		deletes relbranches that jump to the next instruction
 */
{
    OCODE *curpos,  *index;
    curpos = ip;

    if (!curpos->back)
        return ;
    do
    {
        curpos = curpos->back;
    }
    while (curpos && (curpos->opcode == op_label || curpos->opcode == op_line ||
			   curpos->opcode == op_blockstart || curpos->opcode == op_blockend));
    if (!curpos)
        return ;
    while ((curpos->opcode == op_bra) || curpos->opcode == op_cmp || curpos
        ->opcode == op_cmpi || curpos->opcode == op_tst || (curpos->opcode ==
        op_bne) || (curpos->opcode == op_beq) || (curpos->opcode == op_bge) || 
        (curpos->opcode == op_ble) || (curpos->opcode == op_bgt) || (curpos
        ->opcode == op_blt) || (curpos->opcode == op_bhs) || (curpos->opcode ==
        op_bls) || (curpos->opcode == op_bhi) || (curpos->opcode == op_blo))
    {
        index = curpos->fwd;
        if ((curpos->opcode == op_cmpi || curpos->opcode == op_cmp || curpos
            ->opcode == op_tst))
        {
            if (curpos->fwd->opcode == op_label)
            {
                curpos->back->fwd = curpos->fwd;
                curpos->fwd->back = curpos->back;
                curpos = curpos->back;
            }
            else
                break;
        }
        else
        {
            do
            {
                if ((index->opcode == op_label) && (curpos->oper1->mode ==
                    am_direct) && ((int)index->oper1 == curpos->oper1->offset
                    ->v.i))
                {
                    curpos->back->fwd = curpos->fwd;
                    curpos->fwd->back = curpos->back;
                    curpos = curpos->back;
                    break;
                }
                index = index->fwd;
            }
            while (index != ip->fwd);
            if (index == ip->fwd)
                break;
        }
        while (curpos->opcode == op_label || curpos->opcode == op_line || 
			   curpos->opcode == op_blockstart || curpos->opcode == op_blockend)
            curpos = curpos->back;
    }
}

//-------------------------------------------------------------------------

void opt3(void)
/*
 *      peephole optimizer. This routine calls the instruction
 *      specific optimization routines above for each instruction
 *      in the peep list.
 */
{
    OCODE *ip = peep_head;
    if (!prm_peepopt)
        return ;
    while (ip != 0)
    {
        if (!ip->noopt)
        {
            if (ip->opcode != op_line && ip->opcode != op_label && ip->opcode 
                != op_slit)
            {
                if (ip->oper1 && ip->oper1->mode == am_indx && ip->oper1
                    ->offset->v.i == 0)
                    ip->oper1->mode = am_ind;
                if (ip->oper2 && ip->oper2->mode == am_indx && ip->oper2
                    ->offset->v.i == 0)
                    ip->oper2->mode = am_ind;
            }
            // Fix references to FP regs to use size 'X'
            switch (ip->opcode)
            {
                case op_line:
                case op_label:
                case op_seqx:
                case op_slit:
                case op_funclabel:
                case op_genword:
                case op_void:
                case op_dcl:
                case op_dcr:
                case op_blockstart:
                case op_blockend:
                    break;
                default:
                    if (ip->oper1)
                        if (ip->oper1->mode == am_freg)
                            if (!ip->oper2 || ip->oper2->mode == am_freg)
                                ip->length = 10;
                    break;
            }
            switch (ip->opcode)
            {
                case op_move:
                    peep_move(ip);
                    break;
                case op_add:
                    peep_add(ip);
                    break;
                case op_sub:
                    peep_sub(ip);
                    break;
                case op_tst:
                    peep_tst(ip);
                    break;
                case op_cmp:
                    peep_cmp(ip);
                    break;
                case op_andi:
                    peep_andi(ip);
                    break;
                case op_and:
                    if (ip->oper1->mode == am_immed)
                    {
                        ip->opcode = op_andi;
                        if (ip->oper1->offset->v.i == 0)
                            ip->opcode = op_move;
                    }
                    break;
                case op_or:
                    if (ip->oper1->mode == am_immed)
                    {
                        ip->opcode = op_ori;
                        if (ip->oper1->offset->v.i == 0)
                        {
                            ip->back->fwd = ip->fwd;
                            ip->fwd->back = ip->back;
                        }
                    }
                    break;
                case op_eor:
                    if (ip->oper1->mode == am_immed)
                    {
                        ip->opcode = op_eori;
                        if (ip->oper1->offset->v.i == 0)
                        {
                            ip->back->fwd = ip->fwd;
                            ip->fwd->back = ip->back;
                        }
                    }
                    break;
                case op_muls:
                    peep_muldiv(ip, op_asl);
                    break;
                case op_label:
                    peep_label(ip);
                    break;
                case op_lea:
                    peep_lea(ip);
                    break;
                case op_bra:
                case op_jmp:
                case op_rts:
                    peep_uctran(ip);
            }
        }
        ip = ip->fwd;
    }
}
