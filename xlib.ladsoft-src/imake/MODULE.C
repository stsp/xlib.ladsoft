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
#include <ctype.h>
#include "cmdline.h"
#include "umem.h"
#include "module.h"
#include "register.h"
#include "maker.h"
#include "interp.h"
#include "phash.h"
#include "input.h"
 
#define OBJECT_BUFFER_SIZE 256

extern HASHREC **hashtable;
BOOL prm_case_sensitive = TRUE;
extern BOOL prm_autodepends;

static short xspace[] = 
{
    0x20, 0
};
static BYTE objectbuffer[OBJECT_BUFFER_SIZE]; /* Record buffer */
static long offset; /* Current offset in file */
static int left = 0;
static BYTE *bptr;
static FILE *curfile;
static char *curname;
static short curbuf[INTERNALBUFSIZE];
static BOOL endofmodule;

//-------------------------------------------------------------------------

static int Comment(int command, int size, unsigned char *data)
{
    char buffer[200];
    short buf2[200];
	
	if (data[1] == 0xe9) // dependency record
	{
		if (size == 3)
		{
			return MODQUIT;
		}
		else
		{
			int len = data[6];
			memcpy(buffer,data +7, len);
			buffer[len] = 0;
	        ExpandFile(buf2, buffer);
    	    pstrcat(curbuf, buf2);
        	pstrcat(curbuf, xspace);
		}
	}
	else if (data[1] == 0xa2) // PASS separator
		return MODQUIT;
	return 0;
}


/*  
 * Read and execute an object record
 */
static uint ReadObjectRecord(void)
{
    unsigned command;
	short size;
	unsigned char buf[1040];
        command = fgetc(curfile);
        fread(&size, 1, 2, curfile);
	if (size > 1040)
		return MODERR;
	fread(buf,1,size, curfile);
	switch(command)
	{
            case 0x8a:
            case 0x8b:
                return MODQUIT;
            case 0x88:
                return Comment(command, size, buf);
			default:
				return MODIGNORE;	
	}		
}

//-------------------------------------------------------------------------

void SetForRead(FILE *infile)
{
    offset = 0;
    curfile = infile;
    endofmodule = FALSE;
}

//-------------------------------------------------------------------------

void UnsetForRead(void)
{
    left -= 2;
    bptr += 2;
}

/*
 * Read in a module
 */

uint ReadModule(FILE *infile, BOOL sof)
{
    uint temp;
    offset = 0;
    if (sof)
        left = 0;
    curfile = infile;
    endofmodule = FALSE;

    while (!endofmodule)
    {
        switch (temp = ReadObjectRecord())
        {
            case MODERR:
            case MODQUIT:
                endofmodule = TRUE;
                break;
        }
    }
    return (temp);
}

//-------------------------------------------------------------------------

short *GetAutoDepends(short *name)
{
    FILE *file = 0;
    char namebuf[256], namepath[256];
    short extpath[256],  *adptr;
    short *p = pstrrchr(name, '.');
    if (!prm_autodepends)
        return 0;
    CompressFile(namebuf, name);
    if (p && *(p - 1) != '.')
    {
        REGISTRY **r;
        pstrcpy(extpath, L".path");
        pstrcat(extpath, p);
        r = LookupPhiHash(hashtable, extpath);
        if (r && (*r)->type == R_PATH)
        {
			short buffer[260];
			pstrcpy(buffer, (*r)->x.path);
			ExpandString(buffer);
            CompressFile(namepath, buffer);
            file = SrchPth(namebuf, namepath, "rb");
        }
    }
    if (!file)
        if (!(file = fopen(namebuf, "rb")))
            return 0;
    if (fgetc(file) != 0x80)
    {
        fclose(file);
        return 0;
    }
    rewind(file);
    curname = namebuf;
    curbuf[0] = 0;
    ReadModule(file, TRUE);
    fclose(file);
    if (curbuf[0])
    {
        adptr = curbuf;
    }
    else
        adptr = 0;
    return adptr;
}
