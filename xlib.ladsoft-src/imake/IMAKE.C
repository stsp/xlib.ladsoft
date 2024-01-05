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
#include <stdlib.h>
#include <ctype.h>
#include <dos.h>
#include "utype.h"
#include "cmdline.h"
#include "umem.h"
#include "imake.h"
#include "input.h"
#include "maker.h"
#include "register.h"
#include "spawner.h"
#include "interp.h"
#include "dump.h"
#include "env.h"
#include "parser.h"
#include "dump.h"
#include "..\version.h"

//#define VERSION 136
#define MV "0x148"

extern char *phipath;

/* Command line parameters */
BOOL prm_autodepends = FALSE;
BOOL prm_buildall = FALSE;
BOOL prm_ignoreenvmatch = FALSE;
char *prm_makefile = 0;
char *prm_includepath = 0;
BOOL prm_ignorestatus = FALSE;
BOOL prm_keep = FALSE;
BOOL prm_displaystamp = FALSE;
BOOL prm_nospawn = FALSE;
BOOL prm_displaydefs = FALSE;
BOOL prm_returnutd = FALSE;
BOOL prm_ignorebuiltin = FALSE;
BOOL prm_suppressdisplay = FALSE;
BOOL prm_usage = FALSE;

/* Global varieables */
int makerv = 0;

char *usage_text = "[switches] [target-list]\n\n"
    "target-list may be any list of targets, wildcards allowed.\n"
    "  When no target is specified the first explicit target\n"
    "  encountered is built.\n\n"
	"valid switches:\n"
    "  -e    don't allow override of environment vars\n"
    "  -fxxx specify makefile (defaults to makefile or makefile.mak)\n"
    "  -h    this help\n"
	"  -i    ignore return status of executed programs\n"
    "  -m    display time stamps\n"
    "  -n    test mode, display but don't execute commands\n"
    "  -p    display definitions and targets\n"
    "  -r    don't run builtins.mak\n"
    "  -s    supress display of commands being executed\n"
    "  -B    build target(s) completely ignoring time stamps\n"
    "  -Dxxx Define xxx\n"
	"  -Ixxx set directory for included make files\n"
    "  -K    keep intermediate files\n"
	"  -Uxxx Prevent xxx from being defined";

void InputSetup(char select, char *string);
void DefineSetup(char select, char *string);
void BoolSetup(char select, char *string);

ARGLIST ArgList[] = 
{
    {
        'a', ARG_BOOL, BoolSetup
    }
    , 
    {
        'e', ARG_BOOL, BoolSetup
    }
    , 
    {
        'f', ARG_CONCATSTRING, InputSetup
    }
    , 
    {
        'h', ARG_BOOL, BoolSetup
    }
    , 
    {
        'i', ARG_BOOL, BoolSetup
    }
    , 
    {
        'm', ARG_BOOL, BoolSetup
    }
    , 
    {
        'n', ARG_BOOL, BoolSetup
    }
    , 
    {
        'p', ARG_BOOL, BoolSetup
    }
    , 
    {
        'q', ARG_BOOL, BoolSetup
    }
    , 
    {
        'r', ARG_BOOL, BoolSetup
    }
    , 
    {
        's', ARG_BOOL, BoolSetup
    }
    , 
    {
        'B', ARG_BOOL, BoolSetup
    }
    , 
    {
        'D', ARG_CONCATSTRING, DefineSetup
    }
    , 
    {
        'I', ARG_CONCATSTRING, IncludeSetup
    }
    , 
    {
        'K', ARG_BOOL, BoolSetup
    }
    , 
    {
        'U', ARG_CONCATSTRING, DefineSetup
    }
    , 
    {
        0, 0, 0
    }
};
void BoolSetup(char select, char *string)
{
    switch (select)
    {
        case 'e':
            prm_ignoreenvmatch = TRUE;
            break;
        case 'h':
            prm_usage = TRUE;
            break;
        case 'i':
            prm_ignorestatus = TRUE;
            break;
        case 'm':
            prm_displaystamp = TRUE;
            break;
        case 'n':
            prm_nospawn = TRUE;
            break;
        case 'p':
            prm_displaydefs = TRUE;
            break;
        case 'q':
            prm_returnutd = TRUE;
            break;
        case 'r':
            prm_ignorebuiltin = TRUE;
            break;
        case 's':
            prm_suppressdisplay = TRUE;
            break;
        case 'B':
            prm_buildall = TRUE;
            break;
        case 'K':
            prm_keep = TRUE;
            break;
    }
}

/*
 * Set up for object file name
 */
static void InputSetup(char select, char *string)
{
    if (prm_makefile)
        DeallocateMemory(prm_makefile);
    prm_makefile = AllocateMemory(strlen(string) + 1);
    strcpy(prm_makefile, string);
}

//-------------------------------------------------------------------------

static void IncludeSetup(char select, char *string)
{
    if (prm_includepath)
        DeallocateMemory(prm_includepath);
    prm_includepath = AllocateMemory(strlen(string) + 1);
    strcpy(prm_includepath, string);
}

//-------------------------------------------------------------------------

void DefineSetup(char select, char *string)
{
    short buf[200],  *p = buf,  *q;
    BOOL def = TRUE;

    if (select == 'U')
        def = FALSE;
    while (*string &&  *string != EQUAL)
        *p++ =  *string++;
    *p++ = 0;

    if (*string)
        string++;
    q = p;
    do
    {
        *p++ =  *string++;
    }
    while (*(string - 1));
    RegisterMacro(buf, q, def, TRUE);
}

//-------------------------------------------------------------------------

void MajorDefines(char *string, char *value)
{
    short name[20], buf[200];
    ExpandFile(name, string);
    ExpandFile(buf, value);
    RegisterMacro(name, buf, TRUE, FALSE);
}

/*
 */
int main(int argc, char *argv[])
{
    char buffer[256],  *p;
    int i, x;
    FILEREC *fr;

    banner("imake Version %s %s", IMAKE_STRING_VERSION, PRODUCT_COPYRIGHT);

    RegisterInit();
    strcpy(buffer, argv[0]);
    p = strrchr(buffer, '\\');
    if (!p)
        p = strrchr(buffer, ':');
    if (p)
    {
        MajorDefines("MAKE", ++p);
        *--p = 0;
    }
    else
        buffer[0] = 0;

    ReadEnvironment();
    MajorDefines("__MSDOS__", "1");
    MajorDefines("__MAKE__", MV);
    MajorDefines("__PHIDOS__", "1");
    MajorDefines("MAKEFLAGS", "UNKNOWN");
    MajorDefines("MAKEDIR", buffer);
    if (!prm_ignorebuiltin)
    {
        if (phipath)
            if (!ReadMakeFile(phipath, "BUILTINS.MAK"))
                ReadMakeFile(buffer, "BUILTINS.MAK");
    }
    buffer[0] = 0;
    /* Scan command line for switches */
    if (!parse_args(&argc, argv, TRUE) || prm_usage)
        usage(argv[0]);

    for (i = 1; i < argc; i++)
        RegisterTarget(argv[i], FALSE);

    if (prm_makefile)
    {
        if (!ReadMakeFile(0, prm_makefile))
            fatal("Couldn't open %s for read", prm_makefile);
    }
    else
        ReadMakeFile(0, "MAKEFILE");
	
	AllAutoDeps();

    if (prm_displaydefs)
        DisplayDefs();

    fr = RecurseAllTargets();
    DoTargets(fr);

    if (prm_returnutd)
        return (fr != 0);

    return (makerv);
}
