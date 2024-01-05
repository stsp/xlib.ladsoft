
#include <stdlib.h>
#include "c_alloc.h"

#define ALLOC_SIZE 8192
typedef struct _allocblk
{
	struct _allocblk *next;
	int size;
	int offset;
	char data[1];
} ALLOCBLK;

static ALLOCBLK *root;

static void *newblk(int size)
{
	ALLOCBLK *bnew = calloc(1, sizeof(ALLOCBLK) + size - 1);
	if (!bnew)
	{
		xprint("no memory");
		exit(1);
	}
	bnew->size = size;
	bnew->offset = 0;
	return bnew;
}
void *allocate(int size)
{
	void *rv ;
	size += 3;
	size &= -4;
	if (!root)
	{
		root = newblk(size > ALLOC_SIZE ? size : ALLOC_SIZE);
	}
	if (root->size - root->offset < size)
	{
		ALLOCBLK *bnew = newblk(size > ALLOC_SIZE ? size : ALLOC_SIZE);
		bnew->next = root;
		root = bnew;
	}
	rv = &root->data[root->offset];
	root->offset += size;
	return rv;
}

void freeall(void)
{
	while (root)
	{
		ALLOCBLK *next = root->next;
		free(root);
		root = next;
	}
}
