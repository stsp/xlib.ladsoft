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
#include "register.h"
#include "macros.h"
#include "input.h"
#include "interp.h"
#include "phash.h"

extern HASHREC **hashtable;

static void BadRepExt(void)
{
    Error("Bad Replacement String");
}

//-------------------------------------------------------------------------

REGISTRY *FindMacro(short *name)
{
    REGISTRY **r = LookupPhiHash(hashtable, name);
    if (r && (*r) && (*r)->type == R_MACRO && (*r)->isdef)
        return (*r);
    return (0);
}

//-------------------------------------------------------------------------

short *ExpandString(short *string)
{
    BOOL changed;
    short *ibuf;
    ibuf = AllocateMemory(INTERNALBUFSIZE *sizeof(short));
    do
    {
        short *d = ibuf,  *s = string;
        short macname[100], ext1[100], ext2[100],  *n;
        REGISTRY *r;
        changed = FALSE;
        while (*s)
        {
            if (*s == MACROSTART && *(s + 1) == MACROLEFT1)
            {
                s += 2;
                goto macroexpand;
            }
            else
            {
                if (*s == MACROLEFT2)
                {
                    s++;
                    macroexpand: changed = TRUE;
                    n = macname;
                    while (*s &&  *s != MACRORIGHT1 &&  *s != MACRORIGHT2 && 
                        *s != COLON &&  *s != COLON2)
                        *n++ =  *s++;
                    *n = 0;
                    switch (*s)
                    {
                        case MACRORIGHT1:
                        case MACRORIGHT2:
                            s++;
                            r = FindMacro(macname);
                            if (r)
                            {
                                short *s = r->x.macro;
                                while (*s)
                                    *d++ =  *s++;
                            }
                            break;
                        case COLON:
                        case COLON2:
                            s++;
                            r = FindMacro(macname);
                            n = ext1;
                            while (*s && (*s != EQUAL &&  *s != ALTEQUAL && n -
                                ext1 < 19))
                                *n++ =  *s++;
                            *n = 0;
                            if (*s != EQUAL &&  *s != ALTEQUAL)
                                BadRepExt();
                            if (*s)
                            {
                                s++;
                                n = ext2;
                                while (*s && (*s != MACRORIGHT1 &&  *s !=
                                    MACRORIGHT2 && n - ext2 < 19))
                                    *n++ =  *s++;
                                *n = 0;
                            }
                            if (*s)
                                s++;
                            n = d;
                            if (r)
                            {
                                short *s = r->x.macro;
                                while (*s)
                                {
                                    if (!pstrncmp(s, ext1, pstrlen(ext1)))
                                    {
                                        pstrncpy(d, ext2, pstrlen(ext2));
                                        d += pstrlen(ext2);
                                        s += pstrlen(ext1);
                                    }
                                    else
                                        *d++ =  *s++;
                                }
                            }
                            *d = 0;
                            break;
                        default:
                            Error("Unterminated macro");
                    }
                }
                else
                {
                    *d++ =  *s++;
                }
            }
        }
        *d = 0;
        pstrcpy(string, ibuf);
    }
    while (changed)
        ;
    DeallocateMemory(ibuf);
    return (string);
}

//-------------------------------------------------------------------------

short *ExpandSelf(short *name, short *value)
{
    REGISTRY *r;
    short *ibuf = AllocateMemory(INTERNALBUFSIZE *sizeof(short));
    short *d = ibuf,  *s = value,  *xs;
    short macname[100],  *n, ext1[100], ext2[100];
    pstrcpy(macname, name);
    r = FindMacro(macname);
    while (*s)
    {
        xs = s;
        if (*s == MACROSTART && *(s + 1) == MACROLEFT1)
        {
            s += 2;
            goto macroexpand;
        }
        else
        {
            if (*s == MACROLEFT2)
            {
                s++;
                macroexpand: n = macname;
                while (*s &&  *s != MACRORIGHT1 &&  *s != MACRORIGHT2 &&  *s !=
                    COLON &&  *s != COLON2)
                    *n++ =  *s++;
                *n = 0;
                if (pstrcmp(macname, name))
                {
                    while (xs != s)
                        *d++ =  *xs++;
                    continue;
                }
                switch (*s)
                {
                    case MACRORIGHT1:
                    case MACRORIGHT2:
                        s++;
                        if (r)
                        {
                            short *s = r->x.macro;
                            while (*s)
                                *d++ =  *s++;
                        }
                        break;
                    case COLON:
                    case COLON2:
                        s++;
                        n = ext1;
                        while (*s && (*s != EQUAL &&  *s != ALTEQUAL && n -
                            ext1 < 19))
                            *n++ =  *s++;
                        *n = 0;
                        if (*s != EQUAL &&  *s != ALTEQUAL)
                            BadRepExt();
                        if (*s)
                        {
                            s++;
                            n = ext2;
                            while (*s && (*s != MACRORIGHT1 &&  *s !=
                                MACRORIGHT2 && n - ext2 < 19))
                                *n++ =  *s++;
                            *n = 0;
                        }
                        if (*s)
                            s++;
                        n = d;
                        if (r)
                        {
                            short *s = r->x.macro;
                            while (*s)
                            {
                                if (!pstrncmp(s, ext1, pstrlen(ext1)))
                                {
                                    pstrncpy(d, ext2, pstrlen(ext2));
                                    d += pstrlen(ext2);
                                    s += pstrlen(ext1);
                                }
                                else
                                    *d++ =  *s++;
                            }
                        }
                        *d = 0;
                        break;
                    default:
                        Error("Unterminated macro");
                }
            }
            else
            {
                *d++ =  *s++;
            }
        }
    }
    *d = 0;
    pstrcpy(value, ibuf);
    DeallocateMemory(ibuf);
    return (value);

}
