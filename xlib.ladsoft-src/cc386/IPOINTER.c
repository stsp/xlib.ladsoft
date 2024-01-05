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
 * ipointer.c
 *
 * determine what pointers point to
 * This can use up a lot of memory for the pointer arrays
 */
#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include "utype.h"	
#include "cmdline.h"	
#include "lists.h"
#include "expr.h"
#include "c.h"
#include "iexpr.h"
#include "iopt.h"
#include "diag.h"

#define PHASHSIZE DAGSIZE

extern int global_flag;
extern int blocknum;
extern int *dfst;
extern BLOCKLIST **blockarray;
extern unsigned char bittab[8];

BYTE *temparray4;
int pointer_count, source_count;
IMODE **pointerarray;
QUAD **sourcearray;
int sourcebytes;
static int max_pointer, max_source;
static IMODE everything;

#ifdef DUMP_GCSE_INFO        
    extern FILE *icdFile,  *outputFile;
    static void dumpArray(char *title, BYTE *set)
    {
        int i, j;
        fprintf(icdFile, "\n;%s\n", title);
        for (j = 0; j < pointer_count; j++)
        {
            fprintf(icdFile, ";   ");
            putamode(pointerarray[j]);
            fprintf(icdFile, "\n");
            if (isset(set + j * sourcebytes, 0))
                fprintf(icdFile, ";      Everything\n");
            for (i = 1; i < source_count; i++)
            if (isset(set + j * sourcebytes, i))
            {
                fprintf(icdFile, ";      ");
                put_code(sourcearray[i]);
            }

        }

    }
    static void dumpPointers(void)
    {
        int i;
        FILE *temp;
        if (!icdFile)
            return ;
        temp = outputFile;
        outputFile = icdFile;
        fprintf(icdFile, "\n; Pointers: %d\n", pointer_count);
        for (i = 0; i < pointer_count; i++)
        {
            fprintf(icdFile, "; ");
            putamode(pointerarray[i]);
            fprintf(icdFile, "\n");
        }
        fprintf(icdFile, "\n; Pointer Sources: %d\n", source_count);
        for (i = 1; i < source_count; i++)
        {
            fprintf(icdFile, "; ");
            put_code(sourcearray[i]);
        }
        fprintf(icdFile, "\n; Pointer in outs\n");
        for (i = 0; i < blocknum; i++)
        {
            char buf[256];
            sprintf(buf, "; Pointer in %d\n", i + 1);
            dumpArray(buf, blockarray[i]->block->p_pin);
            sprintf(buf, "; Pointer out %d\n", i + 1);
            dumpArray(buf, blockarray[i]->block->p_pout);
        }
        outputFile = temp;
    }
#endif 

//-------------------------------------------------------------------------

static void insertPtr(IMODE *var)
{
    IMODE *ptr;
    SYM *sp;
    if (!var->offset)
        return ;
    sp = varsp(var->offset);
    if (sp)
    {
        if (sp->ipointerindx)
            return ;
        if (var->mode == i_direct && (!sp->tp || sp->tp->type != bt_pointer &&
            sp->tp->type != bt_ref))
            return ;
        sp->ipointerindx = pointer_count + 1;
        if (ptr->mode == i_ind)
        {
            if (sp && sp->imind)
                ptr = sp->imind;
            else
            {
                global_flag++;
                ptr = sp->imind = xalloc(sizeof(IMODE));
                global_flag--;
                *ptr =  *var;
                ptr->mode = i_ind;
            }
        }
        else
            ptr = var;

    }
    else
    {
        if (ptr->mode == i_ind)
        {
            global_flag++;
            ptr = xalloc(sizeof(IMODE));
            global_flag--;
            *ptr =  *var;
            ptr->mode = i_direct;
        }
        else
            ptr = var;
    }
    if (pointer_count >= max_pointer)
    {
        IMODE **new_array = oalloc(sizeof(IMODE*)*(max_pointer += 256));
        if (pointerarray)
            memcpy(new_array, pointerarray, sizeof(IMODE*)*(max_pointer - 256));
        pointerarray = new_array;
    }
    pointerarray[pointer_count++] = ptr;
}

//-------------------------------------------------------------------------

static void insertSource(QUAD *var)
{
    if (source_count >= max_source)
    {
        QUAD **new_array = oalloc(sizeof(QUAD*)*(max_source += 256));
        if (sourcearray)
            memcpy(new_array, sourcearray, sizeof(QUAD*)*(max_source - 256));
        sourcearray = new_array;
    }
    var->sourceindx = source_count;
    sourcearray[source_count++] = var;
}

//-------------------------------------------------------------------------

static void FindPointers(void)
{
    int i;
    BLOCK *l = blockarray[0]->block;
    QUAD *list = l->head;
    pointerarray = 0;
    max_pointer = pointer_count = 0;
    while (list)
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
                if (list->ans)
                    insertPtr(list->ans);
                if (list->dc.left && list->dc.left->mode == i_ind)
                    insertPtr(list->dc.left);
                if (list->dc.right && list->dc.right->mode == i_ind)
                    insertPtr(list->dc.right);
                break;

        }
        list = list->fwd;
    }

}

//-------------------------------------------------------------------------

static void FindSources(void)
{
    int i;
    BLOCK *l = blockarray[0]->block;
    QUAD *list = l->head;

    sourcearray = xalloc(256 *sizeof(IMODE*));
    sourcearray[0] = &everything;
    max_source = 256;
    source_count = 1;
    while (list)
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
                if (list->ans && (list->ans->mode == i_direct || list->ans
                    ->mode == i_ind))
                {
                    IMODE p =  *list->ans;
                    p.mode = i_direct;
                    for (i = 0; i < pointer_count; i++)
                    if (equalimode(&p, pointerarray[i]))
                    {
                        insertSource(list);
                        break;
                    }
                }
                break;

        }
        list = list->fwd;
    }

}

//-------------------------------------------------------------------------

static void CreateBitmaps(void)
{
    char *mem;
    int i, j;
    int c;
    sourcebytes = (source_count + 9) / 8;
    if (sourcebytes % 4)
        sourcebytes += (4-sourcebytes % 4);
    c = pointer_count * sourcebytes;
    mem = oalloc((blocknum *2+1) *c);
    for (i = 0; i < blocknum; i++)
    {
        blockarray[i]->block->p_pin = mem;
        mem += c;
        blockarray[i]->block->p_pout = mem;
        mem += c;
    }
    temparray4 = mem;
    mem += c;
}

//-------------------------------------------------------------------------

void ptrindex(QUAD *list, BLOCK *t, BYTE *temparray4)
{
    int changed = FALSE, i, j, pi, done;
    SYM *sp,  *sp2;
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
            sp = varsp(list->ans->offset);
            if (sp)
            {
                /* This is looser than it should be, if they add
                 * a constant to a pointer to a non-array we will still
                 * assume it points to the non-array
                 */
                pi = sp->ipointerindx;
                if (pi)
                {
                    pi--;
                    if (list->ans->mode == i_direct)
                    {
                        memset(temparray4 + pi * sourcebytes, 0, sourcebytes);
                        if (!list->dc.right)
                        {
                            sp = varsp(list->dc.left->offset);
                            if (list->dc.left->mode == i_immed)
                            {
                                if (sp)
                                {
                                    setbit(temparray4 + pi * sourcebytes, list
                                        ->sourceindx);
                                }
                            }
                            else if (list->dc.left->mode == i_direct)
                            {
                                if (sp && sp->ipointerindx)
                                    memcpy(temparray4 + pi * sourcebytes,
                                        temparray4 + (sp->ipointerindx - 1)
                                        *sourcebytes, sourcebytes);
                            }
                        }
                        else
                        {
                            done = FALSE;
                            sp = varsp(list->dc.right->offset);
                            if (list->dc.right->mode == i_immed)
                            {
                                if (sp)
                                {
                                    sp2 = varsp(list->dc.left->offset);
                                    if (!sp2 || list->dc.left->mode != i_immed)
                                    {
                                        setbit(temparray4 + pi * sourcebytes,
                                            list->sourceindx);
                                        done = TRUE;
                                    }
                                }
                            }
                            else if (list->dc.right->mode == i_direct)
                            {
                                if (sp && sp->ipointerindx)
                                {
                                    sp2 = varsp(list->dc.left->offset);
                                    if (!sp2 || list->dc.left->mode != i_immed)
                                    {
                                        memcpy(temparray4 + pi * sourcebytes,
                                            temparray4 + (sp->ipointerindx - 1)
                                            *sourcebytes, sourcebytes);
                                        done = TRUE;
                                    }
                                }
                            }
                            if (!done)
                            {
                                sp = varsp(list->dc.left->offset);
                                if (list->dc.left->mode == i_immed)
                                {
                                    if (sp)
                                    {
                                        sp2 = varsp(list->dc.right->offset);
                                        if (!sp2 || list->dc.right->mode !=
                                            i_immed)
                                        {
                                            setbit(temparray4 + pi *
                                                sourcebytes, list->sourceindx);
                                        }
                                    }
                                }
                                else if (list->dc.left->mode == i_direct)
                                {
                                    if (sp && sp->ipointerindx)
                                    {
                                        sp2 = varsp(list->dc.right->offset);
                                        if (!sp2 || list->dc.right->mode !=
                                            i_immed)
                                            memcpy(temparray4 + pi *
                                                sourcebytes, temparray4 + (sp
                                                ->ipointerindx - 1)
                                                *sourcebytes, sourcebytes);
                                    }
                                }
                            }
                        }
                    }
                    else if (list->ans->mode == i_ind)
                    {
                        if (isset(temparray4 + pi * sourcebytes, 0))
                        {
                            for (i = 0; i < pointer_count; i++)
                            {
                                memset(temparray4 + i * sourcebytes, 0,
                                    sourcebytes);
                                setbit(temparray4 + i * sourcebytes, 0);
                            }
                        }
                        else
                        for (i = 1; i < source_count; i++)
                        {
                            if (isset(temparray4 + pi * sourcebytes, i))
                            {
                                int pi1 =  - 1;
                                QUAD *q = sourcearray[i];
                                sp = varsp(q->dc.left->offset);
                                if (sp && q->dc.left->mode == i_immed && sp
                                    ->ipointerindx)
                                {
                                    pi1 = sp->ipointerindx - 1;
                                }
                                else
                                {
                                    if (q->dc.right)
                                    {
                                        sp = varsp(q->dc.right->offset);
                                        if (sp && q->dc.right->mode == i_immed
                                            && sp->ipointerindx)
                                        {
                                            pi1 = sp->ipointerindx - 1;
                                        }
                                    }
                                }
                                if (pi1 !=  - 1)
                                {
                                    memset(temparray4 + pi1 * sourcebytes, 0,
                                        sourcebytes);
                                    if (!list->dc.right)
                                    {
                                        sp = varsp(list->dc.left->offset);
                                        if (list->dc.left->mode == i_immed)
                                        {
                                            if (sp)
                                            {
                                                setbit(temparray4 + pi1 *
                                                    sourcebytes, list
                                                    ->sourceindx);
                                            }
                                        }
                                        else if (list->dc.left->mode ==
                                            i_direct)
                                        {
                                            if (sp && sp->ipointerindx)
                                                memcpy(temparray4 + pi1 *
                                                    sourcebytes, temparray4 + 
                                                    (sp->ipointerindx - 1)
                                                    *sourcebytes, sourcebytes);
                                        }
                                    }
                                    else
                                    {
                                        done = FALSE;
                                        sp = varsp(list->dc.right->offset);
                                        if (list->dc.right->mode == i_immed)
                                        {
                                            if (sp)
                                            {
                                                sp2 = varsp(list->dc.left
                                                    ->offset);
                                                if (!sp2 || list->dc.left->mode
                                                    != i_immed)
                                                {
                                                    setbit(temparray4 + pi1 *
                                                        sourcebytes, list
                                                        ->sourceindx);
                                                    done = TRUE;
                                                }
                                            }
                                        }
                                        else if (list->dc.right->mode ==
                                            i_direct)
                                        {
                                            if (sp && sp->ipointerindx)
                                            {
                                                sp2 = varsp(list->dc.left
                                                    ->offset);
                                                if (!sp2 || list->dc.left->mode
                                                    != i_immed)
                                                {
                                                    memcpy(temparray4 + pi1 *
                                                        sourcebytes, temparray4
                                                        + (sp->ipointerindx - 1)
                                                        *sourcebytes,
                                                        sourcebytes);
                                                    done = TRUE;
                                                }
                                            }
                                        }
                                        if (!done)
                                        {
                                            sp = varsp(list->dc.left->offset);
                                            if (list->dc.left->mode == i_immed)
                                            {
                                                if (sp)
                                                {
                                                    sp2 = varsp(list->dc.right
                                                        ->offset);
                                                    if (!sp2 || list->dc.right
                                                        ->mode != i_immed)
                                                    {
                                                        setbit(temparray4 + pi1
                                                            * sourcebytes, list
                                                            ->sourceindx);
                                                    }
                                                }
                                            }
                                            else if (list->dc.left->mode ==
                                                i_direct)
                                            {
                                                if (sp && sp->ipointerindx)
                                                {
                                                    sp2 = varsp(list->dc.right
                                                        ->offset);
                                                    if (!sp2 || list->dc.right
                                                        ->mode != i_immed)
                                                        memcpy(temparray4 + pi1
                                                            * sourcebytes,
                                                            temparray4 + (sp
                                                            ->ipointerindx - 1)
                                                            *sourcebytes,
                                                            sourcebytes);
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
        case i_parm:
            // This needs work for pointers to pointers
            sp = varsp(list->dc.left->offset);
            if (sp)
            {
                pi = sp->ipointerindx;
                if (pi)
                {
                    pi--;
                    if (list->dc.left->mode == i_immed)
                    {
                        memset(temparray4 + pi * sourcebytes, 0, sourcebytes);
                        setbit(temparray4 + pi * sourcebytes, 0); // everything
                    }
                    else if (list->dc.left->mode == i_direct)
                    {
                        if (isset(temparray4 + pi * sourcebytes, 0))
                        {
                            for (i = 0; i < pointer_count; i++)
                            {
                                memset(temparray4 + i * sourcebytes, 0,
                                    sourcebytes);
                                setbit(temparray4 + i * sourcebytes, 0);
                            }
                        }
                        else
                        for (i = 1; i < source_count; i++)
                        {
                            if (isset(temparray4 + pi * sourcebytes, i))
                            {
                                int pi1;
                                QUAD *q = sourcearray[i];
                                sp = varsp(q->dc.left->offset);
                                if (sp && q->dc.left->mode == i_immed && sp
                                    ->ipointerindx)
                                {
                                    pi1 = sp->ipointerindx - 1;
                                    memset(temparray4 + pi1 * sourcebytes, 0,
                                        sourcebytes);
                                    setbit(temparray4 + pi1 * sourcebytes, 0);
                                }
                                else
                                {
                                    if (q->dc.right)
                                    {
                                        sp = varsp(q->dc.right->offset);
                                        if (sp && q->dc.right->mode == i_immed
                                            && sp->ipointerindx)
                                        {
                                            pi1 = sp->ipointerindx - 1;
                                            memset(temparray4 + pi1 *
                                                sourcebytes, 0, sourcebytes);
                                            setbit(temparray4 + pi1 *
                                                sourcebytes, 0);
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
            for (i = 0; i < pointer_count; i++)
            switch (pointerarray[i]->offset->nodetype)
            {
            case en_nacon:
            case en_nalabcon:
            case en_napccon:
            case en_labcon:
            case en_absacon:
                memset(temparray4 + i * sourcebytes, 0, sourcebytes);
                setbit(temparray4 + i * sourcebytes, 0);
                break;
            }
            break;

    }
}

//-------------------------------------------------------------------------

static int ptr_transform(BLOCK *t)
{
    int changed = FALSE, i, j;
    SYM *sp,  *sp2;
    QUAD *list = t->head;
    memcpy(temparray4, t->p_pin, sourcebytes *pointer_count);
    while (list && list->back != t->tail)
    {
        ptrindex(list, t, temparray4);
        list = list->fwd;
    }
    for (i = 0; i < pointer_count; i++)
    {
        int pos = i * sourcebytes;
        for (j = 0; j < sourcebytes; j++)
        if (temparray4[pos + j] != t->p_pout[pos + j])
        {
            t->p_pout[pos + j] = temparray4[pos + j];
            changed = TRUE;
        }
    }
    return changed;

}

//-------------------------------------------------------------------------

static void CalculateUsage(void)
{
    int changed, i, j, k;
    int size = pointer_count * sourcebytes;
    // Global pointers start by pointing to everything
    // technically they shouldn't be able to point to locals.  Oh well...
    for (i = 0; i < pointer_count; i++)
    switch (pointerarray[i]->offset->nodetype)
    {
        case en_nacon:
        case en_nalabcon:
        case en_napccon:
        case en_labcon:
        case en_absacon:
            setbit(blockarray[0]->block->p_pin + i * sourcebytes, 0);
            break;
    }

    do
    {
        changed = FALSE;
        for (i = 0; i < blocknum; i++)
        {
            BLOCK *t = blockarray[dfst[i]]->block;
            BLOCKLIST *l = t->flowback;
            if (t->flowback)
            {
                memset(t->p_pin, 0, size);
                while (l)
                {
                    /* I hand optimized this because BCxx has lousy loop optimization */
                    BYTE *pin = t->p_pin;
                    BYTE *pout = l->block->p_pout;
                    for (j = 0; j < size; j += 4)*((unsigned*)(pin + j))
                         |= *((unsigned*)(pout + j));
                    l = l->link;
                }
            }
            changed |= ptr_transform(t);
        }
    }
    while (changed)
        ;
}

//-------------------------------------------------------------------------

void CalculatePointers(void)
{
    FindPointers();
    FindSources();
    CreateBitmaps();
    CalculateUsage();
    #ifdef DUMP_GCSE_INFO
        dumpPointers();
    #endif 
}

//-------------------------------------------------------------------------

void RundownPointers(void)
{
    int i;
    for (i = 0; i < pointer_count; i++)
    {
        SYM *sp = varsp(pointerarray[i]->offset);
        if (sp)
            sp->ipointerindx = 0;
    }
}
