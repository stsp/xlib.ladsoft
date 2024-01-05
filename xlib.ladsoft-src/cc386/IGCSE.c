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
 * igcse.c
 *
 * Go through the block list replacing common subexpressions with copy
 * statements
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

#define AHASH DAGSIZE
extern int *dfst;
extern int blocknum;
extern BLOCKLIST **blockarray;
extern unsigned char bittab[8];
extern BYTE *temparray4;
extern int sourcebytes, pointer_count, source_count;
extern IMODE **pointerarray;
extern QUAD **sourcearray;
extern int tempnum;

static QUAD **availarray;
static int avail_count, avail_bytes, max_avail;
static QUAD **csearray;
static int cse_max, cse_count;
static LIST3 **availhash,  **availhash2;
static int csesize =  - 1000, csexit;
static SYM **tempindexarray;

#ifdef DUMP_GCSE_INFO
    extern FILE *icdFile,  *outputFile;
    static void dump_avail(void)
    {
        int i;
        char buf[256];
        FILE *temp;
        if (!icdFile)
            return ;
        temp = outputFile;
        outputFile = icdFile;
        fprintf(icdFile, "\n; Available Expressions: %d\n", avail_count);
        for (i = 0; i < avail_count; i++)
        {
            fprintf(icdFile, "; ");
            put_code(availarray[i]);
        }
        fprintf(icdFile, "\n; Sets:\n");
        for (i = 0; i < blocknum; i++)
        {
            sprintf(buf, "; Avail Gen block %d", i + 1);
            dumpset(blockarray[i]->block->p_agen, availarray, buf, avail_count);
            sprintf(buf, "; Avail kill block %d", i + 1);
            dumpset(blockarray[i]->block->p_akill, availarray, buf, avail_count)
                ;
            sprintf(buf, "; Avail in block %d", i + 1);
            dumpset(blockarray[i]->block->p_ain, availarray, buf, avail_count);
            sprintf(buf, "; Avail out block %d", i + 1);
            dumpset(blockarray[i]->block->p_aout, availarray, buf, avail_count);
        }
        outputFile = temp;
    }
#endif 
static int CacheAvail(QUAD *var)
{
    LIST *l = oalloc(sizeof(LIST));
    int hash;
    l->data = var;
    hash = dhash(var, sizeof(struct _basic_dag));
    l->link = availhash2[hash];
    availhash2[hash] = l;
}

//-------------------------------------------------------------------------

static QUAD *FindAvail(QUAD *var)
{
    LIST *l;
    int hash = dhash(var, sizeof(struct _basic_dag));
    l = availhash2[hash];
    while (l)
    {
        if (!memcmp(var, l->data, sizeof(struct _basic_dag)))
            return l->data;
        l = l->link;
    } return 0;

}

//-------------------------------------------------------------------------

static void enter_avail(QUAD *var)
{
    QUAD *l;
    if (var->dc.opcode == i_assn && var->dc.left->mode == i_immed && 
        (isfloatconst(var->dc.left->offset->nodetype) || isintconst(var
        ->dc.left->offset->nodetype)))
        return ;
    if (var->available)
        return ;
    l = FindAvail(var);
    if (l)
    {
        SYM *sp;
        var->available = l->available;
        sp = varsp(var->ans->offset);
        if (sp->storage_class == sc_temp)
        {
            int hash = dhash(var, sizeof(struct _basic_dag));
            LIST *l = availhash2[hash];
            while (l)
            {
                if (!memcmp(l->data, var, sizeof(struct _basic_dag)))
                {
                    SYM *sp1 = varsp(((QUAD*)l->data)->ans->offset);
                    if (sp1->storage_class == sc_temp && !sp1->iglobal && !sp1
                        ->ipointerindx)
                    {
                        tempindexarray[sp->value.i] = sp1;
                        return ;
                    } 
                }
                l = l->link;
            }
        }
        CacheAvail(var);
        return ;
    }
    if (avail_count >= max_avail)
    {
        QUAD **newarray = oalloc(sizeof(QUAD*)*(max_avail += 256));
        if (availarray)
            memcpy(newarray, availarray, (max_avail - 256) *sizeof(QUAD*));
        availarray = newarray;
    }
    var->available = avail_count + 1;
    availarray[avail_count++] = var;
    anshash(availhash, var->dc.left, (QUAD*)avail_count);
    anshash(availhash, var->dc.right, (QUAD*)avail_count);
    CacheAvail(var);
}

//-------------------------------------------------------------------------

static void ReplaceOneTemp(IMODE **v)
{
    SYM *sp;
    if (! *v)
        return ;
    sp = varsp((*v)->offset);
    if (sp)
    if (sp->storage_class == sc_temp)
    {
        if (sp = tempindexarray[sp->value.i])
        {
            if ((*v)->mode == i_ind)
            {
                if (sp->imind == 0)
                {
                    sp->imind == xalloc(sizeof(IMODE));
                    sp->imind->mode == i_ind;
                    sp->imind->offset = makenode(en_tempref, sp, 0);
                    sp->imind->size = (*v)->size;
                }
                *v = sp->imind;
            }
            else if ((*v)->mode == i_direct)
            {
                if (sp->imvalue == 0)
                {
                    sp->imvalue == xalloc(sizeof(IMODE));
                    sp->imvalue->mode == i_direct;
                    sp->imvalue->offset = makenode(en_tempref, sp, 0);
                    sp->imvalue->size = (*v)->size;
                }
                *v = sp->imvalue;
            }
        }
    }
}

//-------------------------------------------------------------------------

static void ReplaceTemps(QUAD *list)
{
    ReplaceOneTemp(&list->ans);
    ReplaceOneTemp(&list->dc.left);
    ReplaceOneTemp(&list->dc.right);
}

//-------------------------------------------------------------------------

static void CountAvail(void)
{
    int block = 0, i, j, k;

    avail_count = 0;

    availarray = 0;

    max_avail = 0;

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
                    ReplaceTemps(head);
                    enter_avail(head);

                    // FALL THROUGH
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
                    ReplaceTemps(head);
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

static void CreateAvailBitArrays(void)
{
    int i;
    avail_bytes = (avail_count + 8) / 8;
    if (avail_bytes % 4)
        avail_bytes += 4-avail_bytes % 4;
    for (i = 0; i < blocknum; i++)
    {
        blockarray[i]->block->p_agen = oalloc(avail_bytes);
        blockarray[i]->block->p_akill = oalloc(avail_bytes);
        blockarray[i]->block->p_ain = oalloc(avail_bytes);
        blockarray[i]->block->p_aout = oalloc(avail_bytes);
    }
}

//-------------------------------------------------------------------------

int isvolatile(QUAD *item)
{
    if (item->dc.left && item->dc.left->offset && (item->dc.left->offset
        ->cflags &DF_VOL))
        return 1;
    if (item->dc.right && item->dc.right->offset && (item->dc.right->offset
        ->cflags &DF_VOL))
        return 1;
    return 0;
}

//-------------------------------------------------------------------------

static void CalculateAvailBasics(void)
{
    int i, j, k, left, right, ans;
    int ptr, ptr2, gencount, pgen, pagen, def2;
    int left1, right1, ans1;
    LIST *l1;
    for (i = 0; i < blocknum; i++)
    {
        BLOCK *l = blockarray[i]->block;
        QUAD *list = l->head;
        memcpy(temparray4, l->p_pin, sourcebytes *pointer_count);
        while (list && list->back != l->tail)
        {
            int skipptr = FALSE;
            ptr = INT_MAX;
            gencount = 0;
            pgen =  - 1;
            switch (list->dc.opcode)
            {
                case i_assn:
                    if (list->ans == list->dc.left)
                    {
                        list->fwd->back = list->back;
                        list->back->fwd = list->fwd;
                        skipptr = TRUE;
                        break;
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
                    // do available for this expression
                    if (list->available)
                    {
                        if (!isvolatile(list))
                            setbit(l->p_agen, list->available - 1);
                        clearbit(l->p_akill, list->available - 1);
                    }
                    // invalidate available for previous uses of variable
                    if (list->ans)
                    {
                        l1 = findans(availhash, list->ans);
                        while (l1)
                        {
                            clearbit(l->p_agen, (int)l1->data - 1);
                            setbit(l->p_akill, (int)l1->data - 1);
                            l1 = l1->link;
                        }
                    }
                    // take pointers into account for avail & reaching
                    if (list->ans->mode == i_ind)
                    {
                        BYTE *t4;
                        ptr = findPointer(list->ans);
                        if (ptr < pointer_count)
                        {
                            t4 = temparray4 + ptr * sourcebytes;
                            if (isset(t4, 0))
                            {
                                memset(l->p_akill, 0xff, avail_bytes);
                                memset(l->p_agen, 0, avail_bytes);
                            }
                            else
                            {
                                for (j = 1; j < source_count; j++)
                                {
                                    if (isset(t4, j))
                                    {
                                        if (sourcearray[j])
                                        {
                                            SYM *sp = varsp(sourcearray[j]
                                                ->dc.left->offset);
                                            if (sp)
                                            {
                                                l1 = findans(availhash, sp
                                                    ->imvalue);
                                                while (l1)
                                                {
                                                    clearbit(l->p_agen, (int)l1
                                                        ->data - 1);
                                                    setbit(l->p_akill, (int)l1
                                                        ->data - 1);
                                                    gencount++;
                                                    l1 = l1->link;
                                                }
                                            }
                                            if (sourcearray[j]->dc.right)
                                            {
                                                sp = varsp(sourcearray[j]
                                                    ->dc.right->offset);
                                                if (sp)
                                                {
                                                    l1 = findans(availhash, sp
                                                        ->imvalue);
                                                    while (l1)
                                                    {
                                                        clearbit(l->p_agen, 
                                                            (int)l1->data - 1);
                                                        setbit(l->p_akill, (int)
                                                            l1->data - 1);
                                                        gencount++;
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
                    else
                    {
                        // if we changed a pointer, invalidate further uses of
                        // the pointer
                        ptr = findPointer(list->ans);
                        if (ptr < pointer_count)
                        {
                            SYM *sp = varsp(list->ans);
                            if (sp)
                            {
                                l1 = findans(availhash, sp->imind);
                                while (l1)
                                {
                                    clearbit(l->p_agen, (int)l1->data - 1);
                                    setbit(l->p_akill, (int)l1->data - 1);
                                    l1 = l1->link;
                                }
                            }
                        }
                    }
                    // FALLTHROUGH
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
                    if (list->dc.opcode == i_parm)
                    {
                        SYM *sp = varsp(list->dc.left);
                        if (sp && list->dc.left->mode == i_immed)
                        {
                            if (list->dc.left->mode == i_immed)
                            {
                                l1 = findans(availhash, sp->imvalue);
                                while (l1)
                                {
                                    clearbit(l->p_agen, (int)l1->data - 1);
                                    setbit(l->p_akill, (int)l1->data - 1);
                                    l1 = l1->link;
                                }
                            }
                        }
                        else if (sp && list->dc.left->mode == i_direct)
                        {
                            ptr = findPointer(list->dc.left);
                            if (ptr < pointer_count)
                            {
                                BYTE *t4 = temparray4 + ptr * sourcebytes;
                                if (isset(t4, 0))
                                {
                                    memset(l->p_akill, 0xff, avail_bytes);
                                    memset(l->p_agen, 0, avail_bytes);
                                }
                                else
                                {
                                    for (j = 1; j < source_count; j++)
                                    {
                                        if (isset(t4, j))
                                        {
                                            SYM *sp = varsp(sourcearray[j]
                                                ->dc.left->offset);
                                            if (sp)
                                            {
                                                l1 = findans(availhash, sp
                                                    ->imvalue);
                                                while (l1)
                                                {
                                                    clearbit(l->p_agen, (int)l1
                                                        ->data - 1);
                                                    setbit(l->p_akill, (int)l1
                                                        ->data - 1);
                                                    gencount++;
                                                    l1 = l1->link;
                                                }
                                            }
                                            if (sourcearray[j]->dc.right)
                                            {
                                                sp = varsp(sourcearray[j]
                                                    ->dc.right->offset);
                                                if (sp)
                                                {
                                                    l1 = findans(availhash, sp
                                                        ->imvalue);
                                                    while (l1)
                                                    {
                                                        clearbit(l->p_agen, 
                                                            (int)l1->data - 1);
                                                        setbit(l->p_akill, (int)
                                                            l1->data - 1);
                                                        gencount++;
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
                    break;
                case i_gosub:
                    // kill globals
                    for (j = 0; j < avail_count; j++)
                    {
                        IMODE *s = availarray[j]->dc.left;
                        if (s && s->offset)
                        {
                            switch (s->offset->nodetype)
                            {
                            case en_nacon:
                            case en_napccon:
                            case en_nalabcon:
                            case en_labcon:
                            case en_absacon:
                                if (s->mode == i_direct)
                                {
                                    l1 = findans(availhash, list->ans);
                                    while (l1)
                                    {
                                        clearbit(l->p_agen, (int)l1->data - 1);
                                        setbit(l->p_akill, (int)l1->data - 1);
                                        l1 = l1->link;
                                    }
                                }
                                else if (s->mode == i_ind)
                                {
                                    BYTE *t4;
                                    ptr = findPointer(list->ans);
                                    if (ptr < pointer_count)
                                    {
                                        t4 = temparray4 + ptr * sourcebytes;
                                        if (isset(t4, 0))
                                        {
                                            memset(l->p_akill, 0xff,
                                                avail_bytes);
                                            memset(l->p_agen, 0, avail_bytes);
                                        }
                                        else
                                        {
                                            for (k = 1; k < source_count; k++)
                                            {
                                                if (isset(t4, k))
                                                {
                                                    SYM *sp = varsp
                                                        (sourcearray[k]
                                                        ->dc.left->offset);
                                                    if (sp)
                                                    {
                                                        l1 = findans(availhash,
                                                            sp->imvalue);
                                                        while (l1)
                                                        {
                                                            clearbit(l->p_agen,
                                                                (int)l1->data -
                                                                1);
                                                            setbit(l->p_akill, 
                                                                (int)l1->data -
                                                                1);
                                                            gencount++;
                                                            l1 = l1->link;
                                                        }
                                                    }
                                                    if (sourcearray[k]
                                                        ->dc.right)
                                                    {
                                                        sp = varsp
                                                            (sourcearray[k]
                                                            ->dc.right->offset);
                                                        if (sp)
                                                        {
                                                            l1 = findans
                                                                (availhash, sp
                                                                ->imvalue);
                                                            while (l1)
                                                            {
                                                                clearbit(l
                                                                    ->p_agen, 
                                                                    (int)l1
                                                                    ->data - 1);
                                                                setbit(l
                                                                    ->p_akill, 
                                                                    (int)l1
                                                                    ->data - 1);
                                                                gencount++;
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
                        s = availarray[j]->dc.right;
                        if (s && s->offset)
                        {
                            switch (s->offset->nodetype)
                            {
                            case en_nacon:
                            case en_napccon:
                            case en_nalabcon:
                            case en_labcon:
                            case en_absacon:
                                if (s->mode == i_direct)
                                {
                                    l1 = findans(availhash, list->ans);
                                    while (l1)
                                    {
                                        clearbit(l->p_agen, (int)l1->data - 1);
                                        setbit(l->p_akill, (int)l1->data - 1);
                                        l1 = l1->link;
                                    }
                                }
                                else if (s->mode == i_ind)
                                {
                                    BYTE *t4;
                                    ptr = findPointer(list->ans);
                                    if (ptr < pointer_count)
                                    {
                                        t4 = temparray4 + ptr * sourcebytes;
                                        if (isset(t4, 0))
                                        {
                                            memset(l->p_akill, 0xff,
                                                avail_bytes);
                                            memset(l->p_agen, 0, avail_bytes);
                                        }
                                        else
                                        {
                                            for (k = 1; k < source_count; k++)
                                            {
                                                if (isset(t4, k))
                                                {
                                                    SYM *sp = varsp
                                                        (sourcearray[k]
                                                        ->dc.left->offset);
                                                    if (sp)
                                                    {
                                                        l1 = findans(availhash,
                                                            sp->imvalue);
                                                        while (l1)
                                                        {
                                                            clearbit(l->p_agen,
                                                                (int)l1->data -
                                                                1);
                                                            setbit(l->p_akill, 
                                                                (int)l1->data -
                                                                1);
                                                            gencount++;
                                                            l1 = l1->link;
                                                        }
                                                    }
                                                    if (sourcearray[k]
                                                        ->dc.right)
                                                    {
                                                        sp = varsp
                                                            (sourcearray[k]
                                                            ->dc.right->offset);
                                                        if (sp)
                                                        {
                                                            l1 = findans
                                                                (availhash, sp
                                                                ->imvalue);
                                                            while (l1)
                                                            {
                                                                clearbit(l
                                                                    ->p_agen, 
                                                                    (int)l1
                                                                    ->data - 1);
                                                                setbit(l
                                                                    ->p_akill, 
                                                                    (int)l1
                                                                    ->data - 1);
                                                                gencount++;
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
                    }
            }
            if (!skipptr)
                ptrindex(list, l, temparray4);
            list = list->fwd;
        }
    }
}

//-------------------------------------------------------------------------

static void CalculateAvailable(void)
{
    int i, changed, j;
    for (i = 0; i < blocknum; i++)
        memcpy(blockarray[i]->block->p_aout, blockarray[i]->block->p_agen,
            avail_bytes);
    do
    {
        changed = FALSE;
        for (i = 0; i < blocknum; i++)
        {
            BLOCK *t = blockarray[dfst[i]]->block;
            BLOCKLIST *l = t->flowback;
            if (l)
                memset(t->p_ain, 0xff, avail_bytes);
            while (l)
            {
                for (j = 0; j < avail_bytes; j++)t->p_ain[j] &= l->block
                    ->p_aout[j];
                l = l->link;
            }
            for (j = 0; j < avail_bytes; j++)
            {
                int v = t->p_aout[j];
                t->p_aout[j] = t->p_agen[j] | (t->p_ain[j] &~t->p_akill[j]);
                changed |= (v != t->p_aout[j]);
            }
        }
    }
    while (changed)
        ;

}

//-------------------------------------------------------------------------

static void EnterCSE(QUAD *list)
{
    if (cse_count >= cse_max)
    {
        QUAD **newarray = oalloc(sizeof(QUAD*)*(cse_max += 16));
        if (cse_count)
            memcpy(newarray, csearray, sizeof(QUAD*) * cse_count);
        csearray = newarray;
    }
    csearray[cse_count++] = list;
}

//-------------------------------------------------------------------------

static void GatherCSE(BLOCKLIST *l, QUAD *match)
{
    QUAD *list;
    while (l)
    {
        if (!(l->block->flags &BLOCKLIST_VISITED))
        {
            l->block->flags |= BLOCKLIST_VISITED;
            list = l->block->tail;
            while (list != l->block->head)
            {
                if (!memcmp(match, list, sizeof(struct _basic_dag)))
                    break;
                list = list->back;
            }
            if (list == l->block->head)
            {
                GatherCSE(l->block->flowback, match);
                if (csexit)
                    return ;
            } 
            else
            {
                if (csesize !=  - 1000 && list->ans->size != csesize)
                {
                    csexit = TRUE;
                    return ;
                }
                EnterCSE(list);
                csesize = list->ans->size;
            }
        }
        l = l->link;
    }
}

//-------------------------------------------------------------------------

static void unvisit(void)
{
    int i;
    for (i = 0; i < blocknum; i++)
        blockarray[i]->block->flags = 0;
}

//-------------------------------------------------------------------------

static int ScanForGCSE(BLOCK *l, int index)
{
    int rv = FALSE;
    QUAD *list = l->head;
    QUAD *match = availarray[index];
    BLOCKLIST *s;
    IMODE *im,  *ima;
    QUAD *t;
    LIST *l1;
    int i, ptr, j;
    SYM *sp;
    memcpy(temparray4, l->p_pin, sourcebytes *pointer_count);
    while (1)
    {
        while (list && list->back != l->tail && memcmp(match, list, sizeof
            (struct _basic_dag)))
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
                    if (list->ans == match->dc.left || list->ans == match
                        ->dc.right)
                        return rv;
                    // take pointers into account for avail & reaching
                    if (list->ans->mode == i_ind)
                    {
                        BYTE *t4;
                        ptr = findPointer(list->ans);
                        if (ptr < pointer_count)
                        {
                            t4 = temparray4 + ptr * sourcebytes;
                            if (isset(t4, 0))
                            {
                                return rv;
                            }
                            else
                            {
                                for (i = 1; i < source_count; i++)
                                {
                                    if (isset(t4, i))
                                    {
                                        sp = varsp(sourcearray[i]->dc.left
                                            ->offset);
                                        if (sp->imvalue == match->dc.left || sp
                                            ->imvalue == match->dc.right)
                                            return rv;
                                        if (sourcearray[i]->dc.right)
                                        {
                                            sp = sourcearray[i]->dc.right
                                                ->offset;
                                            if (sp->imvalue == match->dc.left 
                                                || sp->imvalue == match
                                                ->dc.right)
                                                return rv;
                                        }
                                    }
                                }
                            }
                        }
                    }
                    else
                    {
                        ptr = findPointer(list->ans);
                        if (ptr < pointer_count)
                        {
                            SYM *sp = varsp(list->ans->offset);
                            if (sp)
                            {
                                l1 = findans(availhash, sp->imind);
                                if (l1)
                                    return rv;
                            }
                        }
                    }
                    break;
                case i_parm:
                    sp = varsp(list->dc.left->offset);
                    if (list->dc.left->mode == i_immed)
                    {
                        if (sp)
                        {
                            if (sp->imvalue == match->dc.left || sp->imvalue ==
                                match->dc.right)
                                return rv;
                        }
                    }
                    else
                    {
                        if (list->dc.left->mode == i_direct)
                        {
                            ptr = findPointer(list->dc.left);
                            if (ptr < pointer_count)
                            {
                                BYTE *t4 = temparray4 + ptr * sourcebytes;
                                if (isset(t4, 0))
                                    return rv;
                                else
                                for (i = 1; i < source_count; i++)
                                {
                                    if (isset(t4, i))
                                    {
                                        sp = varsp(sourcearray[i]->dc.left
                                            ->offset);
                                        if (sp->imvalue == match->dc.left || sp
                                            ->imvalue == match->dc.right)
                                            return rv;
                                        sp = varsp(sourcearray[i]->dc.right
                                            ->offset);
                                        if (sp)
                                            if (sp->imvalue == match->dc.left 
                                                || sp->imvalue == match
                                                ->dc.right)
                                                return rv;
                                    }
                                }
                            }
                        }
                    }
                    break;
                case i_gosub:
                    switch (match->dc.left->offset->nodetype)
                    {
                    case en_nacon:
                    case en_napccon:
                    case en_nalabcon:
                    case en_labcon:
                    case en_absacon:
                        return rv;

                    }
                    switch (match->dc.right->offset->nodetype)
                    {
                    case en_nacon:
                    case en_napccon:
                    case en_nalabcon:
                    case en_labcon:
                    case en_absacon:
                        return rv;

                    }
                    for (i = 0; i < pointer_count; i++)
                    switch (pointerarray[i]->offset->nodetype)
                    {
                    case en_nacon:
                    case en_napccon:
                    case en_nalabcon:
                    case en_labcon:
                    case en_absacon:
                        if (isset(temparray4 + i * sourcebytes, 0))
                            return rv;
                        for (j = 1; j < source_count; j++)
                        {
                            if (sourcearray[j]->dc.left == match->dc.left)
                                return rv;
                            if (sourcearray[j]->dc.left == match->dc.right)
                                return rv;
                            if (sourcearray[j]->dc.right == match->dc.left)
                                return rv;
                            if (sourcearray[j]->dc.right == match->dc.right)
                                return rv;
                        }
                        break;
                    }
                    break;

            }
            ptrindex(list, l, temparray4);
            list = list->fwd;
        }
        if (!list || list->back == l->tail)
            return rv;
        unvisit();
        cse_count = 0;
        csesize =  - 1000;
        csexit = FALSE;
        GatherCSE(l->flowback, match);
        if (cse_count && !csexit)
        {
            rv = TRUE;
            im = 0;
            for (i = 0; i < cse_count; i++)
            {
                sp = varsp(csearray[i]->ans->offset);
                if (!sp->iglobal && sp->storage_class == sc_temp)
                {
                    im = sp->imvalue;
                    break;
                }
            }
            if (!im)
                im = tempreg(csesize, FALSE);
            for (i = 0; i < cse_count; i++)
            {
                if (im != csearray[i]->ans)
                {
                    ima = csearray[i]->ans;
                    csearray[i]->ans = im;
                    t = xalloc(sizeof(QUAD));
                    t->ans = ima;
                    t->dc.left = im;
                    t->dc.opcode = i_assn;
                    t->fwd = csearray[i]->fwd;
                    t->fwd->back = t;
                    t->back = csearray[i];
                    t->back->fwd = t;
                }
            }
            list->dc.left = im;
            list->dc.opcode = i_assn;
            list->dc.right = 0;
        }
        else
            list = list->fwd;
    }
}

//-------------------------------------------------------------------------

void gcse(void)
{
    int i, j;
    int done = FALSE;
    availhash = oalloc(sizeof(LIST3*) * AHASH);
    availhash2 = oalloc(sizeof(LIST3*) * AHASH);
    tempindexarray = oalloc(sizeof(SYM*) * tempnum);
    CountAvail();
    CreateAvailBitArrays();
    while (!done)
    {
        done = TRUE;
        CalculateAvailBasics();
        CalculateAvailable();
        #ifdef DUMP_GCSE_INFO
            dump_avail();
        #endif 
        for (i = 0; i < blocknum; i++)
        {
            BLOCK *l = blockarray[dfst[i]]->block;
            for (j = 0; j < avail_count; j++)
                if (isset(l->p_ain, j))
                    if (ScanForGCSE(l, j))
                        done = FALSE;
        }
    }
}

//-------------------------------------------------------------------------

void gcseRundown(void)
{
    cse_max = cse_count = 0;
}
