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
#include <commctrl.h>
#include <stdio.h>
#include <stdlib.h>
#include "header.h"
#include "parser.h"
#include "codecomp.h"
#define FROMIDE

#ifdef FROMIDE
extern PROJECTITEM *workArea;
extern DWINFO *editWindows;
extern char szInstallPath[];
extern HANDLE editHeap;
extern CRITICAL_SECTION ewMutex;
#endif
void *allocate(int size, void **root1);
void allocfreeall(void **root1);
typedef struct fname
{
	struct fname *next;
	char name[260];
	void *allocblk;
} FNAME;

#define MAX_FNAME 400

static FNAME *base_fname;
static FNAME *fname_list[MAX_FNAME];
int fname_count;

typedef struct {
	char *base;
	char *current;
	EDITDATA *data;
} FileText;

char *DOSDEF = "__DOS__";
char *WIN32DEF = "__WIN32__";
char *defines[] = { 
	"???","",
	"_intrinsic","",
	"_import","",
	"_export","", 
	"__int64","long long",
	"__stdcall", "",
	"__cdecl", "" ,
	"__STDC__", "1",
	"__STDC_VERSION__", "199901L",
	"__CCDL__", "0.0",
	"__i386__", "",
	"__386__", "",
};
#define DEFINE_COUNT (sizeof(defines)/sizeof(char *) / 2)

static char prm_searchpath[10000] ;
static char sys_searchpath[256] ;

static char *localEmptyString = "";
void __stdcall typecallback(char *symname, char *filename, 
					   int linestart, int lineend, type_t *typeinfo,
					   void *userdata);
void __stdcall symcallback(char *symname, char *filename, 
					   int linestart, int lineend, type_t *typeinfo,
					   void *userdata);
int __stdcall file_callback(char *name, int sys_inc, void **func, void **arg,
							unsigned char *lines_used, int line_count);
void *__stdcall alloc_callback(void *ptr_or_size, int alloc);

void deleteSymsForFile(char *name);
static char *lookup_fname(char *name);

void CodeCompInit(void)
{
}
void *calloc_wrapper(int n, int size)
{
    if (!base_fname)
        return 0;
	return allocate(n * size, &base_fname->allocblk);
}
void free_wrapper(void *data)
{
#ifdef XXXXX
	int i;
	if (data == localEmptyString)
		return;
	free(data);
#endif
}
char *strdup_wrapper(const char *str)
{
    char *rv;
    if (!base_fname)
        return 0;
	rv = allocate(strlen(str)+1, &base_fname->allocblk);
	if (rv)
		strcpy(rv, str);
	else
		rv = localEmptyString;
	return rv;
}
#ifdef FROMIDE
PROJECTITEM *findItem(PROJECTITEM *list, char *name)
{
    PROJECTITEM *rv = NULL;
	while (list && !rv)
	{
        if (list->children)
            rv = findItem(list->children, name);
        if (!rv)
        {
			if (!stricmp(list->realName, name))
				rv = list;
		}
		list = list->next;
	}
	return rv;
}
static int ParseDefines(char *defs, char *arr[][2])
{
    int count = 0;
    char buf[5000];
    while (*defs)
    {
        int quote = 0;
        char buf[5000];
        char *p = buf;
        while (isspace(*defs)) defs++;
        while (*defs && (!isspace(*defs) || quote))
        {
            if (quote && *defs == quote)
                quote = 0, defs++;
            else if (!quote && *defs == '"')
                quote = *defs++;
            else
                *p++ = *defs++;
        }
        *p = 0;
        if (buf[0])
        {
            p = strchr(buf, '=');
            if (p)
            {
                *p = 0;
                arr[count][0] = strdup(buf);
                arr[count][1] = strdup(p + 1);
            }
            else
            {
                arr[count][0] = strdup(buf);
                arr[count][1] = strdup("");
            }
            count++;
        }
    }
    return count;
}
int doparse(char *name)
{
	int i;
	int defineCount = DEFINE_COUNT;
	char **localDefines ;
    char *projDefines[400][2];
	int buildType;
#ifdef FROMIDE
	PROJECTITEM *proj = NULL;
	int l = strlen(name);
	if (workArea)
		proj= findItem(workArea, name);
	if (l < 3 || tolower(name[l-1]) != 'c' && name[l-2] != '.')
		return FALSE;
	if (proj)
	{
		char *p, buf[5000];
        PROJECTITEM *pi = proj;
        while (pi && pi->type != PJ_PROJ) pi = pi->parent;
        if (pi)
        {
    		strcpy(prm_searchpath, pi->realName);
    		p = strrchr(prm_searchpath,'\\');
    		if (!p)
    		{
    			p = strrchr(prm_searchpath, ':');
    			if (!p)
    				prm_searchpath[0] = 0;
    			else
    				p[1] = 0;
    		}
    		else
    			*p = 0;
            buf[0] = 0;
            PropGetString(proj, "__INCLUDE", buf, sizeof(buf));
    		if (buf[0])
    		{
    			strcat(prm_searchpath, ";");
    			strcat(prm_searchpath, buf);
    		}
            PropGetString(proj, "__DEFINE", buf, sizeof(buf));
            defineCount += ParseDefines(buf, projDefines);
		}
	}
	else
#endif
	{
		char *p;
		strcpy(prm_searchpath, name);
		p = strrchr(prm_searchpath,'\\');
		if (!p)
		{
			p = strrchr(prm_searchpath, ':');
			if (!p)
				prm_searchpath[0] = 0;
			else
				p[1] = 0;
		}
		else
			*p = 0;
	}	
	deleteFileData(name);
	localDefines = calloc(2*defineCount, sizeof(char *));
	if (!localDefines)
		return FALSE;
    if (proj)
    {
        PROJECTITEM *pj = proj;
        while (pj && pj->type != PJ_PROJ) pj = pj->parent;
    	buildType = PropGetInt(pj, "__PROJECTTYPE");
    }
	else
		buildType = BT_CONSOLE;
	if (buildType ==BT_DOS || buildType == BT_RAW)
	{
		defines[0] = DOSDEF;
	}
	else
	{
		defines[0] = WIN32DEF;
	}
		
	memcpy(localDefines, defines, DEFINE_COUNT * 2 * sizeof(char *));
#ifdef FROMIDE
	for (i = DEFINE_COUNT; i < defineCount; i++)
	{
		localDefines[i*2] = projDefines[i-DEFINE_COUNT][0];
		localDefines[i*2 + 1] = projDefines[i-DEFINE_COUNT][1];
	}
#endif
	sprintf(sys_searchpath, "%s\\include", szInstallPath);
	fname_count = 0;
	base_fname = NULL;
    parse(name, file_callback, alloc_callback,NULL,NULL, 
		  defineCount, localDefines, symcallback, 0);
	free(localDefines);
	for (i = DEFINE_COUNT; i < defineCount; i++)
	{
		free(projDefines[i-DEFINE_COUNT][0]);
		free(projDefines[i-DEFINE_COUNT][1]);
    }
	return TRUE;	
}
#endif
static char *parsepath(char *path, char *buffer)
{
    char *pos = path;

    /* Quit if hit a ';' */
    while (*pos)
    {
        if (*pos == ';')
        {
            pos++;
            break;
        }
        *buffer++ =  *pos++;
    }
    *buffer = 0;

    /* Return a null pointer if no more data */
    if (*pos)
        return (pos);

    return (0);
}
FILE *searchPath(char *string, char *searchpath, char *mode)
{
    FILE *in;
    char *newpath = searchpath;

    /* Search local path */
    in = fopen((char*)string, mode);
    if (in)
    {
        return (in);
    }
    else if (!searchpath || searchpath[0] == 0)
        return 0;
    else
    {
        /* If no path specified we search along the search path */
        if (string[0] != '\\' && string[1] != ':')
        {
            char buffer[200];
            while (newpath)
            {
                /* Create a file name along this path */
                newpath = parsepath(newpath, buffer);
                if (buffer[strlen(buffer) - 1] != '\\')
                    strcat(buffer, "\\");
                strcat(buffer, (char*)string);

                /* Check this path */
                in = fopen(buffer, mode);
                if (in)
                {
                    strcpy(string, buffer);
                    return (in);
                }
            }
        }
    }
    return (0);
}
static FILE *srchpth(char *name, char *path, char *attrib, int sys)
{
    FILE *rv = searchPath(name, path, attrib);
    char buf[265],  *p;
    if (rv !=  NULL || !sys)
        return rv;
    strcpy(buf, name);
    p = strrchr(buf, '.');
    if (p && !stricmp(p, ".h"))
    {
        *p = 0;
        rv = searchPath(buf, path, attrib);
        if (rv !=  NULL)
            strcpy(name, buf);
    }
    return rv;
}
static const void * string_getline(FileText *arg)
{
	static char line[16384];
	char *p = line;
	int n = 512;
	if (*arg->current == 0)
		return NULL;
	line[0] = 0;
	while (*arg->current && *arg->current != '\n' && n--)
	{
		if (*arg->current != '\r')
			*p++ = *(arg->current)++;
		else
			(arg->current)++;
	}
			
	*p = 0;
	if (*arg->current == '\n')
		++(arg->current);
	return line;
}
void deleteFileData(char *name)
{
	deleteSymsForFile(name);
	lookup_fname(name);
	allocfreeall(&base_fname->allocblk);
}
void *__stdcall alloc_callback(void *ptr_or_size, int alloc)
{
	if (alloc)
	{
        	return calloc_wrapper((int)ptr_or_size, 1);
       
	}
	else
	{
		free_wrapper(ptr_or_size);
		return 0;
	}
}
void ccLineChange(char *name, int drawnLineno, int delta)
{
	EDITDATA *e;
	DWINFO *ptr;
	int i;
	if (delta == 0 || !PropGetBool(NULL, "CODE_COMPLETION"))
		return;
	EnterCriticalSection(&ewMutex);
	ptr = editWindows;
    while (ptr)
    {
		if (!xstricmp(name,ptr->dwName))
		{
			e = ptr->editData;
			break;
		}
        ptr = ptr->next;
    }
	if (!e || !e->cd->lineData)
	{
		LeaveCriticalSection(&ewMutex);
		return;
	}
	if (delta > 0)
	{
		int offset = delta/8;
		int shl = delta & 7;
		int shr = 8 - shl;
		if (shr == 8)
			shr = 0;
		if ((delta+drawnLineno + 7)/8 > e->cd->lineDataMax)
		{
			unsigned char *newData = calloc_wrapper(e->cd->lineDataMax + 1000, 1);
			if (newData)
			{
				memcpy(newData, e->cd->lineData, e->cd->lineDataMax);
				free_wrapper(e->cd->lineData);
				e->cd->lineData = newData;
				e->cd->lineDataMax+= 1000;
			}
		}
		for (i=e->cd->lineDataMax-1; i >= drawnLineno/8; i--)
		{
			if (i - offset < drawnLineno/8)
			{
				e->cd->lineData[i] = 0xff;
			}
			else if (i - offset == drawnLineno/ 8)
			{
				e->cd->lineData[i] = (e->cd->lineData[i-offset] << shl) | ~(255 << shl);
			}
			else
			{
				e->cd->lineData[i] = (e->cd->lineData[i-offset-1] >> shr) 
				| (e->cd->lineData[i-offset] << shl);
			}
		}
	}
	else
	{
		int offset;
		int shr;
		int shl;
		delta = - delta;
		shr = delta & 7;
		shl = 8 - (delta & 7);
		if (shl == 8)
			shl = 0;
		offset = delta/8;
		for (i = drawnLineno/8; i < e->cd->lineDataMax; i++)
		{
			if (i + offset >= e->cd->lineDataMax)
			{
				e->cd->lineData[i] = 0xff;
			}
			else if (i + offset == e->cd->lineDataMax-1)
			{
				e->cd->lineData[i] = (e->cd->lineData[i+offset] >> shr) | ~(255 >> shr);
			}
			else
			{
				e->cd->lineData[i] = (e->cd->lineData[i+offset+1] << shl) 
					| (e->cd->lineData[i+offset] >> shr);
			}
		}
	}
	LeaveCriticalSection(&ewMutex);
}
static char *ccGetEditData(HWND hwnd)
{
    EDITDATA *p = (EDITDATA *)GetWindowLong(hwnd, 0);
    int i;
    INTERNAL_CHAR *x;
    char *rv, *q;
    if (!p)
        return 0;
    i = p->cd->textlen;
    if (!i)
        return 0;
    rv = q = calloc(i+1, 1);
    if (!rv)
        return 0;
    x = p->cd->text;
    while (i--)
        *q++ = x++->ch;
    *q = 0;
    return rv;
}
int __stdcall file_callback(char *name, int sys_inc, void **func, void **arg,
							unsigned char *lines_used, int linecount)
{
		DWINFO *ptr;
		FILE *fp;
		FileText *ftext ;
		int i;
		EDITDATA *editData;
//		Sleep(1);
		if (!name)
		{
			ftext = *arg;
			if (lines_used && ftext->data)
			{
				EnterCriticalSection(&ewMutex);
		        ptr = editWindows;
		        while (ptr)
		        {
		        	if (ftext->data == ptr->editData)
					{
						ftext->data->cd->lineData = lines_used;
						ftext->data->cd->lineDataMax = linecount;
						InvalidateRect(editData->self, 0, 0);
						break;
					}
					ptr = ptr->next;
				}
				LeaveCriticalSection(&ewMutex);
				if (!ptr)
					free_wrapper(lines_used);
			}
			if (fname_count)
				base_fname = fname_list[--fname_count];
			FreeEditData(ftext->base);
			free(ftext);
			return 0;
		}
#ifdef FROMIDE
		EnterCriticalSection(&ewMutex);
        ptr = editWindows;
        while (ptr)
        {
        	if (!xstricmp(name, ptr->dwName))
			{
				char *editdata;
                HWND xx = ptr->dwHandle;
				ftext = calloc(1, sizeof(FileText));
				ftext->data = ptr->editData;
				ftext->data->cd->lineData = NULL;
				LeaveCriticalSection(&ewMutex);

				editdata = ccGetEditData(xx);
				ftext->base = ftext->current = editdata;
				if (!editdata)
				{
					return 1;
				}
				if (base_fname)
					fname_list[fname_count++] = base_fname;
				deleteFileData(name);
				*func = string_getline;
				*arg = ftext;
				return 0;
			}
            ptr = ptr->next;
        }
		LeaveCriticalSection(&ewMutex);
#endif
		// allocate
	    if (!sys_inc)
	        fp = srchpth(name, prm_searchpath, "r", FALSE)
	            ;
	    if (sys_inc || fp ==  NULL)
	    {
	        fp = srchpth(name, sys_searchpath, "r", TRUE);
	        if (fp == NULL && sys_inc)
	            fp = srchpth(name, prm_searchpath, "r", FALSE);
	    }
		if (fp == NULL)
        {
			return 1;
        }
		if (fp)
		{
			long len;
			char *str;
			if (base_fname)
				fname_list[fname_count++] = base_fname;
			deleteFileData(name);
			fseek(fp, 0, SEEK_END);
			len = ftell(fp);
			fseek(fp, 0, SEEK_SET);
			str = calloc(len + 1, 1);
			if (str)
			{
				len = fread(str, 1, len, fp);
				str[len] = 0;
			}
			fclose(fp);
			ftext = calloc(1, sizeof(FileText));
            if (!ftext)
            {
                free(str);
                return 1;
            }
			ftext->base = ftext->current = str;
			*func = string_getline;
			*arg = ftext;
			return 0;
		}
		return 1;
}
typedef struct
{
	TYPE *newtype;
	type_t *oldtype;
} ENTRY;

#define HASH_SIZE 1024

static SYM *symbol_hash[HASH_SIZE];
static FNAME *fname_hash[HASH_SIZE];

#define MAX_TYPE_ENTRIES 1000
static ENTRY type_entries[MAX_TYPE_ENTRIES];
static int type_entry_count = 0;

static unsigned int ComputeHash(const unsigned char *string)
{
    unsigned long hash = 5381;
    unsigned int c;

    while (c = *string++)
        hash = ((hash << 5) + hash) ^ c;

    return hash % HASH_SIZE;
}
static char *lookup_fname(char *name)
{
	unsigned hash = ComputeHash(name);
	FNAME **names = &fname_hash[hash];
	FNAME *newname;
	while (*names)
	{
		if (!strcmp((*names)->name, name))
		{
			base_fname = *names;
			return (*names)->name;
		}
		names = &(*names)->next;
	}
	newname = calloc(1, sizeof(FNAME));
	if (newname)
	{
		base_fname = newname;
		strcpy(newname->name, name);
		newname->next = fname_hash[hash];
		fname_hash[hash] = newname;
		return newname->name;
	}
	return NULL;
}
static SYM *mksym(char *name, TYPE *usertype, char *filename, int startline, int endline)
{
	SYM *sym = calloc_wrapper(1, sizeof(SYM));
	if (sym)
	{
		sym->link = NULL;
		sym->name = strdup_wrapper(name);
		sym->filename = base_fname->name;
		sym->type = usertype;
		sym->startline = startline;
		sym->endline = endline;
	}
	return sym;
}
static void insertlocalsym(TYPE *type, char *symname, TYPE *usertype, 
					char *filename, int linestart, int lineend)
{
	SYM *sym = mksym(symname, usertype, filename, linestart, lineend);
	if (sym)
	{
		SYM **syms = &type->syms;
		while (*syms)
			syms = &(*syms)->link;
		*syms = sym;
	}
}
static void insertsym(char *symname, TYPE *usertype, 
					char *filename, int linestart, int lineend)
{
	SYM *sym = mksym(symname, usertype, filename, linestart, lineend);
	if (sym)
	{
		unsigned hash = ComputeHash(symname);
		SYM **syms = & symbol_hash[hash];
		sym->link = *syms;
		*syms = sym;
	}
}
static TYPE *clonetype(type_t *typeinfo)
{
	type_t *temp = typeinfo;
	TYPE *rv =NULL, **list = &rv;
	int i;
	while (temp->btype == BT_POINTER)
	{
		*list = calloc_wrapper(1, sizeof(TYPE));
		if (!*list)
			return NULL;
		(*list)->type = temp->btype;
		(*list)->t_const = temp->t_const;
		(*list)->t_volatile = temp->t_volatile;
		(*list)->t_restrict = temp->t_restrict;
		if (temp->typedefname)
			(*list)->typedefname = strdup_wrapper(temp->typedefname);
		list = &(*list)->link;
		temp = temp->link;
	}
	switch(temp->btype)
	{
		case BT_FUNC:
		case BT_STRUCT:
		case BT_UNION:
//		case BT_ENUM:
			for (i = 0; i < type_entry_count; i++)
			{
				if (type_entries[i].oldtype == temp)
				{
					*list = type_entries[i].newtype;
					return rv;
				}
			}
			*list = calloc_wrapper(1, sizeof(TYPE));
			if (!*list)
				return NULL;
			(*list)->type = temp->btype;
			(*list)->t_const = temp->t_const;
			(*list)->t_volatile = temp->t_volatile;
			(*list)->t_restrict = temp->t_restrict;
			if (temp->name)
				(*list)->name = strdup_wrapper(temp->name);
			if (temp->typedefname)
				(*list)->typedefname = strdup_wrapper(temp->typedefname);
			if (type_entry_count < MAX_TYPE_ENTRIES)
			{
				type_entries[type_entry_count].oldtype = temp;
				type_entries[type_entry_count++].newtype = *list;
			}
			enumTypes(temp, typecallback, *list);
			temp = temp->link;
			list = &(*list)->link;
			if (temp)
				*list = clonetype(temp);
			return rv;								 
		default:
			*list = calloc_wrapper(1, sizeof(TYPE));
			if (!*list)
				return NULL;
			if (temp->typedefname)
				(*list)->typedefname = strdup_wrapper( temp->typedefname);
			(*list)->type = temp->btype;
			(*list)->t_const = temp->t_const;
			(*list)->t_volatile = temp->t_volatile;
			(*list)->t_restrict = temp->t_restrict;
			return rv;
	}
}
void __stdcall typecallback(char *symname, char *filename, 
					   int linestart, int lineend, type_t *typeinfo,
					   void *userdata)
{
	if (typeinfo != NULL)
	{
		TYPE *type = (TYPE *)userdata;
		TYPE *usertype = clonetype(typeinfo);
		insertlocalsym(type, symname, usertype, filename, linestart, lineend);
	}
}
void __stdcall symcallback(char *symname, char *filename, 
					   int linestart, int lineend, type_t *typeinfo,
					   void *userdata)
{
	TYPE *usertype;
	type_t *temp = typeinfo;
	type_entry_count = 0;
	if (temp == NULL)
		return;
	while (temp->btype == BT_POINTER)
	{
		temp = temp->link;
	}

	switch(temp->btype)
	{
		case BT_FUNC:
		case BT_STRUCT:
		case BT_UNION:
//		case BT_ENUM:
			break;
		default:
			break;
	}
	usertype = clonetype(typeinfo);

	if (usertype)
	{
		insertsym(symname, usertype, filename, linestart, lineend);
	}
}
void unclone_type(TYPE *type);
void unclone_symlist(SYM *syms)
{
	while (syms)
	{
		SYM *next = syms->link;
		unclone_type(syms->type);
		if (syms->name)
		{
			free_wrapper(syms->name);
		}
		free_wrapper(syms);
		syms = next;
	}
}
void unclone_type(TYPE *type)
{
	while (type)
	{
		TYPE *next;
		int i;
		for (i=0; i < type_entry_count; i++)
		{
			if (type_entries[i].newtype == type)
			{
				return;
			}
		}
		next = type->link;
		switch(type->type)
		{
			case BT_FUNC:
			case BT_STRUCT:
			case BT_UNION:
//			case BT_ENUM:
				if (i >= type_entry_count && type_entry_count < MAX_TYPE_ENTRIES)
				{
					type_entries[type_entry_count].oldtype = -1;
					type_entries[type_entry_count++].newtype = type;
					unclone_symlist(type->syms);
					unclone_type(type->link);
					if (type->name)
						free_wrapper(type->name);
					if (type->typedefname)
						free_wrapper(type->typedefname);
					free_wrapper(type);
				}
				return;
			default:
				break;
		}
		if (type->name)
			free_wrapper(type->name);
		if (type->typedefname)
			free_wrapper(type->typedefname);
		free_wrapper(type);
		type = next;
	}
}
void deleteSym(SYM *sym)
{
	type_entry_count = 0;
	unclone_type(sym->type);
	if (sym->name)
	{
		free_wrapper(sym->name);
	}
	free_wrapper(sym);
}
void deleteSymsForFile(char *name)
{
	int i;
	for (i=0; i < HASH_SIZE; i++)
	{
		SYM **syms = &symbol_hash[i];
		while (*syms)
		{
			if (!stricmp((*syms)->filename, name))
			{
				SYM *next = (*syms)->link;
//				deleteSym(*syms);
				*syms = next;
			}
			else
				syms = &(*syms)->link;
		}
	}
}
TYPE *parse_lookup_symtype(char *name, char *module, int line)
{
	unsigned hash = ComputeHash(name);
	SYM *sym = symbol_hash[hash];
	SYM *match = NULL;
	SYM *match1 = NULL;
	while (sym)
	{
		if (!strcmp(sym->name, name))
		{
			if (!stricmp(sym->filename, module))
			{
				if (line >= sym->startline && line <= sym->endline)
				{
					if (match)
					{
						if (match->startline < sym->startline)
							match = sym;
					}
					else
						match = sym;
				}
			}
			else 
				match1 = sym;
				
		}
		sym = sym->link;
	}
	if (!match)
	{
		match = match1;
	}
	if (match)
	{
		return match->type;
	}
	return NULL;
}
const char *typenames[] = 
{
	"bit", "bool", "char", "unsigned char", "short", "unsigned short", "wchar_t", "enum",
	"int", "unsigned", "long", "unsigned long" ," long long" , "unsigned long long",
	"float", "double", "long double", 
	"float _Imaginary", "double _Imaginary", "long double _Imaginary",
	"float _Complex", "double _Complex", "long double _Complex",
	"void"
};
void formatquals(char *buf, TYPE *type)
{
	if (type->t_const)
		strcat(buf, "const ");
	if (type->t_volatile)
		strcat(buf, "volatile ");
	if (type->t_restrict)
		strcat(buf, "restrict ");
}
void formattype(char *buf, TYPE *type)
{
	char *name;
	int str = FALSE;
	if (!type)
		return;
	if (type->typedefname)
	{
		name = type->typedefname;
	}
	else switch (type->type)
	{
		case BT_STRUCT:
			name = "struct ";
			str = TRUE;
			break;
		case BT_UNION:
			name = "union ";
			str = TRUE;
			break;
		case BT_FUNC:
			name = "(*)() ";
			str = TRUE;
			break;
		case BT_ENUM:
			name = "enum ";
			str = TRUE;
			break;
		case BT_POINTER:
			formattype(buf, type->link);
			name = "*";
			break;
		default:
			name = typenames[type->type];
			break;
	}
	if (name[0] == '*')
	{
		strcat(buf, "* ");
		formatquals(buf, type);
	}
	else
	{
		formatquals(buf, type);
		sprintf(buf + strlen(buf), "%s ", name);
		if (str && type->name)
			sprintf(buf + strlen(buf), "%s ", type->name);
	}		
}
int formatproto(char *proto, char *name, TYPE *type, int *offsets)
{
	SYM *syms = type->syms;
	int i = 0;
	proto[0] = 0;
	formattype(proto, type->link);
	strcat(proto, name);
	strcat(proto, "(");
	while (syms)
	{
		offsets[i++] = strlen(proto);
		formattype(proto + strlen(proto), syms->type);
		strcat(proto, syms->name);
		if (syms->link)
			strcat(proto, ", ");
		syms = syms->link;
	}
	offsets[i] = strlen(proto);
	strcat(proto, ")");	
	return i;
}
int parse_lookup_prototype(char *proto, char *name, int *offsets)
{
	if (name[0])
	{
		unsigned hash = ComputeHash(name);
		SYM *sym = symbol_hash[hash];
		while (sym)
		{
			if (!strcmp(name, sym->name))
				if (sym->type->type == BT_FUNC)
				{
					return formatproto(proto, name, sym->type, offsets);
				}
			sym = sym->link;
		}
	}	
	return 0;
}
#ifdef TEST
void dumpquals(TYPE *type)
{
	if (type->t_const)
		printf("const ");
	if (type->t_volatile)
		printf("volatile ");
	if (type->t_restrict)
		printf("restrict ");
}
void dumpsym(SYM *sym, int level);
void dumptype(TYPE *type, int level)
{
	char *name;
	int str = FALSE;
	if (!type)
		return;
	switch (type->type)
	{
		case BT_STRUCT:
			name = "struct ";
			str = TRUE;
			break;
		case BT_UNION:
			name = "union ";
			str = TRUE;
			break;
		case BT_FUNC:
			dumptype(type->link, level);
			name = "(*)() ";
			str = TRUE;
			break;
		case BT_ENUM:
			name = "enum ";
			str = TRUE;
			break;
		case BT_POINTER:
			dumptype(type->link, level);
			name = "*";
			break;
		default:
			name = typenames[type->type];
			break;
	}
	if (name[0] == '*')
	{
		printf(" *");
		dumpquals(type);
	}
	else
	{
		dumpquals(type);
		printf("%s ", name);
		if (str && type->name)
			printf("%s ", type->name);
	}		
	if (str)
	{
		SYM *syms = type->syms;
		int i;
		fputc('\n', stdout);
		for (i=0; i < type_entry_count; i++)
		{
			if (type_entries[i].newtype == type)
				return;
		}
		if (type_entry_count >= MAX_TYPE_ENTRIES)
			return;
		type_entries[type_entry_count++].newtype = type;
		
		while(syms)
		{
			dumpsym(syms, level +1);
			syms = syms->link;
		}
	}
}
void dumpsym(SYM *sym, int level)
{
	int i;
	for (i=0; i <level; i++)
		fputc('\t', stdout);
	printf("%s(%d:%d):%s ", sym->filename, sym->startline, sym->endline, sym->name);
	dumptype(sym->type, level);
	fputc('\n', stdout);
}
void dump(void)
{
	int i;
	for (i=0; i < HASH_SIZE; i++)
	{
		SYM *syms = symbol_hash[i];
		while (syms)
		{
			type_entry_count = 0;
			dumpsym(syms, 0);
			syms = syms->link;
		}
	}
}
int main(int argc, char* argv[])
{
	if (argv[1])
	{
		strcpy(sys_searchpath, "C:\\program files\\cc386\\include");
        parse(argv[1],file_callback, NULL,NULL,
			   DEFINE_COUNT, defines, symcallback, 0);
		dump();
		deleteSymsForFile("q.c");
	}
    return 0;
}
#endif

