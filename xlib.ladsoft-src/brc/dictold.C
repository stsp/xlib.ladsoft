/* 
Browse Linker
Copyright 2003-2011 David Lindauer.

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
#include <stdio.h>
#include <string.h>
#include "browse.h"
#include "cmdline.h"
#include "umem.h"
#include "dict.h"

typedef struct dictree
{
    struct dictree *next;
    struct dictree *lower;
    struct symdata *endsym;
    int offset;
    char key;
} DICTREE;

extern HASHREC **symhash;

static DICTREE *root;
static int dictpos;
void CalculateDictionary(void)
{
    int i;

    for (i = 0; i < HASH_TABLE_SIZE; i++)
    {
        struct symdata *p = symhash[i];
        while (p)
        {
            DICTREE *old;
            DICTREE **d = &root,  *m;
            char *s = p->name;
            while (*d &&  *s)
            {
                old =  *d;
                if ((*d)->key ==  *s)
                {
                    d = &(*d)->lower;
                    s++;
                } 
                else
                    d = &(*d)->next;
            }
            while (*s)
            {
                *d = AllocateMemory(sizeof(DICTREE));
                old =  *d;
                (*d)->key =  *s;
                d = &(*d)->lower;
                s++;
            }
            old->endsym = p;
            p = p->link;
        }

    }
}

//-------------------------------------------------------------------------

int WriteLevel(FILE *fil, DICTREE *level)
{
    DICTREE *d = level;
    int count = 0, rv;
    while (d)
    {
        d->offset = WriteLevel(fil, d->lower);
        count++;
        d = d->next;
    }
    rv = dictpos;
    fputc(count, fil);
    dictpos++;
    d = level;
    while (d)
    {
        fputc(d->key, fil);
        dictpos++;
        fwrite(&d->offset, 4, 1, fil);
        if (d->endsym)
            fwrite(&d->endsym->fileoffs, 1, 4, fil);
        else
            fwrite(&d->endsym, 1, 4, fil);
        dictpos += 8;
        d = d->next;
    }
    return rv;
}

//-------------------------------------------------------------------------

int WriteDictionary(FILE *file, int base)
{
    dictpos = base;
    return WriteLevel(file, root);
}
