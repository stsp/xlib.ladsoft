/* 
IMAKE make utility
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
#include "utype.h"
#include "cmdline.h"
#include "interp.h"
#include "umem.h"
#include "imake.h"
#include "register.h"
#include "phash.h"
#include "input.h"
#include "module.h"

extern HASHREC **hashtable = 0;
LIST *targetlist = 0;

static short *stripspaces(short *buf)
{
    short *rv = buf,  *q;
    while (iswhitespacechar(*rv))
        rv++;
    q = rv + pstrlen(rv) - 1;
    while (iswhitespacechar(*q))
        q--;
    *(q + 1) = 0;
    return (rv);
}

//-------------------------------------------------------------------------

static void Redef(REGISTRY **r, int type, BOOL erronchange)
{
    if (erronchange && r && !(*r)->canchange)
        Error("Can't redefine symbol");

    if (r && (*r)->type != type)
        Error("Attempt to redefine symbol type");
}

//-------------------------------------------------------------------------

void RegisterPath(short *buf, short *bufptr)
{
    short *p1,  *p2;
    REGISTRY **r;
    p1 = stripspaces(buf);
    p2 = stripspaces(bufptr);
    r = LookupPhiHash(hashtable, buf);
    Redef(r, R_PATH, FALSE);
    if (r)
    {
        short buffer[1000],  *p = buffer;
        pstrcpy(buffer, (*r)->x.path);
        p += pstrlen(buffer);
        *p++ = ';';
        pstrcpy(p, p2);
        DeallocateMemory((*r)->x.path);
        (*r)->x.path = AllocateMemory(pstrlen(buffer) *2+2);
        pstrcpy((*r)->x.path, buffer);
    }
    else
    {
        REGISTRY *r;
        r = AllocateMemory(sizeof(REGISTRY));
        r->name = AllocateMemory(pstrlen(p1) *2+2);
        r->type = R_PATH;
        r->canchange = TRUE;
        pstrcpy(r->name, p1);
        r->x.path = AllocateMemory(pstrlen(p2) *2+2);
        pstrcpy(r->x.path, p2);
        AddPhiHash(hashtable, r);
    }
}

//-------------------------------------------------------------------------

void RegisterTarget(short *name, BOOL infile)
{
    short *p;
    short buffer[256];
    LIST *h = &targetlist,  *l;
    if (infile && targetlist)
        return ;
    p = buffer;
    if (!infile)
    {
        char *nm = name;
        while (*nm)
            *p++ =  *nm++;
        *p = 0;
        p = buffer;
    }
    else
        p = name;
    while (h->link)
        h = h->link;
    l = AllocateMemory(sizeof(LIST));
    l->data = AllocateMemory(pstrlen(p) *2+2);
    pstrcpy(l->data, p);
    l->link = 0;
    h->link = l;
}

//-------------------------------------------------------------------------

void RegisterDest(short *name)
{
    short buf[100];
    short *p1,  *p2;
    REGISTRY **r;
    p1 = stripspaces(name);
    p2 = buf;
    do
    {
        *p2++ =  *p1++;
    }
    while (*p1 &&  *p1 != PERIOD);
    if (!p1)
        Error("Illegal implicit");
    *p2 = 0;


    r = LookupPhiHash(hashtable, p1);
    Redef(r, R_DEST, FALSE);
    if (r)
    {
        short buffer[1000],  *p = buffer;
        pstrcpy(buffer, (*r)->x.path);
        p += pstrlen(buffer);
        *p++ = ';';
        pstrcpy(p, buf);
        DeallocateMemory((*r)->x.path);
        (*r)->x.path = AllocateMemory(pstrlen(buffer) *2+2);
        pstrcpy((*r)->x.path, buffer);
    }
    else
    {
        REGISTRY *r;
        r = AllocateMemory(sizeof(REGISTRY));
        r->name = AllocateMemory(pstrlen(p1) *2+2);
        r->type = R_DEST;
        r->canchange = TRUE;
        pstrcpy(r->name, p1);
        r->x.path = AllocateMemory(pstrlen(buf) *2+2);
        pstrcpy(r->x.path, buf);
        AddPhiHash(hashtable, r);
    }
}

//-------------------------------------------------------------------------

void RegisterMacro(short *name, short *value, BOOL def, BOOL canchange)
{
    REGISTRY **r;
    short *p1,  *p2;
    p1 = stripspaces(name);
    p2 = stripspaces(value);
    r = LookupPhiHash(hashtable, p1);
    Redef(r, R_MACRO, FALSE);
    if (r)
    {
        if ((*r)->canchange)
        {
            DeallocateMemory((*r)->x.macro);
            (*r)->x.macro = AllocateMemory(pstrlen(p2) *2+2);
            pstrcpy((*r)->x.macro, p2);
            (*r)->isdef = def;
            (*r)->canchange = canchange;
        }
    }
    else
    {
        REGISTRY *r;
        r = AllocateMemory(sizeof(REGISTRY));
        r->x.macro = AllocateMemory(pstrlen(p2) *2+2);
        pstrcpy(r->x.macro, p2);
        r->name = AllocateMemory(pstrlen(p1) *2+2);
        pstrcpy(r->name, p1);
        r->isdef = def;
        r->type = R_MACRO;
        r->canchange = canchange;
        AddPhiHash(hashtable, r);
    }
}

//-------------------------------------------------------------------------

void RegisterImplicit(short *name, short **commands)
{
    REGISTRY **r,  *s;
    short *p1;
    p1 = stripspaces(name);
    r = LookupPhiHash(hashtable, name);
    Redef(r, R_IMPLICIT, FALSE);
    if (!r)
    {
        RegisterDest(name);
        s = AllocateMemory(sizeof(REGISTRY));
    }
    else
    {
        short **q = (*r)->x.commands;
        s =  *r;
        DeallocateMemory(s->name);
        while (*q)
            DeallocateMemory(*q++);
        DeallocateMemory(s->x.commands);
    }
    s->name = AllocateMemory(pstrlen(p1) *2+2);
    pstrcpy(s->name, p1);
    s->x.commands = commands;
    s->canchange = TRUE;
    s->type = R_IMPLICIT;
    if (!r)
        AddPhiHash(hashtable, s);
}

//-------------------------------------------------------------------------

void RegisterExplicitName(short *name, short *value)
{
    REGISTRY **r,  *s;
    short *p1,  *p2;
    p1 = stripspaces(name);
    p2 = stripspaces(value);
    RegisterTarget(p1, TRUE);
    r = LookupPhiHash(hashtable, name);
    Redef(r, R_EXPLICIT, TRUE);
    s = AllocateMemory(sizeof(REGISTRY));
    s->name = AllocateMemory(pstrlen(p1) *2+2);
    pstrcpy(s->name, p1);
//    if (!(s->depends = GetAutoDepends(name)))
//    {
        s->depends = AllocateMemory(pstrlen(p2) *2+2);
        pstrcpy(s->depends, p2);
//    }
    s->x.commands = 0;
    s->canchange = FALSE;
    s->type = R_EXPLICIT;
    AddPhiHash(hashtable, s);
}

//-------------------------------------------------------------------------

void RegisterExplicitCommands(short *name, short **commands)
{
    REGISTRY **r;
    short *p1;
    p1 = stripspaces(name);
    r = LookupPhiHash(hashtable, p1);
    (*r)->x.commands = commands;
}

//-------------------------------------------------------------------------

void RegisterInit(void)
{
    hashtable = CreateHashTable(HASH_TABLE_SIZE);
}
