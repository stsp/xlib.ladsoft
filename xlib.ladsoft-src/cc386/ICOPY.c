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
 * icopy.c
 *
 * iteratively determine candidates for copy propagation, and
 * then go through the blocks and replace as many copy statements
 * as possible
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

#define CHASH DAGSIZE

extern int blocknum;
extern int *dfst;
extern BLOCKLIST **blockarray;
extern BYTE *temparray4;
extern int sourcebytes, pointer_count, source_count;
extern QUAD **defiarray;
extern unsigned char bittab[8];
extern QUAD **sourcearray;
extern int prm_debug;

static LIST3 **copyhash;
static QUAD **copyarray;
static int max_copies;
static int copy_count, copybytes;
static BYTE *temparray5;

#ifdef DUMP_GCSE_INFO
    extern FILE *icdFile,  *outputFile;
    static void dump_copies(void)
    {
        int i;
        char buf[256];
        FILE *temp;
        if (!icdFile)
            return ;
        temp = outputFile;
        outputFile = icdFile;
        fprintf(icdFile, "\n; Copies: %d\n", copy_count);
        for (i = 0; i < copy_count; i++)
        {
            fprintf(icdFile, "; ");
            put_code(copyarray[i]);
        }
        fprintf(icdFile, "\n; Sets:\n");
        for (i = 0; i < blocknum; i++)
        {
            sprintf(buf, "; Gen block %d", i + 1);
            dumpset(blockarray[i]->block->p_cgen, copyarray, buf, copy_count);
            sprintf(buf, "; kill block %d", i + 1);
            dumpset(blockarray[i]->block->p_ckill, copyarray, buf, copy_count);
            sprintf(buf, "; in block %d", i + 1);
            dumpset(blockarray[i]->block->p_cin, copyarray, buf, copy_count);
            sprintf(buf, "; out block %d", i + 1);
            dumpset(blockarray[i]->block->p_cout, copyarray, buf, copy_count);
        }
        outputFile = temp;
    }
#endif 
static int used(QUAD *match, QUAD *list)
{
    SYM *sp;
    if (list->dc.left && list->dc.left->mode != i_immed)
    {
        sp = varsp(list->dc.left->offset);
        if (sp)
            if (sp->imvalue == match->dc.left || sp->imvalue == match->ans)
                return TRUE;
    }
    if (list->dc.right && list->dc.right->mode != i_immed)
    {
        sp = varsp(list->dc.right->offset);
        if (sp)
            if (sp->imvalue == match->dc.left || sp->imvalue == match->ans)
                return TRUE;
    }
    if (list->ans && list->ans->mode == i_ind)
    {
        sp = varsp(list->ans->offset);
        if (sp)
            if (sp->imvalue == match->dc.left || sp->imvalue == match->ans)
                return TRUE;
    }
    return FALSE;
}

//-------------------------------------------------------------------------

static void EnterCopy(QUAD *cp)
{
    if (cp->ans->mode != i_direct || cp->dc.left->mode != i_direct)
        return ;
    if (cp->ans->size != cp->dc.left->size)
        return ;
    if (copy_count >= max_copies)
    {
        QUAD **newarray = oalloc(sizeof(QUAD*)*(max_copies += 256));
        if (copy_count)
            memcpy(copyarray, newarray, sizeof(QUAD*) * copy_count);
        copyarray = newarray;
    }
    cp->copy = copy_count;
    anshash(copyhash, cp->dc.left, (QUAD*)copy_count);
    anshash(copyhash, cp->ans, (QUAD*)copy_count);
    copyarray[copy_count++] = cp;
}

//-------------------------------------------------------------------------

static void CountCopies(void)
{
    QUAD *list = blockarray[0]->block->head;
    max_copies = copy_count = 0;
    while (list)
    {
        if (list->dc.opcode == i_assn)
        {
            EnterCopy(list);
        }
        list = list->fwd;
    }
}

//-------------------------------------------------------------------------

static void CreateBitmaps(void)
{
    int i;
    copybytes = (copy_count + 8) / 8;
    if (copybytes % 4)
        copybytes += 4-copybytes % 4;
    for (i = 0; i < blocknum; i++)
    {
        blockarray[i]->block->p_cgen = oalloc(copybytes);
        blockarray[i]->block->p_ckill = oalloc(copybytes);
        blockarray[i]->block->p_cin = oalloc(copybytes);
        blockarray[i]->block->p_cout = oalloc(copybytes);
    }
    temparray5 = oalloc(copybytes);
}

//-------------------------------------------------------------------------

static void CalculateCopyBasics(void)
{
    int i;
    QUAD *list;
    BLOCK *l;
    for (i = 0; i < blocknum; i++)
    {
        l = blockarray[i]->block;
        list = l->head;
        memcpy(temparray4, l->p_pin, sourcebytes *pointer_count);
        while (list && list->back != l->tail)
        {
            switch (list->dc.opcode)
            {
                case i_assn:
                    if (list->ans->mode == i_direct && list->dc.left->mode ==
                        i_direct && list->ans->size == list->dc.left->size)
                    {
                        setbit(l->p_cgen, list->copy);
                        clearbit(l->p_ckill, list->copy);
                    }
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
                    if (list->ans->mode == i_ind)
                    {
                        int ptr = findPointer(list->ans);
                        if (ptr < pointer_count)
                        {
                            if (isset(temparray4 + ptr * sourcebytes, 0))
                            {
                                memset(l->p_cgen, 0, copybytes);
                                memset(l->p_ckill, 0xff, copybytes);
                            }
                            else
                            for (i = 1; i < source_count; i++)
                            {
                                if (isset(temparray4 + ptr * sourcebytes, i))
                                {
                                    int def2 = FindDefinition(i, list);
                                    if (def2 ==  - 1)
                                        DIAG(
                                            "Calculate Copy Basics - could not find definition");
                                    else
                                    {
                                        SYM *sp = varsp(defiarray[def2]
                                            ->dc.left->offset);
                                        LIST *l1;
                                        if (sp)
                                        {
                                            l1 = findans(copyhash, sp->imvalue);
                                            while (l1)
                                            {
                                                clearbit(l->p_cgen, (int)l1
                                                    ->data);
                                                setbit(l->p_ckill, (int)l1
                                                    ->data);
                                                l1 = l1->link;
                                            }
                                        }
                                        if (defiarray[def2]->dc.right)
                                        {
                                            sp = varsp(defiarray[def2]
                                                ->dc.right->offset);
                                            if (sp)
                                            {
                                                l1 = findans(copyhash, sp
                                                    ->imvalue);
                                                while (l1)
                                                {
                                                    clearbit(l->p_cgen, (int)l1
                                                        ->data);
                                                    setbit(l->p_ckill, (int)l1
                                                        ->data);
                                                    l1 = l1->link;
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                    else if (list->ans->mode == i_direct)
                    {
                        LIST *l1 = findans(copyhash, list->ans);
                        while (l1)
                        {
                            if (list->dc.opcode != i_assn || list->copy != (int)
                                l1->data)
                            {
                                clearbit(l->p_cgen, (int)l1->data);
                                setbit(l->p_ckill, (int)l1->data);
                            }
                            l1 = l1->link;
                        }
                    }
                    break;
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
                    break;
                case i_parm:
                    if (list->dc.left->mode == i_direct)
                    {
                        SYM *sp = varsp(list->dc.left->offset);
                        if (sp)
                        {
                            int ptr = findPointer(list->dc.left);
                            if (ptr < pointer_count)
                            {
                                if (isset(temparray4, 0))
                                {
                                    memset(l->p_cgen, 0, copybytes);
                                    memset(l->p_ckill, 0xff, copybytes);
                                }
                                else
                                {
                                    for (i = 1; i < source_count; i++)
                                    {
                                        if (isset(temparray4 + ptr *
                                            sourcebytes, i))
                                        {
                                            int def2 = FindDefinition(i, list);
                                            if (def2 ==  - 1)
                                                DIAG(
                                                    "Calculate Copy Basics - could not find definition");
                                            else
                                            {
                                                SYM *sp = varsp(defiarray[def2]
                                                    ->dc.left->offset);
                                                LIST *l1;
                                                if (sp)
                                                {
                                                    l1 = findans(copyhash, sp
                                                        ->imvalue);
                                                    while (l1)
                                                    {
                                                        clearbit(l->p_cgen, 
                                                            (int)l1->data);
                                                        setbit(l->p_ckill, (int)
                                                            l1->data);
                                                        l1 = l1->link;
                                                    }
                                                }
                                                if (defiarray[def2]->dc.right)
                                                {
                                                    sp = varsp(defiarray[def2]
                                                        ->dc.right->offset);
                                                    if (sp)
                                                    {
                                                        l1 = findans(copyhash,
                                                            sp->imvalue);
                                                        while (l1)
                                                        {
                                                            clearbit(l->p_cgen,
                                                                (int)l1->data);
                                                            setbit(l->p_ckill, 
                                                                (int)l1->data);
                                                            l1 = l1->link;
                                                        }
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                    break;
                case i_gosub:
                    for (i = 0; i < copy_count; i++)
                    {
                        IMODE *s = copyarray[i]->ans;
                        switch (s->offset->nodetype)
                        {
                        case en_nacon:
                        case en_napccon:
                        case en_nalabcon:
                        case en_labcon:
                        case en_absacon:
                            if (s->mode == i_direct)
                            {
                                clearbit(l->p_cgen, i);
                                setbit(l->p_ckill, i);
                            }
                            else if (s->mode == i_ind){}
                            break;
                        }
                        s = copyarray[i]->dc.left;
                        switch (s->offset->nodetype)
                        {
                        case en_nacon:
                        case en_napccon:
                        case en_nalabcon:
                        case en_labcon:
                        case en_absacon:
                            if (s->mode == i_direct)
                            {
                                clearbit(l->p_cgen, i);
                                setbit(l->p_ckill, i);
                            }
                            else if (s->mode == i_ind){}
                            break;
                        }
                    }
                    break;
            }
            ptrindex(list, l, temparray4);
            list = list->fwd;
        }
    }
}

//-------------------------------------------------------------------------

static void CalculateCopyEqns(void)
{
    int i, changed, j;
    for (i = 0; i < blocknum; i++)
        memcpy(blockarray[i]->block->p_cout, blockarray[i]->block->p_cgen,
            copybytes);
    do
    {
        changed = FALSE;
        for (i = 0; i < blocknum; i++)
        {
            BLOCK *t = blockarray[dfst[i]]->block;
            BLOCKLIST *l = t->flowback;
            if (l)
                memset(t->p_cin, 0xff, copybytes);
            while (l)
            {
                for (j = 0; j < copybytes; j++)t->p_cin[j] &= l->block
                    ->p_cout[j];
                l = l->link;
            }
            for (j = 0; j < copybytes; j++)
            {
                int v = t->p_cout[j];
                t->p_cout[j] = t->p_cgen[j] | (t->p_cin[j] &~t->p_ckill[j]);
                changed |= (v != t->p_cout[j]);
            }
        }
    }
    while (changed)
        ;

}

//-------------------------------------------------------------------------

static int ScanForCopy(BLOCK *l, int index)
{
    QUAD *list = l->head;
    QUAD *match = copyarray[index];
    BLOCKLIST *s;
    IMODE *im,  *ima;
    QUAD *t;
    LIST *l1;
    int i, ptr, j;
    int stopped = FALSE;
    memcpy(temparray4, l->p_cin, sourcebytes *pointer_count);
    while (list && list->back != l->tail)
    {
        switch (list->dc.opcode)
        {
            case i_assn:
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
                if (list->ans == match->dc.left || list->ans == match->ans)
                {
                    stopped = TRUE;
                    break;
                }
                if (list->ans->mode == i_ind)
                {
                    int ptr = findPointer(list->ans);
                    if (ptr < pointer_count)
                    {
                        if (isset(temparray4 + ptr * sourcebytes, 0))
                        {
                            stopped = TRUE;
                            break;
                        }
                        else
                        for (i = 1; i < source_count; i++)
                        {
                            if (isset(temparray4 + ptr * sourcebytes, i))
                            {
                                SYM *sp = varsp(sourcearray[i]->ans->offset);
                                if (sp->imvalue == match->dc.left || sp
                                    ->imvalue == match->dc.right)
                                {
                                    stopped = TRUE;
                                    break;
                                }
                            }
                        }
                    }
                }
                else if (list->ans->mode == i_direct)
                {
                    ptr = findPointer(list->ans);
                    if (ptr < pointer_count)
                    {
                        SYM *sp = varsp(list->ans->offset);
                        if (sp)
                        {
                            l1 = findans(copyhash, sp->imind);
                            if (l1)
                            {
                                stopped = TRUE;
                                break;
                            }
                        }
                    }
                }
                // fall through
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
                if (stopped && used(match, list))
                    return FALSE;
                break;
            case i_parm:
                if (list->dc.left->mode == i_immed)
                {
                    SYM *sp = varsp(list->dc.left->offset);
                    if (sp)
                    {
                        if (sp->imvalue == match->dc.left || sp->imvalue ==
                            match->ans)
                        {
                            stopped = TRUE;
                            break;
                        }
                    }
                }
                else if (list->dc.left->mode == i_direct)
                {
                    SYM *sp = varsp(list->dc.left->offset);
                    if (sp)
                    {
                        int ptr = findPointer(list->dc.left);
                        if (ptr < pointer_count)
                        {
                            if (isset(temparray4, 0))
                            {
                                stopped = TRUE;
                                break;
                            }
                            else
                            {
                                for (i = 1; i < source_count; i++)
                                {
                                    if (isset(temparray4 + ptr * sourcebytes, i)
                                        )
                                    {
                                        SYM *sp = varsp(sourcearray[i]->ans
                                            ->offset);
                                        if (sp->imvalue == match->dc.left || sp
                                            ->imvalue == match->dc.right)
                                        {
                                            stopped = TRUE;
                                            break;
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
                if (stopped && used(match, list))
                    return FALSE;
                break;
            case i_gosub:
                for (i = 0; i < copy_count; i++)
                {
                    if (copyarray[i] != list)
                    {
                        IMODE *s = copyarray[i]->ans;
                        switch (s->offset->nodetype)
                        {
                        case en_nacon:
                        case en_napccon:
                        case en_nalabcon:
                        case en_labcon:
                        case en_absacon:
                            if (s == match->dc.left || s == match->ans)
                            {
                                stopped = TRUE;
                                break;
                            }
                            break;
                        }
                    }
                }
                break;
        }
        ptrindex(list, l, temparray4);
        list = list->fwd;
    }
    return TRUE;
}

//-------------------------------------------------------------------------

IMODE *replace(IMODE *im, QUAD *match)
{
    SYM *sp;
    if (!im)
        return im;
    if (im->mode != i_ind && im->mode != i_direct)
        return im;
    sp = varsp(im->offset);
    if (!sp)
        return im;
    if (sp->imvalue == match->ans)
    {
        if (sp->imvalue == im)
            return match->dc.left;
        else
        {
            sp = varsp(match->dc.left->offset);
            return sp->imind;
        }
    }
    return im;
}

//-------------------------------------------------------------------------

static void ReplaceCopies(void)
{
    int i, j;
    for (i = blocknum - 1; i >= 0; i--)
    {
        BLOCK *l = blockarray[dfst[i]]->block;
        for (j = 0; j < copy_count; j++)
            if (isset(l->p_cin, j))
                if (!ScanForCopy(l, j))
                    setbit(temparray5, j);
    }
    for (i = 0; i < blocknum; i++)
    {
        BLOCK *l = blockarray[i]->block;
        for (j = 0; j < copy_count; j++)
        {
            if (!isset(temparray5, j) && isset(l->p_cin, j))
            {
                QUAD *list = l->head;
                QUAD *match = copyarray[j];
                while (list && list->back != l->tail)
                {
                    if (list->ans && list->ans->mode == i_ind)
                        list->ans = replace(list->ans, match);
                    list->dc.right = replace(list->dc.right, match);
                    list->dc.left = replace(list->dc.left, match);
                    list = list->fwd;
                }
            }
        }
    }
    for (j = 0; j < copy_count; j++)
    if (!isset(temparray5, j))
    {
        QUAD *match = copyarray[j];
        SYM *sp = varsp(match->ans->offset);
        if (!prm_debug || sp && sp->storage_class == sc_temp)
        {
            //            match->fwd->back = match->back ;
            //            match->back->fwd = match->fwd ;
        }
    }
}

//-------------------------------------------------------------------------

void CopyPropagation(void)
{
    return ;
    copyhash = oalloc(sizeof(LIST3*) * CHASH);
    CountCopies();
    CreateBitmaps();
    CalculateCopyBasics();
    CalculateCopyEqns();
    ReplaceCopies();
    #ifdef DUMP_GCSE_INFO
        dump_copies();
    #endif 
}
