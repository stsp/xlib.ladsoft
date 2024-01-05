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
/*
 * iconst.c
 *
 * Constant folding.  Is done at the same time as CSEs.
 *
 */
#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include "utype.h"	
#include "cmdline.h"	
#include "lists.h"
#include "expr.h"
#include "c.h"
#include "iexpr.h"
#include "iopt.h"
#include "diag.h"

extern int prm_foldconst;
extern **name_hash;

#ifdef XXXXX
IMODE *intconst(LLONG_TYPE val)
{
    return make_immed(val);
}
#endif

//-------------------------------------------------------------------------

void badconst(void)
{
    DIAG("ICODE- Bad constant node");
}

void ReassignMulDiv(QUAD *d, enum e_iops op, LLONG_TYPE val, int fromleft)
{
	IMODE *ip = make_immed(val);
	if (fromleft) {
		d->dc.left = d->dc.right;
		if (d->livein & IM_LIVERIGHT)
			d->livein = IM_LIVELEFT;
		else
			d->livein = 0;
	}
	if (!ToQuadConst(&ip))
		d->livein |= IM_LIVERIGHT;
	d->dc.right = ip;	
	d->dc.opcode = op;
}
void ReassignInt(QUAD *d, LLONG_TYPE val)
{
	IMODE *ip = make_immed(val);
	if (ToQuadConst(&ip))
		d->livein = IM_LIVERIGHT;
	else
		d->livein = IM_LIVELEFT | IM_LIVERIGHT;
	d->dc.left = ip ;
	d->dc.right = 0;
	d->dc.opcode = i_assn;
}
void ReassignFloat(QUAD *d, long double val)
{
	IMODE *ip = make_fimmed(val);
	if (ToQuadConst(&ip))
		d->livein = IM_LIVERIGHT;
	else
		d->livein = IM_LIVELEFT | IM_LIVERIGHT;
	d->dc.left = ip ;
	d->dc.right = 0;
	d->dc.opcode = i_assn;
}
void ReassignCompare(QUAD *d, int yes)
{
    if (d->dc.opcode >= i_jc) {
        if (yes)
            d->dc.opcode = i_goto;
        else {
            d->dc.opcode = i_nop;
        }
    } else {
        ReassignInt(d,yes);
    }
}
void ASNFromLeft(QUAD *d)
{
	d->dc.right = 0;
	d->livein |= IM_LIVERIGHT;
	d->dc.opcode = i_assn;
}
void ASNFromRight(QUAD *d)
{
	d->dc.left = d->dc.right;
	d->dc.right = 0;
    if (d->livein & IM_LIVERIGHT)
    	d->livein = IM_LIVERIGHT | IM_LIVELEFT;
    else
        d->livein = IM_LIVERIGHT;
	d->dc.opcode = i_assn;
}
//-------------------------------------------------------------------------
/*
 * when we get here the quad node will have node replacements
 * for named nodes done.  Constants will always have been replaced
 * from the named node describing the constant at this point.
 * If either or both of the
 * node arguments are constants the QUAD left and or right pointers
 * will point to QUAD nodes describing the constants and the
 * appropriate bits in the livein member of the quad will be clear
 */
static int xgetmode(QUAD *d, union ival **left, union ival **right)
{
    int mode = icnone;
    QUAD *ql = d->dc.left,  *qr = d->dc.right;
    *left =  *right = 0;
    if (!(d->livein &IM_LIVELEFT) && (ql && (ql->dc.opcode == i_icon || ql
        ->dc.opcode == i_fcon || ql->dc.opcode == i_imcon || ql->dc.opcode == i_cxcon)))
        *left = &(ql->dc.v);
    if (!(d->livein &IM_LIVERIGHT) && (qr && (qr->dc.opcode == i_icon || qr
        ->dc.opcode == i_fcon || qr->dc.opcode == i_imcon || qr->dc.opcode == i_cxcon)))
        *right = &(qr->dc.v);
	if (*left) {
		if (*right) {
			switch(ql->dc.opcode) {
				case i_icon:
					switch(qr->dc.opcode) {
						case i_icon:
							mode = icll;
							break ;
						case i_fcon:
							mode = iclr;
							break ;
						case i_imcon:
							mode = icli;
							break ;
						case i_cxcon:
							mode = iclc;
							break ;
					}
					break ;
				case i_fcon:
					switch(qr->dc.opcode) {
						case i_icon:
							mode = icrl;
							break ;
						case i_fcon:
							mode = icrr;
							break ;
						case i_imcon:
							mode = icri;
							break ;
						case i_cxcon:
							mode = icrc;
							break ;
					}
					break ;
				case i_imcon:
					switch(qr->dc.opcode) {
						case i_icon:
							mode = icil;
							break ;
						case i_fcon:
							mode = icir;
							break ;
						case i_imcon:
							mode = icii;
							break ;
						case i_cxcon:
							mode = icic;
							break ;
					}
					break ;
				case i_cxcon:
					switch(qr->dc.opcode) {
						case i_icon:
							mode = iccl;
							break ;
						case i_fcon:
							mode = iccr;
							break ;
						case i_imcon:
							mode = icci;
							break ;
						case i_cxcon:
							mode = iccc;
							break ;
					}
					break ;
			}
		} else {
			switch(ql->dc.opcode) {
				case i_icon:
					mode = icln;
					break ;
				case i_fcon:
					mode = icrn;
					break ;
				case i_imcon:
					mode = icin;
					break ;
				case i_cxcon:
					mode = iccn;
					break ;
				default:
					break ;
			}
		}
	} else if (*right) {
		switch(qr->dc.opcode) {
			case i_icon:
				mode = icnl;
				break ;
			case i_fcon:
				mode = icnr;
				break ;
			case i_imcon:
				mode = icni;
				break ;
			case i_cxcon:
				mode = icnc;
				break ;
			default:
				break ;
		}
	}
    return (mode);
}
//-------------------------------------------------------------------------
LLONG_TYPE calmask(int i)
{
	if (pwrof2(i) != -1)
		return i - 1;
	return (LLONG_TYPE)-1;
}
void ReCast(int size, QUAD *in, QUAD *newMode)
{
	if (size) {
		if (size <= ISZ_ULONGLONG) {
			LLONG_TYPE i;
			if (in->dc.opcode == i_icon)
				i = CastToInt(size, 	in->dc.v.i);
			else
				i = CastToInt(size, 	in->dc.v.f);
			if (!newMode) {
				in->dc.v.i = i;
				return in;
			} else {
				memset(&newMode->dc,0,sizeof(newMode->dc));
				newMode->dc.opcode = i_icon;
				newMode->dc.v.i = i;
				newMode->ans = make_immed(i);
				return newMode;
			}
		} else if (size >= ISZ_FLOAT) {
			long double f;
			if (in->dc.opcode == i_icon)
				f = CastToFloat(size, 	in->dc.v.i);
			else
				f = CastToFloat(size, 	in->dc.v.f);
			if (!newMode) {
				in->dc.v.f = f;
				return in;
			} else {
				memset(&newMode->dc,0,sizeof(newMode->dc));
				newMode->dc.opcode = i_fcon;
				newMode->dc.v.f = f;
				newMode->ans = make_fimmed(f);
				return newMode;
			}
		} else
			return in;
	}
	return in;
}
//-------------------------------------------------------------------------
void ConstantFold(QUAD *d)
{
    int index, shift;
    union ival *left = 0,  *right = 0;
    if (!prm_foldconst)
        return ;
    switch (d->dc.opcode)
    {
		case i_setne:
		case i_jne:
            index = xgetmode(d, &left, &right);
            switch(index)
            {
            case icll:
                ReassignCompare(d,left->i != right->i);
                break;
            case iclr:
                ReassignCompare(d,left->i != right->f);
                break;
            case icrl:
                ReassignCompare(d,left->f != right->i);
                break;
            case icrr:
                ReassignCompare(d,left->f != right->f);
                break;
            }		
            break;
		case i_sete:
		case i_je:
            index = xgetmode(d, &left, &right);
            switch(index)
            {
            case icll:
                ReassignCompare(d,left->i == right->i);
                break;
            case iclr:
                ReassignCompare(d,left->i == right->f);
                break;
            case icrl:
                ReassignCompare(d,left->f == right->i);
                break;
            case icrr:
                ReassignCompare(d,left->f == right->f);
                break;
            }		
            break;
		case i_setc:
		case i_jc:
            index = xgetmode(d, &left, &right);
            switch(index)
            {
            case icll:
                if (right->i == 0)
                    ReassignCompare(d,0);
                else
                    ReassignCompare(d,(unsigned LLONG_TYPE)left->i < (unsigned LLONG_TYPE)right->i);
                break;
            case iclr:
                if (right->f == 0)
                    ReassignCompare(d,0);
                else
                    ReassignCompare(d,(unsigned LLONG_TYPE)left->i < right->f);
                break;
            case icrl:
                if (right->i == 0)
                    ReassignCompare(d,0);
                else
                    ReassignCompare(d,left->f < (unsigned LLONG_TYPE)right->i);
                break;
            case icrr:
                if (right->f == 0)
                    ReassignCompare(d,0);
                else
                    ReassignCompare(d,left->f < right->f);
                break;
            }		
            break;
		case i_setbe:
		case i_jbe:
            index = xgetmode(d, &left, &right);
            switch(index)
            {
            case icll:
                ReassignCompare(d,(unsigned LLONG_TYPE)left->i <= (unsigned LLONG_TYPE)right->i);
                break;
            case iclr:
                ReassignCompare(d,(unsigned LLONG_TYPE)left->i <= right->f);
                break;
            case icrl:
                ReassignCompare(d,left->f <= (unsigned LLONG_TYPE)right->i);
                break;
            case icrr:
                ReassignCompare(d,left->f <= right->f);
                break;
            }		
            break;
		case i_seta:
		case i_ja:
            index = xgetmode(d, &left, &right);
            switch(index)
            {
            case icll:
                ReassignCompare(d,(unsigned LLONG_TYPE)left->i > (unsigned LLONG_TYPE)right->i);
                break;
            case iclr:
                ReassignCompare(d,(unsigned LLONG_TYPE)left->i > right->f);
                break;
            case icrl:
                ReassignCompare(d,left->f > (unsigned LLONG_TYPE)right->i);
                break;
            case icrr:
                ReassignCompare(d,left->f > right->f);
                break;
            }		
            break;
		case i_setnc:
		case i_jnc:
            index = xgetmode(d, &left, &right);
            switch(index)
            {
            case icll:
                ReassignCompare(d,(unsigned LLONG_TYPE)left->i >= (unsigned LLONG_TYPE)right->i);
                break;
            case iclr:
                ReassignCompare(d,(unsigned LLONG_TYPE)left->i >= right->f);
                break;
            case icrl:
                ReassignCompare(d,left->f >= (unsigned LLONG_TYPE)right->i);
                break;
            case icrr:
                ReassignCompare(d,left->f >= right->f);
                break;
            }		
            break;
		case i_setl:
		case i_jl:
            index = xgetmode(d, &left, &right);
            switch(index)
            {
            case icll:
                ReassignCompare(d,left->i < right->i);
                break;
            case iclr:
                ReassignCompare(d,left->i < right->f);
                break;
            case icrl:
                ReassignCompare(d,left->f < right->i);
                break;
            case icrr:
                ReassignCompare(d,left->f < right->f);
                break;
            }		
            break;
		case i_setle:
		case i_jle:
            index = xgetmode(d, &left, &right);
            switch(index)
            {
            case icll:
                ReassignCompare(d,left->i <= right->i);
                break;
            case iclr:
                ReassignCompare(d,left->i <= right->f);
                break;
            case icrl:
                ReassignCompare(d,left->f <= right->i);
                break;
            case icrr:
                ReassignCompare(d,left->f <= right->f);
                break;
            }		
            break;
		case i_setg:
		case i_jg:
            index = xgetmode(d, &left, &right);
            switch(index)
            {
            case icll:
                ReassignCompare(d,left->i > right->i);
                break;
            case iclr:
                ReassignCompare(d,left->i > right->f);
                break;
            case icrl:
                ReassignCompare(d,left->f > right->i);
                break;
            case icrr:
                ReassignCompare(d,left->f > right->f);
                break;
            }		
            break;
		case i_setge:
		case i_jge:
            index = xgetmode(d, &left, &right);
            switch(index)
            {
            case icll:
                ReassignCompare(d,left->i >= right->i);
                break;
            case iclr:
                ReassignCompare(d,left->i >= right->f);
                break;
            case icrl:
                ReassignCompare(d,left->f >= right->i);
                break;
            case icrr:
                ReassignCompare(d,left->f >= right->f);
                break;
            }		
            break;
#ifdef XXXXX
        case i_add:
            index = xgetmode(d, &left, &right);
            switch (index)
            {
            case icll:
                ReassignInt(d,left->i + right->i);
                break;
            case iclr:
                ReassignFloat(d,left->i + right->f);
                break;
            case icrl:
                ReassignFloat(d,left->f + right->i);
                break;
            case icrr:
                ReassignFloat(d,left->f + right->f);
                break;
            case icln:
                if (left->i == 0)
                {
					ASNFromRight(d);
                }
                break;
            case icrn:
                if (left->f == 0)
                {
					ASNFromRight(d);
                }
                break;
            case icnl:
                if (right->i == 0)
                {
					ASNFromLeft(d);
                }
                break;
            case icnr:
                if (right->f == 0)
                {
					ASNFromLeft(d);
                }
                break;
            }
            break;
        case i_sub:
            index = xgetmode(d, &left, &right);
            switch (index)
            {
            case icnone:
                if (d->dc.left == d->dc.right)
                {
					ReassignInt(d,0);
                }
                break;
            case icll:
				ReassignInt(d,left->i - right->i);
                break;
            case iclr:
				ReassignFloat(d,left->i - right->f);
                break;
            case icrl:
				ReassignFloat(d,left->f - right->i);
                break;
            case icrr:
				ReassignFloat(d,left->f - right->f);
                break;
            case icln:
                if (left->i == 0)
                {
					ASNFromLeft(d);
                }
                break;
            case icrn:
                if (left->f == 0)
                {
                }
                break;
            case icnl:
                if (right->i == 0)
                {
                }
                break;
            case icnr:
                if (right->f == 0)
                {
                }
                break;
            }
            break;
        case i_udiv:
            index = xgetmode(d, &left, &right);
            switch (index)
            {
            case icll:
                if ((unsigned LLONG_TYPE)right->i != 0)
					ReassignInt(d,(unsigned LLONG_TYPE)left->i / (unsigned LLONG_TYPE)right->i);
                else
                    d->dc.v.i = 0;
                break;
            case iclr:
                if (right->f != 0)
					ReassignFloat(d,(unsigned LLONG_TYPE)left->i / right->f);
                else
                    d->dc.v.f = 0;
                break;
            case icrl:
                if ((unsigned LLONG_TYPE)right->i != 0)
					ReassignFloat(d,left->f / (unsigned LLONG_TYPE)right->i);
                else
                    d->dc.v.f = 0;
                break;
            case icrr:
                if (right->f != 0)
					ReassignFloat(d,left->f / right->f);
                else
                    d->dc.v.f = 0;
                break;
            case icln:
                if ((unsigned LLONG_TYPE)left->i == 0)
                {
					ReassignInt(d,0);
                }
                break;
            case icrn:
                if (left->f == 0)
                {
					ReassignFloat(d,0);
                }
                break;
            case icnl:
                if ((unsigned LLONG_TYPE)right->i == 1)
                {
					ASNFromLeft(d);
                }
                else if ((shift = pwrof2((unsigned LLONG_TYPE)right->i)) !=  - 1)
                {
					ReassignMulDiv(d,i_lsr,shift,FALSE);
                }
                break;
            case icnr:
                if (right->f == 1)
                {
					ASNFromLeft(d);
                }
                break;
            }
            break;
        case i_umod:
            index = xgetmode(d, &left, &right);
            switch (index)
            {
            case icnone:
                break;
            case icll:
                if ((unsigned LLONG_TYPE)right->i != 0)
					ReassignInt(d,(unsigned LLONG_TYPE)left->i % (unsigned LLONG_TYPE)right->i);
                else
                    d->dc.v.i = 0;
            case icnl:
                if (shift = calmask((unsigned LLONG_TYPE)right->i))
                {
					ReassignMulDiv(d,i_and,shift,FALSE);
                }
                break;
            default:
                badconst();
                break;
            }
            break;
        case i_sdiv:
            index = xgetmode(d, &left, &right);
            switch (index)
            {
            case icll:
                if (right->i != 0)
					ReassignInt(d,left->i / right->i);
                else
                    d->dc.v.i = 0;
                break;
            case iclr:
                if (right->f != 0)
					ReassignFloat(d,left->i / right->f);
                else
                    d->dc.v.f = 0;
                break;
            case icrl:
                if (right->i != 0)
					ReassignFloat(d,left->f / right->i);
                else
                    d->dc.v.f = 0;
                break;
            case icrr:
                if (right->f != 0)
					ReassignFloat(d,left->f / right->f);
                else
                    d->dc.v.f = 0;
                break;
            case icln:
                if (left->i == 0)
                {
					ReassignInt(d,0);
                }
                break;
            case icrn:
                if (left->f == 0)
                {
					ReassignFloat(d,0);
                }
                break;
            case icnl:
                if (right->i == 1)
                {
					ASNFromLeft(d);
                }
                else if ((shift = pwrof2(right->i)) !=  - 1)
                {
					ReassignMulDiv(d,i_lsr,shift,FALSE);
                }
                break;
            case icnr:
                if (right->f == 1)
                {
					ASNFromLeft(d);
                }
                break;
            }
            break;
        case i_smod:
            index = xgetmode(d, &left, &right);
            switch (index)
            {
            case icnone:
                break;
            case icll:
                if (right->i != 0)
					ReassignInt(d,left->i % right->i);
                else
                    d->dc.v.i = 0;
                break;
            case icln:
                if (left->i == 0)
                {
					ReassignInt(d,0);
                }
                break;
            case icnl:
                if (shift = calmask(right->i))
                {
					ReassignMulDiv(d,i_and,shift,FALSE);
                }
                break;
            default:
                badconst();
                break;
            }
            break;
        case i_umul:
		case i_arrayindex:
            index = xgetmode(d, &left, &right);
            switch (index)
            {
            case icll:
				ReassignInt(d,(unsigned LLONG_TYPE)left->i *(unsigned LLONG_TYPE)right->i);
                break;
            case iclr:
				ReassignFloat(d,(unsigned LLONG_TYPE)left->i * right->f);
                break;
            case icrl:
				ReassignFloat(d,left->f *(unsigned LLONG_TYPE)right->i);
                break;
            case icrr:
				ReassignFloat(d,left->f * right->f);
                break;
            case icln:
                if ((unsigned LLONG_TYPE)left->i == 0)
                {
					ReassignInt(d,0);
                }
                else if ((unsigned LLONG_TYPE)left->i == 1)
                {
					ASNFromRight(d);
                }
                else if ((shift = pwrof2((unsigned LLONG_TYPE)left->i)) !=  - 1)
                {
					ReassignMulDiv(d,i_lsl,shift,TRUE);                }

                break;
            case icrn:
                if (left->f == 0)
                {
					ReassignFloat(d,0);
                }
                else if ((unsigned LLONG_TYPE)left->i == 1)
                {
					ASNFromRight(d);
                }
                break;
            case icnl:
                if ((unsigned LLONG_TYPE)right->i == 0)
                {
					ReassignInt(d,0);
                }
                else if ((unsigned LLONG_TYPE)right->i == 1)
                {
					ASNFromLeft(d);
                }
                else if ((shift = pwrof2((unsigned LLONG_TYPE)right->i)) !=  - 1)
                {
					ReassignMulDiv(d,i_lsl,shift,FALSE);
                }
                break;
            case icnr:
                if (right->f == 0)
                {
					ReassignFloat(d,0);
                }
                else if ((unsigned LLONG_TYPE)right->i == 1)
                {
					ASNFromLeft(d);
                }
                break;
            }
            break;
        case i_smul:
            index = xgetmode(d, &left, &right);
            switch (index)
            {
            case icll:
				ReassignInt(d,left->i * right->i);
                break;
            case iclr:
				ReassignFloat(d,left->i * right->f);
                break;
            case icrl:
				ReassignFloat(d,left->f * right->i);
                break;
            case icrr:
				ReassignFloat(d,left->f * right->f);
                break;
            case icln:
                if (left->i == 0)
                {
					ReassignInt(d,0);
                }
                else if (left->i == 1)
                {
					ASNFromRight(d);
                }
                else if ((shift = pwrof2(left->i)) !=  - 1)
                {
					ReassignMulDiv(d,i_lsl,shift,TRUE);
                }

                break;
            case icrn:
                if (left->f == 0)
                {
					ReassignFloat(d,0);
                }
                else if (left->i == 1)
                {
					ASNFromRight(d);
                }
                break;
            case icnl:
                if (right->i == 0)
                {
					ReassignInt(d,0);
                }
                else if (right->i == 1)
                {
					ASNFromLeft(d);
                }
                else if ((shift = pwrof2(right->i)) !=  - 1)
                {
					ReassignMulDiv(d,i_lsl,shift,FALSE);
                }
                break;
            case icnr:
                if (right->f == 0)
                {
					ReassignFloat(d,0);
                }
                else if (right->i == 1)
                {
					ASNFromLeft(d);
                }
                break;
            }
            break;
        case i_lsl:
            index = xgetmode(d, &left, &right);
            switch (index)
            {
            case icnone:
                break;
            case icll:
                ReassignInt(d,(unsigned LLONG_TYPE)left->i << (unsigned LLONG_TYPE)right->i);
                break;
            case icln:
                if (left->i == 0)
                {
					ReassignInt(d,0);
                }
                break;
            case icnl:
                if (right->i == 0)
                {
					ASNFromLeft(d);
                }
                break;
            default:
                badconst();
                break;
            }
            break;
        case i_lsr:
            index = xgetmode(d, &left, &right);
            switch (index)
            {
            case icnone:
                break;
            case icll:
                ReassignInt(d, (unsigned LLONG_TYPE)left->i >> (unsigned LLONG_TYPE)right->i);
                break;
            case icln:
                if (left->i == 0)
                {
					ReassignInt(d,0);
                }
                break;
            case icnl:
                if (right->i == 0)
                {
					ASNFromLeft(d);
                }
                break;
            default:
                badconst();
                break;
            }
            break;
        case i_asl:
            index = xgetmode(d, &left, &right);
            switch (index)
            {
            case icnone:
                break;
            case icll:
                ReassignInt(d, left->i << right->i);
                break;
            case icln:
                if (left->i == 0)
                {
					ReassignInt(d,0);
                }
                break;
            case icnl:
                if (right->i == 0)
                {
					ASNFromLeft(d);
                }
                break;
            default:
                badconst();
                break;
            }
            break;
        case i_asr:
            index = xgetmode(d, &left, &right);
            switch (index)
            {
            case icnone:
                break;
            case icll:
                ReassignInt(d, left->i >> right->i);
                break;
            case icln:
                if (left->i == 0)
                {
					ReassignInt(d,0);
                }
                break;
            case icnl:
                if (right->i == 0)
                {
					ASNFromLeft(d);
                }
                break;
            default:
                badconst();
                break;
            }
            break;
        case i_and:
            index = xgetmode(d, &left, &right);
            switch (index)
            {
            case icnone:
                break;
            case icll:
				ReassignInt(d,left->i & right->i);
                break;
            case icln:
                if (left->i == 0)
                {
					ReassignInt(d,0);
                }
                break;
            case icnl:
                if (right->i == 0)
                {
					ReassignInt(d,0);
                }
                break;
            default:
                badconst();
                break;
            }
            break;
        case i_or:
            index = xgetmode(d, &left, &right);
            switch (index)
            {
            case icnone:
                break;
            case icll:
                ReassignInt(d, left->i | right->i);
                break;
            case icln:
                if (left->i == 0)
                {
					ASNFromRight(d);
                }
                break;
            case icnl:
                if (right->i == 0)
                {
					ASNFromLeft(d);
                }
                break;
            default:
                badconst();
                break;
            }
            break;
        case i_eor:
            index = xgetmode(d, &left, &right);
            switch (index)
            {
            case icnone:
                break;
            case icll:
				ReassignInt(d, left->i ^ right->i);
                break;
            case icln:
                if (left->i == 0)
                {
					ASNFromRight(d);
                }
                break;
            case icnl:
                if (right->i == 0)
                {
					ASNFromLeft(d);
                }
                break;
            default:
                badconst();
                break;
            }
            break;
        case i_neg:
            index = xgetmode(d, &left, &right);
            switch (index)
            {
            case icnone:
                break;
            case icln:
				ReassignInt(d,-d->dc.v.i);
                break;
            case icrn:
				ReassignFloat(d,-d->dc.v.f);
                break;
            }
            break;
        case i_not:
            index = xgetmode(d, &left, &right);
            switch (index)
            {
            case icnone:
                break;
            case icln:
				ReassignInt(d,~d->dc.v.i);
                break;
			}
            break;
    }
	if (d->dc.opcode == i_assn) {
			// propagate constants and resolve casts to constants
			if (d->ans->mode == i_direct && d->ans->offset->nodetype == en_tempref && !(d->livein & IM_LIVELEFT)) {
				QUAD *q = d->dc.left ;
				if (q->dc.opcode == i_icon || q->dc.opcode == i_fcon || q->dc.opcode == i_imcon || q->dc.opcode == i_cxcon) {
					ReCast(d->ans->size, q, d->dc.left );					
				} else if (q->dc.opcode == i_assn && !(q->livein & IM_LIVELEFT)) {
					q = q->dc.left;
					if (q->dc.opcode == i_icon || q->dc.opcode == i_fcon || q->dc.opcode == i_imcon || q->dc.opcode == i_cxcon) {
						if (d->ans->size == d->dc.left->size)
							d->dc.left = q;
						else
							ReCast(d->ans->size, q, d->dc.left);
					}
				}
			}
#endif
	}
}
