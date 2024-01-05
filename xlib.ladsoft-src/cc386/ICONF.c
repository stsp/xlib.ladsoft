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
 * C configuration when we are using ICODE code generator output
 */
#include <stdio.h>
#include <string.h>
#include "utype.h"
#include "lists.h"
#include "expr.h"
#include "c.h"
#include "iexpr.h"
#include "iopt.h"

#define ADRSTART 0
#define FLOATSTART 0

extern int packdata[], packlevel;
extern TYP stdchar;
extern FILE *icdFile;

#ifdef BRIEFHELP
    char *usage_text = "[+e/+i/f+l/w+A/C/D/E/I/O] file list";
#else 
    char *usage_text = "[options] files\n"
        "+e     - dump errors to file          /fname - specify parameter file\n""+i     - dump preprocessed file       +l     - dump listing file\n""/fname - specify parameter file       +l     - dump listing file\n""/w-xxx - disable a warning            +A     - disable extensions\n""/C     - codegen parameters           /Dxxx  - define something\n""/Enn   - max number of errors         /Ipath - specify include path\n""/O     - optimizer parameters\n""Codegen parameters: (/C[+][-][params])\n""  +d   - display diagnostics          -b     - no BSS\n""  -l   - no C source in ASM file      -m     - no leading underscores\n""  +r   - reverse order of bit ops     -R     - no param base register\n""Optimizer parameters (/O[+][-][params])\n""  -c   - no common code deletions     -d     - no dead code deletions\n""  -f   - no constant folding          -g     - no GCSE optimizations\n""  -l   - no LCSE optimizations        -v     - no live variable analysis\n""  -x   - no copy propagation\n""\nTime: " __TIME__ "  Date: " __DATE__;
#endif 

char *registers[10];

char GLBDEFINE[] = "_icode_";
char SOURCEXT[] = ".asm";
char ENVNAME[] = "ccicode";
char PROGNAME[] = "ccicode";
//char OBJEXT[] = ".obj";
char OBJEXT[] = "";

int prm_browse;
int stdmemberptrsize = 4;
int prm_microlink = FALSE;
int prm_locsub = TRUE, prm_optcode = TRUE, prm_foldconst = TRUE, prm_optdead =
    TRUE;
int prm_globsub = TRUE, prm_copyprop = TRUE, prm_optlive = TRUE;
int floatregs, dataregs, addrregs;
int prm_68020 = FALSE, prm_largedata = FALSE;
short mindatregs = 0;
short useddatregs = 0;
short maxdatregs = 0;
short minadrregs = ADRSTART;
short usedadrregs = ADRSTART;
short maxadrregs = ADRSTART;
short minfloatregs = FLOATSTART;
short usedfloatregs = FLOATSTART;
short maxfloatregs = FLOATSTART;
short minalign = 1;
short intype = bt_int;
short uintype = bt_unsigned;
short littlend = TRUE;
char defineconfig[] = "_ICODE_";
char progname[] = "CCICODE";
char progext[] = ".ICD";
static char regname[] = "processor reg";
long stackadd = 3, stackmod = 0xfffffffcL;
long structadd = 3, structmod = 0xfffffffcL;
int prm_nasm = FALSE;
int stddefalignment = 4;
int stdenumsize = 4;
int stdretblocksize = 8;
int stdinttype = bt_int;
int stdunstype = bt_unsigned;
int stdintsize = 4;
int stdshortsize = 2;
int stdwchar_tsize = 2;
int stdlongsize = 4;
int stdlonglongsize = 8;
int stdfloatsize = 4;
int stddoublesize = 8;
int stdldoublesize = 10;
int stdaddrsize = 4;
int regdsize = 4;
int regasize = 4;
int regfsize = 10;
TYP stdfloat = 
{
    bt_float, 0, UF_DEFINED | UF_USED,  - 1,  - 1, 4
};
TYP stdimaginary = 
{
    bt_fimaginary, 0, UF_DEFINED | UF_USED,  - 1,  - 1, 4
};
TYP stddouble = 
{
    bt_double, 0, 0,  - 1,  - 1, 8
};
TYP stdrimaginary = 
{
    bt_rimaginary, 0, 0,  - 1,  - 1, 8
};
TYP stdlonglong = 
{
    bt_longlong, 0, 0,  - 1,  - 1, 8
};
TYP stdunsigned = 
{
    bt_unsigned, 0, 0,  - 1,  - 1, 4
};
TYP stdunsignedlonglong = 
{
    bt_unsignedlong, 0, 0,  - 1,  - 1, 8
};
TYP stdenum = 
{
    bt_enum, 1, UF_DEFINED,  - 1,  - 1, 4, 0, 0, "stdenum"
};
TYP stdconst = 
{
    bt_int, 1, UF_DEFINED,  - 1,  - 1, 4, 0, 0, "stdconst"
};
TYP stdstring = 
{
    bt_pointer, 0, 0,  - 1,  - 1, 4, 0, &stdchar
};
TYP stdint = 
{
    bt_int, 0, UF_DEFINED | UF_USED,  - 1,  - 1, 4
};
TYP stdlongdouble = 
{
    bt_longdouble, 0, 0,  - 1,  - 1, 10
};
TYP stduns = 
{
    bt_unsigned, 0, 0,  - 1,  - 1, 4
};

KEYWORDS prockeywords[] = 
{
    {
        0, "_trap", kw__trap, 
    }
    , 
    {
        0, "_interrupt", kw__interrupt
    }
    , 
    {
        0, "_intrinsic", kw__intrinsic, 4
    }
    , 
    {
        0, "_abs", kw__abs
    }
    , 
    {
        0, "_genword", kw__genword
    }
    , 
    {
        0, 0, 0
    }
};
/* #ifdef i386
{0,"_interrupt", kw__interrupt},{0,"_genbyte", kw__genword },
{0,"_EAX", kw__EAX}, {0,"_ECX", kw__ECX},{0,"_EDX", kw__EDX},
{0,"_EBX", kw__EBX},{0,"_ESP", kw__ESP},{0,"_EBP", kw__EBP},
{0,"_ESI", kw__ESI},{0,"_EDI", kw__EDI},
#else
{0,"_trap", kw__trap,}, {0,"_interrupt", kw__interrupt},
{0,"_abs", kw__abs }, {0,"_genword", kw__genword },
{0,"_D0",kw__D0},{0,"_D1",kw__D1},{0,"_D2",kw__D2},{0,"_D3",kw__D3},
{0,"_D4",kw__D4},{0,"_D5",kw__D5},{0,"_D6",kw__D6},{0,"_D7",kw__D7},
{0,"_A0",kw__A0},{0,"_A1",kw__A1},{0,"_A2",kw__A2},{0,"_A3",kw__A3},
{0,"_A4",kw__A4},{0,"_A5",kw__A5},{0,"_A6",kw__A6},{0,"_A7",kw__A7},
{0,"_FP0",kw__FP0},{0,"_FP1",kw__FP1},{0,"_FP2",kw__FP2},{0,"_FP3",kw__FP3},
{0,"_FP4",kw__FP4},{0,"_FP5",kw__FP5},{0,"_FP6",kw__FP6},{0,"_FP7",kw__FP7},
#endif */
/* #ifndef i386
case 'L':
prm_largedata = bool;
break;
case '2':
prm_68020 = bool;
break;
#endif */
/*
if (prm_68020)
prm_largedata = FALSE;
 */

void confsetup(void){}
int confcodegen(char s, int bool)
{
    return 0;
}

//-------------------------------------------------------------------------

int confoptgen(char s, int bool)
{
    switch (s)
    {
        case 'c':
            prm_optcode = bool;
            break;
        case 'f':
            prm_foldconst = bool;
            break;
        case 'd':
            prm_optdead = bool;
            break;
        case 'l':
            prm_locsub = bool;
            break;
        case 'g':
            prm_globsub = bool;
            break;
        case 'x':
            prm_copyprop = bool;
            break;
        case 'v':
            prm_optlive = bool;
            break;

        default:
            return 0;
    }
    return 1;
}

//-------------------------------------------------------------------------

int confcg(char ch, int bool)
{
    return 0;
}

//-------------------------------------------------------------------------

int alignment(int sc, TYP *tp)
{
    switch (tp->type)
    {
        case bt_char:
        case bt_unsignedchar:
        case bt_bool:
            return 1;
        case bt_short:
        case bt_unsignedshort:
            return 2;
        case bt_long:
        case bt_unsignedlong:
            return 4;
        case bt_longlong:
        case bt_unsignedlonglong:
            return 4;
        case bt_int:
        case bt_unsigned:
            return 4;
        case bt_enum:
            return 2;
        case bt_pointer:
        case bt_matchall:
            return 4;
        case bt_segpointer:
            return 2;
        case bt_farpointer:
            return 4;
        case bt_memberptr:
        case bt_float:
            return 4;
        case bt_double:
            return 8;
        case bt_longdouble:
            return 12;
        case bt_fcomplex:
            return 8;
        case bt_rcomplex:
            return 16;
        case bt_lrcomplex:
            return 24;
        case bt_class:
        case bt_struct:
        case bt_union:
            return tp->alignment;
        default:
            return 1;
    }
}

//-------------------------------------------------------------------------

int getalign(int sc, TYP *tp)
{
    int align = alignment(sc, tp);
    if (sc != sc_auto)
        if (packdata[packlevel] < align)
            align = packdata[packlevel];
    return align;
}

//-------------------------------------------------------------------------

long getautoval(long val)
{
    return val;
}

//-------------------------------------------------------------------------

funcvaluesize(int size)
{
    return 0;
}
