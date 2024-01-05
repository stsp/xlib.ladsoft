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
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <dir.h>
#include <ctype.h>

#define TRUE 1
#define FALSE 0

#define MAXFILES 10000

extern char *_oscmd;
extern HINSTANCE *hInstance;

int _argc;
char **_argv;
char *_passed_name;

static char oldcwd[256];
static int oldDrive = 0;
static struct ffblk fileBlock;
static char **filelist = 0;
static int filenum = 0, filecount = 0;

#ifndef _A_NORMAL
    #define _A_NORMAL 0
#endif 
static char *FirstFile(char *spec)
{
    if (findfirst(spec, &fileBlock, _A_NORMAL))
        return (0);
    return (fileBlock.ff_name);
} static char *NextFile(void)
{
    if (findnext(&fileBlock))
        return (0);
    return (fileBlock.ff_name);
}

//-------------------------------------------------------------------------

static int rfsort(const void *elem1, const void *elem2)
{
    return (strcmp(*(char **)elem1, *(char **)elem2));
}

//-------------------------------------------------------------------------

static char *litlate(char *dir, char *t)
{
    char *q, buf[256];
    if (!t)
        return 0;
    if (dir)
    {
        strcpy(buf, dir);
        q = strrchr(buf, '\\');
        if (q)
            strcpy(q + 1, t);
        else
            strcpy(buf, t);
    }
    else
        strcpy(buf, t);
    q = calloc(1,strlen(buf) + 1);
    if (!q)
    {
        fprintf(stdout, "out of memory");
        exit(1);
    }
    strcpy(q, buf);
    return q;
}

//-------------------------------------------------------------------------

static void ClearFiles(void)
{
    free(filelist);
    filelist = 0;
    filecount = 0;
}

//-------------------------------------------------------------------------

static char *ReadFiles(char *spec)
{
    char buf[256];
    int fullpath = FALSE;
    if (spec[0] == '.' && spec[1] == '.')
    {
        char *p;
        getcwd(buf, 256);
        p = strlen(buf) + buf;

        while (spec[0] == '.' && spec[1] == '.')
        {
            spec += 2;
            while (*(p - 1) != '\\' && *(p - 1) != ':')
                p--;
            if (*(p - 1) == '\\')
                p--;
            if (spec[0] != '\\')
                break;
            spec++;
        }
        *p++ = '\\';
        strcpy(p, spec);
    }
    else
        strcpy(buf, spec);
    if (strchr(buf, '\\'))
        fullpath = TRUE;
    if (!filelist)
    {
        filelist = calloc(1,sizeof(char*) * MAXFILES);
        if (!filelist)
        {
            fprintf(stderr, "out of memory");
            exit(1);
        }
    }
    if ((filelist[0] = litlate(fullpath ? buf : 0, FirstFile(buf))) == 0)
    {
        return 0;
    }
    while ((filelist[++filecount] = litlate(fullpath ? buf : 0, NextFile())) !=
        0)
    if (filecount >= MAXFILES - 1)
    {
        fprintf(stderr, 
            "Too many files... qualify file names to limit to %d at a time",
            MAXFILES);
        break;
    }
    return (char*)1;
}

//-------------------------------------------------------------------------

static int qualify(char *name)
{
    int i;
    for (i = 0; i < strlen(name); i++)
        if (!isalnum(*name) &&  *name != '\\' &&  *name != ':' &&  *name != '*'
            &&  *name != '?' &&  *name != '.')
            return FALSE;
    return TRUE;
}

//-------------------------------------------------------------------------

char **CmdLineToC(int *_argc, char *cmd)
{
    char buf[200];
    char *bufp[100],  *ocl;
    char *_cmdline = cmd;
    char **_argv = 0;
    int inquote = 0;
    char *dir;
    char *file;
    char *fileName;
    int drive;
    filenum = 0;
    filecount = 0;
    *_argc = 1;
    while (*_cmdline)
    {
        while (isspace(*_cmdline))
            _cmdline++;
        if (*_cmdline)
        {
            int wasquote = FALSE;
            int i = 0;
            while ((inquote || !isspace(*_cmdline)) &&  *_cmdline)
            {
                if (*_cmdline == '"')
                {
                    wasquote = TRUE;
                    inquote = !inquote;
                    _cmdline++;
                    continue;
                }
                buf[i++] =  *_cmdline++;
            }
            buf[i++] = 0;
            if (!wasquote && qualify(buf))
            {
                if ((fileName = ReadFiles(buf)) == 0)
                {
                    goto join;
                }
                if (filecount)
                {
                    _argv = realloc(_argv, (*_argc + filecount) *sizeof(char*));
                    memcpy(_argv +  *_argc, filelist, filecount *sizeof(char*));
                    *_argc += filecount;
                    ClearFiles();
                }
                else
                    goto join;
            }
            else
            {
                join: _argv = realloc(_argv, (*_argc + 1) *sizeof(char*));
                _argv[(*_argc)++] = strdup(buf);
            }

        }
    }
    _argv = realloc(_argv, (*_argc + 1) *sizeof(char*));
    _argv[ *_argc] = 0;
    _passed_name = _argv[0];
    GetModuleFileName(hInstance, buf, 200);
    _argv[0] = strdup(buf);
    return _argv;
}
