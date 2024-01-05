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
#include <stdarg.h>
#include <dos.h>
#include "lists.h"
#include "expr.h"
#include "c.h"
#include "gen68.h"
#include "diag.h"
#include "lists.h"

#define DEBUG_VERSION 3.1

#define LDPERLINE 32
extern FILELIST *incfiles;
extern VIRTUAL_LIST *virtualFirst;
extern int prm_debug;
extern FILE *outputFile;
extern char version[];
extern char *infile;
extern EMIT_TAB segs[MAX_SEGS];
extern LIST *libincludes;
extern LINEBUF *linelist;
extern HASHREC **gsyms;
extern LIST *localfuncs,  *localdata;
extern int prm_bss;
extern int outcode_base_address;
extern int anyusedfloat;

static int firstVirtualSeg;
static int segxlattab[MAX_SEGS];
static int dgroupVal;
static int extSize, extIndex, pubIndex;
static int checksum = 0;
static int options = 1; // signed addr
static int bitspermau = 8;
static int maus = 4;
static int bigendchar = 'M';


char *segnames[] = 
{
    0, "code", "data", "bss", "const", "string", "cstartup", "crundown", 
        "cppinit", "cppexit", "codefix", "datafix", "lines", "types", "symbols",
        "browse"
};
char *segclasses[] = 
{
    0, "code", "data", "bss", "const", "string", "cstartup", "crundown", 
        "cppinit", "cppexit", "codefix", "datafix", "lines", "types", "symbols",
        "browse"
};
static char *segchars[] = 
{
//    0, "R", "W", "W", "R", "R", "R", "R", "R", "R", "R", "R", "R", "R", "R", "R"
    0, "C", "C", "C", "C", "C", "C", "C", "C", "C", "C", "C", "C", "C", "C", "C"
};
void obj_init(void)
{
}
static void emit_record(char *format, ...)
{
    char buffer[256];
    int i, l;
    va_list ap;

    va_start(ap, format);
    vsprintf(buffer, format, ap);
    l = strlen(buffer);
    for (i = 0; i < l; i++)
        if (buffer[i] > 31)
            checksum += buffer[i];
    va_end(ap);
    oprintf(outputFile, "%s", buffer);
}

//-------------------------------------------------------------------------

static void emit_cs(int toclear)
{
    if (toclear)
    {
        emit_record("CS.\r\n");
    }
    else
    {
        checksum += 'C';
        checksum += 'S';
        emit_record("CS%02X.\r\n", checksum &127);
    }
    checksum = 0;
}

//-------------------------------------------------------------------------

static void link_header(char *name)
{
    struct date thedate;
    struct time thetime;
    char fpspec = 'I';
    getdate(&thedate);
    gettime(&thetime);
    if (anyusedfloat)
        fpspec = 'F';
    emit_record("MB%cCC68K,%02X%s.\r\n", fpspec, strlen(name), name);
    emit_record("CO104,08%08lX.\r\n", options);
    emit_record("AD%x,%x,%c.\r\n", bitspermau, maus, bigendchar);
    emit_record("DT%04d%02d%02d%02d%02d%02d.\r\n", thedate.da_year,
        thedate.da_mon, thedate.da_day, thetime.ti_hour, thetime.ti_min,
        thetime.ti_sec);
    emit_record("CO101,07ENDHEAD.\r\n");
}

//-------------------------------------------------------------------------

static void link_trailer(void)
{
    emit_record("ME.\r\n");
} void link_FileTime(char *file)
{
    #ifdef XXXXX
        unsigned short time, date;
        char buf[256];
        int fd;
        *(short*)buf = 0xe900;
        if (file)
        {
            if (_dos_open(file, 0, &fd))
            {
                time = 0;
                date = 0;
            }
            else
            {
                _dos_getftime(fd, &date, &time);
                _dos_close(fd);
            }
            *(short*)(buf + 2) = time;
            *(short*)(buf + 4) = date;
            memcpy(buf + 7, file, strlen(file));
            buf[6] = strlen(file);
            emit_record(COMENT, buf, 7+buf[6]);
        }
        else
            emit_record(COMENT, buf, 2);
    #endif 
}

//-------------------------------------------------------------------------

void link_DebugMarker(void){}
void link_LibMod(void)
{
    #ifdef XXXXX
        if (libincludes)
        {
            while (libincludes)
            {
                char buf[256],  *p;
                p = strrchr(libincludes->data, '\\');
                if (p)
                    p++;
                else
                    p = libincludes->data;

                buf[0] = 0xa3;
                buf[1] = strlen(p);
                strcpy(buf + 2, p);
                emit_record(COMENT, buf, buf[1] + 2);
                libincludes = libincludes->link;
            }
        }
    #endif 
}

//-------------------------------------------------------------------------

void link_Segs(void)
{
    char buf[512];
    int i;
    VIRTUAL_LIST *v = virtualFirst;
    firstVirtualSeg = 1;
    for (i = 1; i < MAX_SEGS; i++)
    {
        if (segxlattab[i])
        {
            emit_record("ST%X,%s,%02X%s.\r\n", segxlattab[i], segchars[i],
                strlen(segnames[i]), segnames[i]);
            emit_record("SA%X,%x.\r\n", segxlattab[i], 4);
            emit_record("ASS%X,%lX.\r\n", segxlattab[i], segs[i].curlast);
            firstVirtualSeg++;
        }
    }
    i = firstVirtualSeg;
    while (v)
    {
        v->sp->value.i = i - firstVirtualSeg;
        putsym(buf + 1, v->sp, v->sp->name);
        buf[0] = '@';
        emit_record("ST%X,%s,E,%02X%s.\r\n", i, v->data ? segchars[dataseg]:
            segchars[codeseg], strlen(buf), buf);
        emit_record("SA%X,%x.\r\n", i, 4);
        emit_record("ASS%X,%lX.\r\n", i++, v->seg->curlast);
        v = v->next;
    }
}

//-------------------------------------------------------------------------

void link_putext(SYM *sp)
{

    char buf[512];
    putsym(buf, sp, sp->name);
    sp->value.i = extIndex++;
    emit_record("NX%X,%02X%s.\r\n", sp->value.i, strlen(buf), buf);
    #ifdef XXXXX
        if (prm_debug)
            if (r->datatype &TY_TYPE)
                emit_record("ATX%X,T%lX.\r\n", r->id, r->datatype &~TY_TYPE);
            else
                if (r->datatype &TY_NAME)
                    emit_record("ATX%X,I%lX.\r\n", r->id, r->datatype &~TY_NAME)
                        ;
                else
                    emit_record("ATX%X,%lX.\r\n", r->id, r->datatype);
        q = q->link;
    }
#endif 
}

//-------------------------------------------------------------------------

void link_ExtDefs(void)
{
    int i;
    SYM *sp;
    LIST *lf = localfuncs;
    extIndex = 1;
    extSize = 0;
    for (i = 0; i < HASHTABLESIZE; i++)
    {
        if ((sp = (SYM*)gsyms[i]) != 0)
        {
            while (sp)
            {
                if (sp->storage_class == sc_externalfunc && sp->mainsym
                    ->extflag && !(sp->tp->cflags &DF_INTRINS))
                {
                    link_putext(sp);
                }
                if (sp->storage_class == sc_external && sp->mainsym->extflag)
                {
                    link_putext(sp);
                }
                if (sp->storage_class == sc_defunc)
                {
                    SYM *sp1 = sp->tp->lst.head;
                    while (sp1)
                    {
                        if (sp1->storage_class == sc_externalfunc && sp1
                            ->mainsym->extflag && !(sp1->tp->cflags &DF_INTRINS)
                            )
                        {
                            link_putext(sp1);
                        }
                        sp1 = sp1->next;
                    }
                }
                sp = sp->next;
            }
        }
    }

    while (lf)
    {
        sp = lf->data;
        if (sp->storage_class == sc_externalfunc && sp->mainsym->extflag && !
            (sp->tp->cflags &DF_INTRINS))
        {
            link_putext(sp);
        }
        lf = lf->link;
    }
    lf = localdata;
    while (lf)
    {
        sp = lf->data;
        if (sp->mainsym->extflag)
            link_putext(sp);
        lf = lf->link;
    }

}

//-------------------------------------------------------------------------

void link_putimport(SYM *sp)
{
    #ifdef XXXXX
        char buf[512], obuf[512];
        int l, l1;
        putsym(buf, sp, sp->name);
        l = strlen(buf);
        *(short*)obuf = 0xa000;
        obuf[2] = 1; // import
        obuf[3] = 0; // import by name
        obuf[4] = l;
        strcpy(obuf + 5, buf);
        obuf[5+l] = l1 = strlen(sp->importfile);
        strcpy(obuf + 6+l, sp->importfile);
        obuf[6+l + l1] = l;
        strcpy(obuf + 7+l + l1, buf);

        emit_record(COMENT, obuf, 7+2 * l + l1);
    #endif 
}

//-------------------------------------------------------------------------

void link_Imports(void)
{
    SYM *sp;
    int i;
    for (i = 0; i < HASHTABLESIZE; i++)
    {
        if ((sp = (SYM*)gsyms[i]) != 0)
        {
            while (sp)
            {
                if (sp->mainsym->importable && sp->importfile)
                {
                    link_putimport(sp);
                }
                sp = sp->next;
            }
        }
    }
}

//-------------------------------------------------------------------------

int link_getseg(SYM *sp)
{
    if (!sp->tp)
        return dataseg;
    switch (sp->storage_class)
    {
        case sc_member:
            if (sp->value.classdata.gdeclare)
                sp = sp->value.classdata.gdeclare;
            else
                if (sp->tp->type != bt_func && sp->tp->type != bt_ifunc)
                    DIAG("link_getseg - no static member initializer");
            // fall through
        case sc_static:
        case sc_global:
            if (sp->tp->type == bt_func || sp->tp->type == bt_ifunc)
                if (sp->gennedvirtfunc)
                    return sp->value.i + firstVirtualSeg;
                else
                    return codeseg;
            /* fall through */
        case sc_external:
            if (sp->tp->cflags &DF_CONST)
                return constseg;
            else
                if (sp->init || !prm_bss)
                    return dataseg;
                else
                    return bssxseg;
        case sc_externalfunc:
        case sc_label:
            if (sp->gennedvirtfunc)
                return sp->value.i + firstVirtualSeg;
            else
                return codeseg;
        case sc_type:
            if (!strncmp(sp->name, "@$xt", 4))
            {
                return sp->value.i;
            }
        default:
            DIAG("Unknown segment type in link_GetSeg");
            return codeseg;
    }
    // also fix the value.i field of local funcs...
}

//-------------------------------------------------------------------------

void link_putpub(SYM *sp)
{
    char buf[512], obuf[512];
    int seg, group, pos;
    if (sp->mainsym->gennedvirtfunc)
        return ;
    if (sp->value.classdata.templatedata)
        return ;
    if (sp->value.classdata.cppflags &PF_INLINE)
        return ;
    if ((sp->tp->cflags &DF_CONST) && scalarnonfloat(sp->tp) && sp
        ->storage_class == sc_static)
        return ;

    seg = link_getseg(sp);
	seg = segxlattab[seg];
    putsym(buf, sp, sp->name);
    emit_record("NI%X,%02X%s.\r\n", pubIndex, strlen(buf), buf);
    emit_record("ASI%X,R%X,%X,+.\r\n", pubIndex++, seg, sp->mainsym->offset);
    #ifdef XXXXX
        if (prm_debug)
            if (p->datatype &TY_TYPE)
                emit_record("ATI%X,T%lX.\r\n", p->id, p->datatype &~TY_TYPE);
            else
                emit_record("ATI%X,%lX.\r\n", p->id, p->datatype);
    #endif 
}

//-------------------------------------------------------------------------

void link_Publics(void)
{
    SYM *sp;
    LIST *lf = localfuncs;
    int i;
    for (i = 0; i < HASHTABLESIZE; i++)
    {
        if ((sp = (SYM*)gsyms[i]) != 0)
        {
            while (sp)
            {
                if (sp->storage_class == sc_global && !(sp
                    ->value.classdata.cppflags &PF_INLINE))
                {
                    if (!sp->mainsym->gennedvirtfunc)
                        link_putpub(sp);
                }
                sp = sp->next;
            }
        }
    }
    while (lf)
    {
        SYM *sp = lf->data;
        if (sp->storage_class == sc_global && !(sp->value.classdata.cppflags
            &PF_INLINE))
        {
            if (!sp->mainsym->gennedvirtfunc)
                link_putpub(sp);
        }
        lf = lf->link;
    }
}

//-------------------------------------------------------------------------

void link_putexport(SYM *sp)
{
    #ifdef XXXXX
        char buf[512], obuf[512];
        int l;
        putsym(buf, sp, sp->name);
        l = strlen(buf);
        *(short*)obuf = 0xa000;
        obuf[2] = 2; // export
        obuf[3] = 0; // export flags
        obuf[4] = l;
        strcpy(obuf + 5, buf);
        buf[5+l] = l;
        strcpy(obuf + 6+l, buf);

        emit_record(COMENT, obuf, 6+2 * l);
    #endif 
}

//-------------------------------------------------------------------------

void link_Exports(void)
{
    SYM *sp;
    int i;
    for (i = 0; i < HASHTABLESIZE; i++)
    {
        if ((sp = (SYM*)gsyms[i]) != 0)
        {
            while (sp)
            {
                if (sp->storage_class == sc_global && sp->exportable)
                {
                    link_putexport(sp);
                }
                sp = sp->next;
            }
        }
    }
}

//-------------------------------------------------------------------------

void link_PassSeperator(void)
{
    emit_record("CO100,06ENDSYM.\r\n");
}

//-------------------------------------------------------------------------

void link_Fixups(char *buf, FIXUP *fixup, EMIT_LIST *rec, int curseg)
{
    int iseg, xseg;
    int rel = FALSE;
    switch (fixup->fmode)
    {
        case fm_relsymbol:
            rel = TRUE;
        case fm_symbol:
            if (fixup->sym->storage_class == sc_abs)
            {
                if (rel)
                {
                    strcpy(buf, "0");
                    DIAG("Relative absolute in link_Fixups");
                }
                else
                    sprintf(buf, "%X", fixup->sym->offset + fixup->ofs);
                return ;
            }
            iseg = link_getseg(fixup->sym);
            if (fixup->sym->gennedvirtfunc)
                xseg = iseg;
            else
                xseg = segxlattab[iseg];
            {
                SYM *sp = fixup->sym;
                if (sp->mainsym)
                    sp = sp->mainsym;
                if (sp->storage_class == sc_member && sp
                    ->value.classdata.gdeclare && sp->value.classdata.gdeclare
                    ->storage_class == sc_global)
                    sp = sp->value.classdata.gdeclare;
                if (sp->storage_class == sc_global || sp->storage_class ==
                    sc_static || (sp->storage_class == sc_external || sp
                    ->storage_class == sc_externalfunc) && !sp->mainsym
                    ->extflag)
                {
                    if (rel)
                    {
                        if (iseg == curseg)
                        {
                            sprintf(buf, "%X", sp->mainsym->offset - fixup
                                ->address);
                        }
                        else
                        {
                            sprintf(buf, "R%X,%X,+,P,-", xseg, sp->mainsym
                                ->offset);
                        }
                    }
                    else
                    {
                        sprintf(buf, "R%X,%X,+", xseg, sp->mainsym->offset +
                            fixup->ofs);
                    }
                    /* segment relative */
                }
                else
                {
                    if (rel)
                    {
                        sprintf(buf, "X%X,P,-,%X,+", sp->value.i, fixup->ofs);
                    }
                    else
                    {
                        sprintf(buf, "X%X", sp->value.i);
                    }
                    /* extdef relative */
                }
            }
            break;

        case fm_rellabel:
            rel = TRUE;
        case fm_label:
            iseg = LabelSeg(fixup->label);
            xseg = segxlattab[iseg];
            if (rel)
            {
                if (iseg == curseg)
                    sprintf(buf, "%X", LabelAddress(fixup->label) + fixup->ofs 
                        - fixup->address);
                else
                {
                    sprintf(buf, "R%X,%X,+,P,-", xseg, LabelAddress(fixup
                        ->label) + fixup->ofs);
                }
            }
            else
            {
                sprintf(buf, "R%X,%X,+", segxlattab[iseg], LabelAddress(fixup
                    ->label) + fixup->ofs);
            }
            /* segment relative */
            break;
    }
}

//-------------------------------------------------------------------------

static long putlr(FIXUP *p, EMIT_LIST *s, int curseg)
{
    char buf[512];
    link_Fixups(buf, p, s, curseg);
    emit_record("LR(%s,%X).\r\n", buf, p->size);
    return (p->size);
}

//-------------------------------------------------------------------------

static long putld(long start, long end, unsigned char *buf)
{
    long count = 0;
    int i;
    if (start == end)
        return (start);
    while (end - start >= LDPERLINE)
    {
        emit_record("LD");
        for (i = 0; i < LDPERLINE; i++)
            emit_record("%02X", buf[count++]);
        start += LDPERLINE;
        emit_record(".\r\n");
    }
    if (start == end)
        return (start);
    emit_record("LD");
    while (start < end)
    {
        emit_record("%02X", buf[count++]);
        start++;
    }
    emit_record(".\r\n");
    return (start);
}

//-------------------------------------------------------------------------

void link_Data(void)
{
    char buf[1100];
    int i;
    VIRTUAL_LIST *v = virtualFirst;
    for (i = 1; i < MAX_SEGS; i++)
    if (segxlattab[i] && segs[i].curlast)
    {
        EMIT_LIST *rec = segs[i].first;
        emit_record("SB%X.\r\n", segxlattab[i]);
        while (rec)
        {
            int j = 0;
            int len;
            long size = segs[i].curlast;
            FIXUP *f = rec->fixups;
            while (j < rec->filled)
            {
                if (f)
                    len = f->address - rec->address - j;
                else
                    len = rec->filled - j;

                putld(j, len + j, rec->data + j);
                j += len;
                if (f)
                {
                    j += putlr(f, rec, i);
                    f = f->next;
                }
            }
            rec = rec->next;
        }
        emit_cs(FALSE);
    }
    i = firstVirtualSeg;
    while (v)
    {
        EMIT_LIST *rec = v->seg->first;
        emit_record("SB%X.\r\n", i);
        while (rec)
        {
            int j = 0;
            int len;
            long size = v->seg->curlast;
            FIXUP *f = rec->fixups;
            while (j < rec->filled)
            {
                if (f)
                    len = f->address - rec->address - j;
                else
                    len = rec->filled - j;

                putld(j, len + j, rec->data + j);
                j += len;
                if (f)
                {
                    j += putlr(f, rec, i);
                    f = f->next;
                }
            }
            rec = rec->next;
        }
        emit_cs(FALSE);
        i++;
        v = v->next;
    }
}

//-------------------------------------------------------------------------

void link_SourceFile(char *file, int num)
{
    file = fullqualify(file);
    #ifdef XXXXX
        unsigned short time, date;
        char buf[512];
        int fd;
        *(short*)buf = 0xe800;
        buf[2] = num;
        buf[3] = strlen(file);
        strcpy(buf + 4, file);
        if (_dos_open(file, 0, &fd))
        {
            time = 0;
            date = 0;
        }
        else
        {
            _dos_getftime(fd, &date, &time);
            _dos_close(fd);
        }
        *(short*)(buf + 4+buf[3]) = time;
        *(short*)(buf + 6+buf[3]) = date;
        emit_record(COMENT, buf, 8+buf[3]);
    #endif 
}

//-------------------------------------------------------------------------

void link_LineNumbers(int file)
{
    #ifdef XXXXX
        LINEBUF *l = linelist;
        char buf[1100],  *p;
        int lastnum =  - 1;
        while (l)
        {
            p = buf;
            *p++ = 0;
            *p++ = segxlattab[codeseg];
            while (l && p - buf < 1024-6)
            {
                if (l->file == file)
                {
                    //            if (l->address != lastnum) {
                    *(short*)p = l->lineno;
                    *(int*)(p + 2) = l->address;
                    p += 6;
                    lastnum = l->address;
                    //            }
                }
                l = l->next;
            }
            if (p - buf > 2)
            {
                emit_record(LINNUM, buf, p - buf);
            }
        }
    #endif 
}

//-------------------------------------------------------------------------

void link_EmitLineInfo(void)
{
    int i, q;
    FILELIST *l;
    link_SourceFile(infile, 1);
    link_LineNumbers(0);
    for (i = 1, q = 2, l = incfiles; l; i++, l = l->link)
    if (l->hascode)
    {
        link_SourceFile(l->data, q++);
        link_LineNumbers(i);
    }
}

//-------------------------------------------------------------------------

void output_obj_file(void)
{
    LIST *l = incfiles;
    int i, pos = 1;
    extIndex = pubIndex = 1;
    checksum = 0;
    for (i = 1; i < MAX_SEGS; i++)
    {
        if (segs[i].curlast)
        {
            segxlattab[i] = pos++;
        }
        else
            segxlattab[i] = 0;
    }
    link_header(fullqualify(infile));
    emit_cs(FALSE);
    link_Segs();
    emit_cs(FALSE);
    link_ExtDefs();
    link_Publics();
    emit_cs(FALSE);
    link_PassSeperator();
    link_Data();
    link_trailer();
}
