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
 * iflow.c
 *
 * create flow graph and dominator tree
 */
/* Define this to get a dump of the flow graph and dominator tree 
 * These are dumped into ccfg.$$$
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

extern BLOCKLIST *blockhead;
extern int blocknum;


int *dfst;
BLOCKLIST **blockarray;
static int dfstcount;
#ifdef DUMP_GCSE_INFO
    extern FILE *icdFile,  *outputFile;

    /* dump the flow graph */
    static void dump_flowgraph(BLOCKLIST *list)
    {
        while (list)
        {
            BLOCKLIST *list1 = list->block->flowfwd;
            fprintf(icdFile, "\n; %d: ", list->block->blocknum + 1);
            while (list1)
            {
                fprintf(icdFile, " %d", list1->block->blocknum + 1);
                list1 = list1->link;
            }
            fprintf(icdFile, " *");
            list1 = list->block->flowback;
            while (list1)
            {
                fprintf(icdFile, " %d", list1->block->blocknum + 1);
                list1 = list1->link;
            }
            list = list->link;
        }
    }
#endif 
/* find a label on the list */
static BLOCKLIST *findlab(int labnum)
{
    BLOCKLIST *head = blockhead->link;
    while (head)
    {
        QUAD *ihead = head->block->head;
        ihead = ihead->fwd;
        while (ihead && ihead->dc.opcode != i_label)
        {
            if (ihead->dc.opcode == i_block)
                head = head->link;
            ihead = ihead->fwd;
        }
        if (ihead->dc.label == labnum)
            break;
        head = head->link;
    }
    return head;
}

/* insert on a flowgraph node */
static void flowinsert(BLOCKLIST **pos, BLOCKLIST *value)
{
    BLOCKLIST *nblock = oalloc(sizeof(BLOCKLIST));
    nblock->link = (*pos);
    (*pos) = nblock;
    nblock->block = value->block;
}

/* create the flowgraph.  */
static void gather_flowgraph(BLOCKLIST *list)
{
    BLOCKLIST *temp;
    int i = 0;
    blockarray = oalloc(blocknum *sizeof(BLOCKLIST **));
    while (list)
    {
        QUAD *tail = list->block->tail;
        blockarray[i++] = list;
        switch (tail->dc.opcode)
        {
            case i_goto:
                temp = findlab(tail->dc.label);
                if (temp)
                {
                    flowinsert(&temp->block->flowback, list);
                    flowinsert(&list->block->flowfwd, temp);
                }
                else
                {
                    DIAG("flow:unfound label");
                    printf("g %d", tail->dc.label);
                }
                break;
            case i_dc:
                while (tail->dc.opcode == i_dc)
                {
                    temp = findlab(tail->dc.label);
                    if (temp)
                    {
                        flowinsert(&temp->block->flowback, list);
                        flowinsert(&list->block->flowfwd, temp);
                    }
                    else
                    {
                        DIAG("flow:unfound label");
                        printf("dc %d", tail->dc.label);
                    }
                    tail = tail->back;
                }
                break;
            case i_coswitch:
                temp = findlab(tail->dc.label);
                if (temp)
                {
                    flowinsert(&temp->block->flowback, list);
                    flowinsert(&list->block->flowfwd, temp);
                }
                else
                {
                    DIAG("flow:unfound label");
                    printf("cs %d", tail->dc.label);
                }
                temp = list->link;
                if (temp)
                {
                    flowinsert(&temp->block->flowback, list);
                    flowinsert(&list->block->flowfwd, temp);
                }
                break;
            case i_je:
            case i_jne:
            case i_jl:
            case i_jc:
            case i_jg:
            case i_ja:
            case i_jle:
            case i_jbe:
            case i_jge:
            case i_jnc:
                temp = findlab(tail->dc.label);
                if (temp)
                {
                    flowinsert(&temp->block->flowback, list);
                    flowinsert(&list->block->flowfwd, temp);
                }
                else
                {
                    DIAG("flow:unfound label");
                    printf("j %d", tail->dc.label);
                }
                // fall through
            default:
                temp = list->link;
                if (temp)
                {
                    flowinsert(&temp->block->flowback, list);
                    flowinsert(&list->block->flowfwd, temp);
                }
                break;
        }
        list = list->link;
    }
}

//-------------------------------------------------------------------------

void dump_dfst(void)
{
    int i;
    for (i = 0; i < blocknum; i++)
        fprintf(icdFile, "%d, ", dfst[i] + 1);
    fprintf(icdFile, "\n");
}

/* create the depth-first tree */
void create_dfst(int n)
{
    BLOCKLIST *l = blockarray[n]->block->flowfwd;
    blockarray[n]->block->flags |= BLOCKLIST_VISITED;
    while (l)
    {
        if (!(l->block->flags &BLOCKLIST_VISITED))
        {
            // should add edge here
            create_dfst(l->block->blocknum);

        }
        l = l->link;
    }
    blockarray[n]->block->dfstnum = dfstcount;
    dfst[--dfstcount] = n;
}

/* main routine for creating flowgraph and dominator info */
void flows_and_doms(void)
{
    gather_flowgraph(blockhead);
    dfst = oalloc(sizeof(int) *blocknum);
    dfstcount = blocknum;
    create_dfst(0);
    #ifdef DUMP_GCSE_INFO
        if (icdFile)
        {
            fprintf(icdFile, "; Flowgraph dump");
            dump_flowgraph(blockhead);
            fprintf(icdFile, "\n\n; dfst dump\n; ");
            dump_dfst();
        }
    #endif 
}
