/* 
IMAKE make utility
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
#include <stdio.h>
#include <memory.h>
#include <string.h>
#include <dos.h>
#include "utype.h"
#include "cmdline.h"
#include "interp.h"
#include "umem.h"
#include "register.h"
#include "maker.h"
#include "phash.h"
#include "input.h"
#include "imake.h"
#include "macros.h"
#include "module.h"

extern LIST *targetlist;
extern BOOL prm_buildall;
extern BOOL prm_displaystamp;
extern BOOL prm_autodepends ;

extern HASHREC **hashtable;

static char *months[] = 
{
    "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov",
        "Dec"
};

static void nxtarget(short *target)
{
    char buf[200];
    CompressFile(buf, target);
    fatal("Target %s - non-existant with no rules", buf);
}

//-------------------------------------------------------------------------

void CompressFile(char *out, short *in)
{
    do
    {
        *out++ =  *in++ &0xff;
    }
    while (*in);
    *out = 0;
}

//-------------------------------------------------------------------------

void ExpandFile(short *out, char *in)
{
    do
    {
        *out++ =  *in++;
    }
    while (*in);
    *out = 0;
}

//-------------------------------------------------------------------------

static unsigned long getfiletime(short *file)
{
    char buf[200];
    uint handle, date, time;
    CompressFile(buf, file);
    _dos_open(buf, 0, &handle);
    if (handle == 0)
        return (0);
    _dos_getftime(handle, &date, &time);
    _dos_close(handle);
    return (((unsigned long)(date) << 16) + time);
}

//-------------------------------------------------------------------------

FILE *GetPath(short *name)
{
    REGISTRY **r;
    char buf[4096];
    char searchpath[4096];
    short *p = pstrrchr(name, '.');
    short pathext[40];
    FILE *s;
    searchpath[0] = 0;
    if (p && *(p - 1) != '.')
    {
        pstrcpy(pathext, L".path");
        pstrcat(pathext, p);
        r = LookupPhiHash(hashtable, pathext);

        if (r && (*r)->type == R_PATH)
        {
            CompressFile(searchpath, (*r)->x.path);
        }
    }
    CompressFile(buf, name);
    s = SrchPth(buf, searchpath, "r");
    ExpandFile(name, buf);
    return s;
}

void AutoDep(short *string)
{
	short *q, buf[4096], *p;
	short buf2[INTERNALBUFSIZE], * bp=buf2;
	if (!prm_autodepends)
		return ;
	pstrcpy(buf2, string);
	ExpandString(buf2);
	while (bp[0])
	{
		REGISTRY *r;
		q = bp;
		while (*q && !isspace(*q))
			q++ ;
		if (*q) 
		{
			*q++ = 0;
			while(isspace(*q))
				q++;
		}
		pstrcpy(buf,bp);
		p = pstrrchr(buf, '.');
		if (p && *(p-1) != '.')
			*p = 0;
		pstrcat(buf, L".obj");
		p = pstrrchr(buf,'\\');
		if (p)
			p++;
	    r = LookupPhiHash(hashtable, p ? p : buf);

		if (!r)
		{
			FILE *s = GetPath(buf);
			if (s)
			{
				static short *dummy=0;
				short *depends ;
				fclose(s);
				depends = GetAutoDepends(buf);
				if (depends && depends[0])
				{
					RegisterExplicitName(p ? p : buf, depends);	
			        RegisterExplicitCommands(p ? p : buf, &dummy);
					AutoDep(depends);
				}
			}
		}
		bp = q ;
	}
}
void AllAutoDeps(void)
{
    int i;
    REGISTRY *x;
    for (i = 0; i < HASH_TABLE_SIZE; i++)
    {
        x = hashtable[i];
        while (x)
        {
		 	if (x->type == R_EXPLICIT)
				AutoDep(x->depends);
            x = x->link;
        }
    }
}
//-------------------------------------------------------------------------

FILEREC *FileData(short *string)
{
    short pathext[40],  *p;
    short buf2[INTERNALBUFSIZE], buf[4096];
    BOOL exists = FALSE, explicit = FALSE, implicit = FALSE;
	short *depends = 0,  *depends1 = 0,  *target,  *path = 0;
    FILE *s;
    REGISTRY **r = LookupPhiHash(hashtable, string);
    if (r && (*r)->type == R_EXPLICIT)
    {
        explicit = TRUE;
        pstrcpy(buf2, (*r)->depends);
        ExpandString(buf2);
        depends = AllocateMemory(pstrlen(buf2) *2+2);
        pstrcpy(depends, buf2);
    }
    target = AllocateMemory(pstrlen(string) *2+2);
    pstrcpy(target, string);
    pstrcpy(buf, string);
    s = GetPath(buf);

    if (s)
    {
        fclose(s);
        exists = TRUE;
        path = AllocateMemory(pstrlen(buf) *2+2);
        pstrcpy(path, buf);
    }
    else
    {
        path = AllocateMemory(pstrlen(string) *2+2);
        pstrcpy(path, string);
    }
    pstrcpy(buf, string);
    p = pstrrchr(buf, '.');
    if (p && (!explicit || (*r)->x.commands == 0 || (*r)->x.commands[0] == 0))
    {
        if (*(p - 1) != '.')
        {
            pstrcpy(pathext, p);
            r = LookupPhiHash(hashtable, pathext);
            if (r && (*r)->type == R_DEST)
            {
                short *bp,  *t,  *d;
                pstrcpy(buf, target);
                bp = pstrrchr(buf, PERIOD);
                s = 0;
                if (*(bp - 1) != '.')
                {
                    d = (*r)->x.path;
                    while (*d)
                    {
                        t = bp;
                        do
                        {
                            *t++ =  *d++;
                        }
                        while (*d &&  *d != ';');
                        if (*d)
                            d++;
                        *t = 0;
                        pstrcpy(buf2, buf);
                        s = GetPath(buf2);
                        if (s)
                        {
                            fclose(s);
                            break;
                        }
                    }
                }
                DeallocateMemory(target);
                p = pstrrchr(string, '.');
                target = AllocateMemory((pstrlen(bp) + pstrlen(p)) *2+2);
                pstrcpy(target, bp);
                pstrcat(target, p);
                /* Note that this implementation depends on the name being
                given in the MB record of the obje file */
                /* hacked to allow recursion on non-existing targets.
                 * Some ambiguity exists though in the presence of multiple
                 * source resulting in the same obj for example...
                 */
                /*					if (s) { */
                if (depends1)
                {
                    pstrcat(buf2, L" ");
                    pstrcat(buf2, depends1);
                }
                if (depends)
                {
                    pstrcat(buf2, L" ");
                    pstrcat(buf2, depends);
                }
                depends = AllocateMemory(pstrlen(buf2) *2+2);
                pstrcpy(depends, buf2);
                /*					}
                else {
                depends = AllocateMemory(2);
                depends[0] = 0;
                }
                 */
                implicit = TRUE;
            }
        }
    }
    if (exists || explicit || implicit)
    {
        FILEREC *rv = AllocateMemory(sizeof(FILEREC));
        rv->link = 0;
        rv->parent = 0;
        rv->targetable = explicit || implicit;
        rv->implicit = implicit != 0;
        rv->exists = exists;
        rv->depends = depends;
        rv->target = target;
        rv->path = path;
        rv->timedout = FALSE;
	    if (exists)
            rv->time = getfiletime(path);
        else
            rv->time = 0xffffffffL;
        return (rv);
    }
    else
    {
        DeallocateMemory(depends);
        DeallocateMemory(target);
        DeallocateMemory(path);
    }
    return (0);
}

//-------------------------------------------------------------------------

void ReleaseFile(FILEREC *file)
{
    DeallocateMemory(file->depends);
    DeallocateMemory(file->target);
    DeallocateMemory(file->path);
    DeallocateMemory(file);
}

//-------------------------------------------------------------------------

FILEREC *RecurseTarget(short *target)
{
    FILEREC *q = FileData(target),  *r,  *rv = q,  *foundimp = 0;
    short *d,  *de;
    if (!q)
        return (0);
    if (q->targetable)
    {
        d = q->depends;		
        de = d + pstrlen(d);
        while (de > d)
        {
            FILEREC *s;
            *de = 0;
            de--;
            while (de > d && iswhitespacechar(*de))
                de--;
            if (de >= d)
            {
                *(de + 1) = 0;
                do
                {
                    de--;
                }
                while (de >= d && !iswhitespacechar(*de));
            }
            if (de >= d - 1)
            {
                r = RecurseTarget(de + 1);
                if (r)
                {
                    if (de == d - 1)
                        foundimp = r;
                    s = r;
                    while (s->link)
                        s = s->link;
                    s->parent = q;
                    s->link = rv;
                    rv = r;
                }
                else
                    if (!q->implicit)
                        nxtarget(de + 1);
            }
        }
        if (q->implicit)
        {
            short *p = pstrchr(q->depends, ' ');
            if (p)
                *p = 0;
            if (foundimp)
            {
                FILEREC *s;
                if (foundimp && foundimp->depends)
                {
                    short *p = pstrchr(foundimp->depends, ' ');
                    if (p)
                        *p = 0;
                }
                r = RecurseTarget(q->target);
                if (r)
                {
                    s = r;
                    while (s->link)
                        s = s->link;
                    s->parent = q;
                    s->link = rv;
                    rv = r;
                }
            }
            else
            {
                if (q->exists)
                    q->targetable = FALSE;
                else
                {
                    ReleaseFile(q);
                    rv = 0;
                }
            }
        }
    }

    return (rv);
}

//-------------------------------------------------------------------------

BOOL matches(short *string, short *wild)
{
    int i;
    while (*string)
    {
        switch (*wild)
        {
            case STAR:
                {
                    int len = strlen(string) - strlen(++wild);
                    char buf[40];
                    for (i = 0; i < len; i++)
                    {
                        int j;
                        for (j = 0; j < i; j++)
                            buf[j] = '?';
                        strcat(&buf[j], wild);
                        if (matches(string, buf))
                            return (TRUE);
                    }
                }
                return (FALSE);
            case QMARK:
                string++;
                wild++;
                break;
            default:
                if (*string !=  *wild)
                    return (FALSE);
        }
    }
    while (*wild++)
    {
        if (*wild != STAR &&  *wild != QMARK)
            return (FALSE);
    }
    return (TRUE);
}

//-------------------------------------------------------------------------

void Stamp(FILEREC *rec, char *string)
{
    if (prm_displaystamp)
    {
        char buf[200];
        CompressFile(buf, rec->path);
        printf("%-30s:%s ", buf, string);
        if (rec->exists)
        {
            int month, day, year, hour, minute, second;
            year = (rec->time >> 25) + 1980;
            month = (rec->time >> 21) &0xf;
            day = (rec->time >> 16) &0x1f;
            hour = (rec->time >> 11) &0x1f;
            minute = (rec->time >> 5) &0x3f;
            second = (rec->time &0x1f) *2;
            printf("%2d-%s-%4d  %02d:%02d:%02d\n", day, months[month - 1], year,
                hour, minute, second);
        }
        else
            printf("No file\n");
    }
}

//-------------------------------------------------------------------------

FILEREC *Weed(FILEREC *list)
{
    FILEREC *p = list,  **q,  *r;

    while (p)
    {
        if (!p->exists)
            p->timedout = TRUE;
        if (p->parent && (p->time > p->parent->time || p->timedout ||
            prm_buildall))
            p->parent->timedout = TRUE;
        p = p->link;
    }

    p = list;
    while (p)
    {
        q = &(p->link);
        while (*q)
        {
            if (!pstrcmp((*q)->path, p->path))
            {
                r = (*q);
                (*q) = r->link;
                ReleaseFile(r);
            }
            q =  *q;
        }
        p = p->link;
    }

    p = list;
    while (p)
    {
        if (p->timedout)
            Stamp(p, "*");
        else
            Stamp(p, " ");
        p = p->link;
    }

    p = list;
    q = &p;
    while (*q)
    {
        while (*q && !(*q)->timedout)
        {
            r =  *q;
            *q = r->link;
            ReleaseFile(r);
        }
        while (*q && (*q)->timedout)
            q =  *q;
    }


    return (p);
}

//-------------------------------------------------------------------------

FILEREC *RecurseAllTargets(void)
{
    LIST *p = targetlist;
    FILEREC *rv = 0;
    targetlist = 0;
    if (!p)
        return (0);
    while (p)
    {
        short *c = p->data;
        FILEREC **s,  *q;
        if (!pstrchr(c, '*') && !pstrchr(c, '?'))
        {
            q = RecurseTarget(c);
            if (!q)
                nxtarget(c);
            s = &rv;
            while (*s)
                s =  *s;
            *s = q;
        }
        else
        {
            int i;
            REGISTRY *x;
            for (i = 0; i < HASH_TABLE_SIZE; i++)
            {
                x = hashtable[i];
                while (x && x->type == R_EXPLICIT)
                {
                    if (matches(x->name, c))
                    {
                        q = RecurseTarget(c);
                        s = &rv;
                        while (*s)
                            s =  *s;
                        *s = q;
                    }
                    x = x->link;
                }
            }
        }
        p = p->link;
    }
    return (Weed(rv));
}
