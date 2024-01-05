/* 
XRC Resource Compiler
Copyright 1997, 1998 Free Software Foundation, Inc.
Written by Ian Lance Taylor, Cygnus Support.
Copyright 2006-2011 David Lindauer.

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

You may contact the author of this derivative at:
	mailto::camille@bluegrass.net
 */
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include "utype.h"
#include "cmdline.h"
#include "umem.h"
#include "preproc.h"
#include <time.h>
#include "interp.h"

#define MAX_STRLEN 257

extern int prm_packing;
extern int prm_nasm;
extern short inputline[];
extern FILE *inputFile;
extern TABLE *gsyms, defsyms;
extern long ival;
extern char laststr[];
extern HASHREC **defhash;
extern char *infile;
extern int incconst;
extern char *prm_searchpath;
extern int prm_cplusplus, prm_ansi;
extern short *lptr;
extern int cantnewline;
extern char *infile;
extern int backupchar;
extern int prm_cmangle;
extern enum e_sym lastst;
extern char lastid[];
extern char laststr[];
extern int lastch;
extern int lineno;
extern char *inputBuffer;
extern int inputLen;
extern char *ibufPtr;
extern char fname[];

typedef struct _startups_
{
    struct _startups_ *link;
    char *name;
    int prio;
} STARTUPS;
char *errfile;
int errlineno = 0;
IFSTRUCT *ifshold[10];
char *inclfname[10];
FILE *inclfile[10];
int incldepth = 0;
int inclline[10];
int inclhfile[10];
int inclInputLen[10];
char *inclInputBuffer[10];
char *inclibufPtr[10];
int inhfile;
short *lptr;
int inpp;
LIST *incfiles = 0,  *lastinc;

LIST *libincludes = 0;

IFSTRUCT *ifs = 0;
int ifskip = 0;
int elsetaken = 0;

void filemac(short *string);
void datemac(short *string);
void timemac(short *string);
void linemac(short *string);

static char *unmangid; /* In this module we have to ignore leading underscores
    */
static STARTUPS *startuplist,  *rundownlist;
static short defkw[] = 
{
    'd', 'e', 'f', 'i', 'n', 'e', 'd', 0
};
static int definelistcount;
static SYM *definelist[100]; /* Way deep but hey! */
static int skiplevel;
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

static void repdefines(short *lptr);

void pushif(void);

/* Moudle init */
void preprocini()
{
    skiplevel = 0;
    libincludes = 0;
    incldepth = 0;
    incfiles = 0;
    ifs = 0;
    ifskip = elsetaken = 0;
    unmangid = lastid;
    if (prm_cmangle)
        unmangid++;
    startuplist = rundownlist = 0;
}

/* Preprocessor dispatch */
int preprocess(void)
{
    ++lptr;
    lastch = ' ';
    getsym(); /* get first word on line */

    if (lastst != ident)
    {
        generror(ERR_IDEXPECT, 0, 0);
        return incldepth == 0;
    }
    if (strcmp(unmangid, "include") == 0)
        return doinclude(FALSE);
    else if (strcmp(unmangid, "define") == 0)
        return dodefine();
    else if (strcmp(unmangid, "endif") == 0)
        return doendif();
    else if (strcmp(unmangid, "else") == 0)
        return doelse();
    else if (strcmp(unmangid, "ifdef") == 0)
        return doifdef(TRUE);
    else if (strcmp(unmangid, "ifndef") == 0)
        return doifdef(FALSE);
    else if (strcmp(unmangid, "if") == 0)
    {
        repdefines(lptr);
        defcheck(lptr);
        return doif(0);
    }
    else if (strcmp(unmangid, "elif") == 0)
    {
        repdefines(lptr);
        defcheck(lptr);
        return doelif();
    }
    else if (strcmp(unmangid, "undef") == 0)
        return (doundef());
    else if (strcmp(unmangid, "error") == 0)
        return (doerror());
    else if (strcmp(unmangid, "pragma") == 0)
        return (dopragma());
    else if (strcmp(unmangid, "line") == 0)
        return (doline());
    else
    {
        if (!ifskip)
            gensymerror(ERR_PREPROCID, unmangid);
        return incldepth == 0;
    }
}

//-------------------------------------------------------------------------

int doerror(void)
{
    char *temp;
    int i = 0;
    if (ifskip)
        return incldepth == 0;
    temp = AllocateMemory(pstrlen(lptr) + 1);
    while (*lptr)
        temp[i++] = *lptr++;
    temp[i] = 0;
    basicerror(ERR_ERROR, temp);
    return incldepth == 0;
}

//-------------------------------------------------------------------------

static int pragerror(int errno)
{
    char buf[100],  *p = buf, i = 99;
    short *s = lptr;
    while (i-- &&  *s &&  *s != '\n')
        *p++ =  *s++;
    *p = 0;
    basicerror(errno, buf);
    return (incldepth == 0);
}

//-------------------------------------------------------------------------

int dopragma(void)
{
    char buf[40],  *p = buf;
    STARTUPS *a;
    int val = 0, sflag;
    if (ifskip)
        return incldepth == 0;
    cantnewline = TRUE;
    lineToCpp();
    getsym();
    cantnewline = FALSE;
    if (lastst != ident)
        return incldepth == 0;
    if (!strcmp(unmangid, "error"))
        return pragerror(ERR_USERERR);
    else if (!strcmp(unmangid, "warning"))
        return pragerror(ERR_USERWARN);
    else
        return incldepth == 0;

}

//-------------------------------------------------------------------------

int doline(void)
/*
 * Handle #line directive
 */
{
    int n;
    if (ifskip)
        return incldepth == 0;
    getsym();
    if (lastst != iconst)
        gensymerror(ERR_PREPROCID, "#line");
    else
    {
        n = ival;
        getsym();
        if (lastst != sconst)
            gensymerror(ERR_PREPROCID, "#line");
        else
        {
            errfile = litlate(laststr);
            errlineno = n - 1;
        }
    }
    return incldepth == 0;
}

//-------------------------------------------------------------------------

int doinclude(int unquoted)
/*
 * HAndle include files
 */
{
    int rv;
    FILE *oldfile = inputFile;
    incconst = TRUE;
    getsym(); /* get file to include */
    incconst = FALSE;
    if (ifskip)
        return incldepth == 0;
    if (!unquoted)
    {
        if (lastst != sconst)
        {
            gensymerror(ERR_INCLFILE, "include");
            return incldepth == 0;
        }
        strcpy(fname, laststr);
    }
    else if (lastst == ident)
        getfilename();
    if (incldepth > 9)
    {
        generror(ERR_PREPROCID, 0, 0);
        return incldepth == 0;
    }
    
    inputFile = SearchPath(fname, prm_searchpath, "r");
    if (inputFile == 0)
    {
        gensymerror(ERR_CANTOPEN, fname);
        inputFile = oldfile;
        rv = incldepth == 0;
    }
    else
    {
        LIST *list;
        pushif();
        ifshold[incldepth] = ifs;
        elsetaken = 0;
        ifskip = 0;
        ifs = 0;
        inclhfile[incldepth] = inhfile;
        inclline[incldepth] = lineno;
        inclfile[incldepth] = oldfile; /* push current input file */
		inclInputLen[incldepth] = inputLen;
		inclInputBuffer[incldepth] = inputBuffer;
		inclibufPtr[incldepth] = ibufPtr;
        inclfname[incldepth++] = infile;
		inputLen = 0;
		inputBuffer = calloc(INPUT_BUFFER_LEN , 1);
        infile = litlate(fname);
        list = AllocateMemory(sizeof(LIST));
        list->data = infile;
        list->link = 0;
        if (incfiles)
            lastinc = lastinc->link = list;
        else
            incfiles = lastinc = list;
        errfile = infile;
        errlineno = 0;
        rv = incldepth == 1;
        lineno = 0;
        inhfile = !stricmp(infile + strlen(infile) - 2, ".h");
    }
    return rv;
}

//-------------------------------------------------------------------------

short *plitlate(short *string)
{
    short *temp = AllocateMemory(pstrlen(string) *sizeof(short) + sizeof(short))
        ;
    pstrcpy(temp, string);
    return temp;
}

//-------------------------------------------------------------------------

void glbdefine(char *name, char *value)
{
    {
        SYM *sp;
        short *p;
        DEFSTRUCT *def;
        if ((sp = search(name, &defsyms)) != 0)
            return ;
        sp = AllocateMemory(sizeof(SYM));
        sp->name = litlate(name);
        def = AllocateMemory(sizeof(DEFSTRUCT));
        def->args = 0;
        def->argcount = 0;
        def->string = p = AllocateMemory(strlen(value) *sizeof(short) + 2);
        while (*value)
            *p++ =  *value++;
        *p++ = 0;
        sp->value.s = (char*)def;
        insert(sp, &defsyms);
        return ;
    }
}

/* Handle #defines
 * Doesn't check for redefine with different value
 * Does handle ANSI macros
 */
int dodefine(void)
{
    SYM *sp;
    DEFSTRUCT *def;
    short *args[40], count = 0;
    short *olptr;
    int p;
    getsym(); /* get past #define */
    if (ifskip)
        return incldepth == 0;
    olptr = lptr;
    if (lastst != ident)
    {
        generror(ERR_IDEXPECT, 0, 0);
        return incldepth == 0;
    }
    if ((sp = search(unmangid, &defsyms)) != 0)
        undef2();
    sp = AllocateMemory(sizeof(SYM));
    sp->name = litlate(unmangid);
    def = AllocateMemory(sizeof(DEFSTRUCT));
    def->args = 0;
    def->argcount = 0;
    if (lastch == '(')
    {
        getdefsym();
        getdefsym();
        while (lastst == ident)
        {
            args[count++] = plitlate(unmangid);
            getdefsym();
            if (lastst != comma)
                break;
            getdefsym();
        }
        if (lastst != closepa)
            generror(ERR_PUNCT, closepa, 0);
        olptr = lptr - 1;
        def->args = AllocateMemory(count *sizeof(short*));
        memcpy(def->args, args, count *sizeof(short*));
        def->argcount = count + 1;
    }
    while (iswhitespacechar(*olptr))
        olptr++;
    p = pstrlen(olptr);
    if (olptr[p - 1] == 0x0a)
        olptr[p - 1] = 0;
    def->string = plitlate(olptr);
    sp->value.s = (char*)def;
    insert(sp, &defsyms);
    return incldepth == 0;
}

/*
 * Undefine
 */
int doundef(void)
{
    getsym();
    if (!ifskip)
        undef2();
    return (incldepth == 0);
}

//-------------------------------------------------------------------------

int undef2(void)
{
    if (lastst != ident)
        generror(ERR_IDEXPECT, 0, 0);
    else
    {
        SYM **p = (SYM **)LookupHash(unmangid, defhash, HASHTABLESIZE);
        if (p)
        {
            *p = (*p)->next;
        }
    }
}

//-------------------------------------------------------------------------

void getdefsym(void)
{
    if (backupchar !=  - 1)
    {
        lastst = backupchar;
        backupchar =  - 1;
        return ;
    }
    restart:  /* we come back here after comments */
    while (iswhitespacechar(lastch))
        getch();
    if (lastch ==  - 1)
        lastst = eof;
    else if (isdigit(lastch))
        getnum();
    else if (isstartchar(lastch))
    {
        lptr--;
        defid(unmangid, &lptr, 0);
        lastch =  *lptr++;
        lastst = ident;
    }
    else if (getsym2())
        goto restart;
}

//-------------------------------------------------------------------------

int defid(short *name, short **p, char *q)
/*
 * Get an identifier during macro replacement
 */
{
    int count = 0, i = 0;
    while (issymchar(**p))
    {
        if (count < 100)
        {
            name[count++] = *(*p);
            if (q)
				q[i++] = **p;
        }
        (*p)++;
    }
    if (q)
    {
        q[i] = '\0';
    }
    name[count] = 0;
    return (count);
}

/* 
 * Insert a replacement string
 */
int definsert(short *end, short *begin, short *text, int len, int replen)
{
    short *q;
    int i, p, r;
    int val;
    if (begin != inputline)
    if (*text == '#')
    {
        if (*(text - 1) != '#')
        {
            text++;
            while (*text == ' ')
                text++;
            begin--;
            replen++;
            r = pstrlen(text);

            text[r++] = '\"';
            text[r] = 0;
            for (i = r; i >= 0; i--)
                text[i + 1] = text[i];
            *text = '\"';
        }
    }
    p = pstrlen(text);
    val = p - replen;
    r = pstrlen(begin);
    if (val + (int)strlen(begin) + 1 >= len)
    {
        generror(ERR_MACROSUBS, 0, 0);
        return ( - 8000);
    }
    if (val > 0)
        for (q = begin + r + 1; q >= end; q--)
            *(q + val) =  *q;
        else
    if (val < 0)
    {
        r = pstrlen(end) + 1;
        for (q = end; q < end + r; q++)
                *(q + val) =  *q;
    }
    for (i = 0; i < p; i++)
        begin[i] = text[i];
    return (p);
}

/* replace macro args */
int defreplace(short *macro, int count, short **oldargs, short **newargs)
{
    int i, rv;
    int instring = 0;
    short narg[1024];
    short name[100];
    short *p = macro,  *q;
    while (*p)
    {
        if (*p == instring)
            instring = 0;
        else if (*p == '\'' ||  *p == '"')
            instring =  *p;
        else if (!instring && isstartchar(*p))
        {
            q = p;
            defid(name, &p, 0);
            for (i = 0; i < count; i++)
            if (!pstrcmp(name, oldargs[i]))
            {
                pstrcpy(narg, newargs[i]);
                if ((rv = definsert(p, q, narg, 1024-(q - macro), p - q)) ==  -
                    8000)
                    return (FALSE);
                else
                {
                    p = q + rv;
                    break;
                }
            }
        }
        p++;
    }
    return (TRUE);
}

/* Handlers for default macros */
void cnvt(short *out, char *in)
{
    while (*in)
        *out++ =  *in++;
    *out = 0;
}

//-------------------------------------------------------------------------

void filemac(short *string)
{
    char str1[40];
    sprintf(str1, "\"%s\"", infile);
    cnvt(string, str1);
}

//-------------------------------------------------------------------------

void datemac(short *string)
{
    char str1[40];
    struct tm *t1;
    time_t t2;
    time(&t2);
    t1 = localtime(&t2);
    strftime(str1, 40, "\"%b %d %Y\"", t1);
    cnvt(string, str1);
}

//-------------------------------------------------------------------------

void timemac(short *string)
{
    char str1[40];
    struct tm *t1;
    time_t t2;
    time(&t2);
    t1 = localtime(&t2);
    str1[0] = '"';
    strftime(str1, 40, "\"%X\"", t1);
    cnvt(string, str1);
}

//-------------------------------------------------------------------------

void linemac(short *string)
{
    char str1[40];
    sprintf(str1, "%d", lineno);
    cnvt(string, str1);
} 
/* Scan for default macros and replace them */
void defmacroreplace(short *macro, short *name)
{
    int i;
    macro[0] = 0;
    for (i = 0; i < INGROWNMACROS; i++)
    if (!strcmp(name, ingrownmacros[i].s))
    {
        (ingrownmacros[i].func)(macro);
        break;
    }
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

int indefine(SYM *sp)
{
    int i;
    for (i = 0; i < definelistcount; i++)
        if (sp == definelist[i])
            return TRUE;

    return FALSE;
}

//-------------------------------------------------------------------------

void enterdefine(SYM *sp)
{
    definelist[definelistcount++] = sp;
}

//-------------------------------------------------------------------------

void exitdefine(void)
{
    definelistcount--;
}

//-------------------------------------------------------------------------

int replacesegment(short *start, short *end, int *inbuffer, int totallen, int
    *changed)
{
    short macro[1024];
    short name[1024];
    short *args[40];
    char ascii[60];
    int waiting = FALSE, rv;
    short *p,  *q;
    SYM *sp;
    p = start;
    while (p < end)
    {
        q = p;
        if (*p == '"')
        {
            waiting = !waiting;
            p++;
        }
        else if (waiting)
            p++;
        else if (isstartchar(*p))
        {
            defid(name, &p, ascii);
            if ((sp = search(ascii, &defsyms)) != 0 && !indefine(sp))
            {
                DEFSTRUCT *def = sp->value.s;
                enterdefine(sp);
                pstrcpy(macro, def->string);
                if (def->argcount)
                {
                    int count = 0;
                    short *q = p;
                    while (iswhitespacechar(*q))
                        q++;
                    if (*q++ != '(')
                    {
                        exitdefine();
                        goto join;
                    }
                    p = q;
                    if (def->argcount > 1)
                    {
                        do
                        {
                            short *nm = name;
                            int nestedparen = 0;
                            int nestedstring = 0;
                            while (((*p != ',' &&  *p != ')') || nestedparen ||
                                nestedstring) &&  *p != '\n')
                            {
                                if (*p == '(')nestedparen++;
                                if (*p == ')' && nestedparen)
                                    nestedparen--;
                                if (nestedstring)
                                {
                                    if (*p == nestedstring)
                                        nestedstring = 0;
                                }
                                else if (*p == '\'' ||  *p == '"')
                                    nestedstring =  *p;
                                *nm++ =  *p++;
                            }
                            while (iswhitespacechar(*(nm - 1)))
                                nm--;
                            *nm = 0;
                            nm = name;
                            while (iswhitespacechar(*nm))
                                nm++;
                            args[count++] = plitlate(nm);
                        }
                        while (*p++ == ',')
                            ;
                    }
                    else
                        while (iswhitespacechar(*p++))
                            ;
                    if (*(p - 1) != ')' || count != def->argcount - 1)
                    {
                        if (*(p - 1) == '\n')
                            return  - 10;
                        gensymerror(ERR_WRONGMACROARGS, ascii);
                        return  - 1;
                    }
                    /* Can't replace if tokenizing next */
                    if (*p == '#' && *(p + 1) == '#')
                        continue;
                    if (count == 0)
                        goto insert;
                    if (!defreplace(macro, count, def->args, args))
                        return  - 1;
                }
                insert: if ((rv = definsert(p, q, macro, totallen -  *inbuffer,
                    p - q)) ==  - 8000)
                    return  - 1;
                *changed = TRUE;
                rv = replacesegment(q, q + rv, inbuffer, totallen, changed);
                if (rv < 0)
                    return rv;
                end += rv - (p - q);
                *inbuffer += rv - (p - q);
                p = q + rv;
                exitdefine();
                *changed = TRUE;
            }
            else
            {
                join: defmacroreplace(macro, ascii);
                if (macro[0])
                {
                    if ((rv = definsert(p, q, macro, totallen -  *inbuffer, p -
                        q)) ==  - 8000)
                        return  - 1;
                    *changed = TRUE;
                    end += rv - (p - q);
                    *inbuffer += rv - (p - q);
                    p = q + rv;
                }
            }
        }
        else
            p++;
    }
    return (int)(end - start);
}

/* Scan line for macros and do replacements */
int defcheck(short *line)
{
    int x;
    short *p = line,  *q;
    int inbuffer = pstrlen(line);
    int changed = FALSE;
    nodefines();
    if ((x = replacesegment(line, line + inbuffer, &inbuffer, 4096, &changed))
        < 0)
        return x;

    /* Token pasting */
    if (changed)
    {
        p = q = line;
        while (*p)
        {
            if (*p == '#' && *(p + 1) == '#')
                p += 2;
            else
                *q++ =  *p++;
        }
        *q = 0;
    }
    return 0;
}

//-------------------------------------------------------------------------

static void repdefines(short *lptr)
/*
 * replace 'defined' keyword in #IF and #ELIF statements
 */
{
    short *q = lptr;
    short name[40];
    char ascii[60];
    while (*lptr)
    {
        if (!pstrncmp(lptr, defkw, 7))
        {
            int needend = FALSE;
            lptr += 7;
            if (*lptr == '(')
            {
                lptr++;
                needend = TRUE;
            }
            while (iswhitespacechar(*lptr))
                lptr++;
            defid(name, &lptr, ascii);
            while (iswhitespacechar(*lptr))
                lptr++;
            if (needend)
                if (*lptr == ')')
                    lptr++;
                else
                    expecttoken(closepa, 0);
            if (search(ascii, &defsyms) != 0)
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

void pushif(void)
/* Push an if context */
{
    IFSTRUCT *p;
    p = AllocateMemory(sizeof(IFSTRUCT));
    p->link = ifs;
    p->iflevel = ifskip;
    p->elsetaken = elsetaken;
    elsetaken = FALSE;
    ifs = p;
}

//-------------------------------------------------------------------------

void popif(void)
/* Pop an if context */
{
    if (ifs)
    {
        ifskip = ifs->iflevel;
        elsetaken = ifs->elsetaken;
        ifs = ifs->link;
    }
    else
    {
        ifskip = 0;
        elsetaken = 0;
    }
}

//-------------------------------------------------------------------------

void ansieol(void)
{
    if (prm_ansi)
    {
        while (iswhitespacechar(*lptr))
            lptr++;
        if (*lptr)
        {
            lastch =  *lptr;
            lastst = ident;
            generror(ERR_UNEXPECT, 0, 0);
        }
    }
}

//-------------------------------------------------------------------------

int doifdef(int flag)
/* Handle IFDEF */
{
    SYM *sp;
    if (ifskip)
    {
        skiplevel++;
        return (incldepth == 0);
    }
    getch();
    while (isspace(lastch))
        getch();
    if (!isstartchar(lastch))
    {
        generror(ERR_IDEXPECT, 0, 0);
        return incldepth == 0;
    }
    else
        getid();
    sp = search(unmangid, &defsyms);
    pushif();
    if (sp && !flag || !sp && flag)
        ifskip = TRUE;
    ansieol();
    return (incldepth == 0);
}

//-------------------------------------------------------------------------

int doif(int flag)
/* Handle #if */
{
    if (ifskip)
    {
        skiplevel++;
        return (incldepth == 0);
    }
    cantnewline = TRUE;
    getsym();
    pushif();
    inpp = TRUE;
    if (!intexpr())
        ifskip = TRUE;
    else
        elsetaken = TRUE;
    inpp = FALSE;
    cantnewline = FALSE;
    ansieol();
    return (incldepth == 0);
}

//-------------------------------------------------------------------------

int doelif(void)
/* Handle #elif */
{
    int is;
    if (skiplevel)
    {
        return (incldepth == 0);
    }

    cantnewline = TRUE;
    getsym();
    inpp = TRUE;
    is = !intexpr();
    inpp = FALSE;
    cantnewline = FALSE;
    if (!elsetaken)
    {
        if (ifs)
        {
            if (!ifs->iflevel)
			{
				int oldifskip = ifskip;
                ifskip = !ifskip || is || elsetaken;
				if (!oldifskip || !ifskip)
					elsetaken = TRUE;
			}
        }
        else
            generror(ERR_PREPROCMATCH, 0, 0);
    }
    else
    {
        ifskip = TRUE;
        elsetaken = TRUE;
    }
    ansieol();
    return (incldepth == 0);
}

/* handle else */
int doelse(void)
{
    if (skiplevel)
    {
        return (incldepth == 0);
    }
    if (ifs)
    {
        if (!ifs->iflevel)
            ifskip = !ifskip || elsetaken;
    }
    else
        generror(ERR_PREPROCMATCH, 0, 0);
    ansieol();
    return (incldepth == 0);
}

/* HAndle endif */
int doendif(void)
{
    if (skiplevel)
    {
        skiplevel--;
        return (incldepth == 0);
    }
    if (!ifs)
        generror(ERR_PREPROCMATCH, 0, 0);
    popif();
    ansieol();
    return (incldepth == 0);
}
