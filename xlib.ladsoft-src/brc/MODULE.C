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
#include <memory.h>
#include <string.h>
#include <ctype.h>
#include "cmdline.h"
#include "module.h"

char modname[256];
char *lnames[100];
int lnamecount;
long modnumber = 0;
long liboffset;

FILE *curfile;

static BOOL endofmodule;

void skipind(BYTE **data, int *size)
{
    int v = *(*data)++;
    (*size)--;
    if (v &0x80)
    {
        (*size)--;
        (*data)++;
    }
}

//-------------------------------------------------------------------------

int readname(char *buf, BYTE **data)
{
    int len = *(*data)++, i;
    for (i = 0; i < len; i++)
        *buf++ = *(*data)++;
    *buf = 0;
    return len;
}

//-------------------------------------------------------------------------

void GetModName(BYTE *data)
{
    char buf[256],  *y;
    data += 3;
    readname(buf, &data);
    y = strrchr(buf, '\\');
    if (!y)
        y = strrchr(buf, ':');
    if (!y)
        y = buf;
    else
        y++;
    StripExt(y);
    strcpy(modname, y);
}

/*  
 * Read and execute an object record
 */
static uint ReadObjectRecord(FILE *infile, char *name)
{
    uint command, size = 0, index =  - 1;
    unsigned char buf[1200], lname[256];
    char *data = 0;
    int datalen = 0;
    int ofs;
    int segindex = 0;
    lnamecount = 0;
    while (TRUE)
    {
        if (feof(curfile))
            fatal("Invalid library file format");
        buf[0] = fgetc(curfile);
        fread(buf + 1, 1, 2, curfile);
        size = *(short*)(buf + 1);
        if (fread(buf + 3, 1, size, curfile) != size)
            fatal("Invalid object file format in %s", name);
        switch (buf[0])
        {
            case 0x80:
                GetModName(buf);
                break;
            case 0x8a:
            case 0x8b:
                if (data)
                    ParseBRCData(data, datalen, name);
                free(data);
                return MODQUIT;
            case 0x98:
            case 0x99:
                // segdef
                segindex++;
                if (index ==  - 1)
                {
                    int ofs = 4;
                    if (buf[3] < 32)
                        fatal("Absolute segments not supported in %s", name);
                    if (buf[0] &1)
                    {
                        datalen = *(int*)(buf + ofs);
                        ofs += 4;
                    }
                    else
                    {
                        datalen = *(short*)(buf + ofs);
                        ofs += 2;
                    }
                    if (lnamecount < buf[ofs] - 1 || lnamecount < buf[ofs + 1] 
                        - 1)
                        fatal("Not enough lnames in %s", name);
                    if (!stricmp(lnames[buf[ofs] - 1], "$$BROWSE") && !stricmp
                        (lnames[buf[ofs + 1] - 1], "BROWSE"))
                    {
                        index = segindex;
                        data = AllocateMemory(datalen);
                    }
                }
                break;
            case 0xa0:
            case 0xa1:
                // ledata
                if (buf[3] == index && data)
                {
                    int offset;
                    if (buf[0] &1)
                    {
                        offset = *(int*)(buf + 4);
                        ofs = 4;
                    }
                    else
                    {
                        offset = *(unsigned short*)(buf + 4);
                        ofs = 2;
                    }
                    memcpy(data + offset, buf + 4+ofs, size - 2-ofs);
                }
                break;
            case 0x96:
                // lnames
                size -= 4;
                ofs = 3;
                while (size > ofs)
                {
                    char *v = buf + ofs;
                    ofs += readname(lname, &v) + 1;

                    lnames[lnamecount++] = strdup(lname);
                }
                break;
        }
    }
}

/*
 * Read in a module
 */

uint ReadModule(FILE *infile, char *name)
{
    uint temp = FALSE;
    curfile = infile;
    endofmodule = FALSE;

    while (!endofmodule)
    {
        switch (ReadObjectRecord(infile, name))
        {
            case MODQUIT:
                endofmodule = TRUE;
                break;

        }
    }
    return ;
}
