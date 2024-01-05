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
#include "cmdline.h"
#include "umem.h"
#include "interp.h"
#include "input.h"
#include "parser.h"
#include "register.h"
#include "macros.h"
#include "if.h"
#include "maker.h"

extern int level;
extern char *prm_includepath;
extern short *bufptr, buffer[];
extern BOOL done;
extern BOOL ifskip;
extern short xandand[];
extern BOOL prm_autodepends;

void Extraneous(void)
{
    Error("Extraneous characters on line");
}

//-------------------------------------------------------------------------

void readfilename(short **bp, short *buf, short mc)
{
    while (**bp != mc &&  **bp)
        *buf++ = *(*bp)++;
    *buf = 0;
    if (**bp)
        bp++;
}

//-------------------------------------------------------------------------

void readtargetname(short **bp, short *buf, short mc)
{
    readfilename(bp, buf, mc);
    ExpandString(buf);
}

//-------------------------------------------------------------------------

void NewInputFile(void)
{
    short matchchar = QUOTE;
    short filebuf[200];
    char filename[200];
    ExpandString(bufptr);
    skipSpace(&bufptr);
    if (*bufptr == LESS)
        matchchar = GREATER;
    else
        if (*bufptr != QUOTE)
            Error("Bad include specification");
    bufptr++;
    readfilename(&bufptr, filebuf, matchchar);
    if (*bufptr != matchchar)
        Error("Unterminated include");
    else
        bufptr++;
    skipSpace(&bufptr);
    if (*bufptr)
        Extraneous();
    CompressFile(filename, filebuf);
    if (!openFile(filename, prm_includepath, "r"))
        Error("Missing include file %s", filename);
}

//-------------------------------------------------------------------------

void HandleExclamation(void)
{
    short ibuf[1024],  *p = ibuf;
    while (*bufptr && issymchar(*bufptr))
        *p++ =  *bufptr++;
    *p = 0;
    if (!pstrcmp(ibuf, L"include"))
        NewInputFile();
    else
        if (!pstrcmp(ibuf, L"ifdef"))
            ifdef();
        else
            if (!pstrcmp(ibuf, L"ifndef"))
                ifndef();
            else
                if (!pstrcmp(ibuf, L"else"))
                    ifelse();
                else
    /*					if (!pstrcmp(ibuf,xelseif))
     *						ifelseif();
     *
     *					else
     */
                    if (!pstrcmp(ibuf, L"endif"))
                        ifendif();
                    else if (!pstrcmp(ibuf, L"message"))
    {
        char buf[512];
        CompressFile(buf, bufptr + 1);
        printf("%s", buf);
    }
    else
    {
        char buffer[512];
        CompressFile(buffer, ibuf);
        Error("Invalid escape %s", buffer);
    }
}

//-------------------------------------------------------------------------

void escape(int nulls)
{
    while (TRUE)
    {
        NewLine();
        if (done)
            break;
        skipSpace(&bufptr);
        if (!*(bufptr))
            if (nulls)
                break;
            else
                continue;
        if (*bufptr == EXCLAMATION)
        {
            bufptr++;
            HandleExclamation();
            continue;
        }
        else
        {
            if (! *bufptr)
                continue;
            if (!ifskip)
            {
                bufptr = buffer;
                break;
            }
        }
    }
}

//-------------------------------------------------------------------------

short **ReadCommands(int *count)
{
    BOOL infile = FALSE;
    int matchchar = 0;
    short **rv;
    short *lines[COMMANDLINES];
    *count = 0;
    while (!done)
    {
        if (*count >= COMMANDLINES)
            fatal("Too many command lines");
        escape(infile);
        if (!infile && !iswhitespacechar(*bufptr))
            break;
        if (iswhitespacechar(*bufptr))
        {
            skipSpace(&bufptr);
        }
        if (*bufptr || infile)
        {
            if (infile)
            {
                if (pstrchr(bufptr, matchchar))
                {
                    infile = FALSE;
                }
            }
            else
            {
                short *temp;
                if ((temp = pstrstr(bufptr, xandand)) != 0)
                {
                    matchchar = *(temp + 2);
                    infile = TRUE;
                }
            }

            lines[ *count] = AllocateMemory(pstrlen(bufptr) *2+2);
            pstrcpy(lines[ *count], bufptr);
            (*count)++;
        }
    }
    if (infile)
        Error("End of file without match character %c", matchchar);
    lines[(*count)++] = 0;
    if (*count)
    {
        rv = AllocateMemory(sizeof(short*) **count);
        memcpy(rv, lines, sizeof(char*) **count);
    }
    return (rv);
}

//-------------------------------------------------------------------------

void Implicit(void)
{
    short buf[100],  **rv,  *p = buf;
    int count;
    if (!pstrncmp(bufptr, L".path", pstrlen(L".path")))
    {
        while (*bufptr &&  *bufptr != ALTEQUAL &&  *bufptr != EQUAL)
            *p++ =  *bufptr++;
        *p = 0;
        if (*bufptr)
            bufptr++;
        else
            Error("Missing path");
        RegisterPath(buf, bufptr);
        escape(FALSE);
    }
    else
    {
        int *q = pstrchr(bufptr, COLON);
        int *r = pstrchr(bufptr, COLON2);
        if (r && (!q || q > r))
            readtargetname(&bufptr, buf, COLON2);
        else
            readtargetname(&bufptr, buf, COLON);

        if (*bufptr != COLON &&  *bufptr != COLON2)
            Error("Missing colon in implicit rule");
        bufptr++;
        skipSpace(&bufptr);
        if (*bufptr)
            Extraneous();
        rv = ReadCommands(&count);
        RegisterImplicit(buf, rv);
    }
}

//-------------------------------------------------------------------------

void Explicit(void)
{
    short buf[100], value[INTERNALBUFSIZE],  **rv,  *p = buf,  *q,  *r,  *s;
    int count;
    q = pstrchr(bufptr, COLON);
    r = pstrchr(bufptr, COLON2);
    if (r && (!q || q > r))
        q = r;
    if (q == 0)
        q =  - 1;
    r = pstrchr(bufptr, EQUAL);
    s = pstrchr(bufptr, ALTEQUAL);
    if ((r && r < q) || (s && s < q))
    {
        while (*bufptr != EQUAL &&  *bufptr != ALTEQUAL)
            *p++ =  *bufptr++;
        *p = 0;
        while (p > buf && iswhitespacechar(*(p - 1)))
            *--p = 0;
        ++bufptr;
        p = value;
        while (*bufptr)
             *p++ =  *bufptr++;
        *p = 0;
        p = ExpandSelf(buf, value);
        RegisterMacro(buf, p, TRUE, TRUE);
        escape(FALSE);
    }
    else
    {
        q = pstrchr(bufptr, COLON);
        r = pstrchr(bufptr, COLON2);
        if (r && (!q || q > r))
            readtargetname(&bufptr, buf, COLON2);
        else
            readtargetname(&bufptr, buf, COLON);
        if (*bufptr != COLON &&  *bufptr != COLON2)
            Error("Missing colon in explicit rule");
        bufptr++;
        RegisterExplicitName(buf, bufptr);
        rv = ReadCommands(&count);
        RegisterExplicitCommands(buf, rv);
    }
}

//-------------------------------------------------------------------------

BOOL ReadMakeFile(char *path, char *string)
{
    openFile(string, path, "r");
    if (level < 0)
    {
        char buffer[256];
        strcpy(buffer, string);
        AddExt(buffer, ".MAK");
        openFile(buffer, path, "r");
    }
    if (level < 0)
        return (FALSE);
    escape(FALSE);
    while (!done)
    {
        if (iswhitespacechar(*bufptr))
		{
			short *p = bufptr;
			while (isspace(*p)) 
				p++;
		    if (!pstrncmp(p,L".autodepend", pstrlen(L".autodepend")))
    		{
        		prm_autodepends = TRUE;
        		escape(FALSE);
    		}
			else
	            Error("Target expected");
		}
        else if (*bufptr == PERIOD)
            Implicit();
        else
            Explicit();
    }
    return (TRUE);
}
