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
#include "preproc.h"
#include "xrc.h"

extern int basestyle;
extern int laststrlen;
extern char *prm_searchpath;
extern enum e_sym lastst;
extern char lastid[];
extern char laststr[];
extern int lastch;
extern int ival;
extern FILE *inputFile;
extern char *infile;
extern int lineno;
extern int incconst;
int base_language = 0;
char fname[256];
/* We read the directory entries in a cursor or icon file into
instances of this structure.  */

struct icondir
{
    unsigned char width;
    unsigned char height;
    unsigned char colorcount;
    union
    {
        struct 
        {
            unsigned short planes;
            unsigned short bits;
        } icon;
        struct
        {
            unsigned short xhotspot;
            unsigned short yhotspot;
        } cursor;
    }
    u;
    unsigned long bytes;
    unsigned long offset;
};

int rc_lineno;

static RES_DIR *resources;
static int cursors;
static int fonts;
struct fontdir *fontdirs;
RES_INFO fontdirs_resinfo;
static int icons;

static int get_word(FILE *e)
{
    int b1, b2;

    b1 = getc(e);
    b2 = getc(e);
    return ((b2 &0xff) << 8) | (b1 &0xff);

} static unsigned long get_long(FILE *e)
{
    int b1, b2, b3, b4;

    b1 = getc(e);
    b2 = getc(e);
    b3 = getc(e);
    b4 = getc(e);
    return (((((((b4 &0xff) << 8) | (b3 &0xff)) << 8) | (b2 &0xff)) << 8) | (b1
        &0xff));
}

//-------------------------------------------------------------------------

static void get_data(FILE *e, unsigned char *p, long c)
{
    unsigned long got;

    got = fread(p, 1, c, e);
    if (got == c)
        return ;

    fatal("Data file too short");
}

//-------------------------------------------------------------------------

RES_RES *define_standard_resource(RES_DIR *resources, int type, RES_ID *name,
    int language, int dupok)
{
    RES_ID xtype;
    RES_DIR *res;
    RES_RES *r = 0;

    xtype.hasname = 0;
    xtype.v.id = type;

    for (res = resources; res; res = res->link)
    {
        if (compare_type(&xtype, &res->type) && compare_type(name, &res->id) &&
            language == res->language)
        {
            r = res->res;
            if (!dupok)
                return r;
            break;
        }
    }
    if (!r)
    {
        r = AllocateMemory(sizeof(RES_RES));
        r->type = RST_UNINITIALIZED;
    }


    AddResource(&resources, r, &xtype, name, language, dupok);
    return r;
}

//-------------------------------------------------------------------------

void define_accelerator(RES_ID *id, RES_INFO *resinfo, struct accelerator *data)
{
    RES_RES *r = define_standard_resource(&resources, RT_ACCELERATOR, id,
        resinfo->language, 0);
    r->type = RST_ACCELERATOR;
    r->u.acc = data;
    r->info =  *resinfo;
} 

#define BITMAP_SKIP (14)
static long filesize(FILE *e)
{
    long rv;
    fseek(e, 0L, SEEK_END);
    rv = ftell(e);
    fseek(e, 0L, SEEK_SET);
    return rv;
}

//-------------------------------------------------------------------------

void define_bitmap(RES_ID *id, RES_INFO *resinfo, char *filename)
{
    char buf[256];
    FILE *e;
    char *real_filename;
    unsigned char *data;
    int i, size;
    RES_RES *r;

    strcpy(buf, filename);
    e = SearchPath(buf, prm_searchpath, "rb");
    if (!e)
        fatal("File Not Found  %s in line %d", filename, lineno);

    size = filesize(e);

    if (size <= BITMAP_SKIP)
        fatal("%s too small", filename);

    data = AllocateMemory(size - BITMAP_SKIP);

    fseek(e, (long)BITMAP_SKIP, SEEK_SET);

    get_data(e, data, size - BITMAP_SKIP);

    fclose(e);

    r = define_standard_resource(&resources, RT_BITMAP, id, resinfo->language,
        0);

    r->type = RST_BITMAP;
    r->u.data.length = size - BITMAP_SKIP;
    r->u.data.data = data;
    r->info =  *resinfo;
}

//-------------------------------------------------------------------------

void define_cursor(RES_ID *id, RES_INFO *resinfo, char *filename)
{
    char buf[256];
    FILE *e;
    int type, count, i;
    struct icondir *icondirs;
    int first_cursor;
    RES_RES *r;
    struct group_cursor *first,  **pp;

    strcpy(buf, filename);
    e = SearchPath(buf, prm_searchpath, "rb");
    if (!e)
        fatal("File Not Found  %s in line %d", filename, lineno);

    get_word(e);
    type = get_word(e);
    count = get_word(e);
    if (type != 2)
        fatal("cursor file `%s' does not contain cursor data", buf);

    icondirs = AllocateMemory(count *sizeof * icondirs);

    for (i = 0; i < count; i++)
    {
        icondirs[i].width = getc(e);
        icondirs[i].height = getc(e);
        icondirs[i].colorcount = getc(e);
        getc(e);
        icondirs[i].u.cursor.xhotspot = get_word(e);
        icondirs[i].u.cursor.yhotspot = get_word(e);
        icondirs[i].bytes = get_long(e);
        icondirs[i].offset = get_long(e);

        if (!icondirs[i].width)
            icondirs[i].width = 32;
        if (!icondirs[i].height)
            icondirs[i].height = (icondirs[i].bytes - 0x30) / ((32 / 8) *2);

        if (feof(e))
            fatal("Data too short in %s", buf);
    } 

    /* Define each cursor as a unique resource.  */

    first_cursor = cursors;

    for (i = 0; i < count; i++)
    {
        unsigned char *data;
        RES_ID name;
        struct cursor *c;

        if (fseek(e, icondirs[i].offset, SEEK_SET) != 0)
            fatal("file i/o error on %s", buf);

        data = AllocateMemory(icondirs[i].bytes);

        get_data(e, data, icondirs[i].bytes);

        c = AllocateMemory(sizeof *c);
        c->xhotspot = icondirs[i].u.cursor.xhotspot;
        c->yhotspot = icondirs[i].u.cursor.yhotspot;
        c->length = icondirs[i].bytes;
        c->data = data;

        ++cursors;

        name.hasname = 0;
        name.v.id = cursors;

        r = define_standard_resource(&resources, RT_CURSOR, &name, resinfo
            ->language, 0);
        r->type = RST_CURSOR;
        r->u.cursor = c;
        r->info =  *resinfo;
    }

    fclose(e);

    /* Define a cursor group resource.  */

    first = NULL;
    pp = &first;
    for (i = 0; i < count; i++)
    {
        struct group_cursor *cg;

        cg = AllocateMemory(sizeof *cg);
        cg->link = NULL;
        cg->width = icondirs[i].width;
        cg->height = 2 * icondirs[i].height;

        /* FIXME: What should these be set to?  */
        cg->planes = 1;
        cg->bits = 1;

        cg->bytes = icondirs[i].bytes + 4;
        cg->index = first_cursor + i + 1;

        *pp = cg;
        pp = &(*pp)->link;
    }

    free(icondirs);

    r = define_standard_resource(&resources, RT_GROUP_CURSOR, id, resinfo
        ->language, 0);
    r->type = RST_GROUP_CURSOR;
    r->u.group_cursor = first;
    r->info =  *resinfo;
    r->info.memflags |= MF_PURE;
}

//-------------------------------------------------------------------------

void define_dialog(RES_ID *id, RES_INFO *resinfo, struct dialog *dialog)
{
    struct dialog *copy;
    RES_RES *r;

    copy = AllocateMemory(sizeof *copy);
    *copy =  *dialog;

    r = define_standard_resource(&resources, RT_DIALOG, id, resinfo->language,
        0);
    r->type = RST_DIALOG;
    r->u.dialog = copy;
    r->info =  *resinfo;
} int unicode_from_ascii(short **text, char *string, int len)
{
    int i = 0;
    *text = AllocateMemory(len *2+2);
    while (*string)
    {
        (*text)[i++] = (short)*(unsigned char*)string++;
    }
    (*text)[i++] = 0;
    return (i - 1);
}

//-------------------------------------------------------------------------

void string_to_id(RES_ID *val, char *string)
{
    val->hasname = 1;
    val->v.n.len = unicode_from_ascii(&val->v.n.name, string, strlen(string));
}

//-------------------------------------------------------------------------

struct dialog_control *define_control(char *text, long id, long x, long y, long
    width, long height, long class , long style, long exstyle)
{
        struct dialog_control *n = AllocateMemory(sizeof(struct dialog_control))
            ;
        n->link = NULL;
        n->id = id;
        n->style = style;
        n->exstyle = exstyle;
        n->x = x;
        n->y = y;
        n->width = width;
        n->height = height;
        if (class  < 1024)
        {
                n->class.hasname = 0;
                n->class.v.id = class ;
        } 
        else
            string_to_id(&n->class , (void*)class );

        if (text != NULL)
            string_to_id(&n->text, text);
        else
        {
                n->text.hasname = 0;
                n->text.v.id = 0;
        }
        n->data = NULL;
        n->help = 0;

        return n;
}

//-------------------------------------------------------------------------

void define_dlginclude(RES_ID *id, RES_INFO *resinfo, char *filename)
{
    unsigned char *data;
    RES_RES *r;
    long size;

    data = AllocateMemory(size = strlen(filename) + 1);
    strcpy(data, filename);
    r = define_standard_resource(&resources, RT_DLGINCLUDE, id, resinfo
        ->language, 0);

    r->type = RST_DLGINCLUDE;
    r->u.data.length = size;
    r->u.data.data = data;
    r->info =  *resinfo;
}

//-------------------------------------------------------------------------

void define_font(RES_ID *id, RES_INFO *resinfo, char *filename)
{
    FILE *e;
    unsigned char *data;
    RES_RES *r;
    long offset;
    long fontdatalength;
    unsigned char *fontdata;
    struct fontdir *fd;
    const char *device,  *face;
    struct fontdir **pp;
    char buf[256];
    long size;

    strcpy(buf, filename);
    e = SearchPath(buf, prm_searchpath, "rb");
    if (!e)
        fatal("File Not Found  %s in line %d", filename, lineno);

    size = filesize(e);

    data = AllocateMemory(size);

    get_data(e, data, size);

    fclose(e);

    r = define_standard_resource(&resources, RT_FONT, id, resinfo->language, 0);

    r->type = RST_FONT;
    r->u.data.length = size;
    r->u.data.data = data;
    r->info =  *resinfo;

    offset = ((((((data[47] << 8) | data[46]) << 8) | data[45]) << 8) |
        data[44]);
    if (offset > 0 && offset < size)
        device = (char*)data + offset;
    else
        device = "";

    offset = ((((((data[51] << 8) | data[50]) << 8) | data[49]) << 8) |
        data[48]);
    if (offset > 0 && offset < size)
        face = (char*)data + offset;
    else
        face = "";

    ++fonts;

    fontdatalength = 58+strlen(device) + strlen(face);
    fontdata = AllocateMemory(fontdatalength);
    memcpy(fontdata, data, 56);
    strcpy((char*)fontdata + 56, device);
    strcpy((char*)fontdata + 57+strlen(device), face);

    fd = AllocateMemory(sizeof *fd);
    fd->link = NULL;
    fd->index = fonts;
    fd->length = fontdatalength;
    fd->data = fontdata;

    for (pp = &fontdirs;  *pp != NULL; pp = &(*pp)->link)
        ;
    *pp = fd;

    fontdirs_resinfo =  *resinfo;
}

//-------------------------------------------------------------------------


void define_icon(RES_ID *id, RES_INFO *resinfo, char *filename)
{
    FILE *e;
    int type, count, i;
    struct icondir *icondirs;
    int first_icon;
    RES_RES *r;
    struct group_icon *first,  **pp;
    char buf[256];
    strcpy(buf, filename);
    e = SearchPath(buf, prm_searchpath, "rb");
    if (!e)
        fatal("File Not Found  %s in line %d", filename, lineno);

    get_word(e);
    type = get_word(e);
    count = get_word(e);
    if (type != 1)
        fatal("icon file `%s' does not contain icon data", buf);

    /* Read in the icon directory entries.  */

    icondirs = AllocateMemory(count *sizeof * icondirs);

    for (i = 0; i < count; i++)
    {
        icondirs[i].width = getc(e);
        icondirs[i].height = getc(e);
        icondirs[i].colorcount = getc(e);
        getc(e);
        icondirs[i].u.icon.planes = get_word(e);
        icondirs[i].u.icon.bits = get_word(e);
        icondirs[i].bytes = get_long(e);
        icondirs[i].offset = get_long(e);

        if (feof(e))
            fatal("Data too short in %s", buf);
    } 

    /* Define each icon as a unique resource.  */

    first_icon = icons;

    for (i = 0; i < count; i++)
    {
        unsigned char *data;
        struct res_id name;

        if (fseek(e, icondirs[i].offset, SEEK_SET) != 0)
            fatal("file i/o error on %s", buf);

        data = AllocateMemory(icondirs[i].bytes);

        get_data(e, data, icondirs[i].bytes);

        ++icons;

        name.hasname = 0;
        name.v.id = icons;

        r = define_standard_resource(&resources, RT_ICON, &name, resinfo
            ->language, 0);
        r->type = RST_ICON;
        r->u.data.length = icondirs[i].bytes;
        r->u.data.data = data;
        r->info =  *resinfo;
    }

    fclose(e);

    /* Define an icon group resource.  */

    first = NULL;
    pp = &first;
    for (i = 0; i < count; i++)
    {
        struct group_icon *cg;

        /* For some reason, at least in some files the planes and bits
        are zero.  We instead set them from the color.  This is
        copied from rcl.  */

        cg = AllocateMemory(sizeof *cg);
        cg->link = NULL;
        cg->width = icondirs[i].width;
        cg->height = icondirs[i].height;
        cg->colors = icondirs[i].colorcount;

        cg->planes = 1;
        cg->bits = 0;
        while ((1 << cg->bits) < cg->colors)
            ++cg->bits;

        cg->bytes = icondirs[i].bytes;
        cg->index = first_icon + i + 1;

        *pp = cg;
        pp = &(*pp)->link;
    }

    free(icondirs);

    r = define_standard_resource(&resources, RT_GROUP_ICON, id, resinfo
        ->language, 0);
    r->type = RST_GROUP_ICON;
    r->u.group_icon = first;
    r->info =  *resinfo;
    r->info.memflags |= MF_PURE;
}

//-------------------------------------------------------------------------

void define_menu(RES_ID *id, RES_INFO *resinfo, struct menuitem *menuitems)
{
    struct menu *m;
    RES_RES *r;

    m = AllocateMemory(sizeof *m);
    m->items = menuitems;
    m->help = 0;

    r = define_standard_resource(&resources, RT_MENU, id, resinfo->language, 0);
    r->type = RST_MENU;
    r->u.menu = m;
    r->info =  *resinfo;
} 


void define_messagetable(RES_ID *id, RES_INFO *resinfo, char *filename)
{
    FILE *e;
    unsigned char *data;
    RES_RES *r;
    char buf[256];
    long size;

    strcpy(buf, filename);
    e = SearchPath(buf, prm_searchpath, "rb");
    if (!e)
        fatal("File Not Found  %s in line %d", filename, lineno);

    size = filesize(e);

    data = AllocateMemory(size);

    get_data(e, data, size);

    fclose(e);

    r = define_standard_resource(&resources, RT_MESSAGETABLE, id, resinfo
        ->language, 0);

    r->type = RST_MESSAGETABLE;
    r->u.data.length = size;
    r->u.data.data = data;
    r->info =  *resinfo;
}

//-------------------------------------------------------------------------

void define_rcdata(RES_ID *id, RES_INFO *resinfo, struct rcdata_item *data)
{
    RES_RES *r;

    r = define_standard_resource(&resources, RT_RCDATA, id, resinfo->language,
        0);
    r->type = RST_RCDATA;
    r->u.rcdata = data;
    r->info =  *resinfo;
} 

struct rcdata_item *define_rcdata_string(char *string, int len)
{
    struct rcdata_item *ri;
    char *s;

    ri = AllocateMemory(sizeof *ri);
    ri->link = NULL;
    ri->type = RCDATA_STRING;
    ri->u.string.length = len;
    s = AllocateMemory(len);
    memcpy(s, string, len);
    ri->u.string.s = s;

    return ri;
} 

struct rcdata_item *define_rcdata_number(long val, int dword)
{
    struct rcdata_item *ri;

    ri = AllocateMemory(sizeof *ri);
    ri->link = NULL;
    ri->type = dword ? RCDATA_DWORD : RCDATA_WORD;
    ri->u.word = val;

    return ri;
} 

void define_stringtable(RES_INFO *resinfo, long stringid, char *string)
{
    RES_ID id;
    RES_RES *r;

    id.hasname = 0;
    id.v.id = (stringid >> 4) + 1;
    r = define_standard_resource(&resources, RT_STRING, &id, resinfo->language,
        0);

    if (r->type == RST_UNINITIALIZED)
    {
        int i;

        r->type = RST_STRINGTABLE;
        r->u.stringtable = AllocateMemory(sizeof(struct stringtable));
        for (i = 0; i < 16; i++)
        {
            r->u.stringtable->strings[i].length = 0;
            r->u.stringtable->strings[i].string = NULL;
        } 

        r->info =  *resinfo;
    }
    r->u.stringtable->strings[stringid &0xf].length = unicode_from_ascii( &r
        ->u.stringtable->strings[stringid &0xf].string, string, strlen(string));
}

//-------------------------------------------------------------------------

void define_user_data(RES_ID *id, RES_ID *type, RES_INFO *resinfo, struct
    rcdata_item *data)
{
    RES_RES *r;

    r = AllocateMemory(sizeof(*r));
    r->type = RST_USERDATA;
    r->u.userdata = data;
    r->info =  *resinfo;
    AddResource(&resources, r, type, id, resinfo->language, 0);
} 
void define_user_file(RES_ID *id, RES_ID *type, RES_INFO *resinfo, char
    *filename)
{
    FILE *e;
    unsigned char *data;
    RES_RES *r;
    char buf[256];
    long size, xsize;

    strcpy(buf, filename);
    e = SearchPath(buf, prm_searchpath, "rb");
    if (!e)
        fatal("File Not Found  %s in line %d", filename, lineno);

    xsize = size = filesize(e);
    //   if (size & 1)
    //      size++ ;

    data = AllocateMemory(size);

    get_data(e, data, xsize);

    fclose(e);

    r = AllocateMemory(sizeof(*r));
    r->type = RST_USERDATA;
    r->u.userdata = AllocateMemory(sizeof(struct rcdata_item));
    r->u.userdata->link = NULL;
    r->u.userdata->type = RCDATA_BUFFER;
    r->u.userdata->u.buffer.length = size;
    r->u.userdata->u.buffer.data = data;
    r->info =  *resinfo;
    AddResource(&resources, r, type, id, resinfo->language, 0);
}

//-------------------------------------------------------------------------

void define_versioninfo(RES_ID *id, RES_INFO *info, struct fixed_versioninfo
    *fixedverinfo, struct ver_info *verinfo)
{
    RES_RES *r;

    r = define_standard_resource(&resources, RT_VERSION, id, info->language, 0);
    r->type = RST_VERSIONINFO;
    r->u.versioninfo = AllocateMemory(sizeof(struct versioninfo));
    r->u.versioninfo->fixed = fixedverinfo;
    r->u.versioninfo->var = verinfo;
    r->info =  *info;
} 

static void define_fontdirs(void)
{
    RES_RES *r;
    RES_ID id;

    id.hasname = 0;
    id.v.id = 1;

    r = define_standard_resource(&resources, RT_FONTDIR, &id, 0x409, 0);

    r->type = RST_FONTDIR;
    r->u.fontdir = fontdirs;
    r->info = fontdirs_resinfo;
}

//-------------------------------------------------------------------------

void input_resid(RES_ID *id)
{
    int i;
    switch (lastst)
    {
        case ident:
            id->hasname = 1;
            for (i = 0; i < strlen(lastid); i++)
                lastid[i] = toupper(lastid[i]);
            id->v.n.len = unicode_from_ascii(&id->v.n.name, lastid, strlen
                (lastid));
            getsym();
            break;
        default:
			if (is_number())
			{
	            id->hasname = 0;
    	        id->v.id = intexpr();
			}
			else
			{
	            generror(ERR_RESOURCE_ID_EXPECTED, 0, 0);
			}
            break;

    }
}

//-------------------------------------------------------------------------

void input_quoted_resid(RES_ID *id)
{
    if (is_number())
        input_resid(id);
    else if (lastst == sconst)
    {
        id->hasname = 1;
        id->v.n.len = unicode_from_ascii(&id->v.n.name, laststr, laststrlen);
        getsym();
    }
    else
        generror(ERR_INVALIDCLASS, 0, 0);
}

//-------------------------------------------------------------------------

void input_secondary_characteristics(RES_INFO *info)
{
    int done = FALSE;
    info->language = base_language;
    while (!done)
    {
        while (lastst == eol)
            getsym();
        switch (lastst)
        {
            case kw_language:
                getsym();
                info->language = intexpr();
                skip_comma();
                intexpr();
                break;
            case kw_version:
                getsym();
                info->version = intexpr();
                break;
            case kw_characteristics:
                getsym();
                info->characteristics = intexpr();
                break;
            default:
                done = TRUE;
                break;
        }
        if (!done)
        {
            need_eol();
        }
    }
}

//-------------------------------------------------------------------------

void input_memflags(RES_INFO *info)
{
    int done = FALSE;
    while (!done)
    {
        switch (lastst)
        {
            case kw_discardable:
                info->memflags |= MF_DISCARDABLE;
                break;
            case kw_pure:
                info->memflags |= MF_PURE;
                break;
            case kw_preload:
                info->memflags |= MF_PRELOAD;
                break;
            case kw_moveable:
                info->memflags |= MF_MOVEABLE;
                break;
            case kw_nondiscardable:
                info->memflags &= ~MF_DISCARDABLE;
                break;
            case kw_impure:
                info->memflags &= ~MF_PURE;
                break;
            default:
                done = TRUE;
                break;
        }
        if (!done)
        {
            getsym();
        }
        skip_comma();
    }
}

//-------------------------------------------------------------------------

void getfilename(void)
{
    int done = FALSE;
    if (lastst == sconst)
    {
        strcpy(fname, laststr);
        return ;
    }
    fname[0] = 0;
    while (!done)
    {
        switch (lastst)
        {
            case divide:
            case backslash:
                strcat(fname, "\\");
                break;
            case dot:
                strcat(fname, ".");
                break;
            case ident:
                strcat(fname, lastid);
                break;
            default:
                done = TRUE;
                break;
        }
        if (!done)
            getsym();
    }
}

//-------------------------------------------------------------------------

int input_string(CHARACTER **string)
{
    int rv;
    if (lastst == sconst)
    {
        rv = unicode_from_ascii(string, laststr, laststrlen);
        getsym();
    }
    else
        generror(ERR_STRING_EXPECTED, 0, 0);
    return rv;
}

//-------------------------------------------------------------------------

void parse_accelerator(RES_ID *id, RES_INFO *info)
{
    int mode;
    struct accelerator *data,  **p = &data,  *x;
    getsym();
    input_memflags(info);
    need_eol();
    info->memflags |= MF_PURE;
    info->memflags &= ~MF_DISCARDABLE;
    input_secondary_characteristics(info);
    need_begin();
    //  info->memflags = MF_PURE | MF_MOVEABLE ;

    while (TRUE)
    {
        int done;
        x =  *p = AllocateMemory(sizeof(*data));
        x->link = NULL;
        x->flags = 0;
        if (lastst == sconst)
        {
            if (laststr[0] == '^')
                x->key = laststr[1] &0x1f;
            else
                x->key = laststr[0];
            mode = 1;
            getsym();
        } 
        else if (is_number())
        {
            x->key = intexpr();
            mode = 0;
        }
        else
            break;
        skip_comma();
        x->id = intexpr();
        skip_comma();
        done = FALSE;
        while (!done)
        {
            switch (lastst)
            {
                case kw_ascii:
                    x->flags &= ~ACC_VIRTKEY;
                    break;
                case kw_virtkey:
                    x->flags |= ACC_VIRTKEY;
                    break;
                case kw_noinvert:
                    x->flags |= ACC_NOINVERT;
                    break;
                case kw_shift:
                    x->flags |= ACC_SHIFT;
                    break;
                case kw_control:
                    x->flags |= ACC_CONTROL;
                    break;
                case kw_alt:
                    x->flags |= ACC_ALT;
                    break;
                default:
                    done = TRUE;
                    break;
            }
            if (!done)
            {
                getsym();
                skip_comma();
            }
        }
        need_eol();
        p = &(*p)->link;

    }
    *p = 0;
    need_end();
    define_accelerator(id, info, data);
}

//-------------------------------------------------------------------------

void parse_bitmap(RES_ID *id, RES_INFO *info)
{
    incconst = TRUE;
    info->memflags |= MF_PURE;
    info->memflags &= ~MF_DISCARDABLE;
    getsym();
    input_memflags(info);
    getfilename();
    define_bitmap(id, info, fname);
    incconst = FALSE;
    getsym();
    need_eol();
}

//-------------------------------------------------------------------------

void parse_cursor(RES_ID *id, RES_INFO *info)
{
    incconst = TRUE;
    getsym();

    input_memflags(info);
    getfilename();
    define_cursor(id, info, fname);
    incconst = FALSE;
    getsym();
    need_eol();
}

//-------------------------------------------------------------------------

void dlg_error(extended)
{
    if (!extended)
        generror(ERR_NOT_DIALOGEX, 0, 0);
}

//-------------------------------------------------------------------------

void input_dialog_settings(struct dialog *dlg, RES_INFO *info, int extended)
{
    int done = FALSE;
    info->language = base_language;
    while (!done)
    {
        while (lastst == eol)
            getsym();
        switch (lastst)
        {
            case kw_language:
                getsym();
                info->language = intexpr();
                skip_comma();
                intexpr();
                break;
            case kw_version:
                getsym();
                info->version = intexpr();
                break;
            case kw_characteristics:
                getsym();
                info->characteristics = intexpr();
                break;
            case kw_style:
                getsym();
                dlg->style |= intexpr();
                break;
            case kw_exstyle:
                getsym();
                dlg->exstyle |= intexpr();

                break;
            case kw_menu:
                getsym();
                input_resid(&dlg->menu);
                break;
            case kw_font:
                dlg->style |= DS_SETFONT;
                getsym();
                dlg->pointsize = intexpr();
                skip_comma();
                input_string(&dlg->font);
                if (extended && lastst == comma)
                {
                    skip_comma();
                    dlg->ex->weight = intexpr();
                    if (lastst == comma)
                    {
                        skip_comma();
                        dlg->ex->italic = intexpr();
                        if (lastst == comma)
                        {
                           skip_comma();
                           dlg->ex->charset = intexpr();
                        }
                    }
                }
                break;
            case kw_caption:
                //                dlg->style |= WS_DLGFRAME ;
                getsym();
                input_string(&dlg->caption);
                break;
            case kw_class:
                getsym();
                input_quoted_resid(&dlg->class );
                break;
            case kw_help:
                getsym();
                dlg_error(extended);
                dlg->ex->help = intexpr();
                break;
            case kw_weight:
                getsym();
                dlg_error(extended);
                dlg->ex->weight = intexpr();
                break;
            case kw_italic:
                getsym();
                dlg_error(extended);
                dlg->ex->italic = intexpr();
                break;
            default:
                done = TRUE;
                break;
    }
    if (!done)
    {
            need_eol();
    }
}

//-------------------------------------------------------------------------

}

//-------------------------------------------------------------------------

void parse_control_extended(struct dialog_control *c, int extended)
{
        //	if (extended) {
        if (lastst != eol)
        {
                c->exstyle = intexpr();
                skip_comma();
        } if (lastst != eol)
        {
                c->help = intexpr();
                skip_comma();
        }
        //	}
}

//-------------------------------------------------------------------------

void set_class(struct dialog_control *c, int st)
{
        int v = 0;
        int found = TRUE;
        switch (st)
        {
                case kw_auto3state:
                case kw_autocheckbox:
                case kw_autoradiobutton:
                case kw_checkbox:
                case kw_pushbutton:
                case kw_radiobutton:
                case kw_defpushbutton:
                case kw_state3:
                    v = CTL_BUTTON;
                    break;
                case kw_combobox:
                    v = CTL_COMBOBOX;
                    break;
                case kw_ctext:
                case kw_ltext:
                case kw_rtext:
                case kw_icon:
                    v = CTL_STATIC;
                    break;
                case kw_edittext:
                    v = CTL_EDIT;
                    break;
                case kw_groupbox:
                    v = CTL_BUTTON;
                    break;
                case kw_listbox:
                    v = CTL_LISTBOX;
                    break;
                case kw_scrollbar:
                    v = CTL_SCROLLBAR;
                    break;
                case sconst:
                    if (!stricmp(laststr, "button"))
                        v = CTL_BUTTON;
                    else if (!stricmp(laststr, "edit"))
                        v = CTL_EDIT;
                    else if (!stricmp(laststr, "static"))
                        v = CTL_STATIC;
                    else if (!stricmp(laststr, "listbox"))
                        v = CTL_LISTBOX;
                    else if (!stricmp(laststr, "scrollbar"))
                        v = CTL_SCROLLBAR;
                    else if (!stricmp(laststr, "combobox"))
                        v = CTL_COMBOBOX;
                    else
                    {
                            c->class.hasname = TRUE;
                            c->class.v.n.len = unicode_from_ascii(&c
                                ->class.v.n.name, laststr, laststrlen);
                            return ;
                    }
        }
        if (!v)
            generror(ERR_UNKNOWN_DIALOG_CONTROL_CLASS, 0, 0);
        else
        {
                c->class.hasname = 0;
                c->class.v.id = v;
        }
}

//-------------------------------------------------------------------------

void parse_control_generic(struct dialog_control *c, int extended)
{
        getsym();
        input_quoted_resid(&c->text);
        skip_comma();
        c->id = intexpr();
        skip_comma();
        set_class(c, lastst);
        getsym();
        skip_comma();
        basestyle = c->style;
        c->style = intexpr();
        skip_comma();
        c->x = intexpr();
        skip_comma();
        c->y = intexpr();
        skip_comma();
        c->width = intexpr();
        skip_comma();
        c->height = intexpr();
        skip_comma();
        parse_control_extended(c, extended);
} void parse_control_standard(struct dialog_control *c, int class , int style,
    int extended, int text)
{
        int st = lastst;
        set_class(c, st = lastst);
        getsym();
        if (text)
        {
                input_quoted_resid(&c->text);
                skip_comma();
        } c->id = intexpr();
        skip_comma();
        c->x = intexpr();
        skip_comma();
        c->y = intexpr();
        skip_comma();
        c->width = intexpr();
        skip_comma();
        c->height = intexpr();
        skip_comma();
        if (lastst != eol)
        {
                basestyle = style | c->style;
                c->style = intexpr();
                skip_comma();
        }
        else
            c->style |= style;
        parse_control_extended(c, extended);
        switch (st)
        {
                case kw_scrollbar:
                    if (c->style &SBS_VERT)
                        c->style &= ~SBS_HORZ;
                    break;
        }
}

//-------------------------------------------------------------------------

int parse_control(struct dialog_control * * * ctl, int extended)
{
        int rv = FALSE;
        struct dialog_control *c = AllocateMemory(sizeof(struct dialog_control))
            ;
        memset(c, 0, sizeof(struct dialog_control));
        c->style = WS_CHILD | WS_VISIBLE;
        switch (lastst)
        {
                case kw_auto3state:
                    parse_control_standard(c, CTL_BUTTON, BS_AUTO3STATE |
                        WS_TABSTOP, extended, 1);
                    break;
                case kw_autocheckbox:
                    parse_control_standard(c, CTL_BUTTON, BS_AUTOCHECKBOX |
                        WS_TABSTOP, extended, 1);
                    break;
                case kw_autoradiobutton:
                    parse_control_standard(c, CTL_BUTTON, BS_AUTORADIOBUTTON,
                        extended, 1);
                    break;
                case kw_checkbox:
                    parse_control_standard(c, CTL_BUTTON, BS_CHECKBOX |
                        WS_TABSTOP, extended, 1);
                    break;
                case kw_combobox:
                    parse_control_standard(c, CTL_COMBOBOX, WS_TABSTOP,
                        extended, 0);
                    if (!(c->style &3))
                        c->style |= CBS_SIMPLE;
                    break;
                case kw_ctext:
                    parse_control_standard(c, CTL_STATIC, SS_CENTER | WS_GROUP,
                        extended, 1);
                    break;
                case kw_defpushbutton:
                    parse_control_standard(c, CTL_BUTTON, BS_DEFPUSHBUTTON |
                        WS_TABSTOP, extended, 1);
                    break;
                case kw_edittext:
                    parse_control_standard(c, CTL_EDIT, ES_LEFT | WS_BORDER |
                        WS_TABSTOP, extended, 0);
                    break;
                case kw_groupbox:
                    parse_control_standard(c, CTL_STATIC, BS_GROUPBOX, extended,
                        1);
                    break;
                case kw_icon:
                    parse_control_standard(c, CTL_STATIC, SS_ICON, extended, 1);
                    break;
                case kw_listbox:
                    parse_control_standard(c, CTL_LISTBOX, LBS_NOTIFY |
                        WS_BORDER | WS_VSCROLL | WS_TABSTOP, extended, 0);
                    break;
                case kw_ltext:
                    parse_control_standard(c, CTL_STATIC, SS_LEFT | WS_GROUP,
                        extended, 1);
                    break;
                case kw_pushbutton:
                    parse_control_standard(c, CTL_BUTTON, BS_PUSHBUTTON |
                        WS_TABSTOP, extended, 1);
                    break;
                case kw_radiobutton:
                    parse_control_standard(c, CTL_BUTTON, BS_RADIOBUTTON,
                        extended, 1);
                    break;
                case kw_rtext:
                    parse_control_standard(c, CTL_STATIC, SS_RIGHT | WS_GROUP,
                        extended, 1);
                    break;
                case kw_scrollbar:
                    parse_control_standard(c, CTL_SCROLLBAR, SBS_HORZ, extended,
                        0);
                    break;
                case kw_state3:
                    parse_control_standard(c, CTL_BUTTON, BS_3STATE |
                        WS_TABSTOP, extended, 1);
                    break;
                case kw_control:
                    parse_control_generic(c, extended);
                    break;
                default:
                    rv = TRUE;
                    break;
        }
        if (!rv)
        {
                need_eol();
                **ctl = c;
                *ctl = &c->link;
        }
        return !rv;
}

//-------------------------------------------------------------------------

void parse_dialog(RES_ID *id, RES_INFO *info, int extended)
{
        struct dialog *dialog = AllocateMemory(sizeof(struct dialog));
        struct dialog_control **p = &dialog->controls;
        memset(dialog, 0, sizeof(*dialog));
        if (extended)
        {
                dialog->ex = AllocateMemory(sizeof(*dialog->ex));
                memset(dialog->ex, 0, sizeof(*dialog->ex));
                dialog->ex->italic = 256;
        } info->memflags |= MF_PURE;
        dialog->style = 0; // WS_POPUPWINDOW ;
        getsym();
        input_memflags(info);
        dialog->x = intexpr();
        skip_comma();
        dialog->y = intexpr();
        skip_comma();
        dialog->width = intexpr();
        skip_comma();
        dialog->height = intexpr();
        skip_comma();
        need_eol();
        input_dialog_settings(dialog, info, extended);
        need_begin();
        while (parse_control(&p, extended))
            ;
        need_end();
        define_dialog(id, info, dialog);

}

//-------------------------------------------------------------------------

void parse_dlginclude(RES_ID *id, RES_INFO *info)
{
        getsym();
        info->memflags |= MF_PURE;
        input_memflags(info);
        getfilename();
        define_dlginclude(id, info, fname);
        getsym();
        need_eol();
}

//-------------------------------------------------------------------------

void parse_font(RES_ID *id, RES_INFO *info)
{
        incconst = TRUE;
        getsym();
        input_memflags(info);
        getfilename();
        define_font(id, info, fname);
        incconst = FALSE;
        getsym();
        need_eol();
}

//-------------------------------------------------------------------------

void parse_icon(RES_ID *id, RES_INFO *info)
{
        incconst = TRUE;
        getsym();
        input_memflags(info);
        getfilename();
        define_icon(id, info, fname);
        incconst = FALSE;
        getsym();
        need_eol();
}

//-------------------------------------------------------------------------

void get_menu_flags(struct menuitem *m)
{
        int done = FALSE;
        while (!done)
        {
                switch (lastst)
                {
                        case kw_grayed:
                            m->flags |= MI_GRAYED;
                            break;
                        case kw_inactive:
                            m->flags |= MI_INACTIVE;
                            break;
                        case kw_checked:
                            m->flags |= MI_CHECKED;
                            break;
                        case kw_menubarbreak:
                            m->flags |= MI_MENUBARBREAK;
                            break;
                        case kw_menubreak:
                            m->flags |= MI_MENUBREAK;
                            break;
                        case kw_help:
                            m->flags |= MI_HELP;
                            break;
                        case kw_separator:
                            //            m->flags = MI_SEPARATOR ;
                            break;
                        default:
                            done = TRUE;
                            break;
                }
                if (!done)
                {
                        getsym();
								if (lastst != eol)
								{
	                                m->state = intexpr();
    	                            skip_comma();
								}
                        skip_comma();
                }
        }
}

//-------------------------------------------------------------------------

void input_menulist(struct menuitem * * * i, int extended)
{
        struct menuitem **p;
        int done = FALSE;
        need_begin();
        while (!done)
        {
                struct menuitem *m = AllocateMemory(sizeof(struct menuitem));
                memset(m, 0, sizeof(*m));
                switch (lastst)
                {
                        case kw_menuitem:
                            getsym();
                            if (lastst == sconst)
                            {
                                    unicode_from_ascii(&m->text, laststr,
                                        laststrlen);
                                    getsym();
                                    skip_comma();
                            }
                            if (is_number())
                                m->id = intexpr();
                            skip_comma();
                            if (!extended)
                                get_menu_flags(m);
                            else
                            {
                                m->type = intexpr();
                                skip_comma();
								if (lastst != eol)
								{
	                                m->state = intexpr();
    	                            skip_comma();
								}
                                if (lastst != eol)
                                {
                                    m->help = intexpr();
                                    skip_comma();
                                }
                            }
                            break;
                        case kw_popup:
                            getsym();
                            if (lastst == sconst)
                            {
                                unicode_from_ascii(&m->text, laststr,
                                    laststrlen);
                                getsym();
                                skip_comma();
                            }
                            if (is_number())
							{
                                m->id = intexpr();
                                skip_comma();
							}
                            if (!extended)
                                get_menu_flags(m);
                            else
                            {
								if (lastst != eol)
								{
	                                m->type = intexpr();
    	                            skip_comma();
								}
								if (lastst != eol)
								{
	                                m->state = intexpr();
    	                            skip_comma();
								}
                                if (lastst != eol)
                                {
                                    m->help = intexpr();
                                    skip_comma();
                                }
                            }
                            p = &m->popup;
                            need_eol();
                            input_menulist(&p, extended);
                            break;
                        default:
                            done = TRUE;
                            break;
                    }
                    if (!done)
                    {
                        need_eol();
                        **i = m;
                        *i = &(m->link);
                    }
                }
                if (lastst != end && lastst != kw_end)
                    generror(ERR_END_EXPECTED, 0, 0);
                getsym();

            }
            void parse_menu(RES_ID *id, RES_INFO *info, int extended)
            {
                struct menuitem *s = 0,  **p = &s;
                getsym();
                info->memflags |= MF_PURE;
                input_memflags(info);
                need_eol();
                input_secondary_characteristics(info);
                input_menulist(&p, extended);

                need_eol();
                define_menu(id, info, s);

            }
            void parse_rc(RES_ID *id, RES_INFO *info)
            {
                int done = FALSE;
                struct rcdata_item *r = 0,  **p = &r;
                getsym();
                info->memflags |= MF_PURE;
                input_memflags(info);
				if (lastst == sconst)
				{
					char buf[260];
					FILE *e;
					int size;
				    RES_ID xtype;
				
				    xtype.hasname = 0;
				    xtype.v.id = RT_RCDATA;
					getfilename();
					define_user_file(id, &xtype, info, fname);
					getsym();
					need_eol();
					return;
				}
                need_eol();
                need_begin();
                while (!done)
                {
                    switch (lastst)
                    {
                        case sconst:
                            *p = define_rcdata_string(laststr, laststrlen);
                            getsym();
                            break;
                        case iconst:
                        case iuconst:
                        case cconst:
                            *p = define_rcdata_number(ival, FALSE);
                            getsym();
                            break;
                        case lconst:
                        case luconst:
                            *p = define_rcdata_number(ival, TRUE);
                            getsym();
                            break;
                        case eol:
                            getsym();
                            continue;
                        default:
                            done = TRUE;
                    }
                    if (!done)
                    {
                        skip_comma();
                        p = &(*p)->link;
                    }
                }
                need_end();
                define_rcdata(id, info, r);
            }
            void parse_messagetable(RES_ID *id, RES_INFO *info)
            {
                getsym();
                info->memflags |= MF_PURE;
                input_memflags(info);
                getfilename();
                define_messagetable(id, info, fname);
                getsym();
                need_eol();
            }
            void parse_stringtable(RES_INFO *info)
            {
                getsym();
                info->memflags |= MF_PURE;
                input_memflags(info);
                need_eol();
                need_begin();
                while (is_number())
                {
                    int val = intexpr();
                    skip_comma();
                    if (lastst == eol)
                        getsym();
                    if (lastst != sconst)
                        generror(ERR_STRING_EXPECTED, 0, 0);
                    define_stringtable(info, val, laststr);
                    getsym();
                    need_eol();
                }
                need_end();
            }
            void parse_versioninfo(RES_ID *id, RES_INFO *info)
            {
                int val, val1;
                int done = 0, did1 = 0;
                struct fixed_versioninfo *fixedverinfo = AllocateMemory(sizeof
                    (struct fixed_versioninfo));
                struct ver_info *verinfo = 0,  **verinfop = &verinfo;
                getsym();
                info->memflags = MF_PURE | MF_MOVEABLE;
                while (!done)
                {
                    val = val1 = 0;
                    switch (lastst)
                    {
                        case kw_fileversion:
                            getsym();
                            val = intexpr() << 16;
                            if (lastst == comma)
                            {
                                getsym();
                                val |= intexpr() &0xffff;
                                if (lastst == comma)
                                {
                                    getsym();
                                    val1 = intexpr() << 16;
                                    if (lastst == comma)
                                    {
                                        getsym();
                                        val1 |= intexpr() &0xffff;
                                    }
                                }
                            }
                            fixedverinfo->file_version_ms = val;
                            fixedverinfo->file_version_ls = val1;
                            break;
                        case kw_productversion:
                            getsym();
                            val = intexpr() << 16;
                            if (lastst == comma)
                            {
                                getsym();
                                val |= intexpr() &0xffff;
                                if (lastst == comma)
                                {
                                    getsym();
                                    val1 = intexpr() << 16;
                                    if (lastst == comma)
                                    {
                                        getsym();
                                        val1 |= intexpr() &0xffff;
                                    }
                                }
                            }
                            fixedverinfo->product_version_ms = val;
                            fixedverinfo->product_version_ls = val1;
                            break;
                        case kw_fileflagmask:
                            getsym();
                            fixedverinfo->file_flags_mask = intexpr();
                            break;
                        case kw_fileflags:
                            getsym();
                            val = intexpr();
                            while (lastst == eol)
                            {
                                getsym();
                                if (lastst == or)
                                {
                                    getsym();
                                    val |= intexpr();
                                }
                            }
                            fixedverinfo->file_flags = val;
                            break;
                        case kw_fileos:
                            getsym();
                            fixedverinfo->file_os = intexpr();
                            break;
                        case kw_filetype:
                            getsym();
                            fixedverinfo->file_type = intexpr();
                            break;
                        case kw_filesubtype:
                            getsym();
                            fixedverinfo->file_subtype = intexpr();
                            break;
                        case eol:
                            getsym();
                            break;
                            //         case kw_filedate:
                            //            break ;
                        default:
                            if (!did1)
                                generror(ERR_FIXEDDATAEXPECTED, 0, 0);
                            done = 1;
                            break;
                    }
                    did1 = 1;
                }
                need_begin();
                if (lastst != kw_block)
                    generror(ERR_BLOCK_EXPECTED, 0, 0);
                while (lastst == kw_block)
                {
                    getsym();
                    if (lastst != sconst)
                        generror(ERR_INVALID_VERSION_INFO_TYPE, 0, 0);
                    if (!strcmp(laststr, "StringFileInfo"))
                    {
                        struct ver_stringinfo **current;
                        getsym();
                        need_eol();
                        need_begin();
                        if (lastst != kw_block)
                            generror(ERR_BLOCK_EXPECTED, 0, 0);
                        getsym();
                        *verinfop = AllocateMemory(sizeof(struct ver_info));
                        (*verinfop)->type = VERINFO_STRING;
                        input_string(&(*verinfop)->u.string.language);
                        need_eol();
                        current = &(*verinfop)->u.string.strings;
                        need_begin();
                        while (lastst == kw_value)
                        {
                            getsym();
                            if (lastst != sconst)
                                generror(ERR_STRING_EXPECTED, 0, 0);
                            *current = AllocateMemory(sizeof(struct
                                ver_stringinfo));
                            input_string(&(*current)->key);
                            if (lastst != comma)
                                generror(ERR_NEEDCHAR, ',', 0);
                            getsym();
                            (*current)->length = laststrlen + 1;
                            if (lastst != sconst && lastst != lsconst)
                                generror(ERR_STRING_EXPECTED, 0, 0);
                            (*current)->value = AllocateMemory(laststrlen *2+2);
                            if (lastst == sconst)
                                for (val = 0; val < laststrlen; val++)
                                    (*current)->value[val] = laststr[val];
                            else
                                for (val = 0; val < laststrlen; val++)
                                    (*current)->value[val] = ((short*)laststr)[val];
                            (*current)->value[val] = 0;
                            getsym();
                            need_eol();
                            current = &(*current)->link;
                        } 
						need_end();
                    }
                    else if (!strcmp(laststr, "VarFileInfo"))
                    {
                        struct ver_varinfo **current;
                        getsym();
                        need_eol();
                        need_begin();
                        *verinfop = AllocateMemory(sizeof(struct ver_info));
                        (*verinfop)->type = VERINFO_VAR;
                        current = &((*verinfop)->u.var.var);
                        while (lastst == kw_value)
                        {
                            struct ver_varlangchar **cur1;
                            *current = AllocateMemory(sizeof(struct ver_varinfo)
                                );
                            getsym();
                            input_string(&(*current)->key);
                            if (lastst != comma)
                                generror(ERR_NEEDCHAR, ',', 0);
                            getsym();
                            cur1 = &(*current)->intident;
                            while (TRUE)
                            {
                                *cur1 = AllocateMemory(sizeof(struct
                                    ver_varlangchar));
                                (*cur1)->language = intexpr();
                                if (lastst == comma)
                                {
                                    getsym();
                                    (*cur1)->charset = intexpr();
                                } if (lastst != comma)
                                    break;
                                getsym();
                                cur1 = &(*cur1)->link;
                            }
                            need_eol();
                            current = &(*current)->link;
                        }
                    }
                    else
                        generror(ERR_INVALID_VERSION_INFO_TYPE, 0, 0);


                    verinfop = &(*verinfop)->link;
                    need_end();

                }
                need_end();
                define_versioninfo(id, info, fixedverinfo, verinfo);
            }
            void parse_special(RES_ID *id, RES_INFO *info)
            {
                RES_ID type;
                incconst = TRUE;
                info->memflags |= MF_PURE;
                input_resid(&type);
                input_memflags(info);
                getfilename();
                define_user_file(id, &type, info, fname);
                incconst = FALSE;
                getsym();
                need_eol();
            }
            void parse(char *name)
            {
                RES_ID id;
                RES_INFO info;
                infile = name;
                inputFile = fopen(name, "r");
                if (!inputFile)
                    fatal("file %s not found", name);

                getch();
                getsym();
                while (lastst != eof)
                {
                    while (lastst == eol)
                        getsym();
                    memset(&id, 0, sizeof(id));
					if (lastst != kw_stringtable)
	                    if (lastst == ident || is_number())
	                        input_resid(&id);
	                    else
	                    {
	                        int st = lastst;
	                        getsym();
	                        if (lastst >= kw_accelerator && lastst < eol)
	                        {
	                            if (st >= kw_accelerator && st < eol)
	                            {
	                                char *s = namefromkw(st);
	                                id.hasname = 1;
	                                id.v.n.len = unicode_from_ascii(&id.v.n.name, s,
	                                    strlen(s));
	                            }
	                            else
	                                backup(st);
	                        }
	                        else
	                            backup(st);
	                    }
                    memset(&info, 0, sizeof(info));
                    info.memflags |= MF_MOVEABLE | MF_DISCARDABLE;
                    info.language = base_language;
                    switch (lastst)
                    {
                        case kw_accelerator:
                            parse_accelerator(&id, &info);
                            break;
                        case kw_bitmap:
                            parse_bitmap(&id, &info);
                            break;
                        case kw_cursor:
                            parse_cursor(&id, &info);
                            break;
                        case kw_dialog:
                            parse_dialog(&id, &info, FALSE);
                            break;
                        case kw_dialogex:
                            parse_dialog(&id, &info, TRUE);
                            break;
                        case kw_dlginclude:
                            parse_dlginclude(&id, &info);
                            break;
                        case kw_font:
                            parse_font(&id, &info);
                            break;
                        case kw_icon:
                            parse_icon(&id, &info);
                            break;
                        case kw_menu:
                            parse_menu(&id, &info, FALSE);
                            break;
                        case kw_menuex:
                            parse_menu(&id, &info, TRUE);
                            break;
                        case kw_rcdata:
                            parse_rc(&id, &info);
                            break;
                        case kw_versioninfo:
                            parse_versioninfo(&id, &info);
                            break;
                        case kw_messagetable:
                            parse_messagetable(&id, &info);
                            break;
                        case kw_stringtable:
                            parse_stringtable(&info);
                            break;
                        case kw_language:
                            getsym();
                            base_language = intexpr();
                            skip_comma();
                            base_language |= (intexpr() << 10);
                            need_eol();
                            break;
                        case kw_rcinclude:
                            doinclude(TRUE);
                            break;
						case iconst:
						case iuconst:
						case lconst:
						case luconst:
                        case ident:
                            parse_special(&id, &info);
                            break;
                        default:
                            generror(ERR_UNKNOWN_RESOURCE_TYPE, 0, 0);
                            break;
                    }

                }
            }
            RES_DIR *parse_rc_file(char *filename)
            {
                char *cmd;

                parse(filename);

                if (fontdirs != NULL)
                    define_fontdirs();
                return resources;
            }
