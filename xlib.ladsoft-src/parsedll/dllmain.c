#include <windows.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>

#undef TRUE
#undef FALSE

#include "c_lex.h"
#include "c_list.h"
#include "c_parser.h"
#include "c_preprc.h"

#define DLLMAIN
#include "parser.h"

void dumpids(symtab_t *tab, sym_callback_t usercallback, void *userdata);


static file_callback_t filecallback;
static sym_callback_t usercallback;
static alloc_callback_t allocCallback;
static void *userdata;

BOOL WINAPI DllMain( HINSTANCE hinstDll,
                           DWORD fdwReason,
                           LPVOID plvReserved)
{
	if (fdwReason == DLL_PROCESS_ATTACH)
	{
		alloc_init();
	}
	return TRUE;
}

void __export WINAPI parse(char *filename, file_callback_t fcallback, 
					alloc_callback_t acallback,
					char *sysinc, char *userinc,
					int defineCount, char *defines[], sym_callback_t callback,
					void *data)
{
	int i;
	usercallback = callback;
	filecallback = fcallback;
	allocCallback = acallback;
	userdata = data;
	prm_searchpath = userinc;
	sys_searchpath = sysinc;
	preprocini();	
	for (i=0; i < defineCount; i++)
	{
		glbdefine(defines[i*2], defines[i*2 + 1]);
	}

	parser_main(filename);
}

int __export WINAPI enumTypes(type_t *type, sym_callback_t callback, void *data)
{
	if (type->symbols)
	{
		dumpids(type->symbols, callback, data);
		return 0;
	}
	return 1;
}
extern list_t identifiers;		/* head of the identifiers list */
extern list_t labels;
extern list_t types;

extern list_t exited_scopes;
extern list_t exited_types;

void dumpsym(symbol_t *sym, sym_callback_t usercallback, void *userdata)
{
	if (sym->object_type != OBJ_TYPEDEF_NAME)
		usercallback(sym->name, sym->file, sym->startline, sym->endline, 
				 sym->type, userdata);
}
void dumpids(symtab_t *tab, sym_callback_t usercallback, void *userdata)
{
	symbol_t *sym;
	assert(tab != 0);
	if (tab->hashed_symbols)
	{
		int i;
		for (i=0; i < HASH_SIZE; i++)
		{
			list_t *t2 = &tab->hashed_symbols[i];
			for (sym = (symbol_t *)list_first(t2);
				sym != 0;
				sym = (symbol_t *)list_next(t2, sym)) {
				dumpsym(sym, usercallback, userdata);
			}
		}
	}
	else
	{
		for (sym = (symbol_t *)list_first(&tab->symbols);
			sym != 0;
			sym = (symbol_t *)list_next(&tab->symbols, sym)) {
			dumpsym(sym, usercallback, userdata);
		}
	}

}
void dump(void)
{
	symtab_t *t = list_first(&identifiers);
	if (t)
		dumpids(t, usercallback, userdata);
	t = list_first(&exited_scopes);
	while (t)
	{
		dumpids(t, usercallback, userdata);
		t = list_next(&exited_scopes, t);
	}
}
int get_include_context(lex_env_t *env, int sys_inc)
{
	int rv = filecallback(env->le_filename, sys_inc,
					 &env->le_getline, &env->le_getline_arg, NULL, 0);
	if (allocCallback)
	{
		env->le_lines_used = (unsigned char *)allocCallback((void *)1000, TRUE);
		env->le_lines_used_max = 1000;
	}
	return rv;
}
int free_include_context(lex_env_t *env)
{
	return filecallback(NULL, FALSE, 0, &env->le_getline_arg, 
						env->le_lines_used, env->le_lines_used_max);
}
void * userAlloc(int size)
{
	return allocCallback((void *)size, TRUE);
}
void userFree(void *ptr)
{
	allocCallback(ptr, FALSE);
}
