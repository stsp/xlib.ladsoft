/* 
ITOUCH
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
/*
 * itouch.c
 *
 */
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <memory.h>
#include <malloc.h>
#include <time.h>
#include <dos.h>
#include "cmdline.h"
#include "itouch.p"
#include "..\version.h"

// #define VERSION 105

char *usage_text = "[-v] ...";

static BOOL verbose = FALSE;
static unsigned short absdate, abstime;

void boolargs(char select);
ARGLIST ArgList[] = 
{
    {
        'v', ARG_SWITCH, boolargs
    }
    , 
    {
        0, 0, 0
    }
};

static void boolargs(char select)
{
    switch (select)
    {
        case 'v':
            verbose = TRUE;
            break;
    }
}

//-------------------------------------------------------------------------

#ifdef BORLANDC
    int _dos_getpwd(char *string, int null)
    {
        char buf[256],  *p;
        int l;
        getcwd(buf, 255);
        p = strchr(buf, ':');
        if (p)
        {
            p++;
            if (*p == '\\')
                p++;
        }
        else
            p = buf;
        l = strlen(p);

        strcpy(string, p);
        return l;
    }
#endif 
void itouch(FILE *inf, FILE *outf, char *fname)
{
    char buf[1];
    int handle;
    if (!_dos_open(fname, 2, &handle))
    {
        _dos_setftime(handle, absdate, abstime);
        _dos_close(handle);
    }
}

/* Main routine */
int main(int argc, char *argv[])
{
    struct tm *tms;
    time_t xtime;

    banner("itouch Version %s %s", ITOUCH_STRING_VERSION, PRODUCT_COPYRIGHT);

    if (!parse_args(&argc, argv, FALSE) || (argc == 1))
        usage(argv[0]);

    time(&xtime);
    tms = localtime(&xtime);

    abstime = (tms->tm_hour << 11) | (tms->tm_min << 5) | ((tms->tm_sec + 1) /
        2);
    absdate = ((tms->tm_year - 80) << 9) | ((tms->tm_mon + 1) << 5) | (tms
        ->tm_mday);

    /* Run through all files making the changes */
    FileRecurse(argc - 1, argv + 1, verbose ? "Touching" : 0, itouch, 0, FALSE);

    return (0);
}
