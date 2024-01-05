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

#define OBJECT_BUFFER_SIZE 256

extern HASHREC **hashtable;
extern short xpath[];
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
static short *curbuf;
static BOOL endofmodule;
static BYTE putbackbuf;

void ReadHeader(void);
void CheckSum(void);
void ModEnd(void);
void SetMAUs(void);
void Assign(void);
void DateTime(void);
void Comment(void);

void EnumeratedData(void);
void ReadFixup(void);
void PublicDefinitions(void);
void ExternDefinitions(void);
void SectionAlignment(void);
void SectionStart(void);
void SectionDefinitions(void);
void PublicSetAttribs(void);
/* List of defined MUFOM records.  Bit defs:
 *  TE_REPORT: Generate a warning message for this record
 *  TE_IGNORE: Ignore the record
 */
static TYPEXEC main_exec_list[] = 
{
    {
        "AD", TE_REPORT - TE_REPORT, SetMAUs
    }
    , 
    {
        "AS", TE_REPORT - TE_REPORT, Assign
    }
    , 
    {
        "AT", TE_REPORT - TE_REPORT, PublicSetAttribs
    }
    , 
    {
        "CO", TE_REPORT - TE_REPORT, Comment
    }
    , 
    {
        "CS", TE_REPORT - TE_REPORT, CheckSum
    }
    , 
    {
        "DT", TE_REPORT - TE_REPORT, DateTime
    }
    , 
    {
        "LD", TE_REPORT - TE_REPORT, EnumeratedData
    }
    , 
    {
        "LR", TE_REPORT - TE_REPORT, ReadFixup
    }
    , 
    {
        "MB", TE_REPORT - TE_REPORT, ReadHeader
    }
    , 
    {
        "ME", TE_REPORT - TE_REPORT, ModEnd
    }
    , 
    {
        "NI", TE_REPORT - TE_REPORT, PublicDefinitions
    }
    , 
    {
        "NN", TE_REPORT - TE_REPORT, ExternDefinitions
    }
    , 
    {
        "NX", TE_REPORT - TE_REPORT, ExternDefinitions
    }
    , 
    {
        "SA", TE_REPORT - TE_REPORT, SectionAlignment
    }
    , 
    {
        "SB", TE_REPORT - TE_REPORT, SectionStart
    }
    , 
    {
        "ST", TE_REPORT - TE_REPORT, SectionDefinitions
    }
    , 
    {
        "\0\0", 0, 0
    }
};
long theoffset(void)
{
    return (offset - left);
}

/*
 * Quit with a message if a bad record is encountered
 */
void BadObjectFile(void)
{
    fatal("Bad object module %s near file offset 0x%x", curname, theoffset());
}

//-------------------------------------------------------------------------

void CheckForPeriod(void)
{
    if (ReadChar() != '.')
        BadObjectFile();
}

//-------------------------------------------------------------------------

BOOL CheckForComma(BOOL musthave)
{
    BYTE rv = ReadChar();
    if (rv == ',')
        return (TRUE);
    if (musthave || rv != '.')
        BadObjectFile();
    return (FALSE);
}

//-------------------------------------------------------------------------

void WadeToPeriod(void)
{
    while (ReadChar() != '.')
        ;
}

/*
 * Perform  function
 */
static void ReadHeader(void)
{
    BYTE rv;
    char buffer[100];
    short buf2[100];
    rv = ReadChar();
    while ((rv) != '.' && rv != ',')
        rv = ReadChar();
    if (rv == ',')
    {
        int i;
        ReadString(buffer, 60);
        CheckForPeriod();
        for (i = 0; i < strlen(buffer); i++)
            buffer[i] = tolower(buffer[i]);
        ExpandFile(buf2, buffer);
        pstrcat(curbuf, buf2);
        pstrcat(curbuf, xspace);
    }
}

//-------------------------------------------------------------------------

static void ModEnd(void)
{
    WadeToPeriod();
    endofmodule = TRUE;
}

//-------------------------------------------------------------------------

static void Comment(void)
{
    char buffer[100];
    short buf2[100];
    int val = ReadNumber(0);
    CheckForComma(TRUE);
    ReadString(buffer, 100);
    CheckForPeriod();
    if (val == 0x102)
    {
        int i;
        for (i = 0; i < strlen(buffer); i++)
            buffer[i] = tolower(buffer[i]);
        ExpandFile(buf2, buffer);
        pstrcat(curbuf, buf2);
        pstrcat(curbuf, xspace);
    }
    else if (val == 0x101)
        endofmodule = TRUE;
}

//-------------------------------------------------------------------------

static void CheckSum(void)
{
    WadeToPeriod();
}

//-------------------------------------------------------------------------

static void SetMAUs(void)
{
    WadeToPeriod();
}

//-------------------------------------------------------------------------

static void Assign(void)
{
    WadeToPeriod();
}

//-------------------------------------------------------------------------

static void DateTime(void)
{
    WadeToPeriod();
}

//-------------------------------------------------------------------------

void EnumeratedData(void)
{
    WadeToPeriod();
}

//-------------------------------------------------------------------------

void ReadFixup(void)
{
    WadeToPeriod();
}

//-------------------------------------------------------------------------

void PublicDefinitions(void)
{
    WadeToPeriod();
}

//-------------------------------------------------------------------------

void ExternDefinitions(void)
{
    WadeToPeriod();
}

//-------------------------------------------------------------------------

void SectionAlignment(void)
{
    WadeToPeriod();
}

//-------------------------------------------------------------------------

void SectionStart(void)
{
    WadeToPeriod();
}

//-------------------------------------------------------------------------

void SectionDefinitions(void)
{
    WadeToPeriod();
}

//-------------------------------------------------------------------------

void PublicSetAttribs(void)
{
    WadeToPeriod();
}

/*
 * Read data into the objectbuffer
 */
void ReadFile(void)
{
    if (!feof(curfile))
    {
        left = fread(objectbuffer, 1, OBJECT_BUFFER_SIZE, curfile);
        offset += left;
        if (left < 0)
            fatal("File read error in module %s", curname);
        bptr = objectbuffer;
    }
}

/*
 * Read a char & do checksumming
 */
BYTE ReadCharNOCS(void)
{
    BYTE rv;
    if (putbackbuf)
    {
        rv = putbackbuf;
        putbackbuf = 0;
        return (rv);
    }
    while (TRUE)
    {
        if (!left)
        {
            ReadFile();
            if (!left)
                fatal("missing modend statement in module %s", curname);
        }
        left--;
        rv =  *bptr++;
        if (rv > 31)
            break;
    }
    return (rv);
}

//-------------------------------------------------------------------------

BYTE ReadChar(void)
{
    BYTE rv = ReadCharNOCS();
    return (rv);
}

//-------------------------------------------------------------------------

void putback(BYTE ch)
{
    putbackbuf = ch;
}

//-------------------------------------------------------------------------

long ReadNumber(uint digits)
{
    int i;
    long rv = 0;
    if (!digits)
        digits = 200;
    for (i = 0; i < digits; i++)
    {
        BYTE ch = ReadChar();
        if (!isxdigit(ch))
        {
            putback(ch);
            break;
        }
        rv *= 16;
        ch -= '0';
        if (ch > 9)
            ch -= 7;
        rv += ch;
    }
    return (rv);
}

//-------------------------------------------------------------------------

void ReadString(char *buf, int len)
{
    int slen = (int)(ReadNumber(2)), i;
    int rlen = slen > len - 1 ? len - 1: slen;
    for (i = 0; i < rlen; i++)
        if (prm_case_sensitive)
            buf[i] = ReadChar();
        else
            buf[i] = toupper(ReadChar());
    if (rlen >= 0)
        buf[rlen] = 0;
    else
        rlen = 0;
    for (i = rlen; i < slen; i++)
        ReadChar();
}


/*  
 * Read and execute an object record
 */
static uint ReadObjectRecord(void)
{
    uint command;
    TYPEXEC *p = main_exec_list;
    command = ReadChar() + (ReadChar() << 8);

    /* Search the dispatch table */

    while (p->select[0])
    {
        if (*((uint*)p->select) == command)
        {
            uint temp = p->flags;
            if (temp &TE_REPORT)
                printf(
                    "Record type %c%c ignored at offset 0x%lx in module %s\n",
                    p->select[0], p->select[1], theoffset() - 2, curname);
            if (temp &TE_ERROR)
                return (MODERR);
            if ((temp &TE_IGNORE) || !p->run)
             /* Undefined functions are ignored, better have the TE_REPORT bit
                 set!!! */
                return (MODIGNORE);

            /* Dispatch the function */
            p->run();
            break;
        }
        p++;
    }
    /* Return an error if not found */
    if (p->select[0] == 0)
        return (MODERR);

    /* Return the record type if was found */
    return (command);
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
                BadObjectFile();
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
    char cbuf[11];
    char namebuf[100], namepath[200];
    short extpath[100], lbuf[4096],  *adptr;
    short *p = pstrrchr(name, '.');
    if (!prm_autodepends)
        return 0;
    CompressFile(namebuf, name);
    if (p && *(p - 1) != '.')
    {
        REGISTRY **r;
        pstrcpy(extpath, xpath);
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
    fgets(cbuf, 10, file);
    if (cbuf[0] != 'M' || cbuf[1] != 'B')
    {
        fclose(file);
        return 0;
    }
    rewind(file);
    curname = namebuf;
    curbuf = lbuf;
    lbuf[0] = 0;
    ReadModule(file, TRUE);
    fclose(file);
    if (lbuf[0])
    {
        adptr = AllocateMemory(pstrlen(lbuf) *2+2);
        pstrcpy(adptr, lbuf);
    }
    else
        adptr = 0;
    return adptr;
}
