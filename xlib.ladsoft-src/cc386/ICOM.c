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
 * icom.c
 *
 * Collect common code sequences
 *
 */
#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include "lists.h"
#include "utype.h"	
#include "cmdline.h"	
#include "expr.h"
#include "c.h"
#include "iexpr.h"
#include "iopt.h"

extern int firstlabel;
extern int prm_optcode;
extern int nextlabel;

COMGOREC **matches;

/* Make a list of all gotos */
static COMGOREC *find_goto(QUAD *q, int label)
{
    COMGOREC *rv;
    QUAD *tail;
    int stmt = 0;
    while (!stmt && q)
    {
        if (!q)
            return 0;
        /* Find us a goto with this label */
        while (q && (q->dc.opcode != i_goto || q->dc.label != label))
        {
            q = q->fwd;
            if (!q)
                return 0;
        }
        /* Now find the span of possibly common expressions associated
         * with this goto 
         */
        tail = q;
        q = q->back;
        while (q)
        {
            switch (q->dc.opcode)
            {
                case i_goto:
                case i_jc:
                case i_jnc:
                case i_jbe:
                case i_ja:
                case i_je:
                case i_jne:
                case i_jge:
                case i_jg:
                case i_jle:
                case i_jl:
                case i_dc:
                case i_label:
                case i_coswitch:
                    q = q->fwd->fwd;
                    goto done;
                default:
                    stmt++;
                case i_line:
                case i_block:
                case i_passthrough:
                    if (!q->back)
                        goto done;
                    q = q->back;
                    break;
            }
        }
        done: if (!stmt)
            q = tail->fwd;
    }
    /* Link the goto into the chain */
    if (stmt == 0)
        return 0;
    rv = xalloc(sizeof(COMGOREC));
    rv->tail = tail;
    q = q->back;
    rv->head = q;
    rv->size = stmt;
    return rv;
}

/* See if two quads are equal */
static int equalquad(QUAD *stail, QUAD *dtail)
{
    switch (stail->dc.opcode)
    {
        case i_ret:
        case i_rett:
        case i_genword:
        case i_cppini:
//        case i_parmadj:
            return 1;
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
            if (!equalimode(stail->dc.left, dtail->dc.left))
                return 0;
            return equalimode(stail->dc.right, dtail->dc.right);
        case i_sub:
        case i_udiv:
        case i_umod:
        case i_sdiv:
        case i_smod:
        case i_lsl:
        case i_lsr:
        case i_asl:
        case i_asr:
        case i_setc:
        case i_setnc:
        case i_seta:
        case i_setbe:
        case i_setge:
        case i_setle:
        case i_setg:
        case i_setl:
        case i_sete:
        case i_setne:
            return equalimode(stail->dc.left, dtail->dc.left) && equalimode
                (stail->dc.right, dtail->dc.right);

        case i_add:
        case i_umul:
        case i_smul:
        case i_and:
        case i_or:
        case i_eor:
            if (equalimode(stail->dc.left, dtail->dc.left) && equalimode(stail
                ->dc.right, dtail->dc.right))
                return 1;
            return equalimode(stail->dc.left, dtail->dc.right) && equalimode
                (stail->dc.right, dtail->dc.left);

        case i_parm:
        case i_parmblock:
        case i_trap:
        case i_int:
        case i_goto:
        case i_gosub:
        case i_neg:
        case i_not:
            return equalimode(stail->dc.left, dtail->dc.left);

        case i_assnblock:
        case i_assn:
            return equalimode(stail->dc.left, dtail->dc.left) && equalimode
                (stail->ans, dtail->ans);
        default:
            return 0;
    }
}

/* 
 * Compare two goto groups for matches
 * and return length of match
 */
static int compare(COMGOREC *dest, COMGOREC *source)
{
    int i, rv = 0;
    QUAD *dtail,  *stail;
    dtail = dest->tail->back;
    stail = source->tail->back;
    for (i = 0; i < dest->size; i++)
    {
        /* Ignoring block and line statements */
        while (dtail && (dtail->dc.opcode == i_block || dtail->dc.opcode ==
            i_line || dtail->dc.opcode == i_passthrough || dtail->dc.opcode ==
            i_label))
            dtail = dtail->back;
        while (stail && (stail->dc.opcode == i_block || stail->dc.opcode ==
            i_line || stail->dc.opcode == i_passthrough))
            stail = stail->back;
        /* Check for match */
        if (!dtail || !stail)
            return rv;
        if (stail->dc.opcode != dtail->dc.opcode)
            return rv;
        if (!equalquad(stail, dtail))
            return rv;
        rv++;
        dtail = dtail->back;
        stail = stail->back;
    }
    return rv;
}

/* Run through a single label lookin for common expressions
 *
 * Note that this routine may generate new labels, which will eventually
 * themselves be scanned
 */
static void runone(QUAD *q, int label)
{
    int lcount = 0, lbl;
    COMGOREC *t, work;
    QUAD *h = q;
    /* Collect all goto statements */
    while (h && ((t = find_goto(h, label)) != 0))
    {
        matches[lcount++] = t;
        h = t->tail->fwd;
    }
    /* Quit if none */
    if (lcount < 2)
        return ;
    do
    {
        int i, count;
        lbl =  - 1;

        /* Scan for matches and insert them.
         * we WILL be scanning backwords to keep jumps forward as often as
         * possible
         */
        t = matches[--lcount];
        for (i = 0; i < lcount; i++)
        {
            if ((count = compare(t, matches[i])) != 0)
            {
                int qcount = count;
                work =  *t;
                /* Create descriptors for the match */
                if (count != matches[i]->size)
                {
                    matches[i]->head = matches[i]->tail;
                    while (count--)
                    {
                        matches[i]->head = matches[i]->head->back;
                        while (matches[i]->head->dc.opcode == i_block ||
                            matches[i]->head->dc.opcode == i_line || matches[i]
                            ->head->dc.opcode == i_label || matches[i]->head
                            ->dc.opcode == i_passthrough)
                            matches[i]->head = matches[i]->head->back;
                    }
                }
                if (qcount != work.size)
                {
                    work.head = work.tail;
                    while (qcount--)
                    {
                        work.head = work.head->back;
                        while (work.head->dc.opcode == i_block || work.head
                            ->dc.opcode == i_line || work.head->dc.opcode ==
                            i_label || work.head->dc.opcode == i_passthrough)
                            work.head = work.head->back;
                    }
                }
                /* Make sure we are not at the start */
                if (matches[i]->head->back)
                {
                    QUAD *gq = xalloc(sizeof(QUAD));
                    QUAD *lq = xalloc(sizeof(QUAD));
                    lbl = nextlabel++;
                    /* insert a new goto statement */
                    gq->dc.opcode = i_goto;
                    gq->dc.right = gq->ans = gq->dc.left = 0;
                    gq->dc.label = lbl;
                    gq->fwd = matches[i]->tail->fwd;
                    gq->back = matches[i]->head->back;
                    /* dead code deletion */
                    matches[i]->head->back->fwd = gq;
                    matches[i]->tail->fwd->back = gq;
                    matches[i--] = matches[--lcount];
                    /* put in a label */
                    lq->dc.opcode = i_label;
                    lq->dc.right = lq->ans = lq->dc.left = 0;
                    lq->dc.label = lbl;
                    lq->fwd = work.head;
                    lq->back = work.head->back;
                    work.head->back->fwd = lq;
                    work.head->back = lq;
                }
            }
        }
    }
    while (lcount > 1 && lbl !=  - 1)
        ;
}

/* Main routine for common code deletion */
void commondelete(QUAD *quad)
{
    int label;
    for (label = firstlabel; label < nextlabel; label++)
    {
        matches = oalloc((nextlabel - firstlabel) *sizeof(COMGOREC*));
        runone(quad, label);
    }
}
