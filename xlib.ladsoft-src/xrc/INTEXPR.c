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
/*
 * Evaluate an expression which should be known at compile time.
 * This uses recursive descent.  It is roughly analogous to the
 * primary expression handler except it returns a value rather than
 * an enode list
 */
#include <stdio.h>
#include <string.h>
#include "utype.h"
#include "preproc.h"
extern enum e_sym lastst;
extern char lastid[];
extern long ival;
extern TABLE defsyms;
extern int prm_cmangle;
extern SYM *typequal;
extern TABLE lsyms;
extern TABLE tagtable;
extern IFSTRUCT *ifs;
extern int inpp;

int basestyle;

static long ieprimary(void)
/*
 * PRimary integer
 *    id
 *    iconst
 *    (cast )intexpr
 *    (intexpr)
 */
{
    long temp = 0;
    SYM *sp;
    int needclose;
    if (lastst == eol)
        getsym();
    if (lastst == iconst)
    {
        temp = ival;
        getsym();
        return temp;
    } 
    else if (lastst == lconst)
    {
        temp = ival;
        getsym();
        return temp;
    }
    else if (lastst == iuconst)
    {
        temp = ival;
        getsym();
        return temp;
    }
    else if (lastst == luconst)
    {
        temp = ival;
        getsym();
        return temp;
    }
    else if (lastst == cconst)
    {
        temp = ival;
        getsym();
        return temp;
    }
    else if (lastst == openpa)
    {
        getsym();
        temp = intexpr();
        needpunc(closepa, 0);
        return (temp);
    }
    getsym();
    if (!inpp)
        generror(ERR_NEEDCONST, 0, 0);
    return 0;
}

/*
 * Integer unary
 *   - unary
 *   ! unary
 *   ~unary
 *   primary
 */
static long ieunary(void)
{
    long temp = basestyle;
    basestyle = 0;
    switch (lastst)
    {
        case minus:
            getsym();
            temp -= ieunary();
            break;
        case not:
            getsym();
            temp = !ieunary();
            break;
        case kw_not:
            getsym();
            temp &= ~ieunary();
            break;
        case compl:
            getsym();
            temp &= ~ieunary();
            break;
        default:
            temp |= ieprimary();
            break;
    }
    return (temp);
}

//-------------------------------------------------------------------------

static long iemultops(void)
/* Multiply ops */
{
    long val1 = ieunary(), val2;
    while (lastst == star || lastst == divide || lastst == modop)
    {
        long oper = lastst;
        getsym();
        val2 = ieunary();
        switch (oper)
        {
            case star:
                val1 = (unsigned long)val1 *(unsigned long)val2;
                break;
            case divide:
                val1 = (unsigned long)val1 / (unsigned long)val2;
                break;
            case modop:
                val1 = (unsigned long)val1 / (unsigned long)val2;
                break;
        }
    }
    return (val1);
}

//-------------------------------------------------------------------------

static long ieaddops(void)
/* Add ops */
{
    long val1 = iemultops(), val2;
    while (lastst == plus || lastst == minus)
    {
        long oper = lastst;
        getsym();
        if (oper == plus)
        {
            val1 += iemultops();
        }
        else
            val1 -= iemultops();
    }
    return (val1);
}

//-------------------------------------------------------------------------

static long ieshiftops(void)
/* Shift ops */
{
    long val1 = ieaddops(), val2;
    while (lastst == lshift || lastst == rshift)
    {
        long oper = lastst;
        getsym();
        val2 = ieaddops();
        if (oper == lshift)
            val1 <<= val2;
        else
        {
            unsigned long xx = val1;
            xx >>= val2;
            val1 = xx;
        }
    }
    return (val1);
}

//-------------------------------------------------------------------------

static long ierelation(void)
/* non-eq relations */
{
    long val1 = ieshiftops(), val2;
    while (lastst == lt || lastst == gt || lastst == leq || lastst == geq)
    {
        long oper = lastst;
        getsym();
        val2 = ieshiftops();
        switch (oper)
        {
            case lt:
                val1 = (unsigned long)val1 < (unsigned long)val2;
                break;
            case gt:
                val1 = (unsigned long)val1 > (unsigned long)val2;
                break;
            case leq:
                val1 = (unsigned long)val1 <= (unsigned long)val2;
                break;
            case geq:
                val1 = (unsigned long)val1 >= (unsigned long)val2;
                break;
        }
    }
    return (val1);
}

//-------------------------------------------------------------------------

static long ieequalops(void)
/* eq relations */
{
    long val1 = ierelation(), val2;
    while (lastst == eq || lastst == neq)
    {
        long oper = lastst;
        getsym();
        val2 = ierelation();
        if (oper == neq)
            val1 = val1 != val2;
        else
            val1 = val1 == val2;
    }
    return (val1);
}

//-------------------------------------------------------------------------

static long ieandop(void)
/* and op */
{
    long val1 = ieequalops(), val2;
    while (lastst == and)
    {
        getsym();
        val2 = ieequalops();
        val1 = val1 &val2;
    }
    return (val1);
}

//-------------------------------------------------------------------------

static long iexorop(void)
/* xor op */
{
    long val1 = ieandop(), val2;
    while (lastst == uparrow)
    {
        getsym();
        val2 = ieandop();
        val1 = val1 ^ val2;
    }
    return (val1);
}

//-------------------------------------------------------------------------

static long ieorop(void)
/* or op */
{
    long val1 = iexorop(), val2;
    while (lastst == or)
    {
        getsym();
        if (lastst == eol)
            getsym();
        basestyle = val1;
        val1 = iexorop();
    }
    return (val1);
}

//-------------------------------------------------------------------------

static long ielandop(void)
/* logical and op */
{
    long val1 = ieorop(), val2;
    while (lastst == land)
    {
        getsym();
        val2 = ieorop();
        val1 = val1 && val2;
    }
    return (val1);
}

//-------------------------------------------------------------------------

static long ielorop(void)
/* logical or op */
{
    long val1 = ielandop(), val2;
    while (lastst == lor)
    {
        getsym();
        val2 = ielandop();
        val1 = val1 || val2;
    }
    return (val1);
}

//-------------------------------------------------------------------------

static long iecondop(void)
/* Hook op */
{
    long val1 = ielorop(), val2, val3;
    if (lastst == hook)
    {
        getsym();
        val2 = iecondop();
        needpunc(colon, 0);
        val3 = iecondop();
        if (val1)
            val1 = val2;
        else
            val1 = val3;
    }
    return (val1);
}

//-------------------------------------------------------------------------

long intexpr(void)
/* Integer expressions */
{
    long val;
    val = iecondop();
    return val;
}
