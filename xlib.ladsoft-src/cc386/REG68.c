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
#include "lists.h"
#include "expr.h"
#include "c.h"
#include "gen68.h"

#define FUNCHASH 241
/*
 *      this module handles the allocation and de-allocation of
 *      temporary registers. when a temporary register is allocated
 *      the stack depth is saved in the field deep of the address
 *      mode structure. when validate is called on an address mode
 *      structure the stack is popped until the register is restored
 *      to it's pre-push value.
 * 21-Aug-1999 (SJS) Added ColdFire restrictions for movem.
 * 08-May-2001 (SJS) Many modifications to support register forced mode.
 *	Notes:
 *	1. Remove references to opreg...
 * 07-JUL-2001 (DAL) left the latest version of the module as-is
 */

#define DOUBLEREG 240
extern int cf_freedata, cf_freeaddress, cf_freefloat, maxdata, maxaddress;

AMODE push[] = 
{
    {
        am_adec, 7
    }
}

//-------------------------------------------------------------------------

, pop[] = 
{
    {
        am_ainc, 7
    }
}

//-------------------------------------------------------------------------

, mvma7[] = 
{
    {
        am_ind, 7
    }
}

//-------------------------------------------------------------------------

, mvma7i[] = 
{
    {
        am_areg, 7
    }
};
long stackdepth, framedepth;

char aregs[8], dregs[8], fregs[8];
int pushcount;
unsigned char pushedregs[40];
char readytopop[40];
int lastreg1, lastreg2, lastareg1, lastareg2, lastfreg1, lastfreg2;

void regini(void)
{
    int i;
    for (i = 0; i < 8; i++)
    {
        aregs[0] = dregs[0] = fregs[0] = 0;
    }
    initstack();
}

//-------------------------------------------------------------------------

void set_func_mode(int mode)
{
    if (mode)
        pushedregs[pushcount++] = FUNCHASH;
    else if (pushedregs[pushcount - 1] == FUNCHASH)
        pushcount--;
}

//-------------------------------------------------------------------------

void gen_push(int reg, int rmode, int flag)
/*
 *      this routine generates code to push a register onto the stack
 */
{
    AMODE *ap1;
    ap1 = xalloc(sizeof(AMODE));
    ap1->preg = reg &7;
    ap1->mode = rmode;
    if (rmode == am_freg)
    {
        OCODE *new = xalloc(sizeof(OCODE));
        new->opcode = op_fmove;
        new->oper1 = ap1;
        new->length = 10;
        new->oper2 = push;
        add_peep(new);
        stackdepth += 12;
    }
    else
    {
        OCODE *new = xalloc(sizeof(OCODE));
        new->opcode = op_move;
        new->oper1 = ap1;
        new->length = 4;
        new->oper2 = push;
        add_peep(new);
        stackdepth += 4;
    }
}

//-------------------------------------------------------------------------

void gen_pop(int reg, int rmode, int flag)
/*
 *      generate code to pop the primary register in ap from the
 *      stack.
 */
{
    AMODE *ap1;
    ap1 = xalloc(sizeof(AMODE));
    ap1->preg = reg &7;
    ap1->mode = rmode;
    if (rmode == am_freg)
    {
        OCODE *new = xalloc(sizeof(OCODE));
        new->opcode = op_fmove;
        new->oper1 = pop;
        new->length = 10;
        new->oper2 = ap1;
        add_peep(new);
        stackdepth -= 12;
    }
    else
    {
        OCODE *new = xalloc(sizeof(OCODE));
        new->opcode = op_move;
        new->oper1 = pop;
        new->length = 4;
        new->oper2 = ap1;
        add_peep(new);
        stackdepth -= 4;
    }
}

//-------------------------------------------------------------------------

void initstack(void)
/*
 *      this routine should be called before each expression is
 *      evaluated to make sure the stack is balanced and all of
 *      the registers are marked free.
 */
{
    int i;
    if (pushcount)
    {
        if (pushcount > 2)
        {
            AMODE *ap1 = xalloc(sizeof(AMODE));
            ap1->mode = am_immed;
            ap1->offset = makenode(en_icon, (char*)(pushcount *4), 0);
            ap1->preg = 7;
            gen_codes(op_add, 0, ap1, makeareg(7));
        }
        else
            gen_codes(op_add, 4, make_immed(pushcount *4), makeareg(7));
    }
    pushcount = 0;
    for (i = 0; i < 8; i++)
        aregs[i] = dregs[i] = fregs[i] = 0;
}

//-------------------------------------------------------------------------

int next_dreg(void)
{
    int reg;
    if (!dregs[0])
        return dregs[0] *3;
    else
        if (!dregs[1])
            return dregs[1] *3+1;
        else
            if (!dregs[2])
                return dregs[2] *3+2;
            else
                if (lastreg1 == 0)
                    if (lastreg2 == 1)
                        return dregs[2] *3+2;
                    else
                        return dregs[1] *3+1;
                    else if (lastreg1 == 1)
                        if (lastreg2 == 0)
                            return dregs[2] *3+2;
                        else
                            return dregs[0] *3;
                        else if (lastreg1 == 2)
                            if (lastreg2 == 0)
                                return dregs[1] *3+1;
                            else
                                return dregs[0] *3;
}

//-------------------------------------------------------------------------

int next_areg(void)
{
    int reg;
    if (!aregs[0])
        return aregs[0] *3;
    else
        if (!aregs[1])
            return aregs[1] *3+1;
        else
            if (lastareg1 == 0)
                return aregs[1] *3+1;
            else
                return aregs[0] *3;
}

//-------------------------------------------------------------------------

int next_freg(void)
{
    int reg;
    if (!fregs[0])
        return fregs[0] *3;
    else
        if (!fregs[1])
            return fregs[1] *3+1;
        else
            if (!fregs[2])
                return fregs[2] *3+2;
            else
                if (lastfreg1 == 0)
                    if (lastfreg2 == 1)
                        return fregs[2] *3+2;
                    else
                        return fregs[1] *3+1;
                    else if (lastfreg1 == 1)
                        if (lastfreg2 == 0)
                            return fregs[2] *3+2;
                        else
                            return fregs[0] *3;
                        else if (lastfreg1 == 2)
                            if (lastfreg2 == 0)
                                return fregs[1] *3+1;
                            else
                                return fregs[0] *3;
}

//-------------------------------------------------------------------------

AMODE *temp_data(void)
/*
 *      allocate a temporary data register and return it's
 *      addressing mode.
 */
{
    AMODE *ap = xalloc(sizeof(AMODE));
    int reg = next_dreg();
    int rp3 = reg % 3;
    if (dregs[rp3]++)
    {
        gen_push(rp3, am_dreg, 0);
        pushedregs[pushcount] = rp3;
        readytopop[pushcount++] = 0;
    }
    lastreg2 = lastreg1;
    lastreg1 = rp3;
    ap->mode = am_dreg;
    ap->preg = rp3;
    ap->tempflag = TRUE;
    ap->length = 4;
    return ap;
}

//-------------------------------------------------------------------------

AMODE *temp_doubledregs(void)
{
    if (dregs[1] || dregs[0])
    {
        gen_push(1, am_dreg, 0);
        gen_push(0, am_dreg, 0);
        pushedregs[pushcount] = DOUBLEREG;
        readytopop[pushcount++] = 0;
    }
    dregs[1]++;
    dregs[0]++;
}

//-------------------------------------------------------------------------

AMODE *temp_addr(void)
/*
 *      allocate a temporary address register and return it's
 *      addressing mode.
 */
{
    AMODE *ap = xalloc(sizeof(AMODE));
    int reg = next_areg();
    int rp3 = reg % 3+8;
    if (aregs[rp3 &7]++)
    {
        gen_push(rp3, am_areg, 0);
        pushedregs[pushcount] = rp3;
        readytopop[pushcount++] = 0;
    }
    lastareg2 = lastareg1;
    lastareg1 = rp3 &7;
    ap->mode = am_areg;
    ap->preg = rp3 &7;
    ap->tempflag = TRUE;
    ap->length = 4;
    return ap;
}

//-------------------------------------------------------------------------

AMODE *temp_float(void)
/*
 *      allocate a temporary address register and return it's
 *      addressing mode.
 */
{
    AMODE *ap = xalloc(sizeof(AMODE));
    int reg = next_freg();
    int rp3 = reg % 3+16;
    if (fregs[rp3 &7]++)
    {
        gen_push(rp3, am_freg, 0);
        pushedregs[pushcount] = rp3;
        readytopop[pushcount++] = 0;
    }
    lastfreg2 = lastfreg1;
    lastfreg1 = rp3 &7;
    ap->mode = am_freg;
    ap->preg = rp3 &7;
    ap->tempflag = TRUE;
    ap->length = 10;
    return ap;
}

//-------------------------------------------------------------------------

void popa(int reg)
{
    if (reg < 8)
        gen_pop(reg, am_dreg, 0);
    else if (reg < 16)
        gen_pop(reg, am_areg, 0);
    else
        gen_pop(reg, am_freg, 0);

}

//-------------------------------------------------------------------------

void freedata(int dreg)
{
    if (dreg < cf_freedata && dreg >= 0)
    {
        if (dregs[dreg])
        {
            int i;
            for (i = pushcount - 1; i >= 0; i--)
                if (pushedregs[i] == dreg)
                    break;
            if (i >= 0)
            {
                readytopop[i] = 1;
                while (readytopop[pushcount - 1])
                {
                    pushcount--;
                    popa(pushedregs[pushcount]);
                }
            }
            dregs[dreg]--;

        }
    }
}

//-------------------------------------------------------------------------

void freedoubleregs(void)
{
    if (pushedregs[pushcount - 1] == DOUBLEREG)
    {
        dregs[0]--;
        dregs[1]--;
        pushcount--;
        gen_codes(op_move, 4, pop, makedreg(0));
        gen_codes(op_move, 4, pop, makedreg(1));
    }
    else
    {
        freedata(0);
        freedata(1);
    }
    while (pushcount && readytopop[pushcount - 1])
    {
        pushcount--;
        popa(pushedregs[pushcount]);
    }
}

//-------------------------------------------------------------------------

void freeaddr(int areg)
{
    if (areg < cf_freedata && areg >= 0)
    {
        if (aregs[areg])
        {
            int i;
            for (i = pushcount - 1; i >= 0; i--)
                if (pushedregs[i] == areg + 8)
                    break;
            if (i >= 0)
            {
                readytopop[i] = 1;
                while (readytopop[pushcount - 1])
                {
                    pushcount--;
                    popa(pushedregs[pushcount]);
                }
            }
            aregs[areg]--;

        }
    }
}

//-------------------------------------------------------------------------

void freefloat(int freg)
{
    if (freg < cf_freedata && freg >= 0)
    {
        if (fregs[freg])
        {
            int i;
            for (i = pushcount - 1; i >= 0; i--)
                if (pushedregs[i] == freg + 16)
                    break;
            if (i >= 0)
            {
                readytopop[i] = 1;
                while (readytopop[pushcount - 1])
                {
                    pushcount--;
                    popa(pushedregs[pushcount]);
                }
            }
            fregs[freg]--;

        }
    }
}

//-------------------------------------------------------------------------

void freeop(AMODE *ap)
/*
 *      release any temporary registers used in an addressing mode.
 */
{
    if (ap->mode == am_immed || ap->mode == am_direct)
        return ;
     /* no registers used */
    if (ap->mode == am_dreg)
        freedata(ap->preg);
    else if (ap->mode == am_doublereg)
        freedoubleregs();
    else if (ap->mode == am_freg)
        freefloat(ap->preg);
    else if (ap->mode == am_areg || ap->mode == am_ind || ap->mode == am_indx 
        || ap->mode == am_adec || ap->mode == am_ainc)
        freeaddr(ap->preg);
    else if (ap->mode == am_baseindxdata)
    {
        freeaddr(ap->preg);
        freedata(ap->sreg);
    }
    else if (ap->mode == am_baseindxaddr)
    {
        freeaddr(ap->preg);
        freeaddr(ap->sreg);
    }
}
