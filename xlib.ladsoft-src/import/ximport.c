/* 
IMPORT librarian
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
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "cmdline.h"
#include "umem.h"
#include "errors.h"
#include "libm.h"
#include "module.h"
#include "dict.h"
#include "lib.h"
#include "..\version.h"

// #define VERSION 125

extern HASHREC **publichash;
extern int pagesize;
/* Command line parameters */
BOOL prm_case_sensitive = TRUE; /* True if case sensitive */
uint prm_errcount = 25;

/* Global varieables */
LIST *objlist = 0; /* List of object files */
LIST *attriblist = 0;
char *libfile = 0; /* EXE file name */

char *usage_text = 
    "[/u/pxx]  destination  [source | @filename] [source list]\n"
    "\t/u  - make non-case sensitive dictionary\n"
    "\t/pxx- set page size to xx (default 16)\n"
    "\t @filename reads paramaters from file\n\n"
    "\tdestination can be either a .LIB file or a .DEF file.\n"
    "\tif destination is a .LIB file the source must be a .DEF or .DLL file\n"
    "\tif destination is a .DEF file the source must be a .DLL file\n";


void BoolSetup(char select, char *string);
void PageSetup(char select, char *string);

ARGLIST ArgList[] = 
{
    {
        'u', ARG_BOOL, BoolSetup
    }
    , 
    {
        'p', ARG_CONCATSTRING, PageSetup
    }
    , 
    {
        0, 0, 0
    }
};
/*
 * Setup for boolean command line args
 */
static void BoolSetup(char select, char *string)
{
    switch (select)
    {
        case 'u':
            prm_case_sensitive = FALSE;
            break;
    }
}

//-------------------------------------------------------------------------

void PageSetup(char select, char *string)
{
    int val = atoi(string);
    int i, j = 0, mask = 1;
    if (val < 16)
        fatal("Page size too small");
    for (i = 0; i < sizeof(short) *8; i++)
    {
        if (val &mask)
            j++;
        mask <<= 1;
    }
    if (j != 1)
        fatal("Page size must be multiple of 2");
}

//-------------------------------------------------------------------------

char *stristr(char *str1, char *str2)
{
    int l = strlen(str2);
    while (*str1)
    {
        if (!stricmp(str1, str2))
            return str1;
        str1++;
    }
    return 0;
}

/*
 * Insert a file onto one of the lists.  .LIB files go on library list,
 *   anything else is assumed an .obj file regardless of extension
 */
static void InsertAnyFile(char *filename, int attribs)
{
    char *newbuffer, buffer[1000];

    /* Allocate buffer and make .o if no extension */
    strcpy(buffer, filename);
    AddExt(buffer, ".obj");
    newbuffer = (char*)AllocateMemory(strlen(buffer) + 1);
    strcpy(newbuffer, buffer);

    /* Insert file */
    AppendToList(&objlist, newbuffer);
    AppendToList(&attriblist, (void*)attribs);
}

/* 
 * Parse a line of commands and files
 */
static BOOL ReadArgs(int *argc, char **argv, int *mode, int *maxmode, BOOL
    uselib)
{
    BOOL rv;
    int i, st = 1;
    /* minus signs will be eaten by parse_args but we want them to propogate
     * through, so we change them to '#'
     */
    for (i = 1; i <  *argc; i++)
    {
        char *t;
        while ((t = strchr(argv[i], '-')) != 0)
            *t = '#';
        while ((t = strchr(argv[i], '+')) != 0)
            *t = '%';
    }
    /* Scan command line for switches */
    rv = parse_args(argc, argv, TRUE);
    if (uselib)
    {
        char buffer[1000];
        if (*argc < 2)
            return (FALSE);
        strcpy(buffer, argv[1]);
        AddExt(buffer, ".LIB");
        libfile = AllocateMemory(strlen(buffer) + 1);
        strcpy(libfile, buffer);
        st++;
    }

    /* Scan the command line for file names or response files */
    for (i = st; i <  *argc; i++)
    {
        char *p = argv[i];
        while (*p)
            *p++ = (char)toupper(*p);
        p = argv[i];
        if (*p == '*' ||  *p == '#' ||  *p == '%')
        {
            *mode = 0;
            while (*p == '*' ||  *p == '#' ||  *p == '%')
            {
                if (*p++ == '*')
                {
                    *mode |= MD_EXTRACT;
                }
                else
                if (*(p - 1) == '#')
                {
                    *mode |= MD_DELETE;
                }
                else
                {
                    *mode |= MD_INSERT;
                }
            }
        }
        if (*p == '@')
        {
            rv &= ReadResponse(&argv[i][1], mode, maxmode);
        }
        else
        if (*p != '&' &&  *p > 31)
        {
            InsertAnyFile(p,  *mode);
        }
        *maxmode |=  *mode;
    }
    return (rv);
}

/*
 * Read a line of ascii text from a file
 *   Get rid of \n
 */
static void ReadLine(char *buffer, int count, FILE *file, char *name)
{
    char *pos;
    *buffer = 0;
    fgets(buffer, count, file);
    pos = buffer + strlen(buffer) - 1;
    /* The test is needed because my editor doesn't put CR/LF at the end of file */
    if (*pos < 32)
        *pos = 0;
}

/*
 * Read the response file
 */
static BOOL ReadResponse(char *filename, int *mode, int *maxmode)
{
    FILE *in;
    BOOL rv = TRUE;

    /* Open file */
    if ((in = fopen(filename, "r")) == 0)
        fatal("Missing or invalid response file %s", filename);

    /* Read EXE file name */
    while (!feof(in))
    {
        int argc = 1;
        int i = 0;
        char *argv[1000];
        char buffer[1024];
        ReadLine(buffer, 1024, in, filename);
        while (TRUE)
        {
            char ch;
            int quoted = ' ';
            while (buffer[i] == ' ')
                i++;
            if (buffer[i] < 32)
                break;
            if (buffer[i] == '"')
                quoted = buffer[i];
            argv[argc++] = &buffer[i];
            while (buffer[i] > 31 && buffer[i] != quoted)
                i++;
            ch = buffer[i];
            buffer[i++] = 0;
            if (ch < 32)
            {
                break;
            }
        }
        if (argc > 1)
            rv &= ReadArgs(&argc, argv, mode, maxmode, FALSE);
    }
    fclose(in);
    return (rv);
}

/*
 * Main routine
 *   Read command line
 *   Make EXE and MAP filenames if not already extant
 *   Pass 1 init
 *   Pass 1
 *   Pass 1 rundown
 *   pass 2 init
 *   Pass 2
 *   Pass 2 rundown
 */
int main(int argc, char *argv[])
{
    int mode = MD_INSERT, maxmode = 0;

    banner("IMPORT Version %s %s", IMPORT_STRING_VERSION, PRODUCT_COPYRIGHT);

    if (!ReadArgs(&argc, argv, &mode, &maxmode, TRUE))
        usage(argv[0]);
    if (maxmode)
    {
        publichash = CreateHashTable(HASH_TABLE_SIZE);
        if (stristr(libfile, ".DEF"))
        {
            FILE *f = fopen(libfile, "r");
            if (f)
            {
                CreateImports(f, libfile);
                fclose(f);
            }
        }
        else
            ReadLib(maxmode, libfile);
        Extract(maxmode, libfile);
        Delete(maxmode, libfile);
        Insert(maxmode, libfile);
        if (stristr(libfile, ".DEF"))
            OutputDefFile(libfile);
        else
        {
            CalculateDictionary();
            OutputLibrary(libfile);
        }
        RemoveHashTable(publichash);
    }
    else
        usage(argv[0]);
    return (0);
}
