/* 
XRC Resource Compiler
Copyright 1997, 1998 Free Software Foundation, Inc.
Written by Ian Lance Taylor, Cygnus Support.
Copyright 2006-2011 David Lindauer.

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

You may contact the author of this derivative at:
	mailto::camille@bluegrass.net
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <time.h>
#include "cmdline.h"
#include "umem.h"
#include "xrc.h"

extern char *prm_searchpath;

typedef struct res_hdr
{
    unsigned long data_size;
    unsigned long header_size;
} RES_HDR;

static const char *filename;
static RES_DIR *resource_list;

extern char *program_name;

int compare_type(RES_ID *id1, RES_ID *id2)
{
    if (id1->hasname != id2->hasname)
        return FALSE;
    if (!id1->hasname)
    {
        return id1->v.id == id2->v.id;
    }
    else
    {
        if (id1->v.n.len != id2->v.n.len)
            return FALSE;
        return !memcmp(id1->v.n.name, id2->v.n.name, id1->v.n.len *2+2);
    }
}

/* Add a resource to resource directory */
void AddResource(RES_DIR **list, RES_RES *r, RES_ID *type, RES_ID *id, int
    language, int dupok)
{
    RES_DIR *res,  **oldres = list;
    for (res =  *list; res; res = res->link)
    {
        if (compare_type(type, &res->type) && compare_type(id, &res->id) &&
            language == res->language)
        {
            if (!dupok)
                return ;
        }
        oldres = &res->link;
    }
    res =  *oldres = AllocateMemory(sizeof(RES_DIR));
    res->link = NULL;
    res->type =  *type;
    res->id =  *id;
    res->language = language;
    res->res = r;
}

//-------------------------------------------------------------------------

static void ierror(void)
{
    fatal("Error reading %s", filename);
}

/* Read data from file, abort on failure */
static void data_read(void *data, size_t size, int count, FILE *fil)
{
    if (fread(data, size, count, fil) != count)
        ierror();
}

//-------------------------------------------------------------------------

static void align(FILE *fil)
{
    int add = ftell(fil) % 4;
    if (add)
        add = 4-add;
    if (fseek(fil, add, SEEK_CUR) != 0)
        ierror();
}

/* Read a null terminated UNICODE string */
static CHARACTER *read_string(int *len, FILE *fil)
{
    CHARACTER s[256],  *p = s,  *q;
    CHARACTER c;

    *len =  - 1;

    /* there are hardly any names longer than 256 characters */
    do
    {
        data_read(&c, sizeof(c), 1, fil);
        *p++ = c;
        (*len)++;
    }
    while (c);
    q = AllocateMemory((*len + 1) *sizeof(CHARACTER));
    memcpy(q, s, (*len) *2+2);
    return q;
}

/* read a resource identifier */
static void read_id(RES_ID *id, FILE *fil)
{
    unsigned ord;
    CHARACTER *id_s = NULL;
    int len;

    data_read(&ord, sizeof(ord), 1, fil);
    if (ord == 0xFFFF)
     /* an ordinal id */
    {
        data_read(&ord, sizeof(ord), 1, fil);
        id->hasname = FALSE;
        id->v.id = ord;
    }
    else
    /* hasname id */
    {
        if (fseek(fil,  - sizeof(ord), SEEK_CUR) != 0)
            ierror();
        id_s = read_string(&ord, fil);
        id->hasname = TRUE;
        id->v.n.len = ord;
        id->v.n.name = id_s;
    }
}

/* Read a resource entry, returns 0 when all resources are read */
static int entry_read(FILE *fil)
{
    RES_ID *type;
    RES_ID *name;
    RES_INFO info;
    RES_HDR header;
    long version;
    void *buff;

    RES_RES *r;

    align(fil);

    /* Read header */
    if (fread(&header, sizeof(header), 1, fil) != 1)
        return 0;

    /* read resource type */
    read_id(&type, fil);
    /* read resource id */
    read_id(&name, fil);

    align(fil);

    /* Read additional resource header */
    data_read(&info.version, sizeof(info.version), 1, fil);
    data_read(&info.memflags, sizeof(info.memflags), 1, fil);
    data_read(&info.language, sizeof(info.language), 1, fil);
    data_read(&version, sizeof(version), 1, fil);
    data_read(&info.characteristics, sizeof(info.characteristics), 1, fil);

    align(fil);

    /* Allocate buffer for data */
    buff = AllocateMemory(header.data_size);
    /* Read data */
    data_read(buff, header.data_size, 1, fil);
    /* Convert binary data to resource */
    r = convert_to_internal(&type, buff, header.data_size, 0);
    r->info = info;
    /* Add resource to resource directory */
    AddResource(&resource_list, r, &type, &name, info.language, 0);

    return 1;
}

/* Read resource file */
RES_DIR *resource_file_read(const char *fn)
{
    FILE *in;
    RES_HDR header;
    filename = fn;
    in = SearchPath(filename, prm_searchpath, "rb");
    if (!in)
        fatal("can't open `%s' for input", filename);

    data_read(&header, sizeof(header), 1, in);
    if ((header.data_size != 0) || (header.header_size != 0x20))
        fatal("%s is not a valid resource file", filename);

    /* Subtract size of HeaderSize and DataSize */
    if (fseek(in, header.header_size - 8, SEEK_CUR) != 0)
        fatal("%s is not a valid resource file", filename);

    while (entry_read(in))
        ;

    fclose(in);

    return resource_list;
}

//-------------------------------------------------------------------------

static void oerror(void)
{
    fatal("Error writing %s\n", filename);
}

/* Write data to file, abort on failure */
static void data_write(void *data, size_t size, int count, FILE *fil)
{
    if (fwrite(data, size, count, fil) != count)
        oerror();
}


/* Write a resource id */
static void write_id(RES_ID *id, FILE *fil)
{
    if (id->hasname)
        data_write(id->v.n.name, (id->v.n.len + 1) *sizeof(CHARACTER), 1, fil);
    else
    {
        unsigned short i = 0xFFFF;
        data_write(&i, sizeof(i), 1, fil);
        data_write(&id->v.id, sizeof(short), 1, fil);
    }
}

/* Get number of bytes needed to store an id in binary format */
static unsigned long id_size(RES_ID *id)
{
    if (id->hasname)
        return sizeof(CHARACTER)*(id->v.n.len + 1);
    else
        return sizeof(CHARACTER) *2;
}

/* Write a resource header */
static void header_write(int datasize, RES_ID *type, RES_ID *name, RES_INFO
    *info, FILE *fil)
{
    RES_HDR header;
    header.data_size = datasize;
    header.header_size = 24+id_size(type) + id_size(name);
    header.header_size += 3;
    header.header_size &= ~3;

    align(fil);
    data_write(&header, sizeof(header), 1, fil);
    write_id(type, fil);
    write_id(name, fil);

    align(fil);

    data_write(&info->version, sizeof(info->version), 1, fil);
    data_write(&info->memflags, sizeof(short), 1, fil);
    data_write(&info->language, sizeof(short), 1, fil);
    data_write(&info->version, sizeof(info->version), 1, fil);
    data_write(&info->characteristics, sizeof(info->characteristics), 1, fil);
    align(fil);
}

/* Write a resource in binary resource format */
static void bin_write(RES_DIR *res, FILE *fil)
{
    BINDATA *d,  *d1;
    int datasize = 0;

    d = convert_from_internal(res->res, 0);

    d1 = d;
    while (d1)
    {
        datasize += d1->length;
        d1 = d1->link;
    }
    header_write(datasize, &res->type, &res->id, &res->res->info, fil);

    while (d)
    {
        data_write(d->data, d->length, 1, fil);
        d = d->link;
    }
}

//-------------------------------------------------------------------------

void move_stringtable(RES_DIR **resdir)
{
    RES_DIR *end =  *resdir,  **trip = resdir,  **insert = resdir;
    while (end->link)
    {
        end = end->link;
        if (end->res->type != RST_STRINGTABLE)
            insert = &end->link;
    }
    for (trip; trip != insert;)
    {
        if ((*trip)->res->type == RST_STRINGTABLE)
        {
            RES_DIR *x =  *trip;
            *trip = x->link;
            end = end->link = x;
            x->link = NULL;
        }
        else
            trip = &(*trip)->link;
    }

}

/* Write resource file */
void write_resource(const char *fn, RES_DIR *resdir)
{
    int language;
    RES_DIR *res = resdir;
    static const unsigned char reshead[] = 
    {
        0x00, 0x00, 0x00, 0x00, 0x20, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00,
            0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    };
    long fpos;
    FILE *out;

    filename = fn;

    if (resdir)
        move_stringtable(&res);
    out = fopen(filename, "wb");
    if (out == NULL)
        fatal("can't open `%s' for output", filename);

    /* Write 32 bit resource signature */
    data_write(reshead, sizeof(reshead), 1, out);

    /* write resources */

    language =  - 1;
    while (res)
    {
        bin_write(res, out);
        res = res->link;
    }

    /* end file on DWORD boundary */
    fpos = ftell(out);
    if (fpos % 4)
        data_write(reshead, 4-fpos % 4, 1, out);

    fclose(out);
}
