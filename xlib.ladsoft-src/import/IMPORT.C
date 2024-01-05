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
#include <memory.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include "cmdline.h"
#include "umem.h"
#include "module.h"
#include "errors.h"

extern MODULE *modules,  *curmod;
extern long modnumber;

extern FILE *curfile;
extern HASHREC **publichash;

typedef struct import
{
    struct import *link;
    char name[256];
    char libname[256];
    char expname[256];
} IMP_STRUC;

static char libname[256];
static IMP_STRUC *import_list,  **last_import;

static uint header(FILE *fil)
{
    char ibuf[256],  *p;
    int l;
    fgets(ibuf, 256, fil);

    if (strncmp(ibuf, "LIBRARY", 7))
        return 0;

    p = ibuf + 7;
    while (*p && (*p == ' ' ||  *p == '\t'))
        p++;
    strcpy(libname, p);
    if (!libname[0])
        return 0;
    do
    {
        l = strlen(libname) - 1;
        if (libname[l] < 0x20)
            libname[l] = 0;
    }
    while (!libname[l]);

    do
    {
        ibuf[0] = 0;
        fgets(ibuf, 256, fil);
    }
    while (!feof(fil) && strncmp(ibuf, "EXPORTS", 7));

    return !feof(fil);
}

//-------------------------------------------------------------------------

static void addrecord(BYTE *buf, int len)
{
    int i;
    BYTE cs = 0;
    curmod->data = ReallocateMemory(curmod->data, curmod->len + len + 1);
    curmod->len += len + 1;
    for (i = 0; i < len; i++)
        cs -= curmod->data[curmod->offset++] =  *buf++;
    curmod->data[curmod->offset++] = cs;
}

//-------------------------------------------------------------------------

void modstart(char *name)
{
    BYTE buf[256];
    int l;
    buf[0] = 0x80;
    buf[1] = 2+(l = strlen(name));
    buf[2] = 0;
    buf[3] = l;
    memcpy(buf + 4, name, l);
    addrecord(buf, 4+l);
}

//-------------------------------------------------------------------------

void modend(void)
{
    BYTE buf[10];
    buf[0] = 0x8a;
    buf[1] = 2;
    buf[2] = 0;
    buf[3] = 0;
    addrecord(buf, 4);
}

//-------------------------------------------------------------------------

void InsertExportRec(char *libname, char *name, int ord)
{
    BYTE buf[256];
    PUBLIC *x = AllocateMemory(sizeof(PUBLIC)),  *q;
    IMP_STRUC *i = AllocateMemory(sizeof(IMP_STRUC));
    int l = 0, l1 = strlen(libname);
    buf[0] = 0x88;
    buf[2] = 0;
    buf[3] = 0; /* type */
    buf[4] = 0xa0; /* class */
    buf[5] = 0x1; /* subtype */
    if (import_list == 0)
        import_list = i;
    else
        (*last_import) = i;
    last_import = &i->link;
    l = strlen(name);
    buf[7] = l;
    memcpy(buf + 8, name, l);
    strcpy(i->name, name);
    buf[8+l] = l1;
    memcpy(buf + 8+l + 1, libname, l1);
    strcpy(i->libname, libname);
    sprintf(i->expname, "@%d", ord);
    buf[1] = 9+l + l1;
    buf[6] = 1; /* Ordinal */
    buf[9+l + l1] = ord &0xff;
    buf[10+l + l1] = ord >> 8;
    addrecord(buf, 11+l + l1);
    x->name = AllocateMemory(l + 1);
    strcpy(x->name, name);
    x->mod = curmod;
    x->link = 0;
    if ((q = AddHash(publichash, x)) != 0)
    {
        Error("Public %s defined in both %s and %s", x->name, q->mod->name, x
            ->mod->name);
        DeallocateMemory(x->name);
        DeallocateMemory(x);
    }

}

//-------------------------------------------------------------------------

static int imports(FILE *f)
{
    while (!feof(f))
    {
        char entry[256];
        char name[256];
        BYTE buf[256];
		char ibuf[256], *p;
        int l = 0, l1 = strlen(libname), ord;
        int l2 = 0;
        buf[0] = 0x88;
        buf[2] = 0;
        buf[3] = 0; /* type */
        buf[4] = 0xa0; /* class */
        buf[5] = 0x1; /* subtype */
        ibuf[0] = 0;
        fgets(ibuf, 256, f);
		p = strchr(ibuf, '\n');
		if (p)
		{
			*p = 9;
		}
        if (ibuf[0])
        {
            char *p = ibuf;
            PUBLIC *x = AllocateMemory(sizeof(PUBLIC)),  *q;
            IMP_STRUC *i = AllocateMemory(sizeof(IMP_STRUC));
            if (import_list == 0)
                import_list = i;
            else
                (*last_import) = i;
            last_import = &i->link;

            while (*p &&  isspace(*p))
                p++;
			if (!*p)
				break;
            while (*p &&  (isalnum(*p) || *p == '_' || *p=='?' || *p=='@'|| *p=='$'))
                name[l++] =  *p++;
            name[l] = 0;
			strcpy(entry, name);
			l2 = l;
            while (*p &&  isspace(*p))
                p++;
			if (*p == '=')
			{
				l=0;
				p++;
	            while (*p &&  isspace(*p))
	                p++;
	            while (*p &&  (isalnum(*p) || *p == '_' || *p=='?' || *p=='@'|| *p=='$'))
	                name[l++] =  *p++;
	            name[l] = 0;					
			}
            while (*p &&  isspace(*p))
                p++;
            buf[7] = l;
            memcpy(buf + 8, name, l);
            strcpy(i->name, name);
            buf[8+l] = l1;
            memcpy(buf + 8+l + 1, libname, l1);
            strcpy(i->libname, libname);
            if (*p == '@')
            {
                 /* by ordinal */
                ord = atoi(p + 1);
                sprintf(i->expname, "@%d", ord);
                buf[1] = 9+l + l1;
                buf[6] = 1; /* Ordinal */
                buf[9+l + l1] = ord &0xff;
                buf[10+l + l1] = ord >> 8;
                addrecord(buf, 11+l + l1);
            }
            else
            {
				if (*p)
				{
					l2 = 0;
		            while (*p &&  (isalnum(*p) || *p == '_' || *p=='?' || *p=='@'|| *p=='$'))
		                entry[l2++] =  *p++;
		            while (*p &&  isspace(*p))
        		        p++;
	    	        entry[l2] = 0;					
				}
                strcpy(i->expname, entry);
                buf[1] = 8+l + l1 + l2;
                buf[6] = 0; /* name */
                buf[9+l + l1] = l2;
                memcpy(buf + 10+l + l1, entry, l2);
                addrecord(buf, 10+l + l1 + l2);
            }

            x->name = AllocateMemory(l + 1);
            strcpy(x->name, name);
            x->mod = curmod;
            x->link = 0;
            if ((q = AddHash(publichash, x)) != 0)
            {
                Error("Public %s defined in both %s and %s", x->name, q->mod
                    ->name, x->mod->name);
                DeallocateMemory(x->name);
                DeallocateMemory(x);
            }
        }
    }
    return 1;
}

//-------------------------------------------------------------------------

uint CreateImports(FILE *infile, char *name)
{
    MODULE *mod,  **m = &modules;
    curfile = infile;
    mod = AllocateMemory(sizeof(MODULE));
    mod->link = 0;
    mod->name = AllocateMemory(strlen(name) + 1);
    strcpy(mod->name, name);
    mod->data = AllocateMemory(1);
    mod->len = 0;
    mod->offset = 0;
    mod->modname = AllocateMemory(strlen(name) + 1);
    strcpy(mod->modname, name);
    curmod = mod;

    if (!header(infile))
        fatal("Invalid .def file: %s", name);

    if (!feof(infile))
    {
        modstart(name);
        if (!imports(infile))
            fatal("Invalid .def file: %s", name);
        modend();
        modnumber++;
        while (*m)
            m =  *m;
        *m = mod;
    }
    return 1;
}

//-------------------------------------------------------------------------

void OutputDefFile(char *filename)
{
    FILE *fil = fopen(filename, "w");
    char name[256];
    strcpy(name, filename);
    if (!fil)
        fatal("Couldn't open output .def file");
    StripExt(name);
    AddExt(name, ".DLL");
    fprintf(fil, "LIBRARY\t%s\n\nEXPORTS\n", name);

    while (import_list)
    {
        fprintf(fil, "\t\t%s\t\t%s\n", import_list->name, import_list->expname);
        import_list = import_list->link;
    }
    fclose(fil);
}
