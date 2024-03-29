/* 
CCIDE
Copyright 2001-2011 David Lindauer.

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

#include <windows.h>
#include <commctrl.h>
#include <commdlg.h>
#include <richedit.h>
#include <stdio.h>
#include <limits.h>
#include <float.h>

#include "header.h"
#include "cvinfo.h"
#include <ctype.h>

#define PUSH(xxx) varinfo_list[varinfo_count++] = xxx
#define POP()  varinfo_list[--varinfo_count] 


static VARINFO *varinfo_list[200];
static int varinfo_count;

extern THREAD *activeThread;
extern unsigned bitmask[];

#define NVAL 0
#define IVAL 1
#define IUVAL 2
#define RVAL 4

LLONG_TYPE ival;
long double rval;
int lastst;

    static VARINFO *iecondop(char **text, char **typetab, char **symtab,
        DEBUG_INFO *dbg, SCOPE *sc, int towarn);
    static void freeall(void)
    {
        int i;
        for (i = 0; i < varinfo_count; i++)
            FreeVarInfo(varinfo_list[i]);
        varinfo_count = 0;
    }
    static void skipspace(char **text)
    {
        while (**text && isspace(**text))
            (*text)++;
    }
    static VARINFO *ieerr(char **text, VARINFO *v1, VARINFO *v2, char *msg, int towarn)
    {
        if (v1)
            FreeVarInfo(v1);
        if (v2)
            FreeVarInfo(v2);
        freeall();
		if (towarn)
		{
	        if (text)
	            (*text) += strlen(*text);
	        ExtendedMessageBox("Invalid Expression", MB_SETFOREGROUND |
	            MB_SYSTEMMODAL, "%s", msg);
		}
        return 0;
    }
    static int radix36(char c)
    {
        if (isdigit(c))
            return c - '0';
        if (c >= 'a' && c <= 'z')
            return c - 'a' + 10;
        if (c >= 'A' && c <= 'Z')
            return c - 'A' + 10;
        return INT_MAX;
    }

    /*
     *      getbase - get an integer in any base.
     */
    static void getbase(int b, char **ptr, int towarn)
    {
        LLONG_TYPE i;
        int j;
        int errd = 0;
        i = 0;
        while ((j = radix36(**ptr)) < b)
        {
            (*ptr)++;
            if (i > (unsigned LLONG_TYPE)(((LLONG_TYPE) - 1) - j) / b)
            if (!errd)
            {
                ieerr(0, 0, 0, "Constant too large", towarn);
                errd++;
            }
            i = i * b + j;
        }
        ival = i;
        lastst = IVAL;
    }

    /*
     *      getfrac - get fraction part of a floating number.
     */
    static void getfrac(int radix, char **ptr)
    {
        long double frmul;
        frmul = 1.0 / radix;
        while (radix36(**ptr) < radix)
        {
            rval += frmul * radix36(*(*ptr)++);
            frmul /= radix;
        }
        lastst = RVAL;
    }

    /*
     *      getexp - get exponent part of floating number.
     *
     *      this algorithm is primative but usefull.  Floating
     *      exponents are limited to +/-255 but most hardware
     *      won't support more anyway.
     */
    static void getexp(int radix, char **ptr, int towarn)
    {
        int neg = FALSE;
        if (**ptr == '-')
        {
            neg = TRUE;
            (*ptr)++;
        }
        else
        {
            if (**ptr == '+')
                (*ptr)++;
        }
        getbase(10, ptr, towarn);
        if (ival > LDBL_MAX_10_EXP)
        {
            ieerr(0, 0, 0, "Exponent too large", towarn);
            ival = 0;
        };
        if (neg)
            ival =  - ival;
        if (radix == 10)
            rval *= pow10l((long double)ival);
        else
        {
            rval *= powl(2.0, (long double)ival);
        }
        lastst = RVAL;
    }

    /*
     *      getnum - get a number from input.
     *
     *      getnum handles all of the numeric input. it accepts
     *      decimal, octal, hexidecimal, and floating point numbers.
     */
    static void getnum(char **text, int towarn)
    {
        char buf[200],  *ptr = buf;
        int hasdot = FALSE;
        int radix = 10;
        int floatradix = 0;

        lastst = NVAL;
        if (**text == '0')
        {
            (*text)++;
            if (**text == 'x' ||  **text == 'X')
            {
                (*text)++;
                radix = 16;
            }
            else
                radix = 8;
        }
        else
        {
            char *t =  *text;
            while (isxdigit(*t))
                t++;
            if (*t == 'H' ||  *t == 'h')
                radix = 16;
        }
        while (radix36(**text) < radix)
        {
            *ptr++ =  * * text;
            (*text)++;

        }
        if (radix == 16 &&  **text == 'H' ||  **text == 'h')
            (*text)++;
        if (**text == '.')
        {
            if (radix == 8)
                radix = 10;
            *ptr++ =  * * text;
            (*text)++;
            while (radix36(**text) < radix)
            {
                *ptr++ =  * * text;
                (*text)++;
            }
        }
        if ((**text == 'e' ||  **text == 'E') && radix != 16)
            radix = floatradix = 10;
        else if ((**text == 'p' ||  **text == 'P') && radix == 16)
            floatradix = 2;

        if (floatradix)
        {
            *ptr++ =  * * text;
            (*text)++;
            if (**text == '-' ||  **text == '+')
            {
                *ptr++ =  * * text;
                (*text)++;
            }
            while (radix36(**text) < 10)
            {
                *ptr++ =  * * text;
                (*text)++;
            }
        }

        *ptr = 0;
        ptr = buf;
        // at this point the next char is any qualifier after the number

        if (radix36(*ptr) < radix)
            getbase(radix, &ptr, towarn);
        else
        {
            ival = 0;
            lastst = IVAL;
        }
        if (*ptr == '.')
        {
            ptr++;
            rval = ival;
            getfrac(radix, &ptr);
        }
        if (*ptr == 'e' ||  *ptr == 'E' ||  *ptr == 'p' ||  *ptr == 'P')
        {
            if (lastst != RVAL)
            {
                rval = ival;
            }
            ptr++;
            getexp(floatradix, &ptr, towarn);
        }
        if (lastst != RVAL)
        {
            if (**text == 'i')
            {
                if (**text == '6' && *(*text + 1) == '4')
                {
                    (*text)++;
                    (*text)++;
                    (*text)++;
                }
            }
            else if (**text == 'U' ||  **text == 'u')
            {
                lastst = IUVAL;
                (*text)++;
                if (**text == 'L' ||  **text == 'l')
                {
                    if (**text == 'L' ||  **text == 'l')
                    {
                        (*text)++;
                    }
                }
            }
            else if (**text == 'L' ||  **text == 'l')
            {
                (*text)++;
                if (**text == 'L' ||  **text == 'l')
                {
                    (*text)++;
                    if (**text == 'U' ||  **text == 'u')
                    {
                        (*text)++;
                        lastst = IUVAL;
                    }
                }

                else if (**text == 'U' ||  **text == 'u')
                {
                    lastst = IUVAL;
                    (*text)++;
                }
            }
        }
        else
        {
            if (**text == 'F' ||  **text == 'f')
            {
                if (lastst != RVAL)
                {
                    rval = ival;
                }
                lastst = RVAL;
                (*text)++;
            }
            else if (**text == 'L' ||  **text == 'l')
            {
                if (lastst != RVAL)
                {
                    rval = ival;
                }
                lastst = RVAL;
                (*text)++;
            }
        }
        if (isalnum(**text) || (**text) == '_')
        {
            lastst = NVAL;
        }
    }
	static int pointedsize(char **typetab, VARINFO *var)
	{
		VARINFO *p = var->subtype;
		int rv =0 ;
		var->subtype = NULL;
		if (CV_IS_PRIMITIVE(var->type))
		{
			rv = basictypesize(var->type);
		}
		else
		{
			GetPointerInfo(*typetab, var);
			if (CV_IS_PRIMITIVE(var->subtype->type))
			{
				rv = basictypesize(var->subtype->type);
			}
			else if (var->pointer)
				rv = 4;
			else if (var->structure || var->unionx)
				rv = var->arraysize;
			else if (var->array)
				rv = var->arraysize;
			else
				rv = 1 ; /* should never get here */
			FreeVarInfo(var->subtype);
		}
		var->subtype = p;
		return rv;
	}
    static VARINFO *constnode(char **text, int towarn)
    {
        getnum(text, towarn);
        if (lastst == NVAL)
            return ieerr(text, 0, 0, "Invalid constant", towarn);
        if (lastst == RVAL)
        {
            VARINFO *v = calloc(sizeof(VARINFO), 1);
            if (!v)
                return 0;
            v->constant = 1;
            v->fval = rval;
            v->type = T_REAL80;
            return v;
        }
        else if (lastst == IVAL || lastst == IUVAL)
        {
            VARINFO *v = calloc(sizeof(VARINFO), 1);
            if (!v)
                return 0;
            v->constant = 1;
            v->ival = ival;
            v->type = lastst == IVAL ? T_INT8 : T_UINT8;
            return v;
        }
        return 0;
    }
    static void EvalBasicType(VARINFO *var, int *signedtype, SCOPE *scope)
    {
        char data[12];
        int type = HintBasicType(var, signedtype, data);
        var->constant = TRUE;
        switch (type)
        {
            case T_INT8:
                #ifndef BORLANDC
                    var->ival = *(LLONG_TYPE*)data;
                    break;
                #endif 
            default:
                var->ival = 0;
                var->fval = 0;
                var->fvali = 0;
                break;
            case T_INT4:
                if (signedtype)
                    var->ival = *(int*)data;
                else
                    var->ival = *(unsigned*)data;
                break;
            case T_BOOL08:
                var->ival = data[0];
                break;
            case T_REAL32:
            case T_IMAGINARY32:
                var->fval = *(float*)data;
                break;
            case T_REAL80:
            case T_IMAGINARY80:
                var->fval = *(long double*)data;
                break;
            case T_REAL64:
            case T_IMAGINARY64:
                var->fval = *(double*)data;
                break;
            case T_CPLX32:
                var->fval = *(float*)data;
                var->fvali = *(float*)(data+4);
                break ;
            case T_CPLX64:
                var->fval = *(double*)data;
                var->fvali = *(double*)(data+8);
                break ;
            case T_CPLX80:
                var->fval = *(long double*)data;
                var->fvali = *(long double*)(data+10);
                break ;
        }

    }
    static VARINFO *makeconst(VARINFO *var, SCOPE *scope)
    {
        int signedtype;
        if (var->constant)
            return var;
        if (CV_IS_PRIMITIVE(var->type) && !var->array)
        {
            EvalBasicType(var, &signedtype, scope);
            if (var->bitfield)
            {
                if (signedtype)
                {
                    var->ival <<= 32-var->bitstart - var->bitlength;
                    var->ival >>= 32-var->bitlength;
                }
                else
                {
                    var->ival >>= var->bitstart;
                    var->ival &= bitmask[var->bitlength - 1];
                }
            }
        }
        else
        {
            var->constant = TRUE;
            var->type = T_UINT4;
            if (var->pointer)
            {
                var->ival = 0;
				if (var->derefaddress != -1)
					var->ival = var->derefaddress;
				else
	                ReadValue(var->ival, &var->address, 4, var);
                var->pointer = FALSE;
            }
            else
                var->ival = var->address;
            FreeVarInfo(var->subtype);
            var->subtype = 0;
        }
        return var;
    }
    static void truncateconst(VARINFO *var1, VARINFO *var2)
    {
        // nop
    }
    static VARINFO *lookupsym(char **text, char **typetab, char **symtab,
        DEBUG_INFO **dbg, SCOPE *sc, int towarn)
    {
        char buf[256],  *p = buf;
        VARINFO *var;
        int offset;
        while (isalnum(**text) ||  **text == '_')
            *p++ = *(*text)++;
        *p = 0;
        if (sc = FindSymbol(dbg, typetab, symtab, &offset, sc, (char*) buf))
        {
            var = GetVarInfo(*dbg,  *typetab,  *symtab,  offset, (char*)buf, sc, activeThread);
            if (var->udt)
                return ieerr(text, var, 0, "Can't use type here", towarn);
            return var;
        }
        return NULL;
    }
    typedef struct
    {
        char *str;
        int type;
    } CASTTYPE;

    static CASTTYPE regs[] = 
    {
        {
            "al", CV_REG_AL
        }
        , 
        {
            "ah", CV_REG_AH
        }
        , 
        {
            "bl", CV_REG_BL
        }
        , 
        {
            "bh", CV_REG_BH
        }
        , 
        {
            "cl", CV_REG_CL
        }
        , 
        {
            "ch", CV_REG_CH
        }
        , 
        {
            "dl", CV_REG_DL
        }
        , 
        {
            "dh", CV_REG_DH
        }
        , 
        {
            "ax", CV_REG_AX
        }
        , 
        {
            "bx", CV_REG_BX
        }
        , 
        {
            "cx", CV_REG_CX
        }
        , 
        {
            "dx", CV_REG_DX
        }
        , 
        {
            "sp", CV_REG_SP
        }
        , 
        {
            "bp", CV_REG_BP
        }
        , 
        {
            "si", CV_REG_SI
        }
        , 
        {
            "di", CV_REG_DI
        }
        , 
        {
            "eax", CV_REG_EAX
        }
        , 
        {
            "ebx", CV_REG_EBX
        }
        , 
        {
            "ecx", CV_REG_ECX
        }
        , 
        {
            "edx", CV_REG_EDX
        }
        , 
        {
            "esp", CV_REG_ESP
        }
        , 
        {
            "ebp", CV_REG_EBP
        }
        , 
        {
            "esi", CV_REG_ESI
        }
        , 
        {
            "edi", CV_REG_EDI
        }
        , 
    };
    static VARINFO *regnode(char **text)
    {
        int i, l;
        VARINFO *var1;
        char *p =  *text;
        skipspace(&p);
        for (i = 0; i < sizeof(regs) / sizeof(regs[0]); i++)
            if (!strncmp(p, regs[i].str, l = strlen(regs[i].str)) && !isalnum
                (p[l]) && p[l] != '_')
                break;
        if (i >= sizeof(regs) / sizeof(regs[0]))
            return 0;
        *text += strlen(regs[i].str);
        var1 = calloc(sizeof(VARINFO), 1);
        if (!var1)
            return 0;
        var1->type = T_INT4;
        var1->address = regs[i].type;
        var1->explicitreg = 1;
        return var1;
    }
    static CASTTYPE casts[] = 
    {
        {
            "int", T_INT4
        }
        , 
        {
            "long", T_INT4
        }
        , 
        {
            "long long", T_INT8
        }
        , 
        {
            "unsigned", T_UINT4
        }
        , 
        {
            "unsigned long", T_UINT4
        }
        , 
        {
            "unsigned long long", T_UINT8
        }
        , 
        {
            "short", T_SHORT
        }
        , 
        {
            "unsigned short", T_USHORT
        }
        , 
        {
            "char", T_CHAR
        }
        , 
        {
            "unsigned char", T_UCHAR
        }
        , 
        {
            "bool", T_BOOL08
        }
        , 
        {
            "float", T_REAL32
        }
        , 
        {
            "double", T_REAL64
        }
        , 
        {
            "long double", T_REAL80
        }
        , 
        {
            "float imaginary", T_IMAGINARY32
        }
        , 
        {
            "double imaginary", T_IMAGINARY64
        }
        , 
        {
            "long double imaginary", T_IMAGINARY80
        }
    };
    static VARINFO *castnode(char **text, char **typetab, char **symtab,
        DEBUG_INFO **dbg, SCOPE *sc, int towarn)
    {
        VARINFO *var1 = 0;
        int tp, i;
        int indir = 0, l;
        char *p = (*text) + 1;
		char *start = (*text);
        char buf[256],  *q = buf;
        int offset;
        UDTSYM *s = 0;

        skipspace(&p);
        for (i = 0; i < sizeof(casts) / sizeof(casts[0]); i++)
            if (!strncmp(p, casts[i].str, l = strlen(casts[i].str)) && !isalnum
                (p[l]) && p[l] != '_')
                break;
        if (i >= sizeof(casts) / sizeof(casts[0]))
        {
            // IF they type in struct or union, it is a nop, we are keying
            // only off the type name
            if (!strncmp(p, "struct ", 7))
                p += 7;
            else if (!strncmp(p, "union ", 6))
                p += 6;
            if (isalpha(*p) ||  *p == '_')
            {
                while (isalnum(*p) ||  *p == '_')
                    *q++ =  *p++;
                *q = 0;
                if (!FindSymbol(dbg, typetab, symtab, &offset, sc, (char*)buf))
				{
					*text = start;
                    return 0;
				}
                s = (UDTSYM*)(*symtab +  offset);
                if (s->rectyp != S_UDT)
				{
					*text = start;
                    return 0;
				}
                tp = s->typind;
            }
			else
			{
				*text = start;
				return 0;
			}
        }
        else
        {
            p += strlen(casts[i].str);
            tp = casts[i].type;
        }
        if (!isspace(*p) &&  *p != ')' &&  *p != '*')
            return 0;

        skipspace(&p);
        while (*p == '*')
        {
            p++;
            indir++;
            skipspace(&p);
        }
        *text = p;
        if (*(*text)++ != ')')
            return ieerr(text, var1, 0, "Missing ')'", towarn);
        // if we got here we have a valid cast
        var1 = calloc(sizeof(VARINFO), 1);
        if (!var1)
            return 0;
        var1->type = tp;
        if (s)
        {
            if (!indir)
                return ieerr(text, var1, 0, 
                    "struct/union cast must have pointer", towarn);
            indir--;
            DeclType(*typetab, var1);
        }
        while (indir)
        {
            VARINFO *var2 = calloc(sizeof(VARINFO), 1);
            if (!var2)
            {
                FreeVarInfo(var1);
                return 0;
            }
            var2->subtype = var1;
            var2->pointer = TRUE;
            var1 = var2;
            indir--;
        }
        return var1;
    }
    static VARINFO *sizeofop(char **text, char **typetab, char **symtab,
        DEBUG_INFO **dbg, SCOPE *sc, int towarn)
    {
        return ieerr(text, 0, 0, "sizeof not implemented", towarn);
    }
    static VARINFO *ieprimary(char **text, char **typetab, char **symtab,
        DEBUG_INFO *dbg, SCOPE *sc, int towarn)
    /*
     * PRimary integer
     *    id
     *    iconst
     *    (cast )intexpr
     *    (intexpr)
     *    intexpr.id
     *    intexpr->id
     */
    {
        VARINFO *var1 = 0,  *var2,  **var3;
        char buf[256],  *p;
        int needclose;
        skipspace(text);
        if (isdigit(**text) ||  **text == '.')
            var1 = constnode(text, towarn);
        else if (isalpha(**text) ||  **text == '_')
		{
			char *p = *text;
            var1 = lookupsym(text, typetab, symtab, dbg, sc, towarn);
			if (!var1)
			{
				*text = p;
				var1 = regnode(text);
				if (!var1)
				{
			        return ieerr(text, 0, 0, "Undefined symbol", towarn);
				}
			}
		}
        else if (!memcmp(*text, "sizeof", 6))
            return sizeofop(text, typetab, symtab, dbg, sc, towarn);
        else if (**text == '(')
        {
            var1 = castnode(text, typetab, symtab, dbg, sc, towarn);
            if (var1)
            {
                PUSH(var1);
                var2 = ieprimary(text, typetab, symtab, dbg, sc, towarn);
                if (!var2)
                    return 0;
                var1 = POP();
                if (var2->constant)
                {
                    var1->constant = TRUE;
                    var1->ival = var2->ival;
                    var1->fval = var2->fval;
                }
                else
                    var1->address = var2->address;
                FreeVarInfo(var2);
            }
            else
            {
                (*text)++;
                var1 = iecondop(text, typetab, symtab, dbg, sc, towarn);
                if (**text != ')')
                    return ieerr(text, var1, 0, "Missing ')'", towarn);
                (*text)++;
            }
        }
        skipspace(text);
        if (!var1)
            return var1;
        do
        {
            while (**text == '.' ||  **text == '-' && *(*text + 1) == '>')
            {
                int address;
                if (**text == '.')
                {
                    if (!var1->structure && !var1->unionx)return ieerr(text,
                        var1, 0, "Structure or union type expected", towarn)
                        ;
                    if (var1->pointer)
                        return ieerr(text, var1, 0, 
                            "Address of structure expected", towarn);
                    (*text)++;
                    address = var1->address;
                }
                else
                {
                    if (!var1->pointer)
                        return ieerr(text, var1, 0, "Pointer type expected", towarn);
                    (*text) += 2;
                    GetPointerInfo(*typetab, var1);
                    var2 = var1->subtype;
                    var1->subtype = 0;
					if (var1->derefaddress != -1)
						var2->address = var1->derefaddress;
					else
	                    ReadValue(var1->address, &var2->address, 4, var1);
                    FreeVarInfo(var1);
                    var1 = var2;
                    if (!var1->structure && !var1->unionx)
                        return ieerr(text, var1, 0, 
                            "Structure or union type expected", towarn);
                }
                skipspace(text);
                if (!isalpha(**text) &&  **text != '_')
                    return ieerr(text, var1, 0, "Identifier Expected", towarn);
                p = buf;
                while (isalnum(**text) ||  **text == '_')
                    *p++ = *(*text)++;
                *p = 0;
                var2 = var1->subtype;
                var3 = &var1->subtype;
                while (var2 && strcmp(buf, var2->membername))
                {
                    var3 = &var2->link;
                    var2 = var2->link;
                }
                if (!var2)
                    return ieerr(text, var1, 0, "Unknown member name", towarn);
                (*var3) = var2->link;
                var2->address = var1->address + var2->offset;
                var2->offset = 0;
                FreeVarInfo(var1);
                var1 = var2;
                var1->link = 0;
                skipspace(text);
            }
            while (**text == '[')
            {
                (*text)++;
                if (!var1->array && !var1->pointer)
                    return ieerr(text, var1, 0, "Array expected", towarn);
                skipspace(text);
                if (!isdigit(**text))
				{
					VARINFO *vx ;
					int issigned;
					vx = iecondop(text ,typetab, symtab, dbg, sc, towarn);
					if (!vx)
						return vx;
					if (!vx->constant)
						EvalBasicType(vx, &issigned, sc);
					ival = vx->ival;
					FreeVarInfo(vx);
				}
				else
				{
	                getnum(text, towarn);
    	            if (lastst != IVAL && lastst != IUVAL)
        	            return ieerr(text, var1, 0, "Invalid array index", towarn);
				}
           	    skipspace(text);
                if (**text != ']')
                    return ieerr(text, var1, 0, "Missing ']'", towarn);
                (*text)++;
                skipspace(text);
                if (!var1->array)
                {
                    GetPointerInfo(*typetab, var1);
					if (var1->derefaddress != -1)
						var1->subtype->address = var1->derefaddress;
					else
	                    ReadValue(var1->address, &var1->subtype->address, 4, var1); 
					var2 = var1;
					var1 = var1->subtype;
	                var1->address = var1->address + pointedsize(typetab, var1) *ival;
					
					var2->subtype = NULL;
					FreeVarInfo(var2);
                }
                else
                {
					var2 = var1;
					var1 = var1->subtype;
					var1->address = var2->address + var2->itemsize * ival;
					var2->subtype = var1->link;
					var1->link = NULL;
					FreeVarInfo(var2);
                }
            }
        }
        while (**text == '.' ||  **text == '-' && *(*text + 1) == '>')
            ;
        return var1;
    }
    /*
     * Integer unary
     *   - unary
     *   ! unary
     *   ~unary
     *   primary
     */
    static VARINFO *ieunary(char **text, char **typetab, char **symtab,
        DEBUG_INFO *dbg, SCOPE *sc, int towarn)
    {
        VARINFO *val1,  *val2;
        switch (**text)
        {
            case '-':
                if (*(*text + 1) != '>')
                {
                    (*text)++;
                    val1 = ieunary(text, typetab, symtab, dbg, sc, towarn);
                    val1 = makeconst(val1, sc);
                    if (CV_TYP_IS_REAL(val1->type) || CV_TYP_IS_IMAGINARY(val1->type))
                        val1->fval =  - val1->fval;
                    else
                        val1->ival =  - val1->ival;
                    truncateconst(val1, 0);
                }
                else
                    val1 = ieprimary(text, typetab, symtab, dbg, sc, towarn);
                break;
            case '!':
                (*text)++;
                val1 = ieunary(text, typetab, symtab, dbg, sc, towarn);
                val1 = makeconst(val1, sc);
                if (CV_TYP_IS_REAL(val1->type) || CV_TYP_IS_IMAGINARY(val1->type))
                    val1->fval = !val1->fval;
                else
                    val1->ival = !val1->ival;
                break;
            case '~':
                (*text)++;
                val1 = ieunary(text, typetab, symtab, dbg, sc, towarn);
                val1 = makeconst(val1, sc);
                if (CV_TYP_IS_REAL(val1->type) || CV_TYP_IS_IMAGINARY(val1->type))
                    return ieerr(text, 0, 0, "Invalid Floating Operation", towarn);
                else
                    val1->ival = ~val1->ival;
                truncateconst(val1, 0);
                break;
            case '+':
                (*text)++;
                val1 = ieunary(text, typetab, symtab, dbg, sc, towarn);
                val1 = makeconst(val1, sc);
                break;
            case '*':
                (*text)++;
                val1 = ieunary(text, typetab, symtab, dbg, sc, towarn);
                if (!val1 || !val1->pointer)
                    return ieerr(text, val1, 0, "Pointer Type Expected", towarn);
                if (CV_IS_PRIMITIVE(val1->type))
                {
                    val1->type &= ~CV_MMASK;
					val1->pointer = FALSE;
					if (val1->derefaddress != -1)
						val1->address = val1->derefaddress;
					else
	                    ReadValue(val1->address, &val1->address, 4, val1);
                }
                else
                {
                    GetPointerInfo(*typetab, val1);
                    val2 = val1->subtype;
					if (val1->derefaddress != -1)
						val2->address = val1->derefaddress;
					else
	                    ReadValue(val1->address, &val2->address, 4, val1);
                    val1->subtype = 0;
                    FreeVarInfo(val1);
                    val1 = val2;
                }
                break;
            case '&':
                (*text)++;
                val1 = ieunary(text, typetab, symtab, dbg, sc, towarn);
				if (val1) 
                {
                    val1 = makeconst(val1, sc);
                    val1->type = T_VOID ; // using T_VOID as a placeholder for addresses
                }
                break;
            default:
                val1 = ieprimary(text, typetab, symtab, dbg, sc, towarn);
                break;
        }
        return (val1);
    }
    static VARINFO *iemultops(char **text, char **typetab, char **symtab,
        DEBUG_INFO *dbg, SCOPE *sc, int towarn)
    /* Multiply ops */
    {
        VARINFO *val1,  *val2;
        PUSH(val1 = ieunary(text, typetab, symtab, dbg, sc, towarn));
        skipspace(text);
        while (val1 && (*(*text) == '*' || *(*text) == '%' || *(*text) == '\\'))
        {
            int ch = *(*text)++;
            val2 = ieunary(text, typetab, symtab, dbg, sc, towarn);
            if (!val2)
                return 0;
            val1 = makeconst(POP(), sc);
            val2 = makeconst(val2, sc);
            switch (ch)
            {
                case '*':
                    if (CV_TYP_IS_REAL(val1->type) || CV_TYP_IS_IMAGINARY(val1->type))
                        if (CV_TYP_IS_REAL(val2->type) || CV_TYP_IS_IMAGINARY(val2->type))
                            val1->fval = val1->fval *val2->fval;
                        else
                            val1->fval = val1->fval *val2->ival;
                        else
                            if (CV_TYP_IS_REAL(val2->type) || CV_TYP_IS_IMAGINARY(val2->type))
                                val1->fval = val1->ival *val2->fval;
                            else
                                val1->ival = val1->ival *val2->ival;
                    break;
                case '/':
                    if (CV_TYP_IS_REAL(val1->type) || CV_TYP_IS_IMAGINARY(val1->type))
                        if (CV_TYP_IS_REAL(val2->type) || CV_TYP_IS_IMAGINARY(val2->type))
                            val1->fval = val1->fval / val2->fval;
                        else
                            val1->fval = val1->fval / val2->ival;
                        else
                            if (CV_TYP_IS_REAL(val2->type) || CV_TYP_IS_IMAGINARY(val2->type))
                                val1->fval = val1->ival / val2->fval;
                            else
                                val1->ival = val1->ival / val2->ival;
                    break;
                case '%':
                    if (CV_TYP_IS_REAL(val1->type) || CV_TYP_IS_IMAGINARY(val1->type) || CV_TYP_IS_REAL(val2->type) || CV_TYP_IS_IMAGINARY(val2->type)
                        )
                        return ieerr(text, 0, 0, "Invalid Floating Operation", towarn);
                    val1->ival = val1->ival % val2->ival;
                    break;
            }
            truncateconst(val1, val2);
            FreeVarInfo(val2);
            PUSH(val1);
            skipspace(text);
        }
        return (POP());
    }
    static VARINFO *ieaddops(char **text, char **typetab, char **symtab,
        DEBUG_INFO *dbg, SCOPE *sc, int towarn)
    /* Add ops */
    {

        VARINFO *val1,  *val2;
        int mul1 = 1, mul2 = 1;
        PUSH(val1 = iemultops(text, typetab, symtab, dbg, sc, towarn));
        skipspace(text);
        while (val1 && (*(*text) == '+' || *(*text) == '-') && *(*text + 1) !=
            '>')
        {
            int ch = *(*text);
            (*text)++;
            val2 = iemultops(text, typetab, symtab, dbg, sc, towarn);
            if (!val2)
                return 0;
            val1 = POP();
			if (val1->pointer || val1->array || val2->pointer || val2->array)
			{
				if (ch == '+')
				{
					if ((val1->pointer || val1->array) && (val2->pointer || val2->array))
    	                return ieerr(text, val1, val2, "Too many pointers in addition", towarn);
					if (val2->pointer || val2->array)
					{
						VARINFO *val3 = val2 ;
						val2 = val1;
						val1 = val3;
					}
					if (val1->pointer)
					{
						if (val1->derefaddress == -1)
			                ReadValue(val1->address, &val1->derefaddress, 4, val1);
			            val2 = makeconst(val2, sc);
						val1->derefaddress += val2->ival * pointedsize(typetab, val1);
					}
					else
					{
			            val2 = makeconst(val2, sc);
						val1->address += val2->ival * val1->arraysize;
					}
				}
				else
				{
					int size;
					if (!val1->pointer && !val1->array)
						return ieerr(text, val1, val2, "Invalid pointer subtraction", towarn);
					if (val1->pointer)
					{
						if (val1->derefaddress == -1)
			                ReadValue(val1->address, &val1->derefaddress, 4, val1);
						size = pointedsize(typetab, val1);
					}
					else
						size = val1->itemsize;
					if (val2->pointer)
						if (val2->derefaddress == -1)
			                ReadValue(val2->address, &val2->derefaddress, 4, val2);
					if (val2->pointer)
					{
						unsigned v;
						if (val1->pointer)
						{
							v = (val1->derefaddress - val2->derefaddress) / size;
						}
						else
						{
							v = (val1->address - val2->derefaddress) / size;
						}
						val1 = makeconst(val1, sc);
						val1->ival = v;
					}
					else if (val2->array)
					{
						unsigned v;
						if (val1->pointer)
						{
							v = (val1->derefaddress - val2->address) / size;
						}
						else
						{
							v = (val1->address - val2->address) / size;
						}
						val1 = makeconst(val1, sc);
						val1->ival = v;
					}
					else
					{
						if (val1->pointer)
						{
				            val2 = makeconst(val2, sc);
							val1->derefaddress -= val2->ival * pointedsize(typetab, val1);
						}
						else
						{
				            val2 = makeconst(val2, sc);
							val1->address -= val2->ival * val1->itemsize;
						}
					}
				}
			}
			else
			{
join:
	            if (val1->structure || val1->unionx)
	                mul1 = val1->arraysize;
	            else if (val2->structure || val2->unionx)
	                mul2 = val2->arraysize;
	            else if (val1->array)
	            {
	                mul1 = val1->itemsize;
	            }
	            else if (val2->array)
	            {
	                mul2 = val2->itemsize;
	            }
	            val1 = makeconst(val1, sc);
	            val2 = makeconst(val2, sc);
	            if (ch == '+')
	            {
	                if (CV_TYP_IS_REAL(val1->type) || CV_TYP_IS_IMAGINARY(val1->type))
	                    if (CV_TYP_IS_REAL(val2->type) || CV_TYP_IS_IMAGINARY(val2->type))
	                        val1->fval = val1->fval *mul2 + val2->fval *mul1;
	                    else
	                        val1->fval = val1->fval *mul2 + val2->ival *mul1;
	                    else
	                        if (CV_TYP_IS_REAL(val2->type) || CV_TYP_IS_IMAGINARY(val2->type))
	                            val1->fval = val1->ival *mul2 + val2->fval *mul1;
	                        else
	                            val1->ival = val1->ival *mul2 + val2->ival *mul1;
	            }
	            else
	            {
	                if (CV_TYP_IS_REAL(val1->type) || CV_TYP_IS_IMAGINARY(val1->type))
	                    if (CV_TYP_IS_REAL(val2->type) || CV_TYP_IS_IMAGINARY(val2->type))
	                        val1->fval = val1->fval *mul2 - val2->fval *mul1;
	                    else
	                        val1->fval = val1->fval *mul2 - val2->ival *mul1;
	                    else
	                        if (CV_TYP_IS_REAL(val2->type) || CV_TYP_IS_IMAGINARY(val2->type))
	                            val1->fval = val1->ival *mul2 - val2->fval *mul1;
	                        else
	                            val1->ival = val1->ival *mul2 - val2->ival *mul1;
	            }
	            truncateconst(val1, val2);
			}
            FreeVarInfo(val2);
            PUSH(val1);
            skipspace(text);
        }
        return POP();
    }
    static VARINFO *ieshiftops(char **text, char **typetab, char **symtab,
        DEBUG_INFO *dbg, SCOPE *sc, int towarn)
    /* Shift ops */
    {
        VARINFO *val1,  *val2;
        PUSH(val1 = ieaddops(text, typetab, symtab, dbg, sc, towarn));
        skipspace(text);
        while (val1 && ((*(*text) == '<' || *(*text) == '>') && *(*text) == *
            (*text + 1)))
        {
            long oper = *(*text) == '<';
            (*text) += 2;
            val2 = ieaddops(text, typetab, symtab, dbg, sc, towarn);
            if (!val2)
                return 0;
            val1 = POP();
            val1 = makeconst(POP(), sc);
            val2 = makeconst(val2, sc);
            if (CV_TYP_IS_REAL(val1->type) || CV_TYP_IS_IMAGINARY(val1->type) || CV_TYP_IS_REAL(val2->type) || CV_TYP_IS_IMAGINARY(val2->type))
                return ieerr(text, val1, val2, "Invalid floating operation", towarn);
            if (oper)
            {
                val1->ival = val1->ival << val2->ival;
            }
            else
                val1->ival = val1->ival >> val2->ival;
            truncateconst(val1, val2);
            FreeVarInfo(val2);
            PUSH(val1);
            skipspace(text);
        }
        return POP();
    }
    static VARINFO *ierelation(char **text, char **typetab, char **symtab,
        DEBUG_INFO *dbg, SCOPE *sc, int towarn)
    /* non-eq relations */
    {
        VARINFO *val1,  *val2;
        PUSH(val1 = ieshiftops(text, typetab, symtab, dbg, sc, towarn));
        skipspace(text);
        while (val1 && (*(*text) == '<' && *(*text) == '>' && (*(*text + 1) !=
            *(*text))))
        {
            int oper = *(*text) == '<';
            (*text)++;
            if (**text == '=')
            {
                (*text)++;
                oper |= 2;
            }
            val2 = ieshiftops(text, typetab, symtab, dbg, sc, towarn);
            if (!val2)
                return 0;
            val1 = makeconst(POP(), sc);
            val2 = makeconst(val2, sc);
            switch (oper)
            {
                case 0:
                    if (CV_TYP_IS_REAL(val1->type) || CV_TYP_IS_IMAGINARY(val1->type))
                        if (CV_TYP_IS_REAL(val2->type) || CV_TYP_IS_IMAGINARY(val2->type))
                            val1->ival = val1->fval < val2->fval;
                        else
                            val1->ival = val1->fval < val2->ival;
                        else
                            if (CV_TYP_IS_REAL(val2->type) || CV_TYP_IS_IMAGINARY(val2->type))
                                val1->ival = val1->ival < val2->fval;
                            else
                                val1->ival = val1->ival < val2->ival;
                    break;
                case 1:
                    if (CV_TYP_IS_REAL(val1->type) || CV_TYP_IS_IMAGINARY(val1->type))
                        if (CV_TYP_IS_REAL(val2->type) || CV_TYP_IS_IMAGINARY(val2->type))
                            val1->ival = val1->fval > val2->fval;
                        else
                            val1->ival = val1->fval > val2->ival;
                        else
                            if (CV_TYP_IS_REAL(val2->type) || CV_TYP_IS_IMAGINARY(val2->type))
                                val1->ival = val1->ival > val2->fval;
                            else
                                val1->ival = val1->ival > val2->ival;
                    break;
                case 2:
                    if (CV_TYP_IS_REAL(val1->type) || CV_TYP_IS_IMAGINARY(val1->type))
                        if (CV_TYP_IS_REAL(val2->type) || CV_TYP_IS_IMAGINARY(val2->type))
                            val1->ival = val1->fval <= val2->fval;
                        else
                            val1->ival = val1->fval <= val2->ival;
                        else
                            if (CV_TYP_IS_REAL(val2->type) || CV_TYP_IS_IMAGINARY(val2->type))
                                val1->ival = val1->ival <= val2->fval;
                            else
                                val1->ival = val1->ival <= val2->ival;
                    break;
                case 3:
                    if (CV_TYP_IS_REAL(val1->type) || CV_TYP_IS_IMAGINARY(val1->type))
                        if (CV_TYP_IS_REAL(val2->type) || CV_TYP_IS_IMAGINARY(val2->type))
                            val1->ival = val1->fval >= val2->fval;
                        else
                            val1->ival = val1->fval >= val2->ival;
                        else
                            if (CV_TYP_IS_REAL(val2->type) || CV_TYP_IS_IMAGINARY(val2->type))
                                val1->ival = val1->ival >= val2->fval;
                            else
                                val1->ival = val1->ival >= val2->ival;
                    break;
            }
            val1->type = T_UINT4;
            FreeVarInfo(val2);
            PUSH(val1);
            skipspace(text);
        }
        return POP();
    }
    static VARINFO *ieequalops(char **text, char **typetab, char **symtab,
        DEBUG_INFO *dbg, SCOPE *sc, int towarn)
    /* eq relations */
    {
        VARINFO *val1,  *val2;
        PUSH(val1 = ierelation(text, typetab, symtab, dbg, sc, towarn));
        skipspace(text);
        while (val1 && (*(*text + 1) == '=' && (*(*text) == '=' || *(*text) ==
            '!')))
        {
            int ch = *(*text);
            *(*text) += 2;

            val2 = ierelation(text, typetab, symtab, dbg, sc, towarn);
            if (!val2)
                return 0;
            val1 = makeconst(POP(), sc);
            val2 = makeconst(val2, sc);
            if (ch == '!')
            {
                if (CV_TYP_IS_REAL(val1->type) || CV_TYP_IS_IMAGINARY(val1->type))
                    if (CV_TYP_IS_REAL(val2->type) || CV_TYP_IS_IMAGINARY(val2->type))
                        val1->ival = val1->fval != val2->fval;
                    else
                        val1->ival = val1->fval != val2->ival;
                    else
                        if (CV_TYP_IS_REAL(val2->type) || CV_TYP_IS_IMAGINARY(val2->type))
                            val1->ival = val1->ival != val2->fval;
                        else
                            val1->ival = val1->ival != val2->ival;
            }
            else
            {
                if (CV_TYP_IS_REAL(val1->type) || CV_TYP_IS_IMAGINARY(val1->type))
                    if (CV_TYP_IS_REAL(val2->type) || CV_TYP_IS_IMAGINARY(val2->type))
                        val1->ival = val1->fval == val2->fval;
                    else
                        val1->ival = val1->fval == val2->ival;
                    else
                        if (CV_TYP_IS_REAL(val2->type) || CV_TYP_IS_IMAGINARY(val2->type))
                            val1->ival = val1->ival == val2->fval;
                        else
                            val1->ival = val1->ival == val2->ival;
            }
            val1->type = T_UINT4;
            FreeVarInfo(val2);
            PUSH(val1);
            skipspace(text);
        }
        return POP();
    }
    static VARINFO *ieandop(char **text, char **typetab, char **symtab,
        DEBUG_INFO *dbg, SCOPE *sc, int towarn)
    /* and op */
    {
        VARINFO *val1,  *val2;
        PUSH(val1 = ieequalops(text, typetab, symtab, dbg, sc, towarn));
        skipspace(text);
        while (val1 && *(*text) == '&')
        {
            (*text)++;
            val2 = ieequalops(text, typetab, symtab, dbg, sc, towarn);
            if (!val2)
                return 0;
            val1 = POP();
            val1 = makeconst(POP(), sc);
            val2 = makeconst(val2, sc);
            if (CV_TYP_IS_REAL(val1->type) || CV_TYP_IS_IMAGINARY(val1->type) || CV_TYP_IS_REAL(val2->type) || CV_TYP_IS_IMAGINARY(val2->type))
                return ieerr(text, val1, val2, "Invalid floating operation", towarn);
            val1->ival = val1->ival &val2->ival;
            truncateconst(val1, val2);
            FreeVarInfo(val2);
            PUSH(val1);
            skipspace(text);
        }
        return POP();
    }
    static VARINFO *iexorop(char **text, char **typetab, char **symtab,
        DEBUG_INFO *dbg, SCOPE *sc, int towarn)
    /* xor op */
    {
        VARINFO *val1,  *val2;
        PUSH(val1 = ieandop(text, typetab, symtab, dbg, sc, towarn));
        skipspace(text);
        while (val1 && *(*text) == '^')
        {
            (*text)++;
            val2 = ieandop(text, typetab, symtab, dbg, sc, towarn);
            if (!val2)
                return 0;
            val1 = POP();
            val1 = makeconst(POP(), sc);
            val2 = makeconst(val2, sc);
            if (CV_TYP_IS_REAL(val1->type) || CV_TYP_IS_IMAGINARY(val1->type) || CV_TYP_IS_REAL(val2->type) || CV_TYP_IS_IMAGINARY(val2->type))
                return ieerr(text, val1, val2, "Invalid floating operation", towarn);
            val1->ival = val1->ival ^ val2->ival;
            truncateconst(val1, val2);
            FreeVarInfo(val2);
            PUSH(val1);
            skipspace(text);
        }
        return POP();
    }
    static VARINFO *ieorop(char **text, char **typetab, char **symtab,
        DEBUG_INFO *dbg, SCOPE *sc, int towarn)
    /* or op */
    {
        VARINFO *val1,  *val2;
        PUSH(val1 = iexorop(text, typetab, symtab, dbg, sc, towarn));
        skipspace(text);
        while (val1 && *(*text) == '|')
        {
            (*text)++;
            val2 = iexorop(text, typetab, symtab, dbg, sc, towarn);
            if (!val2)
                return 0;
            val1 = POP();
            val1 = makeconst(POP(), sc);
            val2 = makeconst(val2, sc);
            if (CV_TYP_IS_REAL(val1->type) || CV_TYP_IS_IMAGINARY(val1->type) || CV_TYP_IS_REAL(val2->type) || CV_TYP_IS_IMAGINARY(val2->type))
                return ieerr(text, val1, val2, "Invalid floating operation", towarn);
            val1->ival = val1->ival | val2->ival;
            truncateconst(val1, val2);
            FreeVarInfo(val2);
            PUSH(val1);
            skipspace(text);
        }
        return POP();
    }
    static VARINFO *ielandop(char **text, char **typetab, char **symtab,
        DEBUG_INFO *dbg, SCOPE *sc, int towarn)
    /* logical and op */
    {
        VARINFO *val1,  *val2;
        PUSH(val1 = ieorop(text, typetab, symtab, dbg, sc, towarn));
        skipspace(text);
        while (val1 && (**text == '&' && *(*text + 1) == '&'))
        {
            (*text) += 2;
            val2 = ieorop(text, typetab, symtab, dbg, sc, towarn);
            if (!val2)
                return 0;
            val1 = POP();
            val1 = makeconst(POP(), sc);
            val2 = makeconst(val2, sc);
            if (CV_TYP_IS_REAL(val1->type) || CV_TYP_IS_IMAGINARY(val1->type) || CV_TYP_IS_REAL(val2->type) || CV_TYP_IS_IMAGINARY(val2->type))
                return ieerr(text, val1, val2, "Invalid floating operation", towarn);
            val1->ival = val1->ival && val2->ival;
            truncateconst(val1, val2);
            FreeVarInfo(val2);
            PUSH(val1);
            skipspace(text);
        }
        return POP();
    }
    static VARINFO *ielorop(char **text, char **typetab, char **symtab,
        DEBUG_INFO *dbg, SCOPE *sc, int towarn)
    /* logical or op */
    {
        VARINFO *val1,  *val2;
        PUSH(val1 = ielandop(text, typetab, symtab, dbg, sc, towarn));
        skipspace(text);
        while (val1 && (**text == '|' && *(*text + 1) == '|'))
        {
            (*text) += 2;
            val2 = ielandop(text, typetab, symtab, dbg, sc, towarn);
            if (!val2)
                return 0;
            val1 = POP();
            val1 = makeconst(POP(), sc);
            val2 = makeconst(val2, sc);
            if (CV_TYP_IS_REAL(val1->type) || CV_TYP_IS_IMAGINARY(val1->type) || CV_TYP_IS_REAL(val2->type) || CV_TYP_IS_IMAGINARY(val2->type))
                return ieerr(text, val1, val2, "Invalid floating operation", towarn);
            val1->ival = val1->ival || val2->ival;
            truncateconst(val1, val2);
            FreeVarInfo(val2);
            PUSH(val1);
            skipspace(text);
        }
        return POP();
    }
    static VARINFO *iecondop(char **text, char **typetab, char **symtab,
        DEBUG_INFO *dbg, SCOPE *sc, int towarn)
    /* Hook op */
    {
        VARINFO *val1,  *val2,  *val3,  *val4;
        PUSH(val1 = ielorop(text, typetab, symtab, dbg, sc, towarn));
        skipspace(text);
        if (val1 &&  **text == '?')
        {
            (*text)++;
            PUSH(iecondop(text, typetab, symtab, dbg, sc, towarn));
            skipspace(text);
            if (**text != ':')
                return ieerr(text, 0, 0, "Expected ':'", towarn);
            (*text)++;
            val3 = iecondop(text, typetab, symtab, dbg, sc, towarn);
            val2 = POP();
            val1 = POP();
            val1 = makeconst(val1, sc);
            if (CV_TYP_IS_REAL(val1->type) || CV_TYP_IS_IMAGINARY(val1->type))
                if (val1->fval)
                    val4 = val2;
                else
                    val4 = val3;
                else
                    if (val1->ival)
                        val4 = val2;
                    else
                        val4 = val3;
            FreeVarInfo(val1);
            if (val4 == val2)
                FreeVarInfo(val3);
            else
                FreeVarInfo(val2);
            PUSH(val4);
        }
        return POP();
    }
VARINFO *EvalExpr(char **types, char **syms, DEBUG_INFO **dbg, SCOPE *sc,
    char *text, int towarn)
{
        VARINFO *var;
        char *p = text;
        if (! *text)
            return ieerr(&text, 0, 0, "Malformed Expression", towarn);
        varinfo_count = 0;
        var = iecondop(&text, types, syms, dbg, sc, towarn);
        if (varinfo_count ||  *text != 0)
            return ieerr(&text, var, 0, "Malformed Expression", towarn);
        if (!var)
            return var;
        if (var->constant)
        if (!CV_IS_PRIMITIVE(var->type) || var->array)
        {
            var->constant = FALSE;
            var->address = var->ival;
        }
        strcpy(var->membername, p);
        return var;
}
