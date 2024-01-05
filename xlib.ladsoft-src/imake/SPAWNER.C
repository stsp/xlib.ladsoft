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
#include <process.h>
#include <string.h>
#include "utype.h"
#include "cmdline.h"
#include "umem.h"
#include "register.h"
#include "macros.h"
#include "maker.h"
#include "input.h"
#include "spawner.h"
#include "interp.h"
#include "phash.h"

extern BOOL prm_ignorestatus;
extern BOOL prm_keep;
extern BOOL prm_nospawn;
extern BOOL prm_suppressdisplay;
extern HASHREC **hashtable;
extern int makerv;

int tempnum = 0;
short xandand[] = 
{
    0x26, 0x26, 0
};
short xspace[] = 
{
    0x20, 0
};
LIST *tempfiles = 0;

FILE *CreateFile(void)
{
    FILE *rv;
    char buffer[20];
    sprintf(buffer, "MAKEFILE.%03d", tempnum++);
    rv = fopen(buffer, "w");
    if (rv)
    {
        short *val = AllocateMemory(strlen(buffer) *2+2);
        LIST *p = AllocateMemory(sizeof(LIST));
        ExpandFile(val, buffer);
        p->data = val;
        p->link = tempfiles;
        tempfiles = p;
    }
    return (rv);
}

//-------------------------------------------------------------------------

void CloseFile(FILE *fh)
{
    fclose(fh);
}

//-------------------------------------------------------------------------

void DeleteFiles(void)
{
    if (!prm_keep)
    {
        LIST *p = tempfiles;
        char buffer[200];
        while (p)
        {
            LIST *q = p->link;
            CompressFile(buffer, p->data);
            DeallocateMemory(p->data);
            DeallocateMemory(p);
            unlink(buffer);
            p = q;
        }
    }
}

//-------------------------------------------------------------------------
char *GetExeName(char *out, char *p)
{
	int end = ' ';
	while (*p)
	{
		if (*p == end)
			if (end == ' ')
			{
				p++;
				break;
			}
			else
			{
				p++;
				end = ' ';
			}
		else if (*p == '"')
		{
			end = *p++;
		}
		else
		{
			*out++ = *p++;
		}
	}
	*out = 0;
	return p;
}
int SpawnOne(short *command)
{
    int rv = 0;
    char buf[510],  *a;
    CompressFile(buf + 3, command);
    if (!prm_suppressdisplay)
    {
        printf("\t%s\n", buf + 3);
    }
    if (!prm_nospawn)
    {
        fflush(stdout);
        buf[0] = ' ';
        buf[1] = '/';
        buf[2] = 'C';
        if ((buf[3] == 'c' || buf[3] == 'C') && (buf[4] == 'd' || buf[4] == 'D')
            )
        {
            char *p = buf + 5;
            while (*p == '\t' ||  *p == ' ')
                p++;
            #ifndef TEST
                chdir(p);
            #endif 
        }
        else
        {
			char buf1[260], *q;
			int done = FALSE;
			q = GetExeName(buf1, buf + 3);
			if (!strchr(q,'>') && !strchr(q,'<') && !strchr(q, '|'))
			{
				// otherwise may be an internal command
				if (searchpath(buf1))
				{
					rv = spawnlp(P_WAIT, buf1, buf1, q, 0);
					done = TRUE;
				}
			}
			if (!done)
			{
	            a = getenv("COMSPEC");
    	        rv = spawnlp(P_WAIT, a, a, buf, 0);
			}
        }
    }
    if (prm_ignorestatus)
        rv = 0;
    return (rv);
}

//-------------------------------------------------------------------------

void InsertString(short *buffer, short *new, short len)
{
    if (len < 0)
    {
        short *t,  *p;
        pstrcpy(buffer, new);
        p = buffer + pstrlen(buffer);
        t = buffer - len;
        do
        {
            *p++ =  *t++;
        }
        while (*(t - 1));
    }
    else
    {
        short *t,  *p;
        p = buffer + pstrlen(buffer);
        t = p + len;
        do
        {
            *t-- =  *p--;
        }
        while (p >= buffer);
        while (*new)
            *buffer++ =  *new++;
    }
}

//-------------------------------------------------------------------------

void us(char *string, char data)
{
    char buf[10];
    if (string)
        strcpy(buf, string);
    else
    {
        buf[0] = data;
        buf[1] = 0;
    }
    fatal("Invalid macro expansion $%s", buf);
}

/* FIXME - if only one flag is set and we have multiple specifications on a line
 * it won't work right
 */
short *ExpandFiles(short **obuf, short **ibuf, int *len, short **files, int
    flags, short **implicit, BOOL onlyone)
{
    short path[260],  *p,  *q,  *ptr =  *files,  *oldptr =  *files;
    short num = 10000;
	BOOL continued = FALSE;
    if (! *files)
        return  *files;
    if (onlyone)
        num = 1;
    while (num--)
    {
        while (*ptr == ' ')
            ptr++;
        p =  *files;
        q = path;
        while (*ptr &&  *ptr != ' ')
            *q++ =  *ptr++;
        *q = 0;
        if (!(flags &EXT))
        {
            p = pstrrchr(path, PERIOD);
            if (p && *(p - 1) != PERIOD)
                *p = 0;
        }
        if (!(flags &NAME))
        {
            p = pstrrchr(path, '\\');
            if (!p)
                *path = 0;
            else
                *(p + 1) = 0;
        }
        if (!(flags &PATH))
        {
            p = pstrrchr(path, '\\');
            q = path;
            if (p)
            {
                p++;
                while (*p)
                    *q++ =  *p++;
                *q = 0;
            }
        }
        if (flags &TARGEXT)
        {
            p = implicit + 1;
            q = path + pstrlen(path);
            while (*p &&  *p != '.')
                p++;
            while (*p)
                *q++ =  *p++;
            *q = 0;
        }
        if (path[0])
        {
            short *q = path + pstrlen(path);
            while (**ibuf &&  **ibuf != ' ' &&  **ibuf != ',')
                *q++ = *(*ibuf)++;
            *q++ = 0;
        }
        if (path[0] && pstrlen(path) + 1 <  *len)
        {
            p = path;
			if (continued)
	            *(*obuf)++ = ' ';
			continued = TRUE;
            while (*p)
                *(*obuf)++ =  *p++;
            *len -= pstrlen(path) + 1;
            oldptr = ptr;
        }
        else
            break;
    }
    *(*obuf) = 0;
    return oldptr;
}

//-------------------------------------------------------------------------

void InsertFiles(short *obuf, short *ibuf, short **files, short *itarget, BOOL
    oneatatime, BOOL implicit, BOOL toerr)
{
    int len = MAXLEN - pstrlen(ibuf);
    BOOL *found = FALSE;
    short *xptr = 0;
    while (*ibuf && len > 0)
    {
        while (*ibuf &&  *ibuf != MACROSTART)
            *obuf++ =  *ibuf++;
        if (! *ibuf)
            break;
        else
        {
            found++;
            ibuf++;
            switch (*ibuf)
            {
                case STAR:
                    if (*(ibuf + 1) == STAR)
                    {
                        ibuf += 2;
                        if (implicit)
                            xptr = ExpandFiles(&obuf, &ibuf, &len, files, PATH 
                                | NAME | EXT, 0, oneatatime);
                        else
                            us("**", 0);

                    }
                    else
                    {
                        ibuf++;
                        if (implicit)
                            xptr = ExpandFiles(&obuf, &ibuf, &len, files, PATH 
                                | NAME, 0, oneatatime);
                        else
                            xptr = ExpandFiles(&obuf, &ibuf, &len, files, PATH 
                                | NAME, 0, oneatatime);
                    }
                    break;
                case LESS:
                    ibuf++;
                    if (implicit)
                        xptr = ExpandFiles(&obuf, &ibuf, &len, files, PATH |
                            NAME | EXT, 0, oneatatime);
                    else
                        xptr = ExpandFiles(&obuf, &ibuf, &len, files, PATH |
                            NAME | EXT, 0, oneatatime);
                    break;
                case COLON:
                    ibuf++;
                    if (implicit)
                        xptr = ExpandFiles(&obuf, &ibuf, &len, files, PATH, 0,
                            oneatatime);
                    else
                        xptr = ExpandFiles(&obuf, &ibuf, &len, files, PATH, 0,
                            oneatatime);
                    break;
                case PERIOD:
                    ibuf++;
                    if (implicit)
                        xptr = ExpandFiles(&obuf, &ibuf, &len, files, NAME |
                            EXT, 0, oneatatime);
                    else
                        xptr = ExpandFiles(&obuf, &ibuf, &len, files, NAME |
                            EXT, 0, oneatatime);
                    break;
                case AMPERSAND:
                    ibuf++;
                    if (implicit)
                        xptr = ExpandFiles(&obuf, &ibuf, &len, files, NAME, 0,
                            oneatatime);
                    else
                        xptr = ExpandFiles(&obuf, &ibuf, &len, files, NAME, 0,
                            oneatatime);
                    break;
                case AT:
                    ibuf++;
                    if (implicit)
                    {
                        short target[200];
                        short *p,  *q;
                        pstrcpy(target,  *files);
                        p = pstrrchr(target, PERIOD);
                        if (p && *(p - 1) != PERIOD)
                            *p = 0;
                        p = target + pstrlen(target);
                        q = itarget + 1;
                        while (*q != PERIOD)
                            q++;
                        pstrcat(target, q);
                        xptr = ExpandFiles(&obuf, &ibuf, &len, files, NAME |
                            TARGEXT, target, oneatatime);
                    }
                    else
                        xptr = ExpandFiles(&obuf, &ibuf, &len, files, PATH |
                            NAME | EXT, 0, oneatatime);
                    break;
                case QUESTION:
                    ibuf++;
                    if (implicit)
                        xptr = ExpandFiles(&obuf, &ibuf, &len, files, PATH |
                            NAME | EXT, 0, oneatatime);
                    else
                        xptr = ExpandFiles(&obuf, &ibuf, &len, files, NAME, 0,
                            oneatatime);
                    break;
                default:
					if (toerr)
					{
	                    us(0,  *ibuf);
    	                ibuf++;
					}
					else
					{
						*obuf++ = ibuf[-1];
						*obuf++ = *ibuf++;
					}
					break;
            }
        }
    }
    if (!found || xptr && ! *xptr)
        xptr = 0;
    *files = xptr;
    *obuf++ = 0;
}

//-------------------------------------------------------------------------

int SpawnTarget(short *target, short *files, BOOL implicit)
{
    REGISTRY **r = LookupPhiHash(hashtable, target);
    short **commands;
    char buf[510];
    short *buffer,  *buf2;
    buffer = AllocateMemory(INTERNALBUFSIZE *sizeof(short));
    buf2 = AllocateMemory(INTERNALBUFSIZE *sizeof(short));

    if (!r)
        fatal("Internal error #2");

    commands = (*r)->x.commands;

    while (*commands)
    {
        short *tfiles = files;
        short *p =  *commands++,  *temp;
        BOOL oneatatime = FALSE;
        if (p[0] == AT || p[0] == AMPERSAND)
        {
            oneatatime = TRUE;
            p++;
        }
        pstrcpy(buf2, p);
        if ((temp = pstrstr(buf2, xandand)) != 0)
        {
            FILE *fh = CreateFile();
            short terminator = *(temp + 2);
            short *q;
            *temp = 0;
            while (*commands)
            {
				short *tfiles[2];
				tfiles[0] = target;
				tfiles[1] = NULL;
	            InsertFiles(buffer, *commands, &tfiles, target, FALSE, FALSE, FALSE);
                ExpandString(buffer);
                q = buffer;
                while (*q &&  *q != terminator)
                    fputc(*q++, fh);
                fputc(0x0a, fh);
                commands++;
                if (*q == terminator)
                    break;
            }
            CloseFile(fh);

            pstrcat(buf2, tempfiles->data);
            if (*q)
                pstrcat(buf2, q + 1);
        }
        pstrcpy(buffer, buf2);
        ExpandString(buffer);
        while (tfiles)
        {
            InsertFiles(buf2, buffer, &tfiles, target, oneatatime, implicit, TRUE);
            if ((makerv = SpawnOne(buf2)) != 0)
            {
                if (!implicit)
                {
                    CompressFile(buf2, target);
                    if (!prm_suppressdisplay)
                        printf("\n\n\tReturn code %d - Deleting %s\n", makerv,
                            buf2);
                    unlink(buf2);
                }
                else
                {
                    if (!prm_suppressdisplay)
                        printf("\n\n\tReturn code %d\n", makerv);
                }
                DeallocateMemory(buffer);
                DeallocateMemory(buf2);
                return (makerv);
            }
        }
    }
    DeallocateMemory(buffer);
    DeallocateMemory(buf2);
    return (makerv);
}

//-------------------------------------------------------------------------

void DoTargets(FILEREC *list)
{
    short *buf;
    FILEREC *p = list;
    buf = AllocateMemory(INTERNALBUFSIZE *sizeof(short));
    printf("\n");
    while (p)
    {
        FILEREC *r = p;
        buf[0] = 0;
        while (p && !pstrcmp(p->target, r->target))
        {
            pstrcat(buf, xspace);
            if (!p->targetable)
                fatal("Internal error #1");
            if (p->implicit)
                pstrcat(buf, p->depends);
            else
                pstrcat(buf, p->target);
            p = p->link;
        }
        SpawnTarget(r->target, buf, r->implicit);
        if (makerv)
            break;
    }
    DeleteFiles();
    while (list)
    {
        p = list->link;
        ReleaseFile(list);
        list = p;
    }
    DeallocateMemory(buf);
}
