/*CC386 C Compiler
Copyright 1994-2011 David Lindauer.

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

This program is derived from the cc68k complier by 
Matthew Brandt (mailto::mattb@walkingdog.net) 

You may contact the author of this derivative at:

mailto::camille@bluegrass.net
 */
#include <stdio.h>
#include <fcntl.h>
#include <ctype.h>
#include <string.h>
#include <limits.h>
#include <time.h>
#include <stdlib.h>
#include <dir.h>
#include "c_lex.h"
#include "c_list.h"
#include "c_parser.h"
#include "c_alloc.h"
#include "c_preprc.h"
#include "c_preval.h"
 
#define REPLACED_TOKENIZING ((unsigned char)0x80)
#define TOKENIZING_PLACEHOLDER ((unsigned char)0x81)
#define MACRO_REPLACE_SIZE 16384

char *prm_searchpath = ""; 
char *sys_searchpath = "";

static int definelistcount;
static symbol_t *definelist[100]; /* Way deep but hey! */

/* List of standard macros */
#define INGROWNMACROS 4

struct inmac
{
    char *s;
    void(*func)();
} ingrownmacros[INGROWNMACROS] = 
{
    {
        "__FILE__", filemac
    }
    , 
    {
        "__DATE__", datemac, 
    }
    , 
    {
        "__TIME__", timemac
    }
    , 
    {
        "__LINE__", linemac
    }
};

static list_t defines;		/* head of the identifiers list */
static symtab_t *Definesymtab;
bool preprocessorSkip;
static bool initted;
/* Moudle init */
void preprocini()
{
//	if (!initted)
	{
		list_init(&defines);
		Definesymtab = new_symbol_table(&defines, TRUE);
		preprocessorSkip = FALSE;
		initted = TRUE;
	}
}

lex_env_t *parse_hash_directive(const char *line, lex_env_t *le)
{
	char id[256];
	int n = -1;
    while (isspace (*line))
        line++ ;
    if (! *line)
		return le ;
	if (!isalpha(*line) || sscanf(line,"%[a-z]%n", id, &n) != 1)
	{
		xprint( "Invalid preprocessor directive\n");
		return le;
	}
	if (n != -1)
		line += n;
	else
		line += strlen(line);
	while (isspace(*line))
		++line;
    if (strcmp(id, "include") == 0)
        return doinclude(line, le);
    else if (strcmp(id, "define") == 0)
        return dodefine(line, le);
    else if (strcmp(id, "endif") == 0)
        return doendif(line, le);
    else if (strcmp(id, "else") == 0)
        return doelse(line, le);
    else if (strcmp(id, "ifdef") == 0)
        return doifdef(TRUE,line, le);
    else if (strcmp(id, "ifndef") == 0)
        return doifdef(FALSE,line, le);
    else if (strcmp(id, "if") == 0)
    {
        return doif(0,line, le);
    }
    else if (strcmp(id, "elif") == 0)
    {
        return doelif(line, le);
    }
    else if (strcmp(id, "undef") == 0)
        return (doundef(line, le));
    else if (strcmp(id, "error") == 0)
        return (doerror(line, le));
    else if (strcmp(id, "pragma") == 0)
        return (dopragma(line, le));
    else if (strcmp(id, "line") == 0)
        return (doline(line, le));
    else
    {
		xprint("unknown prepocessor directive: %s\n", id);
        return  le;
    }
}

//-------------------------------------------------------------------------

static lex_env_t * doerror(const char *line, lex_env_t *le)
{
	return le + strlen (line);
}



//-------------------------------------------------------------------------

static lex_env_t * dopragma(const char *line, lex_env_t *le)
{
	return le;
}

// parses the _Pragma directive
lex_env_t *Compile_Pragma(const char *line, lex_env_t *le)
{
	while (isspace(*line))
		++line;
	if (*line != '(')
		xprint( "ERROR IN PRAGMA DIRECTIVE");
	else {
		while (isspace(*line))
			++line;
		if (*line != '"')
			xprint( "ERROR IN PRAGMA DIRECTIVE");
		else {
			while (*line && *line != '"')
				++line;
			while (isspace(*line))
				++line;
			if (*line != ')')
				xprint( "ERROR IN PRAGMA DIRECTIVE");
			else
				return le;
		}
	}
	return le;	
}


//-------------------------------------------------------------------------

static lex_env_t * doline(const char *line, lex_env_t *le)
/*
 * Handle #line directive
 */
{
    int n;
	int nitems;
	char name[4096];
	int lnum;
    if (preprocessorSkip)
        return le;
	nitems = sscanf(line, "%d %s", &lnum, name);
	if (nitems < 1) {
		xprint( "Bad # directive \"%s\"", line);
		return le;
	}
	if (nitems == 2) {
		char *buf;
		int len;
		if (name[0] != '\"');
		{
			replace_macros(name, le);
		}
		if (name[0] == '\"')
		{
			int len = strlen(name)-1;
			strcpy(name, name+1);
			if (name[len-1] != '\"')
				xprint("bad #line directive");
			else
				name[len-1] = 0;
		}
		else
		{
			xprint("bad #line directive");
		}
		strcpy(name, buf);			
		len = strlen(name);
		buf = NEW_ARRAY(char, len + 1);
		(void) memcpy(buf, name, len + 1);
		le->le_filename = buf;
	}

	/*  Subtract 1 because we number internally from 0,
	 *  and 1 because we are just about to bump the
	 *  line number.
	 */
	le->le_lnum = lnum - 2;

	return le;
}


/*
 * Pull the next path off the path search list
 */
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

/*
 * For each library:
 * Search local directory and all directories in the search path
 *  until it is found or run out of directories
 */
FILE *SearchPath(char *string, char *searchpath, char *mode)
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
//-------------------------------------------------------------------------

FILE *srchpth(char *name, char *path, char *attrib, int sys)
{
    FILE *rv = SearchPath(name, path, attrib);
    char buf[265],  *p;
    if (rv !=  NULL || !sys)
        return rv;
    strcpy(buf, name);
    p = strrchr(buf, '.');
    if (p && !stricmp(p, ".h"))
    {
        *p = 0;
        rv = SearchPath(buf, path, attrib);
        if (rv !=  NULL)
            strcpy(name, buf);
    }
    return rv;
}

//-------------------------------------------------------------------------

static const char *
filegetline(char *arg)
{
	static char line[512];
	line[0] = 0;

	return fgets(line, sizeof line, (FILE *)arg);
}
static lex_env_t * doinclude(const char *line, lex_env_t *le)
{
    int rv;
	int sys_inc;
	char name[260], *p = name;
	FILE *file = NULL;
    if (preprocessorSkip)
        return le;
	if (*line != '\"' && *line != '<')
	{
		replace_macros(line, le);
	}
	if (*line == '"')
	{
		line ++;
		sys_inc = FALSE;
	}
	else if (*line == '<')
	{
		line ++;
		sys_inc = TRUE;
	}
	while (*line && *line != (sys_inc ? '>' : '"'))
	{
		*p++ = *line++;
	}
	*p = 0;
	if (!*line)
	{
		xprint( "invalid include file specification");
		return le;
	}
	if (sys_searchpath)
	{
	    if (!sys_inc)
	        file = srchpth(name, prm_searchpath, "r", FALSE)
	            ;
	    if (sys_inc || file ==  NULL)
	    {
	        file = srchpth(name, sys_searchpath, "r", TRUE);
	        if (file ==  NULL && sys_inc)
	            file = srchpth(name, prm_searchpath, "r", FALSE);
	    }
	}
    if (sys_searchpath && file ==  NULL)
    {
		xprint("can't open include file %s\n", name);
    }
    else
    {
		lex_env_t *lenv = allocate(sizeof(lex_env_t));
		memcpy(lenv, le, sizeof(*le));
		le->le_filename = allocate(_MAX_PATH);
		strcpy(le->le_filename, name);
		le->le_lines_used = NULL;
		le->le_lines_used_max = 0;
		if (!sys_searchpath)
		{
			if (get_include_context(le, sys_inc))
			{
				xprint("can't open include file %s\n", name);
				return;
			}
		}
		else
		{
			le->le_getline = filegetline;
			le->le_getline_arg = (char *)file;
		}
		le->le_lptr = NULL;
		le->le_lnum = 0;
		le->le_had_error = FALSE;
		le->le_abort_parse = FALSE;
		le->next = lenv;
		le->ifskip = allocate(sizeof(ifskip_t));
		Lexeme = &lenv->le_lexeme;
		memset(Lexeme, 0, sizeof(*Lexeme));
		return le;
    }
    return le;
}
lex_env_t *uninclude(lex_env_t *current)
{
	lex_env_t *freer = current->next;
	if (current->le_getline == filegetline)
		fclose(current->le_getline_arg);
	else
		free_include_context(current);
	if (freer)
	{
		memcpy(current, freer, sizeof(*freer));
	}
	return current;
}
//-------------------------------------------------------------------------

void glbdefine(char *name, char *value)
{
    {
        char *p;
        defines_t *def;
		symbol_t *sym = find_symbol(Definesymtab, name, FALSE);
		if (sym != 0)
            return ;
			
        def = allocate(sizeof(defines_t));
        def->args = 0;
        def->argcount = 0;
        def->string = p = string_copy(value, strlen(value));
        while (*value)
            *p++ =  *value++;
        *p++ = 0;
		sym = install_symbol(Definesymtab, name, SC_NONE, 0);
		sym->data = (void *)def;
        return ;
    }
}

/* Handle #defines
 * Doesn't check for redefine with different value
 * Does handle ANSI macros
 */
lex_env_t * dodefine(const char *line, lex_env_t *le)
{
    symbol_t *sym,  *symo = 0;
    defines_t *def;
	defines_t *defo;
    char *args[40], count = 0;
	char id[MAX_IDENTIFIER_LEN], *p = id;
    const char *olptr;
    int x,i,j;
    if (preprocessorSkip)
        return le;
	if (!isalpha(*line) && *line != '_')
	{
		xprint( "Invalid macro name");
		return le;
	}
	while (isalnum(*line) || *line == '_')	
	{
		*p++ = *line++;
		
	}
    olptr = line;
	*p = 0;
	if ((symo = find_symbol(Definesymtab, id, FALSE)) != NULL)
	{
		sym = symo;
		defo = sym->data;
		sym->data = NULL;
	}
	else
	{	
		sym = install_symbol(Definesymtab, id, SC_NONE, 0);
		defo = NULL;
	}
    def = allocate(sizeof(defines_t));
    def->args = 0;
    def->argcount = 0;
    if (*line == '(')
    {
        int gotcomma=FALSE,nullargs=TRUE;
		++line;
		while (isspace(*line))
			++line;
        while (isalnum(*line) || *line == '_')
        {
            int j;
			p = id;
			while (isalnum(*line) || *line == '_')	
			{
				*p++ = *line++;
			
			}
			*p = 0;
            gotcomma = FALSE;
            nullargs = FALSE;
            args[count++] = string_copy(id, strlen(id));
            for (j=0; j < count-1; j++)
                if (!strcmp(args[count-1],args[j])) {
					xprint("macro argument %s already declared\n", id);
                    break ;
                }
			while (isspace(*line))
				++line;
            if (*line != ',')
                break;
			++line;
			while (isspace(*line))
				++line;
            gotcomma = TRUE;
        }
		while (isspace(*line))
			++line;
        if (prm_c99 && line[0] == '.' && line[1] == '.' && line[2] == '.' && (gotcomma || nullargs)) 
        {
            def->flags |= DS_VARARGS;
            gotcomma = FALSE;
			line += 3;
			while (isspace(*line))
				++line;
        }
        if (*line != ')' || gotcomma)
			xprint( "dodefine: need closepa: %s: %d\n", le->le_filename, le->le_lnum);
		++line;
        olptr = line;
		if (count)
		{
	        def->args = allocate(count * sizeof(char*));
    	    memcpy(def->args, args, count *sizeof(char*));
		}
        def->argcount = count + 1;
    }
    while (isspace(*olptr))
        olptr++;
    def->string = string_copy(olptr, strlen(olptr));
    x = strlen(def->string);
    if (def->string[x - 1] == '\n')
        def->string[--x] = 0;
    for (i=0,j=0; i < x+1; i++,j++)
        if (def->string[i]=='#' && def->string[i+1] == '#') {
            def->string[j] = REPLACED_TOKENIZING;
            i++;
        } else
            def->string[j] = def->string[i];
	x = strlen(def->string);
	for (i=0; i < x; i++)
		if (!isspace(def->string[i]))
		{
			if (def->string[i] == REPLACED_TOKENIZING)
				xprint("invalid define syntax");
			break;
		}
	for (i=x-1; i >= 0; i--)
		if (!isspace(def->string[i]))
		{
			if (def->string[i] == REPLACED_TOKENIZING || def->string[i] == '#')
				xprint("invalid define syntax");
			break;
		}
    sym->data = (void*)def;
    if (defo)
    {
        int same = TRUE;
        if (def->argcount != defo->argcount)
            same = FALSE;
        else
        {
            int i;
            char *p, *q;
            for (i = 0; i < def->argcount - 1 && same; i++)
                if (strcmp(def->args[i], defo->args[i]))
                    same = FALSE;
            p = def->string;
            q = defo->string;
            while (*p && *q) {
                if (isspace(*p))
                    if (isspace(*q)) {
                        while (isspace(*p)) p++;
                        while (isspace(*q)) q++;
                    } else {
                        break ;
                    }
                else
                    if (isspace(*q)) {
                        break ;
                    }
                    else if (*p++ != *q++)
                        break ;
            }
			if (*p)
				while (isspace(*p))
					p++;
			if (*q)
				while (isspace(*q))
					q++;
            if (*p || *q)
                same = FALSE;
        }
        if (!same) {
			xprint("macro definition of %s\n",sym->name);
        }
    }
    return le;
}

/*
 * Undefine
 */
lex_env_t * doundef(const char *line, lex_env_t *le)
{
	char id[MAX_IDENTIFIER_LEN], *p;
	symbol_t *symo ;
    if (preprocessorSkip)
        return le;
	if (!isalpha(*line) && *line != '_')
	{
		xprint( "Invalid macro name in under");
		return le;
	}
	p = id;
	while (isalnum(*line) || *line == '_')	
	{
		*p++ = *line++;	
	}
	if ((symo = find_symbol(Definesymtab, id, FALSE)) != NULL)
	{
		int i;
		defines_t *defs = symo->data;
        list_remove(&Definesymtab->symbols, symo);
	}
	return le;
}



/* The next few functions support recursion blocking for macros.
 * Basicall a list of all active macros is kept and if a lookup would 
 * result in one of those macros, no replacement is done.
 */
void nodefines(void)
{
    definelistcount = 0;
}

//-------------------------------------------------------------------------

int indefine(symbol_t *sym)
{
    int i;
    for (i = 0; i < definelistcount; i++)
        if (sym == definelist[i])
            return TRUE;

    return FALSE;
}

//-------------------------------------------------------------------------

void enterdefine(symbol_t *sym)
{
    definelist[definelistcount++] = sym;
}

//-------------------------------------------------------------------------

void exitdefine(void)
{
    if (definelistcount)
        definelistcount--;
}

//-------------------------------------------------------------------------

int defid(char *name, char **p, char *q)
/*
 * Get an identifier during macro replacement
 */
{
    int count = 0, i = 0;
    while (isalnum(**p) || **p == '_')
    {
            if (count < MAX_IDENTIFIER_LEN-1)
            {
                name[count++] = *(*p);
                if (q)
                    q[i++] = *(*p);
            }
        (*p)++;
    }
    if (q)
        q[count] = 0;
    name[count] = 0;
    return (count);
}

/* 
 * Insert a replacement string
 */
int definsert(char *macro, char *end, char *begin, char *text, int len, int replen)
{
    char *q;
    int i, p, r;
    int val;
    p = strlen(text);
    val = p - replen;
    r = strlen(begin);
    if (val + r + 1 >= len)
    {
		xprint("macro substition error\n");
        return (INT_MIN);
    }
    if (val > 0)
        for (q = begin + r + 1; q >= end; q--)
            *(q + val) =  *q;
    else
        if (val < 0)
        {
            r = strlen(end) + 1;
            for (q = end; q < end + r; q++)
                    *(q + val) =  *q;
        }
    for (i = 0; i < p; i++)
        begin[i] = text[i];
    return (p);
}
int defstringize(char *macro, char *end, char *begin, char *text, int len, int replen)
{
    int count = 0;
    while (begin != macro && isspace(*(begin-1)))
        begin--,count++ ;
    if (begin != macro && *(begin-1) == '#') 
    {
        char replmac[MACRO_REPLACE_SIZE];
        int r = strlen(text),j=0,i;
        replmac[j++] = '"';
        for (i=0; i < r; i++) 
        {
            if (text[i] == '\\' || text[i] == '"')
                replmac[j++] = '\\';
            replmac[j++] = text[i];
        }
        replmac[j++] = '"' ;
        replmac[j] = 0;
        return definsert(macro,end,begin-1,replmac,len,end - begin + 1) - count - 1;
    }
    return MACRO_REPLACE_SIZE;
}
/* replace macro args */
int defreplaceargs(char *macro, int count, char **oldargs, char **newargs, char *varargs)
{
    int i, rv;
    int instring = 0;
    char name[256];
    char *p = macro,  *q;
	int lookbehind = 0;
    while (*p)
    {
        if (instring && *p == instring && *(p - 1) != '\\')
        {
            instring = 0;
        }
        else if (!instring && (*p == '\'' ||  *p == '"') 
            && (p == macro || *(p - 1) != '\\' || *(p - 2) == '\\'))
        {
            instring =  *p;
        }
        else if (!instring && (isalpha(*p)|| *p == '_'))
        {
			char *lookahead;
            char *r = p;
            q = p;
            defid(name, &p, 0);
			lookahead = p;
			while (*lookahead == ' ' || *lookahead == '\t')
				lookahead++;
            r = macro;
            if (prm_c99 && !strcmp(name,"__VA_ARGS__")) 
            {
                if (!varargs[0]) {
					xprint("incorrect macro arguments __VA_ARGS__\n");
                }
                else
                {
                    rv = defstringize(macro,p,q,varargs, MACRO_REPLACE_SIZE - (q - macro), p-q);
                    if (rv < - MACRO_REPLACE_SIZE)
                        return rv;

                    if (rv == MACRO_REPLACE_SIZE) 
                    {
                        if ((rv = definsert(macro, p, q, varargs, MACRO_REPLACE_SIZE-(q - macro), p - q))
                                < -MACRO_REPLACE_SIZE)
                            return (FALSE);
                        else
                            p = q + rv;
                    } else
                        p = q + rv ;
                }
            }
            else for (i = 0; i < count; i++) 
            {
                static char ph[] = {TOKENIZING_PLACEHOLDER,0};
                if (!strcmp(name, oldargs[i]))
                {
					char *arg = newargs[i * 2 + ((*lookahead != REPLACED_TOKENIZING) &&
												(lookbehind != REPLACED_TOKENIZING))];
                    rv = defstringize(macro,p,q,arg, MACRO_REPLACE_SIZE - (q - macro), p-q);
                    if (rv < - MACRO_REPLACE_SIZE)
                        return rv;

                    if (rv == MACRO_REPLACE_SIZE) 
                    {
                        if (strlen(arg) == 0) 
                        {
                            char *r = q;
                            while (r != macro && isspace(*(r-1)))
                                r--;
                            if (r != macro && *(r-1) == REPLACED_TOKENIZING)
                                if ((rv = definsert( macro,p,q, ph+1, MACRO_REPLACE_SIZE-(q - macro), p - q))  < - MACRO_REPLACE_SIZE)
                                    return INT_MIN ;
                                else 
                                {
                                    p = q + rv ;
                                    break ;
                                }
                            else 
                            {
                                char *r = p;
                                while (*r && isspace(*r))
                                    r++;
                                if (*r && *r == REPLACED_TOKENIZING)
                                    if ((rv = definsert( macro, p, q, ph+1, MACRO_REPLACE_SIZE-(q - macro), p - q)) < - MACRO_REPLACE_SIZE)
                                        return rv ;
                                    else 
                                    {
                                        p = q + rv ;
                                        break ;
                                    }
                            }
                        }                        
                        if ((rv = definsert(macro, p, q, arg, MACRO_REPLACE_SIZE-(q - macro), p - q)) 
                            < - MACRO_REPLACE_SIZE)
                            return (FALSE);
                        else
                        {
                            p = q + rv;
                            break;
                        }
                    } 
                    else 
                    {
                        p = q + rv;
                        break;
                    }
                }
            }
			lookbehind = *lookahead;
        }
        if (*p)
            p++;
    }
    return (TRUE);
}
void deftokenizing(char *macro)
{
    int i, rv;
    int instring = 0;
    char *p = macro,  *q;
    while (*p)
    {
        if (*p == instring && *(p - 1) != '\\')
        {
            instring = 0;
        }
        else if ((*p == '\'' ||  *p == '"') 
            && (p == macro || *(p - 1) != '\\' || *(p - 2) == '\\'))
        {
            instring =  *p;
        }
        else if (!instring && *p == REPLACED_TOKENIZING)
        {
            char *b =p, *e = p ;
            while (b != macro && isspace(*(b-1)))
                b-- ;
            while (*++e != 0 && isspace(*e)) ;
            if (b != macro && b[-1] == TOKENIZING_PLACEHOLDER)
                b--;
            if (*e == TOKENIZING_PLACEHOLDER)
                if (*b != TOKENIZING_PLACEHOLDER)
                    e++;
            p = macro;
            while (*e)
                *b++ = *e++;
            *b = 0;
        }
        if (*p)
            p++;
    }
    p = q = macro;
    while (*p) 
    {
        if (*p == TOKENIZING_PLACEHOLDER || *p == REPLACED_TOKENIZING)
            p++;
        else 
            *q++ = *p++;
    }
    *q = 0;
}


//-------------------------------------------------------------------------

char *fullqualify(const char *string)
{
    static char buf[265];
    if (string[1] != ':')
    {
        char *p;
		const char  *q = string;
        getcwd(buf, 265);
        p = buf + strlen(buf);
        while (!strncmp(q, "..\\", 3))
        {
            q += 3;
            while (p > buf &&  *p != '\\')
                p--;
        }
        *p++ = '\\';
        strcpy(p, q);
        return buf;
    }
    return string;
}

//-------------------------------------------------------------------------

void filemac(char *string, lex_env_t *le)
{
    char *p = string;
    char *q = fullqualify(le->le_filename);
    *p++ = '"' ;
    while(*q)
    {
        if (*q == '\\')
            *p++ = *q ;
        *p++ = *q++;
    }
    *p++ = '"';
    *p++ = 0;
}

//-------------------------------------------------------------------------

void datemac(char *string, lex_env_t *le)
{
    struct tm *t1;
    time_t t2;
    time(&t2);
    t1 = localtime(&t2);
    strftime(string, 40, "\"%b %d %Y\"", t1);
}

//-------------------------------------------------------------------------

void timemac(char *string, lex_env_t *le)
{
    struct tm *t1;
    time_t t2;
    time(&t2);
    t1 = localtime(&t2);
    strftime(string, 40, "\"%H:%M:%S\"", t1);
}

//-------------------------------------------------------------------------

void linemac(char *string, lex_env_t *le)
{
    sprintf(string, "%d", le->le_lnum);
} 
/* Scan for default macros and replace them */
void defmacroreplace(char *macro, char *name, lex_env_t *le)
{
    int i;
    macro[0] = 0;
    for (i = 0; i < INGROWNMACROS; i++)
    if (!strcmp(name, ingrownmacros[i].s))
    {
        (ingrownmacros[i].func)(macro, le);
        break;
    }
}


//-------------------------------------------------------------------------

int replacesegment(char *start, char *end, int *inbuffer, int totallen, char **pptr, lex_env_t *le)
{
    char *args[100];
    char macro[MACRO_REPLACE_SIZE],varargs[500];
    char ascii[256];
    char name[256];
    int waiting = FALSE, rv;
    int size = 0;
    char *p,  *q, *r;
    symbol_t *sym;
    int insize,rv1;
    char * orig_end = end ;
    p = start;
    while (p < end)
    {
        q = p;
        if (!waiting && (*p == '"' 
            ||  *p == '\'') && (*(p - 1) != '\\' || *(p - 2) == '\\'))
        {
            waiting =  *p;
            p++;
        }
        else if (waiting)
        {
            if (*p == waiting && (*(p - 1) != '\\' || *(p - 2) == '\\'))
                waiting = 0;
            p++;
        }
        else if (isalpha(*p) || *p == '_')
        {
            defid(name, &p, ascii);
			if ((sym = find_symbol(Definesymtab, ascii, FALSE)) != 0 && !indefine(sym))
            {
                defines_t *def = sym->data;
                if (def->argcount)
                {
                    int count = 0;
                    char *q = p;
                    
                    varargs[0] = 0;
                    
                    while (isspace(*q)) q++ ;
                    if (*(q - 1) == '\n')
                    {
                        return INT_MIN + 1;
                    }
                    if (*q++ != '(')
                    {
                        goto join;
                    }
                    p = q;
                    if (def->argcount > 1)
                    {
                        do
                        {
                            char *nm = macro;
                            int nestedparen = 0, nestedstring = 0;
                            insize = 0;
                            while (isspace(*p)) p++;
                            while (*p && (((*p != ',' &&  *p != ')') ||
                                nestedparen || nestedstring) &&  *p != '\n'))
                            {
                                if (nestedstring)
                                {
                                    if (*p == nestedstring && (*(p - 1) != '\\'
                                        || *(p - 2) == '\\'))
                                        nestedstring = 0;
                                }
                                else if ((*p == '\'' ||  *p == '"') 
                                    && (*(p - 1) != '\\' || *(p - 2) == '\\'))
                                    nestedstring =  *p;
                                else if (*p == '(')
									nestedparen++;
                                else if (*p == ')' && nestedparen)
                                    nestedparen--;
                                *nm++ =  *p++;
                            }
                            while (nm != macro && isspace(nm[-1])) 
                            {
                                nm--;
                            }
                            *nm = 0;
                            size = strlen(macro) ;
                            args[count++] = string_copy(macro, strlen(macro));
                            rv = replacesegment(macro, macro + size, &insize, totallen,0, le);
                            if (rv <-MACRO_REPLACE_SIZE) {
                                return rv;
                            }
                            macro[rv+size] = 0;
                            args[count++] = string_copy(macro, strlen(macro));
                        }
                        while (*p && *p++ == ',' && count/2 != def->argcount-1)
                            ;
                    }
                    else 
                    {
                        count = 0;
                        while (isspace(*p)) p++;
                        if (*p == ')')
                            p++;
                    }
                    if (*(p - 1) != ')' || count/2 != def->argcount - 1)
                    {
                        if (count/2 == def->argcount-1 && prm_c99 && (def->flags & DS_VARARGS)) 
                        {
                            char *q = varargs ;
                            int nestedparen=0;
                            if (!(def->flags & DS_VARARGS))
                                return INT_MIN;
                            while (*p != '\n' && *p && (*p != ')' || nestedparen)) {
                                if (*p == '(')nestedparen++;
                                if (*p == ')' && nestedparen)
                                    nestedparen--;
                                *q++ = *p++;
                            }   
                            *q = 0 ;
                            p++ ;
                            count = (def->argcount - 1) *2;
                        }
                        if (*(p - 1) != ')' || count/2 != def->argcount - 1)
                        {
                            if (!*(p))
                                return  INT_MIN + 1;
							xprint("wrong macro args\n");
                            return  INT_MIN;
                        }
                    }
                    else if (def->flags & DS_VARARGS)
                    {
							xprint("wrong macro args\n");
                    }
                    strcpy(macro,def->string);
                    if (count/2 != 0 || varargs)
                        if (!defreplaceargs(macro, count/2, def->args, args, varargs)) {
                            return  INT_MIN;
                        }
                } else {
                    strcpy(macro,def->string);
                }
                deftokenizing(macro);
                if ((rv1 = definsert(start, p, q, macro, totallen -  *inbuffer,
                    p - q)) < -MACRO_REPLACE_SIZE)
                    return  rv1;
                insize = rv1 - (p - q);
                *inbuffer += insize;
                end += insize;
                p += insize;
                enterdefine(sym);
                rv = replacesegment(q, p, inbuffer, totallen, &p, le) ;
                exitdefine();
                if (rv <-MACRO_REPLACE_SIZE)
                    return rv;
                end += rv ;
            }
            else
            {
                join: defmacroreplace(macro, ascii, le);
                if (macro[0])
                {
                    if ((rv = definsert(start, p, q, macro, totallen -  *inbuffer, p -
                        q)) < - MACRO_REPLACE_SIZE)
                        return  rv;
                    end += rv - (p - q);
                    *inbuffer += rv - (p - q);
                    p += rv - (p-q); 
                }
            }
        }
        else
            p++;
    }
    if (pptr)
        *pptr = p ;
    return end - orig_end ;
}

/* Scan line for macros and do replacements */
int replace_macros(char *line, lex_env_t *le)
{
    char *p = line, *q = p;
    int inbuffer = strlen(line),rv;
    
    nodefines();
    return replacesegment(line, line + inbuffer, &inbuffer, MACRO_REPLACE_SIZE,0, le);
}

//-------------------------------------------------------------------------

static void repdefines(char *lptr)
/*
 * replace 'defined' keyword in #IF and #ELIF statements
 */
{
    char *q = lptr;
    char name[MAX_IDENTIFIER_LEN];
    char ascii[MAX_IDENTIFIER_LEN];
    while (*lptr)
    {
        if (!strncmp(lptr, "defined", 7))
        {
            int needend = FALSE;
            lptr += 7;
            while (isspace(*lptr))
                lptr++;
            if (*lptr == '(')
            {
                lptr++;
                needend = TRUE;
            }
            while (isspace(*lptr))
                lptr++;
            defid(name, &lptr, ascii);
            while (isspace(*lptr))
                lptr++;
            if (needend)
                if (*lptr == ')')
                    lptr++;
                else
					xprint( "defined keyword missing closepa\n");
			if (find_symbol(Definesymtab, name, FALSE) != NULL)
                *q++ = '1';
            else
                *q++ = '0';
            *q++ = ' ';

        }
        else
        {
            *q++ =  *lptr++;
        }
    }
    *q = 0;
}

//-------------------------------------------------------------------------

void pushif(lex_env_t *le)
/* Push an if context */
{
	ifskip_t *p = allocate(sizeof(ifskip_t));
	*p = *(le->ifskip);
	p->next = le->ifskip;
	le->ifskip = p;
	p->elsetaken = FALSE;
}

//-------------------------------------------------------------------------

void popif(lex_env_t *le)
/* Pop an if context */
{
    if (le->ifskip->next)
    {
		ifskip_t *tofree = le->ifskip;
		le->ifskip = le->ifskip->next;
    }
    else
    {
		
        le->ifskip->iflevel = 0;
        le->ifskip->elsetaken = 0;
    }
}


//-------------------------------------------------------------------------

lex_env_t * doifdef(int flag, const char *line, lex_env_t *le)
/* Handle IFDEF */
{
    symbol_t *sym;
    if (preprocessorSkip)
    {
        le->ifskip->iflevel++;
    }
	else
	{
		if (isalpha(*line) || *line == '_')
		{
			char buf[MAX_IDENTIFIER_LEN], *p = buf;
			while (isalnum(*line) || *line == '_')
				*p++ = *line++;
			*p = 0;
			pushif(le);
			sym = find_symbol(Definesymtab, buf, FALSE);
			if (sym && !flag || !sym && flag)
			{
				preprocessorSkip = TRUE;
			}		
		}
		else
		{
			xprint( "#ifdef - invalid indentifier\n");
		}
	}
	return le;
}

//-------------------------------------------------------------------------

lex_env_t * doif(int flag, const char *line, lex_env_t *le)
/* Handle #if */
{
    if (preprocessorSkip)
    {
		le->ifskip->iflevel++;
    }
	else
	{
		repdefines((char *)line);
    	replace_macros((char *)line, le);
	    pushif(le);
		if (!preproc_eval(&line))
			preprocessorSkip = TRUE;
		else
			le->ifskip->elsetaken = TRUE;
	}
	return le;
}

//-------------------------------------------------------------------------

lex_env_t * doelif(const char *line, lex_env_t *le)
/* Handle #elif */
{
    if (le->ifskip->iflevel)
    {
        return le;
    }
	else 
	{
	    if (!le->ifskip->elsetaken)
    	{
        	if (le->ifskip->next)
        	{
				repdefines((char *)line);
				replace_macros((char *)line, le);
            	if (!le->ifskip->iflevel)
                	preprocessorSkip = !preprocessorSkip || !preproc_eval(&line) 
								|| le->ifskip->elsetaken;
            	if (!preprocessorSkip)
                	le->ifskip->elsetaken = TRUE;
        	}
        	else
				xprint("#elif without if");
    	}
    	else
        	preprocessorSkip = TRUE;
	}
	return le;
}

/* handle else */
lex_env_t * doelse(const char *line, lex_env_t *le)
{
    if (!(le->ifskip->iflevel))
    {
	    if (le->ifskip->next)
	    {
	        if (!le->ifskip->next->iflevel)
	            preprocessorSkip = !preprocessorSkip || le->ifskip->elsetaken;
	    }
	    else
			xprint("#else without if");
	}
    return le;
}

/* HAndle endif */
lex_env_t * doendif(const char *line, lex_env_t *le)
{
    if (le->ifskip->iflevel)
    {
        --le->ifskip->iflevel;
    }
    else {
		preprocessorSkip = FALSE;
		if (!le->ifskip->next)
			xprint( "#endif without if");
	    else
			popif(le);
	}
	return le;
}
