/* 
Browse Linker
Copyright 2003-2011 David Lindauer.

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
#include "..\version.h"

/* Command line parameters */

/* Global varieables */
LIST *objlist = 0; /* List of object files */
char *brcfile = 0; /* EXE file name */

char *usage_text = " <output file> <file list> [@cmdfile]";

ARGLIST ArgList[] = 
{
    {
        0, 0, 0
    }
};

/*
 * Insert a file onto one of the lists.  .LIB files go on library list,
 *   anything else is assumed an .obj file regardless of extension
 */
static void InsertAnyFile(char *filename)
{
    char *newbuffer, buffer[260];

    /* Allocate buffer and make .o if no extension */
    strcpy(buffer, filename);
    AddExt(buffer, ".obj");
    newbuffer = (char*)AllocateMemory(strlen(buffer) + 1);
    strcpy(newbuffer, buffer);

    /* Insert file */
    AppendToList(&objlist, newbuffer);
}

/* 
 * Parse a line of commands and files
 */
static BOOL ReadArgs(int *argc, char **argv, BOOL uselib)
{
    BOOL rv;
    int i, st = 1;
    /* Scan command line for switches */
    rv = parse_args(argc, argv, TRUE);
    if (uselib)
    {
        char buffer[256];
        if (*argc < 2)
            return (FALSE);
        strcpy(buffer, argv[1]);
        AddExt(buffer, ".BRW");
        brcfile = AllocateMemory(strlen(buffer) + 1);
        strcpy(brcfile, buffer);
        st++;
    }

    /* Scan the command line for file names or response files */
    for (i = st; i <  *argc; i++)
    {
        char *p = argv[i];
        while (*p)
            *p++ = (char)toupper(*p);
        p = argv[i];
        if (*p == '@')
        {
            rv &= ReadResponse(&argv[i][1]);
        }
        else
        if (*p > 31)
        {
            InsertAnyFile(p);
        }
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
static BOOL ReadResponse(char *filename)
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
        char *argv[500];
        static char buffer[10000];
        ReadLine(buffer, 10000, in, filename);
        while (TRUE)
        {
            char ch;
            int quoted = ' ';
            while (buffer[i] == ' ')
                i++;
            if (buffer[i] < 32)
                break;
            if (buffer[i] == '"')
                quoted = buffer[i++];
            argv[argc++] = &buffer[i];
            while (buffer[i] > 31 && quoted != buffer[i])
                i++;
            ch = buffer[i];
            buffer[i++] = 0;
            if (ch < 32)
            {
                break;
            }
        }
        if (argc > 1)
            rv &= ReadArgs(&argc, argv, FALSE);
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
    banner("BRC Version %s %s", BRC_STRING_VERSION, PRODUCT_COPYRIGHT);

    if (!ReadArgs(&argc, argv, TRUE))
        usage(argv[0]);

    if (!objlist)
        fatal("noinput files specified");
    while (objlist)
    {
        FILE *fil = fopen(objlist->data, "rb");
        if (!fil)
            fatal("Cannot open input file %s", objlist->data);
        ReadModule(fil, objlist->data);
        fclose(fil);
        objlist = objlist->link;
    }
    GenerateOutput(brcfile);
    return (0);
}
