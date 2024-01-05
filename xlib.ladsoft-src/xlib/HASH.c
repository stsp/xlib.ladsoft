/* 
XLIB Librarian
Copyright 1997-2011 David Lindauer.

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

You may contact the author at:
	mailto::camille@bluegrass.net
 */
/*
 * Hash tables, conflicts are resolved by making each entry in the table
 * a linked list; if multiple sybols use the same hash they are just linked
 * together
 */

#include <string.h>
#include <stdio.h>
#include <memory.h>
#include "utype.h"
#include "hash.h"
#include "umem.h"


/*
 * Compute hash for use in internal hash tables.
 * Librarian uses a different hash algorithm and a different table
 * structure, see libs.c for details
 */
static uint ComputeHash(char *string, int size)
{
    uint len = strlen(string), rv;
    char *pe = len + string;
    const BYTE blank = ' ';

    rv = len | blank;
    while (len--)
    {
        BYTE cback = (*--pe) | blank;
        rv = ROTR(rv, 2) ^ cback;
    }
    return (rv % size);
}

/*
 * Add an entry to the hash table.  Return previous entry if it exists
 */
HASHREC *AddHash(HASHREC **table, HASHREC *item)
{
    int size = *(((uint*)table) - 1);
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
HASHREC **LookupHash(HASHREC **table, char *key)
{
    int size = *(((uint*)table) - 1);
    uint index = ComputeHash(key, size);
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
 * Create a hash table
 */
HASHREC **CreateHashTable(int size)
{
    uint *rv = (uint*)AllocateMemory(size *sizeof(HASHREC*) + sizeof(uint));
    *rv++ = size;
    memset(rv, 0, size *sizeof(HASHREC*));
    return ((HASHREC **)rv);
}

/* 
 * Delete a hash table
 */
void RemoveHashTable(HASHREC **t)
{
    DeallocateMemory(((uint*)t) - 1);
}
