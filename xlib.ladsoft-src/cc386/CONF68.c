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
#include <string.h>
#include "lists.h"
#include "expr.h"
#include "c.h"
extern int packdata[], packlevel;
extern long lc_maxauto, framedepth;
extern int prm_linkreg;
extern SYM *currentfunc;

#ifndef COLDFIRE
    char PROGNAME[] = "CC68K";
    char ENVNAME[] = "CC68K";
    char SOURCEXT[] = ".SRC";
    char GLBDEFINE[] = "_m68k_";
    char OBJEXT[] = ".o";
#else 
    char PROGNAME[] = "cfcc";
    char ENVNAME[] = "cfcc";
    char SOURCEXT[] = ".s";
    char GLBDEFINE[] = "_m68k_";
    char OBJEXT[] = "";
#endif 

#ifdef BRIEFHELP
    char *usage_text = "[+e/+i/f+l/w+A/C/D/E/I/O] file list";
#else 
    char *usage_text = "[options] files\n""/9     - C99 mode\n"
        "+e     - dump errors to file        /fname - specify parameter file\n"
        "+i     - dump preprocessed file     +l     - dump listing file\n"
        "/w-xxx - disable a warning          +A     - disable extensions\n"
        "/Dxxx  - define something           /E[+]nn- max number of errors\n"
        "/Ipath - specify include path       +Q     - quiet mode\n"
        "-R     - disable RTTI               +S     - compile via assembly\n"
        "Codegen parameters: (/C[+][-][params])\n""  -b   - no BSS\n"
        "  -c   - don't optimize to CLR      +d     - display diagnostics\n"
        "  +1   - generate 68010 code        +2     - generate 68020 code\n"
        "  -f   - generate coldfire code     -l     - no C source in ASM file\n""  -m   - no leading underscores     +r     - reverse order of bit ops\n""  +s   - small data model           +D     - Data Register Relative\n""  +L   - large data model           +P     - PC Relative\n"
    #ifdef XXXXX
        "  -R   - no param link register\n"
    #endif 
    "  +S   - add stack check code\n" //       +Z     - add profiler calls\n"
    "Optimizer parameters (/O[+][-][params])\n"
        "  +a   - peepopt inline ASM           +i     - inline intrinsics\n"
        "  -m   - don't merge constants        -p     - no peephole opts\n"
    /*"  -RA  - no address register optimizations\n" */
    /*"  -RD  - no data register optimizations\n" */
    /*"  -RF  - no fp register optimizations\n"*/
    "Time: " __TIME__ "  Date: " __DATE__;
#endif 

#ifdef COLDFIRE
    int prm_68020 = TRUE; /* Enabled yet ColdFire restrictions apply */
    int prm_68010 = TRUE;
    int prm_coldfire = TRUE;
    int prm_smalldata = TRUE;
    int prm_smallcode = TRUE;
    int cf_maxaddress = 20;
#else 
    int prm_68020 = FALSE; /* Enabled yet ColdFire restrictions apply */
    int prm_68010 = FALSE;
    int prm_coldfire = FALSE;
    int prm_smalldata = FALSE;
    int prm_smallcode = FALSE;
    int cf_maxaddress = 21;
#endif 
int prm_stackalign = FALSE;
int prm_browse = FALSE;
int stdmemberptrsize = 8;
int prm_nasm = TRUE; /* exists ONLY to support #pragma library() */
int prm_buggyclr = FALSE;
int prm_datarel = FALSE;
int prm_pcrel = FALSE;
int prm_microlink = 0;
int linkreg;
int basereg;
int prm_phiform = 0;
int prm_largedata = FALSE;
int cf_maxdata = 8;
int cf_maxfloat = 40;
int cf_freeaddress = 2;
int cf_freedata = 3;
int cf_freefloat = 3;
int stackadd = 3;
int stackmod =  - 4;
int strucadd = 3;
int strucmod =  - 4;
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
int stdldoublesize = 12;
int stdaddrsize = 4;
int regdsize = 4;
int regasize = 4;
int regfsize = 12;

extern TYP stdchar;
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
        0, "_trap", kw__trap, 4
    }
    , 
    {
        0, "_interrupt", kw__interrupt, 4
    }
    , 
    {
        0, "_absolute", kw__abs, 4
    }
    , 
    {
        0, "_genword", kw__genword, 4
    }
    , 
    {
        0, "pascal", kw__pascal, 4
    }
    , 
    {
        0, "__cdecl", kw__cdecl, 0
    }
    , 
    {
        0, "_cdecl", kw__cdecl, 0
    }
    , 
    {
        0, "cdecl", kw__cdecl, 4
    }
    , 
    {
        0, "stdcall", kw__stdcall, 4
    }
    , 
    {
        0, "_intrinsic", kw__intrinsic, 4
    }
    , 
    {
        0, "_D0", kw_D0, 4
    }
    , 
    {
        0, "_D1", kw_D1, 4
    }
    , 
    {
        0, "_D2", kw_D2, 4
    }
    , 
    {
        0, "_D3", kw_D3, 4
    }
    , 
    {
        0, "_D4", kw_D4, 4
    }
    , 
    {
        0, "_D5", kw_D5, 4
    }
    , 
    {
        0, "_D6", kw_D6, 4
    }
    , 
    {
        0, "_D7", kw_D7, 4
    }
    , 
    {
        0, "_A0", kw_A0, 4
    }
    , 
    {
        0, "_A1", kw_A1, 4
    }
    , 
    {
        0, "_A2", kw_A2, 4
    }
    , 
    {
        0, "_A3", kw_A3, 4
    }
    , 
    {
        0, "_A4", kw_A4, 4
    }
    , 
    {
        0, "_A5", kw_A5, 4
    }
    , 
    {
        0, "_A6", kw_A6, 4
    }
    , 
    {
        0, "_A7", kw_A7, 4
    }
    , 
    {
        0, "_FP0", kw_F0, 4
    }
    , 
    {
        0, "_FP1", kw_F1, 4
    }
    , 
    {
        0, "_FP2", kw_F2, 4
    }
    , 
    {
        0, "_FP3", kw_F3, 4
    }
    , 
    {
        0, "_FP4", kw_F4, 4
    }
    , 
    {
        0, "_FP5", kw_F5, 4
    }
    , 
    {
        0, "_FP6", kw_F6, 4
    }
    , 
    {
        0, "_FP7", kw_F7, 4
    }
    , 
    {
        0, 0, 0
    }
};

char *registers[] = 
{
    "D0", "D1", "D2", "D3", "D4", "D5", "D6", "D7", "", "", "", "", "", "", "",
        "", "A0", "A1", "A2", "A3", "A4", "A5", "A6", "A7", "", "", "", "", "",
        "", "", "", "FP0", "FP1", "FP2", "FP3", "FP4", "FP5", "FP6", "FP7", "",
        "", "", "", "", "", "", ""
};

int confcodegen(char s, int bool)
{
    switch (s)
    {
        case 'f':
            prm_coldfire = bool;
            break;
        case 'c':
            prm_buggyclr = !bool;
            break;
        case 'L':
             /* 68000 specific */
            prm_largedata = bool;
            break;
        case '2':
             /* 68020 specific */
            prm_68020 = bool;
            break;
        case '1':
             /* 68020 specific */
            prm_68010 = bool;
            break;
            //               case 'P':
            //                  prm_phiform = bool;
            //                  break;
        case 's':
            prm_smalldata = prm_smallcode = bool;
            break;
        case 'D':
            prm_datarel = bool;
            break;
        case 'P':
            prm_pcrel = bool;
            break;
        default:
            return 0;
    }
    return 1;
}

//-------------------------------------------------------------------------

void confsetup(void)
{
    if (prm_68020)
        prm_largedata = FALSE;
    if (prm_phiform)
        prm_linkreg = FALSE;
    if (prm_coldfire)
    {
        prm_68020 = TRUE; /* Enabled yet ColdFire restrictions apply */
        prm_68010 = TRUE;
        linkreg = 5;
        basereg = 4;
        if (prm_phiform || prm_linkreg)
        {
            if (prm_datarel)
            {
                cf_maxaddress = 20;
            }
            else
                cf_maxaddress = 21;
        }
        else
        {
            /* Note that the link reg may be used by trap calls even
             * though nothing else uses it, so it can never be freed
             */
            if (prm_datarel)
            {
                cf_maxaddress = 20;
            }
            else
            {
                cf_maxaddress = 21;
            }
        }
    }
    else
    {
        linkreg = 6;
        basereg = 5;
        if (prm_phiform || prm_linkreg)
        {
            if (prm_datarel)
            {
                cf_maxaddress = 21;
            }
            else
                cf_maxaddress = 22;
        }
        else
        {
            /* Note that the link reg may be used by trap calls even
             * though nothing else uses it, so it can never be freed
             */
            if (prm_datarel)
            {
                cf_maxaddress = 21;
            }
            else
            {
                cf_maxaddress = 22;
            }
        }
    }
}

//-------------------------------------------------------------------------

int alignment(int type, TYP *tp)
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
            return 4;
        case bt_pointer:
        case bt_matchall:
            return 4;
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
        case bt_struct:
        case bt_class:
        case bt_union:
            return tp->sp->structAlign ? tp->sp->structAlign : 1;
        default:
            return 1;
    }
}

//-------------------------------------------------------------------------

int getalign(int sc, TYP *tp)
{
    int align = alignment(sc, tp);
    if (sc != sc_auto)
    {
        if (packdata[packlevel] < align)
            align = packdata[packlevel];
        if (prm_68020 && align < 2)
            align = 2;
    }
    return align;
}

//-------------------------------------------------------------------------

long getautoval(long val)
{

    if (prm_linkreg && !currentfunc->intflag)
        return val;
    if (val >= 0)
        if (prm_phiform || currentfunc->intflag)
            return framedepth + val;
        else
            return val;
        else
            return lc_maxauto + val;
}

//-------------------------------------------------------------------------

int funcvaluesize(int size)
{
    return 4-size;
}
