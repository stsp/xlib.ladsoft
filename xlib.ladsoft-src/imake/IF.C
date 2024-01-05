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
#include "utype.h"
#include "umem.h"
#include "if.h"
#include "input.h"
#include "parser.h"
#include "register.h"
#include "macros.h"

extern short *bufptr;
extern HASHREC **hashtable;

BOOL ifskip = FALSE;
LIST *iflist = 0;

static BOOL definedmacro(void)
{
    short buf[200],  *p = buf;
    BOOL rv;
    skipSpace(&bufptr);

    while (*bufptr && !iswhitespacechar(*bufptr))
        *p++ =  *bufptr++;
    bufptr++;
    *p = 0;

    rv = (FindMacro(buf) != 0);
    skipSpace(&bufptr);
    if (*bufptr)
        Extraneous();
    return (rv);
}

//-------------------------------------------------------------------------

static void BadIf(void)
{
    if (!iflist)
        Error("Missing !if");
}

//-------------------------------------------------------------------------

static void ifpush(BOOL flag)
{
    LIST *h = AllocateMemory(sizeof(LIST));
    h->data = (void*)ifskip;
    h->link = iflist;
    iflist = h;
    ifskip = !flag;
}

//-------------------------------------------------------------------------

static void ifpop(void)
{
    LIST *h = iflist;
    BadIf();
    iflist = h->link;
    ifskip = (BOOL)(h->data);
    DeallocateMemory(h);
}

//-------------------------------------------------------------------------

void ifdef(void)
{
    ifpush(definedmacro());
}

//-------------------------------------------------------------------------

void ifndef(void)
{
    ifpush(!definedmacro());
}

//-------------------------------------------------------------------------

void ifelse(void)
{
    BadIf();
    ifskip = !ifskip;
}

//-------------------------------------------------------------------------

void ifendif(void)
{
    ifpop();
}
