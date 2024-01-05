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
 * iblock.c
 *
 * find blocks and do LCSE optimization.  Subroutine calls end blocks;
 * this allows the pointer analysis and GCSE routines to find a wider
 * range of CSEs.  LCSE pass creates lots of dead variables so,
 * we need another optimization to get rid of these later...
 *
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

extern int intsize;
extern int prm_optdead, prm_locsub;
TABLE tempsyms;
QUAD *intermed_head,  *intermed_tail;
BLOCKLIST *blockhead,  *blocktail;
int blocknum;
QUAD *intermed_head,  *intermed_tail;
DAGLIST **name_hash = 0,  **ins_hash = 0;
static short wasgoto = FALSE;
static short nodag = TRUE;

static void add_intermed(QUAD *new);
void gen_nodag(int op, IMODE *res, IMODE *left, IMODE *right);

int equalnode(ENODE *node1, ENODE *node2)
/*
 *      equalnode will return 1 if the expressions pointed to by
 *      node1 and node2 are equivalent.
 */
{
    if (node1 == 0 || node2 == 0)
        return 0;
    if (node1->nodetype != node2->nodetype)
        return 0;
    if (natural_size(node1) != natural_size(node2))
        return 0;
    switch (node1->nodetype)
    {
        case en_nalabcon:
            return node1->v.sp == node2->v.sp;
        case en_labcon:
        case en_autocon:
        case en_nacon:
        case en_napccon:
        case en_autoreg:
        case en_absacon:
            return node1->v.i == node2->v.i;
        default:
            return (lvalue(node1) && equalnode(node1->v.p[0], node2->v.p[0]));
        case en_icon:
        case en_lcon:
        case en_lucon:
        case en_iucon:
        case en_ccon:
        case en_boolcon:
        case en_cucon:
        case en_llcon:
        case en_llucon:
            return node1->v.i == node2->v.i;
        case en_rcon:
        case en_fcon:
        case en_lrcon:
        case en_rimaginarycon:
        case en_fimaginarycon:
        case en_lrimaginarycon:
            return node1->v.f == node2->v.f;
        case en_rcomplexcon:
        case en_fcomplexcon:
        case en_lrcomplexcon:
            return node1->v.c.r == node2->v.c.r && node1->v.c.i == node2->v.c.i;
        case en_tempref:
            return ((SYM*)(node1->v.sp))->value.i == ((SYM*)(node2->v.sp))
                ->value.i;
    }
}

//-------------------------------------------------------------------------

int equalimode(IMODE *ap1, IMODE *ap2)
/*
 * return true if the imodes match
 */
{
    if (!ap1 || !ap2)
        return FALSE;
    if (ap1->mode && ap1->mode != ap2->mode)
        return FALSE;
    //   if (ap1->size && ap1->size != ap2->size)
    //      return FALSE;
    switch (ap1->mode)
    {

        case i_none:
        case i_rsp:
        case i_rret:
        case i_rlink:
        case i_rstruct:
        case i_rstructstar:
            return FALSE;
        default:
            if ((ap1->offset->cflags | ap2->offset->cflags) &DF_VOL)
                return FALSE;
            return equalnode(ap1->offset, ap2->offset);
    }
}

//-------------------------------------------------------------------------

short dhash(BYTE *str, int len)
/*
 * hashing for dag nodes
 */
{
    int i;
    unsigned short v = 0;
    for (i = 0; i < len; i++)
    {
        v = (v << 3) + (v >> 13) + (v >> 3) + (v << 13);
        v += str[i];
    }
    return v % DAGSIZE;
}

//-------------------------------------------------------------------------

QUAD *LookupNVHash(BYTE *key, int size, DAGLIST **table)
{
    int hashval = dhash(key, size);
    DAGLIST *list = table[hashval];
    while (list)
    {
        if (!memcmp(key, list->key, size))
            return list->rv;
        list = list->link;
    }
    return 0;
}

//-------------------------------------------------------------------------

static void ReplaceHash(QUAD *rv, BYTE *key, int size, DAGLIST **table)
{
    int hashval = dhash(key, size);
    DAGLIST **list = &table[hashval],  **flist = list;
    DAGLIST *new;
    while (*list)
    {
        if (!memcmp(key, (*list)->key, size))
        {
            (*list)->rv = rv;
            return ;
        }
        list =  *list;
    }
    new = oalloc(sizeof(DAGLIST));
    new->rv = rv;
    new->key = key;
    new->link =  *flist;
    *flist = new;
}

//-------------------------------------------------------------------------

static void add_intermed(QUAD *new)
/*
 *      add the icode quad to the icode list
 */
{
    if (intermed_head == 0)
    {
        intermed_head = intermed_tail = new;
        new->fwd = 0;
        new->back = 0;
    }
    else
    {
        new->fwd = 0;
        new->back = intermed_tail;
        intermed_tail->fwd = new;
        intermed_tail = new;
    }
}

//-------------------------------------------------------------------------

QUAD *liveout(QUAD *node)
{
    QUAD *outnode;
    outnode = xalloc(sizeof(QUAD));
    outnode->dc.opcode = node->dc.opcode;
    outnode->ans = node->ans;
    outnode->dc.v = node->dc.v;
    outnode->dc.label = node->dc.label;
    if (node->livein &IM_LIVELEFT)
        outnode->dc.left = node->dc.left;
    else
        outnode->dc.left = ((QUAD*)node->dc.left)->ans;
    if (node->livein &IM_LIVERIGHT)
        outnode->dc.right = node->dc.right;
    else
        outnode->dc.right = ((QUAD*)node->dc.right)->ans;
    return outnode;
}

/* See if is a const which is an address */
static int addr_const(IMODE *val)
{
    switch (val->offset->nodetype)
    {
        case en_autocon:
        case en_autoreg:
        case en_nalabcon:
        case en_nacon:
        case en_absacon:
            return 1;
        default:
            return 0;
    }
}
int ToQuadConst(IMODE **im)
{
	if (*im && (*im)->mode == i_immed) {
		QUAD *rv, temp ;
		memset(&temp,0,sizeof(temp));
		if (isintconst((*im)->offset->nodetype)) {
			temp.dc.opcode = i_icon;
			temp.dc.v.i = (*im)->offset->v.i;
		} else if (isfloatconst((*im)->offset->nodetype)) {
			temp.dc.opcode = i_fcon;
			temp.dc.v.f = (*im)->offset->v.f;
		} else if (isimaginaryconst((*im)->offset->nodetype)) {
			temp.dc.opcode = i_imcon;
			temp.dc.v.f = (*im)->offset->v.f;
		} else if (iscomplexconst((*im)->offset->nodetype)) {
			temp.dc.opcode = i_cxcon;
			temp.dc.v.c.r = (*im)->offset->v.c.r;
			temp.dc.v.c.i = (*im)->offset->v.c.i;
		} else {
//			DIAG("ToQuadConst:  unknown constant type");
			// might get address constants here
			return 0;
		}
	    rv = LookupNVHash(&temp, DAGCOMPARE, ins_hash);
		if (!rv) {
			rv = oalloc(sizeof(QUAD));
			*rv = temp ;
			rv->ans = tempreg(ISZ_NONE,0);
	    	add_intermed(rv);
	        ReplaceHash(rv, rv, DAGCOMPARE, ins_hash);
    		wasgoto = FALSE;
		} 
		*im = rv ;
		return 1 ; // it is now not a livein node any more
	}
	return 0;
}
/*
 * this is the primary local CSE subroutine
 *
 * The steps are:
 *  1) replace any named operand IMODE with the QUAD it refers to.  Otherwise
 *     if there is none set the livein flag for the IMODE so it
 *     can be distinguished from quads later
 *  2) constant fold and algebra
 *  3) look up the resulting quad in the CSE table and replace the
 *     instruction with an assigment if it is there, otherwise
 *     enter the quad in the CSE table
 *  4) convert the resulting quad QUAD members back to IMODE members
 *  5) save the final QUAD back to the names table for use in step 1 of
 *     subsequent instructions
 *     
 * if local CSEs are disabled we simply don't put anything in the lookup
 * tables.
 *
 * if a volatile var is on the right-hand side we don't put the
 * CSE in the CSE table.
 *
 * if a variable on the left is volatile we create a temp variable
 * for the result and then assign the volatile to the temp.  This
 * prevents us from using a volatile as an intermediate.
 *
 * by the time we get here, casts will have been assigned unique
 * operand values so they won't be CSE'd.  I may fix it some time in
 * the future.
 *
 * PTR indirections won't be CSE'd because the IMODE structs will be
 * regenerated each time a pointer is indirected.  This is what we want
 * since we can't be sure two pointers don't point to the same thing.
 * if we implement the RESTRICT keyword this will have to change.
 *
 * Yes this does rely on the IMODE addresses being the same each
 * time a variable is used.  IEXPR makes sure the IMODE addresses are
 * constant for each global and local variable, both when used as an
 * address and when used as data.
 */
static void add_dag(QUAD *new)
{
    QUAD *node;
    QUAD *outnode;
    IMODE *t;
    TYP *tp;
    int lptr = 0, rptr = 0, larr = 0, rarr = 0, i;
    /* if the left-hand side is volatile, insert a temp so we can keep
     * going with CSEs
     */
    if (new->ans && new->ans->offset && (new->ans->offset->cflags &DF_VOL))
    {
        QUAD *tquad;
        IMODE *treg;
        treg = tempreg(new->ans->size, 0);
        tquad = xalloc(sizeof(QUAD));
        tquad->ans = new->ans;
        tquad->dc.left = treg;
        tquad->dc.opcode = i_assn;
        new->ans = treg;
        add_dag(new);
        new = tquad;
    }
    if (!name_hash)
    {
        name_hash = oalloc(sizeof(void*) * DAGSIZE);
        ins_hash = oalloc(sizeof(void*) * DAGSIZE);
    }

    /* Transform the quad structure members from imodes to quads */
    node = LookupNVHash(&new->dc.left, sizeof(void*), name_hash);
    if (node)
    {
        if (node->ans->size == new->dc.left->size || node->ans->size ==  - new
            ->dc.left->size)
            new->dc.left = node;
        else
            new->livein |= IM_LIVELEFT;
    }
    else {
		if (!ToQuadConst(&new->dc.left))
        	new->livein |= IM_LIVELEFT;
	}
    node = LookupNVHash(&new->dc.right, sizeof(void*), name_hash);
    if (node)
    {
        if (node->ans->size == new->dc.right->size || node->ans->size ==  - new
            ->dc.right->size)
            new->dc.right = node;
        else
            new->livein |= IM_LIVERIGHT;
    }
    else {
		if (!ToQuadConst(&new->dc.right))
	        new->livein |= IM_LIVERIGHT;
	}

    /* constant folding, now! */
    ConstantFold(new);
	
    /* Now replace the CSE or enter it into the table */
    node = LookupNVHash(new, DAGCOMPARE, ins_hash);
    if (!node)
    {
        if (prm_locsub)
        /* take care of volatiles by not registering volatile expressions
         * in the CSE table.  At this point a temp var will already exist
         * in the case that a volatile exists as the answer.
         */
            if (new->ans && !(new->ans->offset && (new->ans->offset->cflags
                &DF_VOL) || (new->dc.left && (new->livein & IM_LIVELEFT) && new->dc.left->offset && (new
                ->dc.left->offset->cflags &DF_VOL)
                ) || (new->dc.right && (new->livein & IM_LIVERIGHT) && new->dc.right->vol)))
				if (new->ans->mode == i_direct && new->ans->offset->nodetype == en_tempref)
	                ReplaceHash(new, new, DAGCOMPARE, ins_hash);
        /* convert back to a quad structure and generate code */
        node = new;
        outnode = liveout(node);
        add_intermed(outnode);
    }
    else
    {
        outnode = xalloc(sizeof(QUAD));
        outnode->dc.opcode = i_assn;
        outnode->ans = new->ans;
	    outnode->dc.left = node->ans;
        add_intermed(outnode);
    }
    /* Save the new node structure for later lookups 
     * always save constants even when no LCSE is to be done because it
     * is needed for constant-folding in subsequent instructions
     */
    if (new->ans && (prm_locsub || node->dc.opcode == i_icon || node->dc.opcode
        == i_fcon || node->dc.opcode == i_imcon || node->dc.opcode == i_cxcon))
        ReplaceHash(node, &new->ans, sizeof(void*), name_hash);
}

//-------------------------------------------------------------------------

static void flush_dag(void)
{
    int i;
    if (name_hash)
    {
        memset(name_hash, 0, sizeof(void*) * DAGSIZE);
        memset(ins_hash, 0, sizeof(void*) * DAGSIZE);
    }
}

//-------------------------------------------------------------------------

void dag_rundown(void)
{
    name_hash = ins_hash = 0;
}

//-------------------------------------------------------------------------

void addblock(int val)
/*
 * create a block
 */
{
    BLOCKLIST *list;
    BLOCK *block;

    QUAD *q;
    if (blockhead)
    {
        blocktail->block->tail = intermed_tail;
    }
    switch (val)
    {
        case i_ret:
        case i_rett:
            return ;
    }

    /* block statement gets included */
    q = xalloc(sizeof(QUAD));
    q->dc.opcode = i_block;
    q->ans = q->dc.right = 0;
    q->dc.label = blocknum;
    add_intermed(q);
    /* now make a basic block and add to the blocklist */
    block = xalloc(sizeof(BLOCK));
    list = xalloc(sizeof(BLOCK));
    list->link = 0;
    list->block = block;
    block->flowback = 0;
    block->flowfwd = 0;
    block->blocknum = blocknum++;
    if (blockhead == 0)
        blockhead = blocktail = list;
    else
        blocktail = blocktail->link = list;
    block->head = intermed_tail;
}

/*
 * intermed code poitners 
 */
void gen_label(int labno)
/*
 *      add a compiler generated label to the intermediate list.
 */
{
    QUAD *new;
    flush_dag();
    if (!wasgoto)
        addblock(i_label);
    wasgoto = FALSE;
    new = xalloc(sizeof(QUAD));
    new->dc.opcode = i_label;
    new->dc.label = labno;
    add_intermed(new);
}

//-------------------------------------------------------------------------

void gen_icode(int op, IMODE *res, IMODE *left, IMODE *right)
/*
 *      generate a code sequence into the peep list.
 */
{
    QUAD *new;
    switch (op)
    {
        case i_ret:
        case i_rett:
            flush_dag();
            break;
    }
    new = xalloc(sizeof(QUAD));
    new->dc.opcode = op;
    new->dc.left = left;
    new->dc.right = right;
    new->ans = res;
    add_dag(new);
    switch (op)
    {
        case i_ret:
        case i_rett:
            flush_dag();
            addblock(op);
    }
    wasgoto = FALSE;

}

//-------------------------------------------------------------------------

void gen_iiconst(IMODE *res, LLONG_TYPE val)
/*
 *      generate an integer constant sequence into the peep list.
 */
{
    QUAD *new;
    new = xalloc(sizeof(QUAD));
    new->dc.opcode = i_icon;
    new->dc.v.i = val;
    new->ans = res;
    add_dag(new);
    wasgoto = FALSE;
}

//-------------------------------------------------------------------------

void gen_ifconst(IMODE *res, long double val)
/*
 *      generate an integer constant sequence into the peep list.
 */
{
    QUAD *new;
    new = xalloc(sizeof(QUAD));
    new->dc.opcode = i_fcon;
    new->dc.v.f = val;
    new->ans = res;
    add_dag(new);
    wasgoto = FALSE;
}

//-------------------------------------------------------------------------

void gen_igoto(long label)
/*
 *      generate a code sequence into the peep list.
 */
{
    QUAD *new;
    flush_dag();
    new = xalloc(sizeof(QUAD));
    new->dc.opcode = i_goto;
    new->dc.left = new->dc.right = new->ans = 0;
    new->dc.label = label;
    add_intermed(new);
    addblock(i_goto);
    wasgoto = TRUE;
}

//-------------------------------------------------------------------------

void gen_idc(long label)
/*
 *      generate a code sequence into the peep list.
 */
{
    QUAD *new;
    flush_dag();
    new = xalloc(sizeof(QUAD));
    new->dc.opcode = i_dc;
    new->dc.left = new->dc.right = new->ans = 0;
    new->dc.label = label;
    add_intermed(new);
    wasgoto = FALSE;
}

//-------------------------------------------------------------------------

void gen_data(int val)
{
    QUAD *new;
    flush_dag();
    new = xalloc(sizeof(QUAD));
    new->dc.opcode = i_genword;
    new->dc.left = new->dc.right = new->ans = 0;
    new->dc.label = val;
    add_intermed(new);
    wasgoto = FALSE;
}

//-------------------------------------------------------------------------

void gen_icgoto(int op, long label, IMODE *left, IMODE *right)
/*
 *      generate a code sequence into the peep list.
 */
{
    QUAD *new;

    new = xalloc(sizeof(QUAD));
    new->dc.opcode = op;
    new->dc.left = left;
    new->dc.right = right;
    new->ans = 0;
    new->dc.label = label;
    add_dag(new);
    flush_dag();
    addblock(op);
    wasgoto = TRUE;
}

//-------------------------------------------------------------------------

void gen_igosub(int op, IMODE *left)
/*
 *      generate a code sequence into the peep list.
 */
{
    QUAD *new;

    new = xalloc(sizeof(QUAD));
    new->dc.opcode = op;
    new->dc.left = left;
    new->dc.right = 0;
    new->ans = 0;
    new->dc.label = 0;
    add_dag(new);
    flush_dag();
    addblock(op);
    wasgoto = TRUE;
}

//-------------------------------------------------------------------------

void gen_icode2(int op, IMODE *res, IMODE *left, IMODE *right, int label)
/*
 *      generate a code sequence into the peep list.
 */
{
    QUAD *new;
    new = xalloc(sizeof(QUAD));
    new->dc.opcode = op;
    new->dc.left = left;
    new->dc.right = right;
    new->ans = res;
    new->dc.label = label;
    add_intermed(new);
    wasgoto = FALSE;
}

//-------------------------------------------------------------------------

void gen_line(SNODE *stmt)
/*
 * generate a line number statement 
 */
{
    QUAD *new;
    new = xalloc(sizeof(QUAD));
    new->dc.opcode = i_line;
    new->dc.left = (IMODE*)stmt->label;
    new->dc.label = (long)stmt->exp;
    add_intermed(new);
}

//-------------------------------------------------------------------------

void gen_asm(SNODE *stmt)
/*
 * generate an ASM statement
 */
{
    QUAD *new;
    new = xalloc(sizeof(QUAD));
    new->dc.opcode = i_passthrough;
    new->dc.left = (IMODE*)stmt->exp;
    flush_dag();
    add_intermed(new);
}

//-------------------------------------------------------------------------

void gen_nodag(int op, IMODE *res, IMODE *left, IMODE *right)
/*
 *      generate a code sequence into the peep list.
 */
{
    QUAD *new;
    new = xalloc(sizeof(QUAD));
    new->dc.opcode = op;
    new->dc.left = left;
    new->dc.right = right;
    new->ans = res;
    add_intermed(new);
    wasgoto = FALSE;

}
