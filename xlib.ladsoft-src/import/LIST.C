/* 
IMPORT librarian
Copyright 2001-2011 David Lindauer.

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
#include "utype.h"
#include "umem.h"

/* Append data to a list */
void AppendToList(LIST **list, void *data)
{
    LIST **pos = list;
    while (*pos)
        pos = (LIST **) * pos;

    (*pos) = (LIST*)AllocateMemory(sizeof(LIST));
    (*pos)->data = data;
    (*pos)->link = 0;
}

/*
 * Unlink an element from a file list
 */
void *UnlinkFromList(LIST **list, LIST *loc)
{
    LIST **pos = list;
    void *rv;
    while (*pos && (*pos != loc))
        pos = (LIST **) * pos;

    rv = 0;
    if (*pos)
    {
        LIST *temp =  *pos;
        *pos = (*pos)->link;
        rv = temp->data;
        DeallocateMemory(temp);
    }
    return (rv);
}
