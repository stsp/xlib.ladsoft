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
#include "cmdline.h"
#include "browse.h"
#include "umem.h"
#include "dict.h"

#define MAX_BLOCKS 100 

char **filelist;
int filelistmax, filelistcount;
HASHREC **symhash;

static int currentFile;
static int blocklevel;
static struct blocklist blocks[MAX_BLOCKS];

static int readname2(char *s, char *buf)
{
    return readname(s, &buf);
} static void InsertSymData(char *name, struct linedata *ldata)
{
    struct symdata *sym = 0;
    struct linedata *l,  **ol;
    if (!symhash)
    {
        symhash = CreateHashTable(HASH_TABLE_SIZE);
    } 
    else
    {
        sym = LookupHash(symhash, name);
        if (sym)
            sym = *(struct symdata **)sym;
    }

    if (!sym)
    {
        sym = (struct symdata*)AllocateMemory(sizeof(struct symdata));
        sym->name = DuplicateString(name);
        AddHash(symhash, sym);
    }

    if (ldata->external)
        sym->externalcount++;
    else if (ldata->blocklevel == 1)
        sym->argcount++;
    else if (ldata->blocklevel > 1)
        sym->localcount++;
    else
        sym->globalcount++;

    // if it is global scoped override any prototype
    if (ldata->blocklevel == 0)
    {
        l = sym->values;
        ol = &sym->values;
        while (l)
        {
            if (l->blocklevel == 0)
            {
                if (l->type == BRS_STARTFUNC)
                {
                    DeallocateMemory(ldata);
                    return ;
                } 
                else if (ldata->type == BRS_STARTFUNC)
                {
                    *ol = ldata;
                    ldata->link = l->link;
                    DeallocateMemory(l);
                    return ;
                }
            }
            ol = &l->link;
            l = l->link;
        }
    }
    l = sym->values;
    ol = &sym->values;
    while (l)
    {
        if (l->filenum == ldata->filenum)
        {
            if (l->startline == ldata->startline && l->endline == ldata
                ->endline)
                return ;
            if (l->startline > ldata->startline)
            {
                ldata->link =  *ol;
                *ol = ldata;
                return ;
            }
        }
        else if (l->filenum < ldata->filenum)
        {
            ldata->link =  *ol;
            *ol = ldata;
            return ;
        }
        ol = &l->link;
        l = l->link;
    }
    ldata->link =  *ol;
    *ol = ldata;

}

//-------------------------------------------------------------------------

static void InsertBlockData(char *data, struct linedata *ldata)
{
    if (blocks[blocklevel].max <= blocks[blocklevel].count)
    {
        blocks[blocklevel].max += 20;
        blocks[blocklevel].symnames = ReallocateMemory
            (blocks[blocklevel].symnames, blocks[blocklevel].max *sizeof(char*))
            ;
        blocks[blocklevel].linedata = ReallocateMemory
            (blocks[blocklevel].linedata, blocks[blocklevel].max *sizeof(struct
            linedata*));
    } blocks[blocklevel].symnames[blocks[blocklevel].count] = (char*)
        DuplicateString(data);
    blocks[blocklevel].linedata[blocks[blocklevel].count++] = ldata;
}

//-------------------------------------------------------------------------

static int InsertFile(unsigned char *p)
{
    int rv = 2+p[1], i;
    char buf[256];
    readname2(buf, p + 1);
    for (i = 0; i < filelistcount; i++)
    if (!stricmp(buf, filelist[i]))
    {
        currentFile = i;
        return rv;
    }
    currentFile = filelistcount;
    if (filelistcount == filelistmax)
    {
        filelistmax += 20;
        filelist = ReallocateMemory(filelist, (filelistmax) *sizeof(char*));
    }
    filelist[filelistcount++] = DuplicateString(buf);
    return rv;
}

//-------------------------------------------------------------------------

static int InsertVariable(unsigned char *p)
{
    char buf[256], hint[256];
    struct linedata *ldata = AllocateMemory(sizeof(struct linedata));
    int rv = 9;
    int line = *(int*)(p + 1);
    int charpos = *(short*)(p + 5);
    int external = *(short*)(p + 7);
    int temp = readname2(buf, p + 9);
    buf[temp] = 0;
    rv += temp + 1;
    temp = readname2(hint, p + rv);
    rv += temp + 1;
    hint[temp] = 0;
    ldata->type =  *p;
    ldata->startline = line;
    ldata->endline = external &BRF_STATIC ?  - 2:  - 1;
    ldata->charpos = charpos;
    ldata->hint = DuplicateString(hint);
    ldata->external = external &BRF_EXTERNAL;
    ldata->blocklevel = blocklevel;
    ldata->filenum = currentFile;
	ldata->typedata = external & BRF_TYPE ? 1 : 0;
    if (blocklevel == 0)
    {
        InsertSymData(buf, ldata);
    } 
    else
    {
        InsertBlockData(buf, ldata);
    }
    return rv;
}

//-------------------------------------------------------------------------

static int InsertDefine(unsigned char *p)
{
    char buf[256], hint[256];
    struct linedata *ldata = AllocateMemory(sizeof(struct linedata));
    int rv = 7;
    int line = *(int*)(p + 1);
    int charpos = *(short*)(p + 5);
    int temp = readname2(buf, p + 7);
    buf[temp] = 0;
    rv += temp + 1;
    temp = readname2(hint, p + rv);
    rv += temp + 1;
    hint[temp] = 0;
    ldata->type =  *p;
    ldata->startline = line;
    ldata->endline =  - 1;
    ldata->charpos = charpos;
    ldata->hint = DuplicateString(hint);
    ldata->external = 0;
    ldata->blocklevel = blocklevel;
    ldata->filenum = currentFile;
    if (blocklevel == 0)
    {
        InsertSymData(buf, ldata);
    } 
    else
    {
        InsertBlockData(buf, ldata);
    }
    return rv;
}

//-------------------------------------------------------------------------

static int StartFunc(unsigned char *p)
{
    char buf[256], hint[256];
    struct linedata *ldata = AllocateMemory(sizeof(struct linedata));
    int rv = 9;
    int line = *(int*)(p + 1);
    int charpos = *(short*)(p + 5);
    int external = *(short*)(p + 7);
    int temp = readname2(buf, p + 9);
    buf[temp] = 0;
    rv += temp + 1;
    temp = readname2(hint, p + rv);
    rv += temp + 1;
    hint[temp] = 0;
    ldata->type =  *p;
    ldata->startline = line;
    ldata->endline = external &BRF_STATIC ?  - 2:  - 1;
    ldata->charpos = charpos;
    ldata->hint = DuplicateString(hint);
    ldata->external = 0;
    ldata->blocklevel = blocklevel;
    ldata->filenum = currentFile;
    if (blocklevel == 0)
    {
        InsertSymData(buf, ldata);
    } 
    else
    {
        InsertBlockData(buf, ldata);
    }
    blocklevel++;
    if (blocklevel < MAX_BLOCKS)
    {
        blocks[blocklevel].count = 0;
        blocks[blocklevel].start = line;
    }
    return rv;
}

//-------------------------------------------------------------------------

static int EndFunc(unsigned char *p)
{
    char buf[256];
    int rv = 5, i;
    int line = *(int*)(p + 1);
    int temp = readname2(buf, p + 5);
	struct symdata *sym;
    buf[temp] = 0;
    rv += temp + 1;
    sym = LookupHash(symhash, buf);
    if (sym)
	{
        sym = *(struct symdata **)sym;
		sym->values->funcendline = line-1;
	}
    if (blocklevel)
    {
        if (blocklevel < MAX_BLOCKS)
        {
            blocks[blocklevel].end = line;
            for (i = 0; i < blocks[blocklevel].count; i++)
            {
                blocks[blocklevel].linedata[i]->endline = line;
                InsertSymData(blocks[blocklevel].symnames[i],
                    blocks[blocklevel].linedata[i]);
            }
        }
        --blocklevel;
    }
    return rv;
}

//-------------------------------------------------------------------------

static int StartBlock(unsigned char *p)
{
    int line = *(int*)(p + 1);
    ++blocklevel;
    if (blocklevel < MAX_BLOCKS)
    {
        blocks[blocklevel].count = 0;
        blocks[blocklevel].start = line;
    }
    return 5;
}

//-------------------------------------------------------------------------

static int EndBlock(unsigned char *p)
{
    int line = *(int*)(p + 1), i;
    if (blocklevel)
    {
        if (blocklevel < MAX_BLOCKS)
        {
            blocks[blocklevel].end = line;
            for (i = 0; i < blocks[blocklevel].count; i++)
            {
                blocks[blocklevel].linedata[i]->endline = line;
                InsertSymData(blocks[blocklevel].symnames[i],
                    blocks[blocklevel].linedata[i]);
            }
        }
        --blocklevel;
    }
    return 5;
}

//-------------------------------------------------------------------------

void ParseBRCData(unsigned char *buf, int len, char *name)
{
    unsigned char *p;
    blocklevel = 0;
    if (strncmp(buf, BRS_SIGNATURE, 4))
        fatal("Invalid browse information in %s", name);
    for (p = buf + 4; p < buf + len;)
    {
        switch (*p)
        {
            case BRS_STARTFILE:
                p += InsertFile(p);
                break;
            case BRS_VARIABLE:
                p += InsertVariable(p);
                break;
            case BRS_DEFINE:
                p += InsertDefine(p);
                break;
            case BRS_STARTFUNC:
                p += StartFunc(p);
                break;
            case BRS_ENDFUNC:
                p += EndFunc(p);
                break;
            case BRS_BLOCKSTART:
                p += StartBlock(p);
                break;
            case BRS_BLOCKEND:
                p += EndBlock(p);
                break;
            default:
                fatal("Invalid browse information in %s", name);
                break;
        }
    }
}
