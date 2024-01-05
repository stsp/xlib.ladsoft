/* 
XRC Resource Compiler
Copyright 1997, 1998 Free Software Foundation, Inc.
Written by Ian Lance Taylor, Cygnus Support.
Copyright 2006-2011 David Lindauer.

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

You may contact the author of this derivative at:
	mailto::camille@bluegrass.net
 */
/* Handles symbol tables 
 */
#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include "utype.h"
#include "umem.h"
#include "preproc.h"
#define ROTR(x,bits) (((x << (16 - bits)) | (x >> bits)) & 0xffff)
#define ROTL(x,bits) (((x << bits) | (x >> (16 - bits))) & 0xffff)
#define HASHTABLESIZE 1023

extern int prm_cplusplus, prm_cmangle;
HASHREC **defhash = 0;

TABLE defsyms;

void symini(void)
{
    defsyms.head = defsyms.tail = 0;
    if (!defhash)
    {
        defhash = (HASHREC **)malloc(HASHTABLESIZE *sizeof(HASHREC*));
    }
    memset(defhash, 0, HASHTABLESIZE *sizeof(HASHREC*));
}

/* Sym tab hash function */
static unsigned int ComputeHash(char *string, int size)
{
    unsigned int len = strlen(string), rv;
    char *pe = len + string;
    unsigned char blank = ' ';

    rv = len | blank;
    while (len--)
    {
        unsigned char cback = (unsigned char)(*--pe) | blank;
        rv = ROTR(rv, 2) ^ cback;
    }
    return (rv % size);
}

/* Add a hash item to the table */
HASHREC *AddHash(HASHREC *item, HASHREC **table, int size)
{
    int index = ComputeHash(item->key, size);
    HASHREC **p;

    item->link = 0;

    if (*(p = &table[index]))
    {
        HASHREC *q =  *p,  *r =  *p;
        while (q)
        {
            r = q;
            if (!strcmp(r->key, item->key))
                return (r);
            q = q->link;
        }
        r->link = item;
    }
    else
        *p = item;
    return (0);
}

/*
 * Find something in the hash table
 */
HASHREC **LookupHash(char *key, HASHREC **table, int size)
{
    int index = ComputeHash(key, size);
    HASHREC **p;

    if (*(p = &table[index]))
    {
        HASHREC *q =  *p;
        while (q)
        {
            if (!strcmp(q->key, key))
                return (p);
            p =  *p;
            q = q->link;
        }
    }
    return (0);
}

/*
 * Some tables use hash tables and some use linked lists
 * This is the global symbol search routine
 */
SYM *basesearch(char *na, TABLE *table, int checkaccess)
{
    SYM *thead = table->head;
    SYM **p;
    p = ((SYM **)LookupHash(na, defhash, HASHTABLESIZE));
    if (p)
    {
        p =  *p;
    }
    return (SYM*)p;
}

//-------------------------------------------------------------------------

SYM *search(char *na, TABLE *table)
{
    return basesearch(na, table, 1);
}

/* The global symbol insert routine */
void insert(SYM *sp, TABLE *table)

{
    AddHash(sp, defhash, HASHTABLESIZE);
}
