#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include "parser.h"

typedef struct {
	char *base;
	char *current;
} FileText;

char *defines[] = { 
	"_intrinsic","",
	"_import","",
	"_export","", 
	"__int64","long long",
	"__stdcall", "",
	"__cdecl", "" 
};


char *prm_searchpath = "";
char *sys_searchpath = "C:\\program files (x86)\\cc386\\include";
#define DEFINE_COUNT (sizeof(defines)/sizeof(char *) / 2)

void __stdcall typecallback(char *symname, char *filename, 
					   int linestart, int lineend, type_t *typeinfo,
					   void *userdata);
void __stdcall symcallback(char *symname, char *filename, 
					   int linestart, int lineend, type_t *typeinfo,
					   void *userdata);


void *allocArray[10000];

void *calloc_wrapper(int n, int size)
{
	void *rv = calloc(n, size);
	int i;
	for (i =0; i < 10000; i++)
		if (allocArray[i] == NULL)
		{
			allocArray[i] = rv;
			break;
		}
	return rv;
}
void *free_wrapper(void *data)
{
	int i;
	free(data);
	for (i=0; i < 10000; i++)
		if (allocArray[i] == data)
		{
			allocArray[i] = NULL;
			break;
		}
}
void alloc_rundown()
{
	int i;
	int z = 0;
	for (i=0; i < 10000; i++)
		if (allocArray[i] != NULL)
		{
			z++;
		}
	if (z)
		printf("unallocated %d\n", z);
}
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
        if (!strchr(string, '\\'))
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
	static char line[513];
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

int __stdcall file_callback(char *name, int sys_inc, void **func, void **arg, 
							unsigned char *lines_used, int lines)
{
	
		FILE *fp;
		FileText *ftext ;
		if (!name)
		{
			ftext = *arg;
			free_wrapper(ftext->base);
			free_wrapper(ftext);
			return 0;
		}
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
			return ;
		if (fp)
		{
			long len;
			char *str;
			fseek(fp, 0, SEEK_END);
			len = ftell(fp);
			fseek(fp, 0, SEEK_SET);
			str = calloc_wrapper(1, len+1);
			if (str)
			{
				// may read less because of removeal of '\r'
				len = fread(str, 1, len, fp);
				str[len] = 0;
			}
			fclose(fp);
			ftext = calloc_wrapper(1, sizeof(FileText));
			ftext->base = ftext->current = str;
			*func = string_getline;
			*arg = ftext;
			return 0;
		}
		return 1;
}
typedef struct _type
{
	struct _type *link;
	char name[256];
	enum Basictype type;
	struct _sym *syms;
	int t_const : 1;
	int t_volatile : 1;
	int t_restrict : 1;
} TYPE;

typedef struct _sym
{
	struct _sym *link;
	struct _type *type;
	char name[256];
	char filename[256];
	int startline;
	int endline;
} SYM;
typedef struct
{
	TYPE *newtype;
	type_t *oldtype;
} ENTRY;

#define HASH_SIZE 1024
static SYM *symbol_hash[HASH_SIZE];

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
static SYM *mksym(char *name, TYPE *usertype, char *filename, int startline, int endline)
{
	SYM *sym = calloc_wrapper(1, sizeof(SYM));
	if (sym)
	{
		strcpy(sym->name, name);
		sym->type = usertype;
		strcpy(sym->filename, filename);
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
				strcpy((*list)->name, temp->name);
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
	TYPE *type = (TYPE *)userdata;
	TYPE *usertype = clonetype(typeinfo);
	insertlocalsym(type, symname, usertype, filename, linestart, lineend);
}
void __stdcall symcallback(char *symname, char *filename, 
					   int linestart, int lineend, type_t *typeinfo,
					   void *userdata)
{
	TYPE *usertype;
	type_t *temp = typeinfo;
	type_entry_count = 0;
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
			return ;
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
		free_wrapper(syms);
		syms = next;
	}
}
void unclone_type(TYPE *type)
{
	while (type)
	{
		TYPE *next = type->link;
		int i;
		switch(type->type)
		{
			case BT_FUNC:
			case BT_STRUCT:
			case BT_UNION:
//			case BT_ENUM:
				for (i=0; i < type_entry_count; i++)
				{
					if (type_entries[i].newtype == type)
						break;
				}
				if (i >= type_entry_count && type_entry_count < MAX_TYPE_ENTRIES)
				{
					type_entries[type_entry_count++].newtype = type;
					unclone_symlist(type->syms);
				}
				break;
			default:
				break;
		}
		free_wrapper(type);
		type = next;
	}
}
void deleteSym(SYM *sym)
{
	type_entry_count = 0;
	unclone_type(sym->type);
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
				deleteSym(*syms);
				*syms = next;
			}
			else
				syms = &(*syms)->link;
		}
	}
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
        parse(argv[1],file_callback, NULL,NULL, NULL,
			   DEFINE_COUNT, defines, symcallback, 0);
		dump();
		deleteSymsForFile("q.c");
		alloc_rundown();
	}
    return 0;
}
