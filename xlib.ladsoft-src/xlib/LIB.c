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
#include <string.h>
#include <ctype.h>
#include "cmdline.h"
#include "umem.h"
#include "module.h"
#include "lib.h"
#include "libm.h"
#include "errors.h"
#include "dict.h"

extern HASHREC **publichash;
extern long liboffset;
extern int procrevision;
extern BOOL prm_case_sensitive;
extern LIST *objlist; /* List of object files */
extern LIST *attriblist;
extern MODULE *modules;

extern long dictofs, dictpages, pagesize;

int straccmp(char *one, char *two)
{
    while (*one)
    {
        if (toupper(*one) != toupper(*two))
            return 1;
        one++;
        two++;
    }
    if (*two)
        return 1;
    return 0;

}

//-------------------------------------------------------------------------

void ReadLib(int maxmode, char *libname)
{
    FILE *lib;
    BYTE buf[512];
    liboffset = 0;
    if (!(lib = fopen(libname, "rb")))
    {
        if (maxmode &(MD_EXTRACT))
            fatal("Missing library file %s", libname);
        if (maxmode &MD_DELETE && !(maxmode &MD_INSERT))
            fatal("Missing library file %s", libname);
    }
    else
    {
        uint command;
        long pv;
        int i;
        int done = FALSE;
        long ofs, pages, pagesize;
        char mode;
        fread(buf, 1, 10, lib);
        if (buf[0] != 0xf0)
            fatal("Not a library %s", libname);
        pagesize = *(short*)(buf + 1) + 3;
        ofs = *(long*)(buf + 3);
        pages = *(short*)(buf + 7);
        mode = *(buf + 9);
        if (!(mode &1))
        {
            if (prm_case_sensitive)
                Error("Library not case sensitive");
            prm_case_sensitive = FALSE;
        }
		fseek(lib, pagesize, SEEK_SET);

        liboffset = pagesize;
        while (!feof(lib) && !done)
        {
            long t;
            done = ReadModule(lib, libname);
			liboffset += pagesize-1;
			liboffset &= -pagesize;
			fseek(lib, liboffset, SEEK_SET);
        }
        if (!done)
            fatal("Missing module end record in library %s", libname);
        if (maxmode &(MD_INSERT | MD_DELETE))
        {
            char buffer[260];
            FILE *bakfile;
            char iobuf[2048];
			rewind(lib);
            strcpy(buffer, libname);
            StripExt(buffer);
            AddExt(buffer, ".bak");
            if (!(bakfile = fopen(buffer, "wb")))
                fatal("Could not open backup file");
            while (!feof(lib))
            {
                int size = fread(iobuf, 1, 2048, lib);
                fwrite(iobuf, 1, size, bakfile);
            }
            fclose(bakfile);
        }
        fclose(lib);
    }
}

//-------------------------------------------------------------------------

void Extract(int maxmode, char *libname)
{
    LIST *o = objlist,  *a = attriblist;
    while (o)
    {
        int attrib = (int)a->data;
        if (attrib &MD_EXTRACT)
        {
            char buf[260];
            MODULE *m = modules;
            strcpy(buf, o->data);
            StripExt(buf);
            while (m)
            {
                if (!straccmp(buf, m->modname))
                    break;
                m = m->link;
            }
            if (!m)
                Error("Module %s does not exist in library", buf);
            else
            {
                FILE *out;
                long size = m->len;
                AddExt(buf, ".obj");
                if (!(out = fopen(buf, "wb")))
                    fatal("Can't open %s for write", buf);
                fwrite(m->data, 1, size, out);
                fclose(out);
            }
        }
        o = o->link;
    }
}

//-------------------------------------------------------------------------

void Insert(int maxmode, char *libname)
{
    LIST *o = objlist,  *a = attriblist;
    while (o)
    {
        int attrib = (int)a->data;
        if (attrib &MD_INSERT)
        {
            char buf[260];
            MODULE *m = modules;
            strcpy(buf, o->data);
            StripExt(buf);
            while (m)
            {
                if (!straccmp(buf, m->modname))
                    break;
                m = m->link;
            }
            if (m)
                Error("Module %s exists in library", buf);
            else
            {
                FILE *f = fopen(o->data, "rb");
                if (!f)
                    Error("Can't open input module %s", o->data);
                else
                {
                    if (strstr(o->data, ".DEF"))
                        CreateImports(f, o->data);
                    else
                        ReadModule(f, o->data);
                    fclose(f);
                }
            }
        }
        o = o->link;
    }
}

//-------------------------------------------------------------------------

void Delete(int maxmode, char *libname)
{
    LIST *o = objlist,  *a = attriblist;
    while (o)
    {
        int attrib = (int)a->data;
        if (attrib &MD_DELETE)
        {
            char buf[260];
            MODULE *m = modules,  **r = &modules;
            strcpy(buf, o->data);
            StripExt(buf);
            while (m)
            {
                if (!straccmp(buf, m->modname))
                    break;
                r = &m->link;
                m = m->link;
            }
            if (!m)
            {
                if (!(maxmode &MD_INSERT))
                    Error("Module %s does not exist in library", buf);
            }
            else
            {
                int i;
                for (i = 0; i < HASH_TABLE_SIZE; i++)
                {
                    HASHREC **p = &publichash[i];
                    while (*p)
                    {
                        PUBLIC *r =  *p;
                        if (r->mod == m)
                        {
                            *p = r->link;
                            DeallocateMemory(r->name);
                            DeallocateMemory(r);
                        }
                        else
                            p = &r->link;
                    }
                }
                *r = m->link;
                DeallocateMemory(m->name);
                DeallocateMemory(m->data);
                DeallocateMemory(m);
            }
        }
        o = o->link;
    }
}

//-------------------------------------------------------------------------

void OutputLibrary(char *libname)
{
    FILE *lib;
    char buf[10];
    lib = fopen(libname, "wb");
    if (!lib)
        Error("Can't open dictionary %s for write", libname);
    else
    {
        int i;
        long t;
        MODULE *m = modules;
        long libofs = pagesize;
        buf[0] = 0xf0;
        buf[9] = prm_case_sensitive ? 1 : 0;
        *(short*)(buf + 1) = pagesize - 3;
        *(long*)(buf + 3) = dictofs;
        *(short*)(buf + 7) = dictpages;
        fwrite(buf, 1, 10, lib);
		fseek(lib, pagesize, SEEK_SET);
        while (m)
        {
            long size = m->len;
            fwrite(m->data, 1, size, lib);
            libofs += size + pagesize - 1;
			libofs &= -pagesize;
			fseek (lib, libofs, SEEK_SET);
            m = m->link;
        }
        buf[0] = 0xf1;
        *(short*)(buf + 1) = pagesize - 3;
        fwrite(buf, 1, 3, lib);
		libofs += pagesize + 511;
		libofs &= -512;
		fseek(lib, libofs, SEEK_SET);
        WriteDictionary(lib);
        fclose(lib);
    }
}
