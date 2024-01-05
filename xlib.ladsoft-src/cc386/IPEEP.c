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
 * icode peep optimizer
 *
 * Does branch optimizationns of various sorts
 */
#include <stdio.h>
#include "utype.h"
#include "lists.h"
#include "expr.h"
#include "c.h"
#include "iexpr.h"
#include "iopt.h"

extern int firstlabel;
extern int prm_optdead;
extern long nextlabel;
extern int tempnum;

static QUAD **golist; /* list of goto statements */
static QUAD **mapping;

static void psetbit (unsigned char *array, int bitnum)
{
	int mask = bitnum & 7;
	int byte = bitnum >> 3;
	array[byte] |= (1 << mask);
}
static int pisbit(unsigned char *array, int bitnum)
{
	int mask = bitnum & 7;
	int byte = bitnum >> 3;
	return array[byte] & (1 << mask);
}
/* get rid of unused temprefs */
static void scan_unused(QUAD *head)
{
	unsigned char *array = xalloc((tempnum+ 7)/8);
	QUAD *temp = head;
	mapping = xalloc(sizeof(QUAD *) * tempnum);
	while (temp) {
		if (temp->dc.left && (temp->dc.left->mode == i_ind || temp->dc.left->mode == i_direct) &&
			temp->dc.left->offset->nodetype == en_tempref)
				psetbit(array,temp->dc.left->offset->v.sp->value.i);
		if (temp->dc.right && (temp->dc.right->mode == i_ind || temp->dc.right->mode == i_direct) &&
			temp->dc.right->offset->nodetype == en_tempref)
				psetbit(array,temp->dc.right->offset->v.sp->value.i);
		if (temp->ans && (temp->ans->mode == i_ind) && temp->ans->offset->nodetype == en_tempref)
				psetbit(array,temp->ans->offset->v.sp->value.i);
		if (temp->dc.opcode == i_assn) {
			if (temp->ans->mode == i_direct && temp->ans->offset->nodetype == en_tempref)
				if (temp->dc.left->mode == i_direct && temp->dc.left->offset->nodetype == en_tempref)
					mapping[temp->ans->offset->v.sp->value.i] = temp;
//				else if (temp->dc.left->mode == i_icon || temp->dc.left->mode == i_fcon ||
//						temp->dc.left->mode == i_imcon || temp->dc.left->mode == i_cxcon)
//					mapping[temp->ans->offset->v.sp->value.i] = temp;
					
		}
		temp = temp->fwd;	
	}
	while (head) {
		int index ;
//		ReCastConst(head);
		if (head->dc.left && (head->dc.left->mode == i_ind || head->dc.left->mode == i_direct) &&
			head->dc.left->offset->nodetype == en_tempref) {
				index =head->dc.left->offset->v.sp->value.i;
				if (mapping[index] && mapping[index]->dc.left->mode == i_direct && 
						mapping[index]->dc.left->size == mapping[index]->ans->size) {
					enum i_adr a = head->dc.left->mode;
                    int bits = head->dc.left->bits, startbit = head->dc.left->startbit;
					head->dc.left = xalloc(sizeof(IMODE));
					*head->dc.left = *mapping[index]->dc.left;
					head->dc.left->mode = a;
                    head->dc.left->bits = bits, head->dc.left->startbit = startbit;
				}
			}
		if (head->dc.right && (head->dc.right->mode == i_ind || head->dc.right->mode == i_direct) &&
			head->dc.right->offset->nodetype == en_tempref) {
				index =head->dc.right->offset->v.sp->value.i;
				if (mapping[index] && mapping[index]->dc.left->mode == i_direct && 
						mapping[index]->dc.left->size == mapping[index]->ans->size) {
					enum i_adr a = head->dc.right->mode;
                    int bits = head->dc.right->bits, startbit = head->dc.right->startbit;
					head->dc.right = xalloc(sizeof(IMODE));
					*head->dc.right = *mapping[index]->dc.left;
					head->dc.right->mode = a;
                    head->dc.right->bits = bits, head->dc.right->startbit = startbit;
				}
			}
		if (head->ans && (head->ans->mode == i_ind) && head->ans->offset->nodetype == en_tempref) {
				index =head->ans->offset->v.sp->value.i;
				if (mapping[index] && mapping[index]->dc.left->mode == i_direct && 
						mapping[index]->dc.left->size == mapping[index]->ans->size) {
					enum i_adr a = head->ans->mode;
                    int bits = head->ans->bits, startbit = head->ans->startbit;
					head->ans = xalloc(sizeof(IMODE));
					*head->ans = *mapping[index]->dc.left;
					head->ans->mode = a;
                    head->ans->bits = bits, head->ans->startbit = startbit;
				}
			}
		if (head->ans && (head->ans->mode == i_ind || head->ans->mode == i_direct) &&
				head->ans->offset->nodetype == en_tempref) {
			index = head->ans->offset->v.sp->value.i;
			if (!pisbit(array,index) || mapping[index]
					&& mapping[index]->dc.left->size == mapping[index]->ans->size) {
				head->fwd->back = head->back;
				head->back->fwd = head->fwd;
			}
		}
		head = head->fwd;
	}
	
}
static void scan_gotos(QUAD *head)
/*
 * make a list of goto statements
 */
{
    while (head)
    {
        switch (head->dc.opcode)
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
            case i_coswitch:
                golist[head->dc.label - firstlabel] = head;
                break;
        }
        head = head->fwd;
    }
}

//-------------------------------------------------------------------------

static void kill_unreach(QUAD *head)
/*
 * kill unreachable code
 */
{
    while (head && head->fwd && head->fwd->dc.opcode != i_label)
    {
        if (head->fwd->dc.opcode != i_block && head->fwd->dc.opcode != i_blockstart && head->fwd->dc.opcode != i_blockend)
        {
            head->fwd->fwd->back = head;
            head->fwd = head->fwd->fwd;
        }
        else
            head = head->fwd;
    }
}

//-------------------------------------------------------------------------

void peepini(void){}
static void kill_brtonext(QUAD *head)
/*
 * branches to the next statement get wiped
 */
{
    QUAD *temp;
    while (TRUE)
    {
        start: switch (head->dc.opcode)
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
                temp = head->fwd;
                while (temp->dc.opcode == i_label || temp->dc.opcode == i_line 
                    || temp->dc.opcode == i_block || temp->dc.opcode ==
                    i_passthrough || temp->dc.opcode == i_blockstart || temp->dc.opcode == i_blockend)
                {
                    if (temp->dc.opcode == i_label && temp->dc.label == head
                        ->dc.label)
                    {
                        head->fwd->back = head->back;
                        head->back->fwd = head->fwd;
                        head = head->back;
                        while (head && head->back && (head->dc.opcode == i_line
                            || head->dc.opcode == i_block || head->dc.opcode ==
                            i_label || head->dc.opcode == i_passthrough))
                            head = head->back;
                        goto start;
                    }
                    temp = temp->fwd;
                }
            default:
                return ;
        }
    }
}

//-------------------------------------------------------------------------

void kill_labeledgoto(QUAD *head)
/*
 * if any goto goes to a label which is immediately followed by a goto,
 * replaces the original goto label with the new label annd possibly
 * get rid of the labeled goto if it is preceded by another goto
 */
{
    QUAD *newhead,  *back,  *gotoitem;
    newhead = head->fwd;
    /* look for following goto */
    while (newhead && (newhead->dc.opcode == i_line || newhead->dc.opcode ==
        i_block || newhead->dc.opcode == i_passthrough))
        newhead = newhead->fwd;
    if (newhead->dc.opcode != i_goto)
        return ;
    /* got one, look for the ref to it */
    gotoitem = golist[head->dc.label - firstlabel];
    if (!gotoitem || gotoitem->dc.label != head->dc.label)
        return ;
    golist[head->dc.label - firstlabel] = 0;
    gotoitem->dc.label = newhead->dc.label;
    /* got one- kill unreachable code and a possible brtonext that was genned */
    if (gotoitem->dc.opcode == i_goto)
    {
        kill_unreach(gotoitem);
        kill_brtonext(gotoitem);
    }
    /* now see if was preceded by a goto */
    back = head->back;
    while (back && (back->dc.opcode == i_line || back->dc.opcode == i_block ||
        back->dc.opcode == i_passthrough))
        back = back->back;
    /* yep- get rid of the label and the goto */
    if (back->dc.opcode == i_goto || back->dc.opcode == i_dc)
    {
        head->back->fwd = newhead->fwd;
        newhead->fwd->back = head->back;
    }
}

//-------------------------------------------------------------------------

void kill_jumpover(QUAD *head)
/*
 * Conditionnal jumps over gotos get squashed here
 */
{
    int newtype;
    QUAD *newhead = head->fwd;
    /* skip to next statement */
    while (newhead && newhead->dc.opcode == i_block || newhead->dc.opcode ==
        i_line || newhead->dc.opcode == i_passthrough)
        newhead = newhead->fwd;
    if (!newhead)
        return ;
    if (newhead->dc.opcode == i_goto)
    {
        /* if it was a goto, swap the conditional type and 
         * put the new label in the goto statement 
         */
        head->dc.label = newhead->dc.label;
        head->fwd = newhead->fwd;
        newhead->fwd->back = head;
        switch (head->dc.opcode)
        {
            case i_jc:
                newtype = i_jnc;
                break;
            case i_jnc:
                newtype = i_jc;
                break;
            case i_jbe:
                newtype = i_ja;
                break;
            case i_ja:
                newtype = i_jbe;
                break;
            case i_je:
                newtype = i_jne;
                break;
            case i_jne:
                newtype = i_je;
                break;
            case i_jge:
                newtype = i_jl;
                break;
            case i_jg:
                newtype = i_jle;
                break;
            case i_jle:
                newtype = i_jg;
                break;
            case i_jl:
                newtype = i_jge;
                break;
        }
        head->dc.opcode = newtype;
		while (1) {
		    while (newhead && newhead->dc.opcode == i_block || newhead->dc.opcode ==
    		    i_line || newhead->dc.opcode == i_passthrough) {
        		newhead = newhead->fwd;
    		}
			if (!newhead || newhead->dc.opcode != i_goto)
				break;
			newhead->back->fwd = newhead->fwd;
			newhead->fwd->back = newhead->back;
			newhead = newhead->fwd;
		}
	}
}

//-------------------------------------------------------------------------

void peep_icode(QUAD *head)
/*
 * ICODE peep main routine
 */
{
    golist = oalloc(sizeof(QUAD*)*(nextlabel - firstlabel));
	scan_unused(head);
    scan_gotos(head);
    while (head)
    {
        switch (head->dc.opcode)
        {
            case i_goto:
                kill_unreach(head);
                kill_brtonext(head);
                break;
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
                kill_brtonext(head);
                kill_jumpover(head);
                break;
            case i_label:
                kill_labeledgoto(head);
                break;
            case i_nop: /* just kill it */
                head->back->fwd = head->fwd;
                head->fwd->back = head->back;
                break;
        }
        head = head->fwd;
    }
}
