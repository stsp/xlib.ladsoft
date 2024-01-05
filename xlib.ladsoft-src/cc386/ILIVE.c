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
 * ilive.c
 *
 * live variable analysis
 *
 */
#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <limits.h>
#include "utype.h"	
#include "cmdline.h"	
#include "lists.h"
#include "expr.h"
#include "c.h"
#include "iexpr.h"
#include "iopt.h"
#include "diag.h"

extern int global_flag;
extern int blocknum;
extern int *dfst;
extern BLOCKLIST **blockarray;
extern int pointer_count, source_count;
extern IMODE **pointerarray;
extern QUAD **sourcearray;
extern BYTE *temparray4;
extern int sourcebytes;
extern unsigned char bittab[8];

static int variable_count, variable_bytes;
static int max_variables;
static IMODE **usearray;
static BYTE *temparray3;

#ifdef DUMP_GCSE_INFO
    extern FILE *icdFile,  *outputFile;
    static void dumpvarset(BYTE *set, char *title)
    {
        int i;
        fprintf(icdFile, "\n;%s\n", title);
        for (i = 0; i < variable_count; i++)
        if (isset(set, i))
        {
            fprintf(icdFile, "; ");
            putamode(usearray[i]);
            fprintf(icdFile, "\n");
        }

    }
    static void dump_use(void)
    {
        int i;
        char buf[256];
        FILE *temp;
        if (!icdFile)
            return ;
        temp = outputFile;
        outputFile = icdFile;

        fprintf(icdFile, "\n; Used Variables: %d\n", variable_count);
        for (i = 0; i < variable_count; i++)
        {
            fprintf(icdFile, "; ");
            putamode(usearray[i]);
            fprintf(icdFile, "\n");
        }
        fprintf(icdFile, "\n; Sets:\n");
        for (i = 0; i < blocknum; i++)
        {
            sprintf(buf, "; def block %d", i + 1);
            dumpvarset(blockarray[i]->block->p_def, buf);
            sprintf(buf, "; use block %d", i + 1);
            dumpvarset(blockarray[i]->block->p_use, buf);
            sprintf(buf, "; live in block %d", i + 1);
            dumpvarset(blockarray[i]->block->p_live_in, buf);
            sprintf(buf, "; live out block %d", i + 1);
            dumpvarset(blockarray[i]->block->p_live_out, buf);
        }
        outputFile = temp;
    }
#endif 
static void enter_use(IMODE *var, int ans)
{
    if (var)
    {
        if (var->mode == i_ind || var->mode == i_direct && !ans)
        {
            int i;
            SYM *sp;
            if (var->offset->nodetype == en_labcon)
                return ;
            sp = varsp(var->offset);
            if (sp && sp->imvalue)
                var = sp->imvalue;
            if (var->useindx)
                return ;
            if (variable_count >= max_variables)
            {
                IMODE **newarray = oalloc(sizeof(IMODE*)*(max_variables += 256))
                    ;
                if (usearray)
                    memcpy(newarray, usearray, sizeof(QUAD*)*(max_variables -
                        256));
                usearray = newarray;
            }
            if (ans && var->mode == i_ind)
            {
                IMODE *oldvar = var;
                global_flag++;
                var = xalloc(sizeof(IMODE));
                global_flag--;
                *var =  *oldvar;
                var->mode = i_direct;
                if (sp)
                    sp->imvalue = var;
            }
            usearray[variable_count++] = var;
            var->useindx = variable_count;
        }
    }
}

//-------------------------------------------------------------------------

void RundownUse(void)
{
    int i;
    for (i = 0; i < variable_count; i++)
        usearray[i]->useindx = 0;
}

//-------------------------------------------------------------------------

static void CountUses(void)
{
    int block = 0, i, j, k;

    variable_count = 0;

    usearray = 0;

    max_variables = 0;

    for (i = 0; i < blocknum; i++)
    {
        BLOCK *l = blockarray[i]->block;
        QUAD *head = l->head;
        memcpy(temparray4, l->p_pin, sourcebytes *pointer_count);
        while (head && head->back != l->tail)
        {
            switch (head->dc.opcode)
            {
                case i_add:
                case i_sub:
                case i_udiv:
                case i_umod:
                case i_sdiv:
                case i_smod:
                case i_umul:
                case i_smul:
                case i_lsl:
                case i_lsr:
                case i_asl:
                case i_asr:
                case i_neg:
                case i_not:
                case i_and:
                case i_or:
                case i_eor:
                case i_setne:
                case i_sete:
                case i_setc:
                case i_seta:
                case i_setnc:
                case i_setbe:
                case i_setl:
                case i_setg:
                case i_setle:
                case i_setge:
                case i_assn:
                case i_coswitch:
                case i_jc:
                case i_ja:
                case i_je:
                case i_jnc:
                case i_jbe:
                case i_jne:
                case i_jl:
                case i_jg:
                case i_jle:
                case i_jge:
                case i_parm:
                    enter_use(head->dc.left, FALSE);
                    enter_use(head->dc.right, FALSE);
                    enter_use(head->ans, FALSE);
                    break;
                case i_gosub:
                    l->callcount++;
                    break;

            }
            ptrindex(head, l, temparray4);
            head = head->fwd;
        }
    }
}

//-------------------------------------------------------------------------

static void CreateUseBitArrays(void)
{
    int i;
    variable_bytes = (variable_count + 8) / 8;
    ;
    if (variable_bytes % 4)
        variable_bytes += 4-variable_bytes % 4;
    for (i = 0; i < blocknum; i++)
    {
        blockarray[i]->block->p_use = oalloc(variable_bytes);
        blockarray[i]->block->p_def = oalloc(variable_bytes);
        blockarray[i]->block->p_live_in = oalloc(variable_bytes);
        blockarray[i]->block->p_live_out = oalloc(variable_bytes);
    }
    temparray3 = oalloc(variable_bytes);
}

//-------------------------------------------------------------------------

static void use(IMODE *var, int *match, int *opposite)
{
    IMODE *xvar = 0;
    int i;
    SYM *sp;
    *match =  *opposite =  - 1;
    if (!var)
        return  - 1;
    if (var->mode != i_ind && var->mode != i_direct)
        return  - 1;
    if (var->offset->nodetype == en_labcon)
        return  - 1;
    sp = varsp(var->offset);
    if (var->mode == i_ind)
        xvar = sp->imvalue;
    else
        xvar = sp->imind;
    if (!xvar)
    {
        global_flag++;
        xvar = xalloc(sizeof(IMODE));
        global_flag--;
        *xvar =  *var;
        if (var->mode == i_ind)
        {
            xvar->mode = i_direct;
            if (sp)
                sp->imvalue = xvar;
        }
        else
        {
            xvar->mode = i_ind;
            if (sp)
                sp->imind = xvar;
        }
    }
    *match = var->useindx - 1;
    *opposite = xvar->useindx - 1;
    return ;
}

//-------------------------------------------------------------------------

static int use2(QUAD *var, int ans)
{
    int i;
    SYM *sp;
    IMODE *xvar;
    if (!((sp = varsp(var->dc.left->offset)) && var->dc.left->mode == i_immed))
    if (!((sp = varsp(var->dc.right->offset)) && var->dc.right->mode == i_immed)
        )
    {
        DIAG("use2 could not locate definition");
        return 0;
    }
    xvar = sp->imvalue;
    if (xvar)
        return xvar->useindx - 1;
    return  - 1;
}

//-------------------------------------------------------------------------

static void CalculateUseBasics(void)
{
    int i, j, k, left, right, ans;
    int ptr, ptr2, gencount, pgen, pagen, def2;
    int left1, right1, ans1;
    LIST *l1;
    for (i = 0; i < blocknum; i++)
    {
        BLOCK *l = blockarray[i]->block;
        QUAD *list = l->head;
        memset(temparray3, 0, variable_bytes);
        memcpy(temparray4, l->p_pin, sourcebytes *pointer_count);
        while (list && list->back != l->tail)
        {
            ptr = INT_MAX;
            gencount = 0;
            pgen =  - 1;
            switch (list->dc.opcode)
            {
                case i_add:
                case i_sub:
                case i_udiv:
                case i_umod:
                case i_sdiv:
                case i_smod:
                case i_umul:
                case i_smul:
                case i_lsl:
                case i_lsr:
                case i_asl:
                case i_asr:
                case i_neg:
                case i_not:
                case i_and:
                case i_or:
                case i_eor:
                case i_setne:
                case i_sete:
                case i_setc:
                case i_seta:
                case i_setnc:
                case i_setbe:
                case i_setl:
                case i_setg:
                case i_setle:
                case i_setge:
                case i_assn:
                case i_coswitch:
                case i_jc:
                case i_ja:
                case i_je:
                case i_jnc:
                case i_jbe:
                case i_jne:
                case i_jl:
                case i_jg:
                case i_jle:
                case i_jge:
                case i_parm:
                    use(list->dc.left, &left, &left1);
                    use(list->dc.right, &right, &right1);
                    use(list->ans, &ans, &ans1);
                    if (left !=  - 1)
                    {
                        if (!isset(temparray3, left))
                            if (list->dc.left->mode == i_direct || left1 ==  -
                                1 || !isset(temparray3, left1))
                                setbit(l->p_use, left);
                        if (list->dc.left->mode == i_ind)
                        {
                            BYTE *t4;
                            ptr2 = findPointer(list->dc.left);
                            if (ptr2 < pointer_count)
                            {
                                t4 = temparray4 + ptr2 * sourcebytes;
                                if (isset(t4, 0))
                                {
                                    for (j = 0; j < sourcebytes; j++)
                                    {
                                        l->p_use[j] |= ~temparray3[j];
                                    }
                                }
                                else
                                for (j = 1; j < source_count; j++)
                                {
                                    if (isset(t4, j))
                                    {
                                        left = use2(sourcearray[j], FALSE);
                                        if (left !=  - 1 && !isset(temparray3,
                                            left))
                                            setbit(l->p_use, left);
                                    }
                                }
                            }
                        }
                    }
                    if (right !=  - 1)
                    {
                        if (!isset(temparray3, right))
                            if (list->dc.right->mode == i_direct || right1 ==  
                                - 1 || !isset(temparray3, right1))
                                setbit(l->p_use, right);
                        if (list->dc.right->mode == i_ind)
                        {
                            BYTE *t4;
                            ptr2 = findPointer(list->dc.right);
                            if (ptr2 < pointer_count)
                            {
                                t4 = temparray4 + ptr2 * sourcebytes;
                                if (isset(t4, 0))
                                {
                                    for (j = 0; j < sourcebytes; j++)
                                    {
                                        l->p_use[j] |= ~temparray3[j];
                                    }
                                }
                                else
                                for (j = 1; j < source_count; j++)
                                {
                                    if (isset(t4, j))
                                    {
                                        right = use2(sourcearray[j], FALSE);
                                        if (right !=  - 1 && !isset(temparray3,
                                            right))
                                            setbit(l->p_use, right);
                                    }
                                }
                            }
                        }
                    }
                    if (ans !=  - 1 && list->ans->mode == i_ind && !isset
                        (temparray3, ans))
                        if (ans1 ==  - 1 || !isset(temparray3, ans1))
                            setbit(l->p_use, ans);
                    if (ans !=  - 1)
                        setbit(temparray3, ans);

                    if (list->ans && list->ans->mode == i_ind)
                    {
                        if (ptr < pointer_count)
                        {
                            BYTE *t4 = temparray4 + ptr * sourcebytes;
                            if (!isset(t4, 0))
                            {
                                gencount = 0;
                                pgen =  - 1;
                                for (j = 1; j < source_count; j++)
                                {
                                    if (isset(t4, j))
                                    {
                                        gencount++;
                                        pgen = j;
                                        if (gencount > 1)
                                            break;
                                    }
                                }
                                if (gencount == 1)
                                {
                                    ans = use2(sourcearray[pgen], FALSE);
                                    setbit(temparray3, ans);
                                }
                            }
                        }
                    }
                    break;
            }
            ptrindex(list, l, temparray4);
            list = list->fwd;
        }
        for (j = 0; j < variable_bytes; j += 4)
            *(unsigned*)(l->p_def + j) = *(unsigned*)(temparray3 + j) &~*
                (unsigned*)(l->p_use + j);
    }
}

//-------------------------------------------------------------------------

static void CalculateLive(void)
{
    int i, changed, j;
    for (i = 0; i < blocknum; i++)
        memcpy(blockarray[i]->block->p_live_in, blockarray[i]->block->p_use,
            variable_bytes);
    do
    {
        changed = FALSE;
        for (i = 0; i < blocknum; i++)
        {
            BLOCK *t = blockarray[dfst[blocknum - i - 1]]->block;
            BLOCKLIST *l = t->flowfwd;
            memset(t->p_live_out, 0, variable_bytes);
            while (l)
            {
                for (j = 0; j < variable_bytes; j++)t->p_live_out[j] |= l
                    ->block->p_live_in[j];
                l = l->link;
            }
            for (j = 0; j < variable_bytes; j++)
            {
                int v = t->p_live_in[j];
                t->p_live_in[j] = t->p_use[j] | (t->p_live_out[j] &~t->p_def[j])
                    ;
                changed |= (v != t->p_live_in[j]);
            }
        }
    }
    while (changed)
        ;

}

//-------------------------------------------------------------------------

void ScanForDead(void)
{
    int i;
    for (i = blocknum - 1; i >= 0 ; i--)
    {
        BLOCK *l = blockarray[dfst[i]]->block;
        QUAD *list = l->tail;
        memcpy(temparray3, l->p_live_out, variable_bytes);
        while (list && list->fwd != l->head)
        {

            switch (list->dc.opcode)
            {
                case i_add:
                case i_sub:
                case i_udiv:
                case i_umod:
                case i_sdiv:
                case i_smod:
                case i_umul:
                case i_smul:
                case i_lsl:
                case i_lsr:
                case i_asl:
                case i_asr:
                case i_neg:
                case i_not:
                case i_and:
                case i_or:
                case i_eor:
                case i_setne:
                case i_sete:
                case i_setc:
                case i_seta:
                case i_setnc:
                case i_setbe:
                case i_setl:
                case i_setg:
                case i_setle:
                case i_setge:
                case i_assn:
                    if (!isset(temparray3, list->ans->useindx) && list->ans
                        ->mode == i_direct)
                    {
                        list->back->fwd = list->fwd;
                        list->fwd->back = list->back;
                        break;
                    }
                case i_coswitch:
                case i_jc:
                case i_ja:
                case i_je:
                case i_jnc:
                case i_jbe:
                case i_jne:
                case i_jl:
                case i_jg:
                case i_jle:
                case i_jge:
                case i_parm:
					break ;
            }
            list = list->back;
        }
    }

}

//-------------------------------------------------------------------------

void LiveAnalysis(void)
{
    CountUses();
    CreateUseBitArrays();
    CalculateUseBasics();
    CalculateLive();
    ScanForDead();
    #ifdef DUMP_GCSE_INFO
        dump_use();
    #endif 
}
