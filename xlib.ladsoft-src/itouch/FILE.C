/* 
ITOUCH
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
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <dos.h>
#include "cmdline.h"

#define MAXFILES 500

static struct find_t fileBlock;
static char **filelist = 0;
static int filenum = 0, filecount = 0;

void ParseDir(char *spec, int *drive, char **dir, char **file)
{
    char *spec2;
    *drive =  - 1;
    *dir = 0;
    *file = 0;

    while (*spec == ' ')
        spec++;

    spec2 = spec;
    while (*spec2)
        *(spec2++) = toupper(*spec2);

    if (*spec)
    {
        if (*(spec + 1) == ':')
        {
            *drive =  *spec - 'A';
            *dir = spec + 2;
        } 
        else
            *dir = spec;
    }

    if ((spec2 = strrchr(*dir, '\\')) != 0)
    {
        *spec2 = 0;
        *file = spec2 + 1;
    }
    else
    {
        *dir = 0;
        *file = spec;
    }
}

//-------------------------------------------------------------------------

char *FirstFile(char *spec)
{
    if (_dos_findfirst(spec, _A_NORMAL, &fileBlock))
        return (0);
    return (fileBlock.name);
}

//-------------------------------------------------------------------------

char *NextFile(void)
{
    if (_dos_findnext(&fileBlock))
        return (0);
    return (fileBlock.name);
}

//-------------------------------------------------------------------------

int rfsort(const void *elem1, const void *elem2)
{
    return (strcmp(*(char **)elem1, *(char **)elem2));
}

//-------------------------------------------------------------------------

char *litlate(char *t, int drive, char *dir)
{
    char *q;
    if (!t)
        return 0;
    q = malloc(strlen(t) + 4+strlen(dir));
    if (!q)
        fatal("out of memory");
    if (drive !=  - 1)
    {
        q[0] = drive + 'A';
        q[1] = ':';
        q[2] = 0;
    }
    strcat(q, dir);
    strcat(q, "\\");
    strcat(q, t);
    return q;
}

//-------------------------------------------------------------------------

void ClearFiles(void)
{
    int i;
    for (i = 0; i < filecount; i++)
        free(filelist[i]);
    free(filelist);
    filelist = 0;
}

//-------------------------------------------------------------------------

char *GrabFile(void)
{
    if (filenum < filecount)
        return filelist[filenum++];
    return 0;
}

//-------------------------------------------------------------------------

char *ReadFiles(char *spec)
{
    char *file,  *dir;
    char lspec[256];
    int drive;
    strcpy(lspec, spec);
    ParseDir(lspec, &drive, &dir, &file);
    filelist = malloc(sizeof(char*) * MAXFILES);
    if (!filelist)
        fatal("no memory");
    if ((filelist[0] = litlate(FirstFile(spec), drive, dir)) == 0)
    {
        ClearFiles();
        return 0;
    }
    while ((filelist[++filecount] = litlate(NextFile(), drive, dir)) != 0)
        if (filecount >= MAXFILES - 1)
            fatal(
                "Too many files... qualify file names to limit to %d at a time",
                MAXFILES);
    qsort(filelist, filecount, sizeof(char*), rfsort);
    return 1;
}

//-------------------------------------------------------------------------

char *NewName(char *spec)
{
    static char namebuf[15];
    char *pos;
    strcpy(namebuf, spec);
    if (!(pos = strrchr(namebuf, '.')))
        strcat(namebuf, ".~");
    else
    {
        *(pos + 4) = 0;
        *(pos + 3) = *(pos + 2);
        *(pos + 2) = *(pos + 1);
        *(pos + 1) = '~';
    }
    return (namebuf);
}

//-------------------------------------------------------------------------

void repath(char *dest, char *name, char *pwd)
{
    char *p;
    int needslash = 1;

    if (name[0] == '\\' || name[1] == ':')
    {
        strcpy(dest, name);
        return ;
    }
    strcpy(dest, pwd);

    while (*name)
    {
        if (*name == '.' && *(name + 1) == '.')
        {
            if (needslash)
            {
                dest[strlen(dest) - 1] = 0;
                p = strrchr(dest, '\\');
                *p = 0;
            }
            name += 2;
            needslash = 1;
        }
        else
        {
            char *dst = dest + strlen(dest);
            if (needslash &&  *name != '\\')
                *dst++ = '\\';
            needslash = 0;
            *dst++ =  *name++;
            *dst++ = 0;
        }
    }
}

//-------------------------------------------------------------------------

int FileRecurse(int count, char *FileNames[], char *Text, FILEFUNC routine,
    char *ext, BOOL backup)
{
    char *fileName;
    int i;
    filenum = 0;
    filecount = 0;
    remove("ztbpjklx.xuz");
    for (i = 0; i < count; i++)
    {
        char buffer[256];
        char pwd[256];
        int drive = 0;
        if (FileNames[i][1] == ':')
            drive = FileNames[i][0] - 'A' + 1;
        if (drive > 26)
            drive -= 32;
        if (!drive)
        {
            _dos_getdrive(&drive);
        }
        _dos_getpwd(pwd + 3, drive);
        pwd[0] = drive - 1+'A';
        pwd[1] = ':';
        pwd[2] = '\\';
        repath(buffer, FileNames[i], pwd);
        if (ext)
            AddExt(buffer, ext);
        if (ReadFiles(buffer) == 0)
        {
            fprintf(stderr, "No Such Files %s.\n", buffer);
            continue;
        }
    }
    while ((fileName = GrabFile()) != 0)
    {
        if (backup)
        {
            char *newName = NewName(fileName);
            FILE *inf,  *outf;
            if (Text)
            {
                printf("%s %s to %s\n", Text, fileName, newName);
                outf = fopen(newName, "w");
                inf = fopen(fileName, "r");
                (*routine)(inf, outf, fileName);
                fclose(outf);
                fclose(inf);
                rename(newName, "ztbpjklx.xuz");
                rename(fileName, newName);
                rename("ztbpjklx.xuz", fileName);
            }
        }
        else
        {
            FILE *inf;
            if (Text)
                printf("%s %s\n", Text, fileName);
            inf = fopen(fileName, "r");
            (*routine)(inf, 0, fileName);
            if (inf)
                fclose(inf);
        }
    }
    ClearFiles();
    return (0);
}
