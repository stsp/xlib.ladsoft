#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>

#include "c_lex.h"
#include "c_list.h"
#include "c_parser.h"
#include "c_preprc.h"

extern list_t identifiers;		/* head of the identifiers list */
extern list_t labels;
extern list_t types;

extern list_t exited_scopes;
extern list_t exited_types;

void dumpsym(symbol_t *sym, int all)
{
	if (sym->data || all)
	{
		xprint("%s:%s(%d,%d):%s\n",object_name(sym->object_type), 
		   sym->file, sym->startline, sym->endline, sym->name);
		if (sym->data)
		{
			xprint("\t\t");
			sym = sym->data;
			while (sym->object_type & OBJ_POINTER)
			{
				xprint("ptr ");
				sym = sym->data;
			}
			xprint("%s: %s\n",object_name(sym->object_type), sym->name);		
		}
	}
}
void dumpids(symtab_t *tab, int all)
{
	symbol_t *sym;

	assert(tab != 0);
	for (sym = (symbol_t *)list_first(&tab->symbols);
		sym != 0;
		sym = (symbol_t *)list_next(&tab->symbols, sym)) {
		dumpsym(sym, all);
	}

}
void dumptype(symbol_t *sym)
{
	symtab_t *tab = sym->data;
	xprint("%s:%s************************\n",sym->file, sym->name);
	if (tab)
		dumpids(tab, TRUE);
	
}
void dumptypes(symtab_t *tab)
{
	symbol_t *sym;

	assert(tab != 0);
	for (sym = (symbol_t *)list_first(&tab->symbols);
		sym != 0;
		sym = (symbol_t *)list_next(&tab->symbols, sym)) {
		dumptype(sym);
	}
}
void dump(void)
{
	symtab_t *t = list_first(&identifiers);
	dumpids(t, FALSE);
	t = list_first(&exited_scopes);
	while (t)
	{
		dumpids(t, FALSE);
		t = list_next(&exited_scopes, t);
	}
	xprint("***********************types\n");
	t = list_first(&types);
	dumptypes(t);
	t = list_first(&exited_types);
	while (t)
	{
		dumptypes(t);
		t = list_next(&exited_types, t);
	}
}