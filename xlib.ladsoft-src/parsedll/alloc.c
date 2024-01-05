#include <windows.h>
#include <stdlib.h>

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

static ALLOCBLK *root;

static HANDLE heap;
void alloc_init(void)
{
	heap = HeapCreate(0, 2 * 1024 * 1024, 64  * 1024 * 1024);
	root = 0;
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
	}
	else
	{
		bnew = HeapAlloc(heap, HEAP_ZERO_MEMORY, sizeof(ALLOCBLK) + size - 1);
		if (!bnew)
		{
//		xprint("no memory");
		exit(1);
		}
		total_size += size;
		bnew->size = size;
		bnew->offset = 0;
	}
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
void freeall(void **root1)
{
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
			total_size -= root->size;
			HeapFree(heap, 0, root);
		root = next;
	}
}
