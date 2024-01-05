/* 
CC386 C Compiler
Copyright 1994-2011 David Lindauer.

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

This program is derived from the cc68k complier by 
Matthew Brandt (mailto::mattb@walkingdog.net) 

You may contact the author of this derivative at:

mailto::camille@bluegrass.net
 */
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include "lists.h"
#include "expr.h"
#include "c.h"
#include "gen68.h"
#include "diag.h"
#include "lists.h"

#define FULLVERSION

extern struct slit *strtab;
extern ASMNAME oplst[];
extern int global_flag;
extern enum e_sg curseg;
extern SYM *currentfunc;
extern int prm_asmfile;
extern DBGBLOCK *DbgBlocks[];
extern int prm_debug;
extern int prm_68020;
extern int prm_smalldata;

#define PUTWORD(p,x) { tempstore = (x) ; \
*(*(p))++=(((tempstore)>>8)&255); \
*(*(p))++=((tempstore)&255); }

static int fpcop = 1;
static int tempstore; // used in PUTWORD

int outcode_base_address;

EMIT_TAB segs[MAX_SEGS];
LINEBUF *linelist,  *linetail;
VIRTUAL_LIST *virtualFirst,  *virtualLast;


static LABEL **labelbuf;
void outcode_file_init(void)
{
    int i;
    labelbuf = xalloc(400 *sizeof(LABEL*));

    for (i = 0; i < MAX_SEGS; i++)
        memset(&segs[i], 0, sizeof(segs[i]));
    linelist = 0;
    virtualFirst = virtualLast = 0;

} void outcode_func_init(void)
{
    int i;
    //   memset(labelbuf,0,400 * sizeof(LABEL *)) ;

    for (i = 0; i < MAX_SEGS; i++)
        segs[i].curbase = segs[i].curlast;
    if (currentfunc)
    if (curseg == virtseg)
    {
        currentfunc->mainsym->offset = outcode_base_address = 0;
    }
    else
        currentfunc->mainsym->offset = outcode_base_address =
            segs[codeseg].curbase;
}

//-------------------------------------------------------------------------

void InsertLabel(int lbl, int address, int seg)
{
    int x = lbl / 512;
    if (!labelbuf[x])
    {
        global_flag++;
        labelbuf[x] = xalloc(512 *sizeof(LABEL));
        global_flag--;
    }
    labelbuf[x][lbl % 512].address = address;
    labelbuf[x][lbl % 512].seg = seg;
}

//-------------------------------------------------------------------------

int LabelAddress(int lbl)
{
    if (labelbuf)
    {
        LABEL *p = labelbuf[lbl / 512];
        if (p)
            return p[lbl % 512].address;
    }
    return  - 1;
    //   return labelbuf[lbl/512][lbl%512].address ;
}

//-------------------------------------------------------------------------

int LabelSeg(int lbl)
{
    return labelbuf[lbl / 512][lbl % 512].seg;
}

//-------------------------------------------------------------------------

void InsertLine(int address, int line, int file)
{
    LINEBUF *l;
    global_flag++;
    l = xalloc(sizeof(LINEBUF));
    global_flag--;
    l->address = address;
    l->lineno = line;
    l->file = file;
    if (linelist)
        linetail = linetail->next = l;
    else
        linelist = linetail = l;
}

//-------------------------------------------------------------------------

EMIT_TAB *gettab(int seg)
{
    if (seg == virtseg)
        return virtualLast->seg;
    else
    {
        EMIT_TAB *rv = &segs[seg];
        if (!rv->first)
        {
            global_flag++;
            rv->first = rv->last = xalloc(sizeof(EMIT_LIST));
            global_flag--;
        }
        return rv;
    }
}

//-------------------------------------------------------------------------

void emit(int seg, unsigned char *data, int len)
{
    EMIT_TAB *tab = gettab(seg);
    int ofs = 0;
    if (!tab->first)
    {
        global_flag++;
        tab->first = tab->last = xalloc(sizeof(EMIT_LIST));
        global_flag--;
    }
    tab->last->lastfilled = tab->last->filled;
    tab->curlast += len;
    while (len)
    {
        int size = len >= 1024 ? 1024 : len;
        if (tab->last->filled + len > 1024)
        {
            int address = tab->last->address + tab->last->filled;
            global_flag++;
            tab->last = tab->last->next = xalloc(sizeof(EMIT_LIST));
            tab->last->address = address;
            global_flag++;
        }

        memcpy(tab->last->data + tab->last->filled, data + ofs, size);
        tab->last->filled += size;
        len -= size;
        ofs += size;
    }
}

//-------------------------------------------------------------------------

void reverseemit(int seg, unsigned char *data, int len)
{
    char buf[256];
    int i;
    for (i = 0; i < len; i++)
        buf[len - i - 1] = data[i];
    emit(seg, buf, len);
}

//-------------------------------------------------------------------------

void write_to_seg(int seg, int offset, char *value, int len)
{
    EMIT_TAB *tab = gettab(seg);
    EMIT_LIST *lst = tab->first;
    while (lst->address + lst->filled < offset)
        lst = lst->next;
    memcpy(lst->data + offset - lst->address, value, len);
}

//-------------------------------------------------------------------------

void gen_symbol_fixup(enum mode xmode, int seg, int address, SYM *pub, int size,
    int ofs)
{
    FIXUP *fixup;
    EMIT_TAB *tab = gettab(seg);
    global_flag++;
    fixup = xalloc(sizeof(FIXUP));
    global_flag--;
    fixup->fmode = xmode;
    fixup->address = address;
    fixup->size = size;
    fixup->ofs = ofs;
    fixup->sym = pub;
    if (tab->last->fixups)
        tab->last->lastfixup = tab->last->lastfixup->next = fixup;
    else
        tab->last->fixups = tab->last->lastfixup = fixup;

} void gen_label_fixup(enum mode xmode, int seg, int address, int lab, int size,
    int ofs)
{
    FIXUP *fixup;
    EMIT_TAB *tab = gettab(seg);
    global_flag++;
    fixup = xalloc(sizeof(FIXUP));
    global_flag--;
    fixup->fmode = xmode;
    fixup->address = address;
    fixup->size = size;
    fixup->ofs = ofs;
    fixup->label = lab;
    if (tab->last->fixups)
        tab->last->lastfixup = tab->last->lastfixup->next = fixup;
    else
        tab->last->fixups = tab->last->lastfixup = fixup;

} void outcode_dump_muldivval(void){}
void outcode_dump_browsedata(unsigned char *buf, int len)
{
    emit(browseseg, buf, len);
}

//-------------------------------------------------------------------------

void outcode_dumplits(void)
{
    struct slit *v = strtab;
    int val = 0;
    xstringseg();
    while (v != 0)
    {
        InsertLabel(v->label, segs[stringseg].curlast, stringseg);
        emit(curseg, v->str, v->len - 1);

        if (v->type)
            emit(curseg, (unsigned char*) &val, 2);
        else
		{
            emit(curseg, (unsigned char*) &val, 1);
			switch(v->len & 3)
			{
				case 1:
		            emit(curseg, (unsigned char*) &val, 3);
					break;
				case 2:
		            emit(curseg, (unsigned char*) &val, 2);
					break;
				case 3:
		            emit(curseg, (unsigned char*) &val, 1);
					break;
			}
		}
        v = v->next;
    } nl();
}

//-------------------------------------------------------------------------

void outcode_genref(SYM *sp, int offset)
{
    EMIT_TAB *seg = gettab(curseg);
    reverseemit(curseg, &offset, 4);

    gen_symbol_fixup(fm_symbol, curseg, seg->last->address + seg->last
        ->lastfilled, sp, 4, offset);
}

//-------------------------------------------------------------------------

void outcode_gen_labref(int n)
{
    int i = 0;
    EMIT_TAB *seg = gettab(curseg);
    emit(curseg, &i, 4);
    gen_label_fixup(fm_label, curseg, seg->last->address + seg->last
        ->lastfilled, n, 4, 0);
}

/* the labels will already be resolved well enough by this point */
void outcode_gen_labdifref(int n1, int n2)
{
    int i = LabelAddress(n1) - LabelAddress(n2);
    EMIT_TAB *seg = gettab(curseg);
    emit(curseg, &i, 4);
}

//-------------------------------------------------------------------------

void outcode_gensrref(SYM *sp, int val)
{
    int vv = 0;
    EMIT_TAB *seg = gettab(curseg);
    emit(curseg, &vv, 4);
    gen_symbol_fixup(fm_symbol, curseg, seg->last->address + seg->last
        ->lastfilled, sp, 4, 0);
    reverseemit(curseg, &val, 4);
}

//-------------------------------------------------------------------------

void outcode_genstorage(int len)
{
    char buf[256];
    memset(buf, 0, 256);
    while (len >= 256)
    {
        emit(curseg, buf, 256);
        len -= 256;
    }
    if (len)
        emit(curseg, buf, len);
}

//-------------------------------------------------------------------------

void outcode_genfloat(FPF *val)
{
	char buf[512];
	FPFToFloat(buf, val);
    reverseemit(curseg, buf, 4);
}

//-------------------------------------------------------------------------

void outcode_gendouble(FPF *val)
{
	char buf[512];
	FPFToDouble(buf, val);
    reverseemit(curseg, buf, 8);
}

//-------------------------------------------------------------------------

void outcode_genlongdouble(FPF *val)
{
	char buf[512];
	char ddd[2];
	memset(ddd, 0, sizeof(ddd));
	FPFToLongDouble(buf, val);
    reverseemit(curseg, buf+8, 2);
    reverseemit(curseg, ddd, 2);
    reverseemit(curseg, buf, 8);
}

//-------------------------------------------------------------------------

void outcode_genstring(char *string, int len)
{
    emit(curseg, string, len);
}

//-------------------------------------------------------------------------

void outcode_genbyte(int val)
{
    emit(curseg, &val, 1);
}

//-------------------------------------------------------------------------

void outcode_genword(int val)
{
    reverseemit(curseg, &val, 2);
}

//-------------------------------------------------------------------------

void outcode_genlong(int val)
{
    reverseemit(curseg, &val, 4);
}

//-------------------------------------------------------------------------

void outcode_genlonglong(LLONG_TYPE val)
{
    int vv = val;
    #ifdef BCC32
        val = val < 0 ?  - 1: 0;
    #else 
        val = val >> 32;
    #endif 
    reverseemit(curseg, &val, 4);
    reverseemit(curseg, &vv, 4);
}

//-------------------------------------------------------------------------

void outcode_align(int size)
{
    EMIT_TAB *seg = gettab(curseg);
    int adr = seg->last->address + seg->last->filled;
    adr = size - adr % size;
    if (size != adr)
        outcode_genstorage(adr);
}

//-------------------------------------------------------------------------

void outcode_put_label(int lab)
{
    EMIT_TAB *seg = gettab(curseg);
    InsertLabel(lab, seg->last->address + seg->last->filled, curseg);
}

///////////////////////////////////////////////////

static int putbased(OCODE *ins, AMODE *data, int reg, unsigned char **p)
{
    long val;
    int resolved = 1;

    val = ResolveOffset(ins, data->offset, &resolved);
    if (!resolved)
    {
        if (prm_68020)
        {
            PUTWORD(p, 0x130 + (data->sreg << 12) + (data->mode ==
                am_baseindxdata ? 0 : 0x8000) + (data->scale << 9) + (data->sz 
                == 4 ? 0x800 : 0));
            ins->ofs[ins->rescount] = val;
            ins->resobyte[ins->rescount] = data == ins->oper1 ? 4 : 20;
            ins->addroffset[ins->rescount++] =  *p - ins->outbuf;
            PUTWORD(p, val >> 16);
            PUTWORD(p, val &0xffff);
        }
        else
        {
            DIAG("putbased - Offset too large for 68000");
        }
    }
    else
    {
        if (val <  - 0x80 || val > 0x7f)
        {
            if (prm_68020)
            {
                if (val <  - 0x8000 || val > 0x7fff)
                {
                    PUTWORD(p, 0x130 + (data->sreg << 12) + (data->mode ==
                        am_baseindxdata ? 0 : 0x8000) + (data->scale << 9) + 
                        (data->sz == 4 ? 0x800 : 0));
                    PUTWORD(p, val >> 16);
                    PUTWORD(p, val &0xffff);
                }
                else
                {
                    PUTWORD(p, 0x120 + (data->sreg << 12) + (data->mode ==
                        am_baseindxdata ? 0 : 0x8000) + (data->scale << 9) + 
                        (data->sz == 4 ? 0x800 : 0));
                    PUTWORD(p, val &0xffff);
                }
            }
            else
                DIAG("Offset too large for 68000");
        }
        else
        {
            PUTWORD(p, (data->sreg << 12) + (data->mode == am_baseindxdata ? 0 
                : 0x8000) + (val &0xff) + (data->scale << 9) + (data->sz == 4 ?
                0x800 : 0));
        }
    }
    return data->preg + 0x30;
}

//-------------------------------------------------------------------------

static int putpcbased(OCODE *ins, AMODE *data, unsigned char **p)
{
    long val;
    int resolved = 1;
    val = ResolveOffset(ins, data->offset, &resolved);
    if (!resolved)
    {
        if (prm_68020)
        {
            val += 2;
            PUTWORD(p, 0x130 + (data->sreg << 12) + (data->mode ==
                am_pcindxdata ? 0 : 0x8000) + (data->scale << 9) + (data->sz ==
                4 ? 0x800 : 0));
            ins->ofs[ins->rescount] = val;
            ins->pcrelofs = (*p) - ins->outbuf;
            ins->pcrelres = ins->rescount;
            ins->resobyte[ins->rescount] = data == ins->oper1 ? 4 : 20;
            ins->addroffset[ins->rescount++] =  *p - ins->outbuf;
            ins->reloffset = data;
            ins->branched = BR_PCREL;
            PUTWORD(p, val >> 16);
            PUTWORD(p, val &0xffff);

        }
        else
        {
            DIAG("putpcbased - Offset too large for 68000");
        }
    }
    else
    {
        if (val)
            DIAG("putpcbased PC rel address to non-label");
        PUTWORD(p, (data->sreg << 12) + (data->mode == am_pcindxdata ? 0 :
            0x8000) + (data->scale << 9) + (data->sz == 4 ? 0x800 : 0));
    }
    return 0x3b;
}

//-------------------------------------------------------------------------

static int putaddr(int size, OCODE *ins, AMODE *data, unsigned char **p)
{
    int rv;
    long val;
    int resolved = 1;
    val = ResolveOffset(ins, data->offset, &resolved);
    if (!resolved)
    {
        if (prm_68020)
        {
            int ew = 0x170;
            PUTWORD(p, 0x170);
            ins->ofs[ins->rescount] = val;
            ins->resobyte[ins->rescount] = data == ins->oper1 ? 4 : 20;
            ins->addroffset[ins->rescount++] =  *p - ins->outbuf;
            PUTWORD(p, val >> 16);
            PUTWORD(p, val &0xffff);
            rv = data->preg + 0x30;
        }
        else
        {
            ins->ofs[ins->rescount] = val;
            ins->resobyte[ins->rescount] = data == ins->oper1 ? 2 : 18;
            ins->addroffset[ins->rescount++] =  *p - ins->outbuf;
            PUTWORD(p, val &0xffff);
            rv = data->preg + 0x28;
        }
    }
    else
    {
        if (val <  - 0x8000 || val > 0x7fff)
        {
            if (prm_68020)
            {
                PUTWORD(p, 0x170);
                PUTWORD(p, val >> 16)PUTWORD(p, val &0xffff);
                rv = data->preg + 0x30;
            }
            else
                DIAG("putaddr - Offset too large for 68000");
        }
        else
        {
            PUTWORD(p, val &0xffff);
            rv = data->preg + 0x28;
        }
    }
    return rv;
}

//-------------------------------------------------------------------------

static int putpcaddr(int size, OCODE *ins, AMODE *data, unsigned char **p)
{
    int rv;
    long val;
    int resolved = 1;
    val = ResolveOffset(ins, data->offset, &resolved);
    if (!resolved)
    {
        if (prm_68020)
        {
            int ew = 0x170;
            val += 2;
            PUTWORD(p, ew);
            ins->ofs[ins->rescount] = val;
            ins->pcrelofs = (*p) - ins->outbuf;
            ins->pcrelres = ins->rescount;
            ins->resobyte[ins->rescount] = data == ins->oper1 ? 4 : 20;
            ins->addroffset[ins->rescount++] =  *p - ins->outbuf;
            ins->reloffset = data;
            ins->branched = BR_PCREL;
            PUTWORD(p, val >> 16);
            PUTWORD(p, val &0xffff);
            rv = data->preg + 0x3b;

        }
        else
        {
            ins->ofs[ins->rescount] = val;
            ins->pcrelofs = (*p) - ins->outbuf;
            ins->pcrelres = ins->rescount;
            ins->resobyte[ins->rescount] = data == ins->oper1 ? 2 : 18;
            ins->addroffset[ins->rescount++] =  *p - ins->outbuf;
            ins->reloffset = data;
            ins->branched = BR_PCREL;
            PUTWORD(p, val &0xffff);
            rv = data->preg + 0x3A;
        }
    }
    else
    {
        DIAG("PC rel address to non-label");
        PUTWORD(p, 0);
    }
    return rv;
}

//-------------------------------------------------------------------------

static int putdirect(int size, OCODE *ins, AMODE *data, unsigned char **p)
{
    int resolved = 1;
    int adr = 0;
    adr = ResolveOffset(ins, data->offset, &resolved);
    if (size == 2)
    {
        if (resolved)
        {
            PUTWORD(p, data->offset->v.i);
        }
        else
        {
            ins->ofs[ins->rescount] = adr;
            ins->resobyte[ins->rescount] = data == ins->oper1 ? 2 : 18;
            ins->addroffset[ins->rescount++] =  *p - ins->outbuf;
            PUTWORD(p, adr);
        }
        return 0x38;
    }
    else
    {
        if (resolved)
        {
            PUTWORD(p, data->offset->v.i >> 16);
            PUTWORD(p, data->offset->v.i &0xffff);
        }
        else
        {
            ins->ofs[ins->rescount] = adr;
            ins->resobyte[ins->rescount] = data == ins->oper1 ? 4 : 20;
            ins->addroffset[ins->rescount++] =  *p - ins->outbuf;
            PUTWORD(p, adr >> 16);
            PUTWORD(p, adr);
        }
        return 0x39;
    }
}

//-------------------------------------------------------------------------

static void putam(int size, OCODE *ins, AMODE *data, unsigned char **p)
{
    int resolved = 1;
    int adr = 0;
    switch (data->mode)
    {
        case am_dreg:
            ins->outbuf[1] |= data->preg;
            break;
        case am_areg:
            ins->outbuf[1] |= 8+data->preg;
            break;
        case am_immed:
            ins->outbuf[1] |= 0x3c;
            switch (size)
            {
            case 1:
                adr = ResolveOffset(ins, data->offset, &resolved);
                if (resolved)
                {
                    PUTWORD(p, data->offset->v.i &0xff);
                }
                else
                {
                    ins->ofs[ins->rescount] = adr;
                    ins->resobyte[ins->rescount] = data == ins->oper1 ? 2 : 18;
                    ins->addroffset[ins->rescount++] =  *p - ins->outbuf;
                    PUTWORD(p, adr);
                }
                break;
            case 2:
                adr = ResolveOffset(ins, data->offset, &resolved);
                if (resolved)
                {
                    PUTWORD(p, data->offset->v.i &0xffff);
                }
                else
                {
                    ins->ofs[ins->rescount] = adr;
                    ins->resobyte[ins->rescount] = data == ins->oper1 ? 2 : 18;
                    ins->addroffset[ins->rescount++] =  *p - ins->outbuf;
                    PUTWORD(p, adr);
                }
                break;
            case 4:
                adr = ResolveOffset(ins, data->offset, &resolved);
                if (resolved)
                {
                    PUTWORD(p, data->offset->v.i >> 16);
                    PUTWORD(p, data->offset->v.i &65535);
                }
                else
                {
                    ins->ofs[ins->rescount] = adr;
                    ins->resobyte[ins->rescount] = data == ins->oper1 ? 4 : 20;
                    ins->addroffset[ins->rescount++] =  *p - ins->outbuf;
                    PUTWORD(p, adr >> 16);
                    PUTWORD(p, adr &0xffff);
                }
                break;
            case 7:
                {
					unsigned char buf[4];
					int i;
					FPFToFloat(buf, &data->offset->v.f);
					for (i=3; i >=0; i--)
						*(*p)++ = buf[i];
                }
                break;
            case 8:
                {
					unsigned char buf[8];
					int i;
					FPFToDouble(buf, &data->offset->v.f);
					for (i=7; i >=0; i--)
						*(*p)++ = buf[i];
                }
                break;
            case 10:
                {
					unsigned char buf[8];
					int i;
					FPFToLongDouble(buf, &data->offset->v.f);
					for (i=9; i >=8; i--)
						*(*p)++ = buf[i];
					*(*p)++ = 0;
					*(*p)++ = 0;
					for (i=7; i >=0; i--)
						*(*p)++ = buf[i];
                }
                break;
            }
            break;
        case am_direct:
        case am_adirect:
            ins->outbuf[1] |= putdirect(data->preg, ins, data, p);
            break;
        case am_pcindxdata:
        case am_pcindxaddr:
            ins->outbuf[1] |= putpcbased(ins, data, p);
            break;
        case am_pcindx:
            ins->outbuf[1] |= putpcaddr(size, ins, data, p);
            break;
        case am_ind:
            ins->outbuf[1] |= 0x10 + data->preg;
            break;
        case am_ainc:
            ins->outbuf[1] |= 0x18 + data->preg;
            break;
        case am_adec:
            ins->outbuf[1] |= 0x20 + data->preg;
            break;
        case am_indx:
            ins->outbuf[1] |= putaddr(size, ins, data, p);
            break;
        case am_baseindxdata:
            ins->outbuf[1] |= putbased(ins, data, 0, p);
            break;
        case am_baseindxaddr:
            ins->outbuf[1] |= putbased(ins, data, 1, p);
            break;
        default:
            DIAG("putam - Unknown address mode");
    }
}

//-------------------------------------------------------------------------

static void putam2(int size, OCODE *ins, AMODE *data, unsigned char **p)
{
    short val, val1;
    int vv = ins->outbuf[1] &0x3f;
    ins->outbuf[1] &= 0xc0;
    putam(size, ins, data, p);
    val1 = ins->outbuf[1];
    val = val1 + (ins->outbuf[0] << 8);
    val |= (val1 &0x38) << 3;
    val |= (val1 &0x7) << 9;
    val &= 0xffc0;
    ins->outbuf[0] = val >> 8;
    ins->outbuf[1] = val | vv;
}

//-------------------------------------------------------------------------

static void putbranch(int val, OCODE *ins, unsigned char **p)
{
    int size;
    int resolved = 1;
    long addr;
    switch (size = ins->length)
    {
        case 1:
            ins->branched = BR_BYTE;
            break;
        case 2:
            ins->branched = BR_SHORT;
            break;
        case 4:
            if (prm_68020)
            {
                ins->branched = BR_LONG;
                val |= 0xff;
            }
            else
                DIAG("putbranch - Long branch for 68000");
            break;
        default:
            size = 2;
            if (prm_68020)
            {
                val |= 0xff;
                size = 4;
                ins->branched = BR_LONG;
            }
            else
                ins->branched = BR_SHORT;
            break;
    }
    addr = ResolveOffset(ins, ins->oper1->offset, &resolved);
    if (size == 1)
    {
        PUTWORD(p, val + (addr &0xff));
    }
    else
        PUTWORD(p, val);
    ins->ofs[ins->rescount] = addr;
    ins->reloffset = ins->oper1;
    ins->resobyte[ins->rescount] = size;
    ins->pcrelofs = (*p) - ins->outbuf;
    ins->pcrelres = ins->rescount;
    ins->addroffset[ins->rescount++] =  *p - ins->outbuf;
    if (size != 1)
    {
        if (size == 4)
		{
            PUTWORD(p, addr >> 16);
		}
        PUTWORD(p, addr);
    }
}

//-------------------------------------------------------------------------

static void putdbranch(int val, OCODE *ins, unsigned char **p)
{
    long addr;
    int resolved = 1;
    ins->branched = BR_SHORT | BR_DBRA;
    PUTWORD(p, val + 0xc8 + ins->oper1->preg);
    addr = ResolveOffset(ins, ins->oper2->offset, &resolved);
    ins->ofs[ins->rescount] = addr;
    ins->pcrelofs = 2;
    ins->reloffset = ins->oper2;
    ins->resobyte[ins->rescount] = 18;
    ins->addroffset[ins->rescount++] =  *p - ins->outbuf;
    PUTWORD(p, addr);
}

//-------------------------------------------------------------------------

static void putfbranch(int val, OCODE *ins, unsigned char **p)
{
    int size = 2;
    int resolved = 1;
    long addr;
    val = val + 0xf080 + (fpcop << 9);
    if (prm_68020)
    {
        val |= 0x40;
        size = 4;
        ins->branched = BR_LONG | BR_FLOAT;
    }
    else
        ins->branched = BR_SHORT | BR_FLOAT;
    PUTWORD(p, val);
    addr = ResolveOffset(ins, ins->oper1->offset, &resolved);
    ins->ofs[ins->rescount] = addr;
    ins->pcrelofs = 2;
    ins->reloffset = ins->oper1;
    ins->resobyte[ins->rescount] = size;
    ins->addroffset[ins->rescount++] =  *p - ins->outbuf;
    if (size == 4)
        PUTWORD(p, addr >> 16);
    PUTWORD(p, addr);
}

//-------------------------------------------------------------------------

static void putfdbranch(int val, OCODE *ins, unsigned char **p)
{
    int resolved = 1;
    long addr;
    PUTWORD(p, 0xf048 + (fpcop << 9) + (ins->oper1->preg));
    PUTWORD(p, val);
    addr = ResolveOffset(ins, ins->oper2->offset, &resolved);
    ins->branched = BR_SHORT | BR_FLOAT;
    ins->ofs[ins->rescount] = addr;
    ins->pcrelofs = 4;
    ins->reloffset = ins->oper1;
    ins->resobyte[ins->rescount] = 2;
    ins->addroffset[ins->rescount++] =  *p - ins->outbuf;
    PUTWORD(p, addr);
}

//-------------------------------------------------------------------------

static void putfscc(int val, OCODE *ins, unsigned char **p)
{
    PUTWORD(p, 0xf040 + (fpcop << 9));
    PUTWORD(p, val);
    putam(ins->length, ins, ins->oper1, p);
}

//-------------------------------------------------------------------------

int putftrapcc(int val, OCODE *ins, unsigned char **p)
{
    int dword = FALSE;
    int select = 0xf078 + (fpcop << 9);
    if (!ins->oper1)
        select |= 4;
    else if (ins->oper1->length == 4)
    {
        select |= 3;
        dword = TRUE;
    }
    else
        select |= 2;
    PUTWORD(p, select);
    PUTWORD(p, val);
    if (ins->oper1)
    {
        int resolved = 1;
        int adr = ResolveOffset(ins, ins->oper1->offset, &resolved);
        if (resolved)
        {
            if (dword)
                PUTWORD(p, ins->oper1->offset->v.i >> 16);
            PUTWORD(p, ins->oper1->offset->v.i &0xffff);
        }
        else
        {
            ins->ofs[ins->rescount] = adr;
            ins->resobyte[ins->rescount] = dword ? 4 : 2;
            ins->addroffset[ins->rescount++] =  *p - ins->outbuf;
            if (dword)
                PUTWORD(p, adr >> 16);
            PUTWORD(p, adr &0xffff);
        }

    }
}

//-------------------------------------------------------------------------

static void putmath(int val, OCODE *ins, unsigned char **p)
{
    if (ins->oper2->mode == am_dreg)
    {
        PUTWORD(p, val + (ins->oper2->preg << 9) + ((ins->length / 2) << 6));
        putam(ins->length, ins, ins->oper1, p);
    }
    else
    {
        PUTWORD(p, val + 0x100 + (ins->oper1->preg << 9) + ((ins->length / 2)
            << 6));
        putam(ins->length, ins, ins->oper2, p);
    }
}

//-------------------------------------------------------------------------

static void putmathimm(int val, OCODE *ins, unsigned char **p)
{
    int resolved = 1;
    int adr = 0;
    adr = ResolveOffset(ins, ins->oper1->offset, &resolved);

    PUTWORD(p, val + ((ins->length / 2) << 6));
    switch (ins->length)
    {
        case 1:
            if (resolved)
            {
                PUTWORD(p, ins->oper1->offset->v.i &0xff);
            }
            else
            {
                PUTWORD(p, adr &255);
                ins->ofs[ins->rescount] = adr &255;
                ins->resobyte[ins->rescount] = 2;
                ins->addroffset[ins->rescount++] =  *p - ins->outbuf;
            }
            break;
        case 2:
            if (resolved)
            {
                PUTWORD(p, ins->oper1->offset->v.i);
            }
            else
            {
                PUTWORD(p, adr);
                ins->ofs[ins->rescount] = adr;
                ins->resobyte[ins->rescount] = 2;
                ins->addroffset[ins->rescount++] =  *p - ins->outbuf;
            }
            break;
        case 4:
            if (resolved)
            {
                PUTWORD(p, ins->oper1->offset->v.i >> 16);
                PUTWORD(p, ins->oper1->offset->v.i &65535);
            }
            else
            {
                PUTWORD(p, adr >> 16);
                PUTWORD(p, adr);
                ins->ofs[ins->rescount] = adr;
                ins->resobyte[ins->rescount] = 4;
                ins->addroffset[ins->rescount++] =  *p - ins->outbuf;
            }
            break;
    }
    putam(ins->length, ins, ins->oper2, p);
}

//-------------------------------------------------------------------------

static void putshift(int val, OCODE *ins, unsigned char **p)
{
    if (!ins->oper2)
    {
        val |= 0xc0;
        val |= (val &0x18) << 6;
        val &= ~0x18;
        PUTWORD(p, val);
        putam(ins->length, ins, ins->oper1, p);
    }
    else
    {
        switch (ins->oper1->mode)
        {

            case am_dreg:
                val |= ins->length / 2 << 6;
                val |= 0x20 + (ins->oper1->preg << 9) + ins->oper2->preg;
                PUTWORD(p, val);
                break;
            case am_immed:
                val |= ins->length / 2 << 6;
                val |= ((ins->oper1->offset->v.i &7) << 9) + ins->oper2->preg;
                PUTWORD(p, val);
                break;
        }
    }
}

//-------------------------------------------------------------------------

static void putmuldiv(int longv, int wval, int lval, int ltrail, OCODE *ins,
    unsigned char **p)
{
    int temp;
    if (longv)
    {
        /* divsl & divul */
        PUTWORD(p, 0x4c40);
        if (ins->oper2->mode == am_divsl)
        {
            PUTWORD(p, ltrail | ins->oper2->preg | (ins->oper2->sreg << 12));
        }
        else
            PUTWORD(p, ltrail | ins->oper2->preg | (ins->oper2->preg << 12));
        putam(ins->length, ins, ins->oper1, p);
    }
    else
    {
        if (ins->length == 2)
        {
            PUTWORD(p, wval | (ins->oper2->preg << 9));
            putam(ins->length, ins, ins->oper1, p);
        }
        else
        {
            PUTWORD(p, lval);
            if (ins->oper2->mode == am_divsl)
                temp = 0x400 | ltrail | ins->oper2->preg | (ins->oper2->sreg <<
                    12);
            else
                temp = ltrail | ins->oper2->preg | (ins->oper2->preg << 12);
            PUTWORD(p, temp)putam(ins->length, ins, ins->oper1, p);
        }
    }
}

//-------------------------------------------------------------------------

static void putsbit(int reg, int immed, OCODE *ins, unsigned char **p)
{
    if (ins->oper1->mode == am_immed)
    {
        PUTWORD(p, immed);
        PUTWORD(p, ins->oper1->offset->v.i);
    }
    else
        PUTWORD(p, reg | (ins->oper1->preg << 9));
    putam(ins->length, ins, ins->oper2, p);
}

//-------------------------------------------------------------------------

static void putfbit(int val, OCODE *ins, AMODE *reg, AMODE *data, AMODE *bf,
    unsigned char **p)
{
    PUTWORD(p, val);
    if (reg)
        val = reg->preg << 12;
    else
        val = 0;
    if (bf->mode == am_bf)
    {
        val += bf->preg << 6;
        val += bf->sreg;
    }
    else
    {
        val += bf->preg << 6;
        val += bf->sreg;
        val += 0x0820;
    }
    PUTWORD(p, val);
    putam(4, ins, data, p);
}

//-------------------------------------------------------------------------

static int fsize(int size)
{
    int val;
    switch (size)
    {
        case 1:
            val = 6 << 10;
            break;
        case 2:
            val = 4 << 10;
            break;
        case 4:
            val = 0 << 10;
            break;
        case 7:
            val = 1 << 10;
            break;
        case 8:
            val = 5 << 10;
            break;
        case 10:
        default:
            val = 2 << 10;
            break;
    }
    return val;
}

//-------------------------------------------------------------------------

static void putfloat(int val, int size, OCODE *ins, AMODE *o1, AMODE *o2,
    unsigned char **p)
{
    int doam = FALSE;
    PUTWORD(p, 0xf000 + (fpcop << 9));
    if (!o1)
    {
        if (o2->mode == am_freg)
        {
            //        val |= o2->preg << 7 ; 
            val |= o2->preg << 10;
        }
        else
        {
            val |= fsize(size);
            val |= 0x4000;
            doam = TRUE;
        }
    }
    else if (!o2)
    {
        if (o1->mode == am_freg)
        {
            val |= o1->preg << 7;
            val |= o1->preg << 10;
        }
        else
        {
            val |= fsize(size);
            val |= 0x4000;
            doam = TRUE;
        }
    }
    else
    {
        if (o1->mode == am_freg)
        {
            val |= o1->preg << 10;
        }
        else
        {
            val |= fsize(size);
            val |= 0x4000;
            doam = TRUE;
        }
        val |= o2->preg << 7;
    }
    PUTWORD(p, val);
    if (doam)
        putam(size, ins, ins->oper1, p);
}

//-------------------------------------------------------------------------

int putdecimal(int val, OCODE *ins, unsigned char **p)
{
    if (ins->oper1->mode == ins->oper2->mode)
    {
        if (ins->oper1->mode == am_dreg)
        {
            PUTWORD(p, val + ins->oper1->preg + (ins->oper2->preg << 9));
        }
        else if (ins->oper1->mode == am_adec)
            PUTWORD(p, val + 8+ins->oper1->preg + (ins->oper2->preg << 9));
    }
}

//-------------------------------------------------------------------------

int putaddx(int val, OCODE *ins, unsigned char **p)
{
    putdecimal(val, ins, p);
    ins->outbuf[1] |= (ins->length / 2) << 6;
}

//-------------------------------------------------------------------------

int puttrapcc(OCODE *ins, int val, unsigned char **p)
{
    int dword = FALSE;
    if (!ins->oper1)
        val |= 4;
    else if (ins->oper1->length == 4)
    {
        val |= 3;
        dword = TRUE;
    }
    else
        val |= 2;
    PUTWORD(p, val);
    if (ins->oper1)
    {
        int resolved = 1;
        int adr = ResolveOffset(ins, ins->oper1->offset, &resolved);
        if (resolved)
        {
            if (dword)
                PUTWORD(p, ins->oper1->offset->v.i >> 16);
            PUTWORD(p, ins->oper1->offset->v.i &0xffff);
        }
        else
        {
            ins->ofs[ins->rescount] = adr;
            ins->resobyte[ins->rescount] = dword ? 4 : 2;
            ins->addroffset[ins->rescount++] =  *p - ins->outbuf;
            if (dword)
                PUTWORD(p, adr >> 16);
            PUTWORD(p, adr &0xffff);
        }

    }
}

//-------------------------------------------------------------------------

int putpack(int val, OCODE *ins, unsigned char **p)
{
    val |= ins->oper1->preg;
    val |= ins->oper2->preg << 9;
    if (ins->oper1->mode != am_dreg)
        val |= 8;
    PUTWORD(p, val);
    PUTWORD(p, ins->oper3->offset->v.i);
}

//-------------------------------------------------------------------------

int putfsincos(OCODE *ins, unsigned char **p)
{
    int val = 0x30;
    PUTWORD(p, 0xf000 + (fpcop << 9));
    if (ins->oper1->mode == am_freg)
    {
        val |= ins->oper1->preg << 10;
    }
    else
    {
        val |= fsize(ins->length);
        val |= 0x4000;
    }
    val |= ins->oper2->preg;
    val |= ins->oper3->preg << 7;
    PUTWORD(p, val);
    if (ins->oper1->mode != am_freg)
        putam(ins->length, ins, ins->oper1, p);
}

//-------------------------------------------------------------------------

int revmsk(int msk)
{
    int msk2 = 0;
    int i;
    for (i = 0; i < 16; i++)
    {
        msk2 <<= 1;
        if (msk &1)
            msk2 |= 1;
        msk >>= 1;
    }
    return msk2;
}

//-------------------------------------------------------------------------

int revmskf(int msk)
{
    int msk2 = 0;
    int i;
    for (i = 0; i < 8; i++)
    {
        msk2 <<= 1;
        if (msk &1)
            msk2 |= 1;
        msk >>= 1;
    }
    return msk2;
}

//-------------------------------------------------------------------------

int outcode_putop(OCODE *ins)
{
    int i, len;
    char *pos = ins->outbuf;
    char **p = &pos;
    switch (ins->opcode)
    {
        case op_abcd:
            putdecimal(0xc100, ins, p);
            break;
        case op_add:
            putmath(0xd000, ins, p);
            break;
        case op_adda:
            PUTWORD(p, 0xD0C0 + (ins->oper2->preg << 9) + ((ins->length == 4 ?
                1 : 0) << 8));
            putam(ins->length, ins, ins->oper1, p);
            break;
        case op_addi:
            putmathimm(0x600, ins, p);
            break;
        case op_addq:
            PUTWORD(p, 0x5000 + ((ins->length / 2) << 6) + (((ins->oper1
                ->offset->v.i) &7) << 9));
            putam(ins->length, ins, ins->oper2, p);
            break;
        case op_addx:
            putaddx(0xd100, ins, p);
            break;
        case op_and:
            putmath(0xc000, ins, p);
            break;
        case op_andi:
            if (ins->oper2->mode == am_sr)
            {
                PUTWORD(p, 0x27c)PUTWORD(p, ins->oper1->offset->v.i);
            }
            else if (ins->oper2->mode == am_ccr)
            {
                PUTWORD(p, 0x23c)PUTWORD(p, ins->oper1->offset->v.i &255);
            }
            else
                putmathimm(0x200, ins, p);
            break;
        case op_asl:
            putshift(0xe100, ins, p);
            break;
        case op_asr:
            putshift(0xe000, ins, p);
            break;
        case op_bra:
            putbranch(0x6000, ins, p);
            break;
        case op_beq:
            putbranch(0x6700, ins, p);
            break;
        case op_bmi:
            putbranch(0x6b00, ins, p);
            break;
        case op_bpl:
            putbranch(0x6a00, ins, p);
            break;
        case op_bcc:
            putbranch(0x6400, ins, p);
            break;
        case op_bcs:
            putbranch(0x6500, ins, p);
            break;
        case op_bne:
            putbranch(0x6600, ins, p);
            break;
        case op_blt:
            putbranch(0x6d00, ins, p);
            break;
        case op_ble:
            putbranch(0x6f00, ins, p);
            break;
        case op_bgt:
            putbranch(0x6e00, ins, p);
            break;
        case op_bge:
            putbranch(0x6c00, ins, p);
            break;
        case op_bhi:
            putbranch(0x6200, ins, p);
            break;
        case op_bhs:
            putbranch(0x6400, ins, p);
            break;
        case op_blo:
            putbranch(0x6500, ins, p);
            break;
        case op_bls:
            putbranch(0x6300, ins, p);
            break;
        case op_bvc:
            putbranch(0x6800, ins, p);
            break;
        case op_bvs:
            putbranch(0x6900, ins, p);
            break;
        case op_bchg:
            putsbit(0x0140, 0x0840, ins, p);
            break;
        case op_bclr:
            putsbit(0x0180, 0x0880, ins, p);
            break;
        case op_bfclr:
            putfbit(0xecc0, ins, 0, ins->oper1, ins->oper2, p);
            break;
        case op_bfchg:
            putfbit(0xeac0, ins, 0, ins->oper1, ins->oper2, p);
            break;
        case op_bfexts:
            putfbit(0xebc0, ins, ins->oper3, ins->oper1, ins->oper2, p);
            break;
        case op_bfextu:
            putfbit(0xe9c0, ins, ins->oper3, ins->oper1, ins->oper2, p);
            break;
        case op_bfffo:
            putfbit(0xedc0, ins, ins->oper3, ins->oper1, ins->oper2, p);
            break;
        case op_bfins:
            putfbit(0xefc0, ins, ins->oper1, ins->oper2, ins->oper3, p);
            break;
        case op_bfset:
            putfbit(0xeec0, ins, 0, ins->oper1, ins->oper2, p);
            break;
        case op_bftst:
            putfbit(0xe8c0, ins, 0, ins->oper1, ins->oper2, p);
            break;
        case op_bkpt:
            PUTWORD(p, 0x4848 + ins->oper1->offset->v.i);
            break;
        case op_bset:
            putsbit(0x01c0, 0x08c0, ins, p);
            break;
        case op_bsr:
            putbranch(0x6100, ins, p);
            break;
        case op_btst:
            putsbit(0x0100, 0x0800, ins, p);
            break;
        case op_chk:
            PUTWORD(p, 0x4000 + (ins->oper2->preg << 9) + (ins->length == 2 ?
                0x180 : 0x100));
            putam(ins->length, ins, ins->oper1, p);
            break;
        case op_chk2:
            PUTWORD(p, 0xc0 + (ins->length / 2 << 9));
            if (ins->oper2->mode == am_areg)
            {
                PUTWORD(p, 0x8800 + (ins->oper2->preg << 12));
            }
            else
                PUTWORD(p, 0x0800 + (ins->oper2->preg << 12));
            putam(ins->length, ins, ins->oper1, p);
            break;
        case op_clr:
            PUTWORD(p, 0x4200 + (ins->length / 2 << 6));
            putam(ins->length, ins, ins->oper1, p);
            break;
        case op_cmp:
            /* warning: DX expect as second operand */
            putmath(0xb000, ins, p);
            break;
        case op_cmpa:
            PUTWORD(p, 0xB0C0 + (ins->oper2->preg << 9) + ((ins->length == 4 ?
                1 : 0) << 8));
            putam(ins->length, ins, ins->oper1, p);
            break;
        case op_cmpi:
            putmathimm(0xc00, ins, p);
            break;
        case op_cmpm:
            PUTWORD(p, 0xb108 + (ins->oper1->preg) + (ins->oper2->preg << 9) + 
                (ins->length / 2 << 6));
            break;
        case op_cmp2:
            PUTWORD(p, 0xc0 + ((ins->length / 2) << 9));
            PUTWORD(p, (ins->oper2->preg << 12) + (ins->oper2->mode == am_areg 
                ? 0x8000 : 0));
            putam(ins->length, ins, ins->oper1, p);
            break;
        case op_dbra:
            putdbranch(0x5100, ins, p);
            break;
        case op_dbeq:
            putdbranch(0x5700, ins, p);
            break;
        case op_dbpl:
            putdbranch(0x5a00, ins, p);
            break;
        case op_dbmi:
            putdbranch(0x5b00, ins, p);
            break;
        case op_dbcc:
            putdbranch(0x5400, ins, p);
            break;
        case op_dbcs:
            putdbranch(0x5500, ins, p);
            break;
        case op_dbne:
            putdbranch(0x5600, ins, p);
            break;
        case op_dblt:
            putdbranch(0x5d00, ins, p);
            break;
        case op_dble:
            putdbranch(0x5f00, ins, p);
            break;
        case op_dbgt:
            putdbranch(0x5e00, ins, p);
            break;
        case op_dbge:
            putdbranch(0x5c00, ins, p);
            break;
        case op_dbhi:
            putdbranch(0x5200, ins, p);
            break;
        case op_dbhs:
            putdbranch(0x5400, ins, p);
            break;
        case op_dblo:
            putdbranch(0x5500, ins, p);
            break;
        case op_dbls:
            putdbranch(0x5300, ins, p);
            break;
        case op_dbvc:
            putdbranch(0x5800, ins, p);
            break;
        case op_dbvs:
            putdbranch(0x5900, ins, p);
            break;
        case op_divs:
            putmuldiv(0, 0x81c0, 0x4c40, 0x0800, ins, p);
            break;
        case op_divsl:
            putmuldiv(1, 0, 0x4c40, 0x0800, ins, p);
            break;
        case op_divu:
            putmuldiv(0, 0x80c0, 0x4c40, 0x0000, ins, p);
            break;
        case op_divul:
            putmuldiv(1, 0, 0x4c40, 0x0000, ins, p);
            break;
        case op_eor:
            /* Warning: Dx expected as first operand */
            PUTWORD(p, 0xb000 + 0x100 + (ins->oper1->preg << 9) + ((ins->length
                / 2) << 6));
            putam(ins->length, ins, ins->oper2, p);
            break;
        case op_eori:
            if (ins->oper2->mode == am_sr)
            {
                PUTWORD(p, 0xa7c)PUTWORD(p, ins->oper1->offset->v.i);
            }
            else if (ins->oper2->mode == am_ccr)
            {
                PUTWORD(p, 0xa3c)PUTWORD(p, ins->oper1->offset->v.i &255);
            }
            else
                putmathimm(0xa00, ins, p);
            break;
        case op_exg:
            {
                int val;
                AMODE *o1,  *o2;
                if (ins->oper1->mode == am_areg && ins->oper2->mode == am_dreg)
                {
                    o1 = ins->oper2;
                    o2 = ins->oper1;
                }
                else
                {
                    o1 = ins->oper1;
                    o2 = ins->oper2;
                }
                val = 0xC100 + (o1->preg << 9) + o2->preg;
                if (o2->mode == am_areg)
                    if (o1->mode == am_dreg)
                        val |= 0x88;
                    else
                        val |= 0x48;
                    else
                        val |= 0x40;
                PUTWORD(p, val);
            }
            break;
        case op_ext:
            PUTWORD(p, 0x4880 + (ins->length == 4 ? 0x40 : 0) + ins->oper1
                ->preg);
            break;
        case op_extb:
            PUTWORD(p, 0x49C0 + ins->oper1->preg);
            break;
        case op_illegal:
            PUTWORD(p, 0x4afc);
            break;
        case op_jmp:
            PUTWORD(p, 0x4EC0);
            putam(ins->length, ins, ins->oper1, p);
            break;
        case op_jsr:
            PUTWORD(p, 0x4E80);
            putam(ins->length, ins, ins->oper1, p);
            break;
        case op_lea:
            PUTWORD(p, 0x41C0 + (ins->oper2->preg << 9));
            putam(ins->length, ins, ins->oper1, p);
            break;
        case op_link:
            {
                int val = 0x4800;
                if (ins->length == 4)
                    val += 8;
                else
                    val += 0x650;
                PUTWORD(p, val + ins->oper1->preg);
                if (ins->length == 4)
                    PUTWORD(p, ins->oper2->offset->v.i >> 16);
                PUTWORD(p, ins->oper2->offset->v.i &65535);
            }
            break;
        case op_lsl:
            putshift(0xe108, ins, p);
            break;
        case op_lsr:
            putshift(0xe008, ins, p);
            break;


        case op_move:
            if (ins->oper1->mode == am_sr)
            {
                PUTWORD(p, 0x40c0);
                putam(ins->length, ins, ins->oper2, p);
                break;
            }
            else if (ins->oper1->mode == am_ccr)
            {
                PUTWORD(p, 0x42c0);
                putam(ins->length, ins, ins->oper2, p);
                break;
            }
            else if (ins->oper2->mode == am_sr)
            {
                PUTWORD(p, 0x46c0);
                putam(ins->length, ins, ins->oper1, p);
                break;
            }
            else if (ins->oper2->mode == am_ccr)
            {
                PUTWORD(p, 0x44c0);
                putam(ins->length, ins, ins->oper1, p);
                break;
            }
            // fall through
        case op_movea:
            switch (ins->length)
            {
            case 1:
                PUTWORD(p, 0x1000);
                break;
            case 2:
                PUTWORD(p, 0x3000);
                break;
            case 4:
            default:
                PUTWORD(p, 0x2000);
                break;
            }
            putam(ins->length, ins, ins->oper1, p);
            putam2(ins->length, ins, ins->oper2, p);
            break;
        case op_movem:
            if (ins->oper1->mode == am_mask)
            {
                PUTWORD(p, 0x4880 + (ins->length / 4 << 6));
                PUTWORD(p, revmsk((int)ins->oper1->offset));
                putam(ins->length, ins, ins->oper2, p);
            }
            else
            {
                PUTWORD(p, 0x4c80 + (ins->length / 4 << 6));
                PUTWORD(p, (int)ins->oper2->offset);
                putam(ins->length, ins, ins->oper1, p);
            }
            break;
        case op_movep:
            {
                int val;
                if (ins->oper1->mode == am_dreg)
                {
                    val = 0x188 + (ins->oper1->preg << 9);
                    if (ins->length == 4)
                        val |= 0x40;
                    val |= ins->oper2->preg;
                    PUTWORD(p, val);
                    PUTWORD(p, ins->oper2->offset->v.i);
                }
                else
                {
                    val = 0x108 + (ins->oper2->preg << 9);
                    if (ins->length == 4)
                        val |= 0x40;
                    val |= ins->oper1->preg;
                    PUTWORD(p, val);
                    PUTWORD(p, ins->oper1->offset->v.i);
                }
            }
            break;
        case op_moveq:
            PUTWORD(p, 0x7000 + (ins->oper2->preg << 9) + (ins->oper1->offset
                ->v.i &255));
            break;
        case op_muls:
            putmuldiv(0, 0xc1c0, 0x4c00, 0x0800, ins, p);
            break;
        case op_mulu:
            putmuldiv(0, 0xc0c0, 0x4c00, 0x0000, ins, p);
            break;
        case op_nbcd:

            PUTWORD(p, 0x4800);
            putam(ins->length, ins, ins->oper1, p);
            break;
        case op_neg:
            PUTWORD(p, 0x4400 + (ins->length / 2 << 6));
            putam(ins->length, ins, ins->oper1, p);
            break;
        case op_negx:
            PUTWORD(p, 0x4000 + (ins->length / 2 << 6));
            putam(ins->length, ins, ins->oper1, p);
            break;
        case op_nop:
            PUTWORD(p, 0x4e71);
            break;

        case op_not:
            PUTWORD(p, 0x4600 + (ins->length / 2 << 6));
            putam(ins->length, ins, ins->oper1, p);
            break;
        case op_or:
            putmath(0x8000, ins, p);
            break;
        case op_ori:
            if (ins->oper2->mode == am_sr)
            {
                PUTWORD(p, 0x7c)PUTWORD(p, ins->oper1->offset->v.i);
            }
            else if (ins->oper2->mode == am_ccr)
            {
                PUTWORD(p, 0x03c)PUTWORD(p, ins->oper1->offset->v.i &255);
            }
            else
                putmathimm(0, ins, p);
            break;
        case op_pack:
            putpack(0x8140, ins, p);
            break;
        case op_pea:
            PUTWORD(p, 0x4840);
            putam(ins->length, ins, ins->oper1, p);
            break;
        case op_rol:
            putshift(0xe118, ins, p);
            break;
        case op_ror:
            putshift(0xe018, ins, p);
            break;
        case op_roxl:
            putshift(0xe110, ins, p);
            break;
        case op_roxr:
            putshift(0xe010, ins, p);
            break;
        case op_rtd:
            PUTWORD(p, 0x4e74);
            PUTWORD(p, ins->oper1->offset->v.i);
            break;
        case op_rtr:
            PUTWORD(p, 0x4e77);
            break;
        case op_rts:
            PUTWORD(p, 0x4e75);
            break;
		case op_rte:
			PUTWORD(p, 0x4e73);
			break;
        case op_sbcd:
            putdecimal(0x8100, ins, p);
            break;
        case op_st:
            PUTWORD(p, 0x50c0);
            putam(ins->length, ins, ins->oper1, p);
            break;
        case op_seq:
            PUTWORD(p, 0x57c0);
            putam(ins->length, ins, ins->oper1, p);
            break;
        case op_spl:
            PUTWORD(p, 0x5ac0);
            putam(ins->length, ins, ins->oper1, p);
            break;
        case op_smi:
            PUTWORD(p, 0x5bc0);
            putam(ins->length, ins, ins->oper1, p);
            break;
        case op_scc:
            PUTWORD(p, 0x54c0);
            putam(ins->length, ins, ins->oper1, p);
            break;
        case op_scs:
            PUTWORD(p, 0x55c0);
            putam(ins->length, ins, ins->oper1, p);
            break;
        case op_sne:
            PUTWORD(p, 0x56c0);
            putam(ins->length, ins, ins->oper1, p);
            break;
        case op_slt:
            PUTWORD(p, 0x5dc0);
            putam(ins->length, ins, ins->oper1, p);
            break;
        case op_sle:
            PUTWORD(p, 0x5fc0);
            putam(ins->length, ins, ins->oper1, p);
            break;
        case op_sgt:
            PUTWORD(p, 0x5ec0);
            putam(ins->length, ins, ins->oper1, p);
            break;
        case op_sge:
            PUTWORD(p, 0x5cc0);
            putam(ins->length, ins, ins->oper1, p);
            break;
        case op_shi:
            PUTWORD(p, 0x52c0);
            putam(ins->length, ins, ins->oper1, p);
            break;
        case op_shs:
            PUTWORD(p, 0x54c0);
            putam(ins->length, ins, ins->oper1, p);
            break;
        case op_slo:
            PUTWORD(p, 0x55c0);
            putam(ins->length, ins, ins->oper1, p);
            break;
        case op_sls:
            PUTWORD(p, 0x53c0);
            putam(ins->length, ins, ins->oper1, p);
            break;
        case op_svc:
            PUTWORD(p, 0x58c0);
            putam(ins->length, ins, ins->oper1, p);
            break;
        case op_svs:
            PUTWORD(p, 0x59c0);
            putam(ins->length, ins, ins->oper1, p);
            break;

        case op_sub:
            putmath(0x9000, ins, p);
            break;
        case op_suba:
            PUTWORD(p, 0x90C0 + (ins->oper2->preg << 9) + ((ins->length == 4 ?
                1 : 0) << 8));
            putam(ins->length, ins, ins->oper1, p);
            break;
        case op_subi:
            putmathimm(0x400, ins, p);
            break;

        case op_subq:
            PUTWORD(p, 0x5100 + ((ins->length / 2) << 6) + (((ins->oper1
                ->offset->v.i) &7) << 9));
            putam(ins->length, ins, ins->oper2, p);
            break;
        case op_subx:
            putaddx(0x9100, ins, p);
            break;
        case op_swap:
            PUTWORD(p, 0x4840 + ins->oper1->preg);
            break;
        case op_tas:
            PUTWORD(p, 0x4ac0);
            putam(ins->length, ins, ins->oper1, p);
            break;
        case op_trap:
            PUTWORD(p, 0x4e40 + (ins->oper1->offset->v.i &15));
            break;
        case op_trapv:
            PUTWORD(p, 0x4e76);
            break;
        case op_trapt:
            puttrapcc(ins, 0x50f8, p);
            break;
        case op_trapeq:
            puttrapcc(ins, 0x57f8, p);
            break;
        case op_trappl:
            puttrapcc(ins, 0x5af8, p);
            break;
        case op_trapmi:
            puttrapcc(ins, 0x5bf8, p);
            break;
        case op_trapcc:
            puttrapcc(ins, 0x54f8, p);
            break;
        case op_trapcs:
            puttrapcc(ins, 0x55f8, p);
            break;
        case op_trapne:
            puttrapcc(ins, 0x56f8, p);
            break;
        case op_traplt:
            puttrapcc(ins, 0x5df8, p);
            break;
        case op_traple:
            puttrapcc(ins, 0x5ff8, p);
            break;
        case op_trapgt:
            puttrapcc(ins, 0x5ef8, p);
            break;
        case op_trapge:
            puttrapcc(ins, 0x5cf8, p);
            break;
        case op_traphi:
            puttrapcc(ins, 0x52f8, p);
            break;
        case op_traphs:
            puttrapcc(ins, 0x54f8, p);
            break;
        case op_traplo:
            puttrapcc(ins, 0x55f8, p);
            break;
        case op_trapls:
            puttrapcc(ins, 0x53f8, p);
            break;
        case op_trapvc:
            puttrapcc(ins, 0x58f8, p);
            break;
        case op_trapvs:
            puttrapcc(ins, 0x59f8, p);
            break;

        case op_tst:
            PUTWORD(p, 0x4A00 + (ins->length / 2 << 6));
            putam(ins->length, ins, ins->oper1, p);
            break;

        case op_unlk:
            PUTWORD(p, 0x4E58 + ins->oper1->preg);
            break;
        case op_unpk:
            putpack(0x8180, ins, p);
            break;

        case op_fmove:
            if (ins->oper1->mode == am_fpsr)
            {
                PUTWORD(p, 0xf000 + (fpcop << 9));
                PUTWORD(p, 0xa000 + (2 << 10));
                putam(ins->length, ins, ins->oper2, p);
            }
            else if (ins->oper1->mode == am_fpcr)
            {
                PUTWORD(p, 0xf000 + (fpcop << 9));
                PUTWORD(p, 0xa000 + (4 << 10));
                putam(ins->length, ins, ins->oper2, p);
            }
            else if (ins->oper2->mode == am_fpsr)
            {
                PUTWORD(p, 0xf000 + (fpcop << 9));
                PUTWORD(p, 0x8000 + (2 << 10));
                putam(ins->length, ins, ins->oper1, p);
            }
            else if (ins->oper2->mode == am_fpcr)
            {
                PUTWORD(p, 0xf000 + (fpcop << 9));
                PUTWORD(p, 0x8000 + (4 << 10));
                putam(ins->length, ins, ins->oper1, p);
            }
            else if (ins->oper2->mode == am_freg)
            {
                putfloat(0, ins->length, ins, ins->oper1, ins->oper2, p);
            }
            else
            {
                int val = 0x6000 + (ins->oper1->preg << 7);
                val |= fsize(ins->length);
                PUTWORD(p, 0xe200 + (fpcop << 10));
                PUTWORD(p, val);
                putam(ins->length, ins, ins->oper2, p);
            }
            break;

        case op_fmovem:
            {
                int val = 0xc000;
                PUTWORD(p, 0xf000 + (fpcop << 9));
                if (ins->oper1->mode == am_fmask)
                {
                    val |= 0x3000;
                    val |= revmskf((short)ins->oper1->offset);
                    PUTWORD(p, val);
                    putam(ins->length, ins, ins->oper2, p);
                }
                else
                {
                    val |= revmskf((short)ins->oper2->offset) | 0x1000;
                    PUTWORD(p, val);
                    putam(ins->length, ins, ins->oper1, p);
                }
            }
            break;
        case op_fmovecr:
            PUTWORD(p, 0xf000 + (fpcop << 9));
            PUTWORD(p, 0x5c00 + (ins->oper2->preg << 7) + (ins->oper1->offset
                ->v.i &0x7f));
            break;
        case op_fnop:
            PUTWORD(p, 0xf080 + (fpcop << 9));
            PUTWORD(p, 0);
            break;
        case op_fadd:
            putfloat(0x22, ins->length, ins, ins->oper1, ins->oper2, p);
            break;
        case op_fsub:
            putfloat(0x28, ins->length, ins, ins->oper1, ins->oper2, p);
            break;
        case op_fmul:
            putfloat(0x23, ins->length, ins, ins->oper1, ins->oper2, p);
            break;
        case op_fdiv:
            putfloat(0x20, ins->length, ins, ins->oper1, ins->oper2, p);
            break;
        case op_fmod:
            putfloat(0x21, ins->length, ins, ins->oper1, ins->oper2, p);
            break;

        case op_fcmp:
            putfloat(0x38, ins->length, ins, ins->oper1, ins->oper2, p);
            break;
        case op_ftst:
            putfloat(0x3a, ins->length, ins, 0, ins->oper1, p);
            break;
        case op_fneg:
            putfloat(0x1a, ins->length, ins, ins->oper1, ins->oper2, p);
            break;
        case op_fabs:
            putfloat(0x18, ins->length, ins, ins->oper1, ins->oper2, p);
            break;
        case op_facos:
            putfloat(0x1c, ins->length, ins, ins->oper1, ins->oper2, p);
            break;
        case op_fasin:
            putfloat(0xc, ins->length, ins, ins->oper1, ins->oper2, p);
            break;
        case op_fatan:
            putfloat(0xa, ins->length, ins, ins->oper1, ins->oper2, p);
            break;
        case op_fatanh:
            putfloat(0xb, ins->length, ins, ins->oper1, ins->oper2, p);
            break;
        case op_fcos:
            putfloat(0x1d, ins->length, ins, ins->oper1, ins->oper2, p);
            break;
        case op_fcosh:
            putfloat(0x19, ins->length, ins, ins->oper1, ins->oper2, p);
            break;
        case op_fetox:
            putfloat(0x10, ins->length, ins, ins->oper1, ins->oper2, p);
            break;
        case op_fetoxm1:
            putfloat(0x8, ins->length, ins, ins->oper1, ins->oper2, p);
            break;
        case op_fgetexp:
            putfloat(0x1e, ins->length, ins, ins->oper1, ins->oper2, p);
            break;
        case op_fgetman:
            putfloat(0x1f, ins->length, ins, ins->oper1, ins->oper2, p);
            break;
        case op_fint:
            putfloat(0x1, ins->length, ins, ins->oper1, ins->oper2, p);
            break;
        case op_fintrz:
            putfloat(0x3, ins->length, ins, ins->oper1, ins->oper2, p);
            break;
        case op_flog10:
            putfloat(0x15, ins->length, ins, ins->oper1, ins->oper2, p);
            break;
        case op_flog2:
            putfloat(0x16, ins->length, ins, ins->oper1, ins->oper2, p);
            break;
        case op_flogn:
            putfloat(0x14, ins->length, ins, ins->oper1, ins->oper2, p);
            break;
        case op_flognp1:
            putfloat(0x6, ins->length, ins, ins->oper1, ins->oper2, p);
            break;
        case op_frem:
            putfloat(0x25, ins->length, ins, ins->oper1, ins->oper2, p);
            break;
        case op_fscale:
            putfloat(0x26, ins->length, ins, ins->oper1, ins->oper2, p);
            break;
        case op_fsgldiv:
            putfloat(0x24, ins->length, ins, ins->oper1, ins->oper2, p);
            break;
        case op_fsglmul:
            putfloat(0x27, ins->length, ins, ins->oper1, ins->oper2, p);
            break;
        case op_fsin:
            putfloat(0xe, ins->length, ins, ins->oper1, ins->oper2, p);
            break;
        case op_fsincos:
            putfsincos(ins, p);
            break;

        case op_fsinh:
            putfloat(0x2, ins->length, ins, ins->oper1, ins->oper2, p);
            break;
        case op_fsqrt:
            putfloat(0x4, ins->length, ins, ins->oper1, ins->oper2, p);
            break;
        case op_ftan:
            putfloat(0xf, ins->length, ins, ins->oper1, ins->oper2, p);
            break;
        case op_ftanh:
            putfloat(0x9, ins->length, ins, ins->oper1, ins->oper2, p);
            break;
        case op_ftentox:
            putfloat(0x12, ins->length, ins, ins->oper1, ins->oper2, p);
            break;
        case op_ftwotox:
            putfloat(0x11, ins->length, ins, ins->oper1, ins->oper2, p);
            break;
        case op_fsave:
            PUTWORD(p, 0xf100 + (fpcop << 9));
            putam(ins->length, ins, ins->oper1, p);
            break;
        case op_frestore:
            PUTWORD(p, 0xf140 + (fpcop << 9));
            putam(ins->length, ins, ins->oper1, p);
            break;

        case op_fdbeq:
            putfdbranch(1, ins, p);
            break;
        case op_fdbne:
            putfdbranch(0xe, ins, p);
            break;
        case op_fdbgt:
            putfdbranch(0x12, ins, p);
            break;
        case op_fdbge:
            putfdbranch(0x13, ins, p);
            break;
        case op_fdblt:
            putfdbranch(0x14, ins, p);
            break;
        case op_fdble:
            putfdbranch(0x15, ins, p);
            break;
        case op_fdbngt:
            putfdbranch(0x1d, ins, p);
            break;
        case op_fdbnge:
            putfdbranch(0x1c, ins, p);
            break;
        case op_fdbnlt:
            putfdbranch(0x1b, ins, p);
            break;
        case op_fdbnle:
            putfdbranch(0x1a, ins, p);
            break;
        case op_fdbgl:
            putfdbranch(0x16, ins, p);
            break;
        case op_fdbngl:
            putfdbranch(0x19, ins, p);
            break;
        case op_fdbgle:
            putfdbranch(0x17, ins, p);
            break;
        case op_fdbngle:
            putfdbranch(0x18, ins, p);
            break;
        case op_fdbogt:
            putfdbranch(0x2, ins, p);
            break;
        case op_fdbule:
            putfdbranch(0xd, ins, p);
            break;
        case op_fdboge:
            putfdbranch(0x3, ins, p);
            break;
        case op_fdbult:
            putfdbranch(0xc, ins, p);
            break;
        case op_fdbolt:
            putfdbranch(0x4, ins, p);
            break;
        case op_fdbuge:
            putfdbranch(0xb, ins, p);
            break;
        case op_fdbole:
            putfdbranch(0x5, ins, p);
            break;
        case op_fdbugt:
            putfdbranch(0xa, ins, p);
            break;
        case op_fdbueq:
            putfdbranch(0x9, ins, p);
            break;
        case op_fdbor:
            putfdbranch(0x7, ins, p);
            break;
        case op_fdbun:
            putfdbranch(0x8, ins, p);
            break;
        case op_fdbt:
            putfdbranch(0x15, ins, p);
            break;
        case op_fdbsf:
            putfdbranch(0x10, ins, p);
            break;
        case op_fdbst:
            putfdbranch(0x1f, ins, p);
            break;
        case op_fdbseq:
            putfdbranch(0x11, ins, p);
            break;
        case op_fdbsne:
            putfdbranch(0x1e, ins, p);
            break;
        case op_ftrapeq:
            putftrapcc(1, ins, p);
            break;
        case op_ftrapne:
            putftrapcc(0xe, ins, p);
            break;
        case op_ftrapgt:
            putftrapcc(0x12, ins, p);
            break;
        case op_ftrapge:
            putftrapcc(0x13, ins, p);
            break;
        case op_ftraplt:
            putftrapcc(0x14, ins, p);
            break;
        case op_ftraple:
            putftrapcc(0x15, ins, p);
            break;
        case op_ftrapngt:
            putftrapcc(0x1d, ins, p);
            break;
        case op_ftrapnge:
            putftrapcc(0x1c, ins, p);
            break;
        case op_ftrapnlt:
            putftrapcc(0x1b, ins, p);
            break;
        case op_ftrapnle:
            putftrapcc(0x1a, ins, p);
            break;
        case op_ftrapgl:
            putftrapcc(0x16, ins, p);
            break;
        case op_ftrapngl:
            putftrapcc(0x19, ins, p);
            break;
        case op_ftrapgle:
            putftrapcc(0x17, ins, p);
            break;
        case op_ftrapngle:
            putftrapcc(0x18, ins, p);
            break;
        case op_ftrapogt:
            putftrapcc(0x2, ins, p);
            break;
        case op_ftrapule:
            putftrapcc(0xd, ins, p);
            break;
        case op_ftrapoge:
            putftrapcc(0x3, ins, p);
            break;
        case op_ftrapult:
            putftrapcc(0xc, ins, p);
            break;
        case op_ftrapolt:
            putftrapcc(0x4, ins, p);
            break;
        case op_ftrapuge:
            putftrapcc(0xb, ins, p);
            break;
        case op_ftrapole:
            putftrapcc(0x5, ins, p);
            break;
        case op_ftrapugt:
            putftrapcc(0xa, ins, p);
            break;
        case op_ftrapueq:
            putftrapcc(0x9, ins, p);
            break;
        case op_ftrapor:
            putftrapcc(0x7, ins, p);
            break;
        case op_ftrapun:
            putftrapcc(0x8, ins, p);
            break;
        case op_ftrapt:
            putftrapcc(0x15, ins, p);
            break;
        case op_ftrapsf:
            putftrapcc(0x10, ins, p);
            break;
        case op_ftrapst:
            putftrapcc(0x1f, ins, p);
            break;
        case op_ftrapseq:
            putftrapcc(0x11, ins, p);
            break;
        case op_ftrapsne:
            putftrapcc(0x1e, ins, p);
            break;

        case op_fseq:
            putfscc(1, ins, p);
            break;
        case op_fsne:
            putfscc(0xe, ins, p);
            break;
        case op_fsgt:
            putfscc(0x12, ins, p);
            break;
        case op_fsge:
            putfscc(0x13, ins, p);
            break;
        case op_fslt:
            putfscc(0x14, ins, p);
            break;
        case op_fsle:
            putfscc(0x15, ins, p);
            break;
        case op_fsngt:
            putfscc(0x1d, ins, p);
            break;
        case op_fsnge:
            putfscc(0x1c, ins, p);
            break;
        case op_fsnlt:
            putfscc(0x1b, ins, p);
            break;
        case op_fsnle:
            putfscc(0x1a, ins, p);
            break;
        case op_fsgl:
            putfscc(0x16, ins, p);
            break;
        case op_fsngl:
            putfscc(0x19, ins, p);
            break;
        case op_fsgle:
            putfscc(0x17, ins, p);
            break;
        case op_fsngle:
            putfscc(0x18, ins, p);
            break;
        case op_fsogt:
            putfscc(0x2, ins, p);
            break;
        case op_fsule:
            putfscc(0xd, ins, p);
            break;
        case op_fsoge:
            putfscc(0x3, ins, p);
            break;
        case op_fsult:
            putfscc(0xc, ins, p);
            break;
        case op_fsolt:
            putfscc(0x4, ins, p);
            break;
        case op_fsuge:
            putfscc(0xb, ins, p);
            break;
        case op_fsole:
            putfscc(0x5, ins, p);
            break;
        case op_fsugt:
            putfscc(0xa, ins, p);
            break;
        case op_fsueq:
            putfscc(0x9, ins, p);
            break;
        case op_fsor:
            putfscc(0x7, ins, p);
            break;
        case op_fsun:
            putfscc(0x8, ins, p);
            break;
        case op_fst:
            putfscc(0x15, ins, p);
            break;
        case op_fssf:
            putfscc(0x10, ins, p);
            break;
        case op_fsst:
            putfscc(0x1f, ins, p);
            break;
        case op_fsseq:
            putfscc(0x11, ins, p);
            break;
        case op_fssne:
            putfscc(0x1e, ins, p);
            break;

        case op_fbeq:
            putfbranch(1, ins, p);
            break;
        case op_fbne:
            putfbranch(0xe, ins, p);
            break;
        case op_fbgt:
            putfbranch(0x12, ins, p);
            break;
        case op_fbge:
            putfbranch(0x13, ins, p);
            break;
        case op_fblt:
            putfbranch(0x14, ins, p);
            break;
        case op_fble:
            putfbranch(0x15, ins, p);
            break;
        case op_fbngt:
            putfbranch(0x1d, ins, p);
            break;
        case op_fbnge:
            putfbranch(0x1c, ins, p);
            break;
        case op_fbnlt:
            putfbranch(0x1b, ins, p);
            break;
        case op_fbnle:
            putfbranch(0x1a, ins, p);
            break;
        case op_fbgl:
            putfbranch(0x16, ins, p);
            break;
        case op_fbngl:
            putfbranch(0x19, ins, p);
            break;
        case op_fbgle:
            putfbranch(0x17, ins, p);
            break;
        case op_fbngle:
            putfbranch(0x18, ins, p);
            break;
        case op_fbogt:
            putfbranch(0x2, ins, p);
            break;
        case op_fbule:
            putfbranch(0xd, ins, p);
            break;
        case op_fboge:
            putfbranch(0x3, ins, p);
            break;
        case op_fbult:
            putfbranch(0xc, ins, p);
            break;
        case op_fbolt:
            putfbranch(0x4, ins, p);
            break;
        case op_fbuge:
            putfbranch(0xb, ins, p);
            break;
        case op_fbole:
            putfbranch(0x5, ins, p);
            break;
        case op_fbugt:
            putfbranch(0xa, ins, p);
            break;
        case op_fbueq:
            putfbranch(0x9, ins, p);
            break;
        case op_fbor:
            putfbranch(0x7, ins, p);
            break;
        case op_fbun:
            putfbranch(0x8, ins, p);
            break;
        case op_fbt:
            putfbranch(0x15, ins, p);
            break;
        case op_fbsf:
            putfbranch(0x10, ins, p);
            break;
        case op_fbst:
            putfbranch(0x1f, ins, p);
            break;
        case op_fbseq:
            putfbranch(0x11, ins, p);
            break;
        case op_fbsne:
            putfbranch(0x1e, ins, p);
            break;


        default:
            return FALSE;
    }
    ins->outlen = pos - ins->outbuf;
    return TRUE;
}

//-------------------------------------------------------------------------

static int intsize(long value)
{
    if (value < 0x80 && value >=  - 0x80)
        return 1;
    else
        if (value < 0x8000 && value >=  - 0x8000)
            return 2;
}

///////////////////////////////////////////////////
void outcode_start_virtual_seg(SYM *sp, int data)
{
    VIRTUAL_LIST *x;
    global_flag++;
    x = xalloc(sizeof(VIRTUAL_LIST));
    x->sp = sp;
    x->seg = xalloc(sizeof(EMIT_TAB));
    x->data = data;
    global_flag--;
    if (virtualFirst)
        virtualLast = virtualLast->next = x;
    else
        virtualFirst = virtualLast = x;
}

//-------------------------------------------------------------------------

void outcode_end_virtual_seg(SYM *sp){}
ENODE *GetSymRef(ENODE *n)
{
    ENODE *rv;
    switch (n->nodetype)
    {
        case en_add:
        case en_addstruc:
            if (rv = GetSymRef(n->v.p[0]))
                break;
            rv = GetSymRef(n->v.p[1]);
            break;
        case en_icon:
        case en_ccon:
        case en_cucon:
        case en_lcon:
        case en_lucon:
        case en_iucon:
            return 0;
        case en_labcon:
        case en_nacon:
        case en_autocon:
        case en_absacon:
        case en_nalabcon:
        case en_napccon:
            return n;
        default:
            DIAG("Unexpected node type in GetSymRef");
            break;
    }
    return rv;
}

//-------------------------------------------------------------------------

int ResolveOffset(OCODE *ins, ENODE *n, int *resolved)
{
    int rv = 0;
    if (n)
    {
        switch (n->nodetype)
        {
            case en_add:
            case en_addstruc:
                rv += ResolveOffset(ins, n->v.p[0], resolved);
                rv += ResolveOffset(ins, n->v.p[1], resolved);
                break;
            case en_llcon:
            case en_llucon:
            case en_icon:
            case en_ccon:
            case en_cucon:
            case en_lcon:
            case en_lucon:
            case en_iucon:
            case en_absacon:
                rv += n->v.i;
                break;
            case en_autocon:
            case en_autoreg:
                break;
            case en_labcon:
            case en_nalabcon:
            case en_nacon:
            case en_napccon:
                *resolved = 0;
                break;
            default:
                DIAG("Unexpected node type in ResolveOffset");
                break;
        }
    }
    return rv;
}

//-------------------------------------------------------------------------

int outcode_AssembleIns(OCODE *ins, int address)
{
    unsigned char *pos = ins->outbuf;
    unsigned char **p = &pos;
    if (ins->opcode >= op_abcd)
    {
        int found = outcode_putop(ins);
        ;
        if (found)
        {
            ins->address = address;
            return ins->outlen;
        }
        else
        {
            ins->outlen = 0;
            DIAG("outcode_Assemblens - Unassembled instruction");
            return 0;
        }
    }
    else
    switch (ins->opcode)
    {
        case op_label:
            ins->address = address;
            InsertLabel((int)ins->oper1, address, codeseg);
            return 0;
        case op_funclabel:
            return ins->outlen = 0;
        case op_slit:
            DIAG("outcode_AssembleIns - slit encountered");
            return ins->outlen = 0;
        case op_dcl:
            *(int*)ins->outbuf = 0;
            ins->ofs[ins->rescount] = 0;
            ins->resobyte[ins->rescount] = 4;
            ins->addroffset[ins->rescount++] = 0;
            return ins->outlen = ins->length;
            break;
        case op_line:
        case op_blockstart:
        case op_blockend:
            ins->address = address;
            return ins->outlen = 0;
        case op_genword:
            ins->address = address;
            ins->outbuf[0] = ins->oper1->offset->v.i >> 8;
            ins->outbuf[1] = ins->oper1->offset->v.i;
            return ins->outlen = 1;
        default:
            DIAG("outcode_Assemblens - Unknown directive");
        case op_void:
            return ins->outlen = 0;
    }
}

//-------------------------------------------------------------------------

void outcode_optimize(OCODE *peeplist)
{
    int done = FALSE;
    while (!done)
    {
        OCODE *head = peeplist;
        int offset = 0;
        done = TRUE;
        while (head)
        {
            head->address += offset;
            if (head->branched &(BR_SHORT | BR_LONG | BR_BYTE))
            {
				head->length = 0;
                if (head->reloffset->offset && head->reloffset->offset
                    ->nodetype == en_labcon)
                {
                    if (head->branched &BR_DBRA)
                    {
                        int adr;
                        adr = head->oper2->offset->v.i;
                        adr = LabelAddress(adr);
                        if (adr !=  - 1)
                        {
                            if (adr > head->address)
                                adr += offset;
                            adr = adr - (head->address + 2);
                            head->outbuf[2] = adr >> 8;
                            head->outbuf[3] = adr;
                            head->oper1->length =  - 1;
                            head->outlen = 4;
                            head->rescount = 0;
                        }
                    }
                    else if (head->reloffset)
                    {
                        int adr;
                        adr = head->reloffset->offset->v.i;
                        adr = LabelAddress(adr);
                        if (adr !=  - 1)
                        {
                            if (adr > head->address)
                                adr += offset;
                            adr = adr - (head->address + 2);
                            if (adr < 128 && adr >=  - 127 && !(head->branched
                                &(BR_FLOAT | BR_PCREL)))
                            {
                                if (adr == 0 || adr == 0xff)
                                    DIAG("outcode_optimize - branch to next");
                                if (head->branched &BR_LONG)
                                {
                                    offset -= 4;
                                    done = FALSE;
                                }
                                else if (head->branched &BR_SHORT)
                                {
                                    offset -= 2;
                                    done = FALSE;
                                }
                                head->outbuf[1] = adr;
                                head->oper1->length =  - 1;
                                head->branched &= ~(BR_SHORT | BR_LONG |
                                    BR_BYTE);
                                head->branched |= BR_BYTE;
                                head->outlen = 2;
                                head->rescount = 0;
                            }
                            else if (adr < 32768 && adr >=  - 32768)
                            {
                                if (head->branched &BR_LONG)
                                {
                                    offset -= 2;
                                    done = FALSE;
                                    if (head->branched &BR_FLOAT)
                                    {
                                        head->rescount = 0;
                                        head->outbuf[1] &= ~0x40;
                                    }
                                    else if (head->branched &BR_PCREL)
                                    {
                                        head->resobyte[head->pcrelres] = 0;
                                        head->outbuf[1] &= 0xfe;
                                    }
                                    else
                                    {
                                        head->rescount = 0;
                                        head->outbuf[1] = 0x0;
                                    }
                                }
                                head->outbuf[head->pcrelofs] = adr >> 8;
                                head->outbuf[head->pcrelofs + 1] = adr;
                                head->oper1->length =  - 1;
                                head->branched &= ~(BR_SHORT | BR_LONG |
                                    BR_BYTE);
                                head->branched |= BR_SHORT;
                                head->outlen = 4;

                            }
                            else
                            {
                                head->outbuf[head->pcrelofs] = (adr >> 24);
                                head->outbuf[head->pcrelofs + 1] = adr >> 16;
                                head->outbuf[head->pcrelofs + 2] = adr >> 8;
                                head->outbuf[head->pcrelofs + 3] = adr;
                                head->rescount = 0;
                            }
                        }
                    }
                }
                else if (head->branched &BR_BYTE)
                {
                    int adr = LabelAddress((int)head->oper1->offset->v.i);
                    if (adr !=  - 1)
                    {
                        if (adr == 0 || adr == 0xff)
                            DIAG("outcode_optimize - branch to next");
                        if (adr > head->address)
                            adr += offset;
                        adr = adr - (head->address + 2);
                        head->outbuf[1] = adr;
                    }
                }
            }
            if (head->opcode == op_label && offset)
            {
                InsertLabel((int)head->oper1, head->address, codeseg);
            }
            head = head->fwd;
        }
    }

}

//-------------------------------------------------------------------------

void outcode_dumpIns(OCODE *peeplist)
{
    int i;
    while (peeplist)
    {
        int resolved = FALSE;
        if (peeplist->branched && peeplist->reloffset->offset && peeplist
            ->reloffset->offset->nodetype == en_labcon)
        {
            int adr = 0, dbr = 0;
            adr = peeplist->reloffset->offset->v.i;
            if (adr)
            {
                adr = LabelAddress(adr);
                if (adr !=  - 1)
                {
                    adr = adr - peeplist->address - 2;
                    if (peeplist->branched &BR_BYTE)
                    {
                        if (adr <  - 128 || adr >= 127 || adr == 0 || adr ==
                            0xff)
                            DIAG("Short branch out of range in outcode_dumpIns")
                                ;
                        peeplist->outbuf[1] = adr;
                    }
                    else if (peeplist->branched &BR_SHORT)
                    {
                        if (!(peeplist->branched &(BR_FLOAT | BR_PCREL |
                            BR_DBRA)))
                            peeplist->outbuf[1] = 0x0;
                        peeplist->outbuf[peeplist->pcrelofs] = adr >> 8;
                        peeplist->outbuf[peeplist->pcrelofs + 1] = adr;
                    }
                    else
                    {
                        if (!(peeplist->branched &(BR_FLOAT | BR_PCREL |
                            BR_DBRA)))
                            peeplist->outbuf[1] = 0xff;
                        peeplist->outbuf[peeplist->pcrelofs] = (adr >> 24);
                        peeplist->outbuf[peeplist->pcrelofs + 1] = adr >> 16;
                        peeplist->outbuf[peeplist->pcrelofs + 2] = adr >> 8;
                        peeplist->outbuf[peeplist->pcrelofs + 3] = adr;
                    }
                    resolved = TRUE;
                }
            }
        }
        //      if (peeplist->opcode == op_dd)
        //         *(int *)(peeplist->outbuf) = LabelAddress((int)peeplist->oper1->offset->v.i) ;
        emit(curseg, peeplist->outbuf, peeplist->outlen);
        for (i = 0; i < peeplist->rescount; i++)
        {
            AMODE *oper;
            ENODE *node;
            if (peeplist->resobyte[i])
            {
                if (peeplist->resobyte[i] < 16)
                {
                    oper = peeplist->oper1;
                }
                else
                {
                    oper = peeplist->oper2;
                }
                node = GetSymRef(oper->offset);
                if (!node)
                    DIAG("Unlocated fixup in outcode_dumpIns");
                else if (node->nodetype == en_labcon)
                    gen_label_fixup(peeplist->branched ? fm_rellabel : fm_label,
                        curseg, peeplist->address + peeplist->addroffset[i],
                        node->v.i, peeplist->resobyte[i] &15, peeplist->ofs[i]);
                else if (node->nodetype == en_nalabcon)
                    gen_label_fixup(peeplist->branched ? fm_rellabel : fm_label,
                        curseg, peeplist->address + peeplist->addroffset[i],
                        node->v.sp->value.i, peeplist->resobyte[i] &15,
                        peeplist->ofs[i]);
                else
                    gen_symbol_fixup(peeplist->branched ? fm_relsymbol :
                        fm_symbol, curseg, peeplist->address + peeplist
                        ->addroffset[i], node->v.sp, peeplist->resobyte[i] &15,
                        peeplist->ofs[i]);
            }
        }
        switch (peeplist->opcode)
        {
            case op_line:
                //            InsertLine(peeplist->address,(int)peeplist->oper2,(int)peeplist->oper3) ;
                break;
            case op_blockstart:
                //            if (prm_debug)
                //               DbgBlocks[(int)(peeplist->blocknum)]->startofs = peeplist->address ;
                break;
            case op_blockend:
                //            if (prm_debug)
                //               DbgBlocks[(int)(peeplist->blocknum)]->endofs = peeplist->address ;
                break;
        }
        peeplist = peeplist->fwd;
    }
}

//-------------------------------------------------------------------------

void outcode_gen(OCODE *peeplist)
{
    OCODE *head = peeplist;
    outcode_func_init();
    head = peeplist;
    while (head)
    {
        outcode_base_address += outcode_AssembleIns(head, outcode_base_address);
        head = head->fwd;
    }
    outcode_optimize(peeplist);
    if (!prm_asmfile)
        outcode_dumpIns(peeplist);
}
