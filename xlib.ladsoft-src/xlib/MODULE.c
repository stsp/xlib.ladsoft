/* 
XLIB Librarian
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
#include <ctype.h>
#include "cmdline.h"
#include "umem.h"
#include "module.h"
#include "errors.h"

extern BOOL prm_case_sensitive;

char *modname;
long modnumber = 0;
HASHREC **publichash;
long liboffset;

MODULE *modules = 0,  *curmod;

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

void GetModName(BYTE *data, MODULE *mod)
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
    mod->modname = AllocateMemory(strlen(y) + 1);
    strcpy(mod->modname, y);
}

//-------------------------------------------------------------------------

void PublicDefinitions(int command, int size, BYTE *data)
{
    int len, offsize = 2, skipbase;
    char buf[NAMEMAX + 1];

    if (*data &1)
        offsize = 4;
    data += 3;
    skipbase = !*(short*)data;
    skipind(&data, &size);
    skipind(&data, &size);
    if (skipbase)
    {
        data += 2;
        size -= 2;
    }
    while (size > 1)
    {
        PUBLIC *p = AllocateMemory(sizeof(PUBLIC)),  *q;
        len = readname(buf, &data);
        p->name = AllocateMemory(len + 1);
        strcpy(p->name, buf);
        p->mod = curmod;
        p->link = 0;
        data += offsize;
        size -= offsize + len + 1;
        skipind(&data, &size);
        if ((q = AddHash(publichash, p)) != 0)
        {
            Error("Public %s defined in both %s and %s", unmangle(p->name), q
                ->mod->name, p->mod->name);
            DeallocateMemory(p->name);
            DeallocateMemory(p);
        }
    }
    if (size != 1)
        fatal("Bad publics record in module %s", (curmod)->name);
}

//-------------------------------------------------------------------------

void Comment(int command, int size, BYTE *data)
{
    BYTE x = *(data += 3);
    BYTE type =  *data++;
    BYTE cmd =  *data++;
    BYTE one =  *data++;
    if (cmd == 0xa0 && one == 1)
    {
        BYTE ordflag =  *data++;
        int len;
        char buf[256];
        PUBLIC *p = AllocateMemory(sizeof(PUBLIC)),  *q;
        len = readname(buf, &data);
        p->name = AllocateMemory(len + 1);
        strcpy(p->name, buf);
        p->mod = curmod;
        p->link = 0;
        if ((q = AddHash(publichash, p)) != 0)
        {
            Error("Public %s defined in both %s and %s", unmangle(p->name), q
                ->mod->name, p->mod->name);
            DeallocateMemory(p->name);
            DeallocateMemory(p);
        }
    }
}

/*  
 * Read and execute an object record
 */
static uint ReadObjectRecord(MODULE *mod)
{
    uint command, size = 0;
    uint allocLen = 1;
    while (TRUE)
    {
	uint oldAlloc = allocLen;
        long offset = mod->offset;
        if (feof(curfile))
            fatal("Invalid module file format for module %s", mod->name);
        command = fgetc(curfile);
        fread(&size, 1, 2, curfile);
	while (mod->len + size + 3 > allocLen)
		allocLen *= 2;
	if (oldAlloc != allocLen)
	{
	        mod->data = ReallocateMemory(mod->data, allocLen);
	}
        mod->data[mod->offset++] = command;
        *(short*)(mod->data + mod->offset) = size;
        mod->offset += 2;
        mod->len += 3+size;
        liboffset += 3+size;
        if (fread(mod->data + mod->offset, 1, size, curfile) != size)
            fatal("Invalid object file format in %s", mod->name);
        mod->offset += size;
        switch (command)
        {
            case 0x80:
                GetModName(mod->data + offset, mod);
                break;
            case 0x8a:
            case 0x8b:
                return MODQUIT;
            case 0x90:
            case 0x91:
                PublicDefinitions(command, size, mod->data + offset);
                break;
            case 0x88:
                Comment(command, size, mod->data + offset);
                break;
            case 0xf1:
                return LIBEND;
        }
    }
}

/*
 * Read in a module
 */

uint ReadModule(FILE *infile, char *name)
{
    uint temp = FALSE;
    MODULE *mod,  **m = &modules;
    curfile = infile;
    endofmodule = FALSE;
    mod = AllocateMemory(sizeof(MODULE));
    mod->link = 0;
    mod->name = AllocateMemory(strlen(name) + 1);
    strcpy(mod->name, name);
    mod->data = AllocateMemory(1);
    mod->len = 0;
    mod->offset = 0;
    mod->modname = 0;
    curmod = mod;

    while (!endofmodule)
    {
        switch (ReadObjectRecord(mod))
        {
            case LIBEND:
                temp = TRUE;
            case MODQUIT:
                endofmodule = TRUE;
                break;

        }
    }
    if (!temp)
    {
        modnumber++;
        while (*m)
            m =  *m;
        *m = mod;
    }
    mod->data = ReallocateMemory(mod->data, mod->len);
    return (temp);
}
