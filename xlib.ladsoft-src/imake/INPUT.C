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
#include <ctype.h>
#include <stdarg.h>
#include <string.h>
#include <malloc.h>
#include <memory.h>
#include "cmdline.h"
#include "umem.h"
#include "input.h"
#include "interp.h"

extern BOOL phiused;
extern int makerv;

int level =  - 1;
int lineno[INCLUDELEVELS];
FILE *ifile[INCLUDELEVELS];
char *filenames[INCLUDELEVELS];
int errcount = 0;
short buffer[INTERNALBUFSIZE];
short *bufptr;
BOOL done;

BOOL openFile(char *string, char *path, char *mode)
{
    char buffer[256];
    strcpy(buffer, string);
    if (level == INCLUDELEVELS)
        fatal("Too many levels of include files\n");
    if (!(ifile[level + 1] = SrchPth(buffer, path, mode)))
    {
        return (FALSE);
    }
    level++;
    lineno[level] = 0;
    filenames[level] = AllocateMemory(strlen(string) + 1);
    strcpy(filenames[level], string);
    return (TRUE);
}

//-------------------------------------------------------------------------

void closeFile(void)
{
    fclose(ifile[level]);
    DeallocateMemory(filenames[level]);
    level--;
}

//-------------------------------------------------------------------------

BOOL nextLine(char *buf, int len)
{
    while (!feof(ifile[level]))
    {
        lineno[level]++;
        if (!philine(buf, len, ifile[level]))
            return (FALSE);
        if (buf[0] != 0)
        {
            return (TRUE);
        }
    }
    return (FALSE);
}

//-------------------------------------------------------------------------

void NewLine(void)
{
    int len = 0;
    char lbuf[BUFLEN];
    short *r,  *q = buffer;
    bufptr = buffer;
    *bufptr = 0;
    while (level >= 0)
    {
        if ((done = !nextLine(lbuf, BUFLEN)) == 0)
        {
            char *p = lbuf;
            while (*q++ = parsechar(&p))
                if (++len >= INTERNALBUFSIZE)
                    Error("internal buffer overflow");
            q = buffer;
            r = q;
            while (*q != 0x0a)
            {
                while (!iscommentchar(*q) &&  *q != 0x0a)
                if (iswhitespacechar(*q))
                {
                    *r++ = ' ';
                    q++;
                }
                else
                    *r++ =  *q++;
                if (*q != 0x0a)
                    q++;
                while (!iscommentchar(*q) &&  *q != 0x0a)
                    q++;
            }
            *r-- = 0;
            while (iswhitespacechar(*r) && r > buffer)
                r--;
            if ((*r == EXTENDER1) || (*r == EXTENDER2))
            {
                q = r;
                *q = 0;
            }
            else
            {
                break;
            }
        }
        else
            closeFile();
    }
}

//-------------------------------------------------------------------------

int skipSpace(short **buf)
{
    while (iswhitespacechar(**buf))
        (*buf)++;
    return (0);
}

//-------------------------------------------------------------------------

void Error(char *fmt, ...)
{
    va_list argptr;
    va_start(argptr, fmt);
    printf("Error: ");
    printf("%s(%d): ", filenames[level], lineno[level]);
    vprintf(fmt, argptr);
    va_end(argptr);
    putc('\n', stdout);
    errcount++;
    if (!makerv)
        makerv = 1;
    exit(makerv);
}
