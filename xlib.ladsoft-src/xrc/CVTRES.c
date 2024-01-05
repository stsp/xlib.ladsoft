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

static int get_16(int be, unsigned char *s)
{
    if (be)
    {
        return (*s << 8) + *(s + 1);
    }
    else
    {
        return *(short*)s;
    }
}

//-------------------------------------------------------------------------

static long get_32(int be, unsigned char *s)
{
    if (be)
    {
        return (*s << 24) + (*(s + 1) << 16) + (*(s + 2) << 8) + *(s + 3);
    }
    else
    {
        return *(long*)s;
    }
}

//-------------------------------------------------------------------------

static void checksize(int left, int size)
{
    if (left < size)
        fatal("malformed resource in RES file, exiting");
}

//-------------------------------------------------------------------------

#ifdef XXXXX
    static CHARACTER *to_string(unsigned char *data, int length, int be, int
        *retlen)
    {
        int c, i;
        CHARACTER *ret;

        c = 0;
        while (1)
        {
            checksize(length, (c + 1) *2);
            if (get_16(be, data + c * 2) == 0)
                break;
            ++c;
        }

        ret = (CHARACTER*)AllocateMemory((c + 1) *sizeof(CHARACTER));

        for (i = 0; i < c; i++)
            ret[i] = get_16(be, data + i * 2);
        ret[i] = 0;

        if (retlen != NULL)
            *retlen = c;

        return ret;
    }

    static int to_resid(RES_ID *id, unsigned char *data, int length, int be)
    {
        int first;

        checksize(length, 2);

        first = get_16(be, data);
        if (first == 0xffff)
        {
            checksize(length, 4);
            id->hasname = FALSE;
            id->v.id = get_16(be, data + 2);
            return 4;
        }
        else
        {
            id->hasname = TRUE;
            id->v.n.name = to_string(data, length, be, &id->v.n.len);
            return id->v.n.len *2+2;
        }
    }
    static RES_RES *to_accelerators(unsigned char *data, int length, int be)
    {
        RES_RES *r = AllocateMemory(sizeof(RES_RES));
        struct accelerator *first = NULL,  **pp = &first;

        r->type = RST_ACCELERATOR;

        while (1)
        {
            struct accelerator *a;

            checksize(length, 8);

            a = AllocateMemory(sizeof(*a));

            a->flags = get_16(be, data);
            a->key = get_16(be, data + 2);
            a->id = get_16(be, data + 4);

            a->link = NULL;
            *pp = a;
            pp = &a->link;

            if ((a->flags &ACC_LAST) != 0)
                break;

            data += 8;
            length -= 8;
        } 

        r->u.acc = first;

        return r;
    }
    static RES_RES *to_cursor(unsigned char *data, int length, int be)
    {
        RES_RES *r = AllocateMemory(sizeof(RES_RES));
        struct cursor *c = AllocateMemory(sizeof(struct cursor));

        checksize(length, 4);

        c->xhotspot = get_16(be, data);
        c->yhotspot = get_16(be, data + 2);
        c->length = length - 4;
        c->data = data + 4;

        r->type = RST_CURSOR;
        r->u.cursor = c;

        return r;
    }
    static RES_RES *to_generic(enum res_type type, unsigned char *data, int
        length)
    {

        RES_RES *r = AllocateMemory(sizeof(RES_RES));
        r->type = type;
        r->u.data.data = data;
        r->u.data.length = length;

        return r;
    } 


    static RES_RES *to_dialog(unsigned char *data, int length, int be)
    {
        int version;
        struct dialog *d = AllocateMemory(sizeof(struct dialog));
        int c, sublen, off, i;
        struct dialog_control **pp;
        RES_RES *r = AllocateMemory(sizeof(RES_RES));

        r->type = RST_DIALOG;
        r->u.dialog = d;

        checksize(length, 18);

        version = get_16(be, data);
        if (version != 0xffff)
        {
            d->ex = NULL;
            d->style = get_32(be, data);
            d->exstyle = get_32(be, data + 4);
            off = 8;
        } 
        else
        {
            int signature;

            signature = get_16(be, data + 2);
            if (signature != 1)
                fatal("unexpected dialog signature %d", signature);

            d->ex = (struct dialog_ex*)AllocateMemory(sizeof(struct dialog_ex));
            d->ex->help = get_32(be, data + 4);
            d->exstyle = get_32(be, data + 8);
            d->style = get_32(be, data + 12);
            off = 16;
        }
        checksize(length, off + 10);

        c = get_16(be, data + off);
        d->x = get_16(be, data + off + 2);
        d->y = get_16(be, data + off + 4);
        d->width = get_16(be, data + off + 6);
        d->height = get_16(be, data + off + 8);

        off += 10;

        sublen = to_resid(&d->menu, data + off, length - off, be);
        off += sublen;

        sublen = to_resid(&d->class , data + off, length - off, be);
        off += sublen;

        d->caption = to_string(data + off, length - off, be, &sublen);
        off += sublen * 2+2;

        if (!(d->style &DS_SETFONT))
        {
                d->pointsize = 0;
                d->font = NULL;
                if (d->ex)
                {
                        d->ex->weight = 0;
                        d->ex->italic = 0;
                } 
        }
        else
        {
            checksize(length, off + 2);

            d->pointsize = get_16(be, data + off);
            off += 2;

            if (d->ex)
            {
                checksize(length, off + 4);
                d->ex->weight = get_16(be, data + off);
                d->ex->italic = get_16(be, data + off + 2);
                off += 4;
            }

            d->font = to_string(data + off, length - off, be, &sublen);
            off += sublen * 2+2;
        }

        d->controls = NULL;
        pp = &d->controls;

        for (i = 0; i < c; i++)
        {
            struct dialog_control *dc = AllocateMemory(sizeof(struct
                dialog_control));
            int datalen;

            off = (off + 3) &~3;

            if (!d->ex)
            {
                checksize(length, off + 8);

                dc->style = get_32(be, data + off);
                dc->exstyle = get_32(be, data + off + 4);
                dc->help = 0;
                off += 8;
            } 
            else
            {
                checksize(length, off + 12);
                dc->help = get_32(be, data + off);
                dc->exstyle = get_32(be, data + off + 4);
                dc->style = get_32(be, data + off + 18);
                off += 12;
            }

            checksize(length, off + 10);

            dc->x = get_16(be, data + off);
            dc->y = get_16(be, data + off + 2);
            dc->width = get_16(be, data + off + 4);
            dc->height = get_16(be, data + off + 6);
            dc->id = get_16(be, data + off + 8);

            off += 10;

            sublen = to_resid(&dc->class , data + off, length - off, be);
            off += sublen;

            sublen = to_resid(&dc->text, data + off, length - off, be);
            off += sublen;

            checksize(length, off + 2);

            datalen = get_16(be, data + off);
            off += 2;

            dc->data = NULL;
            if (datalen)
            {
                    off = (off + 3) &~3;

                    checksize(length, off + datalen);
                    dc->data = AllocateMemory(sizeof(struct rcdata_item));
                    dc->data->link = NULL;
                    dc->data->type = RCDATA_BUFFER;
                    dc->data->u.buffer.length = datalen;
                    dc->data->u.buffer.data = data + off;

                    off += datalen;
            }

            dc->link = NULL;
            *pp = dc;
            pp = &dc->link;
        }


        return r;
    }
    static RES_RES *to_fontdir(unsigned char *data, int length, int be)
    {
        RES_RES *r = AllocateMemory(sizeof(RES_RES));
        int c, i;
        struct fontdir *first = NULL,  **pp = &first;

        r->type = RST_FONTDIR;

        checksize(length, 2);

        c = get_16(be, data);

        for (i = 0; i < c; i++)
        {
            struct fontdir *fd = AllocateMemory(sizeof(struct fontdir));
            int off;

            checksize(length, 56);

            fd->index = get_16(be, data);

            off = 56;

            while (off < length && data[off] != '\0')
                ++off;
            ++off;

            while (off < length && data[off] != '\0')
                ++off;
            checksize(length, off);
            ++off;

            fd->length = off;
            fd->data = data;

            fd->link = NULL;
            *pp = fd;
            pp = &fd->link;

            data += off;
            length -= off;
        } 

        r->u.fontdir = first;

        return r;
    }
    static RES_RES *to_group_cursor(unsigned char *data, int length, int be)
    {
        RES_RES *r = AllocateMemory(sizeof(RES_RES));
        int type, c, i;
        struct group_cursor *first = NULL,  **pp = &first;

        checksize(length, 6);

        r->type = RST_GROUP_CURSOR;

        type = get_16(be, data + 2);
        if (type != 2)
            fatal("unexpected group cursor type %d", type);

        c = get_16(be, data + 4);

        data += 6;
        length -= 6;

        for (i = 0; i < c; i++)
        {
            struct group_cursor *gc = AllocateMemory(sizeof(struct group_cursor)
                );

            checksize(length, 14);

            gc->width = get_16(be, data);
            gc->height = get_16(be, data + 2);
            gc->planes = get_16(be, data + 4);
            gc->bits = get_16(be, data + 6);
            gc->bytes = get_32(be, data + 8);
            gc->index = get_16(be, data + 12);

            gc->link = NULL;
            *pp = gc;
            pp = &gc->link;

            data += 14;
            length -= 14;
        } 

        r->u.group_cursor = first;

        return r;
    }
    static RES_RES *to_group_icon(unsigned char *data, int length, int be)
    {
        RES_RES *r = AllocateMemory(sizeof(RES_RES));
        int type, c, i;
        struct group_icon *first = NULL,  **pp = &first;

        r->type = RST_GROUP_ICON;

        checksize(length, 6);

        type = get_16(be, data + 2);
        if (type != 1)
            fatal("unexpected group icon type %d", type);

        c = get_16(be, data + 4);

        data += 6;
        length -= 6;

        for (i = 0; i < c; i++)
        {
            struct group_icon *gi = AllocateMemory(sizeof(struct group_icon));

            checksize(length, 14);

            gi->width = data[0];
            gi->height = data[1];
            gi->colors = data[2];
            gi->planes = get_16(be, data + 4);
            gi->bits = get_16(be, data + 6);
            gi->bytes = get_32(be, data + 8);
            gi->index = get_16(be, data + 12);

            gi->link = NULL;
            *pp = gi;
            pp = &gi->link;

            data += 14;
            length -= 14;
        } 

        r->u.group_icon = first;

        return r;
    }
    static struct menuitem *to_menuitems(unsigned char *data, int length, int
        be, int *read)
    {
        struct menuitem *first = NULL,  **p = &first,  *mi;
        int flags, stroff, slen, itemlen;

        *read = 0;

        while (length > 0)
        {
            checksize(length, 4);
            mi = AllocateMemory(sizeof(*mi));
            mi->state = 0;
            mi->help = 0;

            mi->type = (flags = get_16(be, data)) &~(MI_POPUP | MI_ENDMENU);

            if ((flags &MI_POPUP) == 0)
                stroff = 4;
            else
                stroff = 2;

            checksize(length, stroff);

            mi->text = to_string(data + stroff, length - stroff, be, &slen);

            itemlen = stroff + slen * 2+2;

            if ((flags &MI_POPUP) == 0)
            {
                mi->popup = NULL;
                mi->id = get_16(be, data + 2);
            } 
            else
            {
                int i;

                mi->id = 0;
                mi->popup = to_menuitems(data + itemlen, length - itemlen, be,
                    &i);
                itemlen += i;
            }

            mi->link = NULL;
            *p = mi;
            p = &mi->link;

            data += itemlen;
            length -= itemlen;
            *read += itemlen;

            if ((flags &MI_ENDMENU))
                return first;
        }

        return first;
    }

    static struct menuitem *to_menuexitems(unsigned char *data, int length, int
        be, int *read)
    {
        struct menuitem *first = NULL,  **p = &first,  *mi;
        int flags, slen, itemlen;

        *read = 0;

        while (length > 0)
        {
            checksize(length, 14);

            mi = AllocateMemory(sizeof(*mi));
            mi->type = get_32(be, data);
            mi->state = get_32(be, data + 4);
            mi->id = get_16(be, data + 8);

            flags = get_16(be, data + 10);

            mi->text = to_string(data + 12, length - 12, be, &slen);

            itemlen = 12+slen * 2+2;
            itemlen = (itemlen + 3) &~3;

            if ((flags &1) == 0)
            {
                mi->popup = NULL;
                mi->help = 0;
            } 
            else
            {
                int i;

                checksize(length, itemlen + 4);
                mi->help = get_32(be, data + itemlen);
                itemlen += 4;

                mi->popup = to_menuexitems(data + itemlen, length - itemlen, be,
                    &i);
                itemlen += i;
            }

            mi->link = NULL;
            *p = mi;
            p = &mi->link;

            data += itemlen;
            length -= itemlen;
            *read += itemlen;

            if ((flags &0x80) != 0)
                return first;
        }

        return first;
    }

    static RES_RES *to_menu(unsigned char *data, int length, int be)
    {
        RES_RES *r = AllocateMemory(sizeof(RES_RES));
        struct menu *m;
        int version, read;

        r->type = RST_MENU;

        r->u.menu = m = AllocateMemory(sizeof *m);
        m->help = 0;

        checksize(length, 2);

        version = get_16(be, data);

        if (version == 0)
        {
            checksize(length, 4);
            m->items = to_menuitems(data + 4, length - 4, be,  &read);
        } 
        else if (version == 1)
        {
            int offset;
            checksize(length, 8);
            m->help = get_32(be, data + 4);
            offset = get_16(be, data + 2);
            checksize(length, offset + 4);
            m->items = to_menuexitems(data + 4+offset, length - (4+offset), be,
                &read);
        }
        else
            fatal("unsupported menu version %d", version);

        return r;
    }


    static RES_RES *to_rcdata(unsigned char *data, int length, int be)
    {
        RES_RES *r = AllocateMemory(sizeof(RES_RES));
        struct rcdata_item *ri = AllocateMemory(sizeof(struct rcdata_item));

        r->type = RST_RCDATA;
        r->u.rcdata = ri;

        ri->link = NULL;
        ri->type = RCDATA_BUFFER;
        ri->u.buffer.length = length;
        ri->u.buffer.data = data;

        return r;
    }
    static RES_RES *to_stringtable(unsigned char *data, int length, int be)
    {
        RES_RES *r = AllocateMemory(sizeof(RES_RES));
        struct stringtable *st = AllocateMemory(sizeof(struct stringtable));
        int i;

        r->type = RST_STRINGTABLE;
        r->u.stringtable = st;

        for (i = 0; i < 16; i++)
        {
            int slen;

            checksize(length, 2);
            if (st->strings[i].length = slen = get_16(be, data))
            {
                CHARACTER *s;
                int j;

                checksize(length, 2+2 * slen);
                st->strings[i].string = s = AllocateMemory((slen + 1) *sizeof
                    (CHARACTER));

                for (j = 0; j < slen; j++)
                    s[j] = get_16(be, data + 2+j * 2);
                s[j] = 0;
            } 

            data += 2+2 * slen;
            length -= 2+2 * slen;
        }


        return r;
    }

    static RES_RES *to_userdata(unsigned char *data, int length, int be)
    {
        RES_RES *r = AllocateMemory(sizeof(RES_RES));
        struct rcdata_item *ri = AllocateMemory(sizeof(struct rcdata_item));

        r->type = RST_USERDATA;
        r->u.rcdata = ri;

        ri->link = NULL;
        ri->type = RCDATA_BUFFER;
        ri->u.buffer.length = length;
        ri->u.buffer.data = data;

        return r;
    }
    static void get_version_header(unsigned char *data, int length, int be,
        char *key, CHARACTER **pkey, int *len, int *vallen, int *type, int *off)
    {
        checksize(length, 8);

        *len = get_16(be, data);
        *vallen = get_16(be, data + 2);
        *type = get_16(be, data + 4);

        *off = 6;

        length -= 6;
        data += 6;

        if (key == NULL)
        {
            int i;

            *pkey = to_string(data, length, be, &i);
            *off += i * 2+2;
        } 
        else
        {
            while (1)
            {
                checksize(length, 2);
                if (get_16(be, data) !=  *key)
                    fatal("unexpected version string");

                *off += 2;
                length -= 2;
                data += 2;

                if (*key == '\0')
                    break;

                ++key;
            }
        }

        *off = (*off + 3) &~3;
    }

    static RES_RES *to_version(unsigned char *data, int length, int be)
    {
        int verlen, vallen, type, off;
        struct fixed_versioninfo *fi;
        struct ver_info *first,  **pp;
        struct versioninfo *v;
        RES_RES *r = AllocateMemory(sizeof(RES_RES));

        r->type = RST_VERSIONINFO;

        get_version_header(data, length, be, "VS_VERSION_INFO", (CHARACTER*)
            NULL, &verlen, &vallen, &type, &off);

        if (verlen != length)
            fatal("version length %d does not match resource length %lu",
                verlen, length);

        if (type != 0)
            fatal("unexpected version type %d", type);

        data += off;
        length -= off;

        if (vallen == 0)
            fi = NULL;
        else
        {
            unsigned long signature, fiv;

            if (vallen != 52)
                fatal("unexpected fixed version information length %d", vallen);

            checksize(length, 52);

            signature = get_32(be, data);
            if (signature != 0xfeef04bd)
                fatal("unexpected fixed version signature %lu", signature);

            fiv = get_32(be, data + 4);
            if (fiv != 0 && fiv != 0x10000)
                fatal("unexpected fixed version info version %lu", fiv);

            fi = AllocateMemory(sizeof *fi);

            fi->file_version_ms = get_32(be, data + 8);
            fi->file_version_ls = get_32(be, data + 12);
            fi->product_version_ms = get_32(be, data + 16);
            fi->product_version_ls = get_32(be, data + 20);
            fi->file_flags_mask = get_32(be, data + 24);
            fi->file_flags = get_32(be, data + 28);
            fi->file_os = get_32(be, data + 32);
            fi->file_type = get_32(be, data + 36);
            fi->file_subtype = get_32(be, data + 40);
            fi->file_date_ms = get_32(be, data + 44);
            fi->file_date_ls = get_32(be, data + 48);

            data += 52;
            length -= 52;
        } 

        first = NULL;
        pp = &first;

        while (length > 0)
        {
            struct ver_info *vi = AllocateMemory(sizeof(struct ver_info));
            int ch;

            checksize(length, 8);

            ch = get_16(be, data + 6);

            if (ch == 'S')
            {
                struct ver_stringinfo **ppvs = &vi->u.string.strings;

                vi->type = VERINFO_STRING;

                get_version_header(data, length, be, "StringFileInfo", 
                    (CHARACTER*)NULL, &verlen, &vallen, &type,  &off);

                if (vallen != 0)
                    fatal("unexpected stringfileinfo value length %d", vallen);

                data += off;
                length -= off;

                get_version_header(data, length, be, (const char*)NULL,  &vi
                    ->u.string.language, &verlen, &vallen,  &type, &off);

                if (vallen != 0)
                    fatal("unexpected version stringtable value length %d",
                        vallen);

                data += off;
                length -= off;
                verlen -= off;

                vi->u.string.strings = NULL;

                /* It's convenient to round verlen to a 4 byte alignment,
                since we round the subvariables in the loop.  */
                verlen = (verlen + 3) &~3;

                while (verlen > 0)
                {
                    struct ver_stringinfo *vs = AllocateMemory(sizeof(struct
                        ver_stringinfo));
                    int subverlen, vslen, valoff;

                    get_version_header(data, length, be, (const char*)NULL, &vs
                        ->key, &subverlen,  &vallen, &type, &off);

                    subverlen = (subverlen + 3) &~3;

                    data += off;
                    length -= off;

                    vs->value = to_string(data, length, be, &vslen);
                    valoff = vslen * 2+2;
                    valoff = (valoff + 3) &~3;

                    if (off + valoff != subverlen)
                        fatal("unexpected version string length %d != %d + %d",
                            subverlen, off, valoff);

                    vs->link = NULL;
                    *ppvs = vs;
                    ppvs = &vs->link;

                    data += valoff;
                    length -= valoff;

                    if (verlen < subverlen)
                        fatal("unexpected version string length %d < %d",
                            verlen, subverlen);

                    verlen -= subverlen;
                } 
            }
            else if (ch == 'V')
            {
                struct ver_varinfo **ppvv = &vi->u.var.var;

                vi->type = VERINFO_VAR;

                get_version_header(data, length, be, "VarFileInfo", (CHARACTER*)
                    NULL, &verlen, &vallen, &type,  &off);

                if (vallen != 0)
                    fatal("unexpected varfileinfo value length %d", vallen);

                data += off;
                length -= off;

                get_version_header(data, length, be, (const char*)NULL,  &vi
                    ->u.var.key, &verlen, &vallen, &type, &off);

                data += off;
                length -= off;

                vi->u.var.var = NULL;

                while (vallen > 0)
                {
                    struct ver_varinfo *vv;

                    checksize(length, 4);

                    vv = AllocateMemory(sizeof *vv);

                    vv->language = get_16(be, data);
                    vv->charset = get_16(be, data + 2);

                    vv->link = NULL;
                    *ppvv = vv;
                    ppvv = &vv->link;

                    data += 4;
                    length -= 4;

                    if (vallen < 4)
                        fatal("unexpected version value length %d", vallen);

                    vallen -= 4;
                } 
            }
            else
                fatal("unexpected version string");

            vi->link = NULL;
            *pp = vi;
            pp = &vi->link;
        }

        v = AllocateMemory(sizeof *v);
        v->fixed = fi;
        v->var = first;

        r->u.versioninfo = v;

        return r;
    }
#endif 
RES_RES *convert_to_internal(RES_ID *type, unsigned char *data, int length, int
    be)
{
    return 0;
    #ifdef XXXXX
        if (type->hasname)
            return to_userdata(data, length, be);
        else
        {
            switch (type->v.id)
            {
                default:
                    return to_userdata(data, length, be);
                case RT_ACCELERATOR:
                    return to_accelerators(data, length, be);
                case RT_BITMAP:
                    return to_generic(RST_BITMAP, data, length);
                case RT_CURSOR:
                    return to_cursor(data, length, be);
                case RT_DIALOG:
                    return to_dialog(data, length, be);
                case RT_DLGINCLUDE:
                    return to_generic(RST_DLGINCLUDE, data, length);
                case RT_FONT:
                    return to_generic(RST_FONT, data, length);
                case RT_FONTDIR:
                    return to_fontdir(data, length, be);
                case RT_GROUP_CURSOR:
                    return to_group_cursor(data, length, be);
                case RT_GROUP_ICON:
                    return to_group_icon(data, length, be);
                case RT_ICON:
                    return to_generic(RST_ICON, data, length);
                case RT_MENU:
                    return to_menu(data, length, be);
                case RT_MESSAGETABLE:
                    return to_generic(RST_MESSAGETABLE, data, length);
                case RT_RCDATA:
                    return to_rcdata(data, length, be);
                case RT_STRING:
                    return to_stringtable(data, length, be);
                case RT_VERSION:
                    return to_version(data, length, be);
            }
        }
    #endif 
}

//-------------------------------------------------------------------------

static void put_16(int be, int v, unsigned char *s)
{
    if (be)
    {
        *s++ = v >> 8;
        *s++ = v;
    }
    else
    {
        *(short*)s = v;
    }
}

//-------------------------------------------------------------------------

static long put_32(int be, int v, unsigned char *s)
{
    if (be)
    {
        *s++ = v >> 24;
        *s++ = v >> 16;
        *s++ = v >> 8;
        *s++ = v;
    }
    else
    {
        *(long*)s = v;
    }
}

//-------------------------------------------------------------------------

static void align(BINDATA * * * ppp, long *length)
{
    int add;
    BINDATA *d;

    if ((*length &3) == 0)
        return ;

    add = 4-(*length &3);

    d = (BINDATA*)AllocateMemory(sizeof *d);
    d->length = add;
    d->data = AllocateMemory(add);
    memset(d->data, 0, add);

    d->link = NULL;
    **ppp = d;
    *ppp = &(**ppp)->link;

    *length += add;
}

//-------------------------------------------------------------------------

static BINDATA *from_resid(RES_ID id, int be)
{
    BINDATA *d = AllocateMemory(sizeof(*d));
    if (!id.hasname)
    {
        if (!id.v.id)
        {

            d->length = 2;
            d->data = AllocateMemory(4);
            put_16(be, 0, d->data);
        }
        else
        {
            d->length = 4;
            d->data = AllocateMemory(4);
            put_16(be, 0xffff, d->data);
            put_16(be, id.v.id, d->data + 2);
        }
    }
    else
    {
        int i;

        d->length = id.v.n.len * 2+2;
        d->data = AllocateMemory(d->length);
        for (i = 0; i < id.v.n.len + 1; i++)
            put_16(be, (unsigned char)id.v.n.name[i], d->data + i * 2);
    }

    d->link = NULL;

    return d;
}

//-------------------------------------------------------------------------

static BINDATA *from_string(CHARACTER *str, int be)
{
    int len, i;
    BINDATA *d;
    CHARACTER *s;

    len = 1;

    if (str)
        for (s = str;  *s != 0; s++)
            ++len;

    d = AllocateMemory(sizeof *d);
    d->length = len * 2;
    d->data = AllocateMemory(d->length);

    if (str)
        for (s = str, i = 0; i < len; s++, i++)
            put_16(be,  (unsigned char)*s, d->data + i * 2);
        else
            put_16(be, 0, d->data);

    d->link = NULL;

    return d;
}

//-------------------------------------------------------------------------

static BINDATA *from_string_len(CHARACTER *str, int be, int len)
{
    int i;
    BINDATA *d;
    CHARACTER *s;

    d = AllocateMemory(sizeof *d);
    d->length = len * 2;
    d->data = AllocateMemory(d->length);

    if (str)
        for (s = str, i = 0; i < len; s++, i++)
            put_16(be,  (unsigned char)*s, d->data + i * 2);
        else
            put_16(be, 0, d->data);

    d->link = NULL;

    return d;
}

//-------------------------------------------------------------------------

static BINDATA *ascii_to_string(char *s, int be)
{
    size_t len, i;
    BINDATA *d = AllocateMemory(sizeof(BINDATA));

    len = strlen(s);

    d->length = len * 2+2;
    d->data = AllocateMemory(d->length);

    for (i = 0; i < len + 1; i++)
        put_16(be, (unsigned char)s[i], d->data + i * 2);

    d->link = NULL;

    return d;
}

//-------------------------------------------------------------------------

int extended_dialog(struct dialog *dialog)
{
    const struct dialog_control *c;

    if (dialog->ex != NULL)
        return 1;

    for (c = dialog->controls; c != NULL; c = c->link)
        if (c->data != NULL || c->help != 0)
            return 1;

    return 0;
} 

static int extended_menuitems(struct menuitem *menuitems)
{
    const struct menuitem *mi;

    for (mi = menuitems; mi != NULL; mi = mi->link)
    {
        if (mi->help != 0 || mi->state != 0 || mi->type != 0)
            return 1;
        if (mi->popup != NULL && mi->id != 0)
            return 1;
//        if ((mi->type &~(MI_CHECKED | MI_GRAYED | MI_HELP | MI_INACTIVE |
//            MI_MENUBARBREAK | MI_MENUBREAK)) != 0)
//            return 1;
        if (mi->popup != NULL)
        {
            if (extended_menuitems(mi->popup))
                return 1;
        } 
    }

    return 0;
}

//-------------------------------------------------------------------------

int extended_menu(struct menu *menu)
{
    return extended_menuitems(menu->items);
} 


static BINDATA *from_rcdata(struct rcdata_item *items, int be)
{
    BINDATA *first = NULL,  **pp = &first;
    const struct rcdata_item *ri;

    for (ri = items; ri != NULL; ri = ri->link)
    {
        BINDATA *d = AllocateMemory(sizeof(*d));
        switch (ri->type)
        {
            default:
                fatal("invalid RCDATA sect");

            case RCDATA_WORD:
                d->length = 2;
                d->data = AllocateMemory(2);
                put_16(be, ri->u.word, d->data);
                break;

            case RCDATA_DWORD:
                d->length = 4;
                d->data = AllocateMemory(4);
                put_32(be, ri->u.dword, d->data);
                break;

            case RCDATA_STRING:
                d->length = ri->u.string.length;
                d->data = (unsigned char*)ri->u.string.s;
                break;

            case RCDATA_WSTRING:
                {
                    unsigned long i;

                    d->length = ri->u.wstring.length *2;
                    d->data = AllocateMemory(d->length);
                    for (i = 0; i < ri->u.wstring.length; i++)
                        put_16(be, ri->u.wstring.w[i], d->data + i * 2);
                    break;
                }

            case RCDATA_BUFFER:
                d->length = ri->u.buffer.length;
                d->data = (unsigned char*)ri->u.buffer.data;
                break;
        }

        d->link = NULL;
        *pp = d;
        pp = &d->link;
    }

    return first;
}

//-------------------------------------------------------------------------

static BINDATA *from_accelerator(struct accelerator *acc, int be)
{
    BINDATA *first = NULL,  **pp = &first;
    const struct accelerator *a;
    for (a = acc; a != NULL; a = a->link)
    {
        BINDATA *d = AllocateMemory(sizeof(BINDATA));
        d->length = 8;
        d->data = AllocateMemory(8);

        put_16(be, a->flags | (a->link != NULL ? 0 : ACC_LAST), d->data);
        put_16(be, a->key, d->data + 2);
        put_16(be, a->id, d->data + 4);
        put_16(be, 0, d->data + 6);

        d->link = NULL;
        *pp = d;
        pp = &d->link;
    } 

    return first;
}

//-------------------------------------------------------------------------

static BINDATA *from_cursor(struct cursor *c, int be)
{
    BINDATA *d = AllocateMemory(sizeof(BINDATA));
    d->length = 4;
    d->data = AllocateMemory(4);

    put_16(be, c->xhotspot, d->data);
    put_16(be, c->yhotspot, d->data + 2);

    d->link = AllocateMemory(sizeof *d);
    d->link->length = c->length;
    d->link->data = (unsigned char*)c->data;
    d->link->link = NULL;

    return d;
} 

static BINDATA *from_dialog(struct dialog *dialog, int be)
{
    int dialogex;
    BINDATA *first = AllocateMemory(sizeof(BINDATA)),  **pp = &first->link;
    unsigned long length;
    int off, c;
    struct dialog_control *dc;

    dialogex = extended_dialog(dialog);

    first->length = dialogex ? 26 : 18;
    first->data = AllocateMemory(first->length);
    first->link = NULL;

    length = first->length;

    if (!dialogex)
    {
        put_32(be, dialog->style, first->data);
        put_32(be, dialog->exstyle, first->data + 4);
        off = 8;
    } 
    else
    {
        put_16(be, 1, first->data);
        put_16(be, 0xffff, first->data+2);
        if (dialog->ex == NULL)
            put_32(be, 0, first->data + 4);
        else
            put_32(be, dialog->ex->help, first->data + 4);
        put_32(be, dialog->exstyle, first->data + 8);
        put_32(be, dialog->style, first->data + 12);
        off = 16;
    }
    put_16(be, dialog->x, first->data + off + 2);
    put_16(be, dialog->y, first->data + off + 4);
    put_16(be, dialog->width, first->data + off + 6);
    put_16(be, dialog->height, first->data + off + 8);

    *pp = from_resid(dialog->menu, be);
    length += (*pp)->length;
    pp = &(*pp)->link;

    *pp = from_resid(dialog->class , be);
    length += (*pp)->length;
    pp = &(*pp)->link;

    *pp = from_string(dialog->caption, be);
    length += (*pp)->length;
    pp = &(*pp)->link;

    if ((dialog->style &DS_SETFONT) != 0)
    {
            BINDATA *d = AllocateMemory(sizeof(BINDATA));

            d->length = dialogex ? 6 : 2;
            d->data = AllocateMemory(d->length);

            length += d->length;

            put_16(be, dialog->pointsize, d->data);

            if (dialogex)
            if (dialog->ex == NULL)
            {
                    put_16(be, 0, d->data + 2);
                    put_16(be, 0, d->data + 4);
                    put_16(be, 0, d->data + 6);
            }
            else
            {
                    put_16(be, dialog->ex->weight, d->data + 2);
                    put_16(be, dialog->ex->italic, d->data + 4);
                    put_16(be, dialog->ex->charset, d->data + 6);
            }

            *pp = d;
            pp = &d->link;

            *pp = from_string(dialog->font, be);
            length += (*pp)->length;
            pp = &(*pp)->link;
    }

    c = 0;
    for (dc = dialog->controls; dc != NULL; dc = dc->link)
    {
        BINDATA *d = AllocateMemory(sizeof(BINDATA));
        int dcoff;

        ++c;

        align(&pp, &length);

        d->length = dialogex ? 24 : 18;
        d->data = AllocateMemory(d->length);

        length += d->length;

        if (!dialogex)
        {
            put_32(be, dc->style, d->data);
            put_32(be, dc->exstyle, d->data + 4);
            dcoff = 8;
        }
        else
        {
            put_32(be, dc->help, d->data);
            put_32(be, dc->exstyle, d->data + 4);
            put_32(be, dc->style, d->data + 8);
            dcoff = 12;
        }

        put_16(be, dc->x, d->data + dcoff);
        put_16(be, dc->y, d->data + dcoff + 2);
        put_16(be, dc->width, d->data + dcoff + 4);
        put_16(be, dc->height, d->data + dcoff + 6);
		if (dialogex)
	        put_32(be, dc->id, d->data + dcoff + 8);
		else
	        put_16(be, dc->id, d->data + dcoff + 8);

        *pp = d;
        pp = &d->link;

        *pp = from_resid(dc->class , be);
        length += (*pp)->length;
        pp = &(*pp)->link;

        *pp = from_resid(dc->text, be);
        length += (*pp)->length;
        pp = &(*pp)->link;

        d = AllocateMemory(sizeof *d);
        d->length = 2;
        d->data = AllocateMemory(2);

        length += 2;

        d->link = NULL;
        *pp = d;
        pp = &d->link;

        if (dc->data == NULL)
            put_16(be, 0, d->data);
        else
        {
                unsigned long i;

                align(&pp, &length);

                *pp = from_rcdata(dc->data, be);
                i = 0;
                while (*pp != NULL)
                {
                        i += (*pp)->length;
                        pp = &(*pp)->link;
                }

                put_16(be, i, d->data);

                length += i;
        }
    }

    put_16(be, c, first->data + off);

    return first;
}

//-------------------------------------------------------------------------

static BINDATA *from_fontdir(struct fontdir *fontdirs, int be)
{
    BINDATA *first = AllocateMemory(sizeof(BINDATA)),  **pp = &first->link;
    int c;
    const struct fontdir *fd;

    first->length = 2;
    first->data = AllocateMemory(2);
    first->link = NULL;

    c = 0;
    for (fd = fontdirs; fd != NULL; fd = fd->link)
    {
        BINDATA *d;

        ++c;

        d = AllocateMemory(sizeof *d);
        d->length = 2;
        d->data = AllocateMemory(2);

        put_16(be, fd->index, d->data);

        *pp = d;
        pp = &d->link;

        d = AllocateMemory(sizeof *d);
        d->length = fd->length;
        d->data = (unsigned char*)fd->data;

        d->link = NULL;
        *pp = d;
        pp = &d->link;
    } 

    put_16(be, c, first->data);

    return first;
}

//-------------------------------------------------------------------------

static BINDATA *from_generic(unsigned long length, unsigned char *data)
{
    BINDATA *d = AllocateMemory(sizeof(BINDATA));
    d->length = length;
    d->data = (unsigned char*)data;

    d->link = NULL;

    return d;
}

//-------------------------------------------------------------------------

static BINDATA *from_group_cursor(struct group_cursor *gcs, int be)
{
    BINDATA *first = AllocateMemory(sizeof(BINDATA)),  **pp = &first->link;
    const struct group_cursor *gc;
    int c;

    first->length = 6;
    first->data = AllocateMemory(6);
    first->link = NULL;
    pp = &first->link;

    put_16(be, 0, first->data);
    put_16(be, 2, first->data + 2);

    c = 0;
    for (gc = gcs; gc != NULL; gc = gc->link)
    {
        BINDATA *d = AllocateMemory(sizeof(BINDATA));

        ++c;

        d->length = 14;
        d->data = AllocateMemory(14);

        put_16(be, gc->width, d->data);
        put_16(be, gc->height, d->data + 2);
        put_16(be, gc->planes, d->data + 4);
        put_16(be, gc->bits, d->data + 6);
        put_32(be, gc->bytes, d->data + 8);
        put_16(be, gc->index, d->data + 12);

        d->link = NULL;
        *pp = d;
        pp = &d->link;
    } 

    put_16(be, c, first->data + 4);

    return first;
}

//-------------------------------------------------------------------------

static BINDATA *from_group_icon(struct group_icon *group_icons, int be)
{
    BINDATA *first = AllocateMemory(sizeof(BINDATA)),  **pp = &first->link;
    int c;
    const struct group_icon *gi;

    first->length = 6;
    first->data = AllocateMemory(6);
    first->link = NULL;

    put_16(be, 0, first->data);
    put_16(be, 1, first->data + 2);

    c = 0;
    for (gi = group_icons; gi != NULL; gi = gi->link)
    {
        BINDATA *d = AllocateMemory(sizeof(BINDATA));

        ++c;

        d->length = 14;
        d->data = AllocateMemory(14);

        d->data[0] = gi->width;
        d->data[1] = gi->height;
        d->data[2] = gi->colors;
        d->data[3] = 0;
        put_16(be, gi->planes, d->data + 4);
        put_16(be, gi->bits, d->data + 6);
        put_32(be, gi->bytes, d->data + 8);
        put_16(be, gi->index, d->data + 12);

        d->link = NULL;
        *pp = d;
        pp = &d->link;
    } 

    put_16(be, c, first->data + 4);

    return first;
}

//-------------------------------------------------------------------------

static BINDATA *from_menuitems(struct menuitem *items, int be)
{
    BINDATA *first = NULL,  **pp = &first;
    const struct menuitem *mi;

    for (mi = items; mi != NULL; mi = mi->link)
    {
        BINDATA *d = AllocateMemory(sizeof(BINDATA));
        int flags;

        d->length = mi->popup == NULL ? 4 : 2;
        d->data = AllocateMemory(d->length);

        flags = mi->flags;
        if (mi->link == NULL)
            flags |= MI_ENDMENU;
        if (mi->popup != NULL)
            flags |= MI_POPUP;

        put_16(be, flags, d->data);

        if (mi->popup == NULL)
            put_16(be, mi->id, d->data + 2);

        *pp = d;
        pp = &d->link;

        *pp = from_string(mi->text, be);
        pp = &(*pp)->link;

        if (mi->popup != NULL)
        {
            *pp = from_menuitems(mi->popup, be);
            while (*pp != NULL)
                pp = &(*pp)->link;
        } 
    }

    *pp = NULL;
    return first;
}

//-------------------------------------------------------------------------

static BINDATA *from_menuexitems(struct menuitem *items, int be)
{
    BINDATA *first = NULL,  **pp = &first;
    unsigned long length = 0;
    const struct menuitem *mi;

    for (mi = items; mi != NULL; mi = mi->link)
    {
        BINDATA *d = AllocateMemory(sizeof(BINDATA));
        int flags;

        align(&pp, &length);

        d->length = 14;
        d->data = AllocateMemory(14);

        length += 14;

        put_32(be, mi->type, d->data);
        put_32(be, mi->state, d->data + 4);
        put_32(be, mi->id, d->data + 8);

        flags = 0;
        if (mi->link == NULL)
            flags |= 0x80;
        if (mi->popup != NULL)
            flags |= 1;
        put_16(be, flags, d->data + 12);

        *pp = d;
        pp = &d->link;

        *pp = from_string(mi->text, be);
        length += (*pp)->length;
        pp = &(*pp)->link;

        if (mi->popup != NULL)
        {
            align(&pp, &length);

            d = AllocateMemory(sizeof *d);
            d->length = 4;
            d->data = AllocateMemory(4);

            put_32(be, mi->help, d->data);

            *pp = d;
            pp = &d->link;

            *pp = from_menuexitems(mi->popup, be);
            while (*pp != NULL)
            {
                length += (*pp)->length;
                pp = &(*pp)->link;
            } 
        }
    }

    *pp = NULL;
    return first;
}

//-------------------------------------------------------------------------

static BINDATA *from_menu(struct menu *menu, int be)
{
    int menuex;
    BINDATA *d = AllocateMemory(sizeof(BINDATA));

    menuex = extended_menu(menu);

    d->length = menuex ? 8 : 4;
    d->data = AllocateMemory(d->length);

    if (!menuex)
    {
        put_16(be, 0, d->data);
        put_16(be, 0, d->data + 2);

        d->link = from_menuitems(menu->items, be);
    } 
    else
    {
        put_16(be, 1, d->data);
        put_16(be, 4, d->data + 2);
        put_32(be, menu->help, d->data + 4);

        d->link = from_menuexitems(menu->items, be);
    }

    return d;
}

//-------------------------------------------------------------------------


static BINDATA *from_stringtable(struct stringtable *st, int be)
{
    BINDATA *first = NULL,  **pp = &first;
    int i;

    for (i = 0; i < 16; i++)
    {
        int slen, j;
        BINDATA *d = AllocateMemory(sizeof(*d));
        CHARACTER *s;

        slen = st->strings[i].length;
        s = st->strings[i].string;

        d->length = 2+slen * 2;
        d->data = AllocateMemory(d->length);

        put_16(be, slen, d->data);

        for (j = 0; j < slen; j++)
            put_16(be, s[j], d->data + 2+j * 2);

        d->link = NULL;
        *pp = d;
        pp = &d->link;
    } 

    return first;
}

//-------------------------------------------------------------------------

static BINDATA *from_versioninfo(struct versioninfo *versioninfo, int be)
{
    BINDATA *first = AllocateMemory(sizeof(BINDATA)),  **pp = &first->link;
    unsigned long length;
    struct ver_info *vi;

    first->length = 6;
    first->data = AllocateMemory(6);

    length = 6;

    if (versioninfo->fixed == NULL)
        put_16(be, 0, first->data + 2);
    else
        put_16(be, 52, first->data + 2);

    put_16(be, 0, first->data + 4);

    *pp = ascii_to_string("VS_VERSION_INFO", be);
    length += (*pp)->length;
    pp = &(*pp)->link;

    align(&pp, &length);

    if (versioninfo->fixed)
    {
        const struct fixed_versioninfo *fi;
        BINDATA *d = AllocateMemory(sizeof(BINDATA));

        d->length = 52;
        d->data = AllocateMemory(52);

        length += 52;

        fi = versioninfo->fixed;

        put_32(be, 0xfeef04bd, d->data);
        put_32(be, 0x10000, d->data + 4);
        put_32(be, fi->file_version_ms, d->data + 8);
        put_32(be, fi->file_version_ls, d->data + 12);
        put_32(be, fi->product_version_ms, d->data + 16);
        put_32(be, fi->product_version_ls, d->data + 20);
        put_32(be, fi->file_flags_mask, d->data + 24);
        put_32(be, fi->file_flags, d->data + 28);
        put_32(be, fi->file_os, d->data + 32);
        put_32(be, fi->file_type, d->data + 36);
        put_32(be, fi->file_subtype, d->data + 40);
        put_32(be, fi->file_date_ms, d->data + 44);
        put_32(be, fi->file_date_ls, d->data + 48);

        d->link = NULL;
        *pp = d;
        pp = &d->link;
    } 

    for (vi = versioninfo->var; vi != NULL; vi = vi->link)
    {
        BINDATA *vid = AllocateMemory(sizeof(BINDATA));
        unsigned long vilen;

        align(&pp, &length);

        vid->length = 6;
        vid->data = AllocateMemory(6);

        length += 6;
        vilen = 6;

        put_16(be, 0, vid->data + 2);
        put_16(be, 1, vid->data + 4);

        *pp = vid;
        pp = &vid->link;

        switch (vi->type)
        {
            default:
                fatal("invalid var version type");

            case VERINFO_STRING:
                {
                    unsigned long hold, vslen;
                    BINDATA *vsd = AllocateMemory(sizeof(BINDATA));
                    const struct ver_stringinfo *vs;

                    *pp = ascii_to_string("StringFileInfo", be);
                    length += (*pp)->length;
                    vilen += (*pp)->length;
                    pp = &(*pp)->link;

                    hold = length;
                    align(&pp, &length);
                    vilen += length - hold;

                    vsd->length = 6;
                    vsd->data = AllocateMemory(6);

                    length += 6;
                    vilen += 6;
                    vslen = 6;

                    put_16(be, 0, vsd->data + 2);
                    put_16(be, 1, vsd->data + 4);

                    *pp = vsd;
                    pp = &vsd->link;

                    *pp = from_string(vi->u.string.language, be);
                    length += (*pp)->length;
                    vilen += (*pp)->length;
                    vslen += (*pp)->length;
                    pp = &(*pp)->link;

                    for (vs = vi->u.string.strings; vs != NULL; vs = vs->link)
                    {
                        BINDATA *vssd;
                        unsigned long vsslen;

                        hold = length;
                        align(&pp, &length);
                        vilen += length - hold;
                        vslen += length - hold;

                        vssd = AllocateMemory(sizeof *vssd);
                        vssd->length = 6;
                        vssd->data = AllocateMemory(6);

                        length += 6;
                        vilen += 6;
                        vslen += 6;
                        vsslen = 6;

                        put_16(be, vs->length, vssd->data + 2);
                        put_16(be, 1, vssd->data + 4);

                        *pp = vssd;
                        pp = &vssd->link;

                        *pp = from_string(vs->key, be);
                        length += (*pp)->length;
                        vilen += (*pp)->length;
                        vslen += (*pp)->length;
                        vsslen += (*pp)->length;
                        pp = &(*pp)->link;

                        hold = length;
                        align(&pp, &length);
                        vilen += length - hold;
                        vslen += length - hold;
                        vsslen += length - hold;

                        *pp = from_string_len(vs->value, be, vs->length);
                        length += (*pp)->length;
                        vilen += (*pp)->length;
                        vslen += (*pp)->length;
                        vsslen += (*pp)->length;
                        pp = &(*pp)->link;

                        put_16(be, vsslen, vssd->data);
                    }

                    put_16(be, vslen, vsd->data);

                    break;
                }

            case VERINFO_VAR:
                {
                    unsigned long hold;
                    const struct ver_varinfo *vv;

                    *pp = ascii_to_string("VarFileInfo", be);
                    length += (*pp)->length;
                    vilen += (*pp)->length;
                    pp = &(*pp)->link;

                    hold = length;
                    align(&pp, &length);
                    vilen += length - hold;


                    for (vv = vi->u.var.var; vv != NULL; vv = vv->link)
                    {
                        unsigned long vvlen, vvvlen;
                        BINDATA *vvd = AllocateMemory(sizeof(BINDATA));
                        const struct ver_varlangchar *vlc;

                        vvd->length = vvlen = 6;
                        vvd->data = AllocateMemory(6);

                        length += 6;
                        vilen += 6;

                        put_16(be, 0, vvd->data + 4);

                        *pp = vvd;
                        pp = &vvd->link;

                        *pp = from_string(vv->key, be);
                        length += (*pp)->length;
                        vilen += (*pp)->length;
                        vvlen += (*pp)->length;
                        pp = &(*pp)->link;

                        hold = length;
                        align(&pp, &length);
                        vilen += length - hold;
                        vvlen += length - hold;

                        vvvlen = 0;

                        for (vlc = vv->intident; vlc != NULL; vlc = vlc->link)
                        {
                            BINDATA *vvsd = AllocateMemory(sizeof(BINDATA));

                            vvsd->length = 4;
                            vvsd->data = AllocateMemory(4);

                            length += 4;
                            vilen += 4;
                            vvlen += 4;
                            vvvlen += 4;

                            put_16(be, vlc->language, vvsd->data);
                            put_16(be, vlc->charset, vvsd->data + 2);

                            vvsd->link = NULL;
                            *pp = vvsd;
                            pp = &vvsd->link;
                        }

                        put_16(be, vvlen, vvd->data);
                        put_16(be, vvvlen, vvd->data + 2);

                    }
                }
                break;
        }

        put_16(be, vilen, vid->data);
    }

    put_16(be, length, first->data);
    *pp = NULL;
    return first;
}

//-------------------------------------------------------------------------

BINDATA *convert_from_internal(RES_RES *res, int be)
{
    switch (res->type)
    {
        default:
            abort();
        case RST_BITMAP:
        case RST_FONT:
        case RST_ICON:
        case RST_MESSAGETABLE:
        case RST_DLGINCLUDE:
            return from_generic(res->u.data.length, res->u.data.data);
        case RST_ACCELERATOR:
            return from_accelerator(res->u.acc, be);
        case RST_CURSOR:
            return from_cursor(res->u.cursor, be);
        case RST_GROUP_CURSOR:
            return from_group_cursor(res->u.group_cursor, be);
        case RST_DIALOG:
            return from_dialog(res->u.dialog, be);
        case RST_FONTDIR:
            return from_fontdir(res->u.fontdir, be);
        case RST_GROUP_ICON:
            return from_group_icon(res->u.group_icon, be);
        case RST_MENU:
            return from_menu(res->u.menu, be);
        case RST_RCDATA:
            return from_rcdata(res->u.rcdata, be);
        case RST_STRINGTABLE:
            return from_stringtable(res->u.stringtable, be);
        case RST_USERDATA:
            return from_rcdata(res->u.rcdata, be);
        case RST_VERSIONINFO:
            return from_versioninfo(res->u.versioninfo, be);
    }
}
