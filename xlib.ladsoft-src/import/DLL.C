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
#include "cmdline.h"
#include "umem.h"
#include "module.h"
#include "lib.h"
#include "libm.h"
#include "errors.h"
#include "dict.h"
#include "pefile.h"

//#define DEBUG

#ifndef DEBUG
    extern MODULE *modules,  *curmod;
    extern long modnumber;

    extern FILE *curfile;
    extern HASHREC **publichash;
#endif 

void parseExports(FILE *f, char *name, int phys, int rva)
{
    char *xx = strrchr(name, '\\');
    int i;
    struct export_header_struct head;
    if (!xx)
        xx = name;
    fseek(f, phys, SEEK_SET);
    fread(&head, sizeof(head), 1, f);
    for (i = 0; i < head.n_name_ptrs; i++)
    {
        int nameptr;
        short ord;
        char buf[256];
        fseek(f, head.name_rva - rva + phys + i * 4, SEEK_SET);
        fread(&nameptr, 4, 1, f);
        fseek(f, head.ordinal_rva - rva + phys + i * 2, SEEK_SET);
        fread(&ord, 2, 1, f);
        if (nameptr)
            fseek(f, nameptr - rva + phys, SEEK_SET);
        else
            buf[0] = 0;
        fread(buf, 256, 1, f);
        #ifndef DEBUG
            InsertExportRec(name, buf, ord + head.ord_base);
        #else 
            printf("%s @%d\n", buf, ord + head.ord_base);
        #endif 
    } 
}

//-------------------------------------------------------------------------

void doExports(FILE *infile, char *name, int phys, int rva)
{
    #ifndef DEBUG
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
        modstart(name);
    #endif 
    parseExports(infile, name, phys, rva);
    #ifndef DEBUG
        modend();
        modnumber++;
        while (*m)
            m =  *m;
        *m = mod;
    #endif 
}

//-------------------------------------------------------------------------

void LoadDLL(char *f, char *name)
{
    char buf[9], *p;
    struct pe_header_struct pe;
    struct pe_object_struct obj;
    int i;
	p = strrchr(name, '\\');
	if (p && p[-1] != '\\')
		name = p + 1;
    fseek(f, 0, SEEK_SET);
    fread(buf, 2, 1, f);
    if (buf[0] != 'M' || buf[1] != 'Z')
        fatal("Invalid DLL fe");
    fseek(f, 0x3c, SEEK_SET);
    fread(buf, 4, 1, f);
    i = *(int*)(buf);
    fseek(f, i, SEEK_SET);
    fread(&pe, sizeof(struct pe_header_struct), 1, f);
    if (pe.sig != 0x4550)
        fatal("Input DLL is not in PE format");
    for (i = 0; i < pe.num_objects; i++)
    {
        fread(&obj, sizeof(struct pe_object_struct), 1, f);
        if (pe.export_rva >= obj.virtual_addr + obj.virtual_size)
            continue;
        doExports(f, name, obj.raw_ptr + pe.export_rva - obj.virtual_addr,
            pe.export_rva);
        break;
    } 
}

//-------------------------------------------------------------------------

#ifdef DEBUG
    main()
    {
        FILE *fil = fopen("msvcrt.dll", "rb");
        LoadDLL(fil, "a");
        fclose(fil);
    }
#endif
