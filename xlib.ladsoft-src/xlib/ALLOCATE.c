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
#include <stdlib.h>
#include <stdio.h>
#include <memory.h>
#include "cmdline.h"
#include "umem.h"

void OutOfMemory(void)
{
    fatal("Out Of Memory");
}

//-------------------------------------------------------------------------

void *ReallocateMemory(void *buf, uint size)
{
    void *ptr = realloc(buf, size);
    if (!ptr)
        OutOfMemory();
    return (ptr);
}

/*
 * Allocate memory.  Dump out if none
 */
void *AllocateMemory(uint size)
{
    void *ptr = malloc(size);
    if (!ptr)
        OutOfMemory();
    memset(ptr, 0, size);
    return (ptr);
}

/*
 * Deallocate memory.  Synonomous to FREE
 */
void DeallocateMemory(void *pos)
{
    free(pos);
}
