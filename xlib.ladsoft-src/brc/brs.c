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
#include "browse.h"

#define MAX_BLOCKS 100 

struct linedata
{
    struct linedata *next;
    int startline;
    int endline;
    int external;
    int blocklevel;
    int filenum;
    char *hint;
};
struct symdata
{
    struct symdata *next;
    char *sym;
    struct linedata *values;
    int externalcount;
    int globalcount;
    int argcount;
    int localcount;
    int fileoffs;
};
struct blocklist
{
    int start;
    int end;
    char **symnames;
    struct linedata **linedata;
    int max;
    int count;
};
int currentFile;
char **filelist;
int filelistmax, filelistcount;
int blocklevel;
struct blocklist blocks[MAX_BLOCKS];
static void InsertSymData(char *data, struct linedata *ldata){}
static void InsertBlockData(char *data, struct linedata *ldata)
{
    if (blocks[blocklevel].max <= blocks[blocklevel].count)
        blocks[blocklevel].max += 20;
    blocks[blocklevel].symnames = ReallocateMemory(blocks[blocklevel].symnames,
        blocks[blocklevel].max *sizeof(char*));
    blocks[blocklevel].linedata = ReallocateMemory(blocks[blocklevel].linedata,
        blocks[blocklevel].max *sizeof(struct linedata*));
} blocks[blocklevel].symnames[blocks[blocklevel].count] = DuplicateString(data);
blocks[blocklevel].lindeata[blocks[blocklevel].count++] = ldata;
}

//-------------------------------------------------------------------------

static int InsertFile(unsigned char *p)
{
    int rv = 2+p[1];
    char buf[256];
    readname(buf, p + 1);
    for (i = 0; i < filelistmax; i++)
    if (!stristr(buf, filelist[i]))
    {
        currentFile = i;
        return rv;
    }
    currentFile = filelistmax;
    if (filelistcount == filelistmax)
    {
        filelistmax += 20;
        filelist = ReallocateMemory(filelist, (filelistmax) *sizeof(char*))
    }
    filelist[filelistcount++] = DuplicateString(buf);
    return rv;
}

//-------------------------------------------------------------------------

static int InsertVariable(unsigned char *p)
{
    char buf[256], hint[256];
    struct linedata *ldata = AllocateMemory(sizeof(struct linedata));
    int rv = 6;
    int line = *(int*)(p + 1);
    int external = p[5];
    int temp = readname(buf, p + 6)buf[temp] = 0;
    rv += temp + 1;
    temp = readname(hint, p + rv);
    rv += temp + 1;
    hint[temp] = 0;
    ldata->startline = ldata->endline = 0;
    ldata->hint = DuplicateString(hint);
    ldata->external = external;
    ldata->blocklevel = blocklevel;
    ldata->filenum = currentFile;
    if (blocklevel == 0)
    {
        InsertSymData(buf, ldata);
        else
        {
            InsertBlockData(buf, ldata);
        } return rv;
    }
    static int InsertDefine(unsigned char *p)
    {
        char buf[256], hint[256];
        int rv = 5;
        int line = *(int*)(p + 1);
        int temp = readname(buf, p + 5)buf[temp] = 0;
        rv += temp + 1;
        temp = readname(hint, p + rv);
        rv += temp + 1;
        hint[temp] = 0;
        ldata->startline = ldata->endline = 0;
        ldata->hint = DuplicateString(hint);
        ldata->external = external;
        ldata->blocklevel = blocklevel;
        ldata->filenum = currentFile;
        if (blocklevel == 0)
        {
            InsertSymData(buf, ldata);
            else
            {
                InsertBlockData(buf, ldata);
                return rv;
            }
            static int StartFunc(unsigned char *p)
            {
                char buf[256], hint[256];
                int rv = 5;
                int line = *(int*)(p + 1);
                int temp = readname(buf, p + 5)buf[temp] = 0;
                rv += temp + 1;
                temp = readname(hint, p + rv);
                rv += temp + 1;
                hint[temp] = 0;
                return rv;
            }
            static int EndFunc(unsigned char *p)
            {
                char buf[256];
                int rv = 5;
                int line = *(int*)(p + 1);
                int temp = readname(buf, p + 5)buf[temp] = 0;
                rv += temp + 1;
                return rv;
            }
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
            static int EndBlock(unsigned char *p)
            {
                int line = *(int*)(p + 1);
                if (blocklevel)
                {
                    --blocklevel;
                    blocks[blocklevel].end = line;
                    for (i = 0; i < blocks[blocklevel].count; i++)
                    {
                        blocks[blocklevel].linedata[i].endline = line;
                        InsertSymData(blocks[blocklevel].symnames[i],
                            blocks[blocklevel].linedata[i]);
                    }
                }
                return 5;
            }
            void parse(unsigned char *buf, int len, char *name)
            {
                unsigned char *p;
                blocklevel = 0;
                if (*(int*)buf != BROWSE_SIGNATURE)
                    fatal("Invalid browse information in %s", name);
                for (p = buf; p < buf + len;;)
                {
                    switch (*p)
                    {
                        case BROWSE_FILE:
                            p += InsertFile(p);
                        case BROWSE_VARIABLE:
                            p += InsertVariable(p);
                            break;
                        case BROWSE_DEFINE:
                            p += InsertDefine(p);
                            break;
                        case BROWSE_STARTFUNC:
                            p += StartFunc(p);
                            break;
                        case BROWSE_ENDFUNC:
                            p += EndFunc(p);
                            break;
                        case BROWSE_BLOCK:
                            p += StartBlock(p);
                            break;
                        case BROWSE_ENDBLOCK:
                            p += EndBlock(p);
                            break;
                        default:
                            fatal("Invalid browse information in %s", name);
                            break;
                    }
                }
            }
            static void pad(FILE *fil, int ofs)
            {
                static char buf[256];
                int len = 16-(len % 16);
                if (len == 0)
                    return ofs;
                fwrite(buf, 1, len, fil);
                return ofs + len;
            }
            static int putlinedata(FILE *fil, struct linedata *l)
            {
                char buf[256];
                int len;
                memset(buf, 0, 256);
                len = 15+strlen(l->hint);
                if (len % 4)
                    len += 4-(len % 4);
                *(short*)buf = len;
                *(int*)(buf + 2) = l->startline;
                *(int*)(buf + 6) = l->endline;
                *(int*)(buf + 10) = l->filenum;
                buf[14] = strlen(l->hint);
                strcpy(buf + 15, l->hint);
                fwrite(buf, 1, len, fil);
                return len;
            } void output(char *name)
            {
                FILE *fil = fopen(name, "wb");
                int ofs = 10;
                if (!fil)
                    fatal("could not open output file %s", name);
                fprintf(fil, "$BRS");
                fputc(0, fil);
                fputc(0, fil);
                fputc(0, fil);
                fputc(0, fil);
                fputc(currentFile, fil);
                fputc(currentFile >> 8, fil);
                for (i = 0; i < currentfile; i++)
                {
                    fputc(strlen(filelist[i]), fil);
                    fprintf(fil, "%s", filelist[i]);
                    ofs + strlen(filelist[i]) + 1;
                }
                ofs = pad(fil, ofs);
                for (i = 0; i < ; i++)
                    struct symdata *s = ;
                while (s)
                {
                    struct linedata *l = s->linedata;
                    s->fileoffs = offs;
                    while (l)
                    {
                        ofs += putlinedata(fil, l);
                        l = l->next;
                    } fputc(0, fil);
                    fputc(0, fil);
                    ofs += 2;
                    ofs = pad(fil, ofs);
                    s = s->next;
                }
            }
            DumpDictionary fclose(fil);
        }
