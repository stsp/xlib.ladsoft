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
 * handle intrinsics
 */
#include <stdio.h>
#include <string.h>
#include "lists.h"
#include "expr.h"
#include "c.h"
#include "ccerr.h"
#include "lists.h"
#include "gen386.h"

#define KWINTRINSHASH 31

extern int prm_cmangle, prm_intrinsic;
extern long nextlabel;

static HASHREC **hashtable = 0;
static INTRINSICS intrinsic_table[] = 
{
    {
        0, "abs", 1
    }
    , 
    {
        0, "div", 2
    }
    , 
    {
        0, "ldiv", 3
    }
    , 
    {
        0, "_rotl", 4
    }
    , 
    {
        0, "_rotr", 5
    }
    , 
    {
        0, "memcmp", 6
    }
    , 
    {
        0, "memcpy", 7
    }
    , 
    {
        0, "memmove", 8
    }
    , 
    {
        0, "memset", 9
    }
    , 
    {
        0, "strcat", 10
    }
    , 
    {
        0, "strcmp", 11
    }
    , 
    {
        0, "strcpy", 12
    }
    , 
    {
        0, "strlen", 13
    }
    , 
    {
        0, "strncat", 14
    }
    , 
    {
        0, "strncmp", 15
    }
    , 
    {
        0, "strncpy", 16
    }
    , 
    {
        0, "memchr", 17
    }
    , 
    {
        0, "strchr", 18
    }
    , 
    {
        0, "strrchr", 19
    }
    , 
    {
        0, "_crotl", 28
    }
    , 
    {
        0, "_crotr", 29
    }
    , 
    {
        0, 0, 0
    }
    , 
};
void IntrinsIni(void)
{
    INTRINSICS *q = intrinsic_table;
    if (!hashtable)
    {
        hashtable = (INTRINSICS **)malloc(KWINTRINSHASH *sizeof(INTRINSICS*));
        memset(hashtable, 0, KWINTRINSHASH *sizeof(INTRINSICS*));
        if (!hashtable)
            fatal("Out of memory");
        while (q->word)
        {
            AddHash(q, hashtable, KWINTRINSHASH);
            q++;
        }
    }
}

//-------------------------------------------------------------------------

void SearchIntrins(SYM *sp)
{
    HASHREC **q;
    char *p = sp->name;
    if (!prm_intrinsic)
    {
        sp->tp->cflags &= ~DF_INTRINS;
        return ;
    }
    if (prm_cmangle)
        p++;
    q = LookupHash(p, hashtable, KWINTRINSHASH);
    if (!q)
    {
        gensymerror(ERR_NOINTRINSFOUND, sp->name);
        sp->tp->cflags &= ~DF_INTRINS;
        return ;
    }
    sp->value.i = ((INTRINSICS*)(*q))->val;
}

//-------------------------------------------------------------------------

static int oneparm(ENODE *parms)
{
    if (!parms)
        return 0;
    return 1;
}

//-------------------------------------------------------------------------

static int twoparms(ENODE *parms)
{
    if (!parms || !parms->v.p[1])
        return 0;
    return 1;
}

//-------------------------------------------------------------------------

static int threeparms(ENODE *parms)
{
    if (!parms || !parms->v.p[1] || !parms->v.p[1]->v.p[1])
        return 0;
    return 1;
}

//-------------------------------------------------------------------------

AMODE *HandleIntrins(ENODE *node, int novalue)
{
    ENODE *parms = node->v.p[1]->v.p[1]->v.p[0];
    AMODE *ap1,  *ap2,  *ap3;
    SYM *sp;
    int xchg = FALSE;
    if (prm_intrinsic && (node->v.p[1]->v.p[0]->nodetype == en_nacon || node
        ->v.p[1]->v.p[0]->nodetype == en_napccon) && ((sp = node->v.p[1]
        ->v.p[0]->v.sp)->tp->cflags &DF_INTRINS)){}
    if (node->v.p[1]->v.p[0]->nodetype == en_nacon || node->v.p[1]->v.p[0]
        ->nodetype == en_napccon)
        node->v.p[1]->v.p[0]->v.sp->tp->cflags &= ~DF_INTRINS;
    return 0;
}
