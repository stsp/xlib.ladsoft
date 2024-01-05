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
 * igeqn.c
 *
 * iteratively determine equations for gen, kill, in, out, and so forth
 * uses lots of hash tables to speed searches through the arrays
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

#define DHASH DAGSIZE

extern int global_flag;
extern int blocknum;
extern int *dfst;
extern BLOCKLIST **blockarray;
extern int pointer_count, source_count;
extern IMODE **pointerarray;
extern QUAD **sourcearray;
extern BYTE *temparray4;
extern int sourcebytes;
extern int tempnum;

unsigned char bittab[8] = 
{
    1, 2, 4, 8, 16, 32, 64, 128
};
QUAD **defiarray;


static BYTE *temparray1,  *temparray2;
static int definition_count, definition_bytes;
static int max_definitions;
static QUAD **defarray;
static LIST2 **defhash;
static LIST3 **defhash2;

#ifdef DUMP_GCSE_INFO
    extern FILE *icdFile,  *outputFile;
    void dumpset(BYTE *set, QUAD **array, char *title, int count)
    {
        int i;
        fprintf(icdFile, "\n;%s\n", title);
        for (i = 0; i < count; i++)
        if (isset(set, i))
        {
            if (array == defarray && defiarray[i])
            {
                fprintf(icdFile, "; (");
                putamode(defiarray[i]->dc.left);
                if (defiarray[i]->dc.right)
                {
                    fprintf(icdFile, ", ");
                    putamode(defiarray[i]->dc.right);
                }
                fprintf(icdFile, ") ");
            }
            else
                fprintf(icdFile, "; ");
            put_code(array[i]);
        }

    }
    static void dumpeqn(void)
    {
        int i;
        char buf[256];
        FILE *temp;
        if (!icdFile)
            return ;
        temp = outputFile;
        outputFile = icdFile;
        fprintf(icdFile, "\n; Definitions: %d\n", definition_count);
        for (i = 0; i < definition_count; i++)
        {
            fprintf(icdFile, "; ");
            if (defiarray[i])
            {
                fprintf(icdFile, "( ");
                putamode(defiarray[i]->dc.left);
                if (defiarray[i]->dc.right)
                {
                    fprintf(icdFile, ", ");
                    putamode(defiarray[i]->dc.right);
                }
                fprintf(icdFile, ") ");
            }
            put_code(defarray[i]);
        }

        fprintf(icdFile, "\n; Sets:\n");
        for (i = 0; i < blocknum; i++)
        {
            sprintf(buf, "; Gen block %d", i + 1);
            dumpset(blockarray[i]->block->p_gen, defarray, buf,
                definition_count);
            sprintf(buf, "; kill block %d", i + 1);
            dumpset(blockarray[i]->block->p_kill, defarray, buf,
                definition_count);
            sprintf(buf, "; in block %d", i + 1);
            dumpset(blockarray[i]->block->p_in, defarray, buf, definition_count)
                ;
            sprintf(buf, "; out block %d", i + 1);
            dumpset(blockarray[i]->block->p_out, defarray, buf,
                definition_count);
        }
        outputFile = temp;
    }
#endif 
void anshash(LIST3 **table, IMODE *ans, QUAD *value)
{
    int hash;
    LIST3 *l3;
    LIST *list;
    if (!ans)
        return ;
    hash = dhash(&ans, sizeof(ans));
    l3 = table[hash];
    while (l3)
    {
        if (l3->ans == ans)
            break;
        l3 = l3->link;
    }
    if (!l3)
    {
        l3 = oalloc(sizeof(LIST3));
        l3->ans = ans;
        l3->link = table[hash];
        table[hash] = l3;
    }
    list = oalloc(sizeof(LIST));
    list->link = l3->decllist;
    l3->decllist = list;
    list->data = value;
}

//-------------------------------------------------------------------------

LIST *findans(LIST3 **table, IMODE *ans)
{
    int j = dhash(&ans, sizeof(ans));
    LIST3 *l3 = table[j];
    while (l3)
    {
        if (l3->ans == ans)
            return l3->decllist;
        l3 = l3->link;
    }
    return 0;
}

//-------------------------------------------------------------------------

static int CacheDefinition(int id, IMODE *ans, QUAD *value)
{
    LIST2 *l = oalloc(sizeof(LIST2));
    int hash;
    l->data.ans = ans;
    l->data.val = value;
    l->id = id;
    hash = dhash(&l->data, sizeof(l->data));
    l->link = defhash[hash];
    defhash[hash] = l;

    anshash(defhash2, ans, value);
}

//-------------------------------------------------------------------------

int FindDefinition(int j, QUAD *list)
{
    int hash;
    struct _l2data data;
    LIST2 *l;
    data.ans = sourcearray[j];
    data.val = list;
    hash = dhash(&data, sizeof(data));
    l = defhash[hash];
    while (l)
    {
        if (!memcmp(&data, &l->data, sizeof(data)))
        {
            return l->id;
        } l = l->link;
    }
    return  - 1;
}

//-------------------------------------------------------------------------

static void enter_definition(QUAD *ans, QUAD *head)
{
    if (definition_count >= max_definitions)
    {
        QUAD **newarray = oalloc(sizeof(QUAD*)*(max_definitions += 256));
        if (defarray)
            memcpy(newarray, defarray, sizeof(QUAD*)*(max_definitions - 256));
        defarray = newarray;
        newarray = oalloc(sizeof(IMODE*)*(max_definitions));
        if (defiarray)
            memcpy(newarray, defiarray, sizeof(IMODE*)*(max_definitions - 256));
        defiarray = newarray;
    }
    if (ans)
    {
        CacheDefinition(definition_count, ans, head);
        defarray[definition_count] = head;
        defiarray[definition_count] = ans;
        definition_count++;
    }
    else
    {
        CacheDefinition(definition_count, head->ans, head);
        defarray[definition_count] = head;
        defiarray[definition_count] = head;
        head->definition = definition_count++;
    }
}

//-------------------------------------------------------------------------

int findPointer(IMODE *ans)
{
    IMODE p;
    int k;
    SYM *sp;
    if (ans->mode != i_ind)
        return INT_MAX;
    sp = varsp(ans->offset);
    if (!sp->ipointerindx)
        return INT_MAX;
    return sp->ipointerindx - 1;
}

//-------------------------------------------------------------------------

static void CountDefinitions(void)
{
    int block = 0, i, j, k;

    definition_count = 0;

    defarray = 0;

    max_definitions = 0;

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
                    enter_definition(0, head);
                    k = findPointer(head->ans);
                    if (k < pointer_count)
                    {
                        for (j = 1; j < source_count; j++)
                        if (isset(temparray4 + k * sourcebytes, j))
                        {
                            enter_definition(sourcearray[j], head);
                        }
                    }
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
                    if (head->dc.left)
                    {
                        k = findPointer(head->dc.left);
                        if (k < pointer_count)
                        {
                            for (j = 1; j < source_count; j++)
                            if (isset(temparray4 + k * sourcebytes, j))
                            {
                                enter_definition(sourcearray[j], head);
                            }
                        }
                    }
                    if (head->dc.right)
                    {
                        k = findPointer(head->dc.right);
                        if (k < pointer_count)
                        {
                            for (j = 1; j < source_count; j++)
                            if (isset(temparray4 + k * sourcebytes, j))
                            {
                                enter_definition(sourcearray[j], head);
                            }
                        }
                    }
                    break;
                case i_gosub:
                    l->callcount++;
                    break;
                    break;

            }
            ptrindex(head, l, temparray4);
            head = head->fwd;
        }
    }
}

//-------------------------------------------------------------------------

void CreateBitArrays(void)
{
    int i;
    definition_bytes = (definition_count + 8) / 8;
    if (definition_bytes % 4)
        definition_bytes += 4-definition_bytes % 4;
    for (i = 0; i < blocknum; i++)
    {
        blockarray[i]->block->p_gen = oalloc(definition_bytes);
        blockarray[i]->block->p_kill = oalloc(definition_bytes);
        blockarray[i]->block->p_in = oalloc(definition_bytes);
        blockarray[i]->block->p_out = oalloc(definition_bytes);
    }
    temparray1 = oalloc(definition_bytes);
    temparray2 = oalloc(definition_bytes);
}

//-------------------------------------------------------------------------

void CalculateBlockBasics(void)
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
                    // start reaching defs
                    memset(temparray1, 0, definition_bytes);
                    memset(temparray2, 0, definition_bytes);
                    setbit(temparray1, list->definition); // gen
                    // gen bits
                    l1 = findans(defhash2, list->ans);
                    if (!l1)
                        DIAG("CalculateBlockBasics - can't find defintion");
                    else
                    {
                        while (l1)
                        {
                            int d = ((QUAD*)(l1->data))->definition;
                            if (list->definition != d)
                                setbit(temparray2, d);
                            l1 = l1->link;
                        }
                    }
                    if (list->ans->mode == i_ind)
                    {
                        BYTE *t4;
                        ptr = findPointer(list->ans);
                        if (ptr < pointer_count)
                        {
                            t4 = temparray4 + ptr * sourcebytes;
                            if (isset(t4, 0))
                            {
                                memset(temparray1, 0xff, definition_bytes);
                            }
                            else
                            {
                                for (j = 1; j < source_count; j++)
                                {
                                    if (isset(t4, j))
                                    {
                                        def2 = FindDefinition(j, list);
                                        if (def2 ==  - 1)
                                            DIAG(
                                                "CalculateBlockBasics - can't find def");
                                        else
                                        {
                                            // gen and kill in presence of pointers
                                            setbit(temparray1, def2);
                                            clearbit(temparray2, def2);
                                        }
                                    }
                                }
                            }
                        }
                    }
                    // finish reaching defs
                    for (j = 0; j < definition_bytes; j += 4)
                    {
                        *(unsigned*)(l->p_gen + j) &= ~*(unsigned*)(temparray2 
                            + j);
                        *(unsigned*)(l->p_gen + j) |= *(unsigned*)(temparray1 +
                            j);
                        *(unsigned*)(l->p_kill + j) &= ~*(unsigned*)(temparray1
                            + j);
                        *(unsigned*)(l->p_kill + j) |= *(unsigned*)(temparray2 
                            + j);
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
                        memset(temparray1, 0, definition_bytes);
                        memset(temparray2, 0, definition_bytes);
                        if (sp && list->dc.left->mode == i_immed)
                        {
                            l1 = findans(defhash2, sp->imvalue);
                            if (!l1)
                                DIAG(
                                    "CalculateBlockBasics - can't find defintion");
                            else
                            {
                                while (l1)
                                {
                                    int d = ((QUAD*)(l1->data))->definition;
                                    if (list->definition != d)
                                        setbit(temparray2, d);
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
                                    memset(temparray1, 0xff, definition_bytes);
                                }
                                else
                                {
                                    for (j = 1; j < source_count; j++)
                                    {
                                        if (isset(t4, j))
                                        {
                                            def2 = FindDefinition(j, list);
                                            if (def2 ==  - 1)
                                                DIAG(
                                                    "CalculateBlockBasics - can't find def");
                                            else
                                            {
                                                // gen and kill in presence of pointers
                                                setbit(temparray1, def2);
                                                clearbit(temparray2, def2);
                                            }
                                        }
                                    }
                                }
                            }
                        }
                        for (j = 0; j < definition_bytes; j += 4)
                        {
                            *(unsigned*)(l->p_gen + j) &= ~*(unsigned*)
                                (temparray2 + j);
                            *(unsigned*)(l->p_gen + j) |= *(unsigned*)
                                (temparray1 + j);
                            *(unsigned*)(l->p_kill + j) &= ~*(unsigned*)
                                (temparray1 + j);
                            *(unsigned*)(l->p_kill + j) |= *(unsigned*)
                                (temparray2 + j);
                        }
                    }
                    break;
                case i_gosub:
                    // kill globals
                    for (j = 0; j < definition_count; j++)
                    {
                        IMODE *s = defiarray[j]->ans;
                        switch (s->offset->nodetype)
                        {
                        case en_nacon:
                        case en_napccon:
                        case en_nalabcon:
                        case en_labcon:
                        case en_absacon:
                            if (s->mode == i_direct)
                            {
                                clearbit(l->p_gen, j);
                                setbit(l->p_kill, j);
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
                                        memset(temparray1, 0xff,
                                            definition_bytes);
                                    }
                                    else
                                    {
                                        for (k = 1; k < source_count; k++)
                                        {
                                            if (isset(t4, k))
                                            {
                                                clearbit(l->p_gen, k);
                                                setbit(l->p_kill, k);
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
            }
            ptrindex(list, l, temparray4);
            list = list->fwd;
        }
    }
}

//-------------------------------------------------------------------------

void CalculateReaching(void)
{
    int i, changed, j;
    for (i = 0; i < blocknum; i++)
        memcpy(blockarray[i]->block->p_out, blockarray[i]->block->p_gen,
            definition_bytes);
    do
    {
        changed = FALSE;
        for (i = 0; i < blocknum; i++)
        {
            BLOCK *t = blockarray[dfst[i]]->block;
            BLOCKLIST *l = t->flowback;
            memset(t->p_in, 0, definition_bytes);
            while (l)
            {
                for (j = 0; j < definition_bytes; j++)t->p_in[j] |= l->block
                    ->p_out[j];
                l = l->link;
            }
            for (j = 0; j < definition_bytes; j++)
            {
                int v = t->p_out[j];
                t->p_out[j] = t->p_gen[j] | (t->p_in[j] &~t->p_kill[j]);
                changed |= (v != t->p_out[j]);
            }
        }
    }
    while (changed)
        ;

}

//-------------------------------------------------------------------------

void CalculateEquations(QUAD *head)
{
    return ;
    defhash = oalloc(sizeof(LIST2*) * DHASH);
    defhash2 = oalloc(sizeof(LIST3*) * DHASH);
    CountDefinitions();
    CreateBitArrays();
    CalculateBlockBasics();
    CalculateReaching();
    #ifdef DUMP_GCSE_INFO
        dumpeqn();
    #endif 
}
