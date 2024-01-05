/* 
CCIDE
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
#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
/* using a windows heap for the code completion because the way I've implemented it it uses a *lot* of memory,
 * and the borland runtime gets confused...
 */
int total_size;

static void *freeblocks;

#define ALLOC_SIZE 8192
typedef struct _allocblk
{
	struct _allocblk *next;
	int size;
	int offset;
	char data[1];
} ALLOCBLK;

static HANDLE heap;
void alloc_init(void)
{
	heap = HeapCreate(0, 2 * 1024 * 1024, 256  * 1024 * 1024);
	freeblocks = 0;
}
static void *newblk(int size)
{
	ALLOCBLK *bnew;
	if (size == ALLOC_SIZE && freeblocks)
	{
		bnew = freeblocks;
		freeblocks = bnew->next;
		memset(bnew->data, 0, bnew->size);
		bnew->offset = 0;
		bnew->next = 0;
	}
	else
	{
		bnew = HeapAlloc(heap, HEAP_ZERO_MEMORY, sizeof(ALLOCBLK) + size - 1);
		if (!bnew)
		{
//		xprint("no memory");
		ExtendedMessageBox("Error",0,"Out Of Memory");
		exit(1);
		}
		total_size += size;
		bnew->size = size;
		bnew->offset = 0;
	}
	return bnew;
}
void *allocate(int size, void **root1)
{
	void *rv ;
	ALLOCBLK *root = *root1;
	size += 3;
	size &= -4;
	if (!root)
	{
		*root1 = root = newblk(size > ALLOC_SIZE ? size : ALLOC_SIZE);
	}
	if (root->size - root->offset < size)
	{
		ALLOCBLK *bnew = newblk(size > ALLOC_SIZE ? size : ALLOC_SIZE);
		bnew->next = root;
		*root1 = root = bnew;
	}
	rv = &root->data[root->offset];
	root->offset += size;
	return rv;
}
void allocfreeall(void **root1)
{
	int donesome = FALSE;
	ALLOCBLK *root = *root1;
	*root1 = NULL;
	
	while (root)
	{
		ALLOCBLK *next = root->next;
#ifdef XXXXX
		if (root->size == ALLOC_SIZE)
		{
			root->next = freeblocks;
			freeblocks = root;
			memset(root->data,0x5a, root->size);
		}
		else
#endif
            memset(root->data, 0, root->size);
			total_size -= root->size;
			HeapFree(heap, 0, root);
		root = next;
		donesome = TRUE;
	}
	if (donesome)
		HeapCompact(heap, 0);
}
